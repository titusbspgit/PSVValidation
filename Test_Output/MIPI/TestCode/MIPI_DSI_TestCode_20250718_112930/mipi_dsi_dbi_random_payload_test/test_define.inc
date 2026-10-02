// Author - AI Force 2.3. 18-Jul-2025 05:59 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case  : mipi_dsi_dbi_random_payload_test
 * Purpose    : Supporting definitions for MIPI DSI DBI Command Mode Random Payload
 *              DMA Write test. Contains headers, macros, arrays, and testcase data.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "test_common.h"
#include "mipi_dsi.h"

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Timeout for polling loops */
#ifndef MIPI_DSI_POLL_TIMEOUT
#define MIPI_DSI_POLL_TIMEOUT 100000U
#endif

/* Global DMA descriptor arrays */
desc_t data_tdbdcb[10];
desc_t data_rebdcb[10];
desc_t cmd_tdbdcb[10];
desc_t cmd_rebdcb[10];

/* Declared arrays (as per test specification, not used in test flow) */
unsigned int wr_cmd_arr[27];
unsigned int WR_CMD_SIZE[27];
