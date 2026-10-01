// Author - AI Force 2.3. 30-Jun-2025 16:15 IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_dsi_dbi_random_payload_test
 * Purpose: Supporting data definitions for MIPI DSI DBI random payload data
 *   transfer testcase using integrated DMA controller (DMA330).
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"
#include <math.h>

/* Timeout for polling DMA completion */
#define MIPI_DSI_POLL_TIMEOUT 100000U

/* Number of random payload iterations */
#define MIPI_DSI_DBI_RANDOM_ITERATION_COUNT 10U

/*
 * DMA descriptor arrays
 * CH0 (data): data_tdbdcb / data_rebdcb
 * CH1 (command): cmd_tdbdcb / cmd_rebdcb
 */
static unsigned long long data_tdbdcb[10];
static unsigned long long data_rebdcb[10];
static unsigned long long cmd_tdbdcb[10];
static unsigned long long cmd_rebdcb[10];

/*
 * Write command arrays
 * wr_cmd_arr: Write command data array
 * WR_CMD_SIZE: Write command size lookup table
 */
static unsigned int wr_cmd_arr[27];
static unsigned int WR_CMD_SIZE[27];
