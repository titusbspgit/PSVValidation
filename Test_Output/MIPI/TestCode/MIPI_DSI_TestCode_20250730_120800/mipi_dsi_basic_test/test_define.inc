// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_dsi_basic_test
 * Purpose: Supporting definitions for basic MIPI DSI DBI data transfer
 *          using the integrated DMA controller (DMA330) with interrupt-driven
 *          completion.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ---------------------------------------------------------------------------
 * Timeout constant for DMA completion polling
 * --------------------------------------------------------------------------- */
#define MIPI_DSI_BASIC_TEST_TIMEOUT 100000U

/* ---------------------------------------------------------------------------
 * Volatile global flags used by ISR and main test flow
 * --------------------------------------------------------------------------- */
static volatile unsigned int int_pend_flag  = 0U;
static volatile unsigned int int_pend1_flag = 0U;

/* ---------------------------------------------------------------------------
 * DMA Descriptor Arrays
 * --------------------------------------------------------------------------- */
static unsigned long long data_tdbdcb[2];
static unsigned long long data_rebdcb[2];
static unsigned long long cmd_tdbdcb[2];
static unsigned long long cmd_rebdcb[2];
