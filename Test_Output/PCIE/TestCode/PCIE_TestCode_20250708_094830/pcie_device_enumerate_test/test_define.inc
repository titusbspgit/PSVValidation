// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_device_enumerate_test
 * Purpose: PCIe device enumeration - link training, cache programming,
 *          link status polling, vendor ID read, command register enable,
 *          memory base programming, system register writes, cache disable,
 *          BAR sizing and assignment, and final synchronization polling.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Macros from Meta TestPlan JSON */
#ifndef DM0_RC
// MANUAL_REVIEW: DM0_RC is a compile-time define; define it in build configuration if needed.
#endif

#ifndef DM1_RC
// MANUAL_REVIEW: DM1_RC is a compile-time define; define it in build configuration if needed.
#endif

#ifndef DM0_EP
// MANUAL_REVIEW: DM0_EP is a compile-time define; define it in build configuration if needed.
#endif

#ifndef DM1_EP
// MANUAL_REVIEW: DM1_EP is a compile-time define; define it in build configuration if needed.
#endif

#ifndef DEBUG_DISPLAY
// MANUAL_REVIEW: DEBUG_DISPLAY is a compile-time define; define it in build configuration if needed.
#endif

/* Polling timeout constant */
#define PCIE_POLL_TIMEOUT 10000U

/* Synchronization register address */
#define PCIE_SYNC_REG_ADDR 0xE6004100UL

/* Synchronization expected value */
#define PCIE_SYNC_EXPECTED 0x12345678UL

/* Link status mask */
#define PCIE_LINK_STATUS_MASK 0xD1U

/* Link status expected value */
#define PCIE_LINK_STATUS_EXPECTED 0xD1U

/* BAR register offsets */
#define PCIE_BAR0_OFF 0x10U
#define PCIE_BAR1_OFF 0x14U
#define PCIE_BAR2_OFF 0x18U
#define PCIE_BAR3_OFF 0x1cU
#define PCIE_BAR4_OFF 0x20U
#define PCIE_BAR5_OFF 0x24U

/* BAR sizing value */
#define PCIE_BAR_SIZE_VAL 0xFFFFFFFFUL

/* BAR assignment values */
#define PCIE_BAR0_ADDR 0x0UL
#define PCIE_BAR1_ADDR 0x4UL
#define PCIE_BAR2_ADDR 0x20000000UL
#define PCIE_BAR3_ADDR 0x40000000UL
#define PCIE_BAR4_ADDR 0x60000000UL
#define PCIE_BAR5_ADDR 0x80000000UL

/* System-level register addresses */
#define PCIE_SYS_REG0 0xE690000CUL
#define PCIE_SYS_REG1 0xE6900010UL
#define PCIE_SYS_REG2 0xE6900014UL
#define PCIE_SYS_REG3 0xE6900018UL
#define PCIE_SYS_REG4 0xE6900030UL
#define PCIE_SYS_REG5 0xE6900034UL
