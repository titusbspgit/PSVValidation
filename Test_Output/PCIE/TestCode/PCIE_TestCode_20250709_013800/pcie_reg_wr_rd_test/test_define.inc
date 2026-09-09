// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "test.h"
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

#define RC_CTL_COUNT            5U
#define SII_COUNT               3U
#define PHY_COUNT               3U
#define CHK_VAL_WR_RD_COUNT     3U
#define CHK_VAL_TOTAL           6U
#define CHK_VAL_PHY_COUNT       3U
#define PHY_RST_RELEASE_VAL     0x01203000UL
#define PHY_CHK_MASK            0x00001FFFU

static const unsigned int rc0_ctl_addr[RC_CTL_COUNT] = {
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE0_DBI_DSP_UTILITY_OFF
};
static const unsigned int rc1_ctl_addr[RC_CTL_COUNT] = {
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE1_DBI_DSP_UTILITY_OFF
};
static const unsigned int ctl_default[RC_CTL_COUNT] = { 0x0, 0x0, 0x0, 0x0, 0x0 };
static const unsigned int sii0_addr[SII_COUNT] = {
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2,
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,
    mizar_PCIE0_SII_PHY_CONTROL_23
};
static const unsigned int sii1_addr[SII_COUNT] = {
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,
    mizar_PCIE1_SII_PHY_CONTROL_23
};
static const unsigned int sii_default[SII_COUNT] = { 0x0, 0x0, 0x0 };
static const unsigned int sii0_write_mask[SII_COUNT] = { 0xFFFFFFFF, 0xFFFFFFFF, 0x000F000F };
static const unsigned int sii1_write_mask[SII_COUNT] = { 0xFFFFFFFF, 0xFFFFFFFF, 0x000F000F };
static const unsigned int phy0_addr[PHY_COUNT] = { 0xE68860B8, 0xE68862B8, 0xE68864B8 };
static const unsigned int phy1_addr[PHY_COUNT] = { 0xE68A60B8, 0xE68A62B8, 0xE68A64B8 };
static const unsigned int phy0_default[PHY_COUNT] = { 0x0, 0x0, 0x0 };
static const unsigned int phy1_default[PHY_COUNT] = { 0x0, 0x0, 0x0 };
static const unsigned int phy0_write_mask[PHY_COUNT] = { 0x1FFF, 0x1FFF, 0x1FFF };
static const unsigned int phy1_write_mask[PHY_COUNT] = { 0x1FFF, 0x1FFF, 0x1FFF };
static const unsigned int chk_val[CHK_VAL_TOTAL] = { 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xFFFF0000 };
static const unsigned int chk_val_phy[CHK_VAL_PHY_COUNT] = { 0x7BAF, 0x0001, 0x003B };
