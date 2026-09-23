// Author - AI Force 2.3. 22-Jul-2025 13:25 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_subsys_reg_wr_rd_test
 * Purpose:   MIPI DSI subsystem register write-read verification.
 *            Defines register addresses for the five subsystem registers
 *            under test.
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
// All five registers are in the MIPI DSI subsystem block (base 0xE6A41000).

#ifndef MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL
#define MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL  0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_LOW_PWR
#define MIZAR_MIPI_DSI_SUBSYS_LOW_PWR                  0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_DBITE
#define MIZAR_MIPI_DSI_SUBSYS_DBITE                    0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV
#define MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV                 0x0UL
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW            0x0UL
#endif
