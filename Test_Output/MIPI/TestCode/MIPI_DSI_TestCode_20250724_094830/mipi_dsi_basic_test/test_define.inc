// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case  : mipi_dsi_basic_test
 * Purpose    : Supporting definitions for MIPI DSI DBI command mode transfer
 *              testcase using the DMAC.
 */

#include "test.h"

/* ============================================================================
 * Headers from Meta TestPlan JSON
 * ============================================================================
 *
 * Headers field was NA. No additional headers to include.
 * ============================================================================ */

/* ============================================================================
 * Impacted Register Address Macros
 * ============================================================================
 *
 * MANUAL_REVIEW: Base addresses and offsets for the following registers are not
 * provided in the Meta TestPlan JSON. Replace the placeholder values below with
 * the correct SoC/IP register addresses from the hardware register specification.
 * ============================================================================ */

/* DSI DMAC Registers */
#ifndef MIZAR_MIPI_DSI_DMAC_INTEN
#define MIZAR_MIPI_DSI_DMAC_INTEN              0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTMIS
#define MIZAR_MIPI_DSI_DMAC_INTMIS             0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTCLR
#define MIZAR_MIPI_DSI_DMAC_INTCLR             0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST0
#define MIZAR_MIPI_DSI_DMAC_DBGINST0           0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST1
#define MIZAR_MIPI_DSI_DMAC_DBGINST1           0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGCMD
#define MIZAR_MIPI_DSI_DMAC_DBGCMD             0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

/* DSI Subsystem Registers */
#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE 0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK   0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW    0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL
#define MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL      0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

/* DSI Host Registers */
#ifndef MIZAR_MIPI_DSI_HOST_PHY_IF_CFG
#define MIZAR_MIPI_DSI_HOST_PHY_IF_CFG         0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_HOST_PCKHDL_CFG
#define MIZAR_MIPI_DSI_HOST_PCKHDL_CFG         0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

#ifndef MIZAR_MIPI_DSI_HOST_CLKMGR_CFG
#define MIZAR_MIPI_DSI_HOST_CLKMGR_CFG         0x0UL  /* MANUAL_REVIEW: Set correct address */
#endif

/* ============================================================================
 * Register Configuration Value Macros
 * ============================================================================
 *
 * MANUAL_REVIEW: The following configuration values are not provided in the
 * Meta TestPlan JSON. Replace with correct values from the hardware/test spec.
 * ============================================================================ */

/* Step 2: Subsystem interrupt enable value (GDMA interrupt bit) */
#ifndef MIPI_DSI_SUBSYS_INT_ENABLE_VAL
#define MIPI_DSI_SUBSYS_INT_ENABLE_VAL         0x0U   /* MANUAL_REVIEW: Set correct value */
#endif

/* Step 3: PHY_IF_CFG value (PHY_STOP_WAIT_TIME) */
#ifndef MIPI_DSI_PHY_IF_CFG_VAL
#define MIPI_DSI_PHY_IF_CFG_VAL                0x0U   /* MANUAL_REVIEW: Set correct value */
#endif

/* Step 4: PCKHDL_CFG value */
#ifndef MIPI_DSI_PCKHDL_CFG_VAL
#define MIPI_DSI_PCKHDL_CFG_VAL                0x0U   /* MANUAL_REVIEW: Set correct value */
#endif

/* Step 5: CLKMGR_CFG value */
#ifndef MIPI_DSI_CLKMGR_CFG_VAL
#define MIPI_DSI_CLKMGR_CFG_VAL                0x0U   /* MANUAL_REVIEW: Set correct value */
#endif

/* Step 7: DBGINST0 debug instruction word */
#ifndef MIPI_DSI_DMAC_DBGINST0_VAL
#define MIPI_DSI_DMAC_DBGINST0_VAL             0x0U   /* MANUAL_REVIEW: Set correct value */
#endif

/* Step 8: DBGINST1 debug instruction word */
#ifndef MIPI_DSI_DMAC_DBGINST1_VAL
#define MIPI_DSI_DMAC_DBGINST1_VAL             0x0U   /* MANUAL_REVIEW: Set correct value */
#endif

/* ============================================================================
 * Validation Constants
 * ============================================================================ */

/* Step 10: GDMA interrupt bit mask for polling INTERRUPT_MASK */
// MANUAL_REVIEW: Exact bit position for GDMA interrupt not provided in Meta TestPlan JSON.
#ifndef MIPI_DSI_SUBSYS_GDMA_INT_BIT
#define MIPI_DSI_SUBSYS_GDMA_INT_BIT           0x1U   /* MANUAL_REVIEW: Set correct bit mask */
#endif

/* ============================================================================
 * Polling Timeout
 * ============================================================================
 *
 * MANUAL_REVIEW: Timeout count for polling loops. The FV Template and Meta
 * TestPlan JSON do not provide a timeout value. Adjust as needed for the target.
 * ============================================================================ */

#ifndef MIPI_DSI_POLL_TIMEOUT
#define MIPI_DSI_POLL_TIMEOUT                  10000U /* MANUAL_REVIEW: Set correct timeout */
#endif

/* ============================================================================
 * Macros from Meta TestPlan JSON
 * ============================================================================
 *
 * Macros field was NA. No additional macros to define.
 * ============================================================================ */

/* ============================================================================
 * Arrays from Meta TestPlan JSON
 * ============================================================================
 *
 * Arrays field was NA. No additional arrays to define.
 * ============================================================================ */
