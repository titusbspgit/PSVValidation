#!/usr/bin/env python3
"""
Run this script locally or in CI to generate and verify the LPDDR4 TestPlan Excel file.
Usage: cd Test_Output/TestPlan && python3 build_and_commit.py
The generated .xlsx file will appear in the same directory.
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import os

ist = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(ist)
ts = now_ist.strftime("%Y%m%d_%H%M%S")
fname = f"LPDDR4_TestPlan_{ts}.xlsx"
fpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), fname)

data = {
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
    "Test Steps / Procedure": "1. Perform GPV programming to set up the bus interconnect.\n2. Initialize error counter to 0.\n3. Configure port addresses and controller/PHY base addresses based on DRAM type (APS_DRAM or MPS_DRAM).\n4. Set bus width to Half Bus mode (bus_width = 1).\n5. Select speed grade: SG3200 (default), SG2667, or SG2133.\n6. Disable Data Bus Inversion (DBI = 0).\n7. Disable Data Mask (DM_EN = 0).\n8. Execute LPDDR4 memory training sequence.\n9. Confirm training completion.\n10. Wait 1000 units for stabilization.\n11. Conditionally read 64-bit data from Port0 at offsets 0x100 and 0x108 and verify against expected patterns (condition based on initiator/DRAM combination).\n12. Read 64-bit data from Port1 at offsets 0x100 and 0x108 and verify against expected patterns.\n13. Report test result based on accumulated error count.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1. Port0 (conditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.\n2. Port1 (unconditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.\n3. The test passes when the error counter (err0) is 0, indicating all read-back data matched the expected patterns.\n4. The test fails when err0 is greater than 0, indicating one or more data mismatches were detected.\n5. Note: The error condition uses logical AND (&&), so both values at offsets 0x100 and 0x108 must mismatch simultaneously to trigger an error.",
    "Remarks": "All 7 macros (APS_DRAM, MPS_DRAM, SG2667, SG2133, A53_INITIATOR, AI_INITIATOR, DSP_INITIATOR) are compile-time configuration flags expected to be defined externally via compiler command-line options. Port0 read-back verification is conditionally compiled using an XOR-based preprocessor expression involving initiator and DRAM type combinations. Port1 read-back verification is always compiled."
}

cols = ["Index","SS / Module","Test Case Name","Feature","Headers","Macros","Arrays","Speed","Mode","Memory Start Offset","Memory End Offset","Test Description","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Remarks"]
widths = [8,14,40,22,35,45,10,30,30,20,20,60,65,18,65,60]

wb = openpyxl.Workbook()
ws = wb.active
ws.title = "TestPlan"
hf = Font(name="Calibri", bold=True, size=11, color="FFFFFF")
hfill = PatternFill(start_color="002060", end_color="002060", fill_type="solid")
ha = Alignment(horizontal="center", vertical="center", wrap_text=True)
cf = Font(name="Calibri", size=10)
ca = Alignment(horizontal="left", vertical="top", wrap_text=True)
bd = Border(left=Side("thin"), right=Side("thin"), top=Side("thin"), bottom=Side("thin"))

for i, c in enumerate(cols, 1):
    cell = ws.cell(row=1, column=i, value=c)
    cell.font, cell.fill, cell.alignment, cell.border = hf, hfill, ha, bd
for i, c in enumerate(cols, 1):
    cell = ws.cell(row=2, column=i, value=data.get(c, ""))
    cell.font, cell.alignment, cell.border = cf, ca, bd
for i, w in enumerate(widths, 1):
    ws.column_dimensions[get_column_letter(i)].width = w
ws.row_dimensions[1].height = 30
ws.row_dimensions[2].height = 300
ws.freeze_panes = "A2"
ws.auto_filter.ref = f"A1:{get_column_letter(len(cols))}2"

wb.save(fpath)
sz = os.path.getsize(fpath)
print(f"Generated: {fname} ({sz} bytes)")
