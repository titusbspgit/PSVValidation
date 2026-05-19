#include <stdlib.h>;
#include <stdio.h>;
#include <test_common.h>;
#include "pcie.h"

/* Impacted Registers Array (from Meta Impacted Registers) */
static const unsigned int impacted_registers[] = {
    mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,
    mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
};
static const unsigned int impacted_registers_count = 2u;

/* Skip Registers Array (from Meta Arrays: none provided) */
static const unsigned int skip_registers[] = { };
static const unsigned int skip_registers_count = 0u;
