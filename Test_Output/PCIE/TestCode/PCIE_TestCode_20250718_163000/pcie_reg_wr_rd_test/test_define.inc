/ Author - AI Force 2.3. Date in IST /
/ (EMBENGG-SYSAPPS) /

/
 * Test Case Name: pcie_reg_wr_rd_test
 * Purpose: Register reset-value verification and register write-read verification
 * for PCIe controller registers across three register groups
 * (RC controller, SII, PHY).
 /

/ Headers /
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

/ Macros: NA - No macros provided for this testcase /

/ Arrays /
/
 * Note: Array contents (register addresses, default values, write masks, check values)
 * must be populated with actual hardware-specific values from the platform configuration.
 * The exact element values are not specified in the input Meta TestPlan JSON.
 /

/
 * RC0 controller register addresses
 * Registers: mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG,
 * mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,
 * mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,
 * mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
 * mizar_PCIE0_DBI_DSP_UTILITY_OFF
 /
unsigned int rc0_ctl_addr[5] = {
 mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG,
 mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,
 mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,
 mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
 mizar_PCIE0_DBI_DSP_UTILITY_OFF
};

/
 * RC1 controller register addresses
 * Registers: mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG,
 * mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,
 * mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,
 * mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
 * mizar_PCIE1_DBI_DSP_UTILITY_OFF
 /
unsigned int rc1_ctl_addr[5] = {
 mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG,
 mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,
 mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,
 mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
 mizar_PCIE1_DBI_DSP_UTILITY_OFF
};

/ Controller register default/reset values /
/ Note: Exact reset values must be populated from hardware specification /
unsigned int ctl_default[5] = {0};

/
 * SII0 register addresses
 * Registers: mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2,
 * mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,
 * mizar_PCIE0_SII_PHY_CONTROL_23
 /
unsigned int sii0_addr[3] = {
 mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2,
 mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,
 mizar_PCIE0_SII_PHY_CONTROL_23
};

/
 * SII1 register addresses
 * Registers: mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,
 * mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,
 * mizar_PCIE1_SII_PHY_CONTROL_23
 /
unsigned int sii1_addr[3] = {
 mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,
 mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,
 mizar_PCIE1_SII_PHY_CONTROL_23
};

/ SII register default/reset values /
/ Note: Exact reset values must be populated from hardware specification /
unsigned int sii_default[3] = {0};

/ SII0 write masks /
/ Note: Exact write mask values must be populated from hardware specification /
unsigned int sii0_write_mask[3] = {0};

/ SII1 write masks /
/ Note: Exact write mask values must be populated from hardware specification /
unsigned int sii1_write_mask[3] = {0};

/
 * PHY0 register addresses
 * Registers: mizar_PCIE0_SII_PHY_RST_CONTROL, 0xE68860B8, 0xE68862B8
 /
unsigned int phy0_addr[3] = {
 mizar_PCIE0_SII_PHY_RST_CONTROL,
 0xE68860B8,
 0xE68862B8
};

/
 * PHY1 register addresses
 * Registers: mizar_PCIE1_SII_PHY_RST_CONTROL, 0xE68A60B8, 0xE68A62B8
 /
unsigned int phy1_addr[3] = {
 mizar_PCIE1_SII_PHY_RST_CONTROL,
 0xE68A60B8,
 0xE68A62B8
};

/ PHY0 register default/reset values /
/ Note: Exact reset values must be populated from hardware specification /
unsigned int phy0_default[3] = {0};

/ PHY1 register default/reset values /
/ Note: Exact reset values must be populated from hardware specification /
unsigned int phy1_default[3] = {0};

/ PHY0 write masks /
/ Note: Exact write mask values must be populated from hardware specification /
unsigned int phy0_write_mask[3] = {0};

/ PHY1 write masks /
/ Note: Exact write mask values must be populated from hardware specification /
unsigned int phy1_write_mask[3] = {0};

/ Check values for controller register write-read verification /
/ Note: Exact check values must be populated from hardware specification /
unsigned int chk_val[6] = {0};

/ Check values for PHY register write-read verification /
/ Note: Exact check values must be populated from hardware specification */
unsigned int chk_val_phy[3] = {0};
