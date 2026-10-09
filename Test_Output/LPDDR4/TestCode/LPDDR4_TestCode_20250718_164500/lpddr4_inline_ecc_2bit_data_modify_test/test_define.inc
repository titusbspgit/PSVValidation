// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mps_sysreg.h"
#include "aps_sysreg.h"
#include "lpddr4.h"
#include "lpddr4_aps_sii.h"
#include "lpddr4_sii.h"
#include "log.h"
#include "mmio.h"
#include "reg_access.h"
#include "gic_funcs.h"

/ Meta Macros /
#define SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_UNCORRECTED_ERR_INTR_FAULT
#define SII_INTR_EN_SBR_DONE_INTR_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR

/ Conditional compilation macros: APS_DRAM, SG2667, SG2133, A53_INITIATOR, /
/ MPS_DRAM, AI_INITIATOR, DSP_INITIATOR are expected to be defined externally /
/ via build configuration or project-level headers. /

/ Meta Arrays */
unsigned long int exp_data_array[50];
