// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_dsi_subsys_reg_wr_rd_test
 * Purpose: Supporting definitions for MIPI DSI subsystem register write-read
 *          verification using array-driven iteration with reset-value and
 *          write-read phases.
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
/* Default Value Macros                                                       */
/* ========================================================================== */
/* MANUAL_REVIEW: Default values are not provided in the Meta TestPlan JSON.   */
/* The macro names are derived from the validation criteria. Actual values     */
/* must be filled in from the SoC/IP specification.                             */

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
#define MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL             0x0UL  /* MANUAL_REVIEW: Set actual default */
#endif

/* ========================================================================== */
/* Read Mask Macros                                                           */
/* ========================================================================== */
/* MANUAL_REVIEW: Read masks define which bits are readable. Actual values     */
/* must be filled in from the SoC/IP specification.                             */

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
#define MIPI_DSI_SUBSYS_INTERRUPT_RAW_RD_MASK             0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

/* ========================================================================== */
/* Write Mask Macros                                                          */
/* ========================================================================== */
/* MANUAL_REVIEW: Write masks define which bits are writable. Actual values    */
/* must be filled in from the SoC/IP specification.                             */

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
#define MIPI_DSI_SUBSYS_INTERRUPT_RAW_WR_MASK             0xFFFFFFFFUL  /* MANUAL_REVIEW: Set actual mask */
#endif

/* ========================================================================== */
/* Test Data Value                                                            */
/* ========================================================================== */

#ifndef MIPI_DSI_SUBSYS_REG_TEST_DATA
#define MIPI_DSI_SUBSYS_REG_TEST_DATA                    0xA5A5A5A5UL  /* MANUAL_REVIEW: Set actual test data */
#endif
