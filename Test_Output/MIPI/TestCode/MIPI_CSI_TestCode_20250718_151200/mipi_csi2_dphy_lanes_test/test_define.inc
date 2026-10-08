// Author - AI Force 2.3. 18-Jul-2025 15:12 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Purpose:  Supporting definitions for MIPI CSI-2 D-PHY lane configuration
 *           and CSI-2 data reception testcase.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* --------------------------------------------------------------------------
 * Macros from Meta TestPlan JSON
 * -------------------------------------------------------------------------- */
#define GDMA_CSI2_DATA_DEST_ADDR2   0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000
#define LS_LE_EN                    1
#define TOTAL_FRAME                 1
#define VC_ID                       3

#ifdef GDMA0_FULL_MEM
#define VRES                        1080
#define HRES                        1920
#else
#define VRES                        3
#define HRES                        64
#endif

#define DATA_TYPE                   CSI2_RGB888
