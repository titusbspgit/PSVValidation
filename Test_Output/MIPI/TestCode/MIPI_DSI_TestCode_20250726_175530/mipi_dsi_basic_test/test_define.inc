// Author - AI Force 2.3. 26-Jul-2025 17:55 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name : mipi_dsi_basic_test
 * Purpose        : Basic MIPI DSI DBI data transfer using 2-channel DMA engine.
 *                  CH0 for data, CH1 for command. Interrupt-driven completion.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"

/* ---------------------------------------------------------------------------
 * Arrays: DMA descriptor control blocks
 * Declared as extern here; defined in mipi_dsi_basic_test.c
 * --------------------------------------------------------------------------- */
extern dma_desc_t data_tdbdcb[2];
extern dma_desc_t data_rebdcb[2];
extern dma_desc_t cmd_tdbdcb[2];
extern dma_desc_t cmd_rebdcb[2];

/* ---------------------------------------------------------------------------
 * Macros: Source data was NA - no testcase-specific macros provided.
 * --------------------------------------------------------------------------- */
