#!/usr/bin/env python3
"""
Agent 7: Excel Generation Script for MIPI_DSI TestPlan
Generates MIPI_DSI_TestPlan_20261113_143000.xlsx with TestPlan and MetaData sheets.
MetaData sheet is set to veryHidden.
TestPlan sheet has Code Generation dropdown validation.
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter
import json, os, sys, base64, requests
from datetime import datetime, timezone, timedelta

GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")
OWNER = "titusbspgit"
REPO = "PSVValidation"
BRANCH = "main"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"
FILENAME = "MIPI_DSI_TestPlan_20261113_143000.xlsx"
FILEPATH = os.path.join(OUTPUT_DIR, FILENAME)

META_JSON = [
    {
        "Test Case Name": "chk_rd_wr",
        "Meta Headers": "File: program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include \"test_define.c\"\n4. #include \"test_common.h\"\n\nFile: test_define.c\n1. #include<mipi_dsi_subsys.h>",
        "Meta Macros": "File: test_define.c\n1. #define CNT 52\n\nFile: program.c\n1. #define SOFT_RST_REG_ADDRESS 0x00000000\n2. #ifdef DEBUG_DISPLAY",
        "Meta Arrays": "File: test_define.c\n\n1. const unsigned long int addr_array[52]={...}\n2. const int default_value_array[52]={...}\n3. const int read_mask_array[52]={...}\n4. const int skip_array[52]={...}\n\nFile: program.c (local to chk_rd_wr)\n5. int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};",
        "Meta Types": "TYPE_DEFINITIONS\nNo typedef, struct, union, or enum definitions visible.\n\nOTHER_DECLARATIONS\nFile: test_define.c\n1. int data_rd,data_wr,rst_val;\n\nFile: program.c\n1. int data_rd,data_wr;\n2. int def_fail_cnt = 0,wr_fail_cnt = 0;\n\nFile: program.c (local to chk_rd_wr)\n1. unsigned long int i,addr,j,exp_val,wr_n;\n2. int chk_val[6]={...};",
        "Meta Test Description": "The chk_rd_wr function is a DV register write-read verification test for the MIPI DSI subsystem.",
        "Meta Test Steps / Procedure": "[source code of chk_rd_wr and test_define.c]",
        "Meta Impacted Registers": "52 registers of the dsi_subsys block at base 0xE6A40000 + 0x1000",
        "Meta Validation / Acceptance Criteria": "Skip Array Check, Write Mask Zero Check, Write-Read Comparison, Overall Test Pass/Fail"
    },
    {
        "Test Case Name": "chk_rst_val",
        "Meta Headers": "File: program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include \"test_define.c\"\n4. #include \"test_common.h\"\n\nFile: test_define.c\n1. #include<mipi_dsi_subsys.h>",
        "Meta Macros": "File: test_define.c\n1. #define CNT 52\n\nFile: program.c\n1. #define SOFT_RST_REG_ADDRESS 0x00000000\n2. #ifdef DEBUG_DISPLAY",
        "Meta Arrays": "File: test_define.c\n\n1. const unsigned long int addr_array[52]={...}\n2. const int default_value_array[52]={...}\n3. const int read_mask_array[52]={...}\n4. const int skip_array[52]={...}",
        "Meta Types": "TYPE_DEFINITIONS\nNo typedef, struct, union, or enum definitions visible.\n\nOTHER_DECLARATIONS\nFile: test_define.c\n1. int data_rd,data_wr,rst_val;\n\nFile: program.c\n1. int data_rd,data_wr;\n2. int def_fail_cnt = 0,wr_fail_cnt = 0;\n\nFile: program.c (local to chk_rst_val)\n1. unsigned long int i,addr;",
        "Meta Test Description": "The chk_rst_val function is a DV register default/reset value verification test for the MIPI DSI subsystem.",
        "Meta Test Steps / Procedure": "[source code of chk_rst_val and test_define.c]",
        "Meta Impacted Registers": "52 registers of the dsi_subsys block at base 0xE6A40000 + 0x1000",
        "Meta Validation / Acceptance Criteria": "Read Mask Zero Check, Default Value Comparison, Overall Test Pass/Fail"
    },
    {
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Meta Headers": "File: program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include \"test_define.c\"\n4. #include \"test_common.h\"\n\nFile: test_define.c\n1. #include<mipi_dsi_subsys.h>",
        "Meta Macros": "File: test_define.c\n1. #define CNT 52\n\nFile: program.c\n1. #define SOFT_RST_REG_ADDRESS 0x00000000\n2. #ifdef DEBUG_DISPLAY",
        "Meta Arrays": "[same arrays as chk_rd_wr plus chk_val]",
        "Meta Types": "[combined types from chk_rst_val and chk_rd_wr]",
        "Meta Test Description": "The mipi_dsi_subsys_reg_wr_rd_test is the top-level DV register verification test for the MIPI DSI subsystem.",
        "Meta Test Steps / Procedure": "[source code of all functions]",
        "Meta Impacted Registers": "52 registers of the dsi_subsys block at base 0xE6A40000 + 0x1000",
        "Meta Validation / Acceptance Criteria": "Read Mask Zero Check, Default Value Comparison, Skip Array Check, Write Mask Zero Check, Write-Read Comparison, Overall Test Pass/Fail"
    },
    {
        "Test Case Name": "soft_reset_chk",
        "Meta Headers": "File: program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include \"test_define.c\"\n4. #include \"test_common.h\"\n\nFile: test_define.c\n1. #include<mipi_dsi_subsys.h>",
        "Meta Macros": "File: program.c\n1. #define SOFT_RST_REG_ADDRESS 0x00000000",
        "Meta Arrays": "NA",
        "Meta Types": "TYPE_DEFINITIONS\nNo typedef.\n\nOTHER_DECLARATIONS\nFile: program.c\n1. int data_rd,data_wr;\n2. int def_fail_cnt = 0,wr_fail_cnt = 0;\n\nFile: program.c (local to soft_reset_chk)\n1. int default_value;",
        "Meta Test Description": "The soft_reset_chk function is a DV soft reset sequence test for the MIPI DSI subsystem.",
        "Meta Test Steps / Procedure": "[source code of soft_reset_chk]",
        "Meta Impacted Registers": "0 matched, 2 unresolved (SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA)",
        "Meta Validation / Acceptance Criteria": "No explicit validation within the function."
    }
]

TESTPLAN_JSON = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "chk_rd_wr",
        "Feature": "Register Write-Read Verification",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase verifies the write-read accessibility of all 52 registers in the MIPI DSI subsystem. The test writes six distinct test patterns to each writable register and reads back the value to confirm correct data retention. The expected read-back value is computed using the write and read masks for each register. Registers that are flagged as non-writable or marked for skipping are excluded from the verification. The test confirms that every writable register correctly accepts and returns data consistent with its defined write and read mask properties.",
        "Test Steps / Procedure": "1. Initialize the write-read failure counter to zero.\n2. For each of the six predefined test patterns, perform the following:\n   a. Select the current test pattern as the write data.\n   b. Iterate over all 52 MIPI DSI subsystem registers.\n   c. For each register, check if it is flagged in the skip list; if so, skip it.\n   d. For each register, check if its write mask indicates it is non-writable; if so, skip it.\n   e. Write the current test pattern to the register.\n   f. Read back the register value.\n   g. Compute the expected value using the write mask and read mask for the register.\n   h. Compare the read-back value against the expected value.\n   i. If the values match, record a pass for that register and pattern.\n   j. If the values do not match, increment the failure counter and log the mismatch details.\n3. After all patterns and registers have been processed, evaluate the overall result.\n4. If the failure counter is zero, the test passes; otherwise, the test fails.",
        "Impacted Registers": "data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
        "Validation / Acceptance Criteria": "1. Registers flagged in the skip list must be correctly excluded from write-read verification.\n2. Registers with a non-writable write mask must be correctly excluded from write-read verification.\n3. For each writable, non-skipped register and each test pattern, the read-back value must match the expected value computed from the write and read masks.\n4. Any mismatch between the read-back value and the expected value must be recorded as a failure.\n5. The overall test passes only if zero write-read mismatches are detected across all six test patterns and all applicable registers; otherwise the test fails.",
        "Remarks": "Six test patterns are used to exercise various bit combinations across all writable registers. The expected value computation accounts for differing write and read masks per register. Certain registers are intentionally skipped based on a predefined skip list. The exact expected value computation logic for registers where write mask differs from read mask was not fully visible in the supplied source excerpts."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "chk_rst_val",
        "Feature": "Register Default/Reset Value Verification",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase verifies that all 52 registers in the MIPI DSI subsystem contain their expected default/reset values after reset. The test iterates over each register, first checking whether the register is readable based on its read mask. Registers that are not readable are skipped. For each readable register, the test reads the current value and compares it against the expected default value. Any mismatch between the read value and the expected default value is recorded as a failure. The test confirms that the MIPI DSI subsystem registers are correctly initialized to their specification-defined reset values.",
        "Test Steps / Procedure": "1. Initialize the default value failure counter to zero.\n2. Iterate over all 52 MIPI DSI subsystem registers.\n3. For each register, check if its read mask indicates it is not readable; if so, skip that register.\n4. For each readable register, read the current register value.\n5. Compare the read value against the expected default value for that register.\n6. If the read value matches the expected default value, record a pass for that register.\n7. If the read value does not match the expected default value, increment the failure counter and log the mismatch details including the expected and actual values.\n8. After all registers have been processed, evaluate the overall result.\n9. If the failure counter is zero, the test passes; otherwise, the test fails.",
        "Impacted Registers": "data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
        "Validation / Acceptance Criteria": "1. Registers with a non-readable read mask must be correctly excluded from default value verification.\n2. For each readable register, the read value must exactly match the expected default/reset value defined for that register.\n3. Any mismatch between the read value and the expected default value must be recorded as a failure.\n4. The overall test passes only if zero default value mismatches are detected across all readable registers; otherwise the test fails.",
        "Remarks": "All 52 registers in the MIPI DSI subsystem are checked for their default values after reset. Non-readable registers are automatically skipped based on their read mask configuration."
    },
    {
        "Index": "3",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "Top-Level Register Verification (Default Value Check + Write-Read Check)",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase is the top-level DV register verification test for the MIPI DSI subsystem. It orchestrates two sub-tests in sequence: first, a default/reset value verification that reads all 52 registers and confirms each readable register contains its expected reset value; second, a write-read verification that writes six distinct test patterns to each writable register and reads back to confirm correct data retention. Registers that are not readable are skipped during the default value check, and registers that are non-writable or flagged for skipping are excluded during the write-read check. The test determines overall pass or fail based on accumulated failure counts from both sub-tests: if any default value mismatch or any write-read mismatch is detected, the test fails; otherwise it passes.",
        "Test Steps / Procedure": "1. Initialize the default value failure counter and the write-read failure counter both to zero.\n2. Execute the default/reset value verification sub-test:\n   a. Iterate over all 52 MIPI DSI subsystem registers.\n   b. For each register, check if its read mask indicates it is not readable; if so, skip that register.\n   c. For each readable register, read the current register value.\n   d. Compare the read value against the expected default/reset value for that register.\n   e. If the values match, record a pass for that register.\n   f. If the values do not match, increment the default value failure counter and log the mismatch details.\n3. Execute the write-read verification sub-test:\n   a. For each of the six predefined test patterns, perform the following:\n      i. Select the current test pattern as the write data.\n      ii. Iterate over all 52 MIPI DSI subsystem registers.\n      iii. For each register, check if it is flagged in the skip list; if so, skip it.\n      iv. For each register, check if its write mask indicates it is non-writable; if so, skip it.\n      v. Write the current test pattern to the register.\n      vi. Read back the register value.\n      vii. Compute the expected value using the write mask and read mask for the register.\n      viii. Compare the read-back value against the expected value.\n      ix. If the values match, record a pass for that register and pattern.\n      x. If the values do not match, increment the write-read failure counter and log the mismatch details.\n4. Evaluate the overall test result: if either the default value failure counter or the write-read failure counter is greater than zero, the test fails; otherwise the test passes.",
        "Impacted Registers": "data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
        "Validation / Acceptance Criteria": "1. During the default value check, registers with a non-readable read mask must be correctly excluded from verification.\n2. For each readable register, the read value must exactly match the expected default/reset value defined for that register.\n3. During the write-read check, registers flagged in the skip list must be correctly excluded from verification.\n4. During the write-read check, registers with a non-writable write mask must be correctly excluded from verification.\n5. For each writable, non-skipped register and each test pattern, the read-back value must match the expected value computed from the write and read masks.\n6. Any default value mismatch or write-read mismatch must be recorded as a failure.\n7. The overall test passes only if zero failures are detected across both the default value check and the write-read check for all applicable registers; otherwise the test fails.",
        "Remarks": "This is the top-level test orchestrator that sequences the default value verification and write-read verification sub-tests. Six test patterns are used during the write-read phase to exercise various bit combinations. The expected value computation during write-read accounts for differing write and read masks per register. Certain registers are intentionally skipped during write-read based on a predefined skip list. The exact expected value computation logic for registers where write mask differs from read mask was not fully visible in the supplied source excerpts."
    },
    {
        "Index": "4",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "soft_reset_chk",
        "Feature": "Soft Reset Sequence Verification",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase verifies the soft reset sequence for the MIPI DSI subsystem. The test reads the current value of the soft reset register to preserve its default state, then writes a predefined reset data value to the soft reset register to assert the soft reset. After a stabilization wait period, the test writes back the original saved value to the soft reset register to de-assert the reset, followed by another stabilization wait period. This confirms that the soft reset assertion and de-assertion sequence can be executed on the MIPI DSI subsystem.",
        "Test Steps / Procedure": "1. Read the current value of the soft reset register and save it as the default value.\n2. Write the predefined reset data value to the soft reset register to assert the soft reset.\n3. Wait for a stabilization period to allow the soft reset to take effect.\n4. Write the previously saved default value back to the soft reset register to de-assert the soft reset.\n5. Wait for a stabilization period to allow the subsystem to return to normal operation.",
        "Impacted Registers": "NA",
        "Validation / Acceptance Criteria": "No explicit pass/fail validation is performed within this testcase. The testcase executes the soft reset assertion and de-assertion sequence only. Validation of the soft reset effect is handled by the calling test context, which evaluates overall pass/fail based on accumulated failure counters from other sub-tests.",
        "Remarks": "The soft reset register macros used in this testcase could not be resolved to specific register names from the IP specification. The testcase performs a reset sequence without internal validation; it relies on the top-level test orchestrator for overall pass/fail determination."
    }
]

TESTPLAN_COLUMNS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks",
    "Code Generation (Required / Not)"
]

METADATA_COLUMNS = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

def generate_workbook():
    wb = openpyxl.Workbook()

    # ── TestPlan sheet ──
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap = Alignment(wrap_text=True, vertical="top")

    for ci, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        cell = ws_tp.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap

    for ri, rec in enumerate(TESTPLAN_JSON, 2):
        for ci, col_name in enumerate(TESTPLAN_COLUMNS, 1):
            if col_name == "Code Generation (Required / Not)":
                val = ""
            else:
                val = rec.get(col_name, "")
            cell = ws_tp.cell(row=ri, column=ci, value=val)
            cell.alignment = wrap

    # Data validation dropdown for Code Generation column
    code_gen_col = len(TESTPLAN_COLUMNS)
    dv = DataValidation(
        type="list",
        formula1='"Required,Not Required"',
        allow_blank=True,
        showDropDown=False
    )
    dv.error = "Please select Required or Not Required"
    dv.errorTitle = "Invalid Entry"
    dv.prompt = "Select Code Generation status"
    dv.promptTitle = "Code Generation"
    first_data_row = 2
    last_data_row = 1 + len(TESTPLAN_JSON)
    col_letter = get_column_letter(code_gen_col)
    dv.add(f"{col_letter}{first_data_row}:{col_letter}{last_data_row}")
    ws_tp.add_data_validation(dv)

    # Auto-size columns (with max width cap)
    for ci in range(1, len(TESTPLAN_COLUMNS) + 1):
        max_len = len(str(ws_tp.cell(row=1, column=ci).value or ""))
        for ri in range(2, last_data_row + 1):
            val = str(ws_tp.cell(row=ri, column=ci).value or "")
            for line in val.split("\n"):
                max_len = max(max_len, len(line))
        ws_tp.column_dimensions[get_column_letter(ci)].width = min(max_len + 2, 60)

    ws_tp.freeze_panes = "A2"

    # ── MetaData sheet ──
    ws_md = wb.create_sheet("MetaData")
    for ci, col_name in enumerate(METADATA_COLUMNS, 1):
        cell = ws_md.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap

    for ri, rec in enumerate(META_JSON, 2):
        for ci, col_name in enumerate(METADATA_COLUMNS, 1):
            val = rec.get(col_name, "")
            cell = ws_md.cell(row=ri, column=ci, value=val)
            cell.alignment = wrap

    for ci in range(1, len(METADATA_COLUMNS) + 1):
        max_len = len(str(ws_md.cell(row=1, column=ci).value or ""))
        for ri in range(2, 2 + len(META_JSON)):
            val = str(ws_md.cell(row=ri, column=ci).value or "")
            for line in val.split("\n"):
                max_len = max(max_len, len(line))
        ws_md.column_dimensions[get_column_letter(ci)].width = min(max_len + 2, 60)

    ws_md.freeze_panes = "A2"
    ws_md.sheet_state = "veryHidden"

    # ── Save ──
    wb.save(FILENAME)
    print(f"Workbook saved: {FILENAME}")
    print(f"File size: {os.path.getsize(FILENAME)} bytes")

    # ── Validate ──
    wb2 = openpyxl.load_workbook(FILENAME)
    assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in wb2.sheetnames, "MetaData sheet missing"
    assert wb2["MetaData"].sheet_state == "veryHidden", "MetaData not veryHidden"
    tp_rows = wb2["TestPlan"].max_row - 1
    md_rows = wb2["MetaData"].max_row - 1
    assert tp_rows == len(TESTPLAN_JSON), f"TestPlan rows {tp_rows} != {len(TESTPLAN_JSON)}"
    assert md_rows == len(META_JSON), f"MetaData rows {md_rows} != {len(META_JSON)}"
    print(f"Validation PASSED: TestPlan={tp_rows} rows, MetaData={md_rows} rows")
    wb2.close()

    # ── Upload to GitHub ──
    with open(FILENAME, "rb") as f:
        content_b64 = base64.b64encode(f.read()).decode("utf-8")

    url = f"https://api.github.com/repos/{OWNER}/{REPO}/contents/{FILEPATH}"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }

    # Check if file exists
    resp = requests.get(url, headers=headers, params={"ref": BRANCH})
    sha = None
    if resp.status_code == 200:
        sha = resp.json().get("sha")

    payload = {
        "message": "Generated TestPlan Excel Workbook",
        "content": content_b64,
        "branch": BRANCH
    }
    if sha:
        payload["sha"] = sha

    resp = requests.put(url, headers=headers, json=payload)
    if resp.status_code in (200, 201):
        data = resp.json()
        commit_sha = data["commit"]["sha"]
        html_url = data["content"]["html_url"]
        print(f"Upload SUCCESS: {html_url}")
        print(f"Commit SHA: {commit_sha}")
        result = {
            "status": "SUCCESS",
            "output_file_path": FILEPATH,
            "github_url": html_url,
            "filename": FILENAME,
            "rows_testplan": tp_rows,
            "rows_metadata": md_rows,
            "commit_sha": commit_sha,
            "validation": "PASSED"
        }
    else:
        print(f"Upload FAILED: {resp.status_code} {resp.text}")
        result = {
            "status": "FAILURE",
            "output_file_path": FILENAME,
            "github_url": "",
            "filename": FILENAME,
            "rows_testplan": tp_rows,
            "rows_metadata": md_rows,
            "commit_sha": "",
            "validation": "PASSED"
        }

    print(json.dumps(result, indent=2))
    return result

if __name__ == "__main__":
    generate_workbook()
