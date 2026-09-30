// Author - AI Force 2.3. 01-Jul-2025 12:48 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose: Validates the MIPI CSI2 internal test pattern generator (PPI PG)
 *          functionality including pattern generator configuration, DMA address
 *          setup, clock gating enable, PHY stop state polling, DMA interrupt
 *          polling, main interrupt status read, and interrupt masking.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

// ============================================================================
// Pattern Generator register address definitions
// MANUAL_REVIEW: Replace 0x0UL placeholder addresses with actual register addresses.
// ============================================================================

#ifndef MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES
#define MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES       0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES
#define MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES       0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG
#define MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG              0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE
#define MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE              0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

// ============================================================================
// CSI2 subsystem wrapper register address definitions
// ============================================================================

#ifndef MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL
#define MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL          0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA
#define MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA             0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE
#define MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE               0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

// ============================================================================
// DMA address register definitions
// ============================================================================

#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA
#define MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA        0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION
#define MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION  0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA
#define MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA        0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION
#define MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION  0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

// ============================================================================
// RB_REG_BASE and clock gating offset
// ============================================================================

#ifndef MIZAR_MIPI_CSI2_RB_REG_BASE
#define MIZAR_MIPI_CSI2_RB_REG_BASE                     0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#define MIPI_CSI2_CLK_GATE_OFFSET  0xf4UL

// ============================================================================
// Interrupt status and mask register address definitions
// ============================================================================

#ifndef MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN
#define MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN                0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL          0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL          0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY                0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE               0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL  0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL    0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL    0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL      0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID            0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED      0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

// ============================================================================
// GDMA base address and DMA interrupt offset
// ============================================================================

// MANUAL_REVIEW: Set gdma_reg_base to the actual GDMA register base address.
#ifndef MIPI_CSI2_GDMA_REG_BASE
#define MIPI_CSI2_GDMA_REG_BASE  0x0UL  /* MANUAL_REVIEW: Set actual GDMA base address */
#endif

// MANUAL_REVIEW: Set MIPI_CSI2_DMA_INTMIS_OFFSET to the actual DMA INTMIS register offset.
#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET  0x0UL  /* MANUAL_REVIEW: Set actual offset */
#endif

// ============================================================================
// Pattern generator configuration values (from test procedure)
// ============================================================================

#define MIPI_CSI2_PG_VRES_VAL          0x10UL
#define MIPI_CSI2_PG_HRES_VAL          0x70140UL
#define MIPI_CSI2_PG_CONFIG_VAL        0xe401UL
#define MIPI_CSI2_PG_ENABLE_VAL        0x1UL
#define MIPI_CSI2_PG_DISABLE_VAL       0x0UL

// ============================================================================
// DMA address configuration values (from test procedure)
// ============================================================================

#define MIPI_CSI2_DMA_AR_CH0_DATA_VAL   0x100UL
#define MIPI_CSI2_DMA_AR_CH0_INSTR_VAL  0x0UL
#define MIPI_CSI2_DMA_AW_CH0_DATA_VAL   0x0UL
#define MIPI_CSI2_DMA_AW_CH0_INSTR_VAL  0x0UL

// ============================================================================
// Clock gating enable value (from test procedure)
// ============================================================================

#define MIPI_CSI2_CLK_GATE_ENABLE_VAL  0x1UL

// ============================================================================
// Virtual channel configuration value
// MANUAL_REVIEW: The exact virtual channel ID value is not specified in the
// Meta TestPlan JSON. Set to the actual value required by the SoC.
// ============================================================================

#ifndef MIPI_CSI2_VIRTUAL_CHANNEL_CFG
#define MIPI_CSI2_VIRTUAL_CHANNEL_CFG  0x0UL  /* MANUAL_REVIEW: Set actual write value */
#endif

// ============================================================================
// Interrupt mask value to mask all sources
// MANUAL_REVIEW: Confirm 0xFFFFFFFF is the correct mask-all value for this IP.
// ============================================================================

#ifndef MIPI_CSI2_INT_MASK_ALL
#define MIPI_CSI2_INT_MASK_ALL  0xFFFFFFFFUL
#endif

// ============================================================================
// Polling timeout constant
// MANUAL_REVIEW: Adjust timeout value based on actual hardware timing.
// ============================================================================

#ifndef MIPI_CSI2_POLL_TIMEOUT
#define MIPI_CSI2_POLL_TIMEOUT  1000U
#endif
