// Author - AI Force 2.3. 18-Jul-2025 01:07 IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_dsi_dbi_random_payload_test
 * Purpose: Supporting data definitions for MIPI DSI DBI random payload
 *          stress test using DMA with 10 iterations of random-sized transfers.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"
#include <math.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros field was NA in Meta TestPlan JSON - no testcase-specific macros provided */

/*
 * Timeout for polling DMA completion per iteration.
 * Used in the run function to prevent unbounded polling.
 */
#define MIPI_DSI_POLL_TIMEOUT 100000U

/* Number of stress test iterations */
#define MIPI_DSI_DBI_ITERATION_COUNT 10U

/* DMA descriptor arrays from Meta TestPlan JSON */
desc_t data_tdbdcb[10];
desc_t data_rebdcb[10];
desc_t cmd_tdbdcb[10];
desc_t cmd_rebdcb[10];

/* Write command arrays from Meta TestPlan JSON */
unsigned int wr_cmd_arr[27];
unsigned int WR_CMD_SIZE[27];
