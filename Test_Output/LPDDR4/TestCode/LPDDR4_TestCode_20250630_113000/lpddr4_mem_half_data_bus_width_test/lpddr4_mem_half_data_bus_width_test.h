// Author - AI Force 2.3. 2025-06-30 11:30:00 IST
// (EMBENGG-SYSAPPS)

#ifndef LPDDR4_MEM_HALF_DATA_BUS_WIDTH_TEST_H
#define LPDDR4_MEM_HALF_DATA_BUS_WIDTH_TEST_H

#include <test.h>

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Initialise the LPDDR4 half bus width testcase context.
 * Parameters:
 *   cfg - framework test configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg);

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Execute the half bus width read-only memory test.
 * Parameters:
 *   cfg - framework test configuration
 *   out - framework test output
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput *out);

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Cleanup after the half bus width test.
 * Parameters:
 *   cfg - framework test configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg);

#endif /* LPDDR4_MEM_HALF_DATA_BUS_WIDTH_TEST_H */
