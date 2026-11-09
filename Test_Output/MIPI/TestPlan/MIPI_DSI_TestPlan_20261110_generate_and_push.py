#!/usr/bin/env python3
"""
Agent 7 - MIPI_DSI Excel TestPlan Generator & GitHub Publisher
Generates: MIPI_DSI_TestPlan_20261110_020000.xlsx
Two sheets: TestPlan (visible), MetaData (veryHidden)
Code Generation column with Excel dropdown validation.

Usage:
  pip install openpyxl requests
  GITHUB_TOKEN=ghp_xxx python MIPI_DSI_TestPlan_20261110_generate_and_push.py

Without GITHUB_TOKEN, generates the .xlsx locally only.
"""
import json, os, sys, base64

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# ============================================================
# EMBEDDED DATA - TESTPLAN_JSON (from Agent 6B)
# ============================================================
TESTPLAN_JSON = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "Register Read/Write Verification",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This test verifies register-level access for the MIPI DSI subsystem across 52 registers. It performs two main validation phases: (1) Default Value Check - reads all 52 registers after reset and compares each read value against its expected default/reset value, skipping registers that are not readable based on their read mask. Any mismatch is counted as a default value failure. (2) Write/Read-Back Check - writes six distinct test data patterns to all writable registers and reads back each register to verify correctness. Registers that are flagged to be skipped or have a zero write mask are excluded from the write/read-back phase. The expected read-back value accounts for writable and readable bit fields using the read mask, write mask, and default value of each register. Any mismatch is counted as a write/read failure. A soft reset check function is defined but is not invoked in the current test flow. The test reports overall pass if zero failures are detected across both phases, and overall fail otherwise.",
        "Test Steps / Procedure": "Step 1: Initialize failure counters for default value check and write/read check to zero.\nStep 2: Execute the Default Value Check phase - iterate over all 52 MIPI DSI subsystem registers. For each register, check if the register is readable (read mask is non-zero). If not readable, skip to the next register. If readable, read the register and compare the read value against the expected default value. If a mismatch is detected, increment the default value failure counter and log the failure.\nStep 3: Execute the Write/Read-Back Check phase - for each of the six test data patterns (all-ones, alternating 0xAA, alternating 0x55, all-zeros, 0xA5A5A5A5, 0xFFFF0000), perform the following: (a) Write sub-phase: iterate over all 52 registers. Skip registers flagged in the skip list. Skip read-only registers (zero write mask). For all other registers, write the current test pattern. (b) Read-back sub-phase: iterate over all 52 registers again. Skip registers flagged in the skip list or that are read-only. Read each register and compute the expected value considering writable bits (via write mask), readable bits (via read mask), and default values for non-writable bit positions. Compare the read value against the computed expected value. If a mismatch is detected, increment the write/read failure counter and log the failure.\nStep 4: Evaluate overall result - if either the default value failure counter or the write/read failure counter is greater than zero, report test FAIL. Otherwise, report test PASS.",
        "Impacted Registers": "data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
        "Validation / Acceptance Criteria": "1. Default Value Verification: For each of the 52 registers, the value read after reset must exactly match the specified default value for that register. Registers with a zero read mask are excluded from this check.\n2. Write/Read-Back Verification: For each of the six test data patterns written, the value read back from each writable register must match the expected value computed as: expected = (written_data AND read_mask AND write_mask) OR (inverted_write_mask AND read_mask AND default_value). Registers in the skip list or with a zero write mask are excluded.\n3. Overall Pass Criteria: The test passes (finish with success) only if zero default value mismatches and zero write/read-back mismatches are detected across all registers and all test patterns. Any non-zero failure count results in test failure.",
        "Remarks": "The test covers 52 MIPI DSI subsystem registers spanning data FIFO threshold, low power, DBI, interrupt, command FIFO, pixel data FIFO, DPI control, video parameters (0-8), FIFO flush, soft control reset, DMAC configuration, DPHY TX configuration, PHY lane, PLL configuration, and DSI host registers. Six test patterns are used to exercise bit toggling coverage. A soft reset check function exists in the test but is currently not invoked. Certain registers are flagged to be skipped during the write/read-back phase. Read-only registers are automatically excluded from write operations based on their write mask."
    }
]

# ============================================================
# EMBEDDED DATA - META_JSON (from Agent 6A)
# ============================================================
META_JSON = [
    {
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Meta Headers": "File: test_define.c\n#include<mipi_dsi_subsys.h>\n\nFile: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"",
        "Meta Macros": "File: test_define.c\n#define CNT 52\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n\nConditional compilation macro referenced via #ifdef but definition not supplied in source:\nDEBUG_DISPLAY",
        "Meta Arrays": "File: test_define.c (file scope)\n\nconst unsigned long int addr_array[52]={MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,...,MIZAR_MIPI_DSI_SUBSYS_DSI_HOST};\nconst int default_value_array[52]={...};\nconst int read_mask_array[52]={...};\nconst int write_mask_array[52]={...};\nconst int skip_array[52]={0,0,0,0,1,0,0,0,0,0,...};\n\nFile: program.c (local to chk_rd_wr function)\nint chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};",
        "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;\n\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;",
        "Meta Test Description": "This testcase performs register-level verification for the MIPI DSI subsystem across 52 registers. It consists of two main phases: (1) Default/reset value check (chk_rst_val) - iterates over all 52 registers via addr_array, reads each register using read_reg(), and compares against expected default values from default_value_array, skipping non-readable registers where read_mask_array is 0x00000000, incrementing def_fail_cnt on mismatch; (2) Write/read-back check (chk_rd_wr) - writes 6 test patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000) to all writable registers and reads back to verify, skipping registers flagged in skip_array or with zero write_mask_array, computing expected value as exp_val = (data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]) where wr_n = write_mask_array[i] ^ 0xffffffff, incrementing wr_fail_cnt on mismatch. A soft_reset_chk function is defined but commented out in test_case(). The test calls finish(1) on any failure (def_fail_cnt > 0 or wr_fail_cnt > 0) or finish(0) on success.",
        "Meta Test Steps / Procedure": "The full source code content from test_define.c and program.c as provided by Agent 5A.",
        "Meta Impacted Registers": "52 unique registers: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0 through vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
        "Meta Validation / Acceptance Criteria": "chk_rst_val: For each register, read value must match default_value_array[i]. chk_rd_wr: For each of 6 test patterns written, read-back must match exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Final: finish(0) if no failures, finish(1) otherwise."
    }
]

TESTPLAN_COLS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks",
    "Code Generation (Required / Not)"
]

META_COLS = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

HEADER_FONT = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
HEADER_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
WRAP = Alignment(wrap_text=True, vertical="top")
FILENAME = "MIPI_DSI_TestPlan_20261110_020000.xlsx"
GITHUB_PATH = f"Test_Output/MIPI/TestPlan/{FILENAME}"
REPO = "titusbspgit/PSVValidation"

def style_header(ws, columns):
    for ci, cn in enumerate(columns, 1):
        c = ws.cell(row=1, column=ci, value=cn)
        c.font = HEADER_FONT
        c.fill = HEADER_FILL
        c.alignment = WRAP
    ws.freeze_panes = "A2"

def auto_width(ws, max_w=80):
    for col in ws.columns:
        mx = 0
        for cell in col:
            if cell.value:
                for line in str(cell.value).split("\n"):
                    mx = max(mx, len(line))
        ws.column_dimensions[col[0].column_letter].width = min(mx + 4, max_w)

# Cross-dataset validation
tp_names = {tc["Test Case Name"] for tc in TESTPLAN_JSON}
meta_names = {mc["Test Case Name"] for mc in META_JSON}
if tp_names - meta_names:
    print(f"CROSS-DATASET VALIDATION FAILED: {tp_names - meta_names}")
    sys.exit(1)

wb = Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = "TestPlan"
style_header(ws_tp, TESTPLAN_COLS)
for ri, tc in enumerate(TESTPLAN_JSON, 2):
    for ci, cn in enumerate(TESTPLAN_COLS, 1):
        v = "" if cn == "Code Generation (Required / Not)" else tc.get(cn, "")
        ws_tp.cell(row=ri, column=ci, value=v).alignment = WRAP

dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=False)
dv.error = "Please select Required or Not Required"
dv.errorTitle = "Invalid Entry"
for r in range(2, len(TESTPLAN_JSON) + 2):
    dv.add(ws_tp.cell(row=r, column=len(TESTPLAN_COLS)))
ws_tp.add_data_validation(dv)
auto_width(ws_tp)

# MetaData sheet
ws_meta = wb.create_sheet("MetaData")
style_header(ws_meta, META_COLS)
for ri, mc in enumerate(META_JSON, 2):
    for ci, cn in enumerate(META_COLS, 1):
        ws_meta.cell(row=ri, column=ci, value=mc.get(cn, "")).alignment = WRAP
auto_width(ws_meta)
ws_meta.sheet_state = "veryHidden"

wb.save(FILENAME)
fsize = os.path.getsize(FILENAME)
print(f"Saved: {FILENAME} ({fsize} bytes)")

# Post-save validation
ok = True
wb2 = load_workbook(FILENAME)
if "TestPlan" not in wb2.sheetnames: ok = False
if "MetaData" not in wb2.sheetnames: ok = False
if wb2["TestPlan"].max_row - 1 != len(TESTPLAN_JSON): ok = False
if wb2["MetaData"].max_row - 1 != len(META_JSON): ok = False
hdrs = [wb2["TestPlan"].cell(1, c).value for c in range(1, wb2["TestPlan"].max_column+1)]
if "Code Generation (Required / Not)" not in hdrs: ok = False
if not wb2["TestPlan"].data_validations.dataValidation: ok = False
wb2.close()
print(f"Validation: {'PASSED' if ok else 'FAILED'}")

# Upload to GitHub
token = os.environ.get("GITHUB_TOKEN", "")
gurl = ""
csha = ""
if token and ok:
    import requests
    with open(FILENAME, "rb") as f:
        b64 = base64.b64encode(f.read()).decode()
    url = f"https://api.github.com/repos/{REPO}/contents/{GITHUB_PATH}"
    h = {"Authorization": f"token {token}", "Accept": "application/vnd.github.v3+json"}
    r = requests.get(url, headers=h)
    sha = r.json().get("sha") if r.status_code == 200 else None
    p = {"message": "Generated TestPlan Excel Workbook", "content": b64, "branch": "main"}
    if sha: p["sha"] = sha
    r = requests.put(url, headers=h, json=p)
    if r.status_code in (200, 201):
        d = r.json()
        gurl = d["content"]["html_url"]
        csha = d["commit"]["sha"]
        print(f"Uploaded: {gurl}")
        print(f"Commit: {csha}")
    else:
        print(f"Upload failed: {r.status_code}")
elif not token:
    print("GITHUB_TOKEN not set. Run with: GITHUB_TOKEN=ghp_xxx python " + sys.argv[0])

print(json.dumps({
    "status": "SUCCESS" if ok and gurl else ("IN_PROGRESS" if ok else "FAILURE"),
    "output_file_path": GITHUB_PATH,
    "github_url": gurl or f"https://github.com/{REPO}/blob/main/{GITHUB_PATH}",
    "filename": FILENAME,
    "rows_testplan": len(TESTPLAN_JSON),
    "rows_metadata": len(META_JSON),
    "commit_sha": csha,
    "validation": "PASSED" if ok else "FAILED"
}, indent=2))
