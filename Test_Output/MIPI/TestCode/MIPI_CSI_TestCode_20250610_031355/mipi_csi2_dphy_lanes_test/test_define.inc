// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

// Meta Headers (preserving first-seen order)
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

// FV framework headers
#include "log.h"
#include "reg_access.h"

// Testcase-specific macros from Meta
#define GDMA_CSI2_DATA_DEST_ADDR2   0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000
#define LS_LE_EN        1
#define TOTAL_FRAME     1
#define VC_ID           3
#define DATA_TYPE       CSI2_RGB888

// Resolution configuration - depends on GDMA0_FULL_MEM compile-time flag
#ifdef GDMA0_FULL_MEM
#define VRES    1080
#define HRES    1920
#else
#define VRES    3
#define HRES    64
#endif
