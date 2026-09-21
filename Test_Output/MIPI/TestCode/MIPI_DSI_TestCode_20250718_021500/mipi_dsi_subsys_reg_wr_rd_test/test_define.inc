// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_subsys_reg_wr_rd_test
 * Purpose: Supporting definitions for MIPI DSI subsystem register write-read
 *          verification with array-driven reset-value and write-read phases.
 */

#include "test.h"

#include <stdint.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ========================================================================== */
/* Impacted Register Address Macros                                           */
/* ========================================================================== */
/* MANUAL_REVIEW: Register address values are not provided in the Meta TestPlan */
/* JSON. The macro names are preserved exactly as supplied. Actual address      */
/* values must be filled in from the SoC/IP register map.                       */

#ifndef MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL
#define MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL   0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_LOW_PWR
#define MIZAR_MIPI_DSI_SUBSYS_LOW_PWR                   0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_DBITE
#define MIZAR_MIPI_DSI_SUBSYS_DBITE                     0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV
#define MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV                  0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

#ifndef MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
#define MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW             0x0UL  /* MANUAL_REVIEW: Set actual address */
#endif

/* ========================================================================== */
/* Register Count                                                             */
/* ========================================================================== */

#define MIPI_DSI_SUBSYS_REG_COUNT  5U

/* ========================================================================== */
/* Default Value Macros                                                       */
/* ========================================================================== */
/* MANUAL_REVIEW: Default values are not provided in the Meta TestPlan JSON.   */
/* The macro names are derived from the validation criteria. Actual values     */
/* must be filled in from the SoC/IP specification.                            */

#ifndef MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL
#define MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL  0x0UL  /* MANUAL_REVIEW: Set actual default */
#endif

#ifndef MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL
#define MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL                  0x0UL  /* MANUAL_REVIEW: Set actual default */
#endif

#ifndef MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL
#define MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL                    0x0UL  /* MANUAL_REVIEW: Set actual default */
#endif

#ifndef MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL
#define MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL                 0x0UL  /* MANUAL_REVIEW: Set actual default */
#endif

#ifndef MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL
#define MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL            0x0UL  /* MANUAL_REVIEW: Set actual default */
#endif

/* ========================================================================== */
/* Read Mask Macros                                                           */
/* ========================================================================== */
/* MANUAL_REVIEW: Read mask values are not provided in the Meta TestPlan JSON. */
/* Actual masks must be filled in from the SoC/IP specification.               */

#ifndef MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_RD_MASK
#define MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_RD_MASK  0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_SUBSYS_LOW_PWR_RD_MASK
#define MIPI_DSI_SUBSYS_LOW_PWR_RD_MASK                  0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_SUBSYS_DBITE_RD_MASK
#define MIPI_DSI_SUBSYS_DBITE_RD_MASK                    0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_SUBSYS_DBI_FDIV_RD_MASK
#define MIPI_DSI_SUBSYS_DBI_FDIV_RD_MASK                 0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_SUBSYS_INTERRUPT_RAW_RD_MASK
#define MIPI_DSI_SUBSYS_INTERRUPT_RAW_RD_MASK            0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

/* ========================================================================== */
/* Write Mask Macros                                                          */
/* ========================================================================== */
/* MANUAL_REVIEW: Write mask values are not provided in the Meta TestPlan JSON.*/
/* Actual masks must be filled in from the SoC/IP specification.               */

#ifndef MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WR_MASK
#define MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WR_MASK  0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_SUBSYS_LOW_PWR_WR_MASK
#define MIPI_DSI_SUBSYS_LOW_PWR_WR_MASK                  0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_SUBSYS_DBITE_WR_MASK
#define MIPI_DSI_SUBSYS_DBITE_WR_MASK                    0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_SUBSYS_DBI_FDIV_WR_MASK
#define MIPI_DSI_SUBSYS_DBI_FDIV_WR_MASK                 0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

#ifndef MIPI_DSI_SUBSYS_INTERRUPT_RAW_WR_MASK
#define MIPI_DSI_SUBSYS_INTERRUPT_RAW_WR_MASK            0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

/* ========================================================================== */
/* Test Data Write Value                                                      */
/* ========================================================================== */

#ifndef MIPI_DSI_SUBSYS_TEST_DATA_WR
#define MIPI_DSI_SUBSYS_TEST_DATA_WR  0xA5A5A5A5UL  /* MANUAL_REVIEW: Set actual test write value */
#endif

/* ========================================================================== */
/* Register Address Array                                                     */
/* ========================================================================== */

static const uintptr_t addr_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_LOW_PWR,
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_DBITE,
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV,
    (uintptr_t)MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
};

/* ========================================================================== */
/* Default Value Array                                                        */
/* ========================================================================== */

static const uint32_t default_val_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    (uint32_t)MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL,
    (uint32_t)MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL,
    (uint32_t)MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL,
    (uint32_t)MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL,
    (uint32_t)MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL
};

/* ========================================================================== */
/* Read Mask Array                                                            */
/* ========================================================================== */

static const uint32_t rd_mask_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    (uint32_t)MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_RD_MASK,
    (uint32_t)MIPI_DSI_SUBSYS_LOW_PWR_RD_MASK,
    (uint32_t)MIPI_DSI_SUBSYS_DBITE_RD_MASK,
    (uint32_t)MIPI_DSI_SUBSYS_DBI_FDIV_RD_MASK,
    (uint32_t)MIPI_DSI_SUBSYS_INTERRUPT_RAW_RD_MASK
};

/* ========================================================================== */
/* Write Mask Array                                                           */
/* ========================================================================== */

static const uint32_t wr_mask_array[MIPI_DSI_SUBSYS_REG_COUNT] = {
    (uint32_t)MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WR_MASK,
    (uint32_t)MIPI_DSI_SUBSYS_LOW_PWR_WR_MASK,
    (uint32_t)MIPI_DSI_SUBSYS_DBITE_WR_MASK,
    (uint32_t)MIPI_DSI_SUBSYS_DBI_FDIV_WR_MASK,
    (uint32_t)MIPI_DSI_SUBSYS_INTERRUPT_RAW_WR_MASK
};
