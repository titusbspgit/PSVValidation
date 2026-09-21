// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_csi2_rb_reg_wr_rd_test
 * Purpose: Supporting definitions for MIPI CSI-2 RB REG write-read
 *          verification testcase. Contains headers, macros, register
 *          addresses, arrays, and testcase constants.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include <mipi_csi2_rb_reg.h>

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros from Meta TestPlan JSON */
// MANUAL_REVIEW: The following macros were listed in the Meta TestPlan JSON
// without explicit values. Define them per the platform/IP configuration.
#ifndef SOFT_RST_REG_ADDRESS
// #define SOFT_RST_REG_ADDRESS  /* MANUAL_REVIEW: value not provided */
#endif

#ifndef SOFT_RST_REG_DATA
// #define SOFT_RST_REG_DATA  /* MANUAL_REVIEW: value not provided */
#endif

/* Register count - addr_array declared with size 60, 10 entries populated */
#ifndef CNT
#define CNT 60U
#endif

/* Arrays from Meta TestPlan JSON */
// MANUAL_REVIEW: The following arrays are declared per the Meta TestPlan JSON.
// Only 10 entries are populated for the actual RB REG registers.
// Remaining entries (index 10-59) are uninitialized (zero).

static const uintptr_t addr_array[60] = {
    MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,     /* index 0 */
    MIZAR_MIPI_CSI2_RB_REG_LINE_INFO,            /* index 1 */
    MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL,   /* index 2 */
    MIZAR_MIPI_CSI2_RB_REG_LANE_CLK,             /* index 3 */
    MIZAR_MIPI_CSI2_RB_REG_MEM,                  /* index 4 */
    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW,        /* index 5 */
    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK,       /* index 6 */
    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE,     /* index 7 */
    MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA,         /* index 8 */
    MIZAR_MIPI_CSI2_RB_REG_FLUSH,                /* index 9 */
    /* indices 10-59 are zero (uninitialized) */
};

// MANUAL_REVIEW: default_value_array values must be populated per the
// register specification. Only 10 entries are meaningful.
static const uint32_t default_value_array[60] = {
    /* MANUAL_REVIEW: populate default reset values for indices 0-9 */
    0x00000000U, /* index 0: virtual_channel */
    0x00000000U, /* index 1: line_info */
    0x00000000U, /* index 2: fifo_threshold_val */
    0x00000000U, /* index 3: lane_clk */
    0x00000000U, /* index 4: mem */
    0x00000000U, /* index 5: interrupt_raw */
    0x00000000U, /* index 6: interrupt_mask */
    0x00000000U, /* index 7: interrupt_enable */
    0x00000000U, /* index 8: control_data */
    0x00000000U, /* index 9: flush */
};

// MANUAL_REVIEW: read_mask_array values must be populated per the
// register specification.
static const uint32_t read_mask_array[60] = {
    /* MANUAL_REVIEW: populate read masks for indices 0-9 */
    0xFFFFFFFFU, /* index 0: virtual_channel */
    0xFFFFFFFFU, /* index 1: line_info */
    0xFFFFFFFFU, /* index 2: fifo_threshold_val */
    0xFFFFFFFFU, /* index 3: lane_clk */
    0xFFFFFFFFU, /* index 4: mem */
    0xFFFFFFFFU, /* index 5: interrupt_raw */
    0xFFFFFFFFU, /* index 6: interrupt_mask */
    0xFFFFFFFFU, /* index 7: interrupt_enable */
    0xFFFFFFFFU, /* index 8: control_data */
    0xFFFFFFFFU, /* index 9: flush */
};

// MANUAL_REVIEW: write_mask_array values must be populated per the
// register specification.
static const uint32_t write_mask_array[60] = {
    /* MANUAL_REVIEW: populate write masks for indices 0-9 */
    0xFFFFFFFFU, /* index 0: virtual_channel */
    0xFFFFFFFFU, /* index 1: line_info */
    0xFFFFFFFFU, /* index 2: fifo_threshold_val */
    0xFFFFFFFFU, /* index 3: lane_clk */
    0xFFFFFFFFU, /* index 4: mem */
    0x00000000U, /* index 5: interrupt_raw (read-only) */
    0xFFFFFFFFU, /* index 6: interrupt_mask */
    0xFFFFFFFFU, /* index 7: interrupt_enable */
    0xFFFFFFFFU, /* index 8: control_data */
    0xFFFFFFFFU, /* index 9: flush */
};

/* skip_array: index 5 (interrupt_raw), 9 (flush), 10 are skipped */
static const uint32_t skip_array[60] = {
    0U, 0U, 0U, 0U, 0U,  /* indices 0-4 */
    1U,                    /* index 5: interrupt_raw - skip */
    0U, 0U, 0U,           /* indices 6-8 */
    1U,                    /* index 9: flush - skip */
    1U,                    /* index 10: skip */
    /* indices 11-59 default to 0 */
};

/* Six distinct test patterns to exercise all writable bit positions */
static const uint32_t chk_val[6] = {
    0x00000000U,
    0xFFFFFFFFU,
    0x55555555U,
    0xAAAAAAAAU,
    0x0F0F0F0FU,
    0xF0F0F0F0U
};
