// Author - AI Force 2.3. 25-Jul-2025 10:00 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_dbi_random_payload_test
 * Purpose: Supporting definitions for MIPI DSI DBI random payload transfer
 *   via GDMA (DMA330) with two DMA channels.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Headers field from Meta TestPlan JSON was NA - no additional headers */

/* DMA interrupt enable mask for both channels (ch0 + ch1) */
#define DMA_INTEN_CH_MASK           0x3U

/* Packet handling configuration value */
#define PCKHDL_CFG_VALUE            0x3DU

/* Clock manager configuration value */
#define CLKMGR_CFG_VALUE            0x107U

/* DMA channel 0 descriptor address */
#define DMA_CH0_DESC_ADDR           0x00A00000UL

/* DMA channel 1 descriptor address */
#define DMA_CH1_DESC_ADDR           0x00A00100UL

/* Expected INTMIS value when both DMA channels complete */
#define DMA_INTMIS_BOTH_CH_DONE     0x3U

/* Polling timeout for INTMIS register */
#define POLL_TIMEOUT                100000U

/* Macros field from Meta TestPlan JSON was NA - no additional macros */

/* Arrays field from Meta TestPlan JSON was NA - no additional arrays */
