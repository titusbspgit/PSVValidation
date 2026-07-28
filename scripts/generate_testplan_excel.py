#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from typing import Any, Dict, List

from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment

TESTPLAN_COLUMNS = [
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

METADATA_COLUMNS = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]

# Utility: normalize keys for fuzzy matching
import re

def norm_key(k: str) -> str:
    return re.sub(r"[^a-z0-9]", "", k.lower())

# Build fuzzy mapping for common aliases
ALIASES = {
    # TestPlan
    "ss/module": "SS / Module",
    "ss module": "SS / Module",
    "ssmodule": "SS / Module",
    "module": "SS / Module",
    "feature": "Feature",
    "testcasename": "Test Case Name",
    "test_case_name": "Test Case Name",
    "name": "Test Case Name",
    "testdescription": "Test Description",
    "description": "Test Description",
    "speed": "Speed",
    "mode": "Mode",
    "memorystartoffset": "Memory Start Offset",
    "memory_start_offset": "Memory Start Offset",
    "startoffset": "Memory Start Offset",
    "memoryendoffset": "Memory End Offset",
    "memory_end_offset": "Memory End Offset",
    "endoffset": "Memory End Offset",
    "remarks": "Remarks",
    "notes": "Remarks",
    "teststeps/procedure": "Test Steps / Procedure",
    "teststepsprocedure": "Test Steps / Procedure",
    "teststeps": "Test Steps / Procedure",
    "procedure": "Test Steps / Procedure",
    "impactedregisters": "Impa cted Registers".replace(" ", ""),  # ensure correct normalization
    "impacted_registers": "Impa cted Registers".replace(" ", ""),
    # We'll handle impacted registers directly below
    "validation/acceptancecriteria": "Validation / Acceptance Criteria",
    "validationacceptancecriteria": "Validation / Acceptance Criteria",
    "acceptancecriteria": "Validation / Acceptance Criteria",
    "codegeneration(required/not)": "Code Generation (Required / Not)",
    "codegeneration": "Code Generation (Required / Not)",
    "code_generation": "Code Generation (Required / Not)",
    # Metadata
    "metatestdescription": "Meta Test Description",
    "meta_test_description": "Meta Test Description",
    "metateststeps/procedure": "Meta Test Steps / Procedure",
    "metateststepsprocedure": "Meta Test Steps / Procedure",
    "metateststeps": "Meta Test Steps / Procedure",
    "metaimpactedregisters": "Meta Impacted Registers",
    "meta_impacted_registers": "Meta Impacted Registers",
    "metavalidation/acceptancecriteria": "Meta Validation / Acceptance Criteria",
    "metavalidationacceptancecriteria": "Meta Validation / Acceptance Criteria",
    "metaheaders": "Meta Headers",
    "metamacros": "Meta Macros",
    "metaarrays": "Meta Arrays",
}

# Fix for impacted registers alias since above had a trick
ALIASES["impactedregisters"] = "Impacted Registers"
ALIASES["impacted_registers"] = "Impacted Registers"

# Precompute normalized canonical names for mapping
CANONICALS = {c: norm_key(c) for c in TESTPLAN_COLUMNS + METADATA_COLUMNS}

# Create alias map normalized -> canonical
ALIAS_TO_CANON = {}
for raw_alias, canon in ALIASES.items():
    ALIAS_TO_CANON[norm_key(raw_alias)] = canon


def coerce_str(v: Any) -> str:
    if v is None:
        return ""
    if isinstance(v, (str, int, float, bool)):
        return str(v)
    try:
        return json.dumps(v, ensure_ascii=False)
    except Exception:
        return str(v)


def apply_header_style(ws):
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="1F4E78", end_color="1F4E78", fill_type="solid")
    for cell in ws[1]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = Alignment(wrap_text=True, vertical="top")
    ws.freeze_panes = "A2"


def wrap_all_cells(ws):
    for row in ws.iter_rows(min_row=1):
        for cell in row:
            cell.alignment = Alignment(wrap_text=True, vertical="top")


def adjust_column_widths(ws, preferred_wide_cols: List[str] = None):
    preferred_wide_cols = preferred_wide_cols or []
    # Map column index to header
    headers = [c.value or "" for c in ws[1]]
    col_max = {}
    for col_idx, header in enumerate(headers, start=1):
        col_letter = ws.cell(row=1, column=col_idx).column_letter
        max_len = len(coerce_str(header))
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            v = row[0].value
            if v is None:
                continue
            l = len(coerce_str(v))
            if l > max_len:
                max_len = l
        # Set bounds
        if header in preferred_wide_cols:
            width = min(max(30, max_len + 2), 70)
        else:
            width = min(max(12, max_len + 2), 40)
        ws.column_dimensions[col_letter].width = width


def build_row_dicts(item: Dict[str, Any], index_val: int) -> (Dict[str, str], Dict[str, str]):
    # Initialize rows with blanks
    test_row = {col: "" for col in TESTPLAN_COLUMNS}
    meta_row = {col: "" for col in METADATA_COLUMNS}
    test_row["Index"] = index_val
    meta_row["Index"] = index_val

    # Helpers to collect unmapped fields
    unmapped_normal: Dict[str, Any] = {}
    unmapped_meta: Dict[str, Any] = {}

    # Prefill shared "Test Case Name" if present with any alias
    # Then iterate keys
    for k, v in item.items():
        nk = norm_key(k)
        # Exact canonical match?
        placed = False
        # Check if it's a meta-like key based on prefix "meta"
        is_meta_like = nk.startswith("meta")

        # Try alias mapping first
        canon = ALIAS_TO_CANON.get(nk)
        if canon in TESTPLAN_COLUMNS:
            test_row[canon] = coerce_str(v)
            placed = True
        elif canon in METADATA_COLUMNS:
            meta_row[canon] = coerce_str(v)
            placed = True
        else:
            # Try exact canonical name normalization matching
            # Iterate through canonical names and compare normalized
            for c in TESTPLAN_COLUMNS:
                if nk == CANONICALS[c]:
                    test_row[c] = coerce_str(v)
                    placed = True
                    break
            if not placed:
                for c in METADATA_COLUMNS:
                    if nk == CANONICALS[c]:
                        meta_row[c] = coerce_str(v)
                        placed = True
                        break
        if not placed:
            # Not matched; store into unmapped buckets
            if is_meta_like:
                unmapped_meta[k] = v
            else:
                unmapped_normal[k] = v

    # Stuff unmapped fields into Remarks / Meta Headers to avoid data loss
    if unmapped_normal:
        extra = coerce_str(unmapped_normal)
        if test_row["Remarks"]:
            test_row["Remarks"] += " | Extra: " + extra
        else:
            test_row["Remarks"] = extra
    if unmapped_meta:
        extra_m = coerce_str(unmapped_meta)
        if meta_row["Meta Headers"]:
            meta_row["Meta Headers"] += " | Extra: " + extra_m
        else:
            meta_row["Meta Headers"] = extra_m

    # Ensure Test Case Name consistency across sheets if present in one
    tcn = test_row.get("Test Case Name") or meta_row.get("Test Case Name")
    if tcn:
        test_row["Test Case Name"] = tcn
        meta_row["Test Case Name"] = tcn

    return test_row, meta_row


def create_workbook(rows: List[Dict[str, Any]]):
    wb = Workbook()
    # Create visible TestPlan sheet as active
    ws_test = wb.active
    ws_test.title = "TestPlan"
    # Create MetaData sheet and set veryHidden later
    ws_meta = wb.create_sheet("MetaData")

    # Write headers
    ws_test.append(TESTPLAN_COLUMNS)
    ws_meta.append(METADATA_COLUMNS)

    # Fill rows
    for i, item in enumerate(rows, start=1):
        # Determine index value
        idx_src = item.get("Index")
        try:
            idx = int(idx_src) if idx_src is not None and str(idx_src).strip() != "" else i
        except Exception:
            idx = i
        test_row, meta_row = build_row_dicts(item, idx)
        ws_test.append([test_row[c] for c in TESTPLAN_COLUMNS])
        ws_meta.append([meta_row[c] for c in METADATA_COLUMNS])

    # Styling and formatting
    apply_header_style(ws_test)
    apply_header_style(ws_meta)
    wrap_all_cells(ws_test)
    wrap_all_cells(ws_meta)

    # Adjust widths
    wide_test_cols = [
        "Test Description",
        "Test Steps / Procedure",
        "Impacted Registers",
        "Validation / Acceptance Criteria",
        "Remarks",
    ]
    wide_meta_cols = [
        "Meta Test Description",
        "Meta Test Steps / Procedure",
        "Meta Impacted Registers",
        "Meta Validation / Acceptance Criteria",
        "Meta Headers",
        "Meta Macros",
        "Meta Arrays",
    ]
    adjust_column_widths(ws_test, preferred_wide_cols=wide_test_cols)
    adjust_column_widths(ws_meta, preferred_wide_cols=wide_meta_cols)

    # Very hide the MetaData sheet
    ws_meta.sheet_state = "veryHidden"

    return wb


def main():
    parser = argparse.ArgumentParser(description="Generate Test Plan Excel from JSON")
    parser.add_argument("--json-path", required=True, help="Path to JSON file containing an array of test cases")
    parser.add_argument("--output-dir", default="Test_Output", help="Directory to write the generated Excel file")
    args = parser.parse_args()

    if not os.path.exists(args.json_path):
        raise FileNotFoundError(f"JSON file not found: {args.json_path}")

    with open(args.json_path, "r", encoding="utf-8") as f:
        try:
            data = json.load(f)
        except json.JSONDecodeError as e:
            raise SystemExit(f"Invalid JSON: {e}")

    if not isinstance(data, list):
        raise SystemExit("Top-level JSON must be an array of objects.")
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            raise SystemExit(f"Each array element must be an object. Element {i} is {type(item)}")

    wb = create_workbook(data)

    # IST timestamp
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    os.makedirs(args.output_dir, exist_ok=True)
    out_path = os.path.join(args.output_dir, f"testplan_{ts}.xlsx")
    wb.save(out_path)

    # Print the output path for logs and also expose as GitHub Actions output if available
    print(out_path)
    if os.environ.get("GITHUB_OUTPUT"):
        with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as gh_out:
            gh_out.write(f"output_path={out_path}\n")


if __name__ == "__main__":
    main()
