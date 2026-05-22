#ifndef TEST_DEFINE_C_INCLUDED
#define TEST_DEFINE_C_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "test_define.c"
#include <pcie.h>

#define CNT 775
#define SOFT_RST_REG_ADDRESS 0x00000000
#define SOFT_RST_REG_DATA 0x00000000

/*
  Register Array generated from Meta Impacted Registers with RAG mapping context.
  Only registers listed in the Meta JSON are used. No additional registers introduced.
  Notes (from RAG mapping):
    - mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG          Offset 0x00, 32b
    - mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG           Offset 0x04, 32b
    - mizar_PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG        Offset 0x08, 32b
    - mizar_PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG Offset 0x0C, 32b
    - mizar_PCIE0_DBI_DSP_BAR0_REG                           Spec Not Found
    - mizar_PCIE0_DBI_DSP_BAR1_REG                           Offset 0x14, 32b
    - mizar_PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG   Spec Not Found
    - mizar_PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG      Spec Not Found
    - mizar_PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG             Spec Not Found
    - mizar_PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG   Spec Not Found
    - mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG                 Offset 0x00 (cap struct), 32b
    - mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS       Spec Not Found
    - mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF                      Spec Not Found
*/

/*
  Arrays sized to CNT per meta macros. Only the provided initial elements are
  populated; remaining indices are zero-initialized by the C language rules.
  This preserves the exact execution sequence and conditional skips in the
  meta procedure without introducing any new registers.
*/
const unsigned long int addr_array[CNT] = {
    mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG,
    mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG,
    mizar_PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG,
    mizar_PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG,
    mizar_PCIE0_DBI_DSP_BAR0_REG,
    mizar_PCIE0_DBI_DSP_BAR1_REG,
    mizar_PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG,
    mizar_PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG,
    mizar_PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG,
    mizar_PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG,
};

const unsigned int default_value_array[CNT] = {
    PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_BAR0_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_BAR1_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG_DEFAULT_VAL,
    PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_DEFAULT_VAL,
};

const unsigned int read_mask_array[CNT] = {
    PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG_READ_MASK,
    PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG_READ_MASK,
    PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG_READ_MASK,
    PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG_READ_MASK,
    PCIE0_DBI_DSP_BAR0_REG_READ_MASK,
    PCIE0_DBI_DSP_BAR1_REG_READ_MASK,
    PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_READ_MASK,
    PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG_READ_MASK,
    PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG_READ_MASK,
    PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_READ_MASK,
};

const unsigned int write_mask_array[CNT] = {
    PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG_WRITE_MASK,
    PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG_WRITE_MASK,
    PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG_WRITE_MASK,
    PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG_WRITE_MASK,
    PCIE0_DBI_DSP_BAR0_REG_WRITE_MASK,
    PCIE0_DBI_DSP_BAR1_REG_WRITE_MASK,
    PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_WRITE_MASK,
    PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG_WRITE_MASK,
    PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG_WRITE_MASK,
    PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_WRITE_MASK,
};

/*
  Skip array extracted from Meta Arrays (skip-related only). Skipped registers
  appear ONLY via this array and are not duplicated elsewhere.
*/
const int skip_array[CNT] = {
    0,0,0,0,1,1,0,0,0,1, 1,1,0,0,1,1,0,0,0,0,
};

/* Write/read check values */
int chk_val[6] = { 0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000 };

#endif /* TEST_DEFINE_C_INCLUDED */
