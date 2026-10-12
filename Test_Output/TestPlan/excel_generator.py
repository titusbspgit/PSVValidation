import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import json
import os

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"

# Input data
data = [
  {
    "Index": "1",
    "SS / Module": "LPDDR4",
    "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
    "Feature": "Half Data Bus Width",
    "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "test_common.h"\n#include "lpddr4.h"',
    "Meta Macros": "APS_DRAM - Preprocessor conditional compilation flag for APS DRAM configuration (used in #if defined(APS_DRAM)); no #define value; externally defined.\nMPS_DRAM - Preprocessor conditional compilation flag for MPS DRAM configuration (used in #else path of APS_DRAM); no #define value; externally defined.\nSG2667 - Preprocessor conditional compilation flag for Speed Grade 2667 selection (used in #if defined(SG2667)); no #define value; externally defined.\nSG2133 - Preprocessor conditional compilation flag for Speed Grade 2133 selection (used in #elif defined(SG2133)); no #define value; externally defined.\nA53_INITIATOR - Preprocessor conditional compilation flag for A53 initiator selection (used in #if defined(A53_INITIATOR)); no #define value; externally defined.\nAI_INITIATOR - Preprocessor conditional compilation flag for AI initiator selection (used in #if defined(AI_INITIATOR)); no #define value; externally <NRP> conditional compilation flag for DSP initiator selection (used in #if defined(DSP_INITIATOR)); no #define value; externally defined.",
    "Meta Arrays": "NA",
    "Speed": "SG3200 (default), SG2667 (when SG2667 defined), SG2133 (when SG2133 defined)",
    "Mode": "Half Data Bus Width (bus_width = 1)",
    "Memory Start Offset": "0x100",
    "Memory End Offset": "0x108",
    "Meta Test Description": "This testcase verifies LPDDR4 memory data integrity when operating in half data bus width mode (bus_width = 1). The test configures the LPDDR4 controller and PHY base addresses based on the DRAM type (APS_DRAM or MPS_DRAM), sets bus_width to 1 (Half Bus), selects the speed grade (SG3200 default, SG2667, or SG2133 via preprocessor defines), disables Data Bus Inversion (<NRP> = 0) and <PERSON> (DM_EN = 0), performs LPDDR4 memory training via <PERSON>(), confirms training completion via training_done(), waits 1000 units for stabilization via wait_on(1000), then performs 64-bit read-back verification on <PERSON> (conditionally compiled based on XOR of initiator/DRAM combinations) and <PERSON> (unconditionally) at offsets 0x100 and 0x108. Read data is compared against expected patterns 0x3333333333333333 and 0x2222222222222222. Errors are accumulated in err0 and the test concludes with finish(err0).",
    "Test Description": "Verify LPDDR4 memory data integrity in half data bus width mode after training. The test configures the memory controller for half bus width operation, performs LPDDR4 training, waits for stabilization, then reads back data from both memory ports at specific offsets and verifies the data matches expected patterns. The test supports both APS_DRAM and MPS_DRAM configurations with multiple speed grades (3200, 2667, 2133).",
    "Meta Test Steps / Procedure": """1. File-scope declarations:
   unsigned long int port0_addr;
   unsigned long int port1_addr;
 
2. Function: int test_case()
 
3. Local variable declarations:
   unsigned long int index;
   unsigned long int offset;
   unsigned long int read_data0;
   unsigned long int read_data1;
   unsigned long int read_data2;
   unsigned long int read_data3;
   unsigned int err0;
 
4. Call gpv_programming();
 
5. err0 = 0;
 
6. Address configuration (preprocessor conditional):
   #if defined(APS_DRAM)
     port0_addr = 0;
     port1_addr = 0x15A0000000;
     ctl_base = 0x9EE03000;
     phy_base = 0x9F000000;
   #else
     port0_addr = 0;
     port1_addr = 0x11A0000000;
     ctl_base = 0x11D003000;
     phy_base = 0x11D500000;
   #endif
 
7. bus_width = 1; //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus
 
8. Speed grade selection (preprocessor conditional):
   #if defined(SG2667)
     SG = 2667;
   #elif defined(SG2133)
     SG = 2133;
   #else
     SG = 3200;
   #endif
 
9. <NRP> = 0;
 
10. DM_EN = 0;
 
11. Call <PERSON>();
 
12. Call training_done();
 
13. Call wait_on(1000);
 
14. Port0 read-back verification (preprocessor conditional):
   #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))
     read_reg64(port0_addr + 0x100, &read_data0);
     read_reg64(port0_addr + 0x108, &read_data1);
     if((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222))
     {
       printf("ERROR: port0 read_data0 = %lx, read_data1 = %lx\\n", read_data0, read_data1);
       err0++;
     }
   #endif
 
15. Port1 read-back verification (unconditional):
   read_reg64(port1_addr + 0x100, &read_data2);
   read_reg64(port1_addr + 0x108, &read_data3);
   if((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222))
   {
     printf("ERROR: port1 read_data2 = %lx, read_data3 = %lx\\n", read_data2, read_data3);
     err0++;
   }
 
16. Call finish(err0);""",
    "Test Steps / Procedure": """1. Perform GPV programming to set up the bus interconnect.
2. Initialize error counter to 0.
3. Configure port addresses and controller/PHY base addresses based on DRAM type (APS_DRAM or MPS_DRAM).
4. Set bus width to Half Bus mode (bus_width = 1).
5. Select speed grade: SG3200 (default), SG2667, or SG2133.
6. Disable Data Bus Inversion (<NRP> = 0).
7. Disable Data Mask (DM_EN = 0).
8. Execute LPDDR4 memory training sequence.
9. Confirm training completion.
10. Wait 1000 units for stabilization.
11. Conditionally read 64-bit data from <LOCATION> at offsets 0x100 and 0x108 and verify against expected patterns (condition based on initiator/DRAM combination).
12. Read 64-bit data from <LOCATION> at offsets 0x100 and 0x108 and verify against expected <LOCATION>. Report test result based on accumulated error count.""",
    "Meta Impacted Registers": "NA",
    "Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": """Port0 validation (conditionally compiled based on: #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))):
  read_reg64(port0_addr + 0x100, &read_data0): expected read_data0 == 0x3333333333333333
  read_reg64(port0_addr + 0x108, &read_data1): expected read_data1 == 0x2222222222222222
  Fail condition: if((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222)) then err0++ and printf error message
 
Port1 validation (unconditional):
  read_reg64(port1_addr + 0x100, &read_data2): expected read_data2 == <NRP>  read_reg64(port1_addr + 0x108, &read_data3): expected read_data3 == 0x2222222222222222
  Fail condition: if((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222)) then err0++ and printf error message
 
Pass condition: err0 == 0 (no mismatches detected)
Fail condition: err0 > 0 (one or more read-back mismatches)
Final call: finish(err0)
 
Note: The comparison uses && (logical AND), meaning both values at offset 0x100 and 0x108 must mismatch simultaneously to trigger an error increment.""",
    "Validation / Acceptance Criteria": """1. <LOCATION> (conditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.
2. <LOCATION> (unconditional): Data read from offset 0x100 must equal 0x3333333333333333 and data read from offset 0x108 must equal 0x2222222222222222.
3. The test passes when the error counter (err0) is 0, indicating all read-back data matched the expected patterns.
4. The test fails when err0 is greater than 0, indicating one or more data mismatches were detected.
5. Note: The error condition uses logical AND (&&), so both values at offsets 0x100 and 0x108 must mismatch simultaneously to trigger an error.""",
    "Remarks": "All 7 macros (APS_DRAM, MPS_DRAM, SG2667, SG2133, A53_INITIATOR, AI_INITIATOR, DSP_INITIATOR) are compile-time configuration flags expected to be defined externally via compiler command-line options. Port0 read-back verification is conditionally compiled using an XOR-based preprocessor expression involving initiator and DRAM type combinations. Port1 read-back verification is always compiled. The variables ctl_base, phy_base, bus_width, SG, <NRP>, DM_EN are declared in included headers (lpddr4.h or test_common.h) and assigned within test_case(). The local variables index and offset are declared but their usage is not visible in the available source context."
  }
]

# Create workbook
wb = openpyxl.Workbook()

# ---- TestPlan Sheet ----
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# Write headers
for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)

# Write data rows
for row_idx, row_data in enumerate(data, 2):
    for col_idx, col_name in enumerate(tp_columns, 1):
        value = row_data.get(col_name, "")
        ws_tp.cell(row=row_idx, column=col_idx, value=value)

# ---- MetaData Sheet ----
ws_md = wb.create_sheet("MetaData")

md_columns = [
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria", "Meta Headers",
    "Meta Macros", "Meta Arrays"
]

# Write headers
for col_idx, col_name in enumerate(md_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)

# Write data rows
for row_idx, row_data in enumerate(data, 2):
    for col_idx, col_name in enumerate(md_columns, 1):
        value = row_data.get(col_name, "")
        ws_md.cell(row=row_idx, column=col_idx, value=value)

# ---- Formatting ----
header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

for ws in [ws_tp, ws_md]:
    # Format headers
    for col_idx in range(1, ws.max_column + 1):
        cell = ws.cell(row=1, column=col_idx)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="center")

    # Wrap text on all data cells
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, max_col=ws.max_column):
        for cell in row:
            cell.alignment = wrap_alignment

    # Auto-size columns
    for col_idx in range(1, ws.max_column + 1):
        max_length = 0
        col_letter = get_column_letter(col_idx)
        for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_length:
                            max_length = len(line)
        adjusted_width = min(max_length + 2, 60)
        if adjusted_width < 12:
            adjusted_width = 12
        ws.column_dimensions[col_letter].width = adjusted_width

    # Freeze first row
    ws.freeze_panes = "A2"

# MetaData sheet very hidden
ws_md.sheet_state = "veryHidden"

# Save
save_path = f"/tmp/{filename}"
wb.save(save_path)

# Verify
file_size = os.path.getsize(save_path)
wb2 = openpyxl.load_workbook(save_path)
sheets = wb2.sheetnames

print(f"FILENAME={filename}")
print(f"SAVE_PATH={save_path}")
print(f"FILE_SIZE={file_size}")
print(f"SHEETS={sheets}")
print(f"TESTPLAN_ROWS={ws_tp.max_row - 1}")
print(f"METADATA_ROWS={ws_md.max_row - 1}")
print("VALIDATION=PASSED")
