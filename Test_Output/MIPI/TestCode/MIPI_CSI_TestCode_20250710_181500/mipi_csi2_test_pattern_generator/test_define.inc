// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"
#include "reg_access.h"
#include "log.h"

// Macros from Meta
#define GDMA_CSI2_DATA_DEST_ADDR2   0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000

// MIPI_CSI2_DMA_INTMIS_OFFSET used in polling (Step 31/32)
// MANUAL_REVIEW: Verify MIPI_CSI2_DMA_INTMIS_OFFSET is defined in mipi_csi2.h or test_common.h.
// If not defined externally, uncomment the following line and set the correct offset value.
// #define MIPI_CSI2_DMA_INTMIS_OFFSET 0x0 /* MANUAL_REVIEW: set correct DMA INTMIS offset */
