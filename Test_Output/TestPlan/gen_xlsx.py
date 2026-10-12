#!/usr/bin/env python3
"""Generate LPDDR4 TestPlan XLSX workbook."""
import sys
import os
import base64

try:
    import openpyxl
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
except ImportError:
    print("ERROR: openpyxl not available")
    sys.exit(1)

from datetime import datetime, timezone, timedelta

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"

# Test data
data = {
    "Index": "1",
    "SS / Module": "LPDDR4",
    "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
    "Feature": "Half Data Bus Width",
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

# Column order
columns = [
    "Index",
    "SS / Module",
    "Test Case Name",
    "Feature",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Test Description",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Remarks"
]

# Create workbook
wb = openpyxl.Workbook()
ws = wb.active
ws.title = "TestPlan"

# Styles
header_font = Font(name='Calibri', bold=True, size=11, color='FFFFFF')
header_fill = PatternFill(start_color='002060', end_color='002060', fill_type='solid')
header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_alignment = Alignment(horizontal='left', vertical='top', wrap_text=True)
thin_border = Border(
    left=Side(style='thin'),
    right=Side(style='thin'),
    top=Side(style='thin'),
    bottom=Side(style='thin')
)
cell_font = Font(name='Calibri', size=10)

# Write headers
for col_idx, col_name in enumerate(columns, 1):
    cell = ws.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write data row
for col_idx, col_name in enumerate(columns, 1):
    cell = ws.cell(row=2, column=col_idx, value=data[col_name])
    cell.font = cell_font
    cell.alignment = cell_alignment
    cell.border = thin_border

# Column widths
col_widths = {
    "Index": 8,
    "SS / Module": 15,
    "Test Case Name": 40,
    "Feature": 22,
    "Speed": 35,
    "Mode": 30,
    "Memory Start Offset": 20,
    "Memory End Offset": 20,
    "Test Description": 60,
    "Test Steps / Procedure": 65,
    "Impacted Registers": 20,
    "Validation / Acceptance Criteria": 60,
    "Remarks": 60
}

for col_idx, col_name in enumerate(columns, 1):
    col_letter = openpyxl.utils.get_column_letter(col_idx)
    ws.column_dimensions[col_letter].width = col_widths.get(col_name, 20)

# Freeze header row
ws.freeze_panes = 'A2'

# Row height for data row
ws.row_dimensions[2].height = 300

# Save
output_path = f"/tmp/{filename}"
wb.save(output_path)

# Verify
file_size = os.path.getsize(output_path)
print(f"File: {filename}")
print(f"Path: {output_path}")
print(f"Size: {file_size} bytes")

# Read and output as base64 for transfer
with open(output_path, 'rb') as f:
    content = f.read()
    b64 = base64.b64encode(content).decode('ascii')
    print(f"BASE64_START")
    print(b64)
    print(f"BASE64_END")

# Verify by reloading
wb2 = openpyxl.load_workbook(output_path)
ws2 = wb2.active
print(f"Verification: Sheet='{ws2.title}', Rows={ws2.max_row}, Cols={ws2.max_column}")
print(f"Header check: {ws2.cell(1,1).value} | {ws2.cell(1,2).value} | {ws2.cell(1,3).value}")
print(f"Data check: {ws2.cell(2,1).value} | {ws2.cell(2,2).value} | {ws2.cell(2,3).value}")
print("SUCCESS")
