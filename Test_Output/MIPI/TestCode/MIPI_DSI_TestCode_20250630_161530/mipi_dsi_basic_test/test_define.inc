// Author - AI Force 2.3. 30-Jun-2025 16:15 IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_dsi_basic_test
 * Purpose: Supporting data definitions for basic MIPI DSI DBI data transfer
 *   testcase using integrated DMA controller (DMA330).
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"

/* Timeout for polling DMA completion */
#define MIPI_DSI_POLL_TIMEOUT 100000U

/*
 * DMA descriptor arrays
 * CH0 (data): data_tdbdcb / data_rebdcb
 * CH1 (command): cmd_tdbdcb / cmd_rebdcb
 */
static unsigned long long data_tdbdcb[2];
static unsigned long long data_rebdcb[2];
static unsigned long long cmd_tdbdcb[2];
static unsigned long long cmd_rebdcb[2];
