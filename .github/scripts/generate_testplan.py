#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json
import os
import re
import sys
import zipfile
from datetime import datetime, timedelta, timezone
from collections import OrderedDict
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

# Embedded JSON data (FULL_JSON_STRUCTURE)
json_text = r'''[
  {
    "Index": 1,
    "SS / Module": "PCIE1 SII RC",
    "Feature": "Register default value and masked read/write verification",
    "Test Case Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Test Description": "Verifies register reset defaults and checks masked write and read behavior for the controller’s register set using predefined patterns and masks.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "0xE68C1000",
    "Memory End Offset": "NA",
    "Remarks": "Addresses that are not readable or writable are skipped. A reset control register is excluded from default checks. A skip list controls which registers are not accessed. An optional soft reset sequence is present but not executed.",
    "Test Steps / Procedure": "1) Read each readable register and compare with documented reset defaults. 2) For each data pattern, write to each writable register that is not skipped. 3) Read back each affected register and compare with the expected value computed using read and write masks. 4) Determine the final result based on the absence of mismatches.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1) Each readable register equals its documented default value → Pass. 2) Each write-read cycle produces the expected masked value → Pass. 3) No default or write-read mismatches at the end of the test → Pass.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Hidden_Test_Description": "Test flow: test_case() calls chk_rst_val() then chk_rd_wr(); optional debug prints are guarded by DEBUG_DISPLAY; soft_reset_chk() is present but commented out; final decision: if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0). chk_rst_val(): loops i=0..CNT-1, addr=addr_array[i]; if (read_mask_array[i] == 0x00000000) skip; if (addr_array[i] == mizar_PCIE1_SII_PHY_RST_CONTROL) skip; data_rd = read_reg(addr); if (data_rd == default_value_array[i]) optional PASS print; else def_fail_cnt++ and print failure with expected and read values. chk_rd_wr(): chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; for each pattern j: data_wr=chk_val[j]; write phase: for i=0..CNT-1, if skip_array[i]==1 skip; if write_mask_array[i]==0x00000000 skip; else write_reg(addr,data_wr) with optional debug; read/compare phase: for i=0..CNT-1, if skip_array[i]==1 skip; if write_mask_array[i]==0x00000000 skip; if read_mask_array[i]==0x00000000 skip; else data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) optional PASS print; else wr_fail_cnt++ and print mismatch. soft_reset_chk(): default_value = read_reg(SOFT_RST_REG_ADDRESS); write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA); wait_on(1000); write_reg(SOFT_RST_REG_ADDRESS, default_value); wait_on(1000).",
    "Hidden_Remarks": "Non-readable addresses are skipped; non-writable addresses are skipped; entries flagged in skip_array are skipped; the PHY reset control register is excluded from default checks; soft_reset_chk exists but is not executed; six patterns are used; final result depends on def_fail_cnt and wr_fail_cnt.",
    "Hidden_Test_Steps_Procedure": "int test_case() { chk_rst_val(); chk_rd_wr(); if (def_fail_cnt > 0 || wr_fail_cnt > 0) { finish(1); } else { finish(0); } } void chk_rst_val() { unsigned long int i, addr; for (i = 0; i < CNT; i++) { addr = addr_array[i]; if (read_mask_array[i] == 0x00000000) { continue; } if (addr_array[i] == mizar_PCIE1_SII_PHY_RST_CONTROL) { continue; } data_rd = read_reg(addr); if (data_rd == default_value_array[i]) { /* optional PASS print */ } else { def_fail_cnt++; /* print failure with expected and read */ } } } void chk_rd_wr() { unsigned long int i, addr, j, exp_val, wr_n; int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}; for (j = 0; j < 6; j++) { data_wr = chk_val[j]; for (i = 0; i < CNT; i++) { addr = addr_array[i]; if (skip_array[i] == 1) { continue; } if (write_mask_array[i] == 0x00000000) { continue; } else { write_reg(addr, data_wr); } } for (i = 0; i < CNT; i++) { addr = addr_array[i]; if (skip_array[i] == 1) { continue; } if (write_mask_array[i] == 0x00000000) { continue; } if (read_mask_array[i] == 0x00000000) { continue; } else { data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) { /* optional PASS print */ } else { wr_fail_cnt++; /* print mismatch */ } } } } } void soft_reset_chk() { int default_value; default_value = read_reg(SOFT_RST_REG_ADDRESS); write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA); wait_on(1000); write_reg(SOFT_RST_REG_ADDRESS, default_value); wait_on(1000); }",
    "Hidden_Impacted_Registers": "mizar_PCIE1_SII_CFG_BAR0_START1,mizar_PCIE1_SII_CFG_BAR0_START2,mizar_PCIE1_SII_CFG_BAR0_LIMIT1,mizar_PCIE1_SII_CFG_BAR0_LIMIT2,mizar_PCIE1_SII_CFG_BAR1_START,mizar_PCIE1_SII_CFG_BAR1_LIMIT1,mizar_PCIE1_SII_CFG_BAR2_START1,mizar_PCIE1_SII_CFG_BAR2_START2,mizar_PCIE1_SII_CFG_BAR2_LIMIT1,mizar_PCIE1_SII_CFG_BAR2_LIMIT2,mizar_PCIE1_SII_CFG_BAR3_START,mizar_PCIE1_SII_CFG_BAR3_LIMIT,mizar_PCIE1_SII_CFG_BAR4_START1,mizar_PCIE1_SII_CFG_BAR4_START2,mizar_PCIE1_SII_CFG_BAR4_LIMIT1,mizar_PCIE1_SII_CFG_BAR4_LIMIT2,mizar_PCIE1_SII_CFG_BAR5_START,mizar_PCIE1_SII_CFG_BAR5_LIMIT,mizar_PCIE1_SII_PCIE1_CONFIG_INFO1,mizar_PCIE1_SII_PCIE1_CONFIG_INFO2,mizar_PCIE1_SII_PCIE1_GEN_CONTROL1,mizar_PCIE1_SII_PCIE1_GEN_CONTROL2,mizar_PCIE1_SII_PCIE1_GEN_CONTROL3,mizar_PCIE1_SII_PCIE1_PM_CONTROL,mizar_PCIE1_SII_PCIE1_CONTROL_PM_STS,mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER1,mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER4,mizar_PCIE1_SII_PCIE1_TRANSMIT_REQ,mizar_PCIE1_SII_PCIE1_RCV_MSG_HDR1,mizar_PCIE1_SII_PCIE1_RCV_MSG_HDR2,mizar_PCIE1_SII_PCIE1_RCV_MSG_HDR3,mizar_PCIE1_SII_PCIE1_RCV_MSG_HDR4,mizar_PCIE1_SII_PCIE1_RCV_MSG_STS,mizar_PCIE1_SII_RCV_INTERRPUT_CTRL,mizar_PCIE1_SII_CFG_EXP_ROM_START,mizar_PCIE1_SII_CFG_EXP_ROM_LIMIT,mizar_PCIE1_SII_CFG_EXP_ROM_INFO,mizar_PCIE1_SII_CXPL_DEBUG_INFO1,mizar_PCIE1_SII_CXPL_DEBUG_INFO2,mizar_PCIE1_SII_CXPL_DEBUG_INFO_EI,mizar_PCIE1_SII_PCIE1_TARGET_INFO1,mizar_PCIE1_SII_PCIE1_TARGET_INFO2,mizar_PCIE1_SII_PCIE1_CONTOLLER_ERROR_STATUS,mizar_PCIE1_SII_PCIE1_CONTROLLER_INT_STS,mizar_PCIE1_SII_PCIE1_CONTROLLER_INTERRUPT_CONTROL,mizar_PCIE1_SII_PHY_RST_CONTROL,mizar_PCIE1_SII_LINK_DEBUG_DATA,mizar_PCIE1_SII_PCIE1_ERR_STS,mizar_PCIE1_SII_PCIE1_ERR_INTERRUPT_CTRL,mizar_PCIE1_SII_CFG_MSI_INT,mizar_PCIE1_SII_LTR_MSG,mizar_PCIE1_SII_LTR_MSG_LATENCY,mizar_PCIE1_SII_APP_LTR_LATENCY,mizar_PCIE1_SII_CFG_LTR_MAX_LATENCY,mizar_PCIE1_SII_OBFF_CNTRL,mizar_PCIE1_SII_SLV_AWMISC_INFO,mizar_PCIE1_SII_SLV_AWMISC_INFO_HDR_34DW_HI,mizar_PCIE1_SII_SLV_AWMISC_INFO_HDR_34DW_LO,mizar_PCIE1_SII_SLV_MISC_INFO,mizar_PCIE1_SII_SLV_MISC_RESP_INFO,mizar_PCIE1_SII_MSTR_AWMISC_INFO_CNTRL,mizar_PCIE1_SII_MSTR_AWMISC_INFO_1,mizar_PCIE1_SII_MSTR_AWMISC_INFO_0,mizar_PCIE1_SII_MSTR_AWMISC_INFO_HDR_34DW_HI,mizar_PCIE1_SII_MSTR_AWMISC_INFO_HDR_34DW_LO,mizar_PCIE1_SII_MSTR_ARMISC_INFO_CNTRL,mizar_PCIE1_SII_MSTR_ARMISC_INFO_1,mizar_PCIE1_SII_MSTR_ARMISC_INFO_0,mizar_PCIE1_SII_MSTR_BMISC_RMISC_CPL_STAT_INFO,mizar_PCIE1_SII_RADM_TIMEOUT_INFO,mizar_PCIE1_SII_CFG_MSI_INFO,mizar_PCIE1_SII_CFG_MSI_DATA,mizar_PCIE1_SII_CFG_MSI_ADDR_HI,mizar_PCIE1_SII_CFG_MSI_ADDR_LO,mizar_PCIE1_SII_CFG_AER_INT_AND_PCIE1_CAP_INT_MSG,mizar_PCIE1_SII_RTLH_RFC_DATA,mizar_PCIE1_SII_APP_HDR_INFO,mizar_PCIE1_SII_APP_HDR_LOG_3,mizar_PCIE1_SII_APP_HDR_LOG_2,mizar_PCIE1_SII_APP_HDR_LOG_1,mizar_PCIE1_SII_APP_HDR_LOG_0,mizar_PCIE1_SII_CFG_BUS_NUM,mizar_PCIE1_SII_CFG_BR_CTRL_SERREN,mizar_PCIE1_SII_APP_DEV_AND_BUS_NUM,mizar_PCIE1_SII_PCIE1_CONTROLLER_INT_STS_1,mizar_PCIE1_SII_PCIE1_CONTROLLER_INTERRUPT_CONTROL_1,mizar_PCIE1_SII_APP_AND_SLOT_CONTROL_REG,mizar_PCIE1_SII_DIAG_CTRL_BUS,mizar_PCIE1_SII_CFG_REG_RO,mizar_PCIE1_SII_CFG_ARI_FWD_EN,mizar_PCIE1_SII_RADM_SLOT_PWR_PAYLOAD,mizar_PCIE1_SII_DIAG_STATUS_BUS_0,mizar_PCIE1_SII_DIAG_STATUS_BUS_1,mizar_PCIE1_SII_DIAG_STATUS_BUS_2,mizar_PCIE1_SII_DIAG_STATUS_BUS_3,mizar_PCIE1_SII_DIAG_STATUS_BUS_4,mizar_PCIE1_SII_DIAG_STATUS_BUS_5,mizar_PCIE1_SII_DIAG_STATUS_BUS_6,mizar_PCIE1_SII_DIAG_STATUS_BUS_7,mizar_PCIE1_SII_DIAG_STATUS_BUS_8,mizar_PCIE1_SII_DIAG_STATUS_BUS_9,mizar_PCIE1_SII_DIAG_STATUS_BUS_10,mizar_PCIE1_SII_DIAG_STATUS_BUS_11,mizar_PCIE1_SII_DIAG_STATUS_BUS_12,mizar_PCIE1_SII_DIAG_STATUS_BUS_13,mizar_PCIE1_SII_DIAG_STATUS_BUS_14,mizar_PCIE1_SII_DIAG_STATUS_BUS_15,mizar_PCIE1_SII_DIAG_STATUS_BUS_16,mizar_PCIE1_SII_DIAG_STATUS_BUS_17,mizar_PCIE1_SII_DIAG_STATUS_BUS_18,mizar_PCIE1_SII_DIAG_STATUS_BUS_19,mizar_PCIE1_SII_RAM_PWR_CNTRL_0,mizar_PCIE1_SII_RAM_PWR_CNTRL_1,mizar_PCIE1_SII_SOFT_RESET_CTRL,mizar_PCIE1_SII_CFG_MSI_PENDING_B,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_1,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_2,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_3,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_4,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_5,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_6,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_7,mizar_PCIE1_SII_PHY_CONTROL_0,mizar_PCIE1_SII_PHY_CONTROL_1,mizar_PCIE1_SII_PHY_CONTROL_2,mizar_PCIE1_SII_PHY_CONTROL_3,mizar_PCIE1_SII_PHY_CONTROL_4,mizar_PCIE1_SII_PHY_CONTROL_5,mizar_PCIE1_SII_PHY_CONTROL_6,mizar_PCIE1_SII_PHY_CONTROL_7,mizar_PCIE1_SII_PHY_CONTROL_8,mizar_PCIE1_SII_PHY_CONTROL_9,mizar_PCIE1_SII_PHY_CONTROL_10,mizar_PCIE1_SII_PHY_CONTROL_11,mizar_PCIE1_SII_PHY_CONTROL_12,mizar_PCIE1_SII_PHY_CONTROL_13,mizar_PCIE1_SII_PHY_CONTROL_14,mizar_PCIE1_SII_PHY_CONTROL_15,mizar_PCIE1_SII_PHY_CONTROL_16,mizar_PCIE1_SII_PHY_CONTROL_17,mizar_PCIE1_SII_PHY_CONTROL_18,mizar_PCIE1_SII_PHY_CONTROL_19,mizar_PCIE1_SII_PHY_CONTROL_20,mizar_PCIE1_SII_PHY_CONTROL_21,mizar_PCIE1_SII_PHY_CONTROL_22,mizar_PCIE1_SII_PHY_CONTROL_23,mizar_PCIE1_SII_PHY_CONTROL_24,mizar_PCIE1_SII_PHY_CONTROL_25,mizar_PCIE1_SII_PHY_CONTROL_26,mizar_PCIE1_SII_MSI_CTRL_IO,mizar_PCIE1_SII_MSI_CTRL_INT_VEC",
    "Hidden_Validation_Acceptance_Criteria": "Default check: if (read_mask_array[i] == 0x00000000) skip; if (addr_array[i] == mizar_PCIE1_SII_PHY_RST_CONTROL) skip; else read_reg(addr) and compare to default_value_array[i]; match → pass; mismatch → def_fail_cnt++ and print failure. Write/read check (for each of six patterns): if (skip_array[i] == 1) skip; if (write_mask_array[i] == 0x00000000) skip; if (read_mask_array[i] == 0x00000000) skip; else read_reg(addr) and compute wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) pass; else wr_fail_cnt++ and print mismatch. Final: if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0).",
    "Hidden_Header_Includes": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"test_define.c\"\n#include<pcie1/pcie_sii_rc_def.h>\n#include<pcie1/pcie_sii_rc_offset.h>",
    "Hidden_Macro_Defines": "#define SOFT_RST_REG_ADDRESS\t0x00000000\n#define SOFT_RST_REG_DATA\t0x00000000\n#define MIZAR_PCIE1_SII_BASE     0xE68C1000\n#define CNT 153",
    "Hidden_Skip_Array_Definition": "const int skip_array[153]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}"
  },
  {
    "Index": 2,
    "SS / Module": "PCIE",
    "Feature": "PCI Express Capability",
    "Test Case Name": "pcie_cfg_wr_rd_test",
    "Test Description": "Configures coherency and exercises configuration space access to base address and command registers after ensuring interface readiness.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "0xE6004100",
    "Memory End Offset": "NA",
    "Remarks": "Execution depends on selected instance flags. Readiness is confirmed before configuration. Completion is indicated by a control marker. No explicit comparisons are performed on base address reads.",
    "Test Steps / Procedure": "1) Initialize the control register. \n2) Perform link setup for the selected instance. \n3) Program coherency control fields in the DBI coherency control register. \n4) Wait until the SII status registers indicate ready. \n5) Program memory base and exercise base address registers. \n6) Enable memory, I/O, and bus mastering in the command register. \n7) Wait until the control indicator shows completion.",
    "Impacted Registers": "DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "Validation / Acceptance Criteria": "1) SII status indicates ready for all required instances → Proceed. \n2) Control indicator shows completion → Pass.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie_cfg_wr_rd_test",
    "Hidden_Test_Description": "Initializes control 0xE6004100 to 0x0; performs instance-dependent link training; programs DBI coherency control fields for both controllers; polls SII status with mask 0xD1 at offset 0xC0 until ready; writes 0x11111111 to control; waits; performs memory base programming; reads first 10 configuration dwords; writes BARs at 0x10..0x24 with 0xFFFFFFFF and reads back; writes BARs with 0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000 and reads back; enables memory, I/O, bus master by writing 0x7 to offset 0x4; finally polls 0xE6004100 until 0x12345678 and calls finish(0).",
    "Hidden_Remarks": "Execution path depends on DM0_RC, DM1_RC, DM0_EP, DM1_EP. SII readiness is checked with mask 0xD1 at offset 0xC0. A non_secure_prot_nic() call is made. The test uses prints for readback but does not assert BAR values. Completion is detected by 0xE6004100 reaching 0x12345678.",
    "Hidden_Test_Steps_Procedure": "1) write_reg(0xE6004100, 0x0). \n2) If DM0_RC: link_training_dm0_x4(4). If DM1_RC: link_training_dm1_x4(4). If DM0_EP: link_training_dm0_x4(4). If DM1_EP: link_training_dm1_x4(4). \n3) rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF); write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1). \n4) rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF); write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1). \n5) rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF); write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1). \n6) rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF); write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1). \n7) wait_on(20); recompute and write combined fields for both PCIE0 and PCIE1 DBI coherency control 3 registers. \n8) data_rd = read_sii0_reg(0xC0); while (((data_rd) & 0xD1) != 0xD1) { data_rd = read_sii0_reg(0xC0); }. \n9) If DM1_RC: data_rd = read_sii1_reg(0xC0); while (((data_rd) & 0xD1) != 0xD1) { data_rd = read_sii1_reg(0xC0); }. \n10) write_reg(0xE6004100, 0x11111111). \n11) wait_on(15000). \n12) If DM0_RC: mem_base_program_dm0_x4(); wait_on(10); for (i=0; i<10; i++) { rd_wr_data1 = read_pcie_slv0_reg(i*0x4); } write_pcie_slv0_reg(0x10,0xFFFFFFFF); 0x14=0xFFFFFFFF; 0x18=0xFFFFFFFF; 0x1C=0xFFFFFFFF; 0x20=0xFFFFFFFF; 0x24=0xFFFFFFFF; read back 0x10..0x24; write 0x10=0x0, 0x14=0x4, 0x18=0x20000000, 0x1C=0x40000000, 0x20=0x60000000, 0x24=0x80000000; read back 0x10..0x24; write_pcie_slv0_reg(0x4, 0x7). \n13) If DM1_RC: mem_base_program_dm1_x4(); for (i=0; i<10; i++) { rd_wr_data1 = read_pcie_slv1_reg(i*0x4); } write_pcie_slv1_reg(0x4, 0x7); write 0x10..0x24 = 0xFFFFFFFF; read back 0x10..0x24; write 0x10=0x0, 0x14=0x4, 0x18=0x20000000, 0x1C=0x40000000, 0x20=0x60000000, 0x24=0x80000000; read back 0x10..0x24. \n14) wait_on(10); data_rd = read_reg(0xE6004100); while (data_rd != 0x12345678) { wait_on(5); data_rd = read_reg(0xE6004100); } finish(0).",
    "Hidden_Impacted_Registers": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "Hidden_Validation_Acceptance_Criteria": "SII0 readiness: loop exits only when ((read_sii0_reg(0xC0) & 0xD1) == 0xD1). If DM1_RC, SII1 readiness: loop exits only when ((read_sii1_reg(0xC0) & 0xD1) == 0xD1). Final condition: loop exits only when read_reg(0xE6004100) == 0x12345678. Upon exiting final loop, finish(0) is called indicating pass.",
    "Hidden_Header_Includes": "#include <stdlib.h>\n#include <stdio.h>\n#include <test_common.h>\n#include \"pcie.h\"",
    "Hidden_Macro_Defines": "NA",
    "Hidden_Skip_Array_Definition": "NA"
  }
]'''

try:
    data = json.loads(json_text)
except Exception as e:
    print(f"JSON parse error: {e}")
    sys.exit(2)

if not isinstance(data, list) or len(data) == 0:
    print("Empty or invalid JSON array")
    sys.exit(2)

# Derive key order (union, first-seen order)
key_order = []
seen = set()
for rec in data:
    if not isinstance(rec, dict):
        print("Invalid record (not an object)")
        sys.exit(2)
    for k in rec.keys():
        if k not in seen:
            seen.add(k)
            key_order.append(k)

# Create workbook and staging sheet 'Data'
wb = Workbook()
ws = wb.active
ws.title = 'Data'

# Write header
for col, key in enumerate(key_order, start=1):
    c = ws.cell(row=1, column=col, value=key)
    c.font = Font(bold=True)

ws.freeze_panes = 'A2'

# Write rows
for r, rec in enumerate(data, start=2):
    for col, key in enumerate(key_order, start=1):
        ws.cell(row=r, column=col, value=rec.get(key, ""))

# Helper to compute best-fit column widths (deterministic approximation)
def autofit_columns(sheet):
    max_width = {}
    for row in sheet.iter_rows(values_only=True):
        for idx, val in enumerate(row, start=1):
            s = '' if val is None else str(val)
            w = min(max(len(s), 3), 80)
            max_width[idx] = max(max_width.get(idx, 0), w)
    for idx, w in max_width.items():
        sheet.column_dimensions[get_column_letter(idx)].width = w + 2

autofit_columns(ws)

# Create META sheet and copy META columns as-is (only those present)
meta_candidates = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
    'Hidden_Header_Includes',
    'Hidden_Macro_Define',  # include if present
    'Hidden_Macro_Defines', # include if present
    'Hidden_Skip_Array_Definition'
]
meta_keys = [k for k in meta_candidates if any(isinstance(rec, dict) and k in rec for rec in data)]
ws_meta = wb.create_sheet('Meta_data_sheet')
for col, key in enumerate(meta_keys, start=1):
    ws_meta.cell(row=1, column=col, value=key).font = Font(bold=True)
for r, rec in enumerate(data, start=2):
    for col, key in enumerate(meta_keys, start=1):
        ws_meta.cell(row=r, column=col, value=rec.get(key, ""))
ws_meta.sheet_state = 'veryHidden'

# Rename 'Data' to 'TestPlan' and reorganize columns
ws.title = 'TestPlan'

main_cols = [
    'Index',
    'SS / Module',
    'Feature',
    'Test Case Name',
    'Test Description',
    'Speed',
    'Mode',
    'Memory Start Offset',
    'Memory End Offset',
    'Remarks',
    'Test Steps / Procedure',
    'Impacted Registers',
    'Validation / Acceptance Criteria',
    'Code Generation (Required / Not)'
]

# Build new sheet content with only main columns, preserving values exactly (blank if missing)
rows = []
for rec in data:
    row = [rec.get(col, "") for col in main_cols]
    rows.append(row)

# Clear existing cells
ws.delete_rows(1, ws.max_row)

# Write main headers
for col, key in enumerate(main_cols, start=1):
    cell = ws.cell(row=1, column=col, value=key)
    cell.font = Font(bold=True)
    cell.alignment = Alignment(horizontal='center', vertical='center')
    cell.fill = PatternFill('solid', fgColor='4472C4')

# Numbering normalization function
num_pat = re.compile(r'^\s*\d+[\.)]\s*')

def renumber_lines(text):
    if text is None:
        return ""
    s = str(text)
    lines = [ln for ln in (ln.strip() for ln in s.splitlines()) if ln]
    if not lines:
        return s
    out = []
    n = 1
    for ln in lines:
        ln = num_pat.sub('', ln)
        out.append(f"{n}. {ln}")
        n += 1
    return "\n".join(out)

# Write data rows, with numbering enforced in two columns
for r, row in enumerate(rows, start=2):
    # Map columns
    values = dict(zip(main_cols, row))
    if 'Test Steps / Procedure' in values and values['Test Steps / Procedure']:
        values['Test Steps / Procedure'] = renumber_lines(values['Test Steps / Procedure'])
    if 'Validation / Acceptance Criteria' in values and values['Validation / Acceptance Criteria']:
        values['Validation / Acceptance Criteria'] = renumber_lines(values['Validation / Acceptance Criteria'])
    for c, key in enumerate(main_cols, start=1):
        ws.cell(row=r, column=c, value=values.get(key, ""))

# Formatting: wrap, alignment, borders, autofit
wrap_cols = {'Test Description', 'Remarks', 'Test Steps / Procedure', 'Validation / Acceptance Criteria'}
header_font = Font(bold=True, color='FFFFFF')
header_fill = PatternFill('solid', fgColor='4472C4')
thin = Side(border_style='thin', color='000000')
border = Border(left=thin, right=thin, top=thin, bottom=thin)

# Apply header formatting again (with white font) and borders
for c, key in enumerate(main_cols, start=1):
    cell = ws.cell(row=1, column=c)
    cell.font = header_font
    cell.alignment = Alignment(horizontal='center', vertical='center')
    cell.fill = header_fill
    cell.border = border

# Determine column indices
col_index = {key: idx for idx, key in enumerate(main_cols, start=1)}

# Apply data formatting
max_row = ws.max_row
max_col = ws.max_column
for r in range(2, max_row + 1):
    for c in range(1, max_col + 1):
        cell = ws.cell(row=r, column=c)
        header = main_cols[c-1]
        if header in wrap_cols:
            cell.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
        elif header == 'Index':
            cell.alignment = Alignment(vertical='top', horizontal='center')
        else:
            cell.alignment = Alignment(vertical='top', horizontal='left')
        cell.border = border

# Approximate autofit columns based on content length
autofit_columns(ws)

# Approximate row height based on wrapped columns content
for r in range(2, max_row + 1):
    max_lines = 1
    for name in wrap_cols:
        c = col_index.get(name)
        if c:
            val = ws.cell(row=r, column=c).value
            if val:
                lines = str(val).count('\n') + 1
                if lines > max_lines:
                    max_lines = lines
    ws.row_dimensions[r].height = max(15, min(15 * max_lines, 409))

# Data validation: only for 'Code Generation (Required / Not)'
if 'Code Generation (Required / Not)' in col_index:
    c = col_index['Code Generation (Required / Not)']
    dv = DataValidation(type='list', formula1='"Required, Blank, Not Required"', allow_blank=True, showDropDown=True)
    ws.add_data_validation(dv)
    rng = f"{get_column_letter(c)}2:{get_column_letter(c)}{max_row}"
    dv.add(rng)

# Safety check: ensure no sheet named 'Data'
if 'Data' in [s.title for s in wb.worksheets]:
    # attempt to delete; if not possible, fail
    try:
        del wb['Data']
    except Exception:
        print('Validation fail: Data sheet still present')
        sys.exit(3)

# Compute IST timestamp and output path
ist = timezone(timedelta(hours=5, minutes=30))
ts = datetime.now(ist).strftime('%Y%m%d_%H%M%S')
filename = f"PCIE_TestPlan_{ts}.xlsx"
out_dir = os.path.join('Test_Output', 'PCIE', 'TestPlan')
os.makedirs(out_dir, exist_ok=True)
output_path = os.path.join(out_dir, filename)

# Save workbook
wb.save(output_path)

# Validate: is zip-based OOXML and loadable
if not zipfile.is_zipfile(output_path):
    print('XLSX validation failed: not a ZIP-based file')
    sys.exit(4)
try:
    _ = load_workbook(output_path, read_only=True)
except Exception as e:
    print(f'XLSX load validation failed: {e}')
    sys.exit(5)

print(output_path)
sys.exit(0)
