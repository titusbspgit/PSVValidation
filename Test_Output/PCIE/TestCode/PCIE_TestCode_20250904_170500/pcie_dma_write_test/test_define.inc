// Author - AI Force 2.3. 04-Sep-2025 17:05 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: pcie_dma_write_test
 * Purpose: PCIe DMA write and read-back validation across all four DMA channels.
 *          Link training, link-up polling, Vendor ID read, bus master enable,
 *          BAR/memory base programming, source memory preload, GIC initialization,
 *          DMA interrupt mask clear, 4-channel DMA write, 4-channel DMA read-back,
 *          interrupt-driven completion with status check and clear.
 */

/* Headers from Meta TestPlan */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Synchronization register */
#define SYNC_REG_ADDR                0xE6004100U
#define SYNC_HANDSHAKE_VAL           0x12345678U

/* DMA transfer length */
#define DMA_TRANSFER_LEN             0x40U

/* Data patterns for source memory preload */
#define DATA_PATTERN_0               0xC0DEBEEDU
#define DATA_PATTERN_1               0xF00DDEAFU

/* DMA source and destination addresses for DM0 */
#define DMA_SRC_ADDR_DM0             0xE6000000U
#define DMA_DST_ADDR_DM0             0x40000000U

/* DMA source and destination addresses for DM1 */
#define DMA_SRC_ADDR_DM1             0xE6000000U
#define DMA_DST_ADDR_DM1             0x40000000U

/* GIC IRQ numbers */
#define GIC_IRQ_DM0                  0x20U
#define GIC_IRQ_DM1                  0x23U

/* PCIE0 DMA register addresses */
#define PCIE0_DMA_WRITE_INT_MASK_OFF     mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF
#define PCIE0_DMA_READ_INT_MASK_OFF      mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF
#define PCIE0_DMA_WRITE_DOORBELL_OFF     mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF
#define PCIE0_DMA_READ_DOORBELL_OFF      mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF
#define PCIE0_DMA_WRITE_INT_STATUS_OFF   mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF
#define PCIE0_DMA_READ_INT_STATUS_OFF    mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF
#define PCIE0_DMA_WRITE_INT_CLEAR_OFF    mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF
#define PCIE0_DMA_READ_INT_CLEAR_OFF     mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF

/* PCIE1 DMA register addresses */
#define PCIE1_DMA_WRITE_INT_MASK_OFF     mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF
#define PCIE1_DMA_READ_INT_MASK_OFF      mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF
#define PCIE1_DMA_WRITE_DOORBELL_OFF     mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF
#define PCIE1_DMA_READ_DOORBELL_OFF      mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF
#define PCIE1_DMA_WRITE_INT_STATUS_OFF   mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF
#define PCIE1_DMA_READ_INT_STATUS_OFF    mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF
#define PCIE1_DMA_WRITE_INT_CLEAR_OFF    mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF
#define PCIE1_DMA_READ_INT_CLEAR_OFF     mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF

/* Macros from Meta TestPlan (conditional compilation defines) */
/* DM0_RC, DM1_RC, DM0_EP, DM1_EP - defined externally via build configuration */
/* DEBUG_DISPLAY - defined externally via build configuration */
