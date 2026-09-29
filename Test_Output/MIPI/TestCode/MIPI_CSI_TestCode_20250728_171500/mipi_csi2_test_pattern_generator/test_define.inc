// Author - AI Force 2.3. 28-Jul-2025 17:15 IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 *
 * Test Case : mipi_csi2_test_pattern_generator
 * Purpose   : Register addresses, masks, polling constants, and context
 *             structure for MIPI CSI2 test pattern generator configuration
 *             and validation including DMA channel 0 setup.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ---- Headers field from Meta TestPlan JSON was NA ---- */

/* ---- Macros field from Meta TestPlan JSON was NA ---- */

/* ---- Arrays field from Meta TestPlan JSON was NA ---- */

/*
 * Register addresses for MIPI CSI2 test pattern generator test.
 * These are preserved exactly as listed in Impacted Registers.
 * Actual base addresses must be provided by the SoC/IP configuration.
 */

/* ---- Pattern Generator Registers ---- */
#ifndef MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES
#define MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES  0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES
#define MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES  0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG
#define MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG         0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE
#define MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE         0x0UL
#endif

/* ---- Virtual Channel / Control Data Registers ---- */
#ifndef MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL
#define MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL    0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA
#define MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA       0x0UL
#endif

/* ---- PHY Stop State Register ---- */
#ifndef MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE
#define MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE         0x0UL
#endif

/* ---- DMA Channel 0 Registers ---- */
#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA
#define MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA        0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION
#define MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION 0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA
#define MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA        0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION
#define MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION 0x0UL
#endif

/* ---- DMA Trigger Register (MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4) ---- */
#ifndef MIZAR_MIPI_CSI2_RB_REG_DMA_TRIGGER
#define MIZAR_MIPI_CSI2_RB_REG_DMA_TRIGGER        0x0UL
#endif

/* ---- DMA Interrupt Status Base and Offset ---- */
#ifndef MIPI_CSI2_GDMA_REG_BASE
#define MIPI_CSI2_GDMA_REG_BASE                    0x0UL
#endif

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET                0x10UL
#endif

/* ---- Interrupt Registers ---- */
#ifndef MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN
#define MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN           0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL     0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL     0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY           0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE          0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL  0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL    0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL    0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL      0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID             0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED       0x0UL
#endif

/* ---- Polling and mask constants ---- */
#define MIPI_CSI2_PHY_STOPSTATE_POLL_MAX   1000U
#define MIPI_CSI2_PHY_STOPSTATE_MASK       0x1U
#define MIPI_CSI2_DMA_POLL_MAX             5000U
#define MIPI_CSI2_DMA_COMPLETION_MASK      0x1U
#define MIPI_CSI2_INT_MASK_ALL             0xFFFFFFFFU

/* ---- Testcase context structure ---- */
typedef struct {
    unsigned int errors;
} mipi_csi2_tpg_test_ctx_t;
