// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: mipi_dsi_dbi_random_payload_test
 * Purpose: Writes 0x0 to control register at 0xE6004100, conditionally invokes
 *          link training, performs cache programming, polls status registers,
 *          programs memory bases and BAR registers, and polls 0xE6004100 for
 *          final completion value 0x12345678.
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
#define MIPI_DSI_DBI_POLL_TIMEOUT 10000U

/*
 * Hardcoded addresses from Meta TestPlan JSON.
 */
#define MIPI_DSI_DBI_CTRL_REG_ADDR       0xE6004100UL
#define MIPI_DSI_DBI_HARDCODED_ADDR_0    0xE690000CUL
#define MIPI_DSI_DBI_HARDCODED_ADDR_1    0xE6900010UL
#define MIPI_DSI_DBI_HARDCODED_ADDR_2    0xE6900014UL
#define MIPI_DSI_DBI_HARDCODED_ADDR_3    0xE6900018UL
#define MIPI_DSI_DBI_HARDCODED_ADDR_4    0xE6900030UL
#define MIPI_DSI_DBI_HARDCODED_ADDR_5    0xE6900034UL

/* Expected final poll value */
#define MIPI_DSI_DBI_EXPECTED_FINAL_VAL  0x12345678U

/*
 * Impacted Registers (from Meta TestPlan JSON):
 * MIZAR_MIPI_DSI_HOST_PHY_IF_CFG
 * MIZAR_MIPI_DSI_HOST_PCKHDL_CFG
 * MIZAR_MIPI_DSI_HOST_CLKMGR_CFG
 * MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL
 * MIZAR_MIPI_DSI_DMAC_INTEN
 * MIZAR_MIPI_DSI_DMAC_DBGINST0
 * MIZAR_MIPI_DSI_DMAC_DBGINST1
 * MIZAR_MIPI_DSI_DMAC_DBGCMD
 * MIZAR_MIPI_DSI_DMAC_INTMIS
 * MIZAR_MIPI_DSI_DMAC_INTCLR
 *
 * Note: Register addresses are expected to be provided by the included headers.
 */

/* Arrays: NA - No arrays provided in Meta TestPlan JSON */
