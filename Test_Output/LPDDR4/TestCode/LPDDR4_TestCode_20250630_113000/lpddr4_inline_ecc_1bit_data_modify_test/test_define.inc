// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

/* Meta Headers */
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mps_sysreg.h"
#include "aps_sysreg.h"
#include "lpddr4.h"
#include "lpddr4_aps_sii.h"
#include "lpddr4_sii.h"

/* ---------------------------------------------------------------------------
 * Meta Macros: Platform-conditional definitions
 * --------------------------------------------------------------------------- */
#ifdef APS_DRAM
#define CTL_INT_NO                              APS_LPDDR4_CTL_INT_NO
#define PHY_INT_NO                              APS_LPDDR4_PHY_INT_NO
#define SYSREG_INTR_EN_ADDR                     MIZAR_APS_SYSREG_INTR_EN
#define SYSREG_CTRL_INTR_EN_MASK                APS_SYSREG_INTR_EN_DDR_CTRL_INTR
#define SYSREG_PHY_INTR_EN_MASK                 APS_SYSREG_INTR_EN_DDRPHY_DWC_DDRPHY_INT_N
#define SYSREG_INTR_MSTS_ADDR                   MIZAR_APS_SYSREG_INTR_MSTS
#define SYSREG_CTRL_INTR_MSTS_MASK              APS_SYSREG_INTR_MSTS_DDR_CTRL_INTR
#define SYSREG_PHY_INTR_MSTS_MASK               APS_SYSREG_INTR_MSTS_DDRPHY_DWC_DDRPHY_INT_N
#define SYSREG_INTR_RSTCR_ADDR                  MIZAR_APS_SYSREG_INTR_RSTCR
#define SYSREG_CTRL_INTR_RSTCR_MASK             APS_SYSREG_INTR_RSTCR_DDR_CTRL_INTR
#define SYSREG_PHY_INTR_RSTCR_MASK              APS_SYSREG_INTR_RSTCR_DDRPHY_DWC_DDRPHY_INT_N
#define SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_APS_SII_DDR_CNTRL_INTR_EN_DDR_ECC_CORRECTED_ERR_INTR_FAULT
#define SII_INTR_EN_SBR_DONE_INTR_MASK          LPDDR4_APS_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR
#else
#define CTL_INT_NO                              MPS_LPDDR4_CTL_INT_NO
#define PHY_INT_NO                              MPS_LPDDR4_PHY_INT_NO
#define SYSREG_INTR_EN_ADDR                     MIZAR_MPS_SYSREG_INTR_EN
#define SYSREG_CTRL_INTR_EN_MASK                MPS_SYSREG_INTR_EN_DDR_CTRL_INTR
#define SYSREG_PHY_INTR_EN_MASK                 MPS_SYSREG_INTR_EN_DDRPHY_DWC_DDRPHY_INT_N
#define SYSREG_INTR_MSTS_ADDR                   MIZAR_MPS_SYSREG_INTR_MSTS
#define SYSREG_CTRL_INTR_MSTS_MASK              MPS_SYSREG_INTR_MSTS_DDR_CTRL_INTR
#define SYSREG_PHY_INTR_MSTS_MASK               MPS_SYSREG_INTR_MSTS_DDRPHY_DWC_DDRPHY_INT_N
#define SYSREG_INTR_RSTCR_ADDR                  MIZAR_MPS_SYSREG_INTR_RSTCR
#define SYSREG_CTRL_INTR_RSTCR_MASK             MPS_SYSREG_INTR_RSTCR_DDR_CTRL_INTR
#define SYSREG_PHY_INTR_RSTCR_MASK              MPS_SYSREG_INTR_RSTCR_DDRPHY_DWC_DDRPHY_INT_N
#define SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_CORRECTED_ERR_INTR_FAULT
#define SII_INTR_EN_SBR_DONE_INTR_MASK          LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR
#endif
