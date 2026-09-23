// Author - AI Force 2.3. 22-Jul-2025 17:50 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: mipi_dsi_dbi_random_payload_test
 * Purpose: Supporting definitions for MIPI DSI DBI DMA transfer with random payload testcase.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Macros field was NA - no testcase-specific macros were provided. */
/* The following register names are used as-is from the Meta TestPlan JSON. */
/* Their address definitions are expected to be provided by the included headers. */

/* MANUAL_REVIEW: MIZAR_MIPI_DSI_HOST_PHY_IF_CFG address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_HOST_PCKHDL_CFG address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_HOST_CLKMGR_CFG address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_DMAC_INTEN address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_DMAC_DBGINST0 address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_DMAC_DBGINST1 address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_DMAC_DBGCMD address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_DMAC_INTMIS address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_DMAC_INTCLR address macro is expected from headers. */

/* MANUAL_REVIEW: MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field mask is expected from headers. */
/* MANUAL_REVIEW: RAM_BASE address macro is expected from headers. */
/* MANUAL_REVIEW: set_data_mask() API is expected from headers. */

/* Number of DBI transfer iterations */
/* MANUAL_REVIEW: Adjust MIPI_DSI_DBI_NUM_ITERATIONS per test requirements. */
#ifndef MIPI_DSI_DBI_NUM_ITERATIONS
#define MIPI_DSI_DBI_NUM_ITERATIONS 10U
#endif

/* Polling timeout for DMAC interrupt detection */
/* MANUAL_REVIEW: Adjust MIPI_DSI_POLL_TIMEOUT value per platform requirements. */
#ifndef MIPI_DSI_POLL_TIMEOUT
#define MIPI_DSI_POLL_TIMEOUT 10000U
#endif

/* Arrays field was NA - no testcase-specific arrays were provided. */
