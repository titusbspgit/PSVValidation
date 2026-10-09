// Author - AI Force 2.3. 09-Jun-2025 21:10 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Purpose: Validates MIPI CSI-2 D-PHY lane configuration by iterating through
 *   lane counts from 4 lanes down to 1 lane, performing DMA-based control and
 *   data packet transfers with interrupt-driven completion polling for each
 *   lane configuration.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

/* Macros from Meta TestPlan */
#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000
#define LS_LE_EN 1
#define TOTAL_FRAME 1
#define VC_ID 3
#define VRES 3
#define HRES 64
#define DATA_TYPE CSI2_RGB888

/* PSV poll timeout constant */
#ifndef PSV_POLL_TIMEOUT
#define PSV_POLL_TIMEOUT 100000
#endif

/* External function declarations */
extern void snps_phy_init(void);
extern void dma_trnsfr_instn_preload(unsigned int preload_loc, unsigned int gdma_reg_base, unsigned int src, unsigned int dest, long long int size, int irq_num);
extern void DMAGO_CSI(unsigned int gdma_reg_base, unsigned int pc, unsigned int channel);
