// Author - AI Force 2.3. 04-Sep-2025 17:05 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: pcie_mem_wr_rd_test
 * Purpose: PCIe memory write and read-back validation through PCIe slave interfaces.
 *          Link training, cache coherency programming, link-up polling, Vendor ID read,
 *          bus master enable, BAR/memory base programming, cache disable,
 *          memory write-read verification at multiple offsets, sync polling.
 */

/* Headers from Meta TestPlan */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Synchronization register */
#define SYNC_REG_ADDR                0xE6004100U
#define SYNC_HANDSHAKE_VAL           0x87654321U
#define SYNC_COMPLETE_VAL            0x12345678U

/* Link status polling mask */
#define LINK_STATUS_MASK             0xD1U

/* Macros from Meta TestPlan (conditional compilation defines) */
/* DM0_RC, DM1_RC, DM0_EP, DM1_EP - defined externally via build configuration */
/* DM0, DM1 - defined externally via build configuration */
/* DEBUG_DISPLAY - defined externally via build configuration */
