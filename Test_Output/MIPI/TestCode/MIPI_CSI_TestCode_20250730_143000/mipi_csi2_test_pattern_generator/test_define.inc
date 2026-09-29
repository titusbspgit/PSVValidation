// Author - AI Force 2.3. 30-Jul-2025 14:30 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: mipi_csi2_test_pattern_generator
 * Purpose: Configures and validates the MIPI CSI2 internal test pattern
 *          generator including vertical and horizontal resolution, pattern
 *          generator config, enable and disable cycle, virtual channel,
 *          control data, PHY stop state polling, DMA channel 0 configuration,
 *          DMA trigger, DMA completion polling, interrupt status read,
 *          and interrupt masking.
 */

#include "test.h"
#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Headers field from Meta TestPlan JSON was NA - no additional headers */

/* Macros field from Meta TestPlan JSON was NA - no additional macros */

/* DMA trigger offset from MIZAR_MIPI_CSI2_RB_REG_BASE */
#define MIPI_CSI2_DMA_TRIGGER_OFFSET  0xf4UL

/* DMA interrupt masked status offset from gdma_reg_base */
#define MIPI_CSI2_DMA_INTMIS_OFFSET   0x10UL

/* Polling timeout constant */
#define MIPI_CSI2_POLL_TIMEOUT  10000U

/* Arrays field from Meta TestPlan JSON was NA - no additional arrays */
