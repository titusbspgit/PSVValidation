import json
import os
from datetime import datetime, timezone, timedelta
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
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]

META_INFO_KEYS = [
    ("generation_timestamp_utc", None),
    ("source_owner", "SOURCE_OWNER"),
    ("source_repo", "SOURCE_REPO"),
    ("source_branch", "SOURCE_BRANCH"),
    ("source_subdir", "SOURCE_SUBDIR"),
    ("ip_name", "IP_NAME"),
    ("total_testcases", "TOTAL_TESTCASES"),
]

def now_utc_iso():
    return datetime.utcnow().replace(tzinfo=timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

def ist_timestamp():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(tz=ist).strftime("%Y%m%d_%H%M%S")

def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)

def ensure_dir(p):
    os.makedirs(p, exist_ok=True)

def build_workbook(data, meta_info):
    wb = Workbook()

    # Sheet 1: TestPlan
    ws1 = wb.active
    ws1.title = "TestPlan"
    # Header
    ws1.append(TESTPLAN_COLUMNS)
    for cell in ws1[1]:
        cell.font = Font(bold=True)
    ws1.freeze_panes = "A2"

    # Rows
    for row in data:
        values = [row.get(col, "") for col in TESTPLAN_COLUMNS]
        ws1.append(values)

    # Sheet 2: MetaData
    ws2 = wb.create_sheet("MetaData")

    # Top key-value meta info
    ws2.append(["Key", "Value"])
    ws2["A1"].font = Font(bold=True)
    ws2["B1"].font = Font(bold=True)

    # generation_timestamp_utc computed now
    ws2.append(["generation_timestamp_utc", meta_info.get("generation_timestamp_utc", now_utc_iso())])
    for key, env_key in META_INFO_KEYS[1:]:
        ws2.append([key, meta_info.get(key, os.getenv(env_key, ""))])

    # Blank row separator
    ws2.append([""])

    # Meta table header
    ws2.append(METADATA_COLUMNS)
    for cell in ws2[ws2.max_row]:
        cell.font = Font(bold=True)
    ws2.freeze_panes = f"A{ws2.max_row+1}"

    # Meta rows aligned with TestPlan order
    for row in data:
        values = [row.get(col, "") for col in METADATA_COLUMNS]
        ws2.append(values)

    # Very hide MetaData sheet
    ws2.sheet_state = 'veryHidden'

    return wb

def main():
    json_path = os.getenv('JSON_PATH', 'data/final_testplan.json')
    output_dir = os.getenv('OUTPUT_DIR', 'Test_Output')

    meta_info = {
        'generation_timestamp_utc': now_utc_iso(),
        'source_owner': os.getenv('SOURCE_OWNER', ''),
        'source_repo': os.getenv('SOURCE_REPO', ''),
        'source_branch': os.getenv('SOURCE_BRANCH', ''),
        'source_subdir': os.getenv('SOURCE_SUBDIR', ''),
        'ip_name': os.getenv('IP_NAME', ''),
        'total_testcases': os.getenv('TOTAL_TESTCASES', ''),
    }

    data = load_json(json_path)
    if not isinstance(data, list):
        raise ValueError('json_data must be an array of objects')

    # Build workbook
    wb = build_workbook(data, meta_info)

    # Determine filename with IST timestamp
    ts = ist_timestamp()
    filename = f"testplan_{ts}.xlsx"

    ensure_dir(output_dir)
    out_path = os.path.join(output_dir, filename)
    wb.save(out_path)

    print(f"Generated: {out_path}")

if __name__ == '__main__':
    main()
