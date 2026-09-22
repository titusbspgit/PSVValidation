// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case Name: mipi_dsi_basic_test
 * Purpose: Supporting definitions for MIPI DSI basic DBI write memory via DMA testcase.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ============================================================ */
/* Headers from Meta TestPlan JSON                              */
/* ============================================================ */
// Headers field was NA in the Meta TestPlan JSON. No additional headers to emit.

/* ============================================================ */
/* Macros from Meta TestPlan JSON                               */
/* ============================================================ */
// Macros field was NA in the Meta TestPlan JSON. No additional macros to emit.

/* ============================================================ */
/* Arrays from Meta TestPlan JSON                               */
/* ============================================================ */
// Arrays field was NA in the Meta TestPlan JSON. No additional arrays to emit.

/* ============================================================ */
/* Impacted Registers (13 registers)                            */
/* Register addresses are not supplied in the Meta TestPlan     */
/* JSON. Each register is preserved as a MANUAL_REVIEW          */
/* placeholder in first-seen order.                             */
/* ============================================================ */

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_DMAC_INTEN was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_DMAC_INTEN
// #define MIZAR_MIPI_DSI_DMAC_INTEN 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE
// #define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_HOST_PHY_IF_CFG was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_HOST_PHY_IF_CFG
// #define MIZAR_MIPI_DSI_HOST_PHY_IF_CFG 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_HOST_PCKHDL_CFG was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_HOST_PCKHDL_CFG
// #define MIZAR_MIPI_DSI_HOST_PCKHDL_CFG 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_HOST_CLKMGR_CFG was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_HOST_CLKMGR_CFG
// #define MIZAR_MIPI_DSI_HOST_CLKMGR_CFG 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL
// #define MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_DMAC_DBGINST0 was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST0
// #define MIZAR_MIPI_DSI_DMAC_DBGINST0 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_DMAC_DBGINST1 was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST1
// #define MIZAR_MIPI_DSI_DMAC_DBGINST1 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_DMAC_DBGCMD was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_DMAC_DBGCMD
// #define MIZAR_MIPI_DSI_DMAC_DBGCMD 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK
// #define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_DMAC_INTMIS was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_DMAC_INTMIS
// #define MIZAR_MIPI_DSI_DMAC_INTMIS 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_DMAC_INTCLR was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_DMAC_INTCLR
// #define MIZAR_MIPI_DSI_DMAC_INTCLR 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

// MANUAL_REVIEW: Register address for MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW was referenced in the Meta TestPlan JSON but no exact address value was supplied. Value must be added manually before use.
#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
// #define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW 0x0UL /* MANUAL_REVIEW: address not supplied */
#endif

/* ============================================================ */
/* Register Field / Bitfield Macros (6 referenced macros)       */
/* Values are not supplied in the Meta TestPlan JSON.            */
/* Each field is preserved as a MANUAL_REVIEW placeholder in    */
/* first-seen order.                                            */
/* ============================================================ */

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR
// #define MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME
// #define MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR
// #define MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for DSI_WRITE_MEMORY_START was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef DSI_WRITE_MEMORY_START
// #define DSI_WRITE_MEMORY_START 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for DMA_SAR was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef DMA_SAR
// #define DMA_SAR 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif

// MANUAL_REVIEW: Macro or register value for DMA_DAR was referenced in the Meta TestPlan JSON but no exact value was supplied. Value must be added manually before use.
#ifndef DMA_DAR
// #define DMA_DAR 0x0UL /* MANUAL_REVIEW: value not supplied */
#endif
