#!/usr/bin/env python3
"""
Agent 7 - Excel Generation and GitHub Publishing
Generates MIPI_DSI_TestPlan_20261110_020000.xlsx
Two sheets: TestPlan (visible), MetaData (veryHidden)
Run: pip install openpyxl requests && python generate_MIPI_DSI_TestPlan_20261110_020000.py
"""
import json, os, base64, requests
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation

TESTPLAN_JSON = json.loads(open("MIPI_DSI_TestPlan_20261110_020000_testplan.json").read())
META_JSON = json.loads(open("MIPI_DSI_TestPlan_20261110_020000_metadata.json").read())

FILENAME = "MIPI_DSI_TestPlan_20261110_020000.xlsx"

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

def style_sheet(ws, columns):
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = WRAP
    ws.freeze_panes = "A2"

def auto_width(ws, max_w=80):
    for col in ws.columns:
        mx = 0
        for cell in col:
            if cell.value:
                lines = str(cell.value).split("\n")
                mx = max(mx, max(len(l) for l in lines))
        adj = min(mx + 4, max_w)
        ws.column_dimensions[col[0].column_letter].width = adj

wb = Workbook()

# --- TestPlan sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"
style_sheet(ws_tp, TESTPLAN_COLS)

for row_idx, tc in enumerate(TESTPLAN_JSON, 2):
    for col_idx, col_name in enumerate(TESTPLAN_COLS, 1):
        if col_name == "Code Generation (Required / Not)":
            val = ""
        else:
            val = tc.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = WRAP

dv = DataValidation(
    type="list",
    formula1='"Required,Not Required"',
    allow_blank=True,
    showDropDown=False
)
dv.error = "Please select Required or Not Required"
dv.errorTitle = "Invalid Entry"
code_gen_col = len(TESTPLAN_COLS)
for r in range(2, len(TESTPLAN_JSON) + 2):
    dv.add(ws_tp.cell(row=r, column=code_gen_col))
ws_tp.add_data_validation(dv)

auto_width(ws_tp)

# --- MetaData sheet ---
ws_meta = wb.create_sheet("MetaData")
style_sheet(ws_meta, META_COLS)

for row_idx, mc in enumerate(META_JSON, 2):
    for col_idx, col_name in enumerate(META_COLS, 1):
        val = mc.get(col_name, "")
        cell = ws_meta.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = WRAP

auto_width(ws_meta)
ws_meta.sheet_state = "veryHidden"

# --- Save ---
wb.save(FILENAME)
print(f"Workbook saved: {FILENAME}")
print(f"TestPlan rows: {len(TESTPLAN_JSON)}")
print(f"MetaData rows: {len(META_JSON)}")
print(f"File size: {os.path.getsize(FILENAME)} bytes")

# --- Upload to GitHub ---
GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")
if GITHUB_TOKEN:
    with open(FILENAME, "rb") as f:
        content_b64 = base64.b64encode(f.read()).decode()
    url = f"https://api.github.com/repos/titusbspgit/PSVValidation/contents/Test_Output/MIPI/TestPlan/{FILENAME}"
    headers = {"Authorization": f"token {GITHUB_TOKEN}", "Accept": "application/vnd.github.v3+json"}
    # Check if file exists
    r = requests.get(url, headers=headers)
    sha = r.json().get("sha") if r.status_code == 200 else None
    payload = {"message": "Generated TestPlan Excel Workbook", "content": content_b64, "branch": "main"}
    if sha:
        payload["sha"] = sha
    r = requests.put(url, headers=headers, json=payload)
    if r.status_code in (200, 201):
        data = r.json()
        print(f"GitHub URL: {data['content']['html_url']}")
        print(f"Commit SHA: {data['commit']['sha']}")
    else:
        print(f"Upload failed: {r.status_code} {r.text}")
else:
    print("GITHUB_TOKEN not set. Skipping upload.")
