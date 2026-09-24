// Author - AI Force 2.3. 26-Jul-2025 18:30 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case  : mipi_dsi_dbi_random_payload_test
 * Purpose    : Supporting definitions for MIPI DSI DBI random payload transfers
 *              using DMAC in a 10-iteration loop.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* =========================================================================
   Impacted Register Address Macros
   =========================================================================
   MANUAL_REVIEW: Exact register base addresses are not provided in the
   Meta TestPlan JSON. The macros below use placeholder address names that
   must be resolved to actual SoC register addresses before compilation.
   ========================================================================= */

/* DSI Host Registers */
#ifndef MIZAR_MIPI_DSI_HOST_PHY_IF_CFG
#define MIZAR_MIPI_DSI_HOST_PHY_IF_CFG          MIZAR_MIPI_DSI_HOST_PHY_IF_CFG_ADDR
#endif

#ifndef MIZAR_MIPI_DSI_HOST_PCKHDL_CFG
#define MIZAR_MIPI_DSI_HOST_PCKHDL_CFG          MIZAR_MIPI_DSI_HOST_PCKHDL_CFG_ADDR
#endif

#ifndef MIZAR_MIPI_DSI_HOST_CLKMGR_CFG
#define MIZAR_MIPI_DSI_HOST_CLKMGR_CFG          MIZAR_MIPI_DSI_HOST_CLKMGR_CFG_ADDR
#endif

/* DSI Subsystem Registers */
#ifndef MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL
#define MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL       MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL_ADDR
#endif

/* DMAC Registers */
#ifndef MIZAR_MIPI_DSI_DMAC_INTEN
#define MIZAR_MIPI_DSI_DMAC_INTEN               MIZAR_MIPI_DSI_DMAC_INTEN_ADDR
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST0
#define MIZAR_MIPI_DSI_DMAC_DBGINST0            MIZAR_MIPI_DSI_DMAC_DBGINST0_ADDR
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST1
#define MIZAR_MIPI_DSI_DMAC_DBGINST1            MIZAR_MIPI_DSI_DMAC_DBGINST1_ADDR
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGCMD
#define MIZAR_MIPI_DSI_DMAC_DBGCMD              MIZAR_MIPI_DSI_DMAC_DBGCMD_ADDR
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTMIS
#define MIZAR_MIPI_DSI_DMAC_INTMIS              MIZAR_MIPI_DSI_DMAC_INTMIS_ADDR
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTCLR
#define MIZAR_MIPI_DSI_DMAC_INTCLR              MIZAR_MIPI_DSI_DMAC_INTCLR_ADDR
#endif

/* =========================================================================
   Testcase Constants and Configuration Values
   =========================================================================
   MANUAL_REVIEW: The following configuration values are not explicitly
   provided in the Meta TestPlan JSON. They are placeholders that must be
   set to the correct SoC/IP-specific values before compilation.
   ========================================================================= */

/* Number of random payload transfer iterations */
#define MIPI_DSI_NUM_ITERATIONS                 10U

/* Expected INTMIS value indicating both DMA channels completed (0x3) */
#define MIPI_DSI_EXPECTED_INTMIS                0x3U

/* PHY_STOP_WAIT_TIME field mask and value for PHY_IF_CFG read-modify-write */
// MANUAL_REVIEW: PHY_STOP_WAIT_TIME mask and value not explicitly provided in Meta TestPlan JSON.
#ifndef MIPI_DSI_PHY_STOP_WAIT_TIME_MASK
#define MIPI_DSI_PHY_STOP_WAIT_TIME_MASK        0xFF00U
#endif

#ifndef MIPI_DSI_PHY_STOP_WAIT_TIME_VAL
#define MIPI_DSI_PHY_STOP_WAIT_TIME_VAL         0x0800U
#endif

/* PCKHDL_CFG write value */
// MANUAL_REVIEW: Exact write value for PCKHDL_CFG not provided in Meta TestPlan JSON.
#ifndef MIPI_DSI_PCKHDL_CFG_VAL
#define MIPI_DSI_PCKHDL_CFG_VAL                 0x1U
#endif

/* CLKMGR_CFG write value */
// MANUAL_REVIEW: Exact write value for CLKMGR_CFG not provided in Meta TestPlan JSON.
#ifndef MIPI_DSI_CLKMGR_CFG_VAL
#define MIPI_DSI_CLKMGR_CFG_VAL                 0x0U
#endif

/* DMAC DBGINST0 first debug instruction word */
// MANUAL_REVIEW: Exact DBGINST0 value (DMA channel thread and instruction encoding)
// not provided in Meta TestPlan JSON. Set to correct value for the target SoC.
#ifndef MIPI_DSI_DMAC_DBGINST0_VAL
#define MIPI_DSI_DMAC_DBGINST0_VAL              0x00A00000U
#endif

/* DMAC DBGINST1 second debug instruction word */
// MANUAL_REVIEW: Exact DBGINST1 value (descriptor address) not provided in Meta TestPlan JSON.
// Set to correct value for the target SoC.
#ifndef MIPI_DSI_DMAC_DBGINST1_VAL
#define MIPI_DSI_DMAC_DBGINST1_VAL              0x00000000U
#endif

/* Polling timeout count for INTMIS polling */
#ifndef MIPI_DSI_POLL_TIMEOUT
#define MIPI_DSI_POLL_TIMEOUT                   10000U
#endif

/* =========================================================================
   Testcase Context Structure
   ========================================================================= */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
} mipi_dsi_dbi_random_payload_test_ctx_t;

/* Meta TestPlan Headers: NA - no additional headers required */
/* Meta TestPlan Macros: NA - no additional macros provided */
/* Meta TestPlan Arrays: NA - no additional arrays provided */
