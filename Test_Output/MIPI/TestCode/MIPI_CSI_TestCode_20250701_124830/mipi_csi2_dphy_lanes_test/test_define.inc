// Author - AI Force 2.3. 01-Jul-2025 07:18 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Purpose: Validates MIPI CSI2 D-PHY lane configuration by iterating through
 *          different lane counts (4 down to 1), configuring virtual channel,
 *          control data, active lanes, polling PHY stop state, performing
 *          system-level operations, reading interrupt status, and masking all
 *          interrupt sources.
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

// MANUAL_REVIEW: Register addresses for the following macros are not provided
// in the Meta TestPlan JSON. Actual hardware addresses must be filled in by
// the integration engineer from the SoC register map or IP header files.

// Impacted register address definitions
// MANUAL_REVIEW: Replace 0x0UL placeholder addresses with actual register addresses.
#ifndef MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL
#define MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL       0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA
#define MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA           0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_N_LANES
#define MIZAR_MIPI_CSI2_HOST_N_LANES                  0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE
#define MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE             0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN
#define MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN               0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL         0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL         0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY               0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE              0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL 0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL   0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL   0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL     0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID           0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED     0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

// Hardcoded system-level addresses from the test procedure
#define MIPI_CSI2_SYS_CFG_ADDR        0xa0243ffcUL
#define MIPI_CSI2_SYS_STATUS_ADDR     0xE6001000UL

// MANUAL_REVIEW: The write value for 0xa0243ffc is not specified in the Meta TestPlan JSON.
// Set the actual configuration value required by the SoC/DMA subsystem.
#ifndef MIPI_CSI2_SYS_CFG_VAL
#define MIPI_CSI2_SYS_CFG_VAL         0x0UL  /* MANUAL_REVIEW: Set actual write value */
#endif

// MANUAL_REVIEW: The write values for virtual channel and control data registers
// are not specified in the Meta TestPlan JSON. Set actual configuration values.
#ifndef MIPI_CSI2_VIRTUAL_CHANNEL_CFG
#define MIPI_CSI2_VIRTUAL_CHANNEL_CFG  0x0UL  /* MANUAL_REVIEW: Set actual write value */
#endif

#ifndef MIPI_CSI2_CONTROL_DATA_CFG
#define MIPI_CSI2_CONTROL_DATA_CFG     0x0UL  /* MANUAL_REVIEW: Set actual write value */
#endif

// Interrupt mask value to mask all sources
// MANUAL_REVIEW: Confirm 0xFFFFFFFF is the correct mask-all value for this IP.
#ifndef MIPI_CSI2_INT_MASK_ALL
#define MIPI_CSI2_INT_MASK_ALL         0xFFFFFFFFUL
#endif

// Lane iteration constants
#define MIPI_CSI2_MAX_LANES  4U
#define MIPI_CSI2_MIN_LANES  1U

// Polling timeout constant
// MANUAL_REVIEW: Adjust timeout value based on actual hardware timing requirements.
#ifndef MIPI_CSI2_POLL_TIMEOUT
#define MIPI_CSI2_POLL_TIMEOUT  1000U
#endif
