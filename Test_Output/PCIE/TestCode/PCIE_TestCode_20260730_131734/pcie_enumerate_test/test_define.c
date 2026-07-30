#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Impacted Registers Array (from Meta Impacted Registers) */
static const char* impacted_registers[] = {
    "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF"
};
static const unsigned int impacted_registers_count = sizeof(impacted_registers) / sizeof(impacted_registers[0]);

/* Impacted Register Specs via RAG (Rg-Emb-Mpsoc-Macro-Reg-Spec) */
typedef struct {
    const char* macro;
    unsigned int offset;
    unsigned int width_bits;
} reg_spec_t;

static const reg_spec_t impacted_reg_specs[] = {
    { "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF", 0x1e8U, 32U },
    { "mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF", 0x1e8U, 32U }
};
static const unsigned int impacted_reg_specs_count = sizeof(impacted_reg_specs) / sizeof(impacted_reg_specs[0]);

/* Skip Registers Array (none specified in Meta Arrays) */
static const char* skip_registers[] = { /* empty */ };
static const unsigned int skip_registers_count = sizeof(skip_registers) / sizeof(skip_registers[0]);
