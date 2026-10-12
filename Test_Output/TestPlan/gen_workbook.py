#!/usr/bin/env python3
"""
LPDDR4 Test Plan Excel Generator
Generates LPDDR4_TestPlan_YYYYMMDD_HHMMSS.xlsx
Run: python3 gen_workbook.py
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import os, sys

ist = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(ist)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp_str}.xlsx"
output_dir = os.path.dirname(os.path.abspath(__file__))
filepath = os.path.join(output_dir, filename)

# ── Test Case Data ──
test_data = {
    "Index": "1",
    "SS / Module": "LPDDR4",
    "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
    "Feature": "Half Data Bus Width",
    "Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "test_common.h"\n#include "lpddr4.h"',
    "Macros": "APS_DRAM, MPS_DRAM, SG2667, SG2133, A53_INITIATOR, AI_INITIATOR, DSP_INITIATOR - All are preprocessor conditional compilation flags; no #define value; externally defined.",
    "Arrays": "NA",
    "Speed": "SG3200 (default), SG2667 (when SG2667 defined), SG2133 (when SG2133 defined)",
    "Mode": "Half Data Bus Width (bus_width = 1)",
    "Memory Start Offset": "0x100",
    "Memory End Offset": "0x108",
    "Test Description": "Verify LPDDR4 memory data integrity in half data bus width mode after training. The test configures the memory controller for half bus width operation, performs LPDDR4 training, waits for stabilization, then reads back data from both memory ports at specific offsets and verifies the data matches expected patterns. The test supports both APS_DRAM and MPS_DRAM configurations with multiple speed grades (3200, 2667, 2133).",
    "Test Steps / Procedure": """1. Perform GPV programming to set up the bus interconnect.
2. Initialize error counter to 0.
3. Configure port addresses and controller/PHY base addresses based on DRAM type (APS_DRAM or MPS_DRAM).
4. Set bus width to Half Bus mode (bus_width = 1).
5. Select speed grade: SG3200 (default), SG2667, or SG2133.
6. Disable Data Bus Inversion (DBI = 0).
7. Disable Data Mask (DM_EN = 0).
8. Execute LPDDR4 memory training sequence.
9. Confirm training completion.
10. Wait 1000 units for stabilization.
11. Conditionally read 64-bit data from Port0 at offsets 0x100 and 0x108 and verify against expected patterns (condition based on initiator/DRAM combination).
12. Read 64-bit data from Port1 at offsets 0x100 and 0x108 and verify against expected patterns.
13. Report test result based on accumulated error count.""",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": """1. Port0 (conditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.
2. Port1 (unconditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.
3. The test passes when the error counter (err0) is 0, indicating all read-back data matched the expected patterns.
4. The test fails when err0 is greater than 0, indicating one or more data mismatches were detected.
5. Note: The error condition uses logical AND (&&), so both values at offsets 0x100 and 0x108 must mismatch simultaneously to trigger an error.""",
    "Remarks": "All 7 macros (APS_DRAM, MPS_DRAM, SG2667, SG2133, A53_INITIATOR, AI_INITIATOR, DSP_INITIATOR) are compile-time configuration flags expected to be defined externally via compiler command-line options. Port0 read-back verification is conditionally compiled using an XOR-based preprocessor expression involving initiator and DRAM type combinations. Port1 read-back verification is always compiled. The variables ctl_base, phy_base, bus_width, SG, DBI, DM_EN are declared in included headers (lpddr4.h or test_common.h) and assigned within test_case(). The local variables index and offset are declared but their usage is not visible in the available source context."
}

# ── Column definitions for the TestPlan sheet ──
columns = [
    ("Index", "Index"),
    ("SS / Module", "SS / Module"),
    ("Test Case Name", "Test Case Name"),
    ("Feature", "Feature"),
    ("Headers", "Headers"),
    ("Macros", "Macros"),
    ("Arrays", "Arrays"),
    ("Speed", "Speed"),
    ("Mode", "Mode"),
    ("Memory Start Offset", "Memory Start Offset"),
    ("Memory End Offset", "Memory End Offset"),
    ("Test Description", "Test Description"),
    ("Test Steps / Procedure", "Test Steps / Procedure"),
    ("Impacted Registers", "Impacted Registers"),
    ("Validation / Acceptance Criteria", "Validation / Acceptance Criteria"),
    ("Remarks", "Remarks"),
]

# ── Create Workbook ──
wb = openpyxl.Workbook()
ws = wb.active
ws.title = "TestPlan"

# ── Styles ──
header_font = Font(name="Calibri", bold=True, size=11, color="FFFFFF")
header_fill = PatternFill(start_color="002060", end_color="002060", fill_type="solid")
header_alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
cell_font = Font(name="Calibri", size=10)
cell_alignment = Alignment(horizontal="left", vertical="top", wrap_text=True)
thin_border = Border(
    left=Side(style="thin"),
    right=Side(style="thin"),
    top=Side(style="thin"),
    bottom=Side(style="thin"),
)

# ── Write Header Row ──
for col_idx, (col_key, col_header) in enumerate(columns, start=1):
    cell = ws.cell(row=1, column=col_idx, value=col_header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# ── Write Data Row ──
for col_idx, (col_key, _) in enumerate(columns, start=1):
    value = test_data.get(col_key, "")
    cell = ws.cell(row=2, column=col_idx, value=value)
    cell.font = cell_font
    cell.alignment = cell_alignment
    cell.border = thin_border

# ── Column Widths ──
col_widths = {
    "Index": 8,
    "SS / Module": 14,
    "Test Case Name": 40,
    "Feature": 22,
    "Headers": 35,
    "Macros": 45,
    "Arrays": 10,
    "Speed": 30,
    "Mode": 30,
    "Memory Start Offset": 20,
    "Memory End Offset": 20,
    "Test Description": 60,
    "Test Steps / Procedure": 65,
    "Impacted Registers": 18,
    "Validation / Acceptance Criteria": 65,
    "Remarks": 60,
}
for col_idx, (col_key, _) in enumerate(columns, start=1):
    ws.column_dimensions[get_column_letter(col_idx)].width = col_widths.get(col_key, 20)

# ── Row height for data row ──
ws.row_dimensions[1].height = 30
ws.row_dimensions[2].height = 300

# ── Freeze top row ──
ws.freeze_panes = "A2"

# ── Auto-filter ──
ws.auto_filter.ref = f"A1:{get_column_letter(len(columns))}2"

# ── Save ──
wb.save(filepath)
print(f"SUCCESS: {filepath}")
print(f"FILE: {filename}")
