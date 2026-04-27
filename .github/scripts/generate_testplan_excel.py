#!/usr/bin/env python3
import json
import argparse
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from typing import List, Dict, Any
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

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

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

WRAP_COLUMNS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

HEADER_FILL = PatternFill(fill_type="solid", fgColor="4472C4")  # Excel blue
THIN = Side(style="thin", color="000000")
ALL_BORDERS = Border(left=THIN, right=THIN, top=THIN, bottom=THIN)


def normalize_value(val: Any) -> Any:
    # Convert arrays to newline-joined strings; keep scalars as-is
    if isinstance(val, list):
        return "\n".join(str(x) for x in val)
    return val


def union_keys_preserve_order(records: List[Dict[str, Any]]) -> List[str]:
    seen: List[str] = []
    s = set()
    for rec in records:
        for k in rec.keys():
            if k not in s:
                s.add(k)
                seen.append(k)
    return seen


def autofit_columns(ws):
    # Approximate auto-fit by measuring max string length per column
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            val = cell.value
            if val is None:
                continue
            # consider wrapped content lines length
            for part in str(val).split("\n"):
                max_len = max(max_len, len(part))
        width = min(max_len + 4, 100)
        ws.column_dimensions[col_letter].width = max(10, width)


def apply_header_format(ws):
    for cell in ws[1]:
        cell.font = Font(bold=True, color="FFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = HEADER_FILL
        cell.border = ALL_BORDERS


def apply_data_format(ws):
    headers = [c.value for c in ws[1]]
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            header = headers[cell.column - 1]
            if header in WRAP_COLUMNS:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            elif header == "Index":
                cell.alignment = Alignment(vertical="top", horizontal="center")
            else:
                cell.alignment = Alignment(vertical="top", horizontal="left")
            cell.border = ALL_BORDERS


def add_codegen_dropdown(ws):
    headers = [c.value for c in ws[1]]
    try:
        idx = headers.index("Code Generation (Required / Not)") + 1
    except ValueError:
        return
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True, errorTitle="Invalid", error="Select from dropdown: Required or Not Required, or leave blank")
    rng = f"{ws.cell(row=2, column=idx).coordinate}:{ws.cell(row=ws.max_row, column=idx).coordinate}"
    dv.add(rng)
    ws.add_data_validation(dv)


def set_hyperlinks(ws, testcases: List[Dict[str, Any]], context_list: List[str]):
    # Build mapping from "<name>/ – <url>"
    mapping: Dict[str, str] = {}
    for entry in context_list:
        if " – " in entry:
            name, url = entry.split(" – ", 1)
        elif " - " in entry:
            name, url = entry.split(" - ", 1)
        else:
            continue
        mapping[name.strip()] = url.strip()

    headers = [c.value for c in ws[1]]
    try:
        col_idx = headers.index("Test Case Name") + 1
    except ValueError:
        return

    for r, tc in enumerate(testcases, start=2):
        name = tc.get("Test Case Name")
        if not name:
            continue
        url = mapping.get(str(name).strip())
        if url:
            cell = ws.cell(row=r, column=col_idx)
            cell.hyperlink = url


def build_workbook(data: Dict[str, Any]) -> Workbook:
    # Determine testcases array
    testcases: List[Dict[str, Any]] = []
    if isinstance(data, dict):
        if isinstance(data.get("test_cases"), list):
            testcases = data["test_cases"]
        elif isinstance(data.get("TestCases"), list):
            testcases = data["TestCases"]
        else:
            raise ValueError("JSON must contain a non-empty array under 'test_cases' or 'TestCases'")
    else:
        raise ValueError("Root JSON must be an object")

    if not testcases:
        raise ValueError("Test cases array is empty")

    # Normalize values
    norm = []
    for tc in testcases:
        norm.append({k: normalize_value(v) for k, v in tc.items()})

    # Phase 1: Data sheet with union of all keys preserving first-seen order
    wb = Workbook()
    ws_data = wb.active
    ws_data.title = "Data"

    all_keys = union_keys_preserve_order(norm)
    ws_data.append(all_keys)
    for rec in norm:
        ws_data.append([rec.get(k, "") for k in all_keys])
    ws_data.freeze_panes = "A2"
    for c in ws_data[1]:
        c.font = Font(bold=True)
        c.alignment = Alignment(horizontal="center", vertical="center")
    autofit_columns(ws_data)

    # Phase 2: Meta_data_sheet with META columns (veryHidden)
    ws_meta = wb.create_sheet("Meta_data_sheet")
    ws_meta.append(META_COLUMNS)
    for tc in norm:
        ws_meta.append([tc.get(k, "") for k in META_COLUMNS])
    ws_meta.sheet_state = 'veryHidden'

    # Phase 2: Build TestPlan from Data with only MAIN columns in strict order
    # Create a temporary sheet and then rename
    ws_tmp = wb.create_sheet("TestPlan_tmp")
    ws_tmp.append(MAIN_COLUMNS)

    # Map headers in Data to indices
    head_map = {ws_data.cell(row=1, column=i).value: i for i in range(1, ws_data.max_column + 1)}

    for r in range(2, ws_data.max_row + 1):
        row_vals = []
        for col in MAIN_COLUMNS:
            src_idx = head_map.get(col)
            row_vals.append(ws_data.cell(row=r, column=src_idx).value if src_idx else "")
        ws_tmp.append(row_vals)

    # Remove Data and set TestPlan
    wb.remove(ws_data)
    ws_tmp.title = "TestPlan"
    ws_test = ws_tmp

    # Hyperlinks on Test Case Name
    context_list = data.get("context_folder_list", []) if isinstance(data, dict) else []
    if isinstance(context_list, list):
        set_hyperlinks(ws_test, testcases=testcases, context_list=context_list)

    # Strict formatting for TestPlan
    apply_header_format(ws_test)
    apply_data_format(ws_test)
    autofit_columns(ws_test)
    add_codegen_dropdown(ws_test)

    return wb


def main():
    ap = argparse.ArgumentParser(description="Generate formatted TestPlan Excel from JSON (Stage1 rules)")
    ap.add_argument("--json-file", "-i", required=True, help="Path to JSON test plan input")
    ap.add_argument("--ip", "-n", required=True, help="IP name for filename prefix")
    ap.add_argument("--outdir", "-o", required=True, help="Output directory inside repo")
    ap.add_argument("--git_message_out", "-gmsg_out", required=False, help="Path to write commit message text")
    ap.add_argument("--output_path_out", "-outpath_out", required=False, help="Path to write final Excel relative path")
    args = ap.parse_args()

    with open(args.json_file, "r", encoding="utf-8") as f:
        data = json.load(f)

    wb = build_workbook(data)

    ist_now = datetime.now(ZoneInfo("Asia/Kolkata"))
    ts = ist_now.strftime("%Y%m%d_%H%M%S")
    commit_ts = ist_now.strftime("%Y-%m-%d %H:%M:%S IST")

    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    outfile = outdir / f"{args.ip}_TestPlan_{ts}.xlsx"

    wb.save(str(outfile))

    commit_msg = f"Add {args.ip} TestPlan generated on {commit_ts} via Stage1 pipeline"

    if args.git_message_out:
        Path(args.git_message_out).write_text(commit_msg, encoding="utf-8")
    if args.output_path_out:
        Path(args.output_path_out).write_text(str(outfile), encoding="utf-8")

    print(f"WROTE:{outfile}")

if __name__ == "__main__":
    main()
