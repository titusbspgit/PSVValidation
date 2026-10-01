// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef LPDDR4_INLINE_ECC_1BIT_DATA_MODIFY_TEST_H
#define LPDDR4_INLINE_ECC_1BIT_DATA_MODIFY_TEST_H

#include <test.h>

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_init
 * Description: Initializes the ECC 1-bit corrected error test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_init(const TestsItem *cfg);

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_run
 * Description: Executes the ECC 1-bit corrected error test sequence.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for status reporting
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_run(const TestsItem *cfg, TestOutput *out);

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_teardown
 * Description: Final cleanup for the ECC 1-bit corrected error test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_teardown(const TestsItem *cfg);

#endif /* LPDDR4_INLINE_ECC_1BIT_DATA_MODIFY_TEST_H */
