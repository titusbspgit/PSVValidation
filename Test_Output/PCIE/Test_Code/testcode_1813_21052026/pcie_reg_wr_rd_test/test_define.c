#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

/* Arrays defined exactly as provided in Meta Arrays */
unsigned int rc0_ctl_addr[5] = {mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H REG, mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H REG, mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH OFF, mizar_PCIE0_DBI_DSP_UTILITY OFF};
unsigned int rc1_ctl_addr[5] = {mizar_PCIE1_DBI_DSP_MSI_CAP OFF_08H REG, mizar_PCIE1_DBI_DSP_MSI_CAP OFF_10H REG, mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH OFF, mizar_PCIE1_DBI_DSP_UTILITY OFF};
unsigned int ctl_default[5] = {0x0, 0x0, 0x0, 0x0, 0x0};
unsigned int sii0_addr[3] = {mizar_PCIE0_SII_PCIE0_TRANSMIT HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT HEADER3, mizar_PCIE0_SII_PHY CONTROL 23};
unsigned int sii1_addr[3] = {mizar_PCIE1_SII_PCIE1_TRANSMIT HEADER2, mizar_PCIE1_SII_PCIE1_TRANSMIT HEADER3, mizar_PCIE1_SII_PHY CONTROL 23};
unsigned int sii_default[3] = {0x0, 0x0, 0x0};
unsigned int sii0_write_mask[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0x000F000F};
unsigned int sii1_write_mask[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0x000F000F};
unsigned int phy0_addr[3] = {0xE68860B8, 0xE68862B8, 0xE68864B8};
unsigned int phy1_addr[3] = {0xE68A60B8, 0xE68A62B8, 0xE68A64B8};
unsigned int phy0_default[3] = {0x0, 0x0, 0x0};
unsigned int phy1_default[3] = {0x0, 0x0, 0x0};
unsigned int phy0_write_mask[3] = {0x1FFF, 0x1FFF, 0x1FFF};
unsigned int phy1_write_mask[3] = {0x1FFF, 0x1FFF, 0x1FFF};
int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};
int chk_val_phy[3] = {0x7baf, 0x1, 0x003b};
