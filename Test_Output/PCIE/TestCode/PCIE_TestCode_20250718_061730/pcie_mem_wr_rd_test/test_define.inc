// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: pcie_mem_wr_rd_test
 * Purpose:   PCIe memory write and read-back verification across the PCIe slave
 *            memory space for dual-mode controllers in x4 configuration with
 *            cache coherency programming and disable phases.
 */

/* Headers */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/*
 * Macros: Compile-time configuration defines.
 * Define one of: DM0_RC, DM1_RC, DM0_EP, DM1_EP to select controller mode.
 * Define DM0 or DM1 for port selection.
 * Define DEBUG_DISPLAY to enable verbose debug logging.
 *
 * These are expected to be defined externally (e.g., via compiler flags).
 * Uncomment below if needed for standalone compilation:
 *
 * #define DM0_RC
 * #define DM1_RC
 * #define DM0_EP
 * #define DM1_EP
 * #define DM0
 * #define DM1
 * #define DEBUG_DISPLAY
 */

/* Arrays: NA - No arrays required for this testcase */
