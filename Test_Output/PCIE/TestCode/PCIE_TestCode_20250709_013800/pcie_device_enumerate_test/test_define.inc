// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_device_enumerate_test
 * Purpose: Supporting definitions for PCIe device enumeration testcase.
 *   Includes headers, macros, register addresses, BAR offsets, and BAR values.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Compile-time mode defines from Meta TestPlan */
// DM0_RC, DM1_RC, DM0_EP, DM1_EP are expected to be defined externally
// via compiler flags (e.g., -DDM0_RC). They are not redefined here.

/* DEBUG_DISPLAY is expected to be provided by the framework or compiler flags. */

/* Synchronization register address */
#define PCIE_SYNC_REG           0xE6004100UL

/* Expected synchronization value for final poll */
#define PCIE_SYNC_EXPECTED      0x12345678UL

/* Polling timeout count */
#define PCIE_POLL_TIMEOUT       1000U

/* Number of BAR registers */
#define PCIE_BAR_COUNT          6U

/* BAR register offsets */
static const unsigned int pcie_bar_offsets[PCIE_BAR_COUNT] = {
    0x10, 0x14, 0x18, 0x1c, 0x20, 0x24
};

/* BAR assignment address values */
static const unsigned int pcie_bar_values[PCIE_BAR_COUNT] = {
    0x00000000, 0x00000004, 0x20000000, 0x40000000, 0x60000000, 0x80000000
};
