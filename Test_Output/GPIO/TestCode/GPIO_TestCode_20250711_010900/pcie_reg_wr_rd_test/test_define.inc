// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_reg_wr_rd_test
 * Purpose: PCIe register write and read test supporting data, macros, and headers.
 *   Validates reset default values and write-read integrity for DBI DSP,
 *   SII, and PHY registers across PCIE0 and PCIE1 controllers.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

/* PHY reset release value */
#define PHY_RST_RELEASE_VAL        0x01203000U

/* PHY 13-bit write mask for comparison */
#define PHY_13BIT_MASK             0x00001FFFU

/* Array size constants */
#define RC_CTL_ADDR_COUNT          5U
#define SII_ADDR_COUNT             3U
#define PHY_ADDR_COUNT             3U
#define CHK_VAL_PATTERN_COUNT      3U

/* PCIE0 DBI DSP register addresses */
static const unsigned long rc0_ctl_addr[RC_CTL_ADDR_COUNT] = {
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE0_DBI_DSP_UTILITY_OFF
};

/* PCIE1 DBI DSP register addresses */
static const unsigned long rc1_ctl_addr[RC_CTL_ADDR_COUNT] = {
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE1_DBI_DSP_UTILITY_OFF
};

/* DBI DSP register reset default values */
static const unsigned int ctl_default[RC_CTL_ADDR_COUNT] = {
    0x0U, 0x0U, 0x0U, 0x0U, 0x0U
};

/* PCIE0 SII register addresses */
static const unsigned long sii0_addr[SII_ADDR_COUNT] = {
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2,
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,
    mizar_PCIE0_SII_PHY_CONTROL_23
};

/* PCIE1 SII register addresses */
static const unsigned long sii1_addr[SII_ADDR_COUNT] = {
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,
    mizar_PCIE1_SII_PHY_CONTROL_23
};

/* SII register reset default values */
static const unsigned int sii_default[SII_ADDR_COUNT] = {
    0x0U, 0x0U, 0x0U
};

/* PCIE0 SII register write masks */
static const unsigned int sii0_write_mask[SII_ADDR_COUNT] = {
    0xFFFFFFFFU, 0xFFFFFFFFU, 0x000F000FU
};

/* PCIE1 SII register write masks */
static const unsigned int sii1_write_mask[SII_ADDR_COUNT] = {
    0xFFFFFFFFU, 0xFFFFFFFFU, 0x000F000FU
};

/* PCIE0 PHY register addresses */
static const unsigned long phy0_addr[PHY_ADDR_COUNT] = {
    0xE68860B8UL, 0xE68862B8UL, 0xE68864B8UL
};

/* PCIE1 PHY register addresses */
static const unsigned long phy1_addr[PHY_ADDR_COUNT] = {
    0xE68A60B8UL, 0xE68A62B8UL, 0xE68A64B8UL
};

/* PCIE0 PHY register reset default values */
static const unsigned int phy0_default[PHY_ADDR_COUNT] = {
    0x0U, 0x0U, 0x0U
};

/* PCIE1 PHY register reset default values */
static const unsigned int phy1_default[PHY_ADDR_COUNT] = {
    0x0U, 0x0U, 0x0U
};

/* PCIE0 PHY register write masks */
static const unsigned int phy0_write_mask[PHY_ADDR_COUNT] = {
    0x1FFFU, 0x1FFFU, 0x1FFFU
};

/* PCIE1 PHY register write masks */
static const unsigned int phy1_write_mask[PHY_ADDR_COUNT] = {
    0x1FFFU, 0x1FFFU, 0x1FFFU
};

/* DBI DSP and SII write-read test patterns (first 3 of 6 used) */
static const unsigned int chk_val[6] = {
    0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xFFFF0000U
};

/* PHY-specific write-read test patterns */
static const unsigned int chk_val_phy[CHK_VAL_PATTERN_COUNT] = {
    0x7BAFU, 0x1U, 0x003BU
};
