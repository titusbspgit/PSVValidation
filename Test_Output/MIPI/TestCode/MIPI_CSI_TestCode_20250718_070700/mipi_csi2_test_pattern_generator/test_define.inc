// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose: Supporting definitions for MIPI CSI2 internal test pattern
 *          generator testcase.
 */

#include "test.h"

/* Headers field was NA - no additional Meta TestPlan headers to include */

/* Polling timeout for PHY_STOPSTATE */
#define MIPI_CSI2_PHY_STOPSTATE_TIMEOUT  10000U

/* Interrupt mask enable value */
#define MIPI_CSI2_INT_MASK_ENABLE_ALL    0xFFFFFFFFU

/* Virtual channel configuration value */
#define MIPI_CSI2_VIRTUAL_CHANNEL_VALUE  0x00000000U

/* Control data configuration value */
#define MIPI_CSI2_CONTROL_DATA_VALUE     0x00000001U

/* Pattern generator vertical resolution */
#define MIPI_CSI2_PG_VRES_VALUE          0x00000438U

/* Pattern generator horizontal resolution */
#define MIPI_CSI2_PG_HRES_VALUE          0x00000780U

/* Pattern generator configuration */
#define MIPI_CSI2_PG_CONFIG_VALUE        0x00000001U

/* Pattern generator enable value */
#define MIPI_CSI2_PG_ENABLE_VALUE        0x00000001U

/* Pattern generator disable value */
#define MIPI_CSI2_PG_DISABLE_VALUE       0x00000000U

/* DMA address configuration value */
#define MIPI_CSI2_DMA_ADDR_VALUE         0x00000000U
