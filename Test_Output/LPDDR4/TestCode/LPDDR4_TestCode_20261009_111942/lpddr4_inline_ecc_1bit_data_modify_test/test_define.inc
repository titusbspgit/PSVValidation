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

// Meta Macros
#define SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_CORRECTED_ERR_INTR_FAULT
#define SII_INTR_EN_SBR_DONE_INTR_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR

// Conditional compilation macros: APS_DRAM, SG2667, SG2133, A53_INITIATOR,
// MPS_DRAM, AI_INITIATOR, DSP_INITIATOR are expected to be defined externally
// via build system -D flags.

// Meta Arrays: exp_data_array[50] - declared in source file as global
// Do not redeclare here.

// Deterministic 64-bit traffic data replacing rand() calls
// 7 elements used for 7 regions (region 0 to 6)
static const unsigned long int ecc_1bit_traffic_data[7] = {
 0xA5A5A5A500000001UL,
 0x5A5A5A5A11111111UL,
 0x1234567890ABCDEFUL,
 0xFEDCBA9876543210UL,
 0x0F0F0F0FF0F0F0F0UL,
 0xAAAAAAAA55555555UL,
 0xDEADC0DE0BADF00DUL
};

// Global variables extern declarations
extern unsigned long int port0_addr;
extern unsigned long int port1_addr;
extern unsigned int int_expected;
extern unsigned int eccstat;
extern unsigned int count;
extern unsigned int region;
extern unsigned int ecc_region_map;
extern unsigned int sbr_init;
extern unsigned int err0;
extern unsigned int int_pend1;
extern unsigned int int_pend;
extern unsigned int num_slverr_taken;

// exp_data_array: unsigned long int exp_data_array[50]
// Declared as global in source file, extern here
extern unsigned long int exp_data_array[50];

// MANUAL_REVIEW: ctl_base, phy_base, bus_width, SG, DBI_EN, DM_EN, ECC_EN
// are expected to be declared in lpddr4.h or test_common.h as global variables
// accessible by the LPDDR4 training and configuration infrastructure.
// If not declared there, uncomment and provide declarations below:
// extern unsigned long int ctl_base;
// extern unsigned long int phy_base;
// extern unsigned int bus_width;
// extern unsigned int SG;
// extern unsigned int DBI_EN;
// extern unsigned int DM_EN;
// extern unsigned int ECC_EN;
