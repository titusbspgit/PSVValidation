// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

// Meta Headers
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

// Framework headers for register access and logging
#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

// Meta Macros
#define GDMA_CSI2_DATA_DEST_ADDR2   0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000

#ifdef DPHY_LANES_TEST

#ifndef LS_LE_EN
#define LS_LE_EN            1
#endif

#ifndef TOTAL_FRAME
#define TOTAL_FRAME         1
#endif

#ifndef VC_ID
#define VC_ID               3
#endif

#ifndef VRES
#ifdef GDMA0_FULL_MEM
#define VRES                1080
#else
#define VRES                3
#endif
#endif

#ifndef HRES
#ifdef GDMA0_FULL_MEM
#define HRES                1920
#else
#define HRES                64
#endif
#endif

#ifndef DATA_TYPE
#define DATA_TYPE           CSI2_RGB888
#endif

#endif // DPHY_LANES_TEST
