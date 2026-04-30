// Author - AI Force 1.3.2. Date 30-04-2026
// (EMBENGG-SYSAPPS)
#include "test_define.c"

// Purpose: I2C0 (master) to I2C1 (slave) DMA transfer in Standard mode; verify interrupt occurrence/clear and data integrity.
// Derived from Hidden_Test_Description.

// Entry point: test_case
// Implements the exact termination criteria from Hidden_Validation_Acceptance_Criteria.
void test_case(void)
{
    unsigned int test_err = 0U;

    // 1) Interrupt: check interrupt status for I2C0
    unsigned int int_status = (unsigned int)read_reg(MIZAR_I2C0_INTR_STS); // I2C0 Interrupt Status
#ifdef DEBUG_DISPLAY
    printf("[I2C-ST][INT] MIZAR_I2C0_INTR_STS=0x%08x\n", int_status);
#endif
    if (int_status != 0x0010U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[I2C-ST][INT][FAIL] Expected 0x0010, got 0x%08x\n", int_status);
#endif
    }

    // Clear and re-check
    write_reg(MIZAR_I2C0_INTR_CLR, int_status); // I2C0 Interrupt Clear
    unsigned int sts_after_clr = (unsigned int)read_reg(MIZAR_I2C0_INTR_STS);
    unsigned int raw_stcr0 = (unsigned int)read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
#ifdef DEBUG_DISPLAY
    printf("[I2C-ST][INT] After clear: INTR_STS=0x%08x RAW_STCR0=0x%08x\n", sts_after_clr, raw_stcr0);
#endif
    if (sts_after_clr != 0x00000000U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[I2C-ST][INT][FAIL] INTR_STS not zero after clear: 0x%08x\n", sts_after_clr);
#endif
    }
    if ((raw_stcr0 & LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT) != 0x00000000U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[I2C-ST][INT][FAIL] RAW_STCR0 I2C0_INTERRUPT bit not cleared. RAW_STCR0=0x%08x\n", raw_stcr0);
#endif
    }

    // 2) Data integrity: 5 words from SRAM_ADDR_1 must match SRAM_ADDR_2
    for (unsigned int i = 0; i < 5U; i++) {
        volatile unsigned int *p1 = (volatile unsigned int *)(SRAM_ADDR_1 + (4U * i));
        volatile unsigned int *p2 = (volatile unsigned int *)(SRAM_ADDR_2 + (4U * i));
        unsigned int v1 = *p1;
        unsigned int v2 = *p2;
#ifdef DEBUG_DISPLAY
        printf("[I2C-ST][DATA] idx=%u src=0x%08x dst=0x%08x\n", i, v1, v2);
#endif
        if (v1 != v2) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[I2C-ST][DATA][FAIL] idx=%u mismatch src=0x%08x dst=0x%08x\n", i, v1, v2);
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[I2C-ST][SUMMARY] test_err=%u\n", test_err);
#endif

    if (test_err == 0U) {
        finish(0); // PASS
    } else {
        finish(1); // FAIL
    }
}
