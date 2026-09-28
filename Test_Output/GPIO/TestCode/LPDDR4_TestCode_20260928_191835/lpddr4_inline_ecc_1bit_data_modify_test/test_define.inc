// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/
 * Test Case: lpddr4_inline_ecc_1bit_data_modify_test
 * Purpose: Validates LPDDR4 inline ECC 1-bit corrected error detection with
 * data modification across multiple ECC regions. Configures ECC with
 * ecc_region_map=0x55, initializes memory via scrubber, writes random
 * data, reads back and checks for ECC corrected errors and interrupt
 * behavior across ECC-enabled and non-ECC regions.
 /

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mps_sysreg.h"
#include "aps_sysreg.h"
#include "lpddr4.h"
#include "lpddr4_aps_sii.h"
#include "lpddr4_sii.h"

/
 * Macros from Meta TestPlan JSON.
 * These macros are expected to be defined by the included platform headers.
 * If not defined, they must be provided by the SoC/IP configuration.
 /

#ifndef CTL_INT_NO
// MANUAL_REVIEW: CTL_INT_NO must be defined by platform headers.
#endif

#ifndef PHY_INT_NO
// MANUAL_REVIEW: PHY_INT_NO must be defined by platform headers.
#endif

#ifndef SYSREG_INTR_EN_ADDR
// MANUAL_REVIEW: SYSREG_INTR_EN_ADDR must be defined by platform headers.
#endif

#ifndef SYSREG_CTRL_INTR_EN_MASK
// MANUAL_REVIEW: SYSREG_CTRL_INTR_EN_MASK must be defined by platform headers.
#endif

#ifndef SYSREG_PHY_INTR_EN_MASK
// MANUAL_REVIEW: SYSREG_PHY_INTR_EN_MASK must be defined by platform headers.
#endif

#ifndef SYSREG_INTR_MSTS_ADDR
// MANUAL_REVIEW: SYSREG_INTR_MSTS_ADDR must be defined by platform headers.
#endif

#ifndef SYSREG_CTRL_INTR_MSTS_MASK
// MANUAL_REVIEW: SYSREG_CTRL_INTR_MSTS_MASK must be defined by platform headers.
#endif

#ifndef SYSREG_PHY_INTR_MSTS_MASK
// MANUAL_REVIEW: SYSREG_PHY_INTR_MSTS_MASK must be defined by platform headers.
#endif

#ifndef SYSREG_INTR_RSTCR_ADDR
// MANUAL_REVIEW: SYSREG_INTR_RSTCR_ADDR must be defined by platform headers.
#endif

#ifndef SYSREG_CTRL_INTR_RSTCR_MASK
// MANUAL_REVIEW: SYSREG_CTRL_INTR_RSTCR_MASK must be defined by platform headers.
#endif

#ifndef SYSREG_PHY_INTR_RSTCR_MASK
// MANUAL_REVIEW: SYSREG_PHY_INTR_RSTCR_MASK must be defined by platform headers.
#endif

#ifndef SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK
// MANUAL_REVIEW: SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK must be defined by platform headers.
#endif

#ifndef SII_INTR_EN_SBR_DONE_INTR_MASK
// MANUAL_REVIEW: SII_INTR_EN_SBR_DONE_INTR_MASK must be defined by platform headers.
#endif

/ Arrays */
static uint64_t exp_data_array[50];
