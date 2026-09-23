// Author - AI Force 2.3. 22-Jul-2025 13:25 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_dbi_random_payload_test
 * Purpose:   MIPI DSI DBI random payload DMA transfer.
 *            Defines register addresses, polling constants, and expected
 *            completion value for the testcase.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ======================================================================== */
/* Register Address Macros                                                  */
/* ======================================================================== */

// MANUAL_REVIEW: Register address values are not provided in the Meta TestPlan JSON.
// Replace 0x0UL placeholders with actual SoC register addresses.

#ifndef MIZAR_MIPI_DSI_HOST_PHY_IF_CFG
#define MIZAR_MIPI_DSI_HOST_PHY_IF_CFG         0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_HOST_PCKHDL_CFG
#define MIZAR_MIPI_DSI_HOST_PCKHDL_CFG         0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_HOST_CLKMGR_CFG
#define MIZAR_MIPI_DSI_HOST_CLKMGR_CFG         0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL
#define MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL      0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTEN
#define MIZAR_MIPI_DSI_DMAC_INTEN              0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST0
#define MIZAR_MIPI_DSI_DMAC_DBGINST0           0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST1
#define MIZAR_MIPI_DSI_DMAC_DBGINST1           0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGCMD
#define MIZAR_MIPI_DSI_DMAC_DBGCMD             0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTMIS
#define MIZAR_MIPI_DSI_DMAC_INTMIS             0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTCLR
#define MIZAR_MIPI_DSI_DMAC_INTCLR             0x0UL
#endif

/* ======================================================================== */
/* Polling and Validation Constants                                         */
/* ======================================================================== */

#ifndef MIPI_DSI_POLL_TIMEOUT
#define MIPI_DSI_POLL_TIMEOUT                  1000U
#endif

#ifndef MIPI_DSI_DMA_COMPLETION_EXPECTED
#define MIPI_DSI_DMA_COMPLETION_EXPECTED        0x3U
#endif
