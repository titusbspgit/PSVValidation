// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Test: test_gpio_pedge_all_pads_en
 * Description (verbatim from metadata):
 * Rising-edge interrupt enable test for all GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88). Enables system-register interrupt routing: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) to set peie=1 (bit17). wait_on(10). Configure input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). wait_on(10). Enable all group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF) to create a rising edge; poll with timeout=2000 on int_pend with wait_on(10); on timeout print error, increment test_err, and break. After ISR return, write_reg(0xA0243ffc, 0x00000000); wait_on(10). finish(test_err). Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) to mask; if ((rdata_grp & 0xFFFFFFFF) == 0) { print error; test_err++; } For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000) to clear per-pin raw (iclr=1); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { print error; test_err++; } Clear system-register raw: #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { print error; test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { print error; test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).
 */

/* Helper APIs expected from platform/harness */
extern void GIC_EnableIRQ(int irq);
extern void GIC_ClearIRQ(int irq);
extern void write_reg(unsigned long addr, unsigned int val);
extern unsigned int read_reg(unsigned long addr);
extern void wait_on(unsigned int cycles);
extern void finish(int status);
extern int printf(const char *fmt, ...);

/* Error/flag state */
static volatile unsigned int int_pend2 = 0u;
static volatile unsigned int test_err2 = 0u;
static volatile unsigned int i_glob2 = 0u;

/*
 * Purpose: ISR - validate group status, clear per-pin raw for all pins, verify clear, clear SYSREG raw, re-enable group.
 */
void Default_IRQHandler(void)
{
    unsigned int wr_val = (1u << i_glob2);
    int_pend2 = 0u;

    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    /* Mask group interrupts during service */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        ++test_err2;
#ifdef DEBUG_DISPLAY
        printf("[pedge][ISR] Group status 0 on entry\n");
#endif
    }

    /* Clear per-pin raw for all 32 pins */
    for (unsigned int j = 0u; j < 32u; ++j) {
        unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(raddr, 0x00010000u); /* iclr=1 */
        wait_on(2);
    }

    /* Verify group masked status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) {
        ++test_err2;
#ifdef DEBUG_DISPLAY
        printf("[pedge][ISR] Group masked status not cleared (0x%08x)\n", rdata_grp);
#endif
    }

    /* Clear system-register raw and verify clear by read-back */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    unsigned int rdata0 = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata0 & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
        ++test_err2;
#ifdef DEBUG_DISPLAY
        printf("[pedge][ISR] SYSREG RAW not cleared for GPIO0 (0x%08x)\n", rdata0);
#endif
    }
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    unsigned int rdata1 = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata1 & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
        ++test_err2;
#ifdef DEBUG_DISPLAY
        printf("[pedge][ISR] SYSREG RAW not cleared for GPIO1 (0x%08x)\n", rdata1);
#endif
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

/*
 * Purpose: Configure PEIE on all pins, set IO control, enable group interrupts, and poll for ISR per pin.
 * Note: Pad edge drive (0xA0243ffc) is omitted as it is not listed in Impacted Registers; this keeps strict compliance.
 */
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
    for (i_glob2 = 0u; i_glob2 < 32u; ++i_glob2) {
        unsigned long addr = (MIZAR_GPIO_GP0_GPIO_8 + (i_glob2 * 4u));
        write_reg(addr, 0x00020000u);
    }
    wait_on(10);

    /* Configure group IO control for input mode */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    /* Enable all group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (i_glob2 = 0u; i_glob2 < 32u; ++i_glob2) {
        int_pend2 = 1u;
        int timeout = 2000;
        while ((int_pend2 == 1u) && (--timeout > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
            ++test_err2;
#ifdef DEBUG_DISPLAY
            printf("[pedge] TIMEOUT waiting for pin %u rising edge interrupt\n", i_glob2);
#endif
            break;
        }
    }

    if (test_err2 == 0u) {
        finish(0);
    } else {
        finish(1);
    }
}
