// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_dma_write_test
 * Purpose: Validates PCIe DMA write and read operations across all four
 *          DMA channels (0-3) for both dual-mode controllers (DM0 and DM1).
 *          Includes link training, link status polling, BAR/memory base
 *          programming, synchronization polling, source memory preload,
 *          GIC/interrupt setup, DMA channel programming with doorbell
 *          trigger and interrupt-driven completion.
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

#ifndef DEBUG_DISPLAY
// MANUAL_REVIEW: DEBUG_DISPLAY is a compile-time define; define it in build configuration if needed.
#endif

/* Polling timeout constant */
#define PCIE_POLL_TIMEOUT 10000U

/* Synchronization register address */
#define PCIE_SYNC_REG_ADDR 0xE6004100UL

/* Synchronization expected value */
#define PCIE_SYNC_EXPECTED 0x12345678UL

/* Link status mask and expected value */
#define PCIE_LINK_STATUS_MASK 0xD1U
#define PCIE_LINK_STATUS_EXPECTED 0xD1U

/* DMA interrupt status mask */
#define DMA_INT_STATUS_MASK 0x0000000FUL

/* GIC IRQ numbers */
#define PCIE_DM0_GIC_IRQ 0x20U
#define PCIE_DM1_GIC_IRQ 0x23U

/* DMA transfer length */
#define PCIE_DMA_XFER_LEN 0x40U

/* Source address */
#define PCIE_SRC_ADDR0 0xE6000000UL

/* DMA preload configuration */
#define PCIE_DMA_PRELOAD_WORDS 128U
#define PCIE_DMA_PRELOAD_OFFSET_WORDS 100U
#define PCIE_DMA_PATTERN0 0xC0DEBEEDUL
#define PCIE_DMA_PATTERN1 0xF00DDEAFUL

/* DM0 DMA write/read addresses */
// MANUAL_REVIEW: DM0 wr_addr0-wr_addr3 and rd_addr0-rd_addr3 are based on 0xA7xxxxxx range.
// Exact values depend on BAR programming. Provide correct values in build configuration.
#define PCIE_DM0_WR_ADDR0 0xA7000000UL
#define PCIE_DM0_WR_ADDR1 0xA7100000UL
#define PCIE_DM0_WR_ADDR2 0xA7200000UL
#define PCIE_DM0_WR_ADDR3 0xA7300000UL
#define PCIE_DM0_RD_ADDR0 0xA7000000UL
#define PCIE_DM0_RD_ADDR1 0xA7100000UL
#define PCIE_DM0_RD_ADDR2 0xA7200000UL
#define PCIE_DM0_RD_ADDR3 0xA7300000UL

/* DM1 DMA write/read addresses */
// MANUAL_REVIEW: DM1 wr_addr0-wr_addr3 and rd_addr0-rd_addr3 are based on 0xC7xxxxxx range.
// Exact values depend on BAR programming. Provide correct values in build configuration.
#define PCIE_DM1_WR_ADDR0 0xC7000000UL
#define PCIE_DM1_WR_ADDR1 0xC7100000UL
#define PCIE_DM1_WR_ADDR2 0xC7200000UL
#define PCIE_DM1_WR_ADDR3 0xC7300000UL
#define PCIE_DM1_RD_ADDR0 0xC7000000UL
#define PCIE_DM1_RD_ADDR1 0xC7100000UL
#define PCIE_DM1_RD_ADDR2 0xC7200000UL
#define PCIE_DM1_RD_ADDR3 0xC7300000UL

/* DMA destination addresses */
// MANUAL_REVIEW: dst_addr0-dst_addr3 exact values depend on system memory map.
// Provide correct values in build configuration.
#define PCIE_DST_ADDR0 0xE6010000UL
#define PCIE_DST_ADDR1 0xE6020000UL
#define PCIE_DST_ADDR2 0xE6030000UL
#define PCIE_DST_ADDR3 0xE6040000UL
