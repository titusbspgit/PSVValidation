// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_rb_reg_wr_rd_test
 * Purpose: Supporting definitions for MIPI CSI2 RB register read/write
 *          verification testcase.
 */

#include "test.h"

/* Headers field was NA - no additional Meta TestPlan headers to include */

/* Number of RB registers under test */
#define MIPI_CSI2_RB_REG_COUNT  10U

/* Test write value for write-read-back verification */
#define MIPI_CSI2_RB_TEST_WR_VALUE  0xFFFFFFFFU

/* Register address array */
static const uint32_t addr_array[MIPI_CSI2_RB_REG_COUNT] = {
    MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,    /* offset 0x00 */
    MIZAR_MIPI_CSI2_RB_REG_LINE_INFO,           /* offset 0x04 */
    MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL,  /* offset 0x08 */
    MIZAR_MIPI_CSI2_RB_REG_LANE_CLK,            /* offset 0x0C */
    MIZAR_MIPI_CSI2_RB_REG_MEM,                 /* offset 0x10 */
    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW,        /* offset 0x14 */
    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK,       /* offset 0x18 */
    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE,     /* offset 0x1C */
    MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA,         /* offset 0x20 */
    MIZAR_MIPI_CSI2_RB_REG_FLUSH                 /* offset 0x24 */
};

/* Reset default value array */
static const uint32_t default_val_array[MIPI_CSI2_RB_REG_COUNT] = {
    0x00000000U,  /* VIRTUAL_CHANNEL */
    0x00000000U,  /* LINE_INFO */
    0x00000000U,  /* FIFO_THRESHOLD_VAL */
    0x00000000U,  /* LANE_CLK */
    0x00000000U,  /* MEM */
    0x00000000U,  /* INTERRUPT_RAW */
    0x00000000U,  /* INTERRUPT_MASK */
    0x00000000U,  /* INTERRUPT_ENABLE */
    0x00000000U,  /* CONTROL_DATA */
    0x00000000U   /* FLUSH */
};

/* Read mask array */
static const uint32_t read_mask_array[MIPI_CSI2_RB_REG_COUNT] = {
    0xFFFFFFFFU,  /* VIRTUAL_CHANNEL */
    0xFFFFFFFFU,  /* LINE_INFO */
    0xFFFFFFFFU,  /* FIFO_THRESHOLD_VAL */
    0xFFFFFFFFU,  /* LANE_CLK */
    0xFFFFFFFFU,  /* MEM */
    0xFFFFFFFFU,  /* INTERRUPT_RAW */
    0xFFFFFFFFU,  /* INTERRUPT_MASK */
    0xFFFFFFFFU,  /* INTERRUPT_ENABLE */
    0xFFFFFFFFU,  /* CONTROL_DATA */
    0xFFFFFFFFU   /* FLUSH */
};

/* Write mask array */
static const uint32_t write_mask_array[MIPI_CSI2_RB_REG_COUNT] = {
    0xFFFFFFFFU,  /* VIRTUAL_CHANNEL */
    0xFFFFFFFFU,  /* LINE_INFO */
    0xFFFFFFFFU,  /* FIFO_THRESHOLD_VAL */
    0xFFFFFFFFU,  /* LANE_CLK */
    0xFFFFFFFFU,  /* MEM */
    0xFFFFFFFFU,  /* INTERRUPT_RAW */
    0xFFFFFFFFU,  /* INTERRUPT_MASK */
    0xFFFFFFFFU,  /* INTERRUPT_ENABLE */
    0xFFFFFFFFU,  /* CONTROL_DATA */
    0xFFFFFFFFU   /* FLUSH */
};
