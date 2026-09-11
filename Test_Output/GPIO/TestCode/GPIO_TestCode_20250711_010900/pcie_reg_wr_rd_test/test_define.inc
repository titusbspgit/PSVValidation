// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_reg_wr_rd_test
 * Purpose: PCIe register reset default and write-read integrity test
 *   supporting data, arrays, masks, and headers for DBI DSP, SII,
 *   and PHY registers across PCIE0 and PCIE1 controllers.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

/* Array size constants */
#define RC0_CTL_COUNT      5U
#define RC1_CTL_COUNT      5U
#define SII_COUNT          3U
#define PHY_COUNT          3U
#define CHK_VAL_COUNT      6U
#define CHK_VAL_WR_RD_COUNT 3U
#define CHK_VAL_PHY_COUNT  3U

/* PHY reset control value */
#define PHY_RST_CONTROL_VAL 0x01203000U

/* PHY check mask for write-read comparison */
#define PHY_CHK_MASK       0x00001FFFU

/* RC0 DBI DSP controller register addresses */
static const unsigned long rc0_ctl_addr[RC0_CTL_COUNT] = {
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE0_DBI_DSP_UTILITY_OFF
};

/* RC1 DBI DSP controller register addresses */
static const unsigned long rc1_ctl_addr[RC1_CTL_COUNT] = {
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE1_DBI_DSP_UTILITY_OFF
};

/* DBI DSP controller register reset default values */
static const unsigned int ctl_default[RC0_CTL_COUNT] = {
    0x0U, 0x0U, 0x0U, 0x0U, 0x0U
};

/* SII0 register addresses */
static const unsigned long sii0_addr[SII_COUNT] = {
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2,
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,
    mizar_PCIE0_SII_PHY_CONTROL_23
};

/* SII1 register addresses */
static const unsigned long sii1_addr[SII_COUNT] = {
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,
    mizar_PCIE1_SII_PHY_CONTROL_23
};

/* SII register reset default values */
static const unsigned int sii_default[SII_COUNT] = {
    0x0U, 0x0U, 0x0U
};

/* SII0 write masks */
static const unsigned int sii0_write_mask[SII_COUNT] = {
    0xFFFFFFFFU, 0xFFFFFFFFU, 0x000F000FU
};

/* SII1 write masks */
static const unsigned int sii1_write_mask[SII_COUNT] = {
    0xFFFFFFFFU, 0xFFFFFFFFU, 0x000F000FU
};

/* PHY0 register addresses */
static const unsigned long phy0_addr[PHY_COUNT] = {
    0xE68860B8UL, 0xE68862B8UL, 0xE68864B8UL
};

/* PHY1 register addresses */
static const unsigned long phy1_addr[PHY_COUNT] = {
    0xE68A60B8UL, 0xE68A62B8UL, 0xE68A64B8UL
};

/* PHY0 register reset default values */
static const unsigned int phy0_default[PHY_COUNT] = {
    0x0U, 0x0U, 0x0U
};

/* PHY1 register reset default values */
static const unsigned int phy1_default[PHY_COUNT] = {
    0x0U, 0x0U, 0x0U
};

/* PHY0 write masks */
static const unsigned int phy0_write_mask[PHY_COUNT] = {
    0x1FFFU, 0x1FFFU, 0x1FFFU
};

/* PHY1 write masks */
static const unsigned int phy1_write_mask[PHY_COUNT] = {
    0x1FFFU, 0x1FFFU, 0x1FFFU
};

/* Write-read check patterns (full array preserved, first 3 used for wr/rd loop) */
static const unsigned int chk_val[CHK_VAL_COUNT] = {
    0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U,
    0x00000000U, 0xA5A5A5A5U, 0xFFFF0000U
};

/* PHY write-read check patterns */
static const unsigned int chk_val_phy[CHK_VAL_PHY_COUNT] = {
    0x7BAFU, 0x0001U, 0x003BU
};
