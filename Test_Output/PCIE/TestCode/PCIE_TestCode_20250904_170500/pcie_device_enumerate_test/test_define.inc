// Author - AI Force 2.3. 04-Sep-2025 17:05 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: pcie_device_enumerate_test
 * Purpose: PCIe device enumeration - link training, cache coherency programming,
 *          link-up polling, Vendor ID read, bus master enable, memory base
 *          programming, system register configuration, cache disable, BAR
 *          enumeration and programming on both slave ports, and final sync polling.
 */

/* Headers from Meta TestPlan */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Synchronization register */
#define SYNC_REG_ADDR                0xE6004100U
#define SYNC_EXPECTED_VAL            0x12345678U

/* System-level configuration register addresses */
#define SYSREG_ADDR_0               0xE690000CU
#define SYSREG_ADDR_1               0xE6900010U
#define SYSREG_ADDR_2               0xE6900014U
#define SYSREG_ADDR_3               0xE6900018U
#define SYSREG_ADDR_4               0xE6900030U
#define SYSREG_ADDR_5               0xE6900034U

/* BAR register offsets */
#define BAR_OFFSET_0                0x10U
#define BAR_OFFSET_1                0x14U
#define BAR_OFFSET_2                0x18U
#define BAR_OFFSET_3                0x1CU
#define BAR_OFFSET_4                0x20U
#define BAR_OFFSET_5                0x24U

/* BAR base addresses for programming */
#define BAR_BASE_ADDR_0             0x00000000U
#define BAR_BASE_ADDR_1             0x00000004U
#define BAR_BASE_ADDR_2             0x20000000U
#define BAR_BASE_ADDR_3             0x40000000U
#define BAR_BASE_ADDR_4             0x60000000U
#define BAR_BASE_ADDR_5             0x80000000U

/* Link status polling mask */
#define LINK_STATUS_MASK            0xD1U

/* Macros from Meta TestPlan (conditional compilation defines) */
/* DM0_RC, DM1_RC, DM0_EP, DM1_EP - defined externally via build configuration */
/* DEBUG_DISPLAY - defined externally via build configuration */
