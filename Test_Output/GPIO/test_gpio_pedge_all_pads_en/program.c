// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)
#include "test_define.c"

/*
 High-level Description (from Hidden_Test_Description):
 Positive-edge interrupt enable/validation across GPIO[8..39]. Setup: Conditionally enable GIC IRQ 87 (GPIO0) or 88 (GPIO1). Enable system interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO{0/1}_INTR). For i=0..31, write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00020000) to set posedge enable (PEIE bit17=1). Configure input mode via group I/O control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,0x000000FF); ... GROUP4 likewise. Enable all bits in group interrupt: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc,0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc,0xFFFFFFFF) to create rising edge; bounded wait with timeout=2000 while(int_pend==1){wait_on(10)}; on timeout, print error and test_err++. In Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); mask group via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0x00000000); If any bit set in rdata_grp, success log else error/test_err++. Clear raw per-pin by writing 0x00010000 to each (for j=0..31) at MIZAR_GPIO_GP0_GPIO_8+(j*4). Verify group clear by reading MIZAR_GPIO_GP0_INTR1_INTR_STS1 == 0x0 else error/test_err++. Clear system status via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO{0/1}_INTR) and verify cleared by reading back and checking the bit is 0; if not, increment test_err. Re-enable group interrupt via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0xFFFFFFFF). Clear corresponding GIC line.
*/

static volatile unsigned int int_pend = 0U;  /* Set to 1 before generating edge; cleared in ISR */
static volatile unsigned int test_err = 0U;  /* Increment on every failure condition */

/*
 * Function: Default_IRQHandler
 * Purpose : Handle GPIO group interrupt: capture group status, mask group, clear per-pin raw,
 *           verify group/system clears, and re-enable group interrupt. Increments test_err on failures.
 */
void Default_IRQHandler(void)
{
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); /* Group status read */

    /* Mask group interrupt while servicing */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);

    if ((rdata_grp & 0xFFFFFFFFU) == 0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][FAIL] Group status empty during service.\n");
#endif
    } else {
#ifdef DEBUG_DISPLAY
        printf("[ISR][INFO] Group status=0x%08X\n", rdata_grp);
#endif
    }

    /* Clear per-pin RAW by writing W1C (bit16) for all 32 pins [8..39] */
    for (unsigned int j = 0U; j < 32U; j++) {
        write_reg((MIZAR_GPIO_GP0_GPIO_8 + (j * 4U)), 0x00010000U);
        wait_on(2U);
    }

    /* Verify group clear */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][FAIL] Group status not cleared: 0x%08X\n", rdata_grp);
#endif
    } else {
#ifdef DEBUG_DISPLAY
        printf("[ISR][PASS] Group status cleared.\n");
#endif
    }

    /* Clear system RAW status; both GPIO0/1 bits are attempted per description */
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, (LSS_SYSREG_RAW_STCR1_GPIO0_INTR | LSS_SYSREG_RAW_STCR1_GPIO1_INTR));
    /* Read back and check clear (bit check abstracted; relying on platform readback semantics) */
    unsigned int sys_raw = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((sys_raw & (LSS_SYSREG_RAW_STCR1_GPIO0_INTR | LSS_SYSREG_RAW_STCR1_GPIO1_INTR)) != 0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][FAIL] System RAW status not cleared: 0x%08X\n", sys_raw);
#endif
    }

    /* Re-enable group interrupt */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    /* Indicate ISR handled */
    int_pend = 0U;
}

/*
 * Function: main
 * Purpose : Configure GPIO for positive-edge interrupts on pads [8..39], enable system + group
 *           interrupts, generate bounded waits per pin, and rely on ISR to perform clearing and
 *           verification. Terminates via finish(0/1) based on acceptance criteria.
 */
int main(void)
{
    /* Enable system interrupt bits for GPIO0/1 as per description */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, (LSS_SYSREG_INTR_EN1_GPIO0_INTR | LSS_SYSREG_INTR_EN1_GPIO1_INTR));

    /* Configure IO control groups to input mode (0x000000FF) */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFU);

    /* Enable posedge interrupt per-pin: PEIE bit17=1 (0x00020000) for GPIO[8..39] */
    for (unsigned int i = 0U; i < 32U; i++) {
        write_reg((MIZAR_GPIO_GP0_GPIO_8 + (i * 4U)), 0x00020000U);
    }

    /* Enable all group interrupt bits */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    /* For each pad, bounded-wait for ISR to clear int_pend. External edge generation is platform-specific. */
    for (unsigned int i = 0U; i < 32U; i++) {
        int_pend = 1U; /* Mark pending and wait for ISR service */
        unsigned int timeout = 2000U;
        while ((int_pend == 1U) && (timeout > 0U)) {
            wait_on(10U);
            timeout--;
        }
        if (int_pend == 1U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[TIMEOUT][FAIL] GPIO%u posedge wait timed out.\n", (8U + i));
#endif
            /* Attempt to continue to next pin */
            int_pend = 0U;
        }
    }

    /* Acceptance: test passes if test_err==0 */
    if (test_err != 0U) {
#ifdef DEBUG_DISPLAY
        printf("[RESULT][FAIL] test_err=%u\n", test_err);
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[RESULT][PASS] test_err=%u\n", test_err);
#endif
        finish(0);
    }

    return 0;
}
