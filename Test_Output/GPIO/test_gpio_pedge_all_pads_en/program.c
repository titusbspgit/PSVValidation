// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)
/*
 * Program for test_gpio_pedge_all_pads_en
 * Description (verbatim):
 * Rising-edge interrupt enable test for all GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88). Enables system-register interrupt routing: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) to set peie=1 (bit17). wait_on(10). Configure input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). wait_on(10). Enable all group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF) to create a rising edge; poll with timeout=2000 on int_pend with wait_on(10); on timeout print error, increment test_err, and break. After ISR return, write_reg(0xA0243ffc, 0x00000000); wait_on(10). finish(test_err). Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) to mask; if ((rdata_grp & 0xFFFFFFFF) == 0) { print error; test_err++; } For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000) to clear per-pin raw (iclr=1); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { print error; test_err++; } Clear system-register raw: #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { print error; test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { print error; test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).
 */

#include "test_define.c"

void program_main(void)
{
    unsigned int error_cnt = 0u;

    /* 1) Enable system-register interrupt routing to GPIO */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* 2) Enable peie=1 (bit17) for all GPIO[8..39] */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(raddr, 0x00020000u);
    }
    wait_on(10);

    /* 3) Configure IO control groups for input mode */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    /* 4) Enable all group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    /* 5) For each pin: poll for masked status; upon observation, clear per-pin raw across all pins, verify cleared, clear sysreg raw, and continue */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned int wr_val = (1u << i);

        int timeout = 2000;
        while (timeout-- > 0) {
            unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if ((sts & wr_val) != 0u) {
                break; /* observed */
            }
            wait_on(10);
        }
        if (timeout <= 0) {
            ++error_cnt; /* timeout -> failure */
#ifdef DEBUG_DISPLAY
            printf("[test_gpio_pedge_all_pads_en] TIMEOUT waiting pin %u rise\n", i);
#endif
            break; /* per metadata */
        }

        /* Mask during service */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

        /* Clear per-pin raw for all 32 pins (iclr=1) */
        for (unsigned int j = 0; j < 32u; ++j) {
            unsigned long raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
            write_reg(raddr2, 0x00010000u);
            wait_on(2);
        }

        /* Verify masked status cleared */
        {
            unsigned int sts2 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (sts2 != 0u) {
                ++error_cnt;
#ifdef DEBUG_DISPLAY
                printf("[test_gpio_pedge_all_pads_en] STS not cleared (0x%08X)\n", sts2);
#endif
            }
        }

        /* Clear system-register raw and verify */
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        {
            unsigned int rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
            if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
                ++error_cnt;
#ifdef DEBUG_DISPLAY
                printf("[test_gpio_pedge_all_pads_en] SYSREG RAW not cleared GPIO0 (0x%08X)\n", rdata);
#endif
            }
        }
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        {
            unsigned int rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
            if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
                ++error_cnt;
#ifdef DEBUG_DISPLAY
                printf("[test_gpio_pedge_all_pads_en] SYSREG RAW not cleared GPIO1 (0x%08X)\n", rdata);
#endif
            }
        }
#endif

        /* Re-enable interrupts */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    }

    /* Acceptance: test passes only if error_cnt==0 */
    if (error_cnt == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[test_gpio_pedge_all_pads_en] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[test_gpio_pedge_all_pads_en] FAIL err=%u\n", error_cnt);
#endif
        finish(1);
    }
}
