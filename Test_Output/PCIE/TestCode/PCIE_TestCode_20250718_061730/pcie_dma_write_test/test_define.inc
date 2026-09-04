// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: pcie_dma_write_test
 * Purpose:   PCIe DMA write and read-back operations across all four DMA channels
 *            (Channel 0-3) for dual-mode controllers with interrupt-driven completion.
 */

/* Headers */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/*
 * Macros: Compile-time configuration defines.
 * Define one of: DM0_RC, DM1_RC, DM0_EP, DM1_EP to select controller mode.
 * Define DEBUG_DISPLAY to enable verbose debug logging.
 *
 * These are expected to be defined externally (e.g., via compiler flags).
 * Uncomment below if needed for standalone compilation:
 *
 * #define DM0_RC
 * #define DM1_RC
 * #define DM0_EP
 * #define DM1_EP
 * #define DEBUG_DISPLAY
 */

/* Arrays: NA - No arrays required for this testcase */
