#!/usr/bin/env python3
"""LPDDR4 TestPlan XLSX Generator - writes workbook to /tmp/"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter
import os, sys

wb = openpyxl.Workbook()
ws = wb.active
ws.title = "TestPlan"

columns = [
    "Index", "SS / Module", "Test Case Name", "Feature", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks"
]

row_data = [
    "1",
    "LPDDR4",
    "lpddr4_mem_half_data_bus_width_test",
    "Half Data Bus Width",
    "SG3200 (default), SG2667 (when SG2667 defined), SG2133 (when SG2133 defined)",
    "Half Data Bus Width (bus_width = 1)",
    "0x100",
    "0x108",
    "Verify LPDDR4 memory data integrity in half data bus width mode after training. The test configures the memory controller for half bus width operation, performs LPDDR4 training, waits for stabilization, then reads back data from both memory ports at specific offsets and verifies the data matches expected patterns. The test supports both APS_DRAM and MPS_DRAM configurations with multiple speed grades (3200, 2667, 2133).",
    "1. Perform GPV programming to set up the bus interconnect.\n2. Initialize error counter to 0.\n3. Configure port addresses and controller/PHY base addresses based on DRAM type (APS_DRAM or MPS_DRAM).\n4. Set bus width to Half Bus mode (bus_width = 1).\n5. Select speed grade: SG3200 (default), SG2667, or SG2133.\n6. Disable Data Bus Inversion (DBI = 0).\n7. Disable Data Mask (DM_EN = 0).\n8. Execute LPDDR4 memory training sequence.\n9. Confirm training completion.\n10. Wait 1000 units for stabilization.\n11. Conditionally read 64-bit data from Port0 at offsets 0x100 and 0x108 and verify against expected patterns (condition based on initiator/DRAM combination).\n12. Read 64-bit data from Port1 at offsets 0x100 and 0x108 and verify against expected patterns.\n13. Report test result based on accumulated error count.",
    "NA",
    "1. Port0 (conditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.\n2. Port1 (unconditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.\n3. The test passes when the error counter (err0) is 0, indicating all read-back data matched the expected patterns.\n4. The test fails when err0 is greater than 0, indicating one or more data mismatches were detected.\n5. Note: The error condition uses logical AND (&&), so both values at offsets 0x100 and 0x108 must mismatch simultaneously to trigger an error.",
    "All 7 macros (APS_DRAM, MPS_DRAM, SG2667, SG2133, A53_INITIATOR, AI_INITIATOR, DSP_INITIATOR) are compile-time configuration flags expected to be defined externally via compiler command-line options. Port0 read-back verification is conditionally compiled using an XOR-based preprocessor expression involving initiator and DRAM type combinations. Port1 read-back verification is always compiled. The variables ctl_base, phy_base, bus_width, SG, DBI, DM_EN are declared in included headers (lpddr4.h or test_common.h) and assigned within test_case(). The local variables index and offset are declared but their usage is not visible in the available source context."
]

hdr_font = Font(name='Calibri', bold=True, size=11, color='FFFFFF')
hdr_fill = PatternFill(start_color='002060', end_color='002060', fill_type='solid')
hdr_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_align = Alignment(horizontal='left', vertical='top', wrap_text=True)
cell_font = Font(name='Calibri', size=10)
bdr = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))

for i, col in enumerate(columns, 1):
    c = ws.cell(row=1, column=i, value=col)
    c.font = hdr_font; c.fill = hdr_fill; c.alignment = hdr_align; c.border = bdr

for i, val in enumerate(row_data, 1):
    c = ws.cell(row=2, column=i, value=val)
    c.font = cell_font; c.alignment = cell_align; c.border = bdr

widths = [8, 15, 40, 22, 35, 30, 20, 20, 60, 65, 20, 60, 60]
for i, w in enumerate(widths, 1):
    ws.column_dimensions[get_column_letter(i)].width = w

ws.freeze_panes = 'A2'
ws.row_dimensions[1].height = 30
ws.row_dimensions[2].height = 300

out = "/tmp/LPDDR4_TestPlan.xlsx"
wb.save(out)
sz = os.path.getsize(out)
print(f"OK size={sz}")
wb2 = openpyxl.load_workbook(out)
print(f"Verified sheet={wb2.active.title} rows={wb2.active.max_row} cols={wb2.active.max_column}")
