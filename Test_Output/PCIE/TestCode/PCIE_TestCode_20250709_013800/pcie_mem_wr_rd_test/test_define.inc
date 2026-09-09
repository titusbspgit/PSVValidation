// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_mem_wr_rd_test
 * Purpose: Supporting definitions for PCIe memory write and read testcase.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Synchronization register address */
#define PCIE_SYNC_REG           0xE6004100UL

/* Expected synchronization value for final poll */
#define PCIE_SYNC_EXPECTED      0x12345678UL

/* Readiness signal value */
#define PCIE_SYNC_READY         0x11111111UL

/* Polling timeout count */
#define PCIE_POLL_TIMEOUT       1000U
