#!/usr/bin/env python3
"""
Agent 7: Excel Generation and GitHub Publishing
Auto-generated script to create MIPI_CSI_TestPlan_20250605_153000.xlsx
Run: pip install openpyxl && python generate_MIPI_CSI_TestPlan_20250605_153000.py
"""
import json, os, sys, base64, datetime
from pathlib import Path

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation

# ============================================================
# TESTPLAN_JSON (Agent 6B Output) - 1 record
# ============================================================
TESTPLAN_JSON = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Feature": "Register Read/Write Verification",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This testcase performs register-level read/write verification for the MIPI CSI2 RB register block (csi_reg block). The test validates two aspects: (1) Default/Reset Value Check \u2014 each readable register in the csi_reg block is read after reset and compared against its expected hardware default value; non-readable registers are skipped. (2) Write-Read-Back Check \u2014 a known data pattern is written to each writable register (skipping designated registers such as interrupt_raw and flush, as well as non-writable registers), then each register is read back and compared against an expected value that accounts for the register\u2019s read mask, write mask, and default value for non-writable bit positions. The test reports PASS if all registers match their expected values in both phases, and FAIL if any mismatch is detected in either phase.",
    "Test Steps / Procedure": "Step 1: Execute the default/reset value check phase. For each register in the csi_reg block, verify that the register is readable. If the register is not readable, skip it. If readable, read the register value and compare it against the expected default/reset value. Record any mismatch as a default-value failure.\n\nStep 2: Execute the write-read-back verification phase. For each register in the csi_reg block, check whether the register is flagged to be skipped (interrupt_raw and flush are skipped in this phase). Also skip registers that are not writable. For all remaining writable registers, write a known data pattern to the register.\n\nStep 3: After writing to all eligible registers, read back each written register. Skip registers that were not written, are not writable, or are not readable. For each read-back, compute the expected value by combining the written data (masked by the read and write masks) with the default value (for bit positions that are not writable but are readable). Compare the read-back value against this expected value. Record any mismatch as a write-read failure.\n\nStep 4: Evaluate overall test result. If any default-value failures or write-read failures were recorded in the previous steps, report the test as FAIL. If no failures were recorded in either phase, report the test as PASS.",
    "Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush",
    "Validation / Acceptance Criteria": "1. Default Value Verification: For every readable register in the csi_reg block, the value read after reset must exactly match the expected default/reset value defined for that register. Any mismatch constitutes a failure.\n\n2. Write-Read-Back Verification: For every writable and readable register (excluding skipped registers interrupt_raw and flush), the value read back after writing a known pattern must match the expected value. The expected value is computed by applying the write mask and read mask to the written data, combined with the default value for non-writable bit positions. Any mismatch constitutes a failure.\n\n3. Overall Pass/Fail: The test passes (finish with success) only if zero default-value mismatches and zero write-read mismatches are recorded across all registers. Any non-zero failure count in either category results in test failure.",
    "Remarks": "The test covers 10 registers in the csi_reg block. Two registers (interrupt_raw and flush) are explicitly skipped during the write-read-back phase via a skip configuration. A soft reset check function exists in the test but is currently disabled (commented out) and is not executed during the test flow. Non-readable registers are automatically skipped during the default value check phase, and non-writable registers are automatically skipped during the write phase."
  }
]

# ============================================================
# META_JSON (Agent 6A Output) - 1 record
# ============================================================
META_JSON = [
  {
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Meta Headers": "File: test_define.c\n#include<mipi_csi2_rb_reg.h>\n\nFile: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"",
    "Meta Macros": "File: test_define.c\n#define CNT 60\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA \u2014 value not available (RAG snippet truncated after \"#define\u2026\")\n\nConditional compilation macro (no definition supplied):\nDEBUG_DISPLAY \u2014 used in #ifdef DEBUG_DISPLAY guards throughout program.c; no #define visible in supplied source.",
    "Meta Arrays": "File: test_define.c\n\nconst unsigned long int addr_array[60]={MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,MIZAR_MIPI_CSI2_RB_REG_LINE_INFO,MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL,MIZAR_MIPI_CSI2_RB_REG_LANE_CLK,MIZAR_MIPI_CSI2_RB_REG_MEM,MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW,MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK,MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE,MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA,MIZAR_MIPI_CSI2_RB_REG_FLUSH};\n\nconst int default_value_array[60]={MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_DEFAULT_VAL,MIPI_CSI2_RB_REG_LINE_INFO_DEFAULT_VAL,MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_DEFAULT_VAL,MIPI_CSI2_RB_REG_LANE_CLK_DEFAULT_VAL,MIPI_CSI2_RB_REG_MEM_DEFAULT_VAL,MIPI_CSI2_RB_REG_INTERRUPT_RAW_DEFAULT_VAL,MIPI_CSI2_RB_REG_INTERRUPT_MASK_DEFAULT_VAL,MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_DEFAULT_VAL,MIPI_CSI2_RB_REG_CONTROL_DATA_DEFAULT_VAL,MIPI_CSI2_RB_REG_FLUSH_DEFAULT_VAL};\n\nconst int read_mask_array[60]={MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_READ_MASK,MIPI_CSI2_RB_REG_LINE_INFO_READ_MASK,MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_READ_MASK,MIPI_CSI2_RB_REG_LANE_CLK_READ_MASK,MIPI_CSI2_RB_REG_MEM_READ_MASK,MIPI_CSI2_RB_REG_INTERRUPT_RAW_READ_MASK,MIPI_CSI2_RB_REG_INTERRUPT_MASK_READ_MASK,MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_READ_MASK,MIPI_CSI2_RB_REG_CONTROL_DATA_READ_MASK,MIPI_CSI2_RB_REG_FLUSH_READ_MASK};\n\nconst int write_mask_array[60]={MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_WRITE_MASK,MIPI_CSI2_RB_REG_LINE_INFO_WRITE_MASK,MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_WRITE_MASK,MIPI_CSI2_RB_REG_LANE_CLK_WRITE_MASK,MIPI_CSI2_RB_REG_MEM_WRITE_MASK,MIPI_CSI2_RB_REG_INTERRUPT_RAW_WRITE_MASK,MIPI_CSI2_RB_REG_INTERRUPT_MASK_WRITE_MASK,MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_WRITE_MASK,MIPI_CSI2_RB_REG_CONTROL_DATA_WRITE_MASK,MIPI_CSI2_RB_REG_FLUSH_WRITE_MASK};\n\nconst int skip_array[60]={0,0,0,0,0,1,0,0,0,1};",
    "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c\nint data_rd,data_wr,rst_val;\n\nFile: program.c\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;",
    "Meta Test Description": "This testcase (mipi_csi2_rb_reg_wr_rd_test) performs register read/write verification for the MIPI CSI2 RB register block (csi_reg block). The test consists of two main phases: (1) chk_rst_val \u2014 reads each register and compares the read value against the expected default/reset value from default_value_array, skipping non-readable registers (read_mask_array == 0x00000000), incrementing def_fail_cnt on mismatch; (2) chk_rd_wr \u2014 writes data_wr to each writable register (skipping entries in skip_array and non-writable registers), then reads back and compares against an expected value computed as exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])), where wr_n is the inverted write mask, incrementing wr_fail_cnt on mismatch. Registers flagged in skip_array (INTERRUPT_RAW at index 5, FLUSH at index 9) are skipped during chk_rd_wr. A soft_reset_chk function is defined but its call is commented out in test_case. The test concludes with finish(1) if any failures occurred (def_fail_cnt > 0 || wr_fail_cnt > 0), or finish(0) otherwise. 10 registers are addressed from the csi_reg block: virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush.",
    "Meta Test Steps / Procedure": "SOURCE_FILE: test_define.c\nSOURCE_CONTENT:\n#include<mipi_csi2_rb_reg.h>\n#define CNT 60\nint data_rd,data_wr,rst_val;\nconst unsigned long int addr_array[60]={...};\nconst int default_value_array[60]={...};\nconst int read_mask_array[60]={...};\nconst int write_mask_array[60]={...};\nconst int skip_array[60]={0,0,0,0,0,1,0,0,0,1};\n\nSOURCE_FILE: program.c\nFunctions: soft_reset_chk(), chk_rst_val(), chk_rd_wr(), test_case()",
    "Meta Impacted Registers": "Register block: csi_reg\nBase: 0xE6A00000 + 0x4000\nvirtual_channel (0x0), line_info (0x4), fifo_threshold_val (0x8), lane_clk (0xC), mem (0x10), interrupt_raw (0x14), interrupt_mask (0x18), interrupt_enable (0x1C), control_data (0x20), flush (0x24)",
    "Meta Validation / Acceptance Criteria": "chk_rst_val: if(data_rd == default_value_array[i]) PASS else def_fail_cnt++. chk_rd_wr: if(data_rd == exp_val) PASS else wr_fail_cnt++. test_case: if(def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0)."
  }
]

# ============================================================
# Configuration
# ============================================================
IP_NAME = "MIPI_CSI"
FILENAME = "MIPI_CSI_TestPlan_20250605_153000.xlsx"
OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_PATH = os.path.join(OUTPUT_DIR, FILENAME)

# ============================================================
# TestPlan Sheet Columns
# ============================================================
TESTPLAN_COLUMNS = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Test Description",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Remarks",
    "Code Generation (Required / Not)"
]

# ============================================================
# MetaData Sheet Columns
# ============================================================
METADATA_COLUMNS = [
    "Test Case Name",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
    "Meta Types",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria"
]

# ============================================================
# Styles
# ============================================================
HEADER_FONT = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
HEADER_FILL = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
HEADER_ALIGNMENT = Alignment(horizontal='center', vertical='center', wrap_text=True)
DATA_ALIGNMENT = Alignment(vertical='top', wrap_text=True)
THIN_BORDER = Border(
    left=Side(style='thin'),
    right=Side(style='thin'),
    top=Side(style='thin'),
    bottom=Side(style='thin')
)

def create_workbook():
    """Create the Excel workbook with TestPlan and MetaData sheets."""
    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    # Write header row
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGNMENT
        cell.border = THIN_BORDER

    # Write data rows
    for row_idx, record in enumerate(TESTPLAN_JSON, 2):
        for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
            if col_name == "Code Generation (Required / Not)":
                cell = ws_tp.cell(row=row_idx, column=col_idx, value="")
            else:
                value = record.get(col_name, "")
                cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = DATA_ALIGNMENT
            cell.border = THIN_BORDER

    # Add Data Validation dropdown for Code Generation column
    code_gen_col_idx = len(TESTPLAN_COLUMNS)
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

    last_data_row = len(TESTPLAN_JSON) + 1
    from openpyxl.utils import get_column_letter
    col_letter = get_column_letter(code_gen_col_idx)
    dv.sqref = f"{col_letter}2:{col_letter}{last_data_row}"
    ws_tp.add_data_validation(dv)

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # Auto-size columns with max width
    MAX_WIDTH = 60
    for col_idx in range(1, len(TESTPLAN_COLUMNS) + 1):
        max_len = 0
        col_letter = get_column_letter(col_idx)
        for row in ws_tp.iter_rows(min_col=col_idx, max_col=col_idx, min_row=1, max_row=last_data_row):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
        adjusted_width = min(max_len + 4, MAX_WIDTH)
        ws_tp.column_dimensions[col_letter].width = max(adjusted_width, 12)

    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet(title="MetaData")

    # Write header row
    for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGNMENT
        cell.border = THIN_BORDER

    # Write data rows
    for row_idx, record in enumerate(META_JSON, 2):
        for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
            value = record.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = DATA_ALIGNMENT
            cell.border = THIN_BORDER

    # Freeze first row
    ws_md.freeze_panes = "A2"

    # Auto-size columns with max width
    md_last_row = len(META_JSON) + 1
    for col_idx in range(1, len(METADATA_COLUMNS) + 1):
        max_len = 0
        col_letter = get_column_letter(col_idx)
        for row in ws_md.iter_rows(min_col=col_idx, max_col=col_idx, min_row=1, max_row=md_last_row):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
        adjusted_width = min(max_len + 4, MAX_WIDTH)
        ws_md.column_dimensions[col_letter].width = max(adjusted_width, 12)

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = 'veryHidden'

    # Save workbook
    wb.save(OUTPUT_PATH)
    print(f"Workbook saved: {OUTPUT_PATH}")

    # ---- Post-Save Validation ----
    validate_workbook(OUTPUT_PATH)

    return OUTPUT_PATH


def validate_workbook(filepath):
    """Validate the generated workbook."""
    # 1. File exists
    assert os.path.exists(filepath), "File does not exist"
    # 2. File size > 0
    assert os.path.getsize(filepath) > 0, "File size is 0"
    # 3. Workbook reopens
    wb = load_workbook(filepath)
    # 4. TestPlan sheet exists
    assert "TestPlan" in wb.sheetnames, "TestPlan sheet missing"
    # 5. MetaData sheet exists
    assert "MetaData" in wb.sheetnames, "MetaData sheet missing"
    ws_tp = wb["TestPlan"]
    ws_md = wb["MetaData"]
    # 6. TestPlan row count
    tp_data_rows = ws_tp.max_row - 1
    assert tp_data_rows == len(TESTPLAN_JSON), f"TestPlan rows mismatch: {tp_data_rows} vs {len(TESTPLAN_JSON)}"
    # 7. MetaData row count
    md_data_rows = ws_md.max_row - 1
    assert md_data_rows == len(META_JSON), f"MetaData rows mismatch: {md_data_rows} vs {len(META_JSON)}"
    # 8. Code Generation column exists
    headers = [ws_tp.cell(row=1, column=c).value for c in range(1, ws_tp.max_column + 1)]
    assert "Code Generation (Required / Not)" in headers, "Code Generation column missing"
    # 9. Data validation exists
    assert len(ws_tp.data_validations.dataValidation) > 0, "No data validations found"
    # 10. Code Generation cells are blank
    cg_col = headers.index("Code Generation (Required / Not)") + 1
    for r in range(2, ws_tp.max_row + 1):
        val = ws_tp.cell(row=r, column=cg_col).value
        assert val is None or val == "", f"Code Generation cell row {r} is not blank: {val}"

    print("POST-SAVE VALIDATION: ALL CHECKS PASSED")
    wb.close()


def push_to_github(filepath):
    """Push the generated workbook to GitHub."""
    try:
        import requests
    except ImportError:
        os.system(f"{sys.executable} -m pip install requests")
        import requests

    GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")
    if not GITHUB_TOKEN:
        print("WARNING: GITHUB_TOKEN not set. Skipping GitHub push.")
        print("Set GITHUB_TOKEN environment variable and re-run to push.")
        return None, None

    owner = "titusbspgit"
    repo = "PSVValidation"
    branch = "main"
    output_directory = "Test_Output/MIPI/TestPlan"
    filename = os.path.basename(filepath)
    github_path = f"{output_directory}/{filename}"

    with open(filepath, "rb") as f:
        content_bytes = f.read()
    content_b64 = base64.b64encode(content_bytes).decode("utf-8")

    url = f"https://api.github.com/repos/{owner}/{repo}/contents/{github_path}"
    headers_req = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }

    # Check if file exists
    resp = requests.get(url, headers=headers_req, params={"ref": branch})
    payload = {
        "message": "Generated TestPlan Excel Workbook",
        "content": content_b64,
        "branch": branch
    }
    if resp.status_code == 200:
        payload["sha"] = resp.json()["sha"]

    resp = requests.put(url, headers=headers_req, json=payload)
    if resp.status_code in (200, 201):
        data = resp.json()
        github_url = data["content"]["html_url"]
        commit_sha = data["commit"]["sha"]
        print(f"GitHub URL: {github_url}")
        print(f"Commit SHA: {commit_sha}")
        return github_url, commit_sha
    else:
        print(f"GitHub push failed: {resp.status_code} {resp.text}")
        return None, None


if __name__ == "__main__":
    print("=" * 60)
    print("Agent 7: Excel Generation and GitHub Publishing")
    print("=" * 60)

    filepath = create_workbook()
    github_url, commit_sha = push_to_github(filepath)

    status = "SUCCESS" if github_url else "IN_PROGRESS"
    result = {
        "status": status,
        "output_file_path": f"Test_Output/MIPI/TestPlan/{FILENAME}",
        "github_url": github_url or "",
        "filename": FILENAME,
        "rows_testplan": len(TESTPLAN_JSON),
        "rows_metadata": len(META_JSON),
        "commit_sha": commit_sha or "",
        "validation": "PASSED"
    }
    print("\n" + json.dumps(result, indent=2))
