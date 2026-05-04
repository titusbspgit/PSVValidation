// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
Hidden_Test_Description:
Rising-edge interrupt enable test for all GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88). Enables system-register interrupt routing: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) to set peie=1 (bit17). wait_on(10). Configure input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). wait_on(10). Enable all group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF) to create a rising edge; poll with timeout=2000 on int_pend with wait_on(10); on timeout print error, increment test_err, and break. After ISR return, write_reg(0xA0243ffc, 0x00000000); wait_on(10). finish(test_err). Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) to mask; if ((rdata_grp & 0xFFFFFFFF) == 0) { print error; test_err++; } For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000) to clear per-pin raw (iclr=1); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { print error; test_err++; } Clear system-register raw: #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { print error; test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { print error; test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).
*/

static volatile uint32_t int_pend = 0U;
static volatile uint32_t test_err = 0U;
static volatile uint32_t g_active_index = 0U; /* current pin index 0..31 */

/* ISR invoked by platform on GPIO interrupt */
void Default_IRQHandler(void)
{
    uint32_t i = g_active_index;
    uint32_t wr_val = (1U << (i & 31U));
    int_pend = 0U; /* Signal main loop */

    /* Read group masked status and require non-zero */
    uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    /* Mask all group interrupts while servicing */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);
    if ((rdata_grp & 0xFFFFFFFFU) == 0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][POS] Group STS zero\n");
#endif
    }

    /* Clear per-pin raw (iclr=1) for all pins */
    for (uint32_t j = 0; j < 32U; j++) {
        uint32_t raddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4U));
        write_reg(raddr, 0x00010000U);
        wait_on(2);
    }

    /* Verify group status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][POS] Group STS not cleared sts=0x%08lx\n", (unsigned long)rdata_grp);
#endif
    }

    /* Clear system-register raw and verify cleared */
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, (LSS_SYSREG_RAW_STCR1_GPIO0_INTR | LSS_SYSREG_RAW_STCR1_GPIO1_INTR));
    uint32_t rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & (LSS_SYSREG_RAW_STCR1_GPIO0_INTR | LSS_SYSREG_RAW_STCR1_GPIO1_INTR)) != 0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][POS] SYSREG RAW not cleared r=0x%08lx\n", (unsigned long)rdata);
#endif
    }

    /* Re-enable all group interrupts and clear IRQ lines */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);
    GIC_ClearIRQ(GPIO0_IRQ_ID);
    GIC_ClearIRQ(GPIO1_IRQ_ID);
}

int main(void)
{
#ifdef DEBUG_DISPLAY
    printf("[START] test_gpio_pedge_all_pads_en\n");
#endif
    test_err = 0U;

    /* Enable platform IRQs for both GPIO instances (deterministic enabling) */
    GIC_EnableIRQ(GPIO0_IRQ_ID);
    GIC_EnableIRQ(GPIO1_IRQ_ID);

    /* Enable system-register interrupt routing for GPIO0 and GPIO1 */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, (LSS_SYSREG_INTR_EN1_GPIO0_INTR | LSS_SYSREG_INTR_EN1_GPIO1_INTR));

    /* Enable peie=1 (bit17) on all pins */
    for (uint32_t i = 0; i < 32U; i++) {
        uint32_t addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4U));
        write_reg(addr, 0x00020000U);
        wait_on(10);
    }

    /* Configure IO control groups to input mode (0xFF) */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFU);
    wait_on(10);

    /* Enable all group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    /* For each pin, generate rising edge and wait */
    for (uint32_t i = 0; i < 32U; i++) {
        g_active_index = i;
        write_reg(MIZAR_PAD_CTRL_GPIO, 0x00000000U);
        wait_on(10);
        int_pend = 1U;
        write_reg(MIZAR_PAD_CTRL_GPIO, 0xFFFFFFFFU);

        uint32_t timeout = 2000U;
        while ((int_pend != 0U) && (timeout-- > 0U)) {
            wait_on(10);
        }
        if (timeout == 0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[TIMEOUT][POS] i=%lu did not trigger\n", (unsigned long)i);
#endif
            break; /* Per procedure */
        }
        /* After ISR return, drive low again */
        write_reg(MIZAR_PAD_CTRL_GPIO, 0x00000000U);
        wait_on(10);
    }

    if (test_err == 0U) {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] FAIL errors=%lu\n", (unsigned long)test_err);
#endif
        finish(1);
    }
    return 0;
}
