// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case    : mipi_csi2_test_pattern_generator
 * Purpose      : Validates the MIPI CSI-2 host controller's internal test
 *                pattern generator (PG) functionality and DMA-based data
 *                capture. Configures virtual channel, enables CSI-2 and DMA
 *                interrupts, initializes D-PHY, polls PHY stop state, programs
 *                DMA channel 0 for CSI-2 data capture with computed transfer
 *                size (320x16, 24bpp, 8-byte aligned), enables the pattern
 *                generator, captures data via DMA, disables the pattern
 *                generator, and polls for DMA completion.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros from Meta TestPlan JSON */
#define GDMA_CSI2_DATA_DEST_ADDR2   0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000
#define PG_PATTERN_VRES_VAL         0x10
#define PG_PATTERN_HRES_VAL         0x70140
#define PG_CONFIG_VAL               0xe401
#define PG_ENABLE_VAL               1
#define PG_DISABLE_VAL              0
#define FRACDIV_OUTPUT_OFFSET        0xf4
#define PHY_STOPSTATE_EXPECTED      0x1000f

/* DMA Interrupt Offsets */
#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET 0x10
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET 0x14
#endif

#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET  0x18
#endif

/* Timeout count for polling loops */
#ifndef MIPI_CSI2_TIMEOUT_COUNT
#define MIPI_CSI2_TIMEOUT_COUNT     100000
#endif
