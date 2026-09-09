// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_dma_write_test
 * Purpose: Supporting definitions for PCIe DMA write and read testcase.
 *   Includes headers, macros, register addresses, DMA parameters, and source
 *   memory patterns.
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
#define PCIE_SYNC_REG               0xE6004100UL

/* Expected synchronization value for sync poll */
#define PCIE_SYNC_EXPECTED          0x12345678UL

/* Polling timeout count */
#define PCIE_DMA_POLL_TIMEOUT       1000U

/* DMA transfer length */
#define PCIE_DMA_XFER_LEN           0x40U

/* Source memory base address */
#define PCIE_DMA_SRC_ADDR0          0xE6000000UL

/* Number of preload words per pattern block */
#define PCIE_DMA_PRELOAD_WORDS      128U

/* Word offset for second pattern block (400 hex / 4 = 256 words) */
#define PCIE_DMA_PRELOAD_OFFSET_WORDS  256U

/* Source memory test patterns */
#define PCIE_DMA_PATTERN1           0xC0DEBEEDUL
#define PCIE_DMA_PATTERN2           0xF00DDEAFUL

/* DMA destination addresses */
#define PCIE_DMA_DST_ADDR0          0xE6010000UL
#define PCIE_DMA_DST_ADDR1          0xE6020000UL
#define PCIE_DMA_DST_ADDR2          0xE6030000UL
#define PCIE_DMA_DST_ADDR3          0xE6040000UL
