// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case  : mipi_csi2_test_pattern_generator
 * Purpose    : Validates the MIPI CSI-2 internal test pattern generator (PPI PG)
 *              and verifies data reception through the DMA path by configuring
 *              virtual channel, enabling interrupts, initializing D-PHY, programming
 *              DMA, enabling pattern generator, and verifying DMA completion.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

/* FV Template framework headers */
#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ---- Compile-time configurable macros from Meta TestPlan ---- */

#ifndef GDMA_CSI2_DATA_DEST_ADDR2
#define GDMA_CSI2_DATA_DEST_ADDR2   0x00000000U
#endif

#ifndef GDMA_CTRL_DATA_DEST_ADDR2
#define GDMA_CTRL_DATA_DEST_ADDR2   0x00000000U
#endif

/* ---- Pattern Generator Configuration ---- */
#define MIPI_CSI2_PG_VRES                   16U
#define MIPI_CSI2_PG_HRES                   320U
#define MIPI_CSI2_PG_VALID_BITS_PER_PIXEL   24U
#define MIPI_CSI2_PG_CONFIG_VALUE           0xe401U
#define MIPI_CSI2_PG_DATA_TYPE              0x24U

/* ---- DMA register offsets ---- */
#define MIPI_CSI2_DMA_INTMIS_OFFSET         0x00000010U
#define MIPI_CSI2_DMA_INTCLR_OFFSET         0x0000002CU

/* ---- GDMA register base address ---- */
#define MIPI_CSI2_GDMA_REG_BASE             0xE6A00000U

/* ---- enableclkgating_csiphy register offset ---- */
#define MIPI_CSI2_ENABLECLKGATING_CSIPHY_OFFSET  0xf4U

/* ---- PHY stop state expected value ---- */
#define MIPI_CSI2_PHY_STOPSTATE_EXPECTED    0x0001000fU

/* ---- External function prototypes ---- */
extern void snps_phy_init(void);
extern void dma_trnsfr_instn_preload_incr_addr(unsigned int base, unsigned int ch, unsigned int src, unsigned int dst, unsigned int size);
extern void DMAGO_CSI(unsigned int base, unsigned int pc, unsigned int ch);
