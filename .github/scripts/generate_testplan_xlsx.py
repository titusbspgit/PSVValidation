import json
import os
import sys
from io import BytesIO
from urllib.request import urlopen, Request
from datetime import datetime
import pytz
from openpyxl import Workbook
from openpyxl.styles import Font

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

def fetch_json(json_url: str):
    req = Request(json_url, headers={"User-Agent": "github-actions-bot"})
    with urlopen(req) as resp:
        data = resp.read()
    text = data.decode("utf-8")
    obj = json.loads(text)
    if not isinstance(obj, list):
        raise ValueError("json_data must be a list (array) of objects")
    for i, row in enumerate(obj):
        if not isinstance(row, dict):
            raise ValueError(f"json_data row {i} is not an object")
    return obj


def build_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"
    ws2 = wb.create_sheet("MetaData")

    # Headers
    ws.append(TESTPLAN_COLUMNS)
    ws2.append(METADATA_COLUMNS)

    # Bold headers
    bold = Font(bold=True)
    for cell in ws[1]:
        cell.font = bold
    for cell in ws2[1]:
        cell.font = bold

    # Freeze first row
    ws.freeze_panes = "A2"
    ws2.freeze_panes = "A2"

    # Data rows (preserve order)
    def get(row, key):
        return row.get(key, "")

    for row in rows:
        ws.append([
            get(row, "Index"),
            get(row, "SS / Module"),
            get(row, "Feature"),
            get(row, "Test Case Name"),
            get(row, "Test Description"),
            get(row, "Speed"),
            get(row, "Mode"),
            get(row, "Memory Start Offset"),
            get(row, "Memory End Offset"),
            get(row, "Remarks"),
            get(row, "Test Steps / Procedure"),
            get(row, "Impacted Registers"),
            get(row, "Validation / Acceptance Criteria"),
            get(row, "Code Generation (Required / Not)"),
        ])
        ws2.append([
            get(row, "Index"),
            get(row, "Test Case Name"),
            get(row, "Meta Test Description"),
            get(row, "Meta Test Steps / Procedure"),
            get(row, "Meta Impacted Registers"),
            get(row, "Meta Validation / Acceptance Criteria"),
            get(row, "Meta Headers"),
            get(row, "Meta Macros"),
            get(row, "Meta Arrays"),
        ])

    # VeryHidden metadata sheet
    ws2.sheet_state = "veryHidden"

    return wb


def ensure_dir(path):
    if not os.path.isdir(path):
        os.makedirs(path, exist_ok=True)


def main():
    json_url = os.environ.get("JSON_URL")
    output_dir = os.environ.get("OUTPUT_DIR", "Test_Output")

    if not json_url:
        print("ERROR: JSON_URL not provided", file=sys.stderr)
        sys.exit(2)

    rows = fetch_json(json_url)
    wb = build_workbook(rows)

    # IST timestamp
    ist = pytz.timezone("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"

    ensure_dir(output_dir)
    out_path = os.path.join(output_dir, filename)
    wb.save(out_path)

    # Record the relative path for the workflow commit step
    meta_out = os.path.join(".github", "scripts", "excel_path.txt")
    ensure_dir(os.path.dirname(meta_out))
    with open(meta_out, "w", encoding="utf-8") as f:
        f.write(out_path.replace("\\", "/"))

    print(f"Generated: {out_path}")

if __name__ == "__main__":
    main()
