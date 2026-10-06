// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case  : mipi_csi2_dphy_lanes_test
 * Purpose    : Validates MIPI CSI-2 DPHY lane configuration by iterating
 *              through all lane counts (4 lanes down to 1 lane) and performing
 *              complete CSI-2 data reception sequences with DMA transfers.
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

#ifndef LS_LE_EN
#define LS_LE_EN                    0
#endif

#ifndef TOTAL_FRAME
#define TOTAL_FRAME                 1
#endif

#ifndef VC_ID
#define VC_ID                       3
#endif

#ifndef VRES
#define VRES                        3
#endif

#ifndef HRES
#define HRES                        64
#endif

#ifndef DATA_TYPE
#define DATA_TYPE                   CSI2_RGB888
#endif

/* ---- DMA register offsets ---- */
#define MIPI_CSI2_DMA_INTMIS_OFFSET     0x00000010U
#define MIPI_CSI2_DMA_INTCLR_OFFSET     0x0000002CU

/* ---- GDMA register base address ---- */
#define MIPI_CSI2_GDMA_REG_BASE         0xE6A00000U

/* ---- Control data read address ---- */
#define MIPI_CSI2_CTRL_DATA_READ_ADDR   0xE6001000U

/* ---- Lane signal address ---- */
#define MIPI_CSI2_LANE_SIGNAL_ADDR      0xa0243ffcU

/* ---- PHY stop state expected value ---- */
#define MIPI_CSI2_PHY_STOPSTATE_EXPECTED 0x0001000fU

/* ---- External function prototypes ---- */
extern void snps_phy_init(void);
extern void dma_trnsfr_instn_preload(unsigned int base, unsigned int ch, unsigned int src, unsigned int dst, unsigned int size);
extern void DMAGO_CSI(unsigned int base, unsigned int pc, unsigned int ch);
