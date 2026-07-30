#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

/* Global error counters and working variables */
int err1 = 0;  /* Error bucket 1 */
int err2 = 0;  /* Error bucket 2 */
int i = 0;     /* Loop index i */
int j = 0;     /* Loop index j */
volatile unsigned int data_rd = 0; /* Read data scratch */

/* Arrays from Meta Arrays (UNCHANGED) */
unsigned int rc0_ctl_addr[5] = {mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,mizar_PCIE0_DBI_DSP_UTILITY_OFF};
unsigned int rc1_ctl_addr[5] = {mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,mizar_PCIE1_DBI_DSP_UTILITY_OFF};
unsigned int ctl_default[5] = {0x0, 0x0, 0x0, 0x0, 0x0};
unsigned int sii0_addr[3] = {mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,mizar_PCIE0_SII_PHY_CONTROL_23};
unsigned int sii1_addr[3] = {mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2, mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,mizar_PCIE1_SII_PHY_CONTROL_23};
unsigned int sii_default[3] = {0x0, 0x0,0x0};
unsigned int sii0_write_mask[3] = {0xFFFFFFFF,0xFFFFFFFF,0xF000F};
unsigned int sii1_write_mask[3] = {0xFFFFFFFF,0xFFFFFFFF,0xF000F};
unsigned int phy0_addr[3] = {0xE68860B8,0xE68862B8,0xE68864B8};
unsigned int phy1_addr[3] ={0xE68A60B8,0xE68A62B8,0xE68A64B8};
unsigned int phy0_default[3] = {0x0,0x0,0x0};
unsigned int phy1_default[3] = {0x0,0x0,0x0};
unsigned int phy0_write_mask[3] = {0x1FFF,0x1FFF,0x1FFF};
unsigned int phy1_write_mask[3] = {0x1FFF,0x1FFF,0x1FFF};

/* Forward declarations per template structure */
int test_case(void);
static void chk_rst_val(void);
static void chk_rd_wr(void);
