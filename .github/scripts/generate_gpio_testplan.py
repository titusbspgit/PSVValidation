import os, json, base64, requests
from io import BytesIO
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment

# Aggregated JSON (preserve order and data exactly)
aggregated_json = [
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "NA",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Sequentially read the listed GPIO GP0 registers and capture the returned values.",
    "Meta Test Description": "The test performs ordered register reads from the GPIO GP0 register macros MIZAR_GPIO_GP0_GPIO_8 through MIZAR_GPIO_GP0_GPIO_27. Each macro is read once, values are captured/logged, and no write operations or explicit assertions are performed. UI-spec mapping for these macros is unresolved (register_name: NA).",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1) Read each GPIO GP0 register listed for this test in order. 2) Record the read values for analysis. 3) Complete the sequence without modifying any register state.",
    "Meta Test Steps / Procedure": "1) Read MIZAR_GPIO_GP0_GPIO_8 and log the value. 2) Read MIZAR_GPIO_GP0_GPIO_9 and log the value. 3) Read MIZAR_GPIO_GP0_GPIO_10 and log the value. 4) Read MIZAR_GPIO_GP0_GPIO_11 and log the value. 5) Read MIZAR_GPIO_GP0_GPIO_12 and log the value. 6) Read MIZAR_GPIO_GP0_GPIO_13 and log the value. 7) Read MIZAR_GPIO_GP0_GPIO_14 and log the value. 8) Read MIZAR_GPIO_GP0_GPIO_15 and log the value. 9) Read MIZAR_GPIO_GP0_GPIO_16 and log the value. 10) Read MIZAR_GPIO_GP0_GPIO_17 and log the value. 11) Read MIZAR_GPIO_GP0_GPIO_18 and log the value. 12) Read MIZAR_GPIO_GP0_GPIO_19 and log the value. 13) Read MIZAR_GPIO_GP0_GPIO_20 and log the value. 14) Read MIZAR_GPIO_GP0_GPIO_21 and log the value. 15) Read MIZAR_GPIO_GP0_GPIO_22 and log the value. 16) Read MIZAR_GPIO_GP0_GPIO_23 and log the value. 17) Read MIZAR_GPIO_GP0_GPIO_24 and log the value. 18) Read MIZAR_GPIO_GP0_GPIO_25 and log the value. 19) Read MIZAR_GPIO_GP0_GPIO_26 and log the value. 20) Read MIZAR_GPIO_GP0_GPIO_27 and log the value.",
    "Impacted Registers": "NA",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; MIZAR_GPIO_GP0_GPIO_11; MIZAR_GPIO_GP0_GPIO_12; MIZAR_GPIO_GP0_GPIO_13; MIZAR_GPIO_GP0_GPIO_14; MIZAR_GPIO_GP0_GPIO_15; MIZAR_GPIO_GP0_GPIO_16; MIZAR_GPIO_GP0_GPIO_17; MIZAR_GPIO_GP0_GPIO_18; MIZAR_GPIO_GP0_GPIO_19; MIZAR_GPIO_GP0_GPIO_20; MIZAR_GPIO_GP0_GPIO_21; MIZAR_GPIO_GP0_GPIO_22; MIZAR_GPIO_GP0_GPIO_23; MIZAR_GPIO_GP0_GPIO_24; MIZAR_GPIO_GP0_GPIO_25; MIZAR_GPIO_GP0_GPIO_26; MIZAR_GPIO_GP0_GPIO_27",
    "Validation / Acceptance Criteria": "NA",
    "Meta Validation / Acceptance Criteria": "NA",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "NA",
    "Meta Macros": "#define GPIO0 1",
    "Meta Arrays": "NA"
  }
]

owner = os.environ.get('OWNER')
repo = os.environ.get('REPO')
branch = os.environ.get('BRANCH')
output_dir = os.environ.get('OUTPUT_DIR')
ip_name = os.environ.get('IP_NAME')

# Required columns
testplan_cols = [
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
metadata_cols = [
  "Index",
  "Meta Test Description",
  "Meta Test Steps / Procedure",
  "Meta Impacted Registers",
  "Meta Validation / Acceptance Criteria",
  "Meta Headers",
  "Meta Macros",
  "Meta Arrays",
]

# Build workbook
wb = Workbook()
ws = wb.active
ws.title = 'TestPlan'
ws_md = wb.create_sheet('MetaData')
ws_md.sheet_state = 'veryHidden'

# Styles
header_font = Font(bold=True)
header_fill = PatternFill(start_color='FFCCE5FF', end_color='FFCCE5FF', fill_type='solid')
wrap = Alignment(wrapText=True, vertical='top')

# Headers
ws.append(testplan_cols)
ws_md.append(metadata_cols)
for cell in ws[1]:
  cell.font = header_font
  cell.fill = header_fill
  cell.alignment = wrap
for cell in ws_md[1]:
  cell.font = header_font
  cell.fill = header_fill
  cell.alignment = wrap

# Data rows (preserve order and exact data)
for row in aggregated_json:
  ws.append([row.get(col, "") for col in testplan_cols])
  ws_md.append([row.get(col, "") for col in metadata_cols])

# Formatting
ws.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

col_widths_tp = {
  'A': 6,
  'B': 14,
  'C': 10,
  'D': 24,
  'E': 60,
  'F': 10,
  'G': 10,
  'H': 18,
  'I': 18,
  'J': 12,
  'K': 70,
  'L': 40,
  'M': 34,
  'N': 24,
}
for col, width in col_widths_tp.items():
  ws.column_dimensions[col].width = width
for row in ws.iter_rows(min_row=2):
  for cell in row:
    cell.alignment = wrap

col_widths_md = {
  'A': 6,
  'B': 70,
  'C': 90,
  'D': 70,
  'E': 34,
  'F': 20,
  'G': 28,
  'H': 20,
}
for col, width in col_widths_md.items():
  ws_md.column_dimensions[col].width = width
for row in ws_md.iter_rows(min_row=2):
  for cell in row:
    cell.alignment = wrap

# IST timestamp
ist = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(ist)
date_str = now_ist.strftime('%Y%m%d')
time_str = now_ist.strftime('%H%M%S')
filename = f"{ip_name}_TestPlan_{date_str}_{time_str}.xlsx"
rel_path = f"{output_dir}{filename}".lstrip('/')

# Save to bytes
bio = BytesIO()
wb.save(bio)
content_b64 = base64.b64encode(bio.getvalue()).decode('ascii')

# GitHub Contents API push
api_base = f"https://api.github.com/repos/{owner}/{repo}/contents/"
url = api_base + rel_path
headers = {
  'Authorization': f"Bearer {os.environ['GITHUB_TOKEN']}",
  'Accept': 'application/vnd.github+json'
}

# Get existing sha if any
sha = None
r = requests.get(url, params={'ref': branch}, headers=headers)
if r.status_code == 200:
  sha = r.json().get('sha')

commit_message = f"Add {ip_name} TestPlan generated on {now_ist.strftime('%Y-%m-%d %H:%M:%S IST')}"
payload = {'message': commit_message, 'content': content_b64, 'branch': branch}
if sha:
  payload['sha'] = sha

pr = requests.put(url, headers=headers, data=json.dumps(payload))
pr.raise_for_status()
out = pr.json()
commit_url = out.get('commit', {}).get('html_url') or out.get('commit', {}).get('url')
print(json.dumps({'output_file_path': rel_path, 'commit_url': commit_url, 'resolved_filename': filename}))
