// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef LPDDR4_MEM_HALF_DATA_BUS_WIDTH_TEST_H
#define LPDDR4_MEM_HALF_DATA_BUS_WIDTH_TEST_H

#include <test.h>

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Initializes the half data bus width test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg);

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Executes the half data bus width read-only test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for status reporting
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput *out);

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Final cleanup for the half data bus width test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg);

#endif /* LPDDR4_MEM_HALF_DATA_BUS_WIDTH_TEST_H */
