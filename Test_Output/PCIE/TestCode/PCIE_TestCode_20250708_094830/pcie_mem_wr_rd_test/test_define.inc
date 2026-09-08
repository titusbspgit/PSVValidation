// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_mem_wr_rd_test
 * Purpose: Validates PCIe memory write and read operations through the
 *          PCIe slave interfaces for DM0_RC, DM1_RC, DM0_EP, and DM1_EP
 *          modes. Includes link training, cache programming, link status
 *          polling, BAR/memory base programming, cache disable, memory
 *          write/read verification, and final synchronization polling.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Compile-time mode defines */
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

#ifndef DM0
// MANUAL_REVIEW: DM0 is a compile-time define; define it in build configuration if needed.
#endif

#ifndef DM1
// MANUAL_REVIEW: DM1 is a compile-time define; define it in build configuration if needed.
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

/* Synchronization readiness value */
#define PCIE_SYNC_READY 0x11111111UL

/* Link status mask and expected value */
#define PCIE_LINK_STATUS_MASK 0xD1U
#define PCIE_LINK_STATUS_EXPECTED 0xD1U

/* DM0_RC memory write/read addresses */
#define PCIE_DM0_RC_ADDR0 0x01040000UL
#define PCIE_DM0_RC_ADDR1 0x01000020UL
#define PCIE_DM0_RC_ADDR2 0x01004000UL

/* DM0_RC memory write/read data patterns */
#define PCIE_DM0_RC_DATA0 0xa5a5a5a5UL
#define PCIE_DM0_RC_DATA1 0xa6a6a6a6UL
#define PCIE_DM0_RC_DATA2 0xa7a7a7a7UL

/* DM1_RC memory write/read addresses */
#define PCIE_DM1_RC_ADDR0 0x01040000UL
#define PCIE_DM1_RC_ADDR1 0x01000020UL
#define PCIE_DM1_RC_ADDR2 0x01004000UL

/* DM1_RC memory write/read data patterns */
#define PCIE_DM1_RC_DATA0 0xb5b5b5b5UL
#define PCIE_DM1_RC_DATA1 0xb5b5b6b6UL
#define PCIE_DM1_RC_DATA2 0xb7b7b5b5UL

/* EP memory write/read addresses */
#define PCIE_EP_ADDR0 0x10100UL
#define PCIE_EP_ADDR1 0x20100UL
#define PCIE_EP_ADDR2 0x1B100UL
#define PCIE_EP_ADDR3 0x2B100UL
#define PCIE_EP_ADDR4 0x30100UL

/* EP memory write/read data pattern */
#define PCIE_EP_DATA 0x5a5a5a5aUL
