#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include <pcie.h>

// Macros (from Meta Macros - unchanged intent)
#define SOFT_RST_REG_ADDRESS 0x00000000
#define SOFT_RST_REG_DATA 0x00000000
#define CNT 775

// Impacted register arrays (mapped via RAG where available)
volatile unsigned int addr_array[CNT] = {
    // Populated from Meta Impacted Registers (order preserved)
    mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG,
    mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS,
    mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF,
    mizar_PCIE0_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG,
    mizar_PCIE0_DBI_USP_TYPE1_STATUS_COMMAND_REG,
    mizar_PCIE0_DBI_USP_LINK_CONTROL_LINK_STATUS_REG,
    mizar_PCIE0_DBI_USP_PCI_MSI_CAP_ID_NEXT_CTRL_REG,
    mizar_PCIE0_DBI_USP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE0_DBI_USP_DMA_WRITE_ENGINE_EN_OFF,
    mizar_PCIE0_DBI_USP_DMA_READ_ENGINE_EN_OFF,
    mizar_PCIE0_DBI_USP_AER_EXT_CAP_HDR_OFF,
    mizar_PCIE0_DBI_USP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0,
    mizar_PCIE0_DBI_USP_IATU_REGION_CTRL_2_OFF_INBOUND_0
    // Remaining entries (CNT-13) default-initialized to 0 by C standard
};

// Default/reset values for each address (0 where spec not found)
volatile unsigned int default_value_array[CNT] = {
    0x00035001, // mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG (PM_CAP_ID=0x1, NEXT_POINTER=0x50, SPEC_VER=0x3)
    0x00000000, // mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS (spec not found)
    0x00000000, // mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF (spec not found)
    0x00000000, // mizar_PCIE0_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG (spec not found)
    0x00000000, // mizar_PCIE0_DBI_USP_TYPE1_STATUS_COMMAND_REG (spec not found)
    0x00000000, // mizar_PCIE0_DBI_USP_LINK_CONTROL_LINK_STATUS_REG (spec not found)
    0x018A7005, // mizar_PCIE0_DBI_USP_PCI_MSI_CAP_ID_NEXT_CTRL_REG
    0x00000000, // mizar_PCIE0_DBI_USP_MSI_CAP_OFF_10H_REG
    0x00000000, // mizar_PCIE0_DBI_USP_DMA_WRITE_ENGINE_EN_OFF (spec not found)
    0x00000000, // mizar_PCIE0_DBI_USP_DMA_READ_ENGINE_EN_OFF (spec not found)
    0x014A0001, // mizar_PCIE0_DBI_USP_AER_EXT_CAP_HDR_OFF
    0x00000000, // mizar_PCIE0_DBI_USP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0
    0x00000000  // mizar_PCIE0_DBI_USP_IATU_REGION_CTRL_2_OFF_INBOUND_0 (spec not found)
    // Remaining entries default-initialized to 0
};

// Read masks (only bits defined as readable by spec; 0 where spec not found)
volatile unsigned int read_mask_array[CNT] = {
    0x0007FFFF, // CAP_ID_NXT_PTR_REG: PM_CAP_ID[7:0], NEXT_POINTER[15:8], SPEC_VER[18:16]
    0x00000000, // DEVICE_CONTROL_DEVICE_STATUS (unknown)
    0x00000000, // PL_DEBUG1_OFF (unknown)
    0x00000000, // TYPE1_DEV_ID_VEND_ID_REG (unknown)
    0x00000000, // TYPE1_STATUS_COMMAND_REG (unknown)
    0x00000000, // LINK_CONTROL_LINK_STATUS_REG (unknown)
    0xFFFFFFFF, // PCI_MSI_CAP_ID_NEXT_CTRL_REG: all fields readable
    0xFFFFFFFF, // MSI_CAP_OFF_10H_REG: all bits readable
    0x00000000, // DMA_WRITE_ENGINE_EN_OFF (unknown)
    0x00000000, // DMA_READ_ENGINE_EN_OFF (unknown)
    0xFFFFFFFF, // AER_EXT_CAP_HDR_OFF: all fields readable
    0x007027FF, // IATU_REGION_CTRL_1_OFF_OUTBOUND_0: TYPE[4:0],TC[7:5],TD[8],ATTR[10:9],INCREASE[13],FUNC_NUM[22:20]
    0x00000000  // IATU_REGION_CTRL_2_OFF_INBOUND_0 (unknown)
    // Remaining entries default to 0
};

// Write masks (bits allowed to be written; 0 where ro or spec not found)
volatile unsigned int write_mask_array[CNT] = {
    0x00000000, // CAP_ID_NXT_PTR_REG (ro fields)
    0x00000000, // DEVICE_CONTROL_DEVICE_STATUS (unknown)
    0x00000000, // PL_DEBUG1_OFF (unknown)
    0x00000000, // TYPE1_DEV_ID_VEND_ID_REG (unknown)
    0x00000000, // TYPE1_STATUS_COMMAND_REG (unknown)
    0x00000000, // LINK_CONTROL_LINK_STATUS_REG (unknown)
    0x04710000, // PCI_MSI_CAP_ID_NEXT_CTRL_REG: EN[16], MULTI_MSG_EN[22:20], EXT_DATA_EN[26]
    0xFFFFFFFF, // MSI_CAP_OFF_10H_REG: all bits writable
    0x00000000, // DMA_WRITE_ENGINE_EN_OFF (unknown)
    0x00000000, // DMA_READ_ENGINE_EN_OFF (unknown)
    0x00000000, // AER_EXT_CAP_HDR_OFF (ro fields)
    0x007027FF, // IATU_REGION_CTRL_1_OFF_OUTBOUND_0: defined rw fields
    0x00000000  // IATU_REGION_CTRL_2_OFF_INBOUND_0 (unknown)
    // Remaining entries default to 0
};

// Skip array (1 = skip in write/read checks). No explicit skips provided in Meta Arrays; default all 0.
volatile unsigned int skip_array[CNT] = { 0 };
