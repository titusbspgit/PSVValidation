#include <stdlib.h>;
#include <stdio.h>;
#include <test_common.h>;
#include "pcie.h"

/* Impacted Registers Array (from Meta + RAG mapping) */
const unsigned long pcie_coherency_regs[2] = {
    mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,
    mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
};
const unsigned int pcie_coherency_regs_count = 2u;

/* Skip Registers Array (from Meta Arrays: NA) */
const unsigned long skip_registers[1] = { 0u }; /* No skipped registers */
const unsigned int skip_registers_count = 0u;
