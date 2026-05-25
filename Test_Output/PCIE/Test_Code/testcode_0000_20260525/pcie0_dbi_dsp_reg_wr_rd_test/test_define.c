#include <stdio.h>
#include <stdlib.h>
"test_common.h"
"test_define.c"
<pcie.h>

#define CNT 775
#define SOFT_RST_REG_ADDRESS 0x00000000
#define SOFT_RST_REG_DATA 0x00000000

/* Impacted Registers Address Array (from Meta Impacted Registers) */
const unsigned long int addr_array[13] = {
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
    mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG,
    mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS,
    mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF
};

/* Default/Mask arrays - sizes aligned to addr_array length as required by procedure */
const unsigned int default_value_array[13] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0
};

const unsigned int read_mask_array[13] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0
};

const unsigned int write_mask_array[13] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* Skip array extracted from Meta Arrays (skip-related arrays only) */
const int skip_array[20]={0,0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,0,0,0,0,};
