// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: mipi_dsi_basic_test
 * Purpose: Writes to control registers, conditionally invokes link training,
 *          performs cache programming, polls status registers, programs memory
 *          bases and BAR registers, and validates final completion.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Macros from Meta TestPlan JSON */
#ifndef DM0_RC
/* #define DM0_RC */
#endif

#ifndef DM1_RC
/* #define DM1_RC */
#endif

#ifndef DM0_EP
/* #define DM0_EP */
#endif

#ifndef DM1_EP
/* #define DM1_EP */
#endif

#ifndef DEBUG_DISPLAY
/* #define DEBUG_DISPLAY */
#endif

/*
 * Polling timeout constant.
 * Used for bounded polling loops in the testcase.
 */
#define MIPI_DSI_POLL_TIMEOUT 10000U

/*
 * Impacted Registers (from Meta TestPlan JSON):
 * MIZAR_MIPI_DSI_DMAC_INTEN
 * MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE
 * MIZAR_MIPI_DSI_HOST_PHY_IF_CFG
 * MIZAR_MIPI_DSI_HOST_PCKHDL_CFG
 * MIZAR_MIPI_DSI_HOST_CLKMGR_CFG
 * MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL
 * MIZAR_MIPI_DSI_DMAC_DBGINST0
 * MIZAR_MIPI_DSI_DMAC_DBGINST1
 * MIZAR_MIPI_DSI_DMAC_DBGCMD
 * MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK
 * MIZAR_MIPI_DSI_DMAC_INTMIS
 * MIZAR_MIPI_DSI_DMAC_INTCLR
 * MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
 *
 * Note: Register addresses are expected to be provided by the included headers.
 * No address values were supplied in the Meta TestPlan JSON.
 */

/* Arrays: NA - No arrays provided in Meta TestPlan JSON */
