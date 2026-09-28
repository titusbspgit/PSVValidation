// Author - AI Force 2.3. 28-Sep-2026 19:18 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: lpddr4_inline_ecc_1bit_data_modify_test
 * Purpose:   Validates LPDDR4 inline ECC 1-bit corrected error detection
 *            with data modification across multiple ECC regions.
 */

#include "test.h"

/* --------------------------------------------------------------------------
 * Meta TestPlan Headers
 * -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mps_sysreg.h"
#include "aps_sysreg.h"
#include "lpddr4.h"
#include "lpddr4_aps_sii.h"
#include "lpddr4_sii.h"

/* --------------------------------------------------------------------------
 * Macros
 * Note: The following macros are expected to be defined by the included
 *       headers above (mps_sysreg.h, aps_sysreg.h, lpddr4.h, lpddr4_aps_sii.h,
 *       lpddr4_sii.h). They are listed here for reference only.
 *
 *   CTL_INT_NO
 *   PHY_INT_NO
 *   SYSREG_INTR_EN_ADDR
 *   SYSREG_CTRL_INTR_EN_MASK
 *   SYSREG_PHY_INTR_EN_MASK
 *   SYSREG_INTR_MSTS_ADDR
 *   SYSREG_CTRL_INTR_MSTS_MASK
 *   SYSREG_PHY_INTR_MSTS_MASK
 *   SYSREG_INTR_RSTCR_ADDR
 *   SYSREG_CTRL_INTR_RSTCR_MASK
 *   SYSREG_PHY_INTR_RSTCR_MASK
 *   SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK
 *   SII_INTR_EN_SBR_DONE_INTR_MASK
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * ECC Register Offsets (from ctl_base, as specified in test steps)
 * -------------------------------------------------------------------------- */
#define ECCCFG0_OFFSET          0x00000070UL
#define ECCCFG1_OFFSET          0x00000074UL
#define ECCSTAT_OFFSET          0x00000078UL
#define ECCCTL_OFFSET           0x0000007CUL
#define ECCERRCNT_OFFSET        0x00000080UL
#define SWCTL_OFFSET            0x00000320UL
#define PCTRL_0_OFFSET          0x00000490UL
#define PCTRL_1_OFFSET          0x00000540UL
#define SBRCTL_OFFSET           0x00000F24UL
#define SBRRANGE1_OFFSET        0x00000F28UL
#define SBRWDATA0_OFFSET        0x00000F2CUL
#define SBRWDATA1_OFFSET        0x00000F30UL
#define SBRSTART0_OFFSET        0x00000F38UL
#define SBRSTART1_OFFSET        0x00000F3CUL
#define SBRRANGE0_OFFSET        0x00000F40UL
#define SBRRANGE1_ADDR_OFFSET   0x00000F44UL
#define SII_INTR_STATUS_OFFSET  0x00008194UL
#define SII_INTR_EN_OFFSET      0x0000819CUL

/* --------------------------------------------------------------------------
 * Testcase Constants
 * -------------------------------------------------------------------------- */
#define ECC_REGION_MAP_VALUE    0x55U
#define ECC_REGION_COUNT        7U
#define ECC_EXPECTED_ERR_COUNT  5U
#define REGION_OFFSET           0x80000000UL

/* --------------------------------------------------------------------------
 * Arrays
 * -------------------------------------------------------------------------- */
static uint64_t exp_data_array[50];
