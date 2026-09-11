// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_mem_wr_rd_test
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

#define PCIE_SYNC_REG              0xE6004100UL
#define PCIE_SYNC_READY_VAL        0x11111111U
#define PCIE_SYNC_COMPLETE_VAL     0x12345678U
#define PCIE_POLL_TIMEOUT          1000U
