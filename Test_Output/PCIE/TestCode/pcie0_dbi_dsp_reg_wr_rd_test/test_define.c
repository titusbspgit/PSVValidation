#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include <pcie.h>

#define SOFT_RST_REG_ADDRESS 0x00000000
#define SOFT_RST_REG_DATA 0x00000000
#define CNT 775

/*
 * Register arrays derived strictly from Meta Impacted Registers list.
 * RAG mapping (Rg-Emb-Mpsoc-Macro-Reg-Spec) used to derive masks and defaults
 * only where specification exists. Where spec is not found, masks are 0 and
 * default values are 0 to ensure those entries are skipped by logic.
 */

/* Addresses array: only first entries populated with impacted register macros. */
static const unsigned int addr_array[CNT] = {
    [0]  = mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG,
    [1]  = mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS,
    [2]  = mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF,
    [3]  = mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG,
    [4]  = mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG,
    [5]  = mizar_PCIE0_DBI_DSP_LINK_CONTROL_LINK_STATUS_REG,
    [6]  = mizar_PCIE0_DBI_DSP_DMA_WRITE_ENGINE_EN_OFF,
    [7]  = mizar_PCIE0_DBI_DSP_DMA_READ_ENGINE_EN_OFF,
    [8]  = mizar_PCIE0_DBI_DSP_AER_EXT_CAP_HDR_OFF,
    [9]  = mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0,
    [10] = mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_2_OFF_INBOUND_0,
    [11] = mizar_PCIE0_DBI_DSP_PCI_MSI_CAP_ID_NEXT_CTRL_REG
};

/* Read mask array: bits defined only where RAG spec is available. */
static const unsigned int read_mask_array[CNT] = {
    /* mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG: bits [21],[19],[18:16],[15:8],[7:0] */
    [0]  = 0x002FFFFF,
    /* mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS: spec not found */
    [1]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF: spec not found */
    [2]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG: [31:0] */
    [3]  = 0xFFFFFFFF,
    /* mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG: bits [4],[3],[2] */
    [4]  = 0x0000001C,
    /* mizar_PCIE0_DBI_DSP_LINK_CONTROL_LINK_STATUS_REG: spec not found */
    [5]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_DMA_WRITE_ENGINE_EN_OFF: spec not found */
    [6]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_DMA_READ_ENGINE_EN_OFF: spec not found */
    [7]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_AER_EXT_CAP_HDR_OFF: [31:0] */
    [8]  = 0xFFFFFFFF,
    /* mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0: bits [10:0] */
    [9]  = 0x000007FF,
    /* mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_2_OFF_INBOUND_0: spec not found */
    [10] = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_PCI_MSI_CAP_ID_NEXT_CTRL_REG: [31:0] */
    [11] = 0xFFFFFFFF
};

/* Write mask array: bits writable only where RAG spec marks as rw. */
static const unsigned int write_mask_array[CNT] = {
    /* mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG: ro only */
    [0]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS: spec not found */
    [1]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF: spec not found */
    [2]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG: ro */
    [3]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG: BME bit[2] is rw */
    [4]  = 0x00000004,
    /* mizar_PCIE0_DBI_DSP_LINK_CONTROL_LINK_STATUS_REG: spec not found */
    [5]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_DMA_WRITE_ENGINE_EN_OFF: spec not found */
    [6]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_DMA_READ_ENGINE_EN_OFF: spec not found */
    [7]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_AER_EXT_CAP_HDR_OFF: ro */
    [8]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0: bits [10:0] are rw */
    [9]  = 0x000007FF,
    /* mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_2_OFF_INBOUND_0: spec not found */
    [10] = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_PCI_MSI_CAP_ID_NEXT_CTRL_REG: bits [26],[22:20],[16] are rw */
    [11] = 0x04710000
};

/* Default/reset value array: only bits with known reset values are populated. */
static const unsigned int default_value_array[CNT] = {
    /* mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG: PM_SPEC_VER=0x3, NEXT=0x50, CAP_ID=0x1 */
    [0]  = 0x00035001,
    /* mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS: spec not found */
    [1]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF: spec not found */
    [2]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG: DEVICE_ID=0xABCD, VENDOR_ID=0x16C3 */
    [3]  = 0xABCD16C3,
    /* mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG: known bits reset to 0 */
    [4]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_LINK_CONTROL_LINK_STATUS_REG: spec not found */
    [5]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_DMA_WRITE_ENGINE_EN_OFF: spec not found */
    [6]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_DMA_READ_ENGINE_EN_OFF: spec not found */
    [7]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_AER_EXT_CAP_HDR_OFF: NEXT_OFFSET=0x148, CAP_VER=0x2, CAP_ID=0x1 */
    [8]  = 0x14820001,
    /* mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0: defined fields reset to 0 */
    [9]  = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_2_OFF_INBOUND_0: spec not found */
    [10] = 0x00000000,
    /* mizar_PCIE0_DBI_DSP_PCI_MSI_CAP_ID_NEXT_CTRL_REG: per spec */
    [11] = 0x018A7005
};

/* Skip array: No explicit skip list provided in Meta Arrays; default to 0. */
static const unsigned char skip_array[CNT] = { 0 };
