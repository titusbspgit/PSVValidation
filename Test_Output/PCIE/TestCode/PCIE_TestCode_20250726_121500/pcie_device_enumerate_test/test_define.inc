// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: pcie_device_enumerate_test
 * Purpose: Supporting definitions for PCIe device enumeration testcase.
 * Provides headers, macros, register addresses, polling constants,
 * and testcase data.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include "pcie.h"

/* Macros from Meta TestPlan JSON */
/* DM0_RC, DM1_RC, DM0_EP, DM1_EP are compile-time mode selectors. */
/* Define the required mode before compilation, e.g.: -DDM0_RC */
/* DEBUG_DISPLAY is expected to be provided by the framework or build system. */

/* PCIe link status polling constants */
#define PCIE_LINK_STATUS_OFFSET 0xC0U
#define PCIE_LINK_UP_MASK 0xD1U
#define PCIE_LINK_UP_VALUE 0xD1U
#define PCIE_POLL_TIMEOUT 1000000U

/*
 * DV code uses this as a scratch/handshake location.
 * In PSV, initialize only; do not depend on scoreboard response.
 */
#define PCIE_DV_HANDSHAKE_ADDR 0xE6004100U
#define PCIE_HANDSHAKE_EXPECTED 0x12345678U

/* System-level register addresses */
#define PCIE_SYS_REG_0 0xE690000CU
#define PCIE_SYS_REG_1 0xE6900010U
#define PCIE_SYS_REG_2 0xE6900014U
#define PCIE_SYS_REG_3 0xE6900018U
#define PCIE_SYS_REG_4 0xE6900030U
#define PCIE_SYS_REG_5 0xE6900034U

/* Arrays: NA */
