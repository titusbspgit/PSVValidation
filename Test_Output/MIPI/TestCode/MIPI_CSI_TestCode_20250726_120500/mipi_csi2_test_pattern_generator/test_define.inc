// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose: Supporting definitions for MIPI CSI-2 PPI Pattern Generator Test.
 *          Validates the internal PPI Pattern Generator functionality
 *          with DMA-based data reception (VRES=16, HRES=320, RGB888).
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros from Meta TestPlan JSON */
#ifndef GDMA_CSI2_DATA_DEST_ADDR2
#define GDMA_CSI2_DATA_DEST_ADDR2       0xE6002000U
#endif

#ifndef GDMA_CTRL_DATA_DEST_ADDR2
#define GDMA_CTRL_DATA_DEST_ADDR2       0xE6001000U
#endif

/* DMA register offsets */
#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET     0x028U
#endif

/* PHY stop state expected value */
#ifndef PHY_STOPSTATE_EXPECTED
#define PHY_STOPSTATE_EXPECTED          0x1000fU
#endif

/* Pattern Generator configuration values */
#ifndef PG_VRES_VALUE
#define PG_VRES_VALUE                   0x10U
#endif

#ifndef PG_HRES_VALUE
#define PG_HRES_VALUE                   0x70140U
#endif

#ifndef PG_CONFIG_VALUE
#define PG_CONFIG_VALUE                 0xe401U
#endif

#ifndef PG_ENABLE_VALUE
#define PG_ENABLE_VALUE                 1U
#endif

#ifndef PG_DISABLE_VALUE
#define PG_DISABLE_VALUE                0U
#endif
