// Author - AI Force 2.3. 28-Jul-2025 17:00 IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 *
 * Test Case : mipi_csi2_dphy_lanes_test
 * Purpose   : Register addresses, masks, polling constants, and context
 *             structure for MIPI CSI2 DPHY lane configuration and validation.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ---- Headers field from Meta TestPlan JSON was NA ---- */

/* ---- Macros field from Meta TestPlan JSON was NA ---- */

/* ---- Arrays field from Meta TestPlan JSON was NA ---- */

/*
 * Register addresses for MIPI CSI2 DPHY lanes test.
 * These are preserved exactly as listed in Impacted Registers.
 * Actual base addresses must be set in the SoC/IP configuration.
 */

#ifndef MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL
#define MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL   0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA
#define MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA      0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE
#define MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE        0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_N_LANES
#define MIZAR_MIPI_CSI2_HOST_N_LANES              0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN
#define MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN          0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL    0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL    0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY          0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE         0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL 0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL   0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL   0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL     0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID            0x0UL
#endif

#ifndef MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED
#define MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED      0x0UL
#endif

/* ---- Polling and mask constants ---- */
#define MIPI_CSI2_PHY_STOPSTATE_POLL_MAX  1000U
#define MIPI_CSI2_PHY_STOPSTATE_MASK      0x1U
#define MIPI_CSI2_INT_MASK_ALL            0xFFFFFFFFU

/* ---- Testcase context structure ---- */
typedef struct {
    unsigned int errors;
} mipi_csi2_dphy_test_ctx_t;
