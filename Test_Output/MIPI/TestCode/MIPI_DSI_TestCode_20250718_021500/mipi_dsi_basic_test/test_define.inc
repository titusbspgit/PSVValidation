// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_basic_test
 * Purpose: Supporting definitions for basic MIPI DSI DBI write memory
 *          operation using DMA with interrupt-driven completion verification.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ========================================================================== */
/* Impacted Register Address Macros                                           */
/* ========================================================================== */
/* MANUAL_REVIEW: Register address values are not provided in the Meta TestPlan */
/* JSON. The macro names are preserved exactly as supplied. Actual address      */
/* values must be filled in from the SoC/IP register map.                       */

#ifndef MIZAR_MIPI_DSI_DMAC_INTEN
#define MIZAR_MIPI_DSI_DMAC_INTEN              0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE  0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_HOST_PHY_IF_CFG
#define MIZAR_MIPI_DSI_HOST_PHY_IF_CFG          0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_HOST_PCKHDL_CFG
#define MIZAR_MIPI_DSI_HOST_PCKHDL_CFG          0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_HOST_CLKMGR_CFG
#define MIZAR_MIPI_DSI_HOST_CLKMGR_CFG          0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL
#define MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL       0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST0
#define MIZAR_MIPI_DSI_DMAC_DBGINST0            0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST1
#define MIZAR_MIPI_DSI_DMAC_DBGINST1            0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGCMD
#define MIZAR_MIPI_DSI_DMAC_DBGCMD              0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK    0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTMIS
#define MIZAR_MIPI_DSI_DMAC_INTMIS              0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTCLR
#define MIZAR_MIPI_DSI_DMAC_INTCLR              0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW     0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

/* ========================================================================== */
/* Configuration Value Macros                                                 */
/* ========================================================================== */
/* MANUAL_REVIEW: Configuration values are not provided in the Meta TestPlan   */
/* JSON. The macro names are derived from the test steps. Actual values must   */
/* be filled in from the SoC/IP specification.                                 */

#ifndef MIPI_DSI_DMAC_INTEN_ENABLE_VAL
#define MIPI_DSI_DMAC_INTEN_ENABLE_VAL                  0x1UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR
#define MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR      0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME
#define MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME     0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME_MASK
#define MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME_MASK 0x0UL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_HOST_PCKHDL_CFG_VAL
#define MIPI_DSI_HOST_PCKHDL_CFG_VAL                    0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_HOST_CLKMGR_CFG_VAL
#define MIPI_DSI_HOST_CLKMGR_CFG_VAL                    0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_SUBSYS_DPI_CONTROL_VAL
#define MIPI_DSI_SUBSYS_DPI_CONTROL_VAL                 0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef DSI_WRITE_MEMORY_START
#define DSI_WRITE_MEMORY_START                          0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef DMA_SAR
#define DMA_SAR                                         0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef DMA_DAR
#define DMA_DAR                                         0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_DMAC_DBGINST1_VAL
#define MIPI_DSI_DMAC_DBGINST1_VAL                      0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_DMAC_DBGCMD_EXECUTE
#define MIPI_DSI_DMAC_DBGCMD_EXECUTE                    0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR
#define MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR        0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_DMAC_INTCLR_VAL
#define MIPI_DSI_DMAC_INTCLR_VAL                        0x1UL  /* MANUAL_REVIEW: Set actual value */
#endif

#ifndef MIPI_DSI_SUBSYS_INTERRUPT_RAW_GDMA_INTR
#define MIPI_DSI_SUBSYS_INTERRUPT_RAW_GDMA_INTR         0x0UL  /* MANUAL_REVIEW: Set actual value */
#endif
