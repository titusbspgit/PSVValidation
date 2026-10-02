// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case : mipi_dsi_dbi_random_payload_test
 * Purpose   : Supporting data, headers, macros, and arrays for
 *             MIPI DSI DBI random payload data transfer with 2-channel DMA.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"
#include <math.h>

/* Macros field was NA - no testcase-specific macros provided */

/* DMA descriptor control block arrays */
dma_desc_t data_tdbdcb[10];
dma_desc_t data_rebdcb[10];
dma_desc_t cmd_tdbdcb[10];
dma_desc_t cmd_rebdcb[10];

/* Write command arrays */
unsigned int wr_cmd_arr[27];
unsigned int WR_CMD_SIZE[27];
