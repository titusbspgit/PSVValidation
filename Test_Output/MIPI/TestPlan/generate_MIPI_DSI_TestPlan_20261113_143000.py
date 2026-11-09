#!/usr/bin/env python3
"""Agent 7 - MIPI_DSI TestPlan Excel Generator & GitHub Publisher
Run: pip install openpyxl requests && python generate_MIPI_DSI_TestPlan_20261113_143000.py
Set GITHUB_TOKEN env var before running.
"""
import json, os, sys, base64, requests
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# ── CONFIG ──
IP_NAME = "MIPI_DSI"
OWNER = "titusbspgit"
REPO = "PSVValidation"
BRANCH = "main"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"
IST = timezone(timedelta(hours=5, minutes=30))
TIMESTAMP = "20261113_143000"
FILENAME = f"{IP_NAME}_TestPlan_{TIMESTAMP}.xlsx"
GITHUB_PATH = f"{OUTPUT_DIR}/{FILENAME}"
TOKEN = os.environ.get("GITHUB_TOKEN", "")

# ── EMBEDDED DATA ──
META_JSON = [{"Test Case Name":"mipi_dsi_subsys_reg_wr_rd_test","Meta Headers":"File: test_define.c\n#include<mipi_dsi_subsys.h>\n\nFile: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"","Meta Macros":"File: test_define.c\n#define CNT 52\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n\nConditional compilation macro referenced via #ifdef but definition not supplied in source:\nDEBUG_DISPLAY","Meta Arrays":"File: test_define.c (file scope)\n\nconst unsigned long int addr_array[52]={MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,...,MIZAR_MIPI_DSI_SUBSYS_DSI_HOST};\nconst int default_value_array[52]={...};\nconst int read_mask_array[52]={...};\nconst int write_mask_array[52]={...};\nconst int skip_array[52]={0,0,0,0,1,0,0,0,0,0,...};\n\nFile: program.c (local to chk_rd_wr function)\nint chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};","Meta Types":"TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;\n\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;","Meta Test Description":"This testcase performs register-level verification for the MIPI DSI subsystem across 52 registers. It consists of two main phases: (1) Default/reset value check (chk_rst_val) - iterates over all 52 registers via addr_array, reads each register using read_reg(), and compares against expected default values from default_value_array, skipping non-readable registers where read_mask_array is 0x00000000, incrementing def_fail_cnt on mismatch; (2) Write/read-back check (chk_rd_wr) - writes 6 test patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000) to all writable registers and reads back to verify, skipping registers flagged in skip_array or with zero write_mask_array, computing expected value as exp_val = (data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]) where wr_n = write_mask_array[i] ^ 0xffffffff, incrementing wr_fail_cnt on mismatch. A soft_reset_chk function is defined but commented out in test_case(). The test calls finish(1) on any failure (def_fail_cnt > 0 or wr_fail_cnt > 0) or finish(0) on success.","Meta Test Steps / Procedure":"The full source code content from test_define.c and program.c as provided by Agent 5A.","Meta Impacted Registers":"52 unique registers: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0 through vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host","Meta Validation / Acceptance Criteria":"chk_rst_val: For each register, read value must match default_value_array[i]. chk_rd_wr: For each of 6 test patterns written, read-back must match exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Final: finish(0) if no failures, finish(1) otherwise."}]

TESTPLAN_JSON = [{"Index":"1","SS / Module":"MIPI_DSI","Test Case Name":"mipi_dsi_subsys_reg_wr_rd_test","Feature":"Register Read/Write Verification","Speed":"NA","Mode":"NA","Memory Start Offset":"NA","Memory End Offset":"NA","Test Description":"This test verifies register-level access for the MIPI DSI subsystem across 52 registers. It performs two main validation phases: (1) Default Value Check - reads all 52 registers after reset and compares each read value against its expected default/reset value, skipping registers that are not readable based on their read mask. Any mismatch is counted as a default value failure. (2) Write/Read-Back Check - writes six distinct test data patterns to all writable registers and reads back each register to verify correctness. Registers that are flagged to be skipped or have a zero write mask are excluded from the write/read-back phase. The expected read-back value accounts for writable and readable bit fields using the read mask, write mask, and default value of each register. Any mismatch is counted as a write/read failure. A soft reset check function is defined but is not invoked in the current test flow. The test reports overall pass if zero failures are detected across both phases, and overall fail otherwise.","Test Steps / Procedure":"Step 1: Initialize failure counters for default value check and write/read check to zero.\nStep 2: Execute the Default Value Check phase - iterate over all 52 MIPI DSI subsystem registers. For each register, check if the register is readable (read mask is non-zero). If not readable, skip to the next register. If readable, read the register and compare the read value against the expected default value. If a mismatch is detected, increment the default value failure counter and log the failure.\nStep 3: Execute the Write/Read-Back Check phase - for each of the six test data patterns (all-ones, alternating 0xAA, alternating 0x55, all-zeros, 0xA5A5A5A5, 0xFFFF0000), perform the following: (a) Write sub-phase: iterate over all 52 registers. Skip registers flagged in the skip list. Skip read-only registers (zero write mask). For all other registers, write the current test pattern. (b) Read-back sub-phase: iterate over all 52 registers again. Skip registers flagged in the skip list or that are read-only. Read each register and compute the expected value considering writable bits (via write mask), readable bits (via read mask), and default values for non-writable bit positions. Compare the read value against the computed expected value. If a mismatch is detected, increment the write/read failure counter and log the failure.\nStep 4: Evaluate overall result - if either the default value failure counter or the write/read failure counter is greater than zero, report test FAIL. Otherwise, report test PASS.","Impacted Registers":"data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host","Validation / Acceptance Criteria":"1. Default Value Verification: For each of the 52 registers, the value read after reset must exactly match the specified default value for that register. Registers with a zero read mask are excluded from this check.\n2. Write/Read-Back Verification: For each of the six test data patterns written, the value read back from each writable register must match the expected value computed as: expected = (written_data AND read_mask AND write_mask) OR (inverted_write_mask AND read_mask AND default_value). Registers in the skip list or with a zero write mask are excluded.\n3. Overall Pass Criteria: The test passes (finish with success) only if zero default value mismatches and zero write/read-back mismatches are detected across all registers and all test patterns. Any non-zero failure count results in test failure.","Remarks":"The test covers 52 MIPI DSI subsystem registers spanning data FIFO threshold, low power, DBI, interrupt, command FIFO, pixel data FIFO, DPI control, video parameters (0-8), FIFO flush, soft control reset, DMAC configuration, DPHY TX configuration, PHY lane, PLL configuration, and DSI host registers. Six test patterns are used to exercise bit toggling coverage. A soft reset check function exists in the test but is currently not invoked. Certain registers are flagged to be skipped during the write/read-back phase. Read-only registers are automatically excluded from write operations based on their write mask."}]

# ── CROSS-DATASET VALIDATION ──
def validate():
    meta_names = {r["Test Case Name"] for r in META_JSON}
    tp_names = {r["Test Case Name"] for r in TESTPLAN_JSON}
    if not tp_names.issubset(meta_names):
        print(f"CROSS-DATASET VALIDATION FAILED: {tp_names - meta_names}")
        return False
    return True

# ── WORKBOOK GENERATION ──
def generate_workbook():
    wb = Workbook()
    # ── TestPlan Sheet ──
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    tp_cols = ["Index","SS / Module","Feature","Test Case Name","Speed","Mode",
               "Memory Start Offset","Memory End Offset","Test Description",
               "Test Steps / Procedure","Impacted Registers",
               "Validation / Acceptance Criteria","Remarks",
               "Code Generation (Required / Not)"]
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap = Alignment(wrap_text=True, vertical="top")
    for ci, col_name in enumerate(tp_cols, 1):
        cell = ws_tp.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap
    for ri, row_data in enumerate(TESTPLAN_JSON, 2):
        ws_tp.cell(row=ri, column=1, value=row_data.get("Index","")).alignment = wrap
        ws_tp.cell(row=ri, column=2, value=row_data.get("SS / Module","")).alignment = wrap
        ws_tp.cell(row=ri, column=3, value=row_data.get("Feature","")).alignment = wrap
        ws_tp.cell(row=ri, column=4, value=row_data.get("Test Case Name","")).alignment = wrap
        ws_tp.cell(row=ri, column=5, value=row_data.get("Speed","")).alignment = wrap
        ws_tp.cell(row=ri, column=6, value=row_data.get("Mode","")).alignment = wrap
        ws_tp.cell(row=ri, column=7, value=row_data.get("Memory Start Offset","")).alignment = wrap
        ws_tp.cell(row=ri, column=8, value=row_data.get("Memory End Offset","")).alignment = wrap
        ws_tp.cell(row=ri, column=9, value=row_data.get("Test Description","")).alignment = wrap
        ws_tp.cell(row=ri, column=10, value=row_data.get("Test Steps / Procedure","")).alignment = wrap
        ws_tp.cell(row=ri, column=11, value=row_data.get("Impacted Registers","")).alignment = wrap
        ws_tp.cell(row=ri, column=12, value=row_data.get("Validation / Acceptance Criteria","")).alignment = wrap
        ws_tp.cell(row=ri, column=13, value=row_data.get("Remarks","")).alignment = wrap
        # Column 14 = Code Generation - leave blank
    # Data Validation dropdown
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True)
    dv.error = "Please select Required or Not Required"
    dv.errorTitle = "Invalid Entry"
    dv.prompt = "Select Code Generation status"
    dv.promptTitle = "Code Generation"
    last_row = 1 + len(TESTPLAN_JSON)
    dv.add(f"N2:N{last_row}")
    ws_tp.add_data_validation(dv)
    ws_tp.freeze_panes = "A2"
    # Auto-size columns
    for ci in range(1, len(tp_cols)+1):
        max_len = len(str(ws_tp.cell(row=1, column=ci).value))
        for ri in range(2, last_row+1):
            val = ws_tp.cell(row=ri, column=ci).value
            if val:
                max_len = max(max_len, min(len(str(val)), 80))
        ws_tp.column_dimensions[get_column_letter(ci)].width = min(max_len + 4, 80)

    # ── MetaData Sheet ──
    ws_md = wb.create_sheet("MetaData")
    ws_md.sheet_state = "veryHidden"
    md_cols = ["Test Case Name","Meta Headers","Meta Macros","Meta Arrays",
               "Meta Types","Meta Test Description","Meta Test Steps / Procedure",
               "Meta Impacted Registers","Meta Validation / Acceptance Criteria"]
    for ci, col_name in enumerate(md_cols, 1):
        cell = ws_md.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap
    for ri, row_data in enumerate(META_JSON, 2):
        ws_md.cell(row=ri, column=1, value=row_data.get("Test Case Name","")).alignment = wrap
        ws_md.cell(row=ri, column=2, value=row_data.get("Meta Headers","")).alignment = wrap
        ws_md.cell(row=ri, column=3, value=row_data.get("Meta Macros","")).alignment = wrap
        ws_md.cell(row=ri, column=4, value=row_data.get("Meta Arrays","")).alignment = wrap
        ws_md.cell(row=ri, column=5, value=row_data.get("Meta Types","")).alignment = wrap
        ws_md.cell(row=ri, column=6, value=row_data.get("Meta Test Description","")).alignment = wrap
        ws_md.cell(row=ri, column=7, value=row_data.get("Meta Test Steps / Procedure","")).alignment = wrap
        ws_md.cell(row=ri, column=8, value=row_data.get("Meta Impacted Registers","")).alignment = wrap
        ws_md.cell(row=ri, column=9, value=row_data.get("Meta Validation / Acceptance Criteria","")).alignment = wrap
    ws_md.freeze_panes = "A2"
    for ci in range(1, len(md_cols)+1):
        ws_md.column_dimensions[get_column_letter(ci)].width = 40

    wb.save(FILENAME)
    return FILENAME

# ── POST-SAVE VALIDATION ──
def post_validate(fname):
    if not os.path.exists(fname):
        return False, "File does not exist"
    if os.path.getsize(fname) == 0:
        return False, "File size is 0"
    wb2 = load_workbook(fname)
    if "TestPlan" not in wb2.sheetnames:
        return False, "TestPlan sheet missing"
    if "MetaData" not in wb2.sheetnames:
        return False, "MetaData sheet missing"
    ws_tp = wb2["TestPlan"]
    ws_md = wb2["MetaData"]
    tp_data_rows = ws_tp.max_row - 1
    md_data_rows = ws_md.max_row - 1
    if tp_data_rows != len(TESTPLAN_JSON):
        return False, f"TestPlan rows {tp_data_rows} != {len(TESTPLAN_JSON)}"
    if md_data_rows != len(META_JSON):
        return False, f"MetaData rows {md_data_rows} != {len(META_JSON)}"
    # Check Code Generation column header
    if ws_tp.cell(row=1, column=14).value != "Code Generation (Required / Not)":
        return False, "Code Generation column missing"
    # Check all Code Generation cells are blank
    for r in range(2, ws_tp.max_row + 1):
        if ws_tp.cell(row=r, column=14).value is not None:
            return False, f"Code Generation cell row {r} not blank"
    # Check data validations exist
    if len(ws_tp.data_validations.dataValidation) == 0:
        return False, "No data validations found"
    wb2.close()
    return True, "PASSED"

# ── GITHUB UPLOAD ──
def upload_to_github(fname):
    with open(fname, "rb") as f:
        content_b64 = base64.b64encode(f.read()).decode()
    url = f"https://api.github.com/repos/{OWNER}/{REPO}/contents/{GITHUB_PATH}"
    headers = {"Authorization": f"token {TOKEN}", "Accept": "application/vnd.github.v3+json"}
    # Check if file exists
    resp = requests.get(url, headers=headers, params={"ref": BRANCH})
    sha = None
    if resp.status_code == 200:
        sha = resp.json().get("sha")
    payload = {"message": "Generated TestPlan Excel Workbook", "content": content_b64, "branch": BRANCH}
    if sha:
        payload["sha"] = sha
    resp = requests.put(url, headers=headers, json=payload)
    if resp.status_code in (200, 201):
        data = resp.json()
        return data["content"]["html_url"], data["commit"]["sha"]
    else:
        print(f"Upload failed: {resp.status_code} {resp.text}")
        return None, None

# ── MAIN ──
if __name__ == "__main__":
    if not validate():
        print(json.dumps({"status":"FAILURE","output_file_path":"","github_url":"","filename":"","rows_testplan":0,"rows_metadata":0,"commit_sha":"","validation":"FAILED"}))
        sys.exit(1)
    fname = generate_workbook()
    ok, msg = post_validate(fname)
    if not ok:
        print(json.dumps({"status":"FAILURE","output_file_path":"","github_url":"","filename":fname,"rows_testplan":len(TESTPLAN_JSON),"rows_metadata":len(META_JSON),"commit_sha":"","validation":f"FAILED: {msg}"}))
        sys.exit(1)
    if not TOKEN:
        print(json.dumps({"status":"IN_PROGRESS","output_file_path":fname,"github_url":"","filename":fname,"rows_testplan":len(TESTPLAN_JSON),"rows_metadata":len(META_JSON),"commit_sha":"","validation":"PASSED"}))
        sys.exit(0)
    gh_url, commit_sha = upload_to_github(fname)
    if gh_url:
        print(json.dumps({"status":"SUCCESS","output_file_path":f"{GITHUB_PATH}","github_url":gh_url,"filename":fname,"rows_testplan":len(TESTPLAN_JSON),"rows_metadata":len(META_JSON),"commit_sha":commit_sha,"validation":"PASSED"}))
    else:
        print(json.dumps({"status":"FAILURE","output_file_path":fname,"github_url":"","filename":fname,"rows_testplan":len(TESTPLAN_JSON),"rows_metadata":len(META_JSON),"commit_sha":"","validation":"PASSED"}))
