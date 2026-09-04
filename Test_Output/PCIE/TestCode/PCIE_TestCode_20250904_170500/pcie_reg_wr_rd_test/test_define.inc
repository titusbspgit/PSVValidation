// Author - AI Force 2.3. 04-Sep-2025 17:05 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: pcie_reg_wr_rd_test
 * Purpose: PCIe register write-read validation for DBI DSP, SII, and PHY registers.
 *          Reset value verification, write-read with 3 check patterns,
 *          SII partial write mask, PHY 16-bit alignment extraction and write mask.
 */

/* Headers from Meta TestPlan */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

/* Number of check patterns (first 3 of 6) */
#define NUM_CHK_PATTERNS             3U

/* SII partial write mask */
#define SII_PARTIAL_WRITE_MASK       0x000F000FU

/* PHY write mask */
#define PHY_WRITE_MASK               0x00001FFFU

/* Check values for write-read verification */
static const unsigned int chk_val[] = {
    0x00000000U,
    0xFFFFFFFFU,
    0x55555555U,
    0xAAAAAAAAU,
    0x5A5A5A5AU,
    0xA5A5A5A5U
};

/* Arrays from Meta TestPlan */
/* PCIE0 DBI DSP controller register arrays */
extern const unsigned int rc0_ctl_addr[];
extern const unsigned int rc0_ctl_rst[];
extern const unsigned int rc0_ctl_mask[];
extern const unsigned int rc0_ctl_addr_size;

/* PCIE1 DBI DSP controller register arrays */
extern const unsigned int rc1_ctl_addr[];
extern const unsigned int rc1_ctl_rst[];
extern const unsigned int rc1_ctl_mask[];
extern const unsigned int rc1_ctl_addr_size;

/* PCIE0 SII register arrays */
extern const unsigned int sii0_addr[];
extern const unsigned int sii0_rst[];
extern const unsigned int sii0_addr_size;

/* PCIE1 SII register arrays */
extern const unsigned int sii1_addr[];
extern const unsigned int sii1_rst[];
extern const unsigned int sii1_addr_size;

/* PCIE0 PHY register arrays */
extern const unsigned int phy0_addr[];
extern const unsigned int phy0_rst[];
extern const unsigned int phy0_addr_size;

/* PCIE1 PHY register arrays */
extern const unsigned int phy1_addr[];
extern const unsigned int phy1_rst[];
extern const unsigned int phy1_addr_size;

/* No conditional compilation macros for this testcase (Macros field = NA) */
/* DEBUG_DISPLAY - defined externally via build configuration */
