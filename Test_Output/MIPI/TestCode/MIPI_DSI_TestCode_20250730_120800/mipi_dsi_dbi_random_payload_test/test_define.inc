// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_dsi_dbi_random_payload_test
 * Purpose: Supporting definitions for MIPI DSI DBI data transfers with
 *          randomized pixel payload sizes using the integrated DMA controller
 *          (DMA330) with polling-based completion across 10 iterations.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"
#include <math.h>

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ---------------------------------------------------------------------------
 * Iteration count and timeout constant for DMA completion polling
 * --------------------------------------------------------------------------- */
#define MIPI_DSI_DBI_RANDOM_PAYLOAD_TEST_ITERATIONS 10U
#define MIPI_DSI_DBI_RANDOM_PAYLOAD_TEST_TIMEOUT    1000000U

/* ---------------------------------------------------------------------------
 * DMA Descriptor Arrays
 * --------------------------------------------------------------------------- */
static unsigned long long data_tdbdcb[10];
static unsigned long long data_rebdcb[10];
static unsigned long long cmd_tdbdcb[10];
static unsigned long long cmd_rebdcb[10];

/* ---------------------------------------------------------------------------
 * Write command arrays
 * --------------------------------------------------------------------------- */
static unsigned int wr_cmd_arr[27];
static unsigned int WR_CMD_SIZE[27];
