import json
import os
import re
from datetime import datetime
from zoneinfo import ZoneInfo
from zipfile import ZipFile
from io import BytesIO
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Embedded FULL_JSON_STRUCTURE (do not edit values)
FULL_JSON = r'''[
  {
    "Index": 1,
    "SS / Module": "PCIE1 SII RC",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Test Description": "Checks default values for PCIE1 SII RC registers and verifies write/read behavior using masks and skip logic across all registers listed in addr_array. Uses data patterns, read/write masks, and default values to compute expected results and determines pass/fail based on mismatches.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Addresses with zero read mask are skipped for default-value reads. One reset-control register is excluded from default-value checking. Addresses flagged in skip_array are skipped for write/read testing. Addresses with zero write mask are skipped for writes and for subsequent read verification. Failure counters (def_fail_cnt, wr_fail_cnt) accumulate mismatches and the test ends with finish(1) on any failure, otherwise finish(0).",
    "Test Steps / Procedure": "Default value check:\n- Loop i=0..CNT-1; addr = addr_array[i]. If read_mask_array[i] == 0x00000000, skip read. If addr_array[i] == mizar_PCIE1_SII_PHY_RST_CONTROL, skip read. Else data_rd = read_reg(addr). If data_rd == default_value_array[i], log PASS else increment def_fail_cnt and log failure.\n\nWrite then read check for six patterns 0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000:\n- For each pattern data_wr:\n  - Write phase: Loop i=0..CNT-1; addr = addr_array[i]. If skip_array[i] == 1, continue. If write_mask_array[i] == 0x00000000, continue. Else write_reg(addr, data_wr).\n  - Read/verify phase: Loop i=0..CNT-1; addr = addr_array[i]. If skip_array[i] == 1, continue. If write_mask_array[i] == 0x00000000, continue. If read_mask_array[i] == 0x00000000, continue. Else data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); If data_rd == exp_val, log PASS else increment wr_fail_cnt and log failure.\n\nTest end:\n- If (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0).",
    "Impacted Registers": "SII_CFG_BAR0_START1\nSII_CFG_BAR0_START2\nSII_CFG_BAR0_LIMIT1\nSII_CFG_BAR0_LIMIT2\nSII_CFG_BAR1_START\nSII_CFG_BAR1_LIMIT1\nSII_PHY_RST_CONTROL\nSII_PCIE1_CONTROLLER_INT_STS\nSII_PCIE1_CONTROLLER_INTERRUPT_CONTROL\nSII_SOFT_RESET_CTRL\nSII_PHY_CONTROL_0\nSII_PHY_CONTROL_1",
    "Validation / Acceptance Criteria": "Default value check: For each i, if read_mask_array[i] != 0x00000000 and addr_array[i] != mizar_PCIE1_SII_PHY_RST_CONTROL, PASS when read_reg(addr_array[i]) == default_value_array[i]; else increment def_fail_cnt.\nWrite/read check: For each pattern and for each i with skip_array[i] == 0, write_mask_array[i] != 0x00000000, and read_mask_array[i] != 0x00000000, expected exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])). PASS when read_reg(addr_array[i]) == exp_val; else increment wr_fail_cnt.\nOverall PASS: def_fail_cnt == 0 and wr_fail_cnt == 0 leading to finish(0). Overall FAIL: any failure increments cause finish(1).",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Hidden_Test_Description": "Checks default values for PCIE1 SII RC registers and verifies write/read behavior using masks and skip logic across all registers listed in addr_array. Uses data patterns, read/write masks, and default values to compute expected results and determines pass/fail based on mismatches.",
    "Hidden_Remarks": "Addresses with zero read mask are skipped for default-value reads. One reset-control register is excluded from default-value checking. Addresses flagged in skip_array are skipped for write/read testing. Addresses with zero write mask are skipped for writes and for subsequent read verification. Failure counters (def_fail_cnt, wr_fail_cnt) accumulate mismatches and the test ends with finish(1) on any failure, otherwise finish(0).",
    "Hidden_Test_Steps_Procedure": "Default value check:\n- Loop i=0..CNT-1; addr = addr_array[i]. If read_mask_array[i] == 0x00000000, skip read. If addr_array[i] == mizar_PCIE1_SII_PHY_RST_CONTROL, skip read. Else data_rd = read_reg(addr). If data_rd == default_value_array[i], log PASS else increment def_fail_cnt and log failure.\n\nWrite then read check for six patterns 0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000:\n- For each pattern data_wr:\n  - Write phase: Loop i=0..CNT-1; addr = addr_array[i]. If skip_array[i] == 1, continue. If write_mask_array[i] == 0x00000000, continue. Else write_reg(addr, data_wr).\n  - Read/verify phase: Loop i=0..CNT-1; addr = addr_array[i]. If skip_array[i] == 1, continue. If write_mask_array[i] == 0x00000000, continue. If read_mask_array[i] == 0x00000000, continue. Else data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); If data_rd == exp_val, log PASS else increment wr_fail_cnt and log failure.\n\nTest end:\n- If (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0).",
    "Hidden_Impacted_Registers": "mizar_PCIE1_SII_CFG_BAR0_START1,mizar_PCIE1_SII_CFG_BAR0_START2,mizar_PCIE1_SII_CFG_BAR0_LIMIT1,mizar_PCIE1_SII_CFG_BAR0_LIMIT2,mizar_PCIE1_SII_CFG_BAR1_START,mizar_PCIE1_SII_CFG_BAR1_LIMIT1,mizar_PCIE1_SII_CFG_BAR2_START1,mizar_PCIE1_SII_CFG_BAR2_START2,mizar_PCIE1_SII_CFG_BAR2_LIMIT1,mizar_PCIE1_SII_CFG_BAR2_LIMIT2,mizar_PCIE1_SII_CFG_BAR3_START,mizar_PCIE1_SII_CFG_BAR3_LIMIT,mizar_PCIE1_SII_CFG_BAR4_START1,mizar_PCIE1_SII_CFG_BAR4_START2,mizar_PCIE1_SII_CFG_BAR4_LIMIT1,mizar_PCIE1_SII_CFG_BAR4_LIMIT2,mizar_PCIE1_SII_CFG_BAR5_START,mizar_PCIE1_SII_CFG_BAR5_LIMIT,mizar_PCIE1_SII_PCIE1_CONFIG_INFO1,mizar_PCIE1_SII_PCIE1_CONFIG_INFO2,mizar_PCIE1_SII_PCIE1_GEN_CONTROL1,mizar_PCIE1_SII_PCIE1_GEN_CONTROL2,mizar_PCIE1_SII_PCIE1_GEN_CONTROL3,mizar_PCIE1_SII_PCIE1_PM_CONTROL,mizar_PCIE1_SII_PCIE1_CONTROL_PM_STS,mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER1,mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER4,mizar_PCIE1_SII_PCIE1_TRANSMIT_REQ,mizar_PCIE1_SII_PCIE1_RCV_MSG_HDR1,mizar_PCIE1_SII_PCIE1_RCV_MSG_HDR2,mizar_PCIE1_SII_PCIE1_RCV_MSG_HDR3,mizar_PCIE1_SII_PCIE1_RCV_MSG_HDR4,mizar_PCIE1_SII_PCIE1_RCV_MSG_STS,mizar_PCIE1_SII_RCV_INTERRPUT_CTRL,mizar_PCIE1_SII_CFG_EXP_ROM_START,mizar_PCIE1_SII_CFG_EXP_ROM_LIMIT,mizar_PCIE1_SII_CFG_EXP_ROM_INFO,mizar_PCIE1_SII_CXPL_DEBUG_INFO1,mizar_PCIE1_SII_CXPL_DEBUG_INFO2,mizar_PCIE1_SII_CXPL_DEBUG_INFO_EI,mizar_PCIE1_SII_PCIE1_TARGET_INFO1,mizar_PCIE1_SII_PCIE1_TARGET_INFO2,mizar_PCIE1_SII_PCIE1_CONTOLLER_ERROR_STATUS,mizar_PCIE1_SII_PCIE1_CONTROLLER_INT_STS,mizar_PCIE1_SII_PCIE1_CONTROLLER_INTERRUPT_CONTROL,mizar_PCIE1_SII_PHY_RST_CONTROL,mizar_PCIE1_SII_LINK_DEBUG_DATA,mizar_PCIE1_SII_PCIE1_ERR_STS,mizar_PCIE1_SII_PCIE1_ERR_INTERRUPT_CTRL,mizar_PCIE1_SII_CFG_MSI_INT,mizar_PCIE1_SII_LTR_MSG,mizar_PCIE1_SII_LTR_MSG_LATENCY,mizar_PCIE1_SII_APP_LTR_LATENCY,mizar_PCIE1_SII_CFG_LTR_MAX_LATENCY,mizar_PCIE1_SII_OBFF_CNTRL,mizar_PCIE1_SII_SLV_AWMISC_INFO,mizar_PCIE1_SII_SLV_AWMISC_INFO_HDR_34DW_HI,mizar_PCIE1_SII_SLV_AWMISC_INFO_HDR_34DW_LO,mizar_PCIE1_SII_SLV_MISC_INFO,mizar_PCIE1_SII_SLV_MISC_RESP_INFO,mizar_PCIE1_SII_MSTR_AWMISC_INFO_CNTRL,mizar_PCIE1_SII_MSTR_AWMISC_INFO_1,mizar_PCIE1_SII_MSTR_AWMISC_INFO_0,mizar_PCIE1_SII_MSTR_AWMISC_INFO_HDR_34DW_HI,mizar_PCIE1_SII_MSTR_AWMISC_INFO_HDR_34DW_LO,mizar_PCIE1_SII_MSTR_ARMISC_INFO_CNTRL,mizar_PCIE1_SII_MSTR_ARMISC_INFO_1,mizar_PCIE1_SII_MSTR_ARMISC_INFO_0,mizar_PCIE1_SII_MSTR_BMISC_RMISC_CPL_STAT_INFO,mizar_PCIE1_SII_RADM_TIMEOUT_INFO,mizar_PCIE1_SII_CFG_MSI_INFO,mizar_PCIE1_SII_CFG_MSI_DATA,mizar_PCIE1_SII_CFG_MSI_ADDR_HI,mizar_PCIE1_SII_CFG_MSI_ADDR_LO,mizar_PCIE1_SII_CFG_AER_INT_AND_PCIE1_CAP_INT_MSG,mizar_PCIE1_SII_RTLH_RFC_DATA,mizar_PCIE1_SII_APP_HDR_INFO,mizar_PCIE1_SII_APP_HDR_LOG_3,mizar_PCIE1_SII_APP_HDR_LOG_2,mizar_PCIE1_SII_APP_HDR_LOG_1,mizar_PCIE1_SII_APP_HDR_LOG_0,mizar_PCIE1_SII_CFG_BUS_NUM,mizar_PCIE1_SII_CFG_BR_CTRL_SERREN,mizar_PCIE1_SII_APP_DEV_AND_BUS_NUM,mizar_PCIE1_SII_PCIE1_CONTROLLER_INT_STS_1,mizar_PCIE1_SII_PCIE1_CONTROLLER_INTERRUPT_CONTROL_1,mizar_PCIE1_SII_APP_AND_SLOT_CONTROL_REG,mizar_PCIE1_SII_DIAG_CTRL_BUS,mizar_PCIE1_SII_CFG_REG_RO,mizar_PCIE1_SII_CFG_ARI_FWD_EN,mizar_PCIE1_SII_RADM_SLOT_PWR_PAYLOAD,mizar_PCIE1_SII_DIAG_STATUS_BUS_0,mizar_PCIE1_SII_DIAG_STATUS_BUS_1,mizar_PCIE1_SII_DIAG_STATUS_BUS_2,mizar_PCIE1_SII_DIAG_STATUS_BUS_3,mizar_PCIE1_SII_DIAG_STATUS_BUS_4,mizar_PCIE1_SII_DIAG_STATUS_BUS_5,mizar_PCIE1_SII_DIAG_STATUS_BUS_6,mizar_PCIE1_SII_DIAG_STATUS_BUS_7,mizar_PCIE1_SII_DIAG_STATUS_BUS_8,mizar_PCIE1_SII_DIAG_STATUS_BUS_9,mizar_PCIE1_SII_DIAG_STATUS_BUS_10,mizar_PCIE1_SII_DIAG_STATUS_BUS_11,mizar_PCIE1_SII_DIAG_STATUS_BUS_12,mizar_PCIE1_SII_DIAG_STATUS_BUS_13,mizar_PCIE1_SII_DIAG_STATUS_BUS_14,mizar_PCIE1_SII_DIAG_STATUS_BUS_15,mizar_PCIE1_SII_DIAG_STATUS_BUS_16,mizar_PCIE1_SII_DIAG_STATUS_BUS_17,mizar_PCIE1_SII_DIAG_STATUS_BUS_18,mizar_PCIE1_SII_DIAG_STATUS_BUS_19,mizar_PCIE1_SII_RAM_PWR_CNTRL_0,mizar_PCIE1_SII_RAM_PWR_CNTRL_1,mizar_PCIE1_SII_SOFT_RESET_CTRL,mizar_PCIE1_SII_CFG_MSI_PENDING_B,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_1,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_2,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_3,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_4,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_5,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_6,mizar_PCIE1_SII_SMLH_LTSSM_STATE_TRAN_7,mizar_PCIE1_SII_PHY_CONTROL_0,mizar_PCIE1_SII_PHY_CONTROL_1,mizar_PCIE1_SII_PHY_CONTROL_2,mizar_PCIE1_SII_PHY_CONTROL_3,mizar_PCIE1_SII_PHY_CONTROL_4,mizar_PCIE1_SII_PHY_CONTROL_5,mizar_PCIE1_SII_PHY_CONTROL_6,mizar_PCIE1_SII_PHY_CONTROL_7,mizar_PCIE1_SII_PHY_CONTROL_8,mizar_PCIE1_SII_PHY_CONTROL_9,mizar_PCIE1_SII_PHY_CONTROL_10,mizar_PCIE1_SII_PHY_CONTROL_11,mizar_PCIE1_SII_PHY_CONTROL_12,mizar_PCIE1_SII_PHY_CONTROL_13,mizar_PCIE1_SII_PHY_CONTROL_14,mizar_PCIE1_SII_PHY_CONTROL_15,mizar_PCIE1_SII_PHY_CONTROL_16,mizar_PCIE1_SII_PHY_CONTROL_17,mizar_PCIE1_SII_PHY_CONTROL_18,mizar_PCIE1_SII_PHY_CONTROL_19,mizar_PCIE1_SII_PHY_CONTROL_20,mizar_PCIE1_SII_PHY_CONTROL_21,mizar_PCIE1_SII_PHY_CONTROL_22,mizar_PCIE1_SII_PHY_CONTROL_23,mizar_PCIE1_SII_PHY_CONTROL_24,mizar_PCIE1_SII_PHY_CONTROL_25,mizar_PCIE1_SII_PHY_CONTROL_26,mizar_PCIE1_SII_MSI_CTRL_IO,mizar_PCIE1_SII_MSI_CTRL_INT_VEC,",
    "Hidden_Validation_Acceptance_Criteria": "Default value check: For each i, if read_mask_array[i] != 0x00000000 and addr_array[i] != mizar_PCIE1_SII_PHY_RST_CONTROL, PASS when read_reg(addr_array[i]) == default_value_array[i]; else increment def_fail_cnt.\nWrite/read check: For each pattern and for each i with skip_array[i] == 0, write_mask_array[i] != 0x00000000, and read_mask_array[i] != 0x00000000, expected exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])). PASS when read_reg(addr_array[i]) == exp_val; else increment wr_fail_cnt.\nOverall PASS: def_fail_cnt == 0 and wr_fail_cnt == 0 leading to finish(0). Overall FAIL: any failure increments cause finish(1).",
    "Hidden_Header_Includes": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"test_define.c\"\n#include<pcie1/pcie_sii_rc_def.h>\n#include<pcie1/pcie_sii_rc_offset.h>",
    "Hidden_Macro_Defines": "#define SOFT_RST_REG_ADDRESS\t0x00000000\n#define SOFT_RST_REG_DATA\t0x00000000\n#define MIZAR_PCIE1_SII_BASE     0xE68C1000\n#define CNT 153",
    "Hidden_Skip_Array_Definition": "const int skip_array[153]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,;}"
  },
  {
    "Index": 2,
    "SS / Module": "PCIE",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie_cfg_wr_rd_test",
    "Test Description": "Verifies PCIe configuration access by training the link, programming coherency settings, and performing write/read checks on configuration registers. The test uses polling to wait for status readiness and completes when the final status condition is met.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Compile-time flags select the controller instance and role. The test waits for status readiness before configuration transactions.",
    "Test Steps / Procedure": "1) Train the link for the selected instance and role.\n2) Update the coherency control register fields.\n3) Wait briefly and update the coherency control register fields again.\n4) Poll the status register until ready.\n5) Program memory base and read initial configuration words.\n6) Write configuration BAR entries, read them back, then program target values and read back again.\n7) Enable memory, IO, and bus master in the command register.\n8) Poll the completion flag and finish when ready.",
    "Impacted Registers": "DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "Validation / Acceptance Criteria": "1) Status readiness check → The status register indicates ready.\n2) Configuration BAR access check → Written values are read back as expected.\n3) Final completion check → The completion flag indicates done.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie_cfg_wr_rd_test",
    "Hidden_Test_Description": "Train link (conditional on DM0_RC/DM1_RC/DM0_EP/DM1_EP), program coherency control fields for both instances via set_data() on mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF and mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, poll SII status at offset 0xC0 until (data_rd & 0xD1) == 0xD1 (for SII0 and conditionally SII1), write 0xE6004100 with 0x11111111, perform memory base programming and configuration space write/read sequences (BARs at 0x10..0x24) for the selected instance, enable memory/IO/bus master, then poll 0xE6004100 until 0x12345678 and finish(0).",
    "Hidden_Remarks": "Compile-time defines (DM0_RC, DM1_RC, DM0_EP, DM1_EP) determine which link training and configuration paths execute. The test uses wait_on delays and busy-wait polling for SII status and a final control/status register value before completing.",
    "Hidden_Test_Steps_Procedure": "Initialization:\n- write_reg(0xE6004100, 0x0).\n\nLink training (one of the following based on compile-time defines):\n- If DM0_RC: link_training_dm0_x4(4).\n- If DM1_RC: link_training_dm1_x4(4).\n- If DM0_EP: link_training_dm0_x4(4).\n- If DM1_EP: link_training_dm1_x4(4).\n\nCache/coherency programming:\n- rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF); write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1).\n- rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF); write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1).\n- rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF); write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1).\n- rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF); write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1).\n- wait_on(20).\n- For PCIE0: rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xF); set 3..6, 27..30, 19..22; write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1).\n- For PCIE1: rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xF); set 3..6, 27..30, 19..22; write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1).\n\nSII status polling:\n- data_rd = read_sii0_reg(0xC0); while (((data_rd) & 0xD1) != 0xD1) { data_rd = read_sii0_reg(0xC0); }.\n- If DM1_RC: data_rd = read_sii1_reg(0xC0); while (((data_rd) & 0xD1) != 0xD1) { data_rd = read_sii1_reg(0xC0); }.\n\nControl register sequence:\n- write_reg(0xE6004100, 0x11111111).\n- wait_on(15000).\n\nIf DM0_RC path:\n- mem_base_program_dm0_x4(); wait_on(10).\n- For i = 0..9: rd_wr_data1 = read_pcie_slv0_reg(i*0x4).\n- First write set: write_pcie_slv0_reg(0x10, 0xFFFFFFFF); 0x14, 0xFFFFFFFF; 0x18, 0xFFFFFFFF; 0x1C, 0xFFFFFFFF; 0x20, 0xFFFFFFFF; 0x24, 0xFFFFFFFF.\n- First read set: read_pcie_slv0_reg(0x10), 0x14, 0x18, 0x1C, 0x20, 0x24.\n- Second write set: write_pcie_slv0_reg(0x10, 0x0); 0x14, 0x4; 0x18, 0x20000000; 0x1C, 0x40000000; 0x20, 0x60000000; 0x24, 0x80000000.\n- Second read set: read_pcie_slv0_reg(0x10), 0x14, 0x18, 0x1C, 0x20, 0x24.\n- Enable: write_pcie_slv0_reg(0x4, 0x7).\n\nIf DM1_RC path:\n- mem_base_program_dm1_x4().\n- For i = 0..9: rd_wr_data1 = read_pcie_slv1_reg(i*0x4).\n- Enable: write_pcie_slv1_reg(0x4, 0x7).\n- BAR writes: write_pcie_slv1_reg(0x10..0x24, 0xFFFFFFFF) then read back 0x10..0x24.\n- Program values: write_pcie_slv1_reg(0x10, 0x0), 0x14, 0x4, 0x18, 0x20000000, 0x1C, 0x40000000, 0x20, 0x60000000, 0x24, 0x80000000; then read back 0x10..0x24.\n\nFinal status polling and end:\n- wait_on(10); data_rd = read_reg(0xE6004100); while (data_rd != 0x12345678) { wait_on(5); data_rd = read_reg(0xE6004100); }.\n- finish(0).",
    "Hidden_Impacted_Registers": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "Hidden_Validation_Acceptance_Criteria": "SII status poll: For SII0, loop until (read_sii0_reg(0xC0) & 0xD1) == 0xD1. If DM1_RC, for SII1, loop until (read_sii1_reg(0xC0) & 0xD1) == 0xD1.\nConfiguration BAR access: Values written via write_pcie_slv[0/1]_reg at offsets 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24 are readable via read_pcie_slv[0/1]_reg and reflect the programmed patterns.\nFinal completion: Poll read_reg(0xE6004100) until it equals 0x12345678, then finish(0).",
    "Hidden_Header_Includes": "#include <stdlib.h>\n#include <stdio.h>\n#include <test_common.h>\n#include \"pcie.h\"",
    "Hidden_Macro_Defines": "NA",
    "Hidden_Skip_Array_Definition": "NA"
  }
]'''

# Constants
OUTPUT_DIR = os.path.join('Test_Output', 'PCIE', 'TestPlan')
VISIBLE_ORDER = [
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
META_PREFERRED_ORDER = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
    'Hidden_Header_Includes',
    # Note: Input uses Hidden_Macro_Defines (plural). Preserve exact key name if present
    'Hidden_Macro_Defines',
    'Hidden_Skip_Array_Definition'
]

BLUE_FILL = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
BORDER_THIN = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))
HEADER_ALIGN = Alignment(horizontal='center', vertical='center', wrap_text=True)
DATA_ALIGN_LEFT_TOP = Alignment(horizontal='left', vertical='top', wrap_text=True)
DATA_ALIGN_CENTER_TOP = Alignment(horizontal='center', vertical='top', wrap_text=True)


def validate_and_load_json(src: str):
    try:
        data = json.loads(src)
        if not isinstance(data, list) or len(data) == 0:
            raise ValueError('JSON must be a non-empty array')
        return data
    except Exception as e:
        raise SystemExit(f'JSON validation failed: {e}')


def build_schema(records):
    cols = []
    seen = set()
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen.add(k)
                cols.append(k)
    return cols


def number_lines(text: str) -> str:
    if text is None:
        return ''
    lines = [ln for ln in str(text).splitlines()]
    out = []
    idx = 1
    for ln in lines:
        raw = ln.strip()
        if raw == '':
            out.append('')
            continue
        # strip common bullet/number prefixes
        raw = re.sub(r'^(?:-\s*|\d+[\.)]\s*)', '', raw)
        out.append(f"{idx}. {raw}")
        idx += 1
    return '\n'.join(out)


def autofit_columns(ws):
    # Approximate auto-fit by max string length per column
    col_widths = {}
    for row in ws.iter_rows(values_only=True):
        for i, val in enumerate(row, start=1):
            s = '' if val is None else str(val)
            l = max(len(part) for part in s.split('\n')) if s else 0
            col_widths[i] = max(col_widths.get(i, 0), l)
    for i, w in col_widths.items():
        ws.column_dimensions[chr(64 + i) if i <= 26 else None]
    for i, w in col_widths.items():
        col_letter = ws.cell(row=1, column=i).column_letter
        ws.column_dimensions[col_letter].width = min(max(w + 2, 12), 80)


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = BORDER_THIN


def save_and_validate_xlsx(wb, path):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    wb.save(path)
    # Validate OOXML by reading ZIP structure
    with ZipFile(path, 'r') as zf:
        names = zf.namelist()
        if '[Content_Types].xml' not in names or not any(n.startswith('xl/') for n in names):
            raise SystemExit('XLSX validation failed: missing OOXML parts')


def main():
    records = validate_and_load_json(FULL_JSON)

    # Build union schema in first-seen order
    schema = build_schema(records)

    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Header
    for c, key in enumerate(schema, start=1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = HEADER_ALIGN
        cell.fill = BLUE_FILL
    ws.freeze_panes = 'A2'

    # Rows (exact values)
    for r, rec in enumerate(records, start=2):
        for c, key in enumerate(schema, start=1):
            ws.cell(row=r, column=c, value=rec.get(key, ''))

    # Create Meta_data_sheet with META columns as-is in preferred order if present
    meta_cols = [k for k in META_PREFERRED_ORDER if k in schema]
    meta_ws = wb.create_sheet('Meta_data_sheet')
    for c, key in enumerate(meta_cols, start=1):
        meta_ws.cell(row=1, column=c, value=key).font = Font(bold=True)
        meta_ws.cell(row=1, column=c).alignment = HEADER_ALIGN
        meta_ws.cell(row=1, column=c).fill = BLUE_FILL
    for r, rec in enumerate(records, start=2):
        for c, key in enumerate(meta_cols, start=1):
            meta_ws.cell(row=r, column=c, value=rec.get(key, ''))
    meta_ws.sheet_state = 'veryHidden'

    # Normalize main sheet in-place: rename Data -> TestPlan
    ws.title = 'TestPlan'

    # Remove META columns from TestPlan and reorder visible columns
    visible_cols = [k for k in VISIBLE_ORDER if k in schema]

    # Build new table data for TestPlan with required column order
    table = [visible_cols]
    for rec in records:
        row = []
        for key in visible_cols:
            val = rec.get(key, '')
            row.append(val)
        table.append(row)

    # Clear existing data on TestPlan
    ws.delete_rows(1, ws.max_row)

    # Write new header and rows
    for c, key in enumerate(visible_cols, start=1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = HEADER_ALIGN
        cell.fill = BLUE_FILL
    ws.freeze_panes = 'A2'

    # Apply numbering to specific columns while writing
    wrap_cols = set(['Test Description', 'Remarks', 'Test Steps / Procedure', 'Validation / Acceptance Criteria'])
    for r_idx, data_row in enumerate(table[1:], start=2):
        for c_idx, key in enumerate(visible_cols, start=1):
            val = data_row[c_idx - 1]
            if key in ('Test Steps / Procedure', 'Validation / Acceptance Criteria'):
                val = number_lines(val)
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Formatting
    # Wrap text columns
    for c_idx, key in enumerate(visible_cols, start=1):
        for r in range(2, ws.max_row + 1):
            cell = ws.cell(row=r, column=c_idx)
            if key in wrap_cols:
                cell.alignment = DATA_ALIGN_LEFT_TOP
            elif key == 'Index':
                cell.alignment = DATA_ALIGN_CENTER_TOP
            else:
                cell.alignment = DATA_ALIGN_LEFT_TOP

    # Header styling is already set; ensure vertical center
    for c in range(1, ws.max_column + 1):
        ws.cell(row=1, column=c).alignment = HEADER_ALIGN

    # Borders
    apply_borders(ws)

    # Approximate auto-fit widths
    autofit_columns(ws)

    # Data validation for Code Generation (Required / Not)
    if 'Code Generation (Required / Not)' in visible_cols:
        col_idx = visible_cols.index('Code Generation (Required / Not)') + 1
        dv = DataValidation(type='list', formula1='"Required, Blank, Not Required"', allow_blank=True, showErrorMessage=True)
        ws.add_data_validation(dv)
        dv.add(f"{ws.cell(row=1, column=col_idx).column_letter}2:{ws.cell(row=1, column=col_idx).column_letter}{ws.max_row}")

    # Enforce final visibility: only TestPlan (visible) and Meta_data_sheet (veryHidden)
    if 'Data' in [s.title for s in wb.worksheets]:
        # Try to delete if exists
        try:
            ds = wb['Data']
            wb.remove(ds)
        except Exception:
            raise SystemExit('Validation failed: lingering Data sheet could not be removed')

    # Compute IST timestamp and filename
    ist = datetime.now(ZoneInfo('Asia/Kolkata'))
    ts_date = ist.strftime('%Y%m%d')
    ts_time = ist.strftime('%H%M%S')
    filename = f"PCIE_TestPlan_{ts_date}_{ts_time}.xlsx"
    rel_path = os.path.join(OUTPUT_DIR, filename)

    # Save and validate
    save_and_validate_xlsx(wb, rel_path)

    # Export variables for workflow step
    with open('.testplan_ts', 'w', encoding='utf-8') as f:
        f.write(f"{ts_date}_{ts_time}")
    with open('.testplan_fname', 'w', encoding='utf-8') as f:
        f.write(rel_path)


if __name__ == '__main__':
    main()
