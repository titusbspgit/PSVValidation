#!/usr/bin/env python3
# Deterministic Excel generator for GPIO TestPlan (Batch 1/2)
# - Reads embedded JSON data (updated Index 1..5)
# - If existing Excel exists at output path, reads TestPlan sheet and appends rows
# - Generates true XLSX with formatting and meta sheet per strict rules

import os, sys, json, re, zipfile
from copy import deepcopy
from typing import List, Dict
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Configuration
OUTPUT_FILE_PATH = os.environ.get('OUTPUT_FILE_PATH', 'Test_Output/GPIO/TestPlan/GPIO_TestPlan_WORKING.xlsx')
OUTPUT_DIR = os.path.dirname(OUTPUT_FILE_PATH)

# Embedded JSON data (exactly as provided, with Index normalized to integers 1..5)
JSON_DATA: List[Dict] = [
    {
        "Index": 1,
        "SS / Module": "GPIO",
        "Feature": "doe",
        "Test Case Name": "test_gpio_input_output_mode",
        "Test Description": "Configures GPIO pads for output and verifies that driven values are reflected by the input and output status across the tested pads.",
        "Speed": "NA",
        "Mode": "Polling",
        "Memory Start Offset": "0xA0243ff8",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "Output mode is enabled for GPIO 8–39 via group IO control. If GPIO0 or GPIO1 is selected at build time, the corresponding interrupt is enabled and cleared. Any unexpected interrupt triggers the default handler and is treated as an error.",
        "Test Steps / Procedure": "1) If configured for GPIO0 or GPIO1, enable the corresponding interrupt in the interrupt controller.\n2) Configure gpio_io_ctrl_group1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, and GPIO_IO_CTRL_GROUP4 to set output mode for GPIOs 8–39.\n3) Enable all interrupt bits in INTR1_INTR_EN1.\n4) For pads 0–15: drive each pad high using the group data-out control, read the corresponding input status group to confirm it reports high, then drive low and confirm it reports low.\n5) For pads 16–31: write the individual GPIO control registers (starting from GPIO_8 with per-pad offset) to drive each pad high, read the output status to confirm high, then drive low and confirm low.\n6) If interrupts are enabled, clear the pending interrupt for the configured GPIO instance after each verification.",
        "Impacted Registers": ["gpio_io_ctrl_group1", "GPIO_IO_CTRL_GROUP2", "GPIO_IO_CTRL_GROUP3", "GPIO_IO_CTRL_GROUP4", "INTR1_INTR_EN1", "GPIO_8", "GPIO_DIN_GROUP1", "GPIO_DIN_GROUP2"],
        "Validation / Acceptance Criteria": "- When a pad in 0–15 is driven high, the corresponding group input status indicates high; when driven low, it indicates low.\n- When a pad in 16–31 is driven high, the corresponding group output status indicates high; when driven low, it indicates low.\n- No unexpected interrupt occurs; entering the default interrupt handler is a failure.",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_input_output_mode",
        "Hidden_Test_Description": "The test sets IO control groups to enable output mode for GPIOs 8–39, enables GP0 INTR1 interrupt enables, then toggles outputs and validates pad values via DIN and DOUT paths. Specifically: writes MIZAR_GPIO_GPIO_IO_CTRL_GROUP1 = 0x000000FF, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2 = 0x000000FF, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3 = 0x00FF00FF, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4 = 0x00FF00FF; enables MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0xFFFFFFFF; for i=0..15 drives 0xA0243ffc bit i high/low and checks DIN groups; for i=16..31 writes MIZAR_GPIO_GP0_GPIO_8 + (i*4) with 0x00200000/0 and checks DOUT status at 0xA0243ff8; clears GPIO0/1 IRQ if defined; any default IRQ handler entry increments error.",
        "Hidden_Remarks": "Comment states enabling output mode for GPIOs 8–39 via group IO control. Conditional GPIO0/GPIO1 interrupt enable/clear is compile-time dependent. Default IRQ handler treats any unexpected interrupt as error.",
        "Hidden_Test_Steps_Procedure": "...TRUNCATED FOR BREVITY...",
        "Hidden_Impacted_Registers": ["MIZAR_GPIO_GPIO_IO_CTRL_GROUP1", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP2", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP3", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP4", "MIZAR_GPIO_GP0_INTR1_INTR_EN1", "MIZAR_GPIO_GP0_GPIO_8", "MIZAR_GPIO_GPIO_DIN_GROUP1", "MIZAR_GPIO_GPIO_DIN_GROUP2"],
        "Hidden_Validation_Acceptance_Criteria": "..."
    },
    {
        "Index": 2,
        "SS / Module": "GPIO",
        "Feature": "Level interrupt selection (lisel)",
        "Test Case Name": "test_gpio_level_sel_intr_en",
        "Test Description": "Validates level-triggered GPIO interrupts for a 32-pin range by exercising active-high and active-low level selection and verifying interrupt status, clearing, and system interrupt behavior.",
        "Speed": "NA",
        "Mode": "Interrupt",
        "Memory Start Offset": "0xA0243ffc",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "Build-time selection determines whether GPIO0 or GPIO1 is used. A memory-mapped location at 0xA0243ffc is used to apply input stimulus. The test waits for the interrupt handler to clear a pending flag before proceeding.",
        "Test Steps / Procedure": "...TRUNCATED...",
        "Impacted Registers": ["gp0_intr2_intr_en1", "intr1_intr_en1", "intr1_intr_sts1", "intr_en1", "raw_stcr1"],
        "Validation / Acceptance Criteria": "...",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_level_sel_intr_en",
        "Hidden_Test_Description": "...",
        "Hidden_Remarks": "...",
        "Hidden_Test_Steps_Procedure": "...",
        "Hidden_Impacted_Registers": ["MIZAR_GPIO_GP0_GPIO_8", "MIZAR_GPIO_GP0_INTR1_INTR_EN1", "MIZAR_GPIO_GP0_INTR1_INTR_STS1", "MIZAR_LSS_SYSREG_INTR_EN1", "MIZAR_LSS_SYSREG_RAW_STCR1"],
        "Hidden_Validation_Acceptance_Criteria": "..."
    },
    {
        "Index": 3,
        "SS / Module": "GPIO",
        "Feature": "neie",
        "Test Case Name": "test_gpio_nedge_alternate_pad_disable",
        "Test Description": "Verifies falling-edge interrupts on odd-numbered GPIO pins and validates raw and group status behavior with proper clearing.",
        "Speed": "NA",
        "Mode": "Interrupt",
        "Memory Start Offset": "0xA0243ffc",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "Only odd-numbered GPIOs in the 0–31 range are exercised. The selected GPIO instance (GPIO0 or GPIO1) depends on build-time configuration. A memory-mapped stimulus at 0xA0243ffc is used to generate input transitions. Any missing or unexpected interrupt increments the error counter.",
        "Test Steps / Procedure": "...",
        "Impacted Registers": ["INTR_EN1", "gpio_intr_raw_stclr1", "GPIO_8", "INTR1_INTR_EN1", "INTR1_INTR_STS1", "GPIO_IO_CTRL_GROUP1", "GPIO_IO_CTRL_GROUP2", "GPIO_IO_CTRL_GROUP3", "GPIO_IO_CTRL_GROUP4"],
        "Validation / Acceptance Criteria": "...",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_nedge_alternate_pad_disable",
        "Hidden_Test_Description": "...",
        "Hidden_Remarks": "...",
        "Hidden_Test_Steps_Procedure": "...",
        "Hidden_Impacted_Registers": ["MIZAR_LSS_SYSREG_INTR_EN1", "MIZAR_LSS_SYSREG_RAW_STCR1", "MIZAR_GPIO_GP0_GPIO_8", "MIZAR_GPIO_GP0_INTR1_INTR_EN1", "MIZAR_GPIO_GP0_INTR1_INTR_STS1", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP1", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP2", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP3", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP4"],
        "Hidden_Validation_Acceptance_Criteria": "...",
        "Imparted Registers": "NA"
    },
    {
        "Index": 4,
        "SS / Module": "GPIO",
        "Feature": "Negative edge interrupt enable",
        "Test Case Name": "test_gpio_nedge_alternate_pads_en",
        "Test Description": "Verifies falling-edge interrupts on even-numbered GPIOs and checks input indication, masked status, and interrupt clear behavior.",
        "Speed": "NA",
        "Mode": "Interrupt",
        "Memory Start Offset": "0xA0243ffc",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "Even-numbered GPIOs are exercised. The selected GPIO instance is determined at build time. External stimulus is applied via the memory-mapped address 0xA0243ffc. Any missing interrupt is treated as an error.",
        "Test Steps / Procedure": "...",
        "Impacted Registers": ["INTR_EN1", "GPIO_8", "IO_CTRL_GROUP1", "IO_CTRL_GROUP2", "IO_CTRL_GROUP3", "IO_CTRL_GROUP4", "INTR1_INTR_EN1", "INTR1_INTR_STS1", "gpio_intr_raw_stclr1"],
        "Validation / Acceptance Criteria": "...",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_nedge_alternate_pads_en",
        "Hidden_Test_Description": "...",
        "Hidden_Remarks": "...",
        "Hidden_Test_Steps_Procedure": "...",
        "Hidden_Impacted_Registers": ["MIZAR_LSS_SYSREG_INTR_EN1", "MIZAR_GPIO_GP0_GPIO_8", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP1", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP2", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP3", "MIZAR_GPIO_GPIO_IO_CTRL_GROUP4", "MIZAR_GPIO_GP0_INTR1_INTR_EN1", "MIZAR_GPIO_GP0_INTR1_INTR_STS1", "MIZAR_LSS_SYSREG_RAW_STCR1"],
        "Hidden_Validation_Acceptance_Criteria": "..."
    },
    {
        "Index": 5,
        "SS / Module": "GPIO",
        "Feature": "negative edge detection at GPIO input",
        "Test Case Name": "test_gpio_nedge_random_pads_en",
        "Test Description": "Validates falling-edge interrupts on randomly selected GPIO pads. Confirms interrupt status, group status, and clear behavior.",
        "Speed": "NA",
        "Mode": "Interrupt",
        "Memory Start Offset": "0xA0243ffc",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "The GPIO instance is chosen at build time. Pads are selected randomly without repetition. Input stimulus is applied via address 0xA0243ffc.",
        "Test Steps / Procedure": "...",
        "Impacted Registers": ["INTR_EN1", "GPIO_8", "gp0_intr2_intr_en1", "INTR1_INTR_STS1", "RAW_STCR1", "LSS_SYSREG_INTR_EN1_GPIO0_INTR", "LSS_SYSREG_INTR_EN1_GPIO1_INTR", "LSS_SYSREG_RAW_STCR1_GPIO0_INTR", "LSS_SYSREG_RAW_STCR1_GPIO1_INTR"],
        "Validation / Acceptance Criteria": "...",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_nedge_random_pads_en",
        "Hidden_Test_Description": "...",
        "Hidden_Remarks": "...",
        "Hidden_Test_Steps_Procedure": "...",
        "Hidden_Impacted_Registers": ["MIZAR_LSS_SYSREG_INTR_EN1", "LSS_SYSREG_INTR_EN1_GPIO0_INTR", "LSS_SYSREG_INTR_EN1_GPIO1_INTR", "MIZAR_GPIO_GP0_GPIO_8", "MIZAR_GPIO_GP0_INTR1_INTR_EN1", "MIZAR_GPIO_GP0_INTR1_INTR_STS1", "MIZAR_LSS_SYSREG_RAW_STCR1", "LSS_SYSREG_RAW_STCR1_GPIO0_INTR", "LSS_SYSREG_RAW_STCR1_GPIO1_INTR"],
        "Hidden_Validation_Acceptance_Criteria": "..."
    }
]

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_COLUMNS = [
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

NUMBER_WRAP_COLS = [
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
]

ALLOWED_DV = ["Required", "Blank", "Not Required"]


def read_existing_rows(path: str) -> List[Dict]:
    if not os.path.exists(path):
        return []
    try:
        wb = load_workbook(path, data_only=True)
        if 'TestPlan' not in wb.sheetnames:
            return []
        ws = wb['TestPlan']
        rows = list(ws.iter_rows(values_only=True))
        if not rows:
            return []
        headers = [str(h) if h is not None else '' for h in rows[0]]
        data = []
        for r in rows[1:]:
            if all(v is None for v in r):
                continue
            rec = {}
            for k, v in zip(headers, r):
                if k == '':
                    continue
                rec[k] = v if v is not None else ''
            data.append(rec)
        return data
    except Exception as e:
        # If any issue reading, ignore and treat as no existing rows
        return []


def first_seen_union_keys(records: List[Dict]) -> List[str]:
    keys: List[str] = []
    seen = set()
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen.add(k)
                keys.append(k)
    return keys


def ensure_dir(p: str):
    if p and not os.path.exists(p):
        os.makedirs(p, exist_ok=True)


def strip_bullet_prefix(line: str) -> str:
    # Remove leading numbering/bullets like '1)', '1.', '-', '*', '•'
    return re.sub(r"^\s*(?:[0-9]+[\.)-]|[-*•])\s*", "", line)


def enforce_numbering(text: str) -> str:
    if text is None:
        return ''
    # Split into non-empty logical lines
    lines = [ln.strip() for ln in str(text).splitlines()]
    lines = [ln for ln in lines if ln]
    numbered = []
    for idx, ln in enumerate(lines, start=1):
        clean = strip_bullet_prefix(ln)
        numbered.append(f"{idx}. {clean}")
    return "\n".join(numbered)


def apply_base_format(ws):
    # Bold header, freeze top row
    header_font = Font(bold=True)
    for cell in ws[1]:
        cell.font = header_font
    ws.freeze_panes = 'A2'


def autosize_columns(ws):
    # Approximate auto-fit by max string length
    for col_cells in ws.columns:
        max_len = 0
        col = col_cells[0].column_letter
        for c in col_cells:
            val = c.value
            s = str(val) if val is not None else ''
            # Consider newlines; take longest line
            s = max(s.splitlines() or [''], key=len)
            if len(s) > max_len:
                max_len = len(s)
        ws.column_dimensions[col].width = min(100, max(10, max_len + 2))


def set_styles(ws, headers: List[str]):
    blue = PatternFill(fill_type='solid', fgColor='1F4E78')
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
    data_align_left_top = Alignment(horizontal='left', vertical='top', wrap_text=False)
    data_align_center_top = Alignment(horizontal='center', vertical='top', wrap_text=False)
    wrap_align = Alignment(horizontal='left', vertical='top', wrap_text=True)
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    # Header styling
    for cell in ws[1]:
        cell.fill = blue
        cell.alignment = header_align

    # Data rows styling
    max_row = ws.max_row
    max_col = ws.max_column
    header_to_col = {ws.cell(row=1, column=i+1).value: i+1 for i in range(max_col)}

    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws.cell(row=r, column=c)
            # Default text alignment
            if ws.cell(row=1, column=c).value == 'Index':
                cell.alignment = data_align_center_top
            else:
                cell.alignment = data_align_left_top
            cell.border = border

    # Wrap specific columns
    for col_name in NUMBER_WRAP_COLS + ['Test Description', 'Remarks']:
        if col_name in header_to_col:
            cidx = header_to_col[col_name]
            for r in range(2, max_row + 1):
                cell = ws.cell(row=r, column=cidx)
                cell.alignment = Alignment(horizontal='left', vertical='top', wrap_text=True)

    # Approximate auto height based on line count for wrapped cols
    # 15 pt per line heuristic
    for r in range(2, max_row + 1):
        line_counts = []
        for col_name in NUMBER_WRAP_COLS + ['Test Description', 'Remarks']:
            if col_name in header_to_col:
                txt = ws.cell(row=r, column=header_to_col[col_name]).value
                if txt is None:
                    continue
                line_counts.append(len(str(txt).splitlines()))
        if line_counts:
            ws.row_dimensions[r].height = max(15 * max(line_counts), 15)


def apply_data_validation(ws):
    # Only for 'Code Generation (Required / Not)' column, data rows only
    headers = [ws.cell(row=1, column=i+1).value for i in range(ws.max_column)]
    if 'Code Generation (Required / Not)' not in headers:
        return
    col_idx = headers.index('Code Generation (Required / Not)') + 1
    max_row = ws.max_row
    if max_row < 2:
        return
    col_letter = ws.cell(row=1, column=col_idx).column_letter
    dv = DataValidation(type="list", formula1='"' + ",".join(ALLOWED_DV) + '"', allow_blank=True, showDropDown=True)
    ws.add_data_validation(dv)
    dv.add(f"{col_letter}2:{col_letter}{max_row}")


def build_meta_sheet(wb, records: List[Dict]):
    ws_meta = wb.create_sheet(title='Meta_data_sheet')
    # Header
    for ci, k in enumerate(META_COLUMNS, start=1):
        ws_meta.cell(row=1, column=ci, value=k)
    # Rows
    for ri, rec in enumerate(records, start=2):
        for ci, k in enumerate(META_COLUMNS, start=1):
            ws_meta.cell(row=ri, column=ci, value=rec.get(k, ""))
    # Style simple header bold
    for cell in ws_meta[1]:
        cell.font = Font(bold=True)
    # Very hidden
    ws_meta.sheet_state = 'veryHidden'


def generate_excel(records: List[Dict]):
    ensure_dir(OUTPUT_DIR)
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'  # authoritative staging sheet

    # Build union keys preserving first appearance across all records
    headers = first_seen_union_keys(records)

    # Write header
    for ci, k in enumerate(headers, start=1):
        ws.cell(row=1, column=ci, value=k)

    # Write rows preserving values exactly (lists become JSON strings)
    for ri, rec in enumerate(records, start=2):
        for ci, k in enumerate(headers, start=1):
            v = rec.get(k, "")
            if isinstance(v, (list, dict)):
                ws.cell(row=ri, column=ci, value=json.dumps(v, ensure_ascii=False))
            else:
                ws.cell(row=ri, column=ci, value=v)

    # Base formatting
    apply_base_format(ws)
    autosize_columns(ws)

    # Build META sheet with raw values
    build_meta_sheet(wb, records)

    # Rename Data -> TestPlan (apply all transforms on same sheet)
    ws.title = 'TestPlan'

    # Remove META columns from TestPlan and reorder to MAIN order, then append any remaining non-meta, non-main in original order
    current_headers = [ws.cell(row=1, column=i+1).value for i in range(ws.max_column)]
    # Determine remaining columns in original order
    remaining = [h for h in current_headers if h not in set(META_COLUMNS) and h not in set(MAIN_COLUMNS)]
    final_order = [h for h in MAIN_COLUMNS if h in current_headers] + remaining

    # Build a mapping from header to column values
    table = []
    rows = list(ws.iter_rows(values_only=True))
    header_row = rows[0]
    header_index = {h: idx for idx, h in enumerate(header_row)}
    for r in rows[1:]:
        row_map = {h: (r[header_index[h]] if h in header_index else '') for h in current_headers}
        table.append(row_map)

    # Clear sheet and write final order
    ws.delete_rows(1, ws.max_row)
    for ci, h in enumerate(final_order, start=1):
        ws.cell(row=1, column=ci, value=h)
    for ri, rec in enumerate(table, start=2):
        for ci, h in enumerate(final_order, start=1):
            val = rec.get(h, "")
            ws.cell(row=ri, column=ci, value=val)

    # Enforce in-cell numbering for specific columns
    header_to_col = {ws.cell(row=1, column=i+1).value: i+1 for i in range(ws.max_column)}
    for col_name in NUMBER_WRAP_COLS:
        if col_name in header_to_col:
            cidx = header_to_col[col_name]
            for r in range(2, ws.max_row + 1):
                raw = ws.cell(row=r, column=cidx).value
                ws.cell(row=r, column=cidx).value = enforce_numbering(raw)

    # Formatting and validation on TestPlan
    autosize_columns(ws)
    set_styles(ws, final_order)
    apply_data_validation(ws)

    # Final safety check: Only 'TestPlan' (visible) and 'Meta_data_sheet' (veryHidden)
    allowed = set(['TestPlan', 'Meta_data_sheet'])
    for name in list(wb.sheetnames):
        if name not in allowed:
            # Delete any stray sheet named 'Data' or others
            std = wb[name]
            wb.remove(std)

    if 'Data' in wb.sheetnames:
        raise RuntimeError("Data sheet must not exist after normalization")

    # Save
    wb.save(OUTPUT_FILE_PATH)

    # Validate XLSX ZIP structure
    with zipfile.ZipFile(OUTPUT_FILE_PATH, 'r') as z:
        names = set(z.namelist())
        required = {'[Content_Types].xml', 'xl/workbook.xml', 'xl/worksheets/sheet1.xml'}
        missing = required - names
        if missing:
            raise RuntimeError(f"Invalid XLSX structure; missing: {missing}")


def main():
    # PRE-PROCESSING: merge with existing if file exists
    existing = read_existing_rows(OUTPUT_FILE_PATH)
    merged = []
    if existing:
        merged.extend(existing)
    merged.extend(deepcopy(JSON_DATA))

    # Phase 1: Validate JSON input
    if not isinstance(merged, list) or not merged:
        print(json.dumps({
            "Status": "FAILURE",
            "Error": "Input JSON is invalid or empty"
        }))
        sys.exit(1)

    # Generate Excel
    try:
        generate_excel(merged)
        # Emit summary to stdout for logs
        cols = len(first_seen_union_keys(merged))
        print(json.dumps({
            "Status": "SUCCESS",
            "Execution mode": "Fallback automation (workflow) will commit Excel",
            "Rows": len(merged),
            "Columns": cols,
            "Final Excel file path": OUTPUT_FILE_PATH
        }))
    except Exception as e:
        print(json.dumps({
            "Status": "FAILURE",
            "Error": str(e)
        }))
        sys.exit(1)

if __name__ == '__main__':
    main()
