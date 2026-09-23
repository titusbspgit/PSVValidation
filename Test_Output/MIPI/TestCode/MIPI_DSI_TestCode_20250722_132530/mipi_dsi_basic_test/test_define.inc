// Author - AI Force 2.3. 22-Jul-2025 13:25 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_basic_test
 * Purpose:   MIPI DSI DBI-mode DMA transfer with interrupt handling.
 *            Defines register addresses, bit field macros, and polling
 *            constants for the testcase.
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

#ifndef MIZAR_MIPI_DSI_DMAC_INTEN
#define MIZAR_MIPI_DSI_DMAC_INTEN              0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE 0x0UL
#endif

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

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST0
#define MIZAR_MIPI_DSI_DMAC_DBGINST0           0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGINST1
#define MIZAR_MIPI_DSI_DMAC_DBGINST1           0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_DBGCMD
#define MIZAR_MIPI_DSI_DMAC_DBGCMD             0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK   0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTMIS
#define MIZAR_MIPI_DSI_DMAC_INTMIS             0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_DMAC_INTCLR
#define MIZAR_MIPI_DSI_DMAC_INTCLR             0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW    0x0UL
#endif

/* ======================================================================== */
/* Bit Field Macros                                                         */
/* ======================================================================== */

// MANUAL_REVIEW: Bit field positions are derived from test step descriptions.
// Verify against the hardware register specification.

#ifndef MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR
#define MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR  (1U << 0)
#endif

#ifndef MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR
#define MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR    (1U << 0)
#endif

#ifndef MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME
#define MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME (0xFFU << 8)
#endif

/* ======================================================================== */
/* Polling Constants                                                        */
/* ======================================================================== */

#ifndef MIPI_DSI_POLL_TIMEOUT
#define MIPI_DSI_POLL_TIMEOUT                  1000U
#endif
