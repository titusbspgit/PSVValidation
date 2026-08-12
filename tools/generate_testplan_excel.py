import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

OWNER = os.getenv("OWNER", "titusbspgit")
REPO = os.getenv("REPO", "PSVValidation")
BRANCH = os.getenv("BRANCH", "main")
OUTPUT_DIR = os.getenv("OUTPUT_DIR", "Test_Output/GPIO/TestPlan").rstrip("/")
IP_NAME = os.getenv("IP_NAME", "GPIO")
AGENT_NAME = "Ag_Excel_Generator Agent"

AGGREGATED_JSON_PATH = os.getenv("AGGREGATED_JSON_PATH", "tools/aggregated_testplan.json")

with open(AGGREGATED_JSON_PATH, "r", encoding="utf-8") as f:
    raw_json_text = f.read()

data = json.loads(raw_json_text)

wb = Workbook()
ws = wb.active
ws.title = "TestPlan"

# Preserve header order exactly as in JSON
headers = list(data[0].keys()) if data else []

# Header styling
header_font = Font(bold=True)
header_fill = PatternFill(fill_type="solid", start_color="FFD9E1F2", end_color="FFD9E1F2")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# Write headers
for col_idx, header in enumerate(headers, start=1):
    cell = ws.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write rows preserving order
for row_idx, item in enumerate(data, start=2):
    for col_idx, header in enumerate(headers, start=1):
        val = item.get(header, "")
        ws.cell(row=row_idx, column=col_idx, value=val).alignment = wrap_alignment

# Freeze header row
ws.freeze_panes = "A2"

# Auto column widths (with reasonable bounds)
def _cell_display_len(v):
    s = "" if v is None else str(v)
    return max((len(part) for part in s.splitlines()), default=0)

for col_idx, header in enumerate(headers, start=1):
    max_len = _cell_display_len(header)
    for row_idx in range(2, len(data) + 2):
        v = ws.cell(row=row_idx, column=col_idx).value
        max_len = max(max_len, _cell_display_len(v))
    width = max(12, min(60, max_len + 2))
    ws.column_dimensions[get_column_letter(col_idx)].width = width

# MetaData sheet
meta = wb.create_sheet("MetaData")
meta.sheet_state = "veryHidden"
meta_headers = ["Key", "Value"]
for col_idx, h in enumerate(meta_headers, start=1):
    c = meta.cell(row=1, column=col_idx, value=h)
    c.font = header_font
    c.fill = header_fill
    c.alignment = wrap_alignment

# IST timestamp
ist = ZoneInfo("Asia/Kolkata")
ts = datetime.now(ist)
# Filename per rule
date_str = ts.strftime("%Y%m%d")
time_str = ts.strftime("%H%M%S")
filename = f"{IP_NAME}_TestPlan_{date_str}_{time_str}.xlsx"

meta_rows = [
    ("owner", OWNER),
    ("repo", REPO),
    ("branch", BRANCH),
    ("output_directory", OUTPUT_DIR + "/"),
    ("ip_name", IP_NAME),
    ("generator", AGENT_NAME),
    ("timestamp_ist", ts.strftime("%Y-%m-%d %H:%M:%S IST")),
    ("filename", filename),
    ("aggregated_json", raw_json_text),
]

for r_idx, (k, v) in enumerate(meta_rows, start=2):
    meta.cell(row=r_idx, column=1, value=k).alignment = wrap_alignment
    meta.cell(row=r_idx, column=2, value=v).alignment = wrap_alignment

meta.column_dimensions["A"].width = 24
meta.column_dimensions["B"].width = 120
meta.freeze_panes = "A2"

# Ensure output directory exists
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Save workbook
out_path = os.path.join(OUTPUT_DIR, filename)
wb.save(out_path)
print(f"Saved: {out_path}")
