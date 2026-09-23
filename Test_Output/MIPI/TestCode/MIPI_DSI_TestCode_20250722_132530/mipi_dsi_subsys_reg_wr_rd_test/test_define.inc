// Author - AI Force 2.3. 22-Jul-2025 13:25 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_subsys_reg_wr_rd_test
 * Purpose:   Register write-read verification for MIPI DSI subsystem registers.
 *            Defines register addresses, expected reset values, read masks,
 *            write masks, skip flags, and test data patterns.
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

/* ======================================================================== */
/* Array Dimension Constants                                                */
/* ======================================================================== */

#define MIPI_DSI_SUBSYS_REG_COUNT      5U
#define MIPI_DSI_SUBSYS_PATTERN_COUNT  5U

/* ======================================================================== */
/* Register Address Array                                                   */
/* ======================================================================== */

static const uintptr_t addr_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_LOW_PWR,
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_DBITE,
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV,
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
};

/* ======================================================================== */
/* Expected Default (Reset) Values                                          */
/* ======================================================================== */

// MANUAL_REVIEW: Expected default reset values are not provided in the Meta
// TestPlan JSON. Replace 0x0U placeholders with actual hardware reset values.

static const uint32_t rst_val_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    0x0U,  /* DATA_FIFO_THRESHOLD_VAL */
    0x0U,  /* LOW_PWR */
    0x0U,  /* DBITE */
    0x0U,  /* DBI_FDIV */
    0x0U   /* INTERRUPT_RAW */
};

/* ======================================================================== */
/* Read Masks                                                               */
/* ======================================================================== */

// MANUAL_REVIEW: Read mask values are not provided in the Meta TestPlan JSON.
// Replace 0xFFFFFFFFU placeholders with actual register read masks.

static const uint32_t rd_mask_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    0xFFFFFFFFU,  /* DATA_FIFO_THRESHOLD_VAL */
    0xFFFFFFFFU,  /* LOW_PWR */
    0xFFFFFFFFU,  /* DBITE */
    0xFFFFFFFFU,  /* DBI_FDIV */
    0xFFFFFFFFU   /* INTERRUPT_RAW */
};

/* ======================================================================== */
/* Write Masks                                                              */
/* ======================================================================== */

// MANUAL_REVIEW: Write mask values are not provided in the Meta TestPlan JSON.
// Replace placeholders with actual register write masks.
// INTERRUPT_RAW is read-only so its write mask is 0x0U.

static const uint32_t wr_mask_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    0xFFFFFFFFU,  /* DATA_FIFO_THRESHOLD_VAL */
    0xFFFFFFFFU,  /* LOW_PWR */
    0xFFFFFFFFU,  /* DBITE */
    0xFFFFFFFFU,  /* DBI_FDIV */
    0x0U          /* INTERRUPT_RAW (read-only) */
};

/* ======================================================================== */
/* Skip Flags (1 = skip write-read verification, read-only register)        */
/* ======================================================================== */

static const uint32_t skip_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    0U,  /* DATA_FIFO_THRESHOLD_VAL */
    0U,  /* LOW_PWR */
    0U,  /* DBITE */
    0U,  /* DBI_FDIV */
    1U   /* INTERRUPT_RAW (read-only, skip write) */
};

/* ======================================================================== */
/* Test Data Patterns                                                       */
/* ======================================================================== */

static const uint32_t data_patterns[MIPI_DSI_SUBSYS_PATTERN_COUNT] = {
    0x00000000U,
    0xFFFFFFFFU,
    0x55555555U,
    0xAAAAAAAAU,
    0xA5A5A5A5U
};
