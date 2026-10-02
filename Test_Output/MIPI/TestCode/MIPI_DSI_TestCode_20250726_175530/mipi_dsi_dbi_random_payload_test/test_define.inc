// Author - AI Force 2.3. 26-Jul-2025 17:55 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name : mipi_dsi_dbi_random_payload_test
 * Purpose        : MIPI DSI DBI data transfers with randomized pixel payload
 *                  sizes using 2-channel DMA engine. CH0 for data, CH1 for
 *                  command. 10 iterations with polling-based DMA completion.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_dsi.h"
#include <math.h>

/* ---------------------------------------------------------------------------
 * Arrays: DMA descriptor control blocks and command arrays
 * Declared as extern here; defined in mipi_dsi_dbi_random_payload_test.c
 * --------------------------------------------------------------------------- */
extern dma_desc_t data_tdbdcb[10];
extern dma_desc_t data_rebdcb[10];
extern dma_desc_t cmd_tdbdcb[10];
extern dma_desc_t cmd_rebdcb[10];

extern unsigned int wr_cmd_arr[27];
extern unsigned int WR_CMD_SIZE[27];

/* ---------------------------------------------------------------------------
 * Macros: Source data was NA - no testcase-specific macros provided.
 * --------------------------------------------------------------------------- */
