// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

// Meta Headers
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mps_sysreg.h"
#include "aps_sysreg.h"
#include "lpddr4.h"
#include "lpddr4_aps_sii.h"
#include "lpddr4_sii.h"

// Required framework/project headers for register access and logging
#include <stdint.h>
#include "log.h"
#include "reg_access.h"
#include "gic_funcs.h"

// Meta Macros
#define SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_UNCORRECTED_ERR_INTR_FAULT
#define SII_INTR_EN_SBR_DONE_INTR_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR

// Conditional compilation macros (expected to be defined externally via build configuration):
// APS_DRAM - controls port address and base address selection, and selects APS-specific SII macro variants
// SG2667 - selects speed grade 2667
// SG2133 - selects speed grade 2133
// A53_INITIATOR - referenced in port access guard
// MPS_DRAM - referenced in port access guard
// AI_INITIATOR - referenced in port access guard
// DSP_INITIATOR - referenced in port access guard

// Meta Arrays: exp_data_array[50] declared in source file (file-scope static)
