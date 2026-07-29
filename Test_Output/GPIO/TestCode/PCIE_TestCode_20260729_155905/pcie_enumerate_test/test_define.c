#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Impacted Registers Array derived from Meta Impacted Registers */
const unsigned long int reg_addr_array[2] = {
    mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,
    mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
};

/* Skip array (no registers skipped) */
const int reg_skip_array[2] = { 0, 0 };
