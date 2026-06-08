// Author - AI Force 1.3.2. Date 08-06-2026
// (EMBENGG-SYSAPPS)

/*
 * Testcase: test_gpio_negedge_intr_en
 * Description (from Meta):
 *   The test configures all GPIO pads for negative-edge interrupt detection
 *   and enables GPIO interrupt generation. It sets pad direction/output
 *   control to generate a high-to-low transition on each pad sequentially.
 *   Global interrupts are enabled and the ISR (Default_IRQHandler) is
 *   expected to service the pad interrupts. For each pad: drive the pad high,
 *   then low to create a negative edge; confirm the corresponding bit is set
 *   in the raw interrupt status; and clear the per-pin interrupt via the
 *   interrupt status clear register. The test also ensures the interrupt is
 *   actually taken (via ISR bookkeeping) and validates that after clearing,
 *   no residual status remains.
 *
 * NOTE: Meta Impacted Registers = NA. Per constraints, no new registers may
 * be introduced. Therefore, the executable logic performs deterministic
 * validation of input prerequisites and terminates with FAIL when required
 * register information is unavailable.
 */

/* Include ONLY test_define.c as per rules */
#include "test_define.c"

/* External functions provided by the test framework/environment */
extern void finish(int status);

/* --------------------------------------------------------------------------
 * Function: test_case
 * Purpose : Convert Meta Test Steps to executable logic with strict ordering.
 *           Since no impacted registers are provided in meta_json, the test
 *           cannot perform register programming or verification without
 *           violating constraints. The test deterministically records this
 *           condition and terminates with FAIL as per acceptance handling.
 * -------------------------------------------------------------------------- */
int test_case(void)
{
    int error_count = 0; // Tracks validation failures

#ifdef DEBUG_DISPLAY
    // Begin test
    printf("[test_gpio_negedge_intr_en] START\n");
    printf("Meta prerequisites: Impacted Registers = NA, Arrays = NA\n");
#endif

    // Step 1: Enable clocks/power for the GPIO IP.
    // Step 2: Program NEG_EDGE_EN for all pads.
    // Step 3: Program INT_EN to enable GPIO interrupts.
    // Step 4: Configure DIR/DATA_OUT to generate transitions.
    // Step 5: Enable CPU/GIC interrupts and ISR installation.
    // Step 6: For each pad: drive 1->0, check INT_RAW_STAT, clear via INT_STAT_CLR, re-check 0.
    // Step 7: Verify ISR bookkeeping/event counts.
    //
    // Per constraints, we must use ONLY registers from Impacted Registers.
    // Since meta_json provides none (NA), executing the above steps would
    // require introducing register definitions, which is disallowed.
#ifdef DEBUG_DISPLAY
    printf("[test_gpio_negedge_intr_en] No impacted registers supplied in meta.\n");
    printf("Cannot perform NEG_EDGE_EN/INT_EN/DIR/DATA_OUT/INT_RAW_STAT/INT_STAT_CLR operations.\n");
    printf("Marking test as FAIL to reflect unmet prerequisites.\n");
#endif
    error_count++;

    // Termination strictly via finish(0)/finish(1)
    if (error_count == 0) {
#ifdef DEBUG_DISPLAY
        printf("[test_gpio_negedge_intr_en] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[test_gpio_negedge_intr_en] FAIL: errors=%d\n", error_count);
#endif
        finish(1);
    }

    // Unreachable in frameworks where finish() does not return
    return 0;
}
