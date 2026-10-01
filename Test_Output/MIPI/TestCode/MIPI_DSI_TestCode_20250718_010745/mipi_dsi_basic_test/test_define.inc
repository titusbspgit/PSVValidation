// Author - AI Force 2.3. 18-Jul-2025 01:07 IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_dsi_basic_test
 * Purpose: Supporting data definitions for basic MIPI DSI DBI write
 *          operation using DMA with interrupt-driven completion.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros field was NA in Meta TestPlan JSON - no testcase-specific macros provided */

/*
 * Timeout for polling DMA completion.
 * Used in the run function to prevent unbounded polling.
 */
#define MIPI_DSI_POLL_TIMEOUT 100000U

/* DMA descriptor arrays from Meta TestPlan JSON */
desc_t data_tdbdcb[2];
desc_t data_rebdcb[2];
desc_t cmd_tdbdcb[2];
desc_t cmd_rebdcb[2];
