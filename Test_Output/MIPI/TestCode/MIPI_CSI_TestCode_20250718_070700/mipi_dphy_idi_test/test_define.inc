// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dphy_idi_test
 * Purpose: Supporting definitions for MIPI CSI2 DPHY IDI (Image Data Interface)
 *          testcase.
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

/* Null/blank packet insertion value */
#define MIPI_CSI2_NULL_BLANK_VALUE       0x00000001U

/* Direct hex addresses from impacted registers */
#define MIPI_CSI2_EXTERNAL_WR_ADDR       0xa0243ffcUL
#define MIPI_CSI2_EXTERNAL_WR_VALUE      0x00000001U
#define MIPI_CSI2_EXTERNAL_RD_ADDR       0xE6001000UL
