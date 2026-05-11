import json, os, sys, zipfile
from copy import deepcopy
from datetime import datetime
from io import StringIO

from openpyxl import Workbook, load_workbook
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter

# Configuration (fixed per task input)
OUTPUT_PATH = os.path.join("Test_Output", "PCIE", "TestPlan")
OUTPUT_FILENAME = "PCIE_TestPlan_20260511_000000.xlsx"

# JSON data embedded deterministically
JSON_DATA = r'''[
  {
    "Index": 1,
    "SS / Module": "PCIE0 DBI DSP",
    "Feature": "DBI register read/write verification",
    "Test Case Name": "pcie0_dbi_dsp_reg_wr_rd_test",
    "Test Description": "Validates PCIe downstream DBI register reset values and masked write-read behavior using predefined data patterns. The test passes only if all comparisons match expected results.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Unreadable registers are skipped. Some registers are excluded from default checks. Entries marked to skip are not tested. Soft reset is not executed.",
    "Test Steps / Procedure": "1) Read all readable DBI registers except the excluded ones and compare with their reset values.\n2) For each test pattern, write to all writable registers that are not skipped.\n3) Read back each eligible register and compute the expected value using read and write masks and the reset value, then compare with the actual value.\n4) If any mismatch is detected, record a failure; otherwise record a pass.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1) Default value check for readable, non-excluded registers → Read value equals reset value.\n2) Write-read check for each eligible register and pattern → Read value equals masked expected value.\n3) Overall result → No mismatches indicates PASS; any mismatch indicates FAIL.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie0_dbi_dsp_reg_wr_rd_test",
    "Hidden_Test_Description": "Purpose: Validate PCIe0 DBI DSP register defaults and masked read/write behavior. Flow: test_case() → chk_rst_val() → chk_rd_wr() → finish(). In chk_rst_val(): iterate i=0..CNT-1 with addr=addr_array[i]; if read_mask_array[i]==0x00000000 then skip (print \"RST : This address 0x%x is not readable, hence skipped for reading\"); if addr equals mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG or mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS or mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF then skip default check; else data_rd=read_reg(addr); if data_rd==default_value_array[i] then (optional PASS log) else def_fail_cnt++ and print \"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\\tRead_data : 0x%x\". In chk_rd_wr(): patterns chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; For each pattern: write phase: for i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 then continue (optional skip log); if write_mask_array[i]==0x00000000 then continue (optional not writable log); else write_reg(addr,data_wr) (optional log). Read/verify phase: for i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 then continue; if write_mask_array[i]==0x00000000 then continue; if read_mask_array[i]==0x00000000 then continue; else data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd==exp_val then (optional PASS log) else wr_fail_cnt++ and print \"Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\\tRead value=0x%x\". test_case(): if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). soft_reset_chk() exists (writes SOFT_RST_REG DATA to SOFT_RST_REG_ADDRESS, waits, restores default) but is not invoked.",
    "Hidden_Remarks": "Default value check skips addresses where read_mask_array[i]==0x00000000. Default value check excludes mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, and mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF. Write phase skips when skip_array[i]==1 or write_mask_array[i]==0x00000000. Read/verify phase skips when skip_array[i]==1 or write_mask_array[i]==0x00000000 or read_mask_array[i]==0x00000000. soft_reset_chk() present but commented out/not called.",
    "Hidden_Test_Steps_Procedure": "1) Invoke chk_rst_val(): iterate i = 0..CNT-1; set addr = addr_array[i]. If read_mask_array[i] == 0x00000000, print \"RST: not readable, skipped\" and continue. If addr matches mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG or mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS or mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF, continue (skip default value check). Read data_rd = read_reg(addr). If data_rd == default_value_array[i], optionally print PASS; else increment def_fail_cnt and print failure including expected and read. 2) Invoke chk_rd_wr(): For each j in {0..5} with data_wr in {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}: Writing phase: iterate i = 0..CNT-1; addr = addr_array[i]; if skip_array[i] == 1, optionally print skip and continue; if write_mask_array[i] == 0x00000000, optionally print not writable and continue; else write_reg(addr, data_wr) and optionally print address and data written. Reading/verification phase: iterate i = 0..CNT-1; addr = addr_array[i]; if skip_array[i] == 1, optionally print skip and continue; if write_mask_array[i] == 0x00000000, optionally print not writable and continue; if read_mask_array[i] == 0x00000000, optionally print not readable and continue; else read data_rd = read_reg(addr); compute wr_n = (write_mask_array[i] ^ 0xffffffff); compute exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd == exp_val, optionally print PASS; else increment wr_fail_cnt and print mismatch including expected and read. 3) In test_case(): after chk_rst_val() and chk_rd_wr(), if (def_fail_cnt > 0 || wr_fail_cnt > 0) call finish(1); else call finish(0). Note: soft_reset_chk() is present but commented out and not executed.",
    "Hidden_Impacted_Registers": "mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF, mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG, mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG, mizar_PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG, mizar_PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE LINE_SIZE REG, mizar_PCIE0_DBI_DSP_BAR0_REG, mizar_PCIE0_DBI_DSP_BAR1_REG, mizar_PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS REG, mizar_PCIE0_DBI_DSP_SEC_STAT_IO LIMIT IO BASE REG, mizar_PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE REG, mizar_PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE REG, mizar_PCIE0_DBI_DSP_PREF_BASE_UPPER REG, mizar_PCIE0_DBI_DSP_PREF_LIMIT_UPPER REG, mizar_PCIE0_DBI_DSP_IO_LIMIT_UPPER_IO_BASE_UPPER REG, mizar_PCIE0_DBI_DSP_TYPE1_CAP_PTR REG, mizar_PCIE0_DBI_DSP_TYPE1_EXP_ROM_BASE REG, mizar_PCIE0_DBI_DSP_BRIDGE_CTRL_INT_PIN_INT_LINE REG, mizar_PCIE0_DBI_DSP_LINK_CONTROL3 REG, mizar_PCIE0_DBI_DSP_DEVICE_CAPABILITIES2 REG, mizar_PCIE0_DBI_DSP_DEVICE_CONTROL2_DEVICE_STATUS2 REG, mizar_PCIE0_DBI_DSP_LINK_CAPABILITIES2 REG, mizar_PCIE0_DBI_DSP_LINK_CONTROL2_LINK_STATUS2 REG, mizar_PCIE0_DBI_DSP_AER_EXT_CAP_HDR OFF, mizar_PCIE0_DBI_DSP_UNCORR_ERR_STATUS OFF, mizar_PCIE0_DBI_DSP_CORR_ERR_STATUS OFF, mizar_PCIE0_DBI_DSP_PCI_MSI_CAP_ID_NEXT_CTRL REG, mizar_PCIE0_DBI_DSP_MSI_CAP OFF_04H REG, mizar_PCIE0_DBI_DSP_MSI_CAP OFF_08H REG, mizar_PCIE0_DBI_DSP_DMA_CTRL OFF, mizar_PCIE0_DBI_DSP_DMA_WRITE_ENGINE_EN OFF, mizar_PCIE0_DBI_DSP_DMA_READ_ENGINE_EN OFF, mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_1 OFF_OUTBOUND_0, mizar_PCIE0_DBI_DSP_IATU_REGION_CTRL_2 OFF_INBOUND_0, mizar_PCIE0_DBI_DSP_TRGT_MAP_CTRL OFF, mizar_PCIE0_DBI_DSP_GEN3_RELATED OFF, mizar_PCIE0_DBI_DSP_GEN3_EQ_CONTROL OFF, mizar_PCIE0_DBI_DSP_ORDER_RULE_CTRL OFF, mizar_PCIE0_DBI_DSP_PIPE_LOOPBACK_CONTROL OFF, mizar_PCIE0_DBI_DSP_MISC_CONTROL_1 OFF, mizar_PCIE0_DBI_DSP_MULTI_LANE_CONTROL OFF, mizar_PCIE0_DBI_DSP_PL_DEBUG0 OFF, mizar_PCIE0_DBI_DSP_PL_APP_BUS_DEV NUM_STATUS OFF, mizar_PCIE0_DBI_DSP_PCIE0_VERSION_NUMBER OFF, mizar_PCIE0_DBI_DSP_PM_UTILITY OFF",
    "Hidden_Validation_Acceptance_Criteria": "Default stage: For each i where read_mask_array[i]!=0x00000000 and addr_array[i] is not mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, or mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF, require read_reg(addr_array[i])==default_value_array[i]; else def_fail_cnt++ and log mismatch. Write-read stage: For each pattern in {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000} and for each i where skip_array[i]==0 and write_mask_array[i]!=0x00000000 and read_mask_array[i]!=0x00000000, after write_reg(addr_array[i],data_wr), require read_reg(addr_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])); else wr_fail_cnt++ and log mismatch. Final: If (def_fail_cnt==0 && wr_fail_cnt==0) then finish(0) PASS; otherwise finish(1) FAIL."
  },
  {
    "Index": 2,
    "SS / Module": "PCIE0 DBI USP",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie0_dbi_usp_reg_wr_rd_test",
    "Test Description": "Checks default values of DBI USP registers and verifies masked write-read behavior across eligible registers. The test passes only if all comparisons match expected results.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Registers with no read access are skipped for default checks. Three registers are excluded from default verification. Entries marked to skip are not exercised. Only readable and writable registers are tested. A soft reset helper is present but not executed.",
    "Test Steps / Procedure": "1) Read all readable DBI USP registers and compare with their documented reset values, excluding the capability pointer, the device control and status register, and the PL debug register. \n2) For each predefined write pattern, write the value to each writable register that is not in the skip list. \n3) Read back each eligible register and derive the expected value using the read mask, write mask, and reset value. \n4) Compare the read value to the expected value and record any mismatches. \n5) Declare the test pass if no default-value or write-read mismatches are recorded; otherwise declare fail.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1) Default value comparison for each readable register (excluding the three noted) → The read value equals the documented reset value. \n2) Write-read comparison for each eligible register and pattern → The read value equals the masked expected value derived from the pattern, masks, and reset value. \n3) Final result → Zero default-check failures and zero write-read failures indicate PASS; any nonzero failure count indicates FAIL.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie0_dbi_usp_reg_wr_rd_test",
    "Hidden_Test_Description": "Verifies default values and masked read-write behavior of PCIe0 DBI USP registers. test_case() invokes chk_rst_val() then chk_rd_wr(); soft_reset_chk() exists but is not executed. In chk_rst_val(): iterate i=0..CNT-1, addr=addr_array[i]; if read_mask_array[i]==0x00000000, skip (not readable). If addr equals mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG or mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS or mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF, skip default check. Otherwise read_reg(addr) and compare against default_value_array[i]; increment def_fail_cnt on mismatch. In chk_rd_wr(): for each data_wr in {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}, perform writes for i=0..CNT-1 if skip_array[i]==0 and write_mask_array[i]!=0x00000000; then perform reads if skip_array[i]==0 and write_mask_array[i]!=0x00000000 and read_mask_array[i]!=0x00000000. Compute wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); compare data_rd to exp_val; increment wr_fail_cnt on mismatch. After checks, finish(1) if any failure count >0 else finish(0).",
    "Hidden_Remarks": "Default checks skip addresses with read_mask_array[i]==0x00000000 and skip three specific registers: mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF. Write operations are skipped if skip_array[i]==1 or write_mask_array[i]==0x00000000. Readback in write-read stage is skipped if write_mask_array[i]==0x00000000 or read_mask_array[i]==0x00000000. soft_reset_chk() is present but not called.",
    "Hidden_Test_Steps_Procedure": "test_case(): call chk_rst_val(); call chk_rd_wr(); if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0). chk_rst_val(): for (i=0;i<CNT;i++): addr=addr_array[i]; if (read_mask_array[i]==0x00000000) continue; if (addr==mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG || addr==mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS || addr==mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF) continue; data_rd=read_reg(addr); if (data_rd==default_value_array[i]) {optional PASS log} else {def_fail_cnt++; print mismatch}. chk_rd_wr(): int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; for (j=0;j<6;j++): data_wr=chk_val[j]; // write phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; else write_reg(addr,data_wr). // read/verify phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; if (read_mask_array[i]==0x00000000) continue; data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) {optional PASS log} else {wr_fail_cnt++; print mismatch}. soft_reset_chk(): reads default_value=read_reg(SOFT_RST_REG_ADDRESS); writes SOFT_RST_REG DATA (0x00000000) to SOFT_RST_REG_ADDRESS; waits; restores default value; waits. Not invoked.",
    "Hidden_Impacted_Registers": "mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL DEVICE_STATUS, mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF, mizar_PCIE0_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG, mizar_PCIE0_DBI_USP_TYPE1_STATUS_COMMAND_REG, mizar_PCIE0_DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG, mizar_PCIE0_DBI_USP_BAR0_REG, mizar_PCIE0_DBI_USP_BAR1_REG, mizar_PCIE0_DBI_USP_LINK_CONTROL2_LINK_STATUS2_REG, mizar_PCIE0_DBI_USP_DEVICE_CAPABILITIES2_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL2_DEVICE_STATUS2_REG, mizar_PCIE0_DBI_USP_LINK_CAPABILITIES2_REG, mizar_PCIE0_DBI_USP_AER_EXT_CAP_HDR OFF, mizar_PCIE0_DBI_USP_UNCORR_ERR_STATUS OFF, mizar_PCIE0_DBI_USP_CORR_ERR_STATUS OFF, mizar_PCIE0_DBI_USP_MSI_CTRL_ADDR OFF, mizar_PCIE0_DBI_USP_MSI_CTRL_UPPER_ADDR OFF, mizar_PCIE0_DBI_USP_IATU REGION_CTRL_1 OFF_OUTBOUND_0, mizar_PCIE0_DBI_USP_IATU REGION_CTRL_2 OFF_OUTBOUND_0, mizar_PCIE0_DBI_USP_IATU LWR_BASE_ADDR OFF_OUTBOUND_0, mizar_PCIE0_DBI_USP_IATU REGION_CTRL_1 OFF_INBOUND_0, mizar_PCIE0_DBI_USP_IATU REGION_CTRL_2 OFF_INBOUND_0",
    "Hidden_Validation_Acceptance_Criteria": "Default stage: For all i where read_mask_array[i]!=0x00000000 and addr_array[i] is not mizar_PCIE0_DBI_USP_CAP ID_NXT_PTR_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, or mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF, require read_reg(addr_array[i])==default_value_array[i]; else def_fail_cnt++. Write-read stage: For each pattern in {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000} and for each i where skip_array[i]==0 and write_mask_array[i]!=0x00000000 and read_mask_array[i]!=0x00000000, after write_reg(addr_array[i],data_wr), require read_reg(addr_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])); else wr_fail_cnt++. Final: If (def_fail_cnt==0 && wr_fail_cnt==0) then finish(0) PASS; otherwise finish(1) FAIL."
  },
  {
    "Index": 3,
    "SS / Module": "PCIE0 SII RC",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie0_sii_rc_reg_wr_rd_test",
    "Test Description": "Validates reset values and masked write-read behavior of the PCIe SII root complex registers. The test passes only when all comparisons match the expected results.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Unreadable registers are skipped. One register is excluded from reset checks. Entries in the skip list are not tested. Writes and reads are skipped when not supported. The soft reset helper is not executed.",
    "Test Steps / Procedure": "1) Read each register that supports reads and compare to its documented reset value, excluding the dedicated reset control register. 2) For each predefined data pattern, write the value to each writable register that is not in the skip list. 3) Read back each eligible register and compute the expected result using the read mask, write mask, and reset value, then compare with the actual value. 4) Record any mismatches during reset checks or write-read checks and determine the final result.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1) Reset value comparison → The read value equals the documented reset value for every readable register not excluded. 2) Write-read comparison → The read value equals the expected masked result derived from the pattern, masks, and reset value for every eligible register and pattern. 3) Final status → Zero reset and write-read mismatches indicates PASS; any mismatch indicates FAIL.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie0_sii_rc_reg_wr_rd_test",
    "Hidden_Test_Description": "Verifies default register values and masked write-read behavior for PCIe0 SII RC registers. test_case() calls chk_rst_val() then chk_rd_wr(); soft_reset_chk() is present but not invoked. In chk_rst_val(): iterate i=0..CNT-1 with addr=addr_array[i]; if read_mask_array[i]==0x00000000, skip as not readable; if addr_array[i]==mizar_PCIE0_SII_PHY_RST_CONTROL, skip default check; else read_reg(addr) into data_rd and compare against default_value_array[i]; on mismatch, increment def_fail_cnt and print failure log; on match, optionally print PASS under DEBUG_DISPLAY. In chk_rd_wr(): for each data_wr pattern in {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}, perform writes: for each i, if skip_array[i]==1 continue; if write_mask_array[i]==0x00000000 continue; else write_reg(addr,data_wr) and optionally print. Then perform reads: for each i, if skip_array[i]==1 continue; if write_mask_array[i]==0x00000000 continue; if read_mask_array[i]==0x00000000 continue; else data_rd=read_reg(addr); compute wr_n=(write_mask_array[i]^0xffffffff); compute exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd==exp_val optionally print PASS; else increment wr_fail_cnt and print mismatch log. At end of test_case(): finish(1) if (def_fail_cnt>0 || wr_fail_cnt>0), else finish(0).",
    "Hidden_Remarks": "Registers with read_mask_array[i]==0x00000000 are skipped for reading. The address mizar_PCIE0_SII_PHY_RST_CONTROL is excluded from default value checks. Addresses in skip_array are not exercised. Writes are skipped when write_mask_array[i]==0x00000000. Readback in the write-read phase is skipped when write_mask_array[i]==0x00000000 or read_mask_array[i]==0x00000000. The soft_reset_chk() helper exists but is not called in test_case().",
    "Hidden_Test_Steps_Procedure": "test_case(): call chk_rst_val(); call chk_rd_wr(); if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0). chk_rst_val(): for (i=0; i<CNT; i++): addr=addr_array[i]; if (read_mask_array[i]==0x00000000) {optional print \"RST : This address 0x%x is not readable, hence skipped\"; continue;} if (addr_array[i]==mizar_PCIE0_SII_PHY_RST_CONTROL) {continue;} data_rd=read_reg(addr); if (data_rd==default_value_array[i]) {optional print PASS;} else {def_fail_cnt++; print \"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\\tRead_data : 0x%x\";}. chk_rd_wr(): int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; for (j=0; j<6; j++): data_wr=chk_val[j]; // Write phase: for (i=0; i<CNT; i++): addr=addr_array[i]; if (skip_array[i]==1) {optional print skip; continue;} if (write_mask_array[i]==0x00000000) {optional print not writable; continue;} write_reg(addr,data_wr); {optional print write details}. // Read/verify phase: for (i=0; i<CNT; i++): addr=addr_array[i]; if (skip_array[i]==1) {optional print skip; continue;} if (write_mask_array[i]==0x00000000) {optional print not writable; continue;} if (read_mask_array[i]==0x00000000) {optional print not readable; continue;} data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) {optional print PASS;} else {wr_fail_cnt++; print \"Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\\tRead value=0x%x\";}. soft_reset_chk(): default_value=read_reg(SOFT_RST_REG_ADDRESS); write_reg(SOFT_RST_REG_ADDRESS,SOFT_RST_REG_DATA); wait_on(1000); write_reg(SOFT_RST_REG_ADDRESS,default_value); wait_on(1000); // not called by test_case().",
    "Hidden_Impacted_Registers": "mizar_PCIE0_SII_CFG_BAR0_START1, mizar_PCIE0_SII_CFG_BAR0_START2, mizar_PCIE0_SII_CFG_BAR0_LIMIT1, mizar_PCIE0_SII_CFG_BAR0_LIMIT2, mizar_PCIE0_SII_CFG_BAR1_START, mizar_PCIE0_SII_CFG_BAR1_LIMIT1, mizar_PCIE0_SII_CFG_BAR2_START1, mizar_PCIE0_SII_CFG_BAR2_START2, mizar_PCIE0_SII_CFG_BAR2_LIMIT1, mizar_PCIE0_SII_CFG_BAR2_LIMIT2, mizar_PCIE0_SII_CFG_BAR3_START, mizar_PCIE0_SII_CFG_BAR3_LIMIT, mizar_PCIE0_SII_CFG_BAR4_START1, mizar_PCIE0_SII_CFG_BAR4_START2, mizar_PCIE0_SII_CFG_BAR4_LIMIT1, mizar_PCIE0_SII_CFG_BAR4_LIMIT2, mizar_PCIE0_SII_CFG_BAR5_START, mizar_PCIE0_SII_CFG_BAR5_LIMIT, mizar_PCIE0_SII_PCIE0_CONFIG_INFO1, mizar_PCIE0_SII_PCIE0_CONFIG_INFO2, mizar_PCIE0_SII_PCIE0_GEN_CONTROL1, mizar_PCIE0_SII_PCIE0_GEN_CONTROL2, mizar_PCIE0_SII_PCIE0_GEN_CONTROL3, mizar_PCIE0_SII_PCIE0_PM_CONTROL, mizar_PCIE0_SII_PCIE0_CONTROL_PM_STS, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER1, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER4, mizar_PCIE0_SII_PCIE0_TRANSMIT_REQ, mizar_PCIE0_SII_PCIE0_RCV_MSG_HDR1, mizar_PCIE0_SII_PCIE0_RCV_MSG_HDR2, mizar_PCIE0_SII_PCIE0_RCV_MSG_HDR3, mizar_PCIE0_SII_PCIE0_RCV_MSG_HDR4, mizar_PCIE0_SII_PCIE0_RCV_MSG_STS, mizar_PCIE0_SII_RCV_INTERRPUT_CTRL, mizar_PCIE0_SII_CFG_EXP_ROM_START, mizar_PCIE0_SII_CFG_EXP_ROM_LIMIT, mizar_PCIE0_SII_CFG_EXP_ROM_INFO, mizar_PCIE0_SII_CXPL_DEBUG_INFO1, mizar_PCIE0_SII_CXPL_DEBUG_INFO2, mizar_PCIE0_SII_CXPL_DEBUG_INFO_EI, mizar_PCIE0_SII_PCIE0_TARGET_INFO1, mizar_PCIE0_SII_PCIE0_TARGET_INFO2, mizar_PCIE0_SII_PCIE0_CONTOLLER ERROR STATUS, mizar_PCIE0_SII_PCIE0_CONTROLLER_INT_STS, mizar_PCIE0_SII_PCIE0_CONTROLLER_INTERRUPT_CONTROL, mizar_PCIE0_SII_PHY_RST_CONTROL, mizar_PCIE0_SII_LINK DEBUG DATA, mizar_PCIE0_SII_PCIE0_ERR_STS, mizar_PCIE0_SII_PCIE0_ERR_INTERRUPT_CTRL, mizar_PCIE0_SII_CFG_MSI_INT, mizar_PCIE0_SII_LTR MSG, mizar_PCIE0_SII_LTR MSG LATENCY, mizar_PCIE0_SII_APP LTR LATENCY, mizar_PCIE0_SII_CFG LTR MAX LATENCY, mizar_PCIE0_SII_OBFF CNTRL, mizar_PCIE0_SII_SLV_AWMISC_INFO, ... (and many more as listed previously)",
    "Hidden_Validation_Acceptance_Criteria": "Default value stage: For each i where read_mask_array[i] != 0x00000000 and addr_array[i] != mizar_PCIE0_SII_PHY_RST_CONTROL, require read_reg(addr_array[i]) == default_value_array[i]; on mismatch def_fail_cnt++ and log failure. Write-read stage: For each pattern in {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000} and for each i where skip_array[i] == 0 and write_mask_array[i] != 0x00000000 and read_mask_array[i] != 0x00000000, after write_reg(addr_array[i], data_wr), require read_reg(addr_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])); on mismatch wr_fail_cnt++ and log error. Final: If (def_fail_cnt == 0 && wr_fail_cnt == 0) then finish(0) PASS, else finish(1) FAIL."
  },
  {
    "Index": 4,
    "SS / Module": "PCIE1 DBI DSP",
    "Feature": "Dbi: R/W",
    "Test Case Name": "pcie1_dbi_dsp_reg_wr_rd_test",
    "Test Description": "Validates PCIe1 DBI DSP register reset values and masked write-read behavior using predefined data patterns. The test iterates through eligible registers, applies masks, and compares readback to expected values.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Unreadable addresses are skipped. Three registers are excluded from reset checks. Skip list entries are not tested. Soft reset is not executed.",
    "Test Steps / Procedure": "1) Read each readable register except DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, and DBI_DSP_PL_DEBUG1_OFF and compare with its reset value. 2) For each data pattern, write to each writable register not in the skip list. 3) Read back each eligible register and compare the value with the mask-derived expected value. 4) Determine pass or fail based on mismatch counters.",
    "Impacted Registers": "DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, DBI_DSP_PL_DEBUG1_OFF",
    "Validation / Acceptance Criteria": "1) Default read equals reset value for each readable register not excluded → Pass. 2) For each pattern and eligible register, readback equals the mask-derived expected value → Pass. 3) Zero default-check and write-read mismatches → Pass; otherwise → Fail.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie1_dbi_dsp_reg_wr_rd_test",
    "Hidden_Test_Description": "The test validates default values and masked write-read behavior of PCIe1 DBI DSP registers. In test_case(), chk_rst_val() executes first, then chk_rd_wr(), followed by finish(1) on any failure count, else finish(0). In chk_rst_val(): for i in 0..CNT-1, addr = addr_array[i]; if read_mask_array[i] == 0x00000000, skip (optional log). If addr == mizar_PCIE1_DBI_DSP_CAP_ID_NXT_PTR REG or addr == mizar_PCIE1_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS or addr == mizar_PCIE1_DBI_DSP_PL_DEBUG1_OFF, skip default check. Otherwise data_rd = read_reg(addr); if data_rd == default_value_array[i], optional PASS log; else def_fail_cnt++ and print mismatch with expected and read values. In chk_rd_wr(): patterns chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}; for each pattern data_wr: Write phase: for each i, addr = addr_array[i]; if skip_array[i] == 1, continue (optional skip log); if write_mask_array[i] == 0x00000000, continue (optional not writable log); else write_reg(addr, data_wr) (optional write detail log). Read/verify phase: for each i, addr = addr_array[i]; if skip_array[i] == 1, continue; if write_mask_array[i] == 0x00000000, continue (optional not writable log); if read_mask_array[i] == 0x00000000, continue (optional not readable log); else data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd == exp_val, optional PASS log; else wr_fail_cnt++ and print mismatch with expected and read values. soft_reset_chk() writes SOFT_RST_REG_DATA to SOFT_RST_REG_ADDRESS and restores default_value, but it is not called.",
    "Hidden_Remarks": "Default checks skip when read_mask_array[i] == 0x00000000. Default checks exclude mizar_PCIE1_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE1_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, and mizar_PCIE1_DBI_DSP_PL_DEBUG1_OFF. Write phase skips when skip_array[i] == 1 or write_mask_array[i] == 0x00000000. Readback phase skips when skip_array[i] == 1 or write_mask_array[i] == 0x00000000 or read_mask_array[i] == 0x00000000. soft_reset_chk() exists but is not executed.",
    "Hidden_Test_Steps_Procedure": "test_case(): call chk_rst_val(); optional debug banner; call chk_rd_wr(); optional debug banner; if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1); else finish(0). chk_rst_val(): for (i = 0; i < CNT; i++): addr = addr_array[i]; if (read_mask_array[i] == 0x00000000) {optional print \"RST : This address 0x%x is not readable, hence skipped for reading\"; continue;} if (addr_array[i] == mizar_PCIE1_DBI_DSP_CAP_ID_NXT_PTR_REG || addr_array[i] == mizar_PCIE1_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS || addr_array[i] == mizar_PCIE1_DBI_DSP_PL_DEBUG1_OFF) {continue;} data_rd = read_reg(addr); if (data_rd == default_value_array[i]) {optional print PASS;} else {defail_cnt++; print \"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\\tRead_data : 0x%x\";}. chk_rd_wr(): int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}; for (j = 0; j < 6; j++): data_wr = chk_val[j]; // Write phase: for (i = 0; i < CNT; i++): addr = addr_array[i]; if (skip_array[i] == 1) {optional print skip; continue;} if (write_mask_array[i] == 0x00000000) {optional print not writable; continue;} write_reg(addr, data_wr); {optional print address and data}. // Read/verify phase: for (i = 0; i < CNT; i++): addr = addr_array[i]; if (skip_array[i] == 1) {optional print skip; continue;} if (write_mask_array[i] == 0x00000000) {optional print not writable; continue;} if (read_mask_array[i] == 0x00000000) {optional print not readable; continue;} data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) {optional print PASS;} else {wr_fail_cnt++; print \"Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\\tRead value=0x%x\";}. soft_reset_chk(): default_value = read_reg(SOFT_RST_REG_ADDRESS); write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG DATA); wait_on(1000); write_reg(SOFT_RST_REG_ADDRESS, default_value); wait_on(1000); // not called.",
    "Hidden_Impacted_Registers": "mizar_PCIE1_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE1_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE1_DBI_DSP_PL_DEBUG1_OFF, mizar_PCIE1_DBI_DSP_TYPE1_DEV_ID_VEND ID REG, mizar_PCIE1_DBI_DSP_TYPE1_STATUS_COMMAND REG, mizar_PCIE1_DBI_DSP_TYPE1_CLASS_CODE_REV_ID REG, mizar_PCIE1_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE LINE SIZE REG, mizar_PCIE1_DBI_DSP_BAR0 REG, mizar_PCIE1_DBI_DSP_BAR1 REG, mizar_PCIE1_DBI_DSP_LINK_CONTROL3 REG, mizar_PCIE1_DBI_DSP_DEVICE_CAPABILITIES2 REG, mizar_PCIE1_DBI_DSP_DEVICE_CONTROL2_DEVICE_STATUS2 REG, mizar_PCIE1_DBI_DSP_LINK CAPABILITIES2 REG, mizar_PCIE1_DBI_DSP_LINK_CONTROL2_LINK_STATUS2 REG, mizar_PCIE1_DBI_DSP_AER_EXT CAP HDR OFF, mizar_PCIE1_DBI_DSP_UNCORR ERR STATUS OFF, mizar_PCIE1_DBI_DSP_CORR ERR STATUS OFF, mizar_PCIE1_DBI_DSP_MSI CTRL ADDR OFF, mizar_PCIE1_DBI_DSP_MSI CTRL UPPER ADDR OFF, mizar_PCIE1_DBI_DSP_MSI CTRL INT_0 EN OFF, mizar_PCIE1_DBI_DSP_MSI CTRL INT_0 MASK OFF, mizar_PCIE1_DBI_DSP_MSI CTRL INT_0 STATUS OFF, mizar_PCIE1_DBI_DSP_GEN3 RELATED OFF, mizar_PCIE1_DBI_DSP_GEN3 EQ CONTROL OFF, mizar_PCIE1_DBI_DSP_ORDER RULE CTRL OFF, mizar_PCIE1_DBI_DSP_PIPE LOOPBACK CONTROL OFF, mizar_PCIE1_DBI_DSP_MISC CONTROL_1 OFF, mizar_PCIE1_DBI_DSP_MULTI LANE CONTROL OFF, mizar_PCIE1_DBI_DSP_TRGT MAP CTRL OFF, mizar_PCIE1_DBI_DSP_IATU REGION CTRL_1 OFF_OUTBOUND_0, mizar_PCIE1_DBI_DSP_IATU REGION CTRL_2 OFF_OUTBOUND_0, mizar_PCIE1_DBI_DSP_IATU LWR BASE ADDR OFF_OUTBOUND_0, mizar_PCIE1_DBI_DSP_IATU REGION CTRL_1 OFF_INBOUND_0, mizar_PCIE1_DBI_DSP_IATU REGION CTRL_2 OFF_INBOUND_0",
    "Hidden_Validation_Acceptance_Criteria": "Default value stage: For each i where read_mask_array[i] != 0x00000000 and addr_array[i] is not mizar_PCIE1_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE1_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, or mizar_PCIE1_DBI_DSP_PL_DEBUG1_OFF, require read_reg(addr_array[i]) == default_value_array[i]; else def_fail_cnt++ and print mismatch. Write-read stage: For each pattern in {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000} and for each i where skip_array[i] == 0 and write_mask_array[i] != 0x00000000 and read_mask_array[i] != 0x00000000, after write_reg(addr_array[i], data_wr), require read_reg(addr_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); else wr_fail_cnt++ and print mismatch. Final result: If (def_fail_cnt == 0 && wr_fail_cnt == 0) then finish(0) PASS; otherwise finish(1) FAIL."
  },
  {
    "Index": 5,
    "SS / Module": "PCIE1 DBI USP",
    "Feature": "DBI Register R/W and Reset-Value Check",
    "Test Case Name": "pcie1_dbi_usp_reg_wr_rd_test",
    "Test Description": "Validates reset values and masked write-read behavior of DBI registers for the PCIe1 upstream port. The test iterates through eligible registers and compares readback with expected results.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Unreadable and unwritable registers are skipped. Some registers are excluded from reset checks. A soft reset helper exists but is not executed.",
    "Test Steps / Procedure": "1) Read each readable register that is not excluded and compare the value with its reset value. 2) For each data pattern, write to each writable register that is not skipped and then read back eligible registers. 3) For each readback, compute the expected value using read and write masks and the reset value, and compare to the actual value. 4) Pass if no mismatches are recorded; otherwise fail.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1) Default value of each readable, non-excluded register equals its reset value → Pass. 2) For each pattern, each eligible register readback equals the mask-derived expected value → Pass. 3) Zero default-check and write-read mismatches → Pass; any mismatch → Fail.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie1_dbi_usp_reg_wr_rd_test",
    "Hidden_Test_Description": "test_case() runs chk_rst_val() then chk_rd_wr(); soft_reset_chk() exists but is not called. After both checks, if def_fail_cnt > 0 or wr_fail_cnt > 0 then finish(1) else finish(0). In chk_rst_val(): iterate i=0..CNT-1; addr = addr_array[i]. If read_mask_array[i] == 0x00000000, skip reading. If addr equals mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG or mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS or mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF, skip default check. Otherwise read_reg(addr) into data_rd and compare to default_value_array[i]; on mismatch increment def_fail_cnt and print failure; else optionally print PASS. In chk_rd_wr(): use patterns chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}. For each pattern: Write phase: iterate i=0..CNT-1; addr = addr_array[i]; if skip_array[i] == 1, continue; if write_mask_array[i] == 0x00000000, continue; else write_reg(addr, data_wr) and optionally log. Read/verify phase: iterate i=0..CNT-1; addr = addr_array[i]; if skip_array[i] == 1, continue; if write_mask_array[i] == 0x00000000, continue; if read_mask_array[i] == 0x00000000, continue; else data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) optionally print PASS; else wr_fail_cnt++ and print mismatch.",
    "Hidden_Remarks": "Addresses with read_mask_array[i] == 0x00000000 are not read. Default value checks exclude mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, and mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF. Skip list entries (skip_array[i] == 1) are not exercised. Writes are skipped if write_mask_array[i] == 0x00000000. Readback in write-read is skipped if write_mask_array[i] == 0x00000000 or read_mask_array[i] == 0x00000000. soft_reset_chk() writes and restores SOFT_RST_REG_ADDRESS but is commented out.",
    "Hidden_Test_Steps_Procedure": "1) chk_rst_val(): for (i=0; i<CNT; i++): addr=addr_array[i]; if (read_mask_array[i]==0x00000000) {skip}; if (addr==mizar_PCIE1_DBI_USP_CAP ID_NXT_PTR_REG || addr==mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS || addr==mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF) {continue}; data_rd=read_reg(addr); if (data_rd==default_value_array[i]) {optional PASS} else {def_fail_cnt++; print mismatch with expected and read}. 2) chk_rd_wr(): int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; For each pattern j: data_wr=chk_val[j]; // Write phase: for (i=0; i<CNT; i++): addr=addr_array[i]; if (skip_array[i]==1) {continue}; if (write_mask_array[i]==0x00000000) {continue}; else {write_reg(addr,data_wr)}. // Read/verify phase: for (i=0; i<CNT; i++): addr=addr_array[i]; if (skip_array[i]==1) {continue}; if (write_mask_array[i]==0x00000000) {continue}; if (read_mask_array[i]==0x00000000) {continue}; data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) {optional PASS} else {wr_fail_cnt++; print mismatch with expected and read}. 3) test_case(): if (def_fail_cnt>0 || wr_fail_cnt>0) {finish(1)} else {finish(0)}. 4) soft_reset_chk() (not called): default=read_reg(0x00000000); write_reg(0x00000000,0x00000000); wait_on(1000); write_reg(0x00000000,default); wait_on(1000).",
    "Hidden_Impacted_Registers": "mizar_PCIE1_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG, mizar_PCIE1_DBI_USP_TYPE1_STATUS_COMMAND_REG, mizar_PCIE1_DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG, ... (and many more as listed previously)",
    "Hidden_Validation_Acceptance_Criteria": "Default value stage: For each i where read_mask_array[i] != 0x00000000 and addr_array[i] is not mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, or mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF, require read_reg(addr_array[i]) == default_value_array[i]; otherwise increment def_fail_cnt and log mismatch. Write-read stage: For each data pattern in {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000} and for each i with skip_array[i] == 0, write_mask_array[i] != 0x00000000, and read_mask_array[i] != 0x00000000, after write_reg(addr_array[i], data_wr), require read_reg(addr_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); otherwise increment wr_fail_cnt and log mismatch. Final result: if (def_fail_cnt == 0 && wr_fail_cnt == 0) finish(0) PASS; else finish(1) FAIL."
  },
  {
    "Index": 6,
    "SS / Module": "PCIE1 SII RC",
    "Feature": "Register R/W and reset-value verification",
    "Test Case Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Test Description": "Verifies that the SII root complex registers retain their default values and support masked write and read operations.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Unreadable registers are skipped. One register is excluded from default checks. Entries marked to skip are not tested. Soft reset is not executed.",
    "Test Steps / Procedure": "1) Read all readable SII root complex registers except the PHY reset control register and compare against documented default values.\n2) For each predefined pattern, write to all writable, non-skipped SII root complex registers.\n3) Read back each eligible register and compare with the value computed by applying the read and write masks with the default value.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1) Default read from each readable, non-excluded register → Equals documented reset value.\n2) For each pattern, read back from each eligible register → Equals value computed using masks and default value.\n3) Overall result → No mismatches indicates pass; any mismatch indicates fail.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Hidden_Test_Description": "program.c: test_case() calls chk_rst_val() then chk_rd_wr(); soft_reset_chk() exists but is not executed. In chk_rst_val(): for i=0..CNT-1, addr=addr_array[i]; if (read_mask_array[i]==0x00000000) skip reading (DEBUG: \"RST : This address 0x%x is not readable, hence skipped for reading\"); if (addr_array[i]==mizar_PCIE1_SII_PHY_RST_CONTROL) skip default check; else data_rd=read_reg(addr); if (data_rd==default_value_array[i]) optionally print PASS under DEBUG_DISPLAY; else def_fail_cnt++ and print \"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\\tRead_data : 0x%x\". In chk_rd_wr(): patterns chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; For each pattern j: data_wr=chk_val[j]. Writing phase: for i=0..CNT-1, addr=addr_array[i]; if (skip_array[i]==1) skip (DEBUG: skipped due to skip_array); if (write_mask_array[i]==0x00000000) skip (DEBUG: not writable); else write_reg(addr,data_wr) (DEBUG: print address and data). Reading phase: for i=0..CNT-1, addr=addr_array[i]; if (skip_array[i]==1) skip; if (write_mask_array[i]==0x00000000) skip (DEBUG: not writable); if (read_mask_array[i]==0x00000000) skip (DEBUG: not readable); else data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) optionally print PASS under DEBUG_DISPLAY; else wr_fail_cnt++ and print \"Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\\tRead value=0x%x\". At end of test_case(): if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0).",
    "Hidden_Remarks": "Default value check skips unreadable addresses (read_mask_array[i]==0x00000000). Default value check excludes mizar_PCIE1_SII_PHY_RST_CONTROL. Write phase skips when skip_array[i]==1 or write_mask_array[i]==0x00000000. Read/verify phase skips when skip_array[i]==1 or write_mask_array[i]==0x00000000 or read_mask_array[i]==0x00000000. soft_reset_chk() is present but commented out and not executed.",
    "Hidden_Test_Steps_Procedure": "test_case(): call chk_rst_val(); optional DEBUG banner; call chk_rd_wr(); optional DEBUG banner; // soft_reset_chk() not called; if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0). chk_rst_val(): for (i=0;i<CNT;i++): addr=addr_array[i]; if (read_mask_array[i]==0x00000000) {optional DEBUG print and continue}; if (addr_array[i]==mizar_PCIE1_SII_PHY_RST_CONTROL) {continue}; data_rd=read_reg(addr); if (data_rd==default_value_array[i]) {optional DEBUG PASS}; else {def_fail_cnt++; printf(\"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\\tRead_data : 0x%x\\n\",addr,default_value_array[i],data_rd);}. chk_rd_wr(): int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; for (j=0;j<6;j++): data_wr=chk_val[j]; // Write phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if (skip_array[i]==1) {optional DEBUG skip; continue}; if (write_mask_array[i]==0x00000000) {optional DEBUG not writable; continue}; write_reg(addr,data_wr); {optional DEBUG write details}. // Read/verify phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if (skip_array[i]==1) {optional DEBUG skip; continue}; if (write_mask_array[i]==0x00000000) {optional DEBUG not writable; continue}; if (read_mask_array[i]==0x00000000) {optional DEBUG not readable; continue}; data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) {optional DEBUG PASS}; else {wr_fail_cnt++; printf(\"Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\\tRead value=0x%x\\n\",addr,exp_val,data_rd);}.",
    "Hidden_Impacted_Registers": "mizar_PCIE1_SII_CFG_BAR0_START1, mizar_PCIE1_SII_CFG_BAR0_START2, mizar_PCIE1_SII_CFG_BAR0_LIMIT1, mizar_PCIE1_SII_CFG_BAR0_LIMIT2, ... (and many more as listed previously)",
    "Hidden_Validation_Acceptance_Criteria": "Default check: For each i where read_mask_array[i] != 0x00000000 and addr_array[i] != mizar_PCIE1_SII_PHY_RST_CONTROL, require read_reg(addr_array[i]) == default_value_array[i]; on mismatch increment def_fail_cnt and log failure. Write-read check: For each pattern in {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000} and for each i where skip_array[i] == 0 and write_mask_array[i] != 0x00000000 and read_mask_array[i] != 0x00000000: after write_reg(addr_array[i], data_wr), require read_reg(addr_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])); on mismatch increment wr_fail_cnt and log error. Final result: if (def_fail_cnt == 0 && wr_fail_cnt == 0) finish(0) PASS; else finish(1) FAIL."
  },
  {
    "Index": 7,
    "SS / Module": "PCIE",
    "Feature": "PCIe configuration space R/W and BAR probing",
    "Test Case Name": "pcie_cfg_wr_rd_test",
    "Test Description": "Programs coherency control settings and exercises PCIe configuration-space read/write flows, polling status and a handshake to determine completion.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Build-time flags select link-training role and instance. Status is polled until ready. External handshake value is required to complete.",
    "Test Steps / Procedure": "1) Initialize: write the test control register to clear state. 2) Program DBI_DSP_COHERENCY_CONTROL_3_OFF fields for instance 0 and instance 1. 3) Poll the SII status register until the ready bits indicate completion. 4) Configure PCIe configuration space registers including BARs; read back values and enable memory, I/O, and bus master. 5) Wait for system synchronization via the test control register and then finish.",
    "Impacted Registers": "DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "Validation / Acceptance Criteria": "1) SII status ready condition is observed during polling → Proceed with configuration. 2) Final synchronization value matches the expected pattern → Test passes.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie_cfg_wr_rd_test",
    "Hidden_Test_Description": "Performs optional link training based on defines; programs coherency control fields in DBI coherency control register 3 for both PCIe instances; polls SII0 (and SII1 if enabled) status until ready; writes handshake 0x11111111 then polls 0xE6004100 until it equals 0x12345678; for RC instances, probes and programs BARs and enables memory/IO/bus master; finally finish(0).",
    "Hidden_Remarks": "Compile-time flags (DM0_RC, DM1_RC, DM0_EP, DM1_EP) determine which link-training and configuration paths run. SII readiness is determined by (data_rd & 0xD1) == 0xD1 on offset 0xC0. The handshake register at 0xE6004100 must eventually read 0x12345678.",
    "Hidden_Test_Steps_Procedure": "1) write_reg(0xE6004100, 0x0). 2) Optionally perform link training. 3) Program coherency control fields on PCIE0 and PCIE1 DBI DSP coherency control 3 registers. 4) Poll SII status for readiness. 5) write_reg(0xE6004100, 0x11111111) and wait; under RC, probe and program BARs and enable features; 6) Poll 0xE6004100 until 0x12345678; 7) finish(0).",
    "Hidden_Impacted_Registers": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "Hidden_Validation_Acceptance_Criteria": "Polling read of SII0 register 0xC0 until ((data_rd & 0xD1) == 0xD1); if DM1_RC, similarly poll SII1 0xC0. Final loop requires read_reg(0xE6004100) to become 0x12345678 before exiting. Successful completion calls finish(0)."
  }
]'''

# Column definitions
META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_ORDER = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation (Required / Not)",
]

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

VALIDATION_COL = "Code Generation (Required / Not)"
VALIDATION_LIST = "Required,Blank,Not Required"

HEADER_FILL = PatternFill(start_color="FF4F81BD", end_color="FF4F81BD", fill_type="solid")
HEADER_FONT = Font(bold=True, color="FFFFFFFF")
HEADER_ALIGN = Alignment(horizontal="center", vertical="center", wrap_text=True)
DATA_ALIGN_LEFT = Alignment(horizontal="left", vertical="top", wrap_text=False)
DATA_ALIGN_LEFT_WRAP = Alignment(horizontal="left", vertical="top", wrap_text=True)
DATA_ALIGN_CENTER = Alignment(horizontal="center", vertical="top", wrap_text=False)
THIN_BORDER = Border(left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin"))


def fail(msg: str):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def parse_json(data_str: str):
    try:
        data = json.loads(data_str)
    except Exception as e:
        fail(f"Invalid JSON: {e}")
    if not isinstance(data, list) or len(data) == 0:
        fail("JSON must be a non-empty array of objects")
    # Validate all rows are dicts
    for i, row in enumerate(data, 1):
        if not isinstance(row, dict):
            fail(f"Row {i} is not an object")
    return data


def union_keys_preserve_order(rows):
    seen = []
    s = set()
    for row in rows:
        for k in row.keys():
            if k not in s:
                s.add(k)
                seen.append(k)
    return seen


def normalize_rows(rows, all_keys):
    norm = []
    for r in rows:
        norm.append({k: r.get(k, "") if r.get(k, None) is not None else "" for k in all_keys})
    return norm


def renumber_multiline(text: str) -> str:
    if text is None:
        return ""
    # Normalize newlines and split
    lines = [ln.strip() for ln in str(text).replace("\r\n", "\n").replace("\r", "\n").split("\n")]
    lines = [ln for ln in lines if ln != ""]
    if not lines:
        return ""
    numbered = [f"{i+1}. {ln}" for i, ln in enumerate(lines)]
    return "\n".join(numbered)


def write_base_sheet(wb: Workbook, rows, all_keys):
    ws = wb.active
    ws.title = "Data"
    # Header
    for c, key in enumerate(all_keys, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
        cell.border = THIN_BORDER
    # Data
    for r_idx, row in enumerate(rows, 2):
        for c, key in enumerate(all_keys, 1):
            val = row.get(key, "")
            ws.cell(row=r_idx, column=c, value=val).border = THIN_BORDER
    ws.freeze_panes = "A2"
    return ws


def autofit_columns(ws):
    # Estimate width by max string length
    for col_idx, col in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), 1):
        max_len = 0
        for cell in col:
            val = "" if cell.value is None else str(cell.value)
            if len(val) > max_len:
                max_len = len(val)
        adj = max_len + 2
        ws.column_dimensions[get_column_letter(col_idx)].width = min(max(10, adj), 120)


def style_headers(ws):
    for cell in ws[1]:
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
        cell.border = THIN_BORDER


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = THIN_BORDER


def make_meta_sheet(wb: Workbook, rows):
    meta = wb.create_sheet("Meta_data_sheet")
    # Header
    for c, key in enumerate(META_COLS, 1):
        cell = meta.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
        cell.border = THIN_BORDER
    # Data
    for r_idx, row in enumerate(rows, 2):
        for c, key in enumerate(META_COLS, 1):
            val = row.get(key, "")
            meta.cell(row=r_idx, column=c, value=val).border = THIN_BORDER
    # Very hidden
    meta.sheet_state = "veryHidden"


def reorder_to_main_inplace(ws, rows):
    # Clear sheet and write only MAIN_ORDER columns
    ws.delete_rows(1, ws.max_row)
    # Header
    for c, key in enumerate(MAIN_ORDER, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
        cell.border = THIN_BORDER
    # Data
    for r_idx, src in enumerate(rows, 2):
        for c, key in enumerate(MAIN_ORDER, 1):
            val = src.get(key, "")
            ws.cell(row=r_idx, column=c, value=val).border = THIN_BORDER
    ws.freeze_panes = "A2"


def apply_wrapping_and_alignment(ws):
    # Determine column index map
    header = [cell.value for cell in ws[1]]
    colmap = {name: idx+1 for idx, name in enumerate(header)}
    max_row = ws.max_row

    for r in range(2, max_row+1):
        for name in header:
            c = colmap[name]
            cell = ws.cell(row=r, column=c)
            if name == "Index":
                cell.alignment = DATA_ALIGN_CENTER
            elif name in WRAP_COLS:
                cell.alignment = DATA_ALIGN_LEFT_WRAP
            else:
                cell.alignment = DATA_ALIGN_LEFT

    # Renumber required columns
    for name in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        if name in colmap:
            c = colmap[name]
            for r in range(2, max_row+1):
                val = ws.cell(row=r, column=c).value
                ws.cell(row=r, column=c, value=renumber_multiline(val))

    # Rough row height based on number of lines in wrapped columns
    for r in range(2, max_row+1):
        max_lines = 1
        for name in WRAP_COLS:
            if name in colmap:
                val = ws.cell(row=r, column=colmap[name]).value
                if val is None:
                    continue
                lines = str(val).count("\n") + 1
                if lines > max_lines:
                    max_lines = lines
        # Approximate 15 pts per line
        ws.row_dimensions[r].height = min(15 * max_lines + 2, 409)

    style_headers(ws)
    apply_borders(ws)
    autofit_columns(ws)


def apply_data_validation(ws):
    header = [cell.value for cell in ws[1]]
    if VALIDATION_COL not in header:
        return
    col_idx = header.index(VALIDATION_COL) + 1
    max_row = ws.max_row
    dv = DataValidation(type="list", formula1=f'"{VALIDATION_LIST}"', allow_blank=True, showErrorMessage=True)
    rng = f"{get_column_letter(col_idx)}2:{get_column_letter(col_idx)}{max_row}"
    dv.add(rng)
    ws.add_data_validation(dv)


def ensure_visibility_and_cleanup(wb: Workbook):
    # Rename Data to TestPlan (it must already be the active sheet)
    data_ws = wb[wb.sheetnames[0]]
    if data_ws.title != "TestPlan":
        data_ws.title = "TestPlan"
    # Ensure no sheet named Data remains
    if "Data" in wb.sheetnames:
        # If some other sheet named Data exists, delete it
        if wb["Data"].title == "Data":
            wb.remove(wb["Data"])
    # Ensure only TestPlan (visible) and Meta_data_sheet (veryHidden) exist
    allowed = {"TestPlan", "Meta_data_sheet"}
    for name in list(wb.sheetnames):
        if name not in allowed:
            # Do not delete Meta_data_sheet or TestPlan; others should not exist
            if name != "Meta_data_sheet" and name != "TestPlan":
                wb.remove(wb[name])


def main():
    rows_in = parse_json(JSON_DATA)
    # Compute union of keys preserving order
    all_keys = union_keys_preserve_order(rows_in)
    rows = normalize_rows(rows_in, all_keys)

    # Build workbook
    wb = Workbook()
    ws = write_base_sheet(wb, rows, all_keys)

    # Create Meta sheet (raw META columns)
    make_meta_sheet(wb, rows)

    # Rename staging sheet directly and reorder columns (in-place on same sheet)
    ws.title = "TestPlan"  # will be enforced again later
    reorder_to_main_inplace(ws, rows)

    # Apply wrapping, alignment, borders, sizing, and numbering
    apply_wrapping_and_alignment(ws)

    # Data validation on the specific column
    apply_data_validation(ws)

    # Final visibility enforcement
    ensure_visibility_and_cleanup(wb)

    # Save
    os.makedirs(OUTPUT_PATH, exist_ok=True)
    out_file = os.path.join(OUTPUT_PATH, OUTPUT_FILENAME)
    wb.save(out_file)

    # Validate XLSX as ZIP-based OOXML
    if not zipfile.is_zipfile(out_file):
        fail("Generated file is not a valid XLSX (zipfile check failed)")
    try:
        _ = load_workbook(out_file, read_only=True, data_only=True)
    except Exception as e:
        fail(f"Generated XLSX failed openpyxl validation: {e}")

    print(f"OK: Saved {out_file}")


if __name__ == "__main__":
    main()
