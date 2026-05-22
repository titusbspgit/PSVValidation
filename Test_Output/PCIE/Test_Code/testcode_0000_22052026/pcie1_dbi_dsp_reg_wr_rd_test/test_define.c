#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "test_define.c"
#include <pcie.h>

// Macros (from Meta Macros)
#define CNT 775
#define SOFT_RST_REG_ADDRESS 0x00000000
#define SOFT_RST_REG_DATA 0x00000000

// Arrays generated from Meta Impacted Registers (addresses)
// RAG mapping consulted for register structure; using macro addresses only.
const unsigned long int addr_array[] = {
    mizar_PCIE1_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG,
    mizar_PCIE1_DBI_DSP_TYPE1_STATUS_COMMAND_REG,
    mizar_PCIE1_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG,
    mizar_PCIE1_DBI_DSP_BAR0_REG,
    mizar_PCIE1_DBI_DSP_CAP_ID_NXT_PTR_REG,
    mizar_PCIE1_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS,
    mizar_PCIE1_DBI_DSP_PL_DEBUG1_OFF
};

// Default values array (no inference; unknown defaults set to 0)
const unsigned int default_value_array[] = {
    0x00000000U, // TYPE1_DEV_ID_VEND_ID_REG default (unknown in Meta)
    0x00000000U, // TYPE1_STATUS_COMMAND_REG default (unknown in Meta)
    0x00000000U, // TYPE1_CLASS_CODE_REV_ID_REG default (unknown in Meta)
    0x00000000U, // BAR0_REG default (unknown in Meta)
    0x00000000U, // CAP_ID_NXT_PTR_REG default (not compared per steps)
    0x00000000U, // DEVICE_CONTROL_DEVICE_STATUS default (excluded from default compare)
    0x00000000U  // PL_DEBUG1_OFF default (excluded from default compare)
};

// Read mask array (no inference; unknown masks set to 0 so logic skips reads)
const unsigned int read_mask_array[] = {
    0x00000000U,
    0x00000000U,
    0x00000000U,
    0x00000000U,
    0x00000000U,
    0x00000000U,
    0x00000000U
};

// Write mask array (no inference; unknown masks set to 0 so logic skips writes)
const unsigned int write_mask_array[] = {
    0x00000000U,
    0x00000000U,
    0x00000000U,
    0x00000000U,
    0x00000000U,
    0x00000000U,
    0x00000000U
};

// Skip array (extracted from Meta Arrays: using first 7 entries)
const int skip_array[] = { 0, 0, 0, 0, 1, 1, 0 };

// Global counters (as used by program.c)
int def_fail_cnt = 0;
int wr_fail_cnt  = 0;
