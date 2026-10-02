// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case : mipi_dsi_basic_test
 * Purpose   : Supporting data, headers, macros, and arrays for
 *             basic MIPI DSI DBI data transfer with 2-channel DMA
 *             and interrupt-driven completion.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"

/* Macros field was NA - no testcase-specific macros provided */

/* Global volatile variables for interrupt handshake */
volatile unsigned int int_pend  = 1U;
volatile unsigned int int_pend1 = 0x3U;
volatile unsigned int err0      = 0U;

/* DMA descriptor control block arrays */
dma_desc_t data_tdbdcb[2];
dma_desc_t data_rebdcb[2];
dma_desc_t cmd_tdbdcb[2];
dma_desc_t cmd_rebdcb[2];
