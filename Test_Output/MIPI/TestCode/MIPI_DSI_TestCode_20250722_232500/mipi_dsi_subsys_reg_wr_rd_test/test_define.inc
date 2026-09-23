// Author - AI Force 2.3. 22-Jul-2025 17:50 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: mipi_dsi_subsys_reg_wr_rd_test
 * Purpose: Supporting definitions for MIPI DSI subsystem register write-read validation testcase.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Register count */
#define MIPI_DSI_SUBSYS_REG_CNT 5U

/* Number of data patterns */
#define MIPI_DSI_SUBSYS_NUM_PATTERNS 6U

/* MANUAL_REVIEW: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_SUBSYS_LOW_PWR address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_SUBSYS_DBITE address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV address macro is expected from headers. */
/* MANUAL_REVIEW: MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW address macro is expected from headers. */

/* Register address array */
static const uint32_t addr_array[MIPI_DSI_SUBSYS_REG_CNT] = {
    MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,
    MIZAR_MIPI_DSI_SUBSYS_LOW_PWR,
    MIZAR_MIPI_DSI_SUBSYS_DBITE,
    MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV,
    MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
};

/*
 * MANUAL_REVIEW: The following rst_val_array, rd_mask_array, and wr_mask_array
 * require actual reset values and masks from the MIPI DSI subsystem register
 * specification. Placeholder values 0x0 and 0xFFFFFFFF are used below.
 * Replace with correct values before compilation.
 */

/* Expected reset values for each register */
static const uint32_t rst_val_array[MIPI_DSI_SUBSYS_REG_CNT] = {
    0x00000000U, /* MANUAL_REVIEW: DATA_FIFO_THRESHOLD_VAL reset value */
    0x00000000U, /* MANUAL_REVIEW: LOW_PWR reset value */
    0x00000000U, /* MANUAL_REVIEW: DBITE reset value */
    0x00000000U, /* MANUAL_REVIEW: DBI_FDIV reset value */
    0x00000000U  /* MANUAL_REVIEW: INTERRUPT_RAW reset value */
};

/* Read masks for each register */
static const uint32_t rd_mask_array[MIPI_DSI_SUBSYS_REG_CNT] = {
    0xFFFFFFFFU, /* MANUAL_REVIEW: DATA_FIFO_THRESHOLD_VAL read mask */
    0xFFFFFFFFU, /* MANUAL_REVIEW: LOW_PWR read mask */
    0xFFFFFFFFU, /* MANUAL_REVIEW: DBITE read mask */
    0xFFFFFFFFU, /* MANUAL_REVIEW: DBI_FDIV read mask */
    0xFFFFFFFFU  /* MANUAL_REVIEW: INTERRUPT_RAW read mask */
};

/* Write masks for each register */
static const uint32_t wr_mask_array[MIPI_DSI_SUBSYS_REG_CNT] = {
    0xFFFFFFFFU, /* MANUAL_REVIEW: DATA_FIFO_THRESHOLD_VAL write mask */
    0xFFFFFFFFU, /* MANUAL_REVIEW: LOW_PWR write mask */
    0xFFFFFFFFU, /* MANUAL_REVIEW: DBITE write mask */
    0xFFFFFFFFU, /* MANUAL_REVIEW: DBI_FDIV write mask */
    0x00000000U  /* INTERRUPT_RAW: not written (skip_array[4]=1) */
};

/* Skip array: 1 = skip write operations for this register */
static const uint32_t skip_array[MIPI_DSI_SUBSYS_REG_CNT] = {
    0U, 0U, 0U, 0U, 1U
};

/* Data patterns for write-read testing */
static const uint32_t data_patterns[MIPI_DSI_SUBSYS_NUM_PATTERNS] = {
    0x55555555U,
    0xAAAAAAAAU,
    0x00000000U,
    0xFFFFFFFFU,
    0x5A5A5A5AU,
    0xA5A5A5A5U
};
