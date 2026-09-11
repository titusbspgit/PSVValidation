// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_dma_write_test
 * Purpose: PCIe DMA write and read test supporting data, macros, and headers.
 *   Validates DMA operations across channels 0-3 for DM0 and DM1 controllers.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Compile-time mode defines (provided externally via build system) */
#ifndef DM0_RC
/* DM0_RC: compile-time define for DM0 Root Complex mode */
/* #define DM0_RC */
#endif

#ifndef DM1_RC
/* DM1_RC: compile-time define for DM1 Root Complex mode */
/* #define DM1_RC */
#endif

#ifndef DM0_EP
/* DM0_EP: compile-time define for DM0 Endpoint mode */
/* #define DM0_EP */
#endif

#ifndef DM1_EP
/* DM1_EP: compile-time define for DM1 Endpoint mode */
/* #define DM1_EP */
#endif

#ifndef DEBUG_DISPLAY
/* DEBUG_DISPLAY: compile-time define for debug display output */
/* #define DEBUG_DISPLAY */
#endif

/* Synchronization register address */
#define PCIE_SYNC_REG              0xE6004100UL

/* Synchronization completion value */
#define PCIE_SYNC_COMPLETE_VAL     0x12345678U

/* Polling timeout count for bounded polling loops */
#define PCIE_POLL_TIMEOUT          1000U

/* DMA interrupt status mask */
#define DMA_INT_STATUS_MASK        0x0000000FU

/* DMA transfer length */
#define DMA_TRANSFER_LEN           0x40U

/* DMA source address */
#define DMA_SRC_ADDR0              0xE6000000UL

/* DMA preload patterns */
#define DMA_PATTERN1               0xC0DEBEEDU
#define DMA_PATTERN2               0xF00DDEAFU

/* DMA preload word count per pattern */
#define DMA_PRELOAD_WORD_COUNT     128U

/* DMA preload offset in words for second pattern (128 words = 0x200 bytes, but source says +400 which is 0x190 hex = 400 decimal words) */
// MANUAL_REVIEW: Source procedure says src_addr0+400. Interpreting as word offset 400 (decimal).
#define DMA_PRELOAD_OFFSET_WORDS   400U

/* GIC IRQ numbers for DM0 and DM1 */
#define PCIE_DM0_GIC_IRQ           0x20U
#define PCIE_DM1_GIC_IRQ           0x23U

/* DM0_RC DMA write/read addresses (0xA7xxxxxx range from procedure step 10) */
// MANUAL_REVIEW: Exact wr_addr0-3, rd_addr0-3 values for DM0_RC were described as 0xA7xxxxxx but exact values not fully specified.
#define DM0_WR_ADDR0               0xA7000000UL
#define DM0_WR_ADDR1               0xA7100000UL
#define DM0_WR_ADDR2               0xA7200000UL
#define DM0_WR_ADDR3               0xA7300000UL
#define DM0_RD_ADDR0               0xA7000000UL
#define DM0_RD_ADDR1               0xA7100000UL
#define DM0_RD_ADDR2               0xA7200000UL
#define DM0_RD_ADDR3               0xA7300000UL

/* DM1_RC DMA write/read addresses (0xC7xxxxxx range from procedure step 10) */
// MANUAL_REVIEW: Exact wr_addr0-3, rd_addr0-3 values for DM1_RC were described as 0xC7xxxxxx but exact values not fully specified.
#define DM1_WR_ADDR0               0xC7000000UL
#define DM1_WR_ADDR1               0xC7100000UL
#define DM1_WR_ADDR2               0xC7200000UL
#define DM1_WR_ADDR3               0xC7300000UL
#define DM1_RD_ADDR0               0xC7000000UL
#define DM1_RD_ADDR1               0xC7100000UL
#define DM1_RD_ADDR2               0xC7200000UL
#define DM1_RD_ADDR3               0xC7300000UL

/* DMA destination addresses */
// MANUAL_REVIEW: Exact dst_addr0-3 values were referenced in procedure but not fully specified.
#define DMA_DST_ADDR0              0xE6010000UL
#define DMA_DST_ADDR1              0xE6020000UL
#define DMA_DST_ADDR2              0xE6030000UL
#define DMA_DST_ADDR3              0xE6040000UL

/* Arrays field: NA - no arrays provided
