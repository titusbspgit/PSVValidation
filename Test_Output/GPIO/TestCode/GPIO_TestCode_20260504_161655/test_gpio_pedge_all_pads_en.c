/*
 * Test: test_gpio_pedge_all_pads_en
 * Description (verbatim from metadata):
 * Rising-edge interrupt enable test for all GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88). Enables system-register interrupt routing: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) to set peie=1 (bit17). wait_on(10). Configure input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). wait_on(10). Enable all group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF) to create a rising edge; poll with timeout=2000 on int_pend with wait_on(10); on timeout print error, increment test_err, and break. After ISR return, write_reg(0xA0243ffc, 0x00000000); wait_on(10). finish(test_err). Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) to mask; if ((rdata_grp & 0xFFFFFFFF) == 0) { print error; test_err++; } For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000) to clear per-pin raw (iclr=1); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { print error; test_err++; } Clear system-register raw: #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { print error; test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { print error; test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).
 */

#include <stdint.h>
#include <stdio.h>

/* Expected platform APIs/macros similar to negedge test. */

#define PAD_CTRL_ADDR 0xA0243ffcu

static volatile uint32_t int_pend = 0;
static volatile uint32_t test_err = 0;
static volatile uint32_t i = 0;

void Default_IRQHandler(void)
{
    uint32_t wr_val = (1u << i);
    int_pend = 0;

    uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    /* Mask group interrupts during service */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        ++test_err;
        printf("[test_gpio_pedge_all_pads_en][ISR] Group status 0 on entry\n");
    }

    /* Clear per-pin raw for all 32 pins */
    for (uint32_t j = 0; j < 32u; ++j) {
        uint32_t raddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(raddr, 0x00010000u); /* iclr=1 */
        wait_on(2);
    }

    /* Verify group masked status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) {
        ++test_err;
        printf("[test_gpio_pedge_all_pads_en][ISR] Group masked status not cleared (0x%08lx)\n",
               (unsigned long)rdata_grp);
    }

    /* Clear system-register raw and verify */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    uint32_t rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
        ++test_err;
        printf("[test_gpio_pedge_all_pads_en][ISR] SYSREG RAW not cleared for GPIO0 (0x%08lx)\n", (unsigned long)rdata);
    }
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    uint32_t rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
        ++test_err;
        printf("[test_gpio_pedge_all_pads_en][ISR] SYSREG RAW not cleared for GPIO1 (0x%08lx)\n", (unsigned long)rdata);
    }
#endif

    /* Re-enable all group interrupts and clear GIC */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif
}

void test_gpio_pedge_all_pads_en(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Enable peie=1 for all pins: 0x00020000 */
    for (i = 0; i < 32u; ++i) {
        uint32_t addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr, 0x00020000u);
    }
    wait_on(10);

    /* Configure group IO control for input mode per metadata */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    /* Enable all group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (i = 0; i < 32u; ++i) {
        /* Drive low, arm, then drive high to create rising edge */
        write_reg(PAD_CTRL_ADDR, 0x00000000u);
        wait_on(10);

        int_pend = 1;

        write_reg(PAD_CTRL_ADDR, 0xFFFFFFFFu);

        int timeout = 2000;
        while ((int_pend == 1u) && (--timeout > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
            printf("[test_gpio_pedge_all_pads_en] TIMEOUT waiting for pin %lu rising edge interrupt\n", (unsigned long)i);
            ++test_err;
            break;
        }

        /* Return to low before next pin */
        write_reg(PAD_CTRL_ADDR, 0x00000000u);
        wait_on(10);
    }

    finish((int)test_err);
}
