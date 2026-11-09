#!/usr/bin/env python3
"""
Agent 7 - MIPI_DSI TestPlan Excel Workbook Generator & GitHub Uploader
Generates MIPI_DSI_TestPlan_20261109_183000.xlsx and pushes it to GitHub.

Usage:
  pip install openpyxl PyGithub
  export GITHUB_TOKEN=<your_pat>
  python generate_MIPI_DSI_TestPlan_20261109_183000.py
"""
import json, os, sys, base64
from datetime import datetime

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# ── Configuration ──
IP_NAME = "MIPI_DSI"
TIMESTAMP = "20261109_183000"
FILENAME = f"{IP_NAME}_TestPlan_{TIMESTAMP}.xlsx"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_XLSX = os.path.join(SCRIPT_DIR, FILENAME)

GITHUB_OWNER = "titusbspgit"
GITHUB_REPO = "PSVValidation"
GITHUB_BRANCH = "main"
GITHUB_DIR = "Test_Output/MIPI/TestPlan"
GITHUB_PATH = f"{GITHUB_DIR}/{FILENAME}"

# ── Inline Data ──
TESTPLAN_JSON = [
  {
    "Index": "1",
    "SS / Module": "MIPI_DSI",
    "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
    "Feature": "Register Write/Read Verification",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This testcase verifies the register write and read functionality of 52 MIPI DSI subsystem registers. The test executes in two phases. Phase 1 performs a default/reset value check by reading all 52 registers after reset and comparing the read values against their expected default values, tracking any mismatches. Phase 2 performs a write/read verification by writing six distinct test data patterns to each writable register, reading back the value, and comparing the read data against the expected value computed using the register write mask, read mask, and default value. Registers marked in a skip list are excluded from both write and read operations. Registers identified as read-only (write mask of all zeros) are also excluded from the write/read phase. Failures in each phase are independently tracked and reported.",
    "Test Steps / Procedure": "1. Initialize failure counters for default value check and write/read check to zero.\n2. Execute Phase 1 - Default Value Verification:\n   a. Iterate through all 52 MIPI DSI subsystem registers.\n   b. For each register, read the current value after reset.\n   c. Apply the read mask to the read value.\n   d. Compare the masked read value against the expected default value for that register.\n   e. If a mismatch is detected, increment the default value failure counter and log the failure.\n3. Execute Phase 2 - Write/Read Verification:\n   a. For each of the six test data patterns (all-ones, 0xAAAAAAAA, 0x55555555, all-zeros, 0xA5A5A5A5, 0xFFFF0000):\n      i. Iterate through all 52 registers.\n      ii. Skip the register if it is flagged in the skip list.\n      iii. Skip the register if its write mask indicates it is read-only.\n      iv. Write the current test data pattern to the register.\n      v. Read back the register value.\n      vi. Compute the expected value using the write mask, read mask, and default value to account for read-only bit fields retaining their default values.\n      vii. Compare the read-back value against the computed expected value.\n      viii. If a mismatch is detected, increment the write/read failure counter and log the failure with the expected and actual values.\n4. Report overall test results based on the failure counters from both phases.",
    "Impacted Registers": "data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
    "Validation / Acceptance Criteria": "1. Phase 1 - Default Value Check: All 52 MIPI DSI subsystem registers must return their expected default values when read after reset. The masked read value for each register must exactly match its specified default value. The default value failure counter must be zero at the end of this phase.\n2. Phase 2 - Write/Read Check: For each of the six test data patterns applied to each writable register (excluding skipped and read-only registers), the read-back value must match the computed expected value. The expected value accounts for writable bits reflecting the written pattern and read-only bits retaining their default values, both filtered through the read mask. The write/read failure counter must be zero at the end of this phase.\n3. Overall: The test passes only if both the default value failure counter and the write/read failure counter remain at zero after all phases complete.",
    "Remarks": "The test covers 52 registers spanning MIPI DSI subsystem functional groups including data FIFO thresholds, low power control, DBI configuration, interrupt management, command and pixel data FIFO thresholds, DPI control, video parameters, FIFO flush, soft reset control, DMAC configuration, DPHY timing and IO configuration, PHY lane control, PLL configuration and observation, and DSI host control. Certain registers are excluded via a skip list. Read-only registers are automatically excluded from the write/read phase based on their write mask being all zeros. Six complementary test patterns are used to exercise both set and clear transitions on all bit positions."
  }
]

META_JSON = [
  {
    "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
    "Meta Headers": "File: program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include \"test_define.c\"\n4. #include \"test_common.h\"\n\nFile: test_define.c\n1. #include<mipi_dsi_subsys.h>",
    "Meta Macros": "File: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA (excerpt truncated; replacement text not visible in supplied source)\nConditional compilation macro: DEBUG_DISPLAY (used in #ifdef guards; definition not visible in supplied excerpts)\n\nFile: test_define.c\n#define CNT 52",
    "Meta Arrays": "File: test_define.c (file scope)\n\n1. const unsigned long int addr_array[52]\n2. const int default_value_array[52]\n3. const int read_mask_array[52]\n4. const int write_mask_array[52]\n5. const int skip_array[52]={0,0,0,0,1,0,0,0,0,0};\n\nFile: program.c (local to chk_rd_wr)\nint chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};",
    "Meta Types": "TYPE_DEFINITIONS\nNo type definitions (typedef, struct, union, enum) visible in supplied source excerpts.\n\nOTHER_DECLARATIONS\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;\n\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;",
    "Meta Test Description": "This testcase for the MIPI_DSI IP performs register write/read verification on 52 MIPI DSI subsystem registers. The test_case() function calls two phases: (1) chk_rst_val(): verifies default/reset values (body not available in supplied excerpts), tracking failures in def_fail_cnt; (2) chk_rd_wr(): writes 6 test patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000) to each writable register, reads back, and compares against expected values, tracking failures in wr_fail_cnt. Registers flagged in skip_array are skipped. Registers with write_mask_array==0x00000000 are treated as read-only and skipped.",
    "Meta Test Steps / Procedure": "SOURCE_FILE: program.c\nSOURCE_CONTENT:\nNote: Content below is from RAG-retrieved verbatim source chunks.\n\n--- Chunk A (file-scope declarations) ---\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"\n\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;\n\n#define SOFT_RST_REG_ADDRESS 0x00000000\n\n--- Chunk B (chk_rd_wr function start) ---\nvoid chk_rd_wr()\n{\n    unsigned long int i,addr,j,exp_val,wr_n;\n    int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};\n    for(j=0;j<6;j++)\n    {\n        data_wr=chk_val[j];\n\n--- Chunk D (test_case function) ---\nint test_case()\n{\n    chk_rst_val();\n    chk_rd_wr();\n}\n\nSOURCE_FILE: test_define.c\n#include<mipi_dsi_subsys.h>\n#define CNT 52",
    "Meta Impacted Registers": "52 address macros matched to UI spec register names: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
    "Meta Validation / Acceptance Criteria": "chk_rst_val(): validates default register values.\nchk_rd_wr(): For each of 6 test patterns, for each of 52 registers: skip if skip_array[i]==1 or write_mask==0x00000000. Write pattern, read back, compare data_rd == exp_val. Pass: print PASS. Fail: increment wr_fail_cnt, print Failed."
  }
]

# ── Column Definitions ──
TP_COLS = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Speed', 'Mode',
    'Memory Start Offset', 'Memory End Offset', 'Test Description',
    'Test Steps / Procedure', 'Impacted Registers',
    'Validation / Acceptance Criteria', 'Remarks',
    'Code Generation (Required / Not)'
]

META_COLS = [
    'Test Case Name', 'Meta Headers', 'Meta Macros', 'Meta Arrays',
    'Meta Types', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria'
]

# ── Styles ──
HDR_FONT = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
HDR_FILL = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
WRAP = Alignment(wrap_text=True, vertical='top')

def auto_width(ws, max_w=60):
    for col in ws.columns:
        mx = 0
        letter = get_column_letter(col[0].column)
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                mx = max(mx, max(len(l) for l in lines))
        ws.column_dimensions[letter].width = min(max(mx + 2, 12), max_w)

def generate_workbook():
    wb = Workbook()

    # ── TestPlan Sheet ──
    ws_tp = wb.active
    ws_tp.title = 'TestPlan'
    ws_tp.freeze_panes = 'A2'
    for ci, h in enumerate(TP_COLS, 1):
        c = ws_tp.cell(row=1, column=ci, value=h)
        c.font = HDR_FONT
        c.fill = HDR_FILL
        c.alignment = WRAP

    for ri, rec in enumerate(TESTPLAN_JSON, 2):
        for ci, h in enumerate(TP_COLS, 1):
            if h == 'Code Generation (Required / Not)':
                ws_tp.cell(row=ri, column=ci).alignment = WRAP
            else:
                val = rec.get(h, '')
                c = ws_tp.cell(row=ri, column=ci, value=val)
                c.alignment = WRAP

    # Data Validation dropdown for Code Generation column
    dv = DataValidation(
        type='list', formula1='"Required,Not Required"',
        allow_blank=True, showDropDown=False
    )
    dv.error = 'Please select Required or Not Required'
    dv.errorTitle = 'Invalid Entry'
    dv.prompt = 'Select Required or Not Required'
    dv.promptTitle = 'Code Generation'
    code_col = len(TP_COLS)
    code_letter = get_column_letter(code_col)
    last_row = len(TESTPLAN_JSON) + 1
    dv.sqref = f'{code_letter}2:{code_letter}{last_row}'
    ws_tp.add_data_validation(dv)
    auto_width(ws_tp)

    # ── MetaData Sheet (veryHidden) ──
    ws_meta = wb.create_sheet('MetaData')
    ws_meta.sheet_state = 'veryHidden'
    ws_meta.freeze_panes = 'A2'
    for ci, h in enumerate(META_COLS, 1):
        c = ws_meta.cell(row=1, column=ci, value=h)
        c.font = HDR_FONT
        c.fill = HDR_FILL
        c.alignment = WRAP

    for ri, rec in enumerate(META_JSON, 2):
        for ci, h in enumerate(META_COLS, 1):
            val = rec.get(h, '')
            c = ws_meta.cell(row=ri, column=ci, value=val)
            c.alignment = WRAP
    auto_width(ws_meta)

    # ── Save ──
    wb.save(OUT_XLSX)
    return OUT_XLSX

def validate_workbook():
    assert os.path.exists(OUT_XLSX), f"File not found: {OUT_XLSX}"
    sz = os.path.getsize(OUT_XLSX)
    assert sz > 0, "File size is 0"
    vwb = load_workbook(OUT_XLSX)
    assert 'TestPlan' in vwb.sheetnames, "TestPlan sheet missing"
    assert 'MetaData' in vwb.sheetnames, "MetaData sheet missing"
    tp_rows = vwb['TestPlan'].max_row - 1
    meta_rows = vwb['MetaData'].max_row - 1
    assert tp_rows == len(TESTPLAN_JSON), f"TestPlan rows: {tp_rows} != {len(TESTPLAN_JSON)}"
    assert meta_rows == len(META_JSON), f"MetaData rows: {meta_rows} != {len(META_JSON)}"
    print(f"Validation PASSED | File: {FILENAME} | Size: {sz} bytes | TP rows: {tp_rows} | Meta rows: {meta_rows}")
    return True

def push_to_github():
    try:
        from github import Github
    except ImportError:
        print("PyGithub not installed. Run: pip install PyGithub")
        return None, None
    token = os.environ.get("GITHUB_TOKEN")
    if not token:
        print("GITHUB_TOKEN not set")
        return None, None
    g = Github(token)
    repo = g.get_repo(f"{GITHUB_OWNER}/{GITHUB_REPO}")
    with open(OUT_XLSX, 'rb') as f:
        content = f.read()
    b64 = base64.b64encode(content).decode('utf-8')
    try:
        existing = repo.get_contents(GITHUB_PATH, ref=GITHUB_BRANCH)
        result = repo.update_file(GITHUB_PATH, "Generated TestPlan Excel Workbook", content, existing.sha, branch=GITHUB_BRANCH)
    except Exception:
        result = repo.create_file(GITHUB_PATH, "Generated TestPlan Excel Workbook", content, branch=GITHUB_BRANCH)
    sha = result['commit'].sha
    url = f"https://github.com/{GITHUB_OWNER}/{GITHUB_REPO}/blob/{GITHUB_BRANCH}/{GITHUB_PATH}"
    print(f"Pushed to GitHub: {url}")
    print(f"Commit SHA: {sha}")
    return url, sha

if __name__ == '__main__':
    print("Generating workbook...")
    generate_workbook()
    print("Validating...")
    validate_workbook()
    print("Pushing to GitHub...")
    url, sha = push_to_github()
    print("Done.")
