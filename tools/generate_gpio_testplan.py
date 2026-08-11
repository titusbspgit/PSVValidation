import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
import subprocess

# Aggregated JSON rows (preserve exactly)
rows = [
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "NA",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Validate basic register access for the GPIO GP0 registers referenced by the test using read/write flows.",
    "Meta Test Description": "The test reads a soft-reset/control register (SOFT_RST_REG_ADDRESS) in program.c, then performs register accesses on multiple GPIO GP0 register macros (MIZAR_GPIO_GP0_GPIO_8 through MIZAR_GPIO_GP0_GPIO_27) referenced in test_define.c. Specific per-register operations are not detailed in the provided sources. Intent implied by the testcase name is to perform register write/read validation over the referenced GPIO GP0 registers.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1. Read the reset/control register used by the test and record the value.\n2. For each GPIO GP0 register referenced by the test (pins 8 through 27), exercise the register access flow used by the test (e.g., write then read back, as applicable).\n3. Record observed values for each access and note any anomalies.",
    "Meta Test Steps / Procedure": "1) program.c: read from SOFT_RST_REG_ADDRESS; capture the returned value. No compare or mask operations specified in the provided inputs.\n2) test_define.c: access the following GPIO GP0 register macros in sequence: MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27. Operation types for these macros are not specified in the inputs (marked as unknown).\n3) No explicit loops, conditions, bitwise operations, waits, interrupts, or assertions are provided in the inputs; treat accesses as straightforward register operations for the referenced macros.",
    "Impacted Registers": "NA",
    "Meta Impacted Registers": "SOFT_RST_REG_ADDRESS; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; MIZAR_GPIO_GP0_GPIO_11; MIZAR_GPIO_GP0_GPIO_12; MIZAR_GPIO_GP0_GPIO_13; MIZAR_GPIO_GP0_GPIO_14; MIZAR_GPIO_GP0_GPIO_15; MIZAR_GPIO_GP0_GPIO_16; MIZAR_GPIO_GP0_GPIO_17; MIZAR_GPIO_GP0_GPIO_18; MIZAR_GPIO_GP0_GPIO_19; MIZAR_GPIO_GP0_GPIO_20; MIZAR_GPIO_GP0_GPIO_21; MIZAR_GPIO_GP0_GPIO_22; MIZAR_GPIO_GP0_GPIO_23; MIZAR_GPIO_GP0_GPIO_24; MIZAR_GPIO_GP0_GPIO_25; MIZAR_GPIO_GP0_GPIO_26; MIZAR_GPIO_GP0_GPIO_27",
    "Validation / Acceptance Criteria": "NA",
    "Meta Validation / Acceptance Criteria": "NA",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "NA",
    "Meta Macros": "#define MIZAR_GPIO_BASE 0xA001A000; #define MIZAR_GPIO_GP0_GPIO_8 MIZAR_GPIO_BASE + GPIO_GP0_GPIO_8_OFFSET; #define GPIO_GP0_GPIO_8_OFFSET 0x0; #define GPIO_GP0_GPIO_8_DEFAULT_VAL 0x00100000; #define GPIO_GP0_GPIO_8_VALID_MASK 0x003F0003; #define GPIO_GP0_GPIO_8_WRITE_MASK 0x003F0000",
    "Meta Arrays": "NA"
  }
]

IP_NAME = "GPIO"
OUTPUT_DIR = os.path.join("Test_Output", IP_NAME, "TestPlan")
BRANCH = "main"
COMMIT_MESSAGE = "Auto-generated TestPlan for GPIO"

# Compute IST timestamp
ist = ZoneInfo("Asia/Kolkata")
now_ist = datetime.now(ist)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"{IP_NAME}_TestPlan_{timestamp}.xlsx"
output_path = os.path.join(OUTPUT_DIR, filename)

# Ensure directory exists
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Create workbook with exactly two sheets
wb = Workbook()
ws = wb.active
ws.title = "TestPlan"
ws_meta = wb.create_sheet("MetaData")
ws_meta.sheet_state = "veryHidden"

# Determine headers from first row preserving order
headers = [
  "Index","SS / Module","Feature","Test Case Name","Test Description",
  "Meta Test Description","Speed","Mode","Memory Start Offset","Memory End Offset",
  "Remarks","Test Steps / Procedure","Meta Test Steps / Procedure","Impacted Registers",
  "Meta Impacted Registers","Validation / Acceptance Criteria","Meta Validation / Acceptance Criteria",
  "Code Generation (Required / Not)","Meta Headers","Meta Macros","Meta Arrays"
]

# Write header
ws.append(headers)

# Styles
header_font = Font(bold=True)
header_fill = PatternFill(start_color="FFD966", end_color="FFD966", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

for col_idx, header in enumerate(headers, start=1):
    cell = ws.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill

# Write data rows preserving order
for r in rows:
    ws.append([r.get(h, "") for h in headers])

# Apply wrap to all data cells and set reasonable column widths
for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=len(headers)):
    for cell in row:
        cell.alignment = wrap

# Column width heuristics
widths = {
  "Index": 6,
  "SS / Module": 14,
  "Feature": 10,
  "Test Case Name": 24,
  "Test Description": 60,
  "Meta Test Description": 80,
  "Speed": 8,
  "Mode": 8,
  "Memory Start Offset": 18,
  "Memory End Offset": 18,
  "Remarks": 16,
  "Test Steps / Procedure": 80,
  "Meta Test Steps / Procedure": 90,
  "Impacted Registers": 24,
  "Meta Impacted Registers": 60,
  "Validation / Acceptance Criteria": 30,
  "Meta Validation / Acceptance Criteria": 36,
  "Code Generation (Required / Not)": 28,
  "Meta Headers": 22,
  "Meta Macros": 70,
  "Meta Arrays": 16
}

for i, h in enumerate(headers, start=1):
    ws.column_dimensions[get_column_letter(i)].width = widths.get(h, 20)

# Freeze first row
ws.freeze_panes = "A2"

# Populate MetaData sheet
meta_items = [
    ("IP_NAME", IP_NAME),
    ("Repo", "titusbspgit/PSVValidation"),
    ("Branch", BRANCH),
    ("Output Directory", OUTPUT_DIR + "/"),
    ("Commit Message", COMMIT_MESSAGE),
    ("Generated At (IST)", now_ist.strftime("%Y-%m-%d %H:%M:%S %Z")),
    ("Filename", filename),
    ("Source JSON Rows Count", str(len(rows))),
]

ws_meta.append(["Key", "Value"])  # headers for metadata
ws_meta["A1"].font = header_font
ws_meta["A1"].fill = header_fill
ws_meta["B1"].font = header_font
ws_meta["B1"].fill = header_fill

for k, v in meta_items:
    ws_meta.append([k, v])

# Save workbook
wb.save(output_path)

# Git commit and push
subprocess.run(["git", "config", "user.name", "github-actions[bot]"], check=True)
subprocess.run(["git", "config", "user.email", "github-actions[bot]@users.noreply.github.com"], check=True)
subprocess.run(["git", "add", output_path], check=True)
# Only commit if there are changes
rc = subprocess.run(["git", "diff", "--cached", "--quiet"]).returncode
if rc != 0:
    subprocess.run(["git", "commit", "-m", COMMIT_MESSAGE], check=True)
    subprocess.run(["git", "push", "origin", BRANCH], check=True)
else:
    print("No changes to commit.")

# retrigger
