// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose: Validates the MIPI CSI-2 internal test pattern generator
 *   functionality by configuring the CSI-2 subsystem to receive data from
 *   the host controller's built-in pattern generator and transferring the
 *   received data via DMA.
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
#define GDMA_CSI2_DATA_DEST_ADDR2    0xE6040080U
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000U

/* Virtual channel ID */
#define VCID                         3U

/* Unselected path: ((vcid + 1) & 0xf) */
#define VCID_UNSELECTED_PATH         (((VCID) + 1U) & 0xfU)

/* MANUAL_REVIEW: GDMA path selection is conditional compilation in DV source. */
/* Using GDMA0_PATH default. Adjust for target GDMA path. */
/* For GDMA0_PATH: vcid_csi2_wrap_reg = vcid | (vcid_unselected_path << 4) | ... */
#ifndef GDMA_PATH
#define GDMA_PATH                    0U
#endif

/* GDMA register base address: 0xE6A00000 + (gdma_path * 0x1000) */
#define GDMA_REG_BASE                (0xE6A00000UL + ((unsigned long)(GDMA_PATH) * 0x1000UL))

/* Virtual channel wrap register value for GDMA0_PATH default */
/* MANUAL_REVIEW: Adjust vcid_csi2_wrap_reg computation for target GDMA path. */
/* GDMA0_PATH: bits[3:0]=vcid, bits[7:4]=vcid_unselected, bits[11:8]=vcid_unselected, bits[15:12]=vcid_unselected */
#ifndef VCID_CSI2_WRAP_REG
#define VCID_CSI2_WRAP_REG           ((VCID) | (VCID_UNSELECTED_PATH << 4) | (VCID_UNSELECTED_PATH << 8) | (VCID_UNSELECTED_PATH << 12))
#endif

/* Pattern generator parameters */
#define PG_HRES                      320U
#define PG_VRES                      16U
#define VALID_BITS_PER_PIXEL         24U

/* MANUAL_REVIEW: DATA_TYPE encoding for PG_CONFIG. */
/* Using RAW8=0x24 as a default from DV source context. */
#ifndef DATA_TYPE
#define DATA_TYPE                    0x24U
#endif

/* Pattern generator config value */
/* MANUAL_REVIEW: Exact PG_CONFIG encoding depends on DV source. */
/* Using (DATA_TYPE | (VCID << 6)) as a reasonable default. */
#ifndef PG_CONFIG_VAL
#define PG_CONFIG_VAL                ((DATA_TYPE) | ((VCID) << 6))
#endif

/* DMA register offsets */
/* MANUAL_REVIEW: These offsets are expected from mipi_csi2.h or test_common.h. */
/* If not provided by those headers, define them here. */
#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET   0x20U
#endif

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET  0x24U
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET  0x2CU
#endif

/* Timeout values for polling loops */
#ifndef PHY_STOPSTATE_TIMEOUT
#define PHY_STOPSTATE_TIMEOUT        100000U
#endif

#ifndef DMA_POLL_TIMEOUT
#define DMA_POLL_TIMEOUT             100000U
#endif
