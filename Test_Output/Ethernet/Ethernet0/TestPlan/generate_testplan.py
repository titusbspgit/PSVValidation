#!/usr/bin/env python3
"""
Agent 7 - Excel TestPlan Generator
Generates Ethernet_TestPlan_YYYYMMDD_HHMMSS.xlsx with TestPlan and MetaData sheets.
Execute: python generate_testplan.py
"""
import json, os, sys
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
TIMESTAMP = now_ist.strftime("%Y%m%d_%H%M%S")
FILENAME = f"Ethernet_TestPlan_{TIMESTAMP}.xlsx"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_PATH = os.path.join(SCRIPT_DIR, FILENAME)

# Load JSON from external file or inline
JSON_PATH = os.path.join(SCRIPT_DIR, "testplan_data.json")

def load_json():
    if os.path.exists(JSON_PATH):
        with open(JSON_PATH, 'r') as f:
            return json.load(f)
    print("ERROR: testplan_data.json not found")
    sys.exit(1)

def create_workbook(data):
    wb = Workbook()
    
    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    
    tp_columns = [
        "Index", "SS / Module", "Feature", "Test Case Name",
        "Test Description", "Speed", "Mode", "Memory Start Offset",
        "Memory End Offset", "Remarks", "Test Steps / Procedure",
        "Impacted Registers", "Validation / Acceptance Criteria", "Code Generation"
    ]
    
    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet("MetaData")
    
    md_columns = [
        "Index", "Test Case Name", "Meta Test Description",
        "Meta Test Steps / Procedure", "Meta Impacted Registers",
        "Meta Validation / Acceptance Criteria", "Meta Headers",
        "Meta Macros", "Meta Arrays"
    ]
    
    # Formatting
    header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wrap_align = Alignment(wrap_text=True, vertical='top')
    thin_border = Border(
        left=Side(style='thin'), right=Side(style='thin'),
        top=Side(style='thin'), bottom=Side(style='thin')
    )
    
    def format_sheet(ws, columns):
        # Write headers
        for col_idx, col_name in enumerate(columns, 1):
            cell = ws.cell(row=1, column=col_idx, value=col_name)
            cell.font = header_font
            cell.fill = header_fill
            cell.alignment = wrap_align
            cell.border = thin_border
        # Freeze first row
        ws.freeze_panes = 'A2'
    
    def populate_sheet(ws, columns, data, field_map):
        for row_idx, item in enumerate(data, 2):
            for col_idx, col_name in enumerate(columns, 1):
                json_key = field_map.get(col_name, col_name)
                value = item.get(json_key, "")
                cell = ws.cell(row=row_idx, column=col_idx, value=value)
                cell.alignment = wrap_align
                cell.border = thin_border
    
    def auto_size(ws, columns, max_width=80):
        for col_idx, col_name in enumerate(columns, 1):
            max_len = len(col_name)
            for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
                for cell in row:
                    if cell.value:
                        lines = str(cell.value).split('\n')
                        for line in lines:
                            max_len = max(max_len, len(line))
            adjusted = min(max_len + 2, max_width)
            ws.column_dimensions[get_column_letter(col_idx)].width = adjusted
    
    # TestPlan field mapping (JSON key -> column name is same)
    tp_map = {c: c for c in tp_columns}
    
    # MetaData field mapping
    md_map = {c: c for c in md_columns}
    
    # Format and populate
    format_sheet(ws_tp, tp_columns)
    populate_sheet(ws_tp, tp_columns, data, tp_map)
    auto_size(ws_tp, tp_columns)
    
    format_sheet(ws_md, md_columns)
    populate_sheet(ws_md, md_columns, data, md_map)
    auto_size(ws_md, md_columns)
    
    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = 'veryHidden'
    
    return wb

def validate_workbook(filepath, data):
    wb = load_workbook(filepath)
    assert "TestPlan" in wb.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in wb.sheetnames, "MetaData sheet missing"
    
    ws_tp = wb["TestPlan"]
    ws_md = wb["MetaData"]
    
    assert ws_tp.max_row == len(data) + 1, f"TestPlan rows mismatch: {ws_tp.max_row}"
    assert ws_md.max_row == len(data) + 1, f"MetaData rows mismatch: {ws_md.max_row}"
    
    # Validate MetaData content matches JSON
    md_columns = [
        "Index", "Test Case Name", "Meta Test Description",
        "Meta Test Steps / Procedure", "Meta Impacted Registers",
        "Meta Validation / Acceptance Criteria", "Meta Headers",
        "Meta Macros", "Meta Arrays"
    ]
    
    for row_idx, item in enumerate(data, 2):
        for col_idx, col_name in enumerate(md_columns, 1):
            cell_val = ws_md.cell(row=row_idx, column=col_idx).value or ""
            json_val = item.get(col_name, "")
            if str(cell_val) != str(json_val):
                print(f"VALIDATION FAILED: Row {row_idx}, Col '{col_name}'")
                print(f"  Cell length: {len(str(cell_val))}")
                print(f"  JSON length: {len(str(json_val))}")
                return False
    
    print("VALIDATION PASSED: All MetaData cells match JSON values")
    return True

def main():
    data = load_json()
    print(f"Loaded {len(data)} testcases")
    
    wb = create_workbook(data)
    wb.save(OUTPUT_PATH)
    print(f"Workbook saved: {OUTPUT_PATH}")
    
    file_size = os.path.getsize(OUTPUT_PATH)
    print(f"File size: {file_size} bytes")
    assert file_size > 0, "File is empty"
    
    valid = validate_workbook(OUTPUT_PATH, data)
    
    print(f"\nFilename: {FILENAME}")
    print(f"Rows TestPlan: {len(data)}")
    print(f"Rows MetaData: {len(data)}")
    print(f"Validation: {'PASSED' if valid else 'FAILED'}")
    
    # Write output info
    with open(os.path.join(SCRIPT_DIR, "output_info.json"), 'w') as f:
        json.dump({
            "filename": FILENAME,
            "filepath": OUTPUT_PATH,
            "rows_testplan": len(data),
            "rows_metadata": len(data),
            "validation": "PASSED" if valid else "FAILED",
            "file_size": file_size
        }, f, indent=2)

if __name__ == "__main__":
    from openpyxl.utils import get_column_letter
    main()
