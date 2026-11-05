#!/usr/bin/env python3
"""
Agent 7 - MIPI_CSI TestPlan Excel Workbook Generator
Run: pip install openpyxl requests && python3 generate_mipi_csi_testplan_20261105_210000.py
Generates MIPI_CSI_TestPlan_20261105_210000.xlsx and pushes to GitHub.
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation
import base64, os, json, sys

try:
    import requests
except ImportError:
    requests = None

GITHUB_TOKEN = os.environ.get('GITHUB_TOKEN', '')
OWNER = 'titusbspgit'
REPO = 'PSVValidation'
BRANCH = 'main'
OUTPUT_DIR = 'Test_Output/MIPI/TestPlan'
FILENAME = 'MIPI_CSI_TestPlan_20261105_210000.xlsx'
OUTPUT_PATH = f'{OUTPUT_DIR}/{FILENAME}'

# ============================================================
# TESTPLAN_JSON (from Agent 6B)
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
        "Test Description": "This testcase verifies the register read/write functionality of 10 MIPI CSI2 RB registers: VIRTUAL_CHANNEL, LINE_INFO, FIFO_THRESHOLD_VAL, LANE_CLK, MEM, INTERRUPT_RAW, INTERRUPT_MASK, INTERRUPT_ENABLE, CONTROL_DATA, and FLUSH. The test executes in two phases. Phase 1 (Default Value Check) reads all registers after reset and verifies that each register holds its expected default/reset value. Phase 2 (Write and Read Check) writes a test data pattern to each writable register, then reads back and compares the read data against an expected value that accounts for the register's read mask, write mask, and default value. Registers INTERRUPT_RAW and FLUSH are explicitly skipped during the write/read phase. Read-only registers are also skipped. The test reports overall pass or fail based on accumulated failure counters from both phases.",
        "Test Steps / Procedure": "Step 1: Initialize the test environment and set failure counters to zero.\nStep 2: Execute the Default Value Check phase - read each of the 10 MIPI CSI2 RB registers and compare the read value against the expected default/reset value for that register. Increment the default-fail counter for any mismatch.\nStep 3: Execute the Write and Read Check phase:\n  Step 3a: Iterate over all registers in the register list.\n  Step 3b: For each register, check if it is flagged in the skip list (INTERRUPT_RAW and FLUSH are skipped). If flagged, skip to the next register.\n  Step 3c: Check if the register is writable. If not writable, skip writing to that register.\n  Step 3d: For writable and non-skipped registers, write the test data pattern to the register.\n  Step 3e: After completing all writes, iterate over all registers again for the read-back phase.\n  Step 3f: For each register, skip if flagged in the skip list, if not writable, or if not readable.\n  Step 3g: For eligible registers, read the register value and compute the expected value using the write mask, read mask, and default value. The expected value accounts for writable bits reflecting the written data and non-writable bits retaining their default values.\n  Step 3h: Compare the read value against the computed expected value. Increment the write-fail counter for any mismatch.\nStep 4: Evaluate overall test result - if either the default-fail counter or the write-fail counter is greater than zero, report test FAIL. Otherwise, report test PASS.",
        "Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush",
        "Validation / Acceptance Criteria": "1. Default Value Check: Each register must return its expected default/reset value when read after reset. The default-fail counter must remain zero.\n2. Write/Read Check: For each writable and non-skipped register, the read-back value after writing the test pattern must match the computed expected value. The expected value is determined by combining the written data (masked by both the read mask and write mask) with the default value (masked by the read mask and the inverse of the write mask). The write-fail counter must remain zero.\n3. Skip Verification: Registers INTERRUPT_RAW and FLUSH must be correctly skipped during the write/read phase. Read-only registers must be skipped for writing.\n4. Overall Pass Criteria: Both the default-fail counter and the write-fail counter must be zero for the test to pass. Any non-zero counter results in test failure.",
        "Remarks": "The soft reset check function exists in the test but is currently disabled. Only 10 of the 60 array slots are populated with valid register entries. Registers INTERRUPT_RAW and FLUSH are explicitly excluded from write/read verification via the skip configuration."
    }
]

# ============================================================
# META_JSON (from Agent 6A)
# ============================================================
META_JSON = [
    {
        "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
        "Meta Headers": "File: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"\n\nFile: test_define.c\n#include<mipi_csi2_rb_reg.h>",
        "Meta Macros": "File: test_define.c\n#define CNT 60\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS\t0x00000000\n#define SOFT_RST_REG_DATA\t0x00000000\n\nConditional compilation directive used in program.c:\n#ifdef DEBUG_DISPLAY\n(DEBUG_DISPLAY is not defined in the supplied source; used as a conditional guard around printf statements.)",
        "Meta Arrays": "File: test_define.c\n\nconst unsigned long int addr_array[60]={\n    MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,\n    MIZAR_MIPI_CSI2_RB_REG_LINE_INFO,\n    MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL,\n    MIZAR_MIPI_CSI2_RB_REG_LANE_CLK,\n    MIZAR_MIPI_CSI2_RB_REG_MEM,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE,\n    MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA,\n    MIZAR_MIPI_CSI2_RB_REG_FLUSH\n};\n\nconst int default_value_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_LINE_INFO_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_LANE_CLK_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_MEM_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_FLUSH_DEFAULT_VAL\n};\n\nconst int read_mask_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_READ_MASK,\n    MIPI_CSI2_RB_REG_LINE_INFO_READ_MASK,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_READ_MASK,\n    MIPI_CSI2_RB_REG_LANE_CLK_READ_MASK,\n    MIPI_CSI2_RB_REG_MEM_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_READ_MASK,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_READ_MASK,\n    MIPI_CSI2_RB_REG_FLUSH_READ_MASK\n};\n\nconst int write_mask_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_WRITE_MASK,\n    MIPI_CSI2_RB_REG_LINE_INFO_WRITE_MASK,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_WRITE_MASK,\n    MIPI_CSI2_RB_REG_LANE_CLK_WRITE_MASK,\n    MIPI_CSI2_RB_REG_MEM_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_WRITE_MASK,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_WRITE_MASK,\n    MIPI_CSI2_RB_REG_FLUSH_WRITE_MASK\n};\n\nconst int skip_array[60]={0,0,0,0,0,1,0,0,0,1};",
        "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;\n\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;",
        "Meta Test Description": "This testcase (mipi_csi2_rb_reg_wr_rd_test) for the MIPI_CSI IP performs register read/write verification on 10 MIPI CSI2 RB registers: VIRTUAL_CHANNEL, LINE_INFO, FIFO_THRESHOLD_VAL, LANE_CLK, MEM, INTERRUPT_RAW, INTERRUPT_MASK, INTERRUPT_ENABLE, CONTROL_DATA, and FLUSH. The test has two main phases: (1) chk_rst_val() checks that all registers hold their expected default/reset values after reset, and (2) chk_rd_wr() writes a test pattern (data_wr) to each writable register (skipping registers flagged in skip_array or with zero write_mask), then reads back and compares against an expected value computed using read_mask, write_mask, and default_value arrays. Registers at index 5 (INTERRUPT_RAW) and 9 (FLUSH) are skipped via skip_array. A soft_reset_chk() function exists but is commented out. Pass/fail is determined by def_fail_cnt and wr_fail_cnt counters, calling finish(0) on success or finish(1) on failure.",
        "Meta Test Steps / Procedure": "SOURCE_FILE: test_define.c\nSOURCE_CONTENT:\n#include<mipi_csi2_rb_reg.h>\n#define CNT 60\nint data_rd,data_wr,rst_val;\nconst unsigned long int addr_array[60]={...};\nconst int default_value_array[60]={...};\nconst int read_mask_array[60]={...};\nconst int write_mask_array[60]={...};\nconst int skip_array[60]={0,0,0,0,0,1,0,0,0,1};\n\nSOURCE_FILE: program.c\nSOURCE_CONTENT:\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\nchk_rst_val(), chk_rd_wr(), test_case() functions as described.",
        "Meta Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush",
        "Meta Validation / Acceptance Criteria": "1. Default Value Check: Each register must return its expected default/reset value when read after reset.\n2. Write/Read Check: For each writable and non-skipped register, the read-back value must match the computed expected value.\n3. Overall: Both def_fail_cnt and wr_fail_cnt must be zero for pass. finish(0) on success, finish(1) on failure."
    }
]


def build():
    wb = openpyxl.Workbook()
    hf = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    hfill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wa = Alignment(wrap_text=True, vertical='top')

    # === TestPlan Sheet ===
    ws = wb.active
    ws.title = 'TestPlan'
    tp_headers = [
        'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Speed', 'Mode',
        'Memory Start Offset', 'Memory End Offset', 'Test Description',
        'Test Steps / Procedure', 'Impacted Registers',
        'Validation / Acceptance Criteria', 'Remarks',
        'Code Generation (Required / Not)'
    ]
    for i, h in enumerate(tp_headers, 1):
        c = ws.cell(row=1, column=i, value=h)
        c.font = hf
        c.fill = hfill
        c.alignment = wa

    for row_idx, tp in enumerate(TESTPLAN_JSON, 2):
        ws.cell(row=row_idx, column=1, value=tp['Index']).alignment = wa
        ws.cell(row=row_idx, column=2, value=tp['SS / Module']).alignment = wa
        ws.cell(row=row_idx, column=3, value=tp['Feature']).alignment = wa
        ws.cell(row=row_idx, column=4, value=tp['Test Case Name']).alignment = wa
        ws.cell(row=row_idx, column=5, value=tp['Speed']).alignment = wa
        ws.cell(row=row_idx, column=6, value=tp['Mode']).alignment = wa
        ws.cell(row=row_idx, column=7, value=tp['Memory Start Offset']).alignment = wa
        ws.cell(row=row_idx, column=8, value=tp['Memory End Offset']).alignment = wa
        ws.cell(row=row_idx, column=9, value=tp['Test Description']).alignment = wa
        ws.cell(row=row_idx, column=10, value=tp['Test Steps / Procedure']).alignment = wa
        ws.cell(row=row_idx, column=11, value=tp['Impacted Registers']).alignment = wa
        ws.cell(row=row_idx, column=12, value=tp['Validation / Acceptance Criteria']).alignment = wa
        ws.cell(row=row_idx, column=13, value=tp['Remarks']).alignment = wa
        # Column 14 (Code Generation) left blank

    # Data Validation dropdown for Code Generation column
    dv = DataValidation(
        type='list',
        formula1='"Required,Not Required"',
        allow_blank=True,
        showDropDown=False,
        showErrorMessage=True
    )
    dv.error = 'Please select Required or Not Required'
    dv.errorTitle = 'Invalid Entry'
    dv.add('N2:N1048576')
    ws.add_data_validation(dv)

    # Column widths
    col_widths = [8, 15, 30, 35, 8, 8, 20, 20, 60, 60, 45, 60, 45, 30]
    for ci in range(1, len(tp_headers) + 1):
        ws.column_dimensions[get_column_letter(ci)].width = col_widths[ci - 1] if ci <= len(col_widths) else 20
    ws.freeze_panes = 'A2'

    # === MetaData Sheet (veryHidden) ===
    wm = wb.create_sheet('MetaData')
    meta_headers = [
        'Test Case Name', 'Meta Headers', 'Meta Macros', 'Meta Arrays',
        'Meta Types', 'Meta Test Description', 'Meta Test Steps / Procedure',
        'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria'
    ]
    for i, h in enumerate(meta_headers, 1):
        c = wm.cell(row=1, column=i, value=h)
        c.font = hf
        c.fill = hfill
        c.alignment = wa

    for row_idx, md in enumerate(META_JSON, 2):
        wm.cell(row=row_idx, column=1, value=md['Test Case Name']).alignment = wa
        wm.cell(row=row_idx, column=2, value=md['Meta Headers']).alignment = wa
        wm.cell(row=row_idx, column=3, value=md['Meta Macros']).alignment = wa
        wm.cell(row=row_idx, column=4, value=md['Meta Arrays']).alignment = wa
        wm.cell(row=row_idx, column=5, value=md['Meta Types']).alignment = wa
        wm.cell(row=row_idx, column=6, value=md['Meta Test Description']).alignment = wa
        wm.cell(row=row_idx, column=7, value=md['Meta Test Steps / Procedure']).alignment = wa
        wm.cell(row=row_idx, column=8, value=md['Meta Impacted Registers']).alignment = wa
        wm.cell(row=row_idx, column=9, value=md['Meta Validation / Acceptance Criteria']).alignment = wa

    # MetaData column widths
    meta_widths = [35, 50, 50, 60, 50, 60, 60, 50, 60]
    for ci in range(1, len(meta_headers) + 1):
        wm.column_dimensions[get_column_letter(ci)].width = meta_widths[ci - 1] if ci <= len(meta_widths) else 30
    wm.freeze_panes = 'A2'
    wm.sheet_state = 'veryHidden'

    # Save workbook
    wb.save(FILENAME)
    sz = os.path.getsize(FILENAME)
    print(f'Saved {FILENAME} ({sz} bytes)')

    # === Post-Save Validation ===
    wb2 = openpyxl.load_workbook(FILENAME)
    assert 'TestPlan' in wb2.sheetnames, 'TestPlan sheet missing'
    assert 'MetaData' in wb2.sheetnames, 'MetaData sheet missing'
    assert wb2['MetaData'].sheet_state == 'veryHidden', 'MetaData not veryHidden'
    tp_rows = wb2['TestPlan'].max_row - 1
    md_rows = wb2['MetaData'].max_row - 1
    assert tp_rows == len(TESTPLAN_JSON), f'TestPlan rows mismatch: {tp_rows} vs {len(TESTPLAN_JSON)}'
    assert md_rows == len(META_JSON), f'MetaData rows mismatch: {md_rows} vs {len(META_JSON)}'
    assert len(wb2['TestPlan'].data_validations.dataValidation) > 0, 'No data validation found'
    # Verify Code Generation column header
    assert wb2['TestPlan'].cell(row=1, column=14).value == 'Code Generation (Required / Not)', 'Code Gen header missing'
    # Verify Code Generation cells are blank
    for r in range(2, tp_rows + 2):
        assert wb2['TestPlan'].cell(row=r, column=14).value is None, f'Code Gen cell row {r} not blank'
    print('Post-Save Validation: PASSED')
    print(f'TestPlan rows: {tp_rows}')
    print(f'MetaData rows: {md_rows}')

    # === Push to GitHub ===
    if GITHUB_TOKEN and requests:
        with open(FILENAME, 'rb') as f:
            content_b64 = base64.b64encode(f.read()).decode()
        url = f'https://api.github.com/repos/{OWNER}/{REPO}/contents/{OUTPUT_PATH}'
        headers = {'Authorization': f'token {GITHUB_TOKEN}', 'Accept': 'application/vnd.github.v3+json'}
        r = requests.get(url, headers=headers, params={'ref': BRANCH})
        sha = r.json().get('sha') if r.status_code == 200 else None
        payload = {'message': 'Generated TestPlan Excel Workbook', 'content': content_b64, 'branch': BRANCH}
        if sha:
            payload['sha'] = sha
        r = requests.put(url, headers=headers, json=payload)
        if r.status_code in (200, 201):
            data = r.json()
            github_url = data['content']['html_url']
            commit_sha = data['commit']['sha']
            print(f'GitHub URL: {github_url}')
            print(f'Commit SHA: {commit_sha}')
            result = {
                'status': 'SUCCESS',
                'output_file_path': OUTPUT_PATH,
                'github_url': github_url,
                'filename': FILENAME,
                'rows_testplan': tp_rows,
                'rows_metadata': md_rows,
                'commit_sha': commit_sha,
                'validation': 'PASSED'
            }
            print(json.dumps(result, indent=2))
        else:
            print(f'GitHub push failed: {r.status_code} {r.text}')
            result = {
                'status': 'FAILURE',
                'output_file_path': OUTPUT_PATH,
                'github_url': '',
                'filename': FILENAME,
                'rows_testplan': tp_rows,
                'rows_metadata': md_rows,
                'commit_sha': '',
                'validation': 'PASSED'
            }
            print(json.dumps(result, indent=2))
    else:
        print('GITHUB_TOKEN not set or requests not available.')
        print('To push: set GITHUB_TOKEN env var and install requests.')
        print(f'Local file ready: {FILENAME}')
        result = {
            'status': 'IN_PROGRESS',
            'output_file_path': FILENAME,
            'github_url': '',
            'filename': FILENAME,
            'rows_testplan': tp_rows,
            'rows_metadata': md_rows,
            'commit_sha': '',
            'validation': 'PASSED'
        }
        print(json.dumps(result, indent=2))


if __name__ == '__main__':
    build()
