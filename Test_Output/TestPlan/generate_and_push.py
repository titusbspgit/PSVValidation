#!/usr/bin/env python3
"""
LPDDR4 TestPlan XLSX Generator & GitHub Publisher
Generates the timestamped XLSX workbook and pushes it to the repository.
Run: python3 generate_and_push.py
Requires: pip install openpyxl PyGithub
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import base64, io, os, sys

# --- IST Timestamp ---
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"

# --- Workbook Creation ---
wb = openpyxl.Workbook()
ws = wb.active
ws.title = "TestPlan"

# --- Column Headers (13 user-facing columns) ---
columns = [
    "Index", "SS / Module", "Test Case Name", "Feature", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks"
]

# --- Row Data ---
row_data = [
    "1",
    "LPDDR4",
    "lpddr4_mem_half_data_bus_width_test",
    "Half Data Bus Width",
    "SG3200 (default), SG2667 (when SG2667 defined), SG2133 (when SG2133 defined)",
    "Half Data Bus Width (bus_width = 1)",
    "0x100",
    "0x108",
    "Verify LPDDR4 memory data integrity in half data bus width mode after training. "
    "The test configures the memory controller for half bus width operation, performs "
    "LPDDR4 training, waits for stabilization, then reads back data from both memory "
    "ports at specific offsets and verifies the data matches expected patterns. The test "
    "supports both APS_DRAM and MPS_DRAM configurations with multiple speed grades "
    "(3200, 2667, 2133).",
    "1. Perform GPV programming to set up the bus interconnect.\n"
    "2. Initialize error counter to 0.\n"
    "3. Configure port addresses and controller/PHY base addresses based on DRAM type (APS_DRAM or MPS_DRAM).\n"
    "4. Set bus width to Half Bus mode (bus_width = 1).\n"
    "5. Select speed grade: SG3200 (default), SG2667, or SG2133.\n"
    "6. Disable Data Bus Inversion (DBI = 0).\n"
    "7. Disable Data Mask (DM_EN = 0).\n"
    "8. Execute LPDDR4 memory training sequence.\n"
    "9. Confirm training completion.\n"
    "10. Wait 1000 units for stabilization.\n"
    "11. Conditionally read 64-bit data from Port0 at offsets 0x100 and 0x108 and verify against expected patterns (condition based on initiator/DRAM combination).\n"
    "12. Read 64-bit data from Port1 at offsets 0x100 and 0x108 and verify against expected patterns.\n"
    "13. Report test result based on accumulated error count.",
    "NA",
    "1. Port0 (conditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.\n"
    "2. Port1 (unconditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.\n"
    "3. The test passes when the error counter (err0) is 0, indicating all read-back data matched the expected patterns.\n"
    "4. The test fails when err0 is greater than 0, indicating one or more data mismatches were detected.\n"
    "5. Note: The error condition uses logical AND (&&), so both values at offsets 0x100 and 0x108 must mismatch simultaneously to trigger an error.",
    "All 7 macros (APS_DRAM, MPS_DRAM, SG2667, SG2133, A53_INITIATOR, AI_INITIATOR, DSP_INITIATOR) "
    "are compile-time configuration flags expected to be defined externally via compiler command-line "
    "options. Port0 read-back verification is conditionally compiled using an XOR-based preprocessor "
    "expression involving initiator and DRAM type combinations. Port1 read-back verification is always "
    "compiled. The variables ctl_base, phy_base, bus_width, SG, DBI, DM_EN are declared in included "
    "headers (lpddr4.h or test_common.h) and assigned within test_case(). The local variables index "
    "and offset are declared but their usage is not visible in the available source context."
]

# --- Styles ---
hdr_font = Font(name='Calibri', bold=True, size=11, color='FFFFFF')
hdr_fill = PatternFill(start_color='002060', end_color='002060', fill_type='solid')
hdr_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_align = Alignment(horizontal='left', vertical='top', wrap_text=True)
cell_font = Font(name='Calibri', size=10)
bdr = Border(
    left=Side(style='thin'), right=Side(style='thin'),
    top=Side(style='thin'), bottom=Side(style='thin')
)

# --- Populate Header Row ---
for i, col in enumerate(columns, 1):
    c = ws.cell(row=1, column=i, value=col)
    c.font = hdr_font
    c.fill = hdr_fill
    c.alignment = hdr_align
    c.border = bdr

# --- Populate Data Row ---
for i, val in enumerate(row_data, 1):
    c = ws.cell(row=2, column=i, value=val)
    c.font = cell_font
    c.alignment = cell_align
    c.border = bdr

# --- Column Widths ---
widths = [8, 15, 40, 22, 35, 30, 20, 20, 60, 65, 20, 60, 60]
for i, w in enumerate(widths, 1):
    ws.column_dimensions[get_column_letter(i)].width = w

# --- Layout ---
ws.freeze_panes = 'A2'
ws.auto_filter.ref = f"A1:{get_column_letter(len(columns))}2"
ws.row_dimensions[1].height = 30
ws.row_dimensions[2].height = 300

# --- Save Locally ---
out_dir = os.path.dirname(os.path.abspath(__file__))
out_path = os.path.join(out_dir, filename)
wb.save(out_path)
sz = os.path.getsize(out_path)

# --- Verify ---
wb2 = openpyxl.load_workbook(out_path)
ws2 = wb2.active
print(f"FILENAME: {filename}")
print(f"PATH: {out_path}")
print(f"SIZE: {sz} bytes")
print(f"VERIFIED: sheet={ws2.title}, rows={ws2.max_row}, cols={ws2.max_column}")

# --- Base64 for GitHub push ---
with open(out_path, "rb") as f:
    b64 = base64.b64encode(f.read()).decode('ascii')
print(f"B64LEN: {len(b64)}")

# --- Optional: Push to GitHub via PyGithub ---
try:
    from github import Github
    token = os.environ.get("GITHUB_TOKEN")
    if token:
        g = Github(token)
        repo = g.get_repo("titusbspgit/PSVValidation")
        target_path = f"Test_Output/TestPlan/{filename}"
        with open(out_path, "rb") as f:
            content = f.read()
        repo.create_file(
            path=target_path,
            message=f"Add LPDDR4 TestPlan workbook {filename}",
            content=content,
            branch="main"
        )
        print(f"PUSHED: {target_path} to titusbspgit/PSVValidation@main")
    else:
        print("GITHUB_TOKEN not set - skipping push. Base64 written to stdout.")
except ImportError:
    print("PyGithub not installed - skipping push. Run: pip install PyGithub")

print("GENERATION_COMPLETE")
