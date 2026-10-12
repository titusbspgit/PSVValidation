#!/usr/bin/env python3
"""
LPDDR4 TestPlan XLSX Builder
Generates a real openpyxl-based Excel workbook with all test plan data.
Run: python3 build_xlsx.py
Output: LPDDR4_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx in the same directory.
"""
import sys
import os
from datetime import datetime, timezone, timedelta

try:
    import openpyxl
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    print("ERROR: openpyxl required. Install: pip install openpyxl")
    sys.exit(1)

# IST Timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"
script_dir = os.path.dirname(os.path.abspath(__file__))
filepath = os.path.join(script_dir, filename)

# ============================================================
# COMPLETE JSON DATA - EXACT PRESERVATION
# ============================================================

row_data = {
    "Index": "1",
    "SS / Module": "LPDDR4",
    "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
    "Feature": "Half Data Bus Width",
    "Speed": "SG3200 (default), SG2667 (when SG2667 defined), SG2133 (when SG2133 defined)",
    "Mode": "Half Data Bus Width (bus_width = 1)",
    "Memory Start Offset": "0x100",
    "Memory End Offset": "0x108",
    "Test Description": "Verify LPDDR4 memory data integrity in half data bus width mode after training. The test configures the memory controller for half bus width operation, performs LPDDR4 training, waits for stabilization, then reads back data from both memory ports at specific offsets and verifies the data matches expected patterns. The test supports both APS_DRAM and MPS_DRAM configurations with multiple speed grades (3200, 2667, 2133).",
    "Remarks": "All 7 macros (APS_DRAM, MPS_DRAM, SG2667, SG2133, A53_INITIATOR, AI_INITIATOR, DSP_INITIATOR) are compile-time configuration flags expected to be defined externally via compiler command-line options. Port0 read-back verification is conditionally compiled using an XOR-based preprocessor expression involving initiator and DRAM type combinations. Port1 read-back verification is always compiled. The variables ctl_base, phy_base, bus_width, SG, <NRP>, DM_EN are declared in included headers (lpddr4.h or test_common.h) and assigned within test_case(). The local variables index and offset are declared but their usage is not visible in the available source context.",
    "Test Steps / Procedure": "1. Perform GPV programming to set up the bus interconnect.\n2. Initialize error counter to 0.\n3. Configure port addresses and controller/PHY base addresses based on DRAM type (APS_DRAM or MPS_DRAM).\n4. Set bus width to Half Bus mode (bus_width = 1).\n5. Select speed grade: SG3200 (default), SG2667, or SG2133.\n6. Disable Data Bus Inversion (<NRP> = 0).\n7. Disable Data Mask (DM_EN = 0).\n8. Execute LPDDR4 memory training sequence.\n9. Confirm training completion.\n10. Wait 1000 units for stabilization.\n11. Conditionally read 64-bit data from <LOCATION> at offsets 0x100 and 0x108 and verify against expected patterns (condition based on initiator/DRAM combination).\n12. Read 64-bit data from <LOCATION> at offsets 0x100 and 0x108 and verify against expected <LOCATION>. Report test result based on accumulated error count.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1. <LOCATION> (conditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.\n2. <LOCATION> (unconditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.\n3. The test passes when the error counter (err0) is 0, indicating all read-back data matched the expected patterns.\n4. The test fails when err0 is greater than 0, indicating one or more data mismatches were detected.\n5. Note: The error condition uses logical AND (&&), so both values at offsets 0x100 and 0x108 must mismatch simultaneously to trigger an error.",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"lpddr4.h\"",
    "Meta Macros": "APS_DRAM - Preprocessor conditional compilation flag for APS DRAM configuration (used in #if defined(APS_DRAM)); no #define value; externally defined.\nMPS_DRAM - Preprocessor conditional compilation flag for MPS DRAM configuration (used in #else path of APS_DRAM); no #define value; externally defined.\nSG2667 - Preprocessor conditional compilation flag for Speed Grade 2667 selection (used in #if defined(SG2667)); no #define value; externally defined.\nSG2133 - Preprocessor conditional compilation flag for Speed Grade 2133 selection (used in #elif defined(SG2133)); no #define value; externally defined.\nA53_INITIATOR - Preprocessor conditional compilation flag for A53 initiator selection (used in #if defined(A53_INITIATOR)); no #define value; externally defined.\nAI_INITIATOR - Preprocessor conditional compilation flag for AI initiator selection (used in #if defined(AI_INITIATOR)); no #define value; externally <NRP> conditional compilation flag for DSP initiator selection (used in #if defined(DSP_INITIATOR)); no #define value; externally defined.",
    "Meta Arrays": "NA",
    "Meta Test Description": "This testcase verifies LPDDR4 memory data integrity when operating in half data bus width mode (bus_width = 1). The test configures the LPDDR4 controller and PHY base addresses based on the DRAM type (APS_DRAM or MPS_DRAM), sets bus_width to 1 (Half Bus), selects the speed grade (SG3200 default, SG2667, or SG2133 via preprocessor defines), disables Data Bus Inversion (<NRP> = 0) and <PERSON> (DM_EN = 0), performs LPDDR4 memory training via <PERSON>(), confirms training completion via training_done(), waits 1000 units for stabilization via wait_on(1000), then performs 64-bit read-back verification on <PERSON> (conditionally compiled based on XOR of initiator/DRAM combinations) and <PERSON> (unconditionally) at offsets 0x100 and 0x108. Read data is compared against expected patterns 0x3333333333333333 and 0x2222222222222222. Errors are accumulated in err0 and the test concludes with finish(err0).",
    "Meta Test Steps / Procedure": "1. File-scope declarations:\n   unsigned long int port0_addr;\n   unsigned long int port1_addr;\n\n2. Function: int test_case()\n\n3. Local variable declarations:\n   unsigned long int index;\n   unsigned long int offset;\n   unsigned long int read_data0;\n   unsigned long int read_data1;\n   unsigned long int read_data2;\n   unsigned long int read_data3;\n   unsigned int err0;\n\n4. Call gpv_programming();\n\n5. err0 = 0;\n\n6. Address configuration (preprocessor conditional):\n   #if defined(APS_DRAM)\n     port0_addr = 0;\n     port1_addr = 0x15A0000000;\n     ctl_base = 0x9EE03000;\n     phy_base = 0x9F000000;\n   #else\n     port0_addr = 0;\n     port1_addr = 0x11A0000000;\n     ctl_base = 0x11D003000;\n     phy_base = 0x11D500000;\n   #endif\n\n7. bus_width = 1; //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus\n\n8. Speed grade selection (preprocessor conditional):\n   #if defined(SG2667)\n     SG = 2667;\n   #elif defined(SG2133)\n     SG = 2133;\n   #else\n     SG = 3200;\n   #endif\n\n9. <NRP> = 0;\n\n10. DM_EN = 0;\n\n11. Call <PERSON>();\n\n12. Call training_done();\n\n13. Call wait_on(1000);\n\n14. Port0 read-back verification (preprocessor conditional):\n   #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))\n     read_reg64(port0_addr + 0x100, &read_data0);\n     read_reg64(port0_addr + 0x108, &read_data1);\n     if((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222))\n     {\n       printf(\"ERROR: port0 read_data0 = %lx, read_data1 = %lx\\n\", read_data0, read_data1);\n       err0++;\n     }\n   #endif\n\n15. Port1 read-back verification (unconditional):\n   read_reg64(port1_addr + 0x100, &read_data2);\n   read_reg64(port1_addr + 0x108, &read_data3);\n   if((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222))\n   {\n     printf(\"ERROR: port1 read_data2 = %lx, read_data3 = %lx\\n\", read_data2, read_data3);\n     err0++;\n   }\n\n16. Call finish(err0);",
    "Meta Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": "Port0 validation (conditionally compiled based on: #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))):\n  read_reg64(port0_addr + 0x100, &read_data0): expected read_data0 == 0x3333333333333333\n  read_reg64(port0_addr + 0x108, &read_data1): expected read_data1 == 0x2222222222222222\n  Fail condition: if((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222)) then err0++ and printf error message\n\nPort1 validation (unconditional):\n  read_reg64(port1_addr + 0x100, &read_data2): expected read_data2 == <NRP>  read_reg64(port1_addr + 0x108, &read_data3): expected read_data3 == 0x2222222222222222\n  Fail condition: if((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222)) then err0++ and printf error message\n\nPass condition: err0 == 0 (no mismatches detected)\nFail condition: err0 > 0 (one or more read-back mismatches)\nFinal call: finish(err0)\n\nNote: The comparison uses && (logical AND), meaning both values at offset 0x100 and 0x108 must mismatch simultaneously to trigger an error increment."
}

# ============================================================
# CREATE WORKBOOK
# ============================================================
wb = openpyxl.Workbook()

# Styles
blue_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
white_font = Font(color="FFFFFF", bold=True)
wrap_align = Alignment(wrap_text=True, vertical="top")

# ---- TESTPLAN SHEET ----
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_headers = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

for ci, h in enumerate(tp_headers, 1):
    c = ws_tp.cell(row=1, column=ci, value=h)
    c.fill = blue_fill
    c.font = white_font
    c.alignment = wrap_align

tp_values = [
    row_data["Index"],
    row_data["SS / Module"],
    row_data["Feature"],
    row_data["Test Case Name"],
    row_data["Test Description"],
    row_data["Speed"],
    row_data["Mode"],
    row_data["Memory Start Offset"],
    row_data["Memory End Offset"],
    row_data["Remarks"],
    row_data["Test Steps / Procedure"],
    row_data["Impacted Registers"],
    row_data["Validation / Acceptance Criteria"],
    ""  # Code Generation blank
]

for ci, v in enumerate(tp_values, 1):
    c = ws_tp.cell(row=2, column=ci, value=v)
    c.alignment = wrap_align

ws_tp.freeze_panes = "A2"

for ci in range(1, len(tp_headers) + 1):
    cl = openpyxl.utils.get_column_letter(ci)
    mx = 0
    for r in ws_tp.iter_rows(min_col=ci, max_col=ci, min_row=1, max_row=ws_tp.max_row):
        for c in r:
            if c.value:
                for ln in str(c.value).split('\n'):
                    mx = max(mx, len(ln))
    ws_tp.column_dimensions[cl].width = max(min(mx + 2, 55), 12)

# ---- METADATA SHEET ----
ws_md = wb.create_sheet("MetaData")

md_headers = [
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria", "Meta Headers",
    "Meta Macros", "Meta Arrays"
]

for ci, h in enumerate(md_headers, 1):
    c = ws_md.cell(row=1, column=ci, value=h)
    c.fill = blue_fill
    c.font = white_font
    c.alignment = wrap_align

md_values = [
    row_data["Index"],
    row_data["Test Case Name"],
    row_data["Meta Test Description"],
    row_data["Meta Test Steps / Procedure"],
    row_data["Meta Impacted Registers"],
    row_data["Meta Validation / Acceptance Criteria"],
    row_data["Meta Headers"],
    row_data["Meta Macros"],
    row_data["Meta Arrays"]
]

for ci, v in enumerate(md_values, 1):
    c = ws_md.cell(row=2, column=ci, value=v)
    c.alignment = wrap_align

ws_md.freeze_panes = "A2"

for ci in range(1, len(md_headers) + 1):
    cl = openpyxl.utils.get_column_letter(ci)
    mx = 0
    for r in ws_md.iter_rows(min_col=ci, max_col=ci, min_row=1, max_row=ws_md.max_row):
        for c in r:
            if c.value:
                for ln in str(c.value).split('\n'):
                    mx = max(mx, len(ln))
    ws_md.column_dimensions[cl].width = max(min(mx + 2, 55), 12)

ws_md.sheet_state = "veryHidden"

# ============================================================
# VALIDATION
# ============================================================
meta_fields_to_check = [
    "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

all_ok = True
for field in meta_fields_to_check:
    ci = md_headers.index(field) + 1
    cv = ws_md.cell(row=2, column=ci).value
    ev = row_data[field]
    if cv != ev:
        print(f"FAIL: {field}")
        all_ok = False

if not all_ok:
    print("META VALIDATION: FAILED")
    sys.exit(1)

print("META VALIDATION: PASSED")

# Save
wb.save(filepath)
print(f"SAVED: {filepath}")

# Reopen check
if not os.path.exists(filepath):
    print("FILE NOT FOUND")
    sys.exit(1)

fsize = os.path.getsize(filepath)
print(f"SIZE: {fsize}")

wb2 = openpyxl.load_workbook(filepath)
assert "TestPlan" in wb2.sheetnames
assert "MetaData" in wb2.sheetnames
print(f"REOPEN: OK")
print(f"TP_ROWS: {wb2['TestPlan'].max_row - 1}")
print(f"MD_ROWS: {wb2['MetaData'].max_row - 1}")
print(f"FILENAME: {filename}")
print("ALL_VALIDATIONS_PASSED")

# Base64 output for GitHub upload
import base64
with open(filepath, 'rb') as f:
    b64content = base64.b64encode(f.read()).decode('utf-8')

b64file = os.path.join(script_dir, 'xlsx_base64.txt')
with open(b64file, 'w') as f:
    f.write(b64content)
print(f"B64_FILE: {b64file}")
print(f"B64_LEN: {len(b64content)}")

wb2.close()
wb.close()
print("DONE")
