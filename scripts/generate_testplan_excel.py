#!/usr/bin/env python3
import json, os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# Aggregated JSON injected directly; preserve exact order/values
AGG_JSON = r'''[
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "NA",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Sequentially read GPIO group 0 registers for pins 8–27 and the soft reset register; record values.",
    "Meta Test Description": "The test performs read operations in order on the following register macros: MIZAR_GPIO_GP0_GPIO_8 through MIZAR_GPIO_GP0_GPIO_27, followed by SOFT_RST_REG_ADDRESS. Resolved addresses are available for GPIO_8 (0xA001A000) through GPIO_27 (0xA001A04C). SOFT_RST_REG_ADDRESS has no resolved address. No write, comparison, or assertion logic is specified; values are captured.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "0xA001A000",
    "Memory End Offset": "0xA001A04C",
    "Remarks": "NA",
    "Test Steps / Procedure": "1. Read GPIO group 0 registers for pins 8–27 and record the values.\n2. Read the soft reset register and record the value.",
    "Meta Test Steps / Procedure": "1) Read MIZAR_GPIO_GP0_GPIO_8 (0xA001A000) and capture the value.\n2) Read MIZAR_GPIO_GP0_GPIO_9 (0xA001A004) and capture the value.\n3) Read MIZAR_GPIO_GP0_GPIO_10 (0xA001A008) and capture the value.\n4) Read MIZAR_GPIO_GP0_GPIO_11 (0xA001A00C) and capture the value.\n5) Read MIZAR_GPIO_GP0_GPIO_12 (0xA001A010) and capture the value.\n6) Read MIZAR_GPIO_GP0_GPIO_13 (0xA001A014) and capture the value.\n7) Read MIZAR_GPIO_GP0_GPIO_14 (0xA001A018) and capture the value.\n8) Read MIZAR_GPIO_GP0_GPIO_15 (0xA001A01C) and capture the value.\n9) Read MIZAR_GPIO_GP0_GPIO_16 (0xA001A020) and capture the value.\n10) Read MIZAR_GPIO_GP0_GPIO_17 (0xA001A024) and capture the value.\n11) Read MIZAR_GPIO_GP0_GPIO_18 (0xA001A028) and capture the value.\n12) Read MIZAR_GPIO_GP0_GPIO_19 (0xA001A02C) and capture the value.\n13) Read MIZAR_GPIO_GP0_GPIO_20 (0xA001A030) and capture the value.\n14) Read MIZAR_GPIO_GP0_GPIO_21 (0xA001A034) and capture the value.\n15) Read MIZAR_GPIO_GP0_GPIO_22 (0xA001A038) and capture the value.\n16) Read MIZAR_GPIO_GP0_GPIO_23 (0xA001A03C) and capture the value.\n17) Read MIZAR_GPIO_GP0_GPIO_24 (0xA001A040) and capture the value.\n18) Read MIZAR_GPIO_GP0_GPIO_25 (0xA001A044) and capture the value.\n19) Read MIZAR_GPIO_GP0_GPIO_26 (0xA001A048) and capture the value.\n20) Read MIZAR_GPIO_GP0_GPIO_27 (0xA001A04C) and capture the value.\n21) Read SOFT_RST_REG_ADDRESS (address NA) and capture the value.\nNo writes, loops, waits, interrupts, or assertions are specified.",
    "Impacted Registers": "NA",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; MIZAR_GPIO_GP0_GPIO_11; MIZAR_GPIO_GP0_GPIO_12; MIZAR_GPIO_GP0_GPIO_13; MIZAR_GPIO_GP0_GPIO_14; MIZAR_GPIO_GP0_GPIO_15; MIZAR_GPIO_GP0_GPIO_16; MIZAR_GPIO_GP0_GPIO_17; MIZAR_GPIO_GP0_GPIO_18; MIZAR_GPIO_GP0_GPIO_19; MIZAR_GPIO_GP0_GPIO_20; MIZAR_GPIO_GP0_GPIO_21; MIZAR_GPIO_GP0_GPIO_22; MIZAR_GPIO_GP0_GPIO_23; MIZAR_GPIO_GP0_GPIO_24; MIZAR_GPIO_GP0_GPIO_25; MIZAR_GPIO_GP0_GPIO_26; MIZAR_GPIO_GP0_GPIO_27; SOFT_RST_REG_ADDRESS",
    "Validation / Acceptance Criteria": "NA",
    "Meta Validation / Acceptance Criteria": "NA",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  }
]'''


def build_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"
    columns = list(rows[0].keys()) if rows else []
    ws.append(columns)
    for row in rows:
        ws.append([row.get(col, "") for col in columns])
    # styles
    header_font = Font(bold=True)
    header_fill = PatternFill(start_color="FFDDEBF7", end_color="FFDDEBF7", fill_type="solid")
    wrap = Alignment(wrap_text=True, vertical="top")
    for cell in ws[1]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap
    for r in ws.iter_rows(min_row=2):
        for cell in r:
            cell.alignment = wrap
    ws.freeze_panes = "A2"
    # widths
    from openpyxl.utils import get_column_letter
    for idx, col in enumerate(columns, start=1):
        max_len = max([len(str(col))] + [len(str(r.get(col, ""))) for r in rows]) if rows else len(str(col))
        ws.column_dimensions[get_column_letter(idx)].width = min(max_len + 2, 100)
    # MetaData sheet
    meta = wb.create_sheet("MetaData")
    meta.sheet_state = "veryHidden"
    meta.append(["Key", "Value"])
    tz = timezone(timedelta(hours=5, minutes=30))
    now = datetime.now(tz)
    for cell in meta[1]:
        cell.font = Font(bold=True)
        cell.fill = header_fill
        cell.alignment = wrap
    meta_rows = [
        ("Repo", "titusbspgit/PSVValidation"),
        ("Branch", "main"),
        ("IP_NAME", "GPIO"),
        ("Generated On IST", now.strftime("%Y-%m-%d %H:%M:%S %z")),
        ("Source JSON Row Count", str(len(rows))),
        ("Columns Count", str(len(columns)))
    ]
    for k, v in meta_rows:
        meta.append([k, v])
        for cell in meta[meta.max_row]:
            cell.alignment = wrap
    return wb


def main():
    rows = json.loads(AGG_JSON)
    tz = timezone(timedelta(hours=5, minutes=30))
    now = datetime.now(tz)
    ts = now.strftime('%Y%m%d_%H%M%S')
    out_dir = os.path.join('Test_Output', 'GPIO', 'TestPlan')
    os.makedirs(out_dir, exist_ok=True)
    filename = f"GPIO_TestPlan_{ts}.xlsx"
    path = os.path.join(out_dir, filename)
    wb = build_workbook(rows)
    wb.save(path)
    print(path)

if __name__ == '__main__':
    main()
