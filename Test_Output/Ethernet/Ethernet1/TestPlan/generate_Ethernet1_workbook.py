#!/usr/bin/env python3
"""
Ethernet1 TestPlan Workbook Generator
Generates a genuine .xlsx workbook with:
- TestPlan sheet (visible) with 14 columns including Code Generation dropdown
- MetaData sheet (veryHidden) with 9 columns
- Proper formatting: bold blue headers, white font, freeze panes, wrap text, auto-width
- Data validation dropdown on Code Generation column
- Post-save validation

Usage: pip install openpyxl && python generate_Ethernet1_workbook.py
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

# ============================================================
# CONFIGURATION
# ============================================================
IP_NAME = "Ethernet1"
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
FILENAME = f"{IP_NAME}_TestPlan_{now_ist.strftime('%Y%m%d_%H%M%S')}.xlsx"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# ============================================================
# TESTPLAN DATA (from Agent 6B)
# ============================================================
TESTPLAN_JSON = [
    {
        "Index": 1,
        "SS / Module": "Ethernet1",
        "Feature": "Register Write-Read Verification",
        "Test Case Name": "ethernet1_reg_wr_rd_test/program.c",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase performs register write-read verification for the Ethernet1 IP. It consists of two main phases: (1) Default Value Check \u2014 iterates over 434 Ethernet1 register addresses, reads each register and compares the read value against the expected default value, skipping registers that are not readable or marked in the skip list. (2) Write-Read Check \u2014 iterates over 6 predefined test patterns, writes each pattern to all writable registers, then reads back and compares against an expected value computed using the write mask, read mask, and default value for each register. Registers marked in the skip list or with zero write/read masks are skipped. A soft reset check function is defined but not invoked in the active test flow. The test reports failure if any default value mismatch or write-read mismatch is detected, otherwise reports pass.",
        "Test Steps / Procedure": "1. Initialize failure counters to zero.\n2. Execute Default Value Check phase:\n   a. Iterate over all 434 Ethernet1 registers.\n   b. For each register, check if it is readable (read mask is non-zero) and not in the reset skip list.\n   c. If skippable, skip to the next register.\n   d. Read the register value.\n   e. Compare the read value against the expected default value.\n   f. If mismatch, increment the default-value failure counter.\n3. Execute Write-Read Check phase:\n   a. For each of the 6 test patterns (all-ones, alternating-AA, alternating-55, all-zeros, alternating-A5, upper-half-ones):\n      i. Write the test pattern to all writable registers, skipping registers in the skip list or with zero write mask.\n      ii. Read back each register, skipping registers in the skip list or with zero write/read masks.\n      iii. Compute the expected read-back value using the write mask, read mask, and default value.\n      iv. Compare the read value against the expected value.\n      v. If mismatch, increment the write-read failure counter.\n4. Evaluate overall result:\n   a. If either failure counter is greater than zero, signal test failure.\n   b. Otherwise, signal test pass.",
        "Impacted Registers": "MAC_CONFIGURATION, MAC_EXT_CONFIGURATION, MAC_PACKET_FILTER, MAC_WD_JB_TIMEOUT, MAC_HASH_TABLE_REG0, MAC_HASH_TABLE_REG1, MAC_VLAN_TAG_CTRL, MAC_VLAN_TAG_DATA, MAC_VLAN_HASH_TABLE, MAC_Q0_TX_FLOW_CTRL",
        "Validation / Acceptance Criteria": "1. Default Value Check: For each readable and non-skipped register, the value read after reset must match the expected default value. Any mismatch increments the default-value failure counter.\n2. Write-Read Check: For each writable and readable register, after writing a test pattern, the read-back value must match the expected value computed from the write mask, read mask, and default value. Any mismatch increments the write-read failure counter.\n3. Overall Pass Criteria: The test passes (finish with success) only if both the default-value failure counter and the write-read failure counter remain at zero. If either counter is greater than zero, the test fails (finish with failure).",
        "Remarks": "The test covers 434 Ethernet1 registers using 6 distinct test patterns for write-read verification. Certain registers are excluded from write-read or default-value checks via skip arrays. A soft reset check function is defined in the source but is commented out and not executed in the active test flow. Only the first 10 register entries were retrievable from the knowledge base; the full set of 434 registers is defined in the test configuration file."
    },
    {
        "Index": 2,
        "SS / Module": "Ethernet1",
        "Feature": "Basic TX Transmit Operation",
        "Test Case Name": "ethernet1_tx_basic_test/program.c",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase performs a basic Ethernet1 TX (transmit) operation. It configures MAC address registers for 4 DMA channels with address enable and DMA channel selection, sets up MAC packet filter with hash filter enabled and receive-all mode, enables TX and RX in MAC configuration with full duplex mode, configures MTL TX queue operation modes with 4096-byte queue size and store-and-forward for 4 queues, programs quantum weights for all 4 TX queues, enables MTL queue RX overflow and TX underflow interrupts, programs DMA system bus mode, preloads TX descriptors, programs DMA TX descriptor ring lengths, list addresses, tail pointers, and channel controls for 4 channels, enables DMA channel interrupts for all 4 channels, enables HSS autoreg Ethernet interrupts, then starts TX on DMA channel 0. The test loops 10 times waiting for transfer complete interrupts using an interrupt-driven polling mechanism. An interrupt handler reads current TX descriptor and buffer pointers for all 4 DMA channels, reads DMA interrupt status, clears individual DMA channel status registers, verifies DMA interrupt status is cleared, and clears the HSS autoreg interrupt. The test completes with a delay and signals pass.",
        "Test Steps / Procedure": "1. Initialize the interrupt pending flag.\n2. Configure MAC Address Registers:\n   a. Program MAC address high registers for DMA channels 0 through 3 with address enable and DMA channel selection.\n   b. Program MAC address low registers for DMA channels 0 through 3 with the lower 32-bit MAC address.\n3. Configure MAC Packet Filter:\n   a. Enable hash filter and receive-all mode.\n4. Configure MAC Configuration:\n   a. Enable TX and RX with full duplex mode.\n5. Configure MAC Extended Configuration.\n6. Configure MTL TX Queue Operation Modes:\n   a. Program TX queue operation mode for queues 0 through 3 with 4096-byte queue size and store-and-forward enabled.\n7. Configure MTL TX Queue Quantum Weights:\n   a. Program quantum/weight values for queues 0 through 3.\n8. Enable MTL Queue Interrupts:\n   a. Enable RX queue overflow and TX queue underflow interrupts for queues 0 through 3.\n9. Configure DMA System Bus Mode:\n   a. Program outstanding request limits, fixed burst, address-aligned burst enable, and burst length selection.\n10. Preload TX Descriptors:\n    a. Preload TX descriptors for DMA channel 0.\n11. Configure DMA TX Descriptor Ring:\n    a. Program TX descriptor ring length for channels 0 through 3.\n    b. Program TX descriptor list base address for channels 0 through 3.\n    c. Program TX descriptor tail pointer for channels 0 through 3.\n12. Configure DMA Channel Controls:\n    a. Program DMA channel control for channels 0 through 3.\n13. Enable DMA Channel Interrupts:\n    a. Enable interrupts for DMA channels 0 through 3.\n14. Enable HSS Autoreg Ethernet Interrupts.\n15. Start TX on DMA Channel 0:\n    a. Program DMA channel 0 TX control to start transmission.\n16. Wait for Transfer Complete Interrupts:\n    a. Loop 10 times, polling the interrupt pending flag.\n    b. On each interrupt, the interrupt handler executes:\n       i. Clear the interrupt pending flag.\n       ii. Read current TX descriptor and buffer pointers for all 4 DMA channels.\n       iii. Read DMA interrupt status.\n       iv. Clear individual DMA channel status registers for channels 0 through 3.\n       v. Read DMA interrupt status again to verify clearing.\n       vi. Clear HSS autoreg interrupt.\n    c. Reset the interrupt pending flag after each iteration.\n17. Wait for a settling delay.\n18. Signal test completion with pass.",
        "Impacted Registers": "MAC_ADDRESS3_HIGH, MAC_ADDRESS2_HIGH, MAC_ADDRESS1_HIGH, MAC_ADDRESS0_HIGH, MAC_ADDRESS3_LOW, MAC_ADDRESS2_LOW, MAC_ADDRESS1_LOW, MAC_ADDRESS0_LOW, MAC_PACKET_FILTER, MAC_CONFIGURATION, MAC_EXT_CONFIGURATION, MTL_TXQ3_OPERATION_MODE, MTL_TXQ2_OPERATION_MODE, MTL_TXQ1_OPERATION_MODE, MTL_TXQ0_OPERATION_MODE, MTL_TXQ3_QUANTUM_WEIGHT, MTL_TXQ2_QUANTUM_WEIGHT, MTL_TXQ1_QUANTUM_WEIGHT, MTL_TXQ0_QUANTUM_WEIGHT, MTL_Q3_INTERRUPT_CONTROL_STATUS, MTL_Q2_INTERRUPT_CONTROL_STATUS, MTL_Q1_INTERRUPT_CONTROL_STATUS, MTL_Q0_INTERRUPT_CONTROL_STATUS, DMA_SYSBUS_MODE, DMA_CH3_TXDESC_RING_LENGTH, DMA_CH2_TXDESC_RING_LENGTH, DMA_CH1_TXDESC_RING_LENGTH, DMA_CH0_TXDESC_RING_LENGTH, DMA_CH3_TXDESC_LIST_ADDRESS, DMA_CH2_TXDESC_LIST_ADDRESS, DMA_CH1_TXDESC_LIST_ADDRESS, DMA_CH0_TXDESC_LIST_ADDRESS, DMA_CH3_TXDESC_TAIL_POINTER, DMA_CH2_TXDESC_TAIL_POINTER, DMA_CH1_TXDESC_TAIL_POINTER, DMA_CH0_TXDESC_TAIL_POINTER, DMA_CH3_CONTROL, DMA_CH2_CONTROL, DMA_CH1_CONTROL, DMA_CH0_CONTROL, DMA_CH1_INTERRUPT_ENABLE, DMA_CH2_INTERRUPT_ENABLE, DMA_CH3_INTERRUPT_ENABLE, DMA_CH0_INTERRUPT_ENABLE, DMA_CH0_TX_CONTROL, DMA_CH0_CURRENT_APP_TXDESC, DMA_CH0_CURRENT_APP_TXBUFFER, DMA_CH1_CURRENT_APP_TXDESC, DMA_CH1_CURRENT_APP_TXBUFFER, DMA_CH2_CURRENT_APP_TXDESC, DMA_CH2_CURRENT_APP_TXBUFFER, DMA_CH3_CURRENT_APP_TXDESC, DMA_CH3_CURRENT_APP_TXBUFFER, DMA_INTERRUPT_STATUS, DMA_CH0_STATUS, DMA_CH1_STATUS, DMA_CH2_STATUS, DMA_CH3_STATUS",
        "Validation / Acceptance Criteria": "1. Interrupt Reception: The test must successfully receive 10 transfer complete interrupts within the polling loop. Each interrupt must cause the interrupt handler to execute and clear the interrupt pending flag.\n2. DMA Channel Status Clearing: After writing to clear all 4 DMA channel status registers in the interrupt handler, the DMA interrupt status register should reflect that the individual channel interrupts have been cleared.\n3. Interrupt Handler Execution: The interrupt handler must read current TX descriptor and buffer pointers for all 4 DMA channels, read DMA interrupt status, clear all 4 DMA channel status registers, and clear the HSS autoreg interrupt on each invocation.\n4. Overall Pass Criteria: The test signals pass unconditionally after all 10 interrupt iterations complete and the settling delay elapses. No explicit data comparison or fail counter is used; the test relies on successful interrupt-driven flow completion as the implicit pass criterion.",
        "Remarks": "No explicit pass/fail assertions or data comparisons are present in this testcase. The test is primarily a functional TX exercise with interrupt-driven flow control. Only DMA channel 0 TX is actively started; TX control for DMA channels 1, 2, and 3 is defined but not invoked in the active test flow. The interrupt handler performs observational reads of TX descriptor and buffer pointers without comparing against expected values."
    }
]

# ============================================================
# METADATA (from Agent 6A) - abbreviated for embedding
# Full content preserved in metadata_data.json
# ============================================================
META_JSON = [
    {
        "Test Case Name": "ethernet1_reg_wr_rd_test/program.c",
        "Meta Headers": "File: ethernet1_reg_wr_rd_test/program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include \"test_define.c\"\n4. #include <test_common.h>\n\nFile: ethernet1_reg_wr_rd_test/test_define.c\n1. #include<ethernet1/ethernet1_def.h>\n2. #include<ethernet1/ethernet1_offset.h>",
        "Meta Macros": "File: ethernet1_reg_wr_rd_test/program.c\n1. #define SOFT_RST_REG_ADDRESS\t0x00000000\n2. #define SOFT_RST_REG_DATA\t0x00000000\n\nFile: ethernet1_reg_wr_rd_test/test_define.c\n1. #define CNT 434\n\nConditional compilation directives (both files):\n- #ifdef DEBUG_DISPLAY ... #endif (used in test_case(), chk_rst_val(), chk_rd_wr() in program.c)",
        "Meta Arrays": "File: ethernet1_reg_wr_rd_test/program.c (function chk_rd_wr, local scope)\nint chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};\n\nFile: ethernet1_reg_wr_rd_test/test_define.c (file scope)\n1. const unsigned long int addr_array[434]={...};\n2. const int default_value_array[434]={...};\n3. const int read_mask_array[434]={...};\n4. const int write_mask_array[434]={...};\n5. const int skip_array[434]={0,0,0,0,0,0,1,1,0,1,1,1,1,};\n6. const int skip_rst_array[434]={0,0,0,0,0,0,0,0,0,0,0,};",
        "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: ethernet1_reg_wr_rd_test/program.c (file scope)\n1. int data_rd,data_wr;\n2. int def_fail_cnt = 0,wr_fail_cnt = 0;\n\nFile: ethernet1_reg_wr_rd_test/test_define.c (file scope)\n1. int data_rd,data_wr,rst_val;",
        "Meta Test Description": "This testcase performs register write-read verification for the Ethernet1 IP...",
        "Meta Test Steps / Procedure": "SOURCE_FILE: ethernet1_reg_wr_rd_test/program.c\nSOURCE_CONTENT:\n#include <stdio.h>\n...",
        "Meta Impacted Registers": "Source: Agent 4 Output\n\nMatched Registers:\n1. MAC_CONFIGURATION\n2. MAC_EXT_CONFIGURATION\n...\n\nUnresolved Tokens:\n1. CNT\n2. SOFT_RST_REG_ADDRESS\n3. SOFT_RST_REG_DATA",
        "Meta Validation / Acceptance Criteria": "File: ethernet1_reg_wr_rd_test/program.c\n\n1. Function: chk_rst_val()\n   Validation: if(data_rd == default_value_array[i])\n   - Pass: prints PASS\n   - Fail: def_fail_cnt++\n\n2. Function: chk_rd_wr()\n   Validation: if(data_rd == exp_val)\n   - Pass: prints PASS\n   - Fail: wr_fail_cnt++\n\n3. Function: test_case()\n   Validation: if(def_fail_cnt > 0 || wr_fail_cnt > 0)\n   - Fail: finish(1)\n   - Pass: finish(0)"
    },
    {
        "Test Case Name": "ethernet1_tx_basic_test/program.c",
        "Meta Headers": "File: ethernet1_tx_basic_test/program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include<ethernet1/ethernet1_def.h>\n4. #include<ethernet1/ethernet1_offset.h>\n5. #include <test_common.h>\n6. #include<ethernet0/ethernet0_programming_sequence.h>",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: ethernet1_tx_basic_test/program.c (file scope)\n1. int data_rd,data_wr;\n2. int def_fail_cnt = 0,wr_fail_cnt = 0;\n3. extern int int_pend;",
        "Meta Test Description": "This testcase performs a basic Ethernet1 TX (transmit) operation...",
        "Meta Test Steps / Procedure": "SOURCE_FILE: ethernet1_tx_basic_test/program.c\nSOURCE_CONTENT:\n...",
        "Meta Impacted Registers": "Source: Agent 4 Output\n\nMatched Registers:\n1-58 registers...",
        "Meta Validation / Acceptance Criteria": "No explicit validation comparisons. Test relies on interrupt-driven flow and finish(0)."
    }
]

# ============================================================
# COLUMN DEFINITIONS
# ============================================================
TP_COLS = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Speed', 'Mode',
    'Memory Start Offset', 'Memory End Offset', 'Test Description',
    'Test Steps / Procedure', 'Impacted Registers',
    'Validation / Acceptance Criteria', 'Remarks',
    'Code Generation (Required / Not)'
]

MD_COLS = [
    'Test Case Name', 'Meta Headers', 'Meta Macros', 'Meta Arrays',
    'Meta Types', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria'
]

# ============================================================
# STYLES
# ============================================================
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')

# ============================================================
# BUILD WORKBOOK
# ============================================================
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# Headers
for ci, col_name in enumerate(TP_COLS, 1):
    cell = ws_tp.cell(row=1, column=ci, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Data rows
for ri, row_data in enumerate(TESTPLAN_JSON, 2):
    for ci, col_name in enumerate(TP_COLS, 1):
        if col_name == 'Code Generation (Required / Not)':
            ws_tp.cell(row=ri, column=ci, value='').alignment = wrap_align
        else:
            val = row_data.get(col_name, '')
            if val is None:
                val = ''
            ws_tp.cell(row=ri, column=ci, value=val).alignment = wrap_align

# Data Validation for Code Generation column
dv = DataValidation(
    type='list',
    formula1='"Required,Not Required"',
    allow_blank=True
)
dv.error = 'Please select Required or Not Required'
dv.errorTitle = 'Invalid Entry'
dv.prompt = 'Select Code Generation status'
dv.promptTitle = 'Code Generation'
cg_col_idx = len(TP_COLS)
cg_col_letter = get_column_letter(cg_col_idx)
for ri in range(2, len(TESTPLAN_JSON) + 2):
    dv.add(ws_tp.cell(row=ri, column=cg_col_idx))
ws_tp.add_data_validation(dv)

# Freeze first row
ws_tp.freeze_panes = 'A2'

# Auto-size columns
for ci in range(1, len(TP_COLS) + 1):
    max_len = len(str(ws_tp.cell(row=1, column=ci).value or ''))
    for ri in range(2, len(TESTPLAN_JSON) + 2):
        val = str(ws_tp.cell(row=ri, column=ci).value or '')
        for line in val.split('\n'):
            if len(line) > max_len:
                max_len = len(line)
    width = min(max_len + 4, 80)
    ws_tp.column_dimensions[get_column_letter(ci)].width = max(width, 12)

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')
ws_md.sheet_state = 'veryHidden'

# Headers
for ci, col_name in enumerate(MD_COLS, 1):
    cell = ws_md.cell(row=1, column=ci, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Data rows
for ri, row_data in enumerate(META_JSON, 2):
    for ci, col_name in enumerate(MD_COLS, 1):
        val = row_data.get(col_name, '')
        if val is None:
            val = ''
        ws_md.cell(row=ri, column=ci, value=val).alignment = wrap_align

# Freeze first row
ws_md.freeze_panes = 'A2'

# Auto-size columns
for ci in range(1, len(MD_COLS) + 1):
    max_len = len(str(ws_md.cell(row=1, column=ci).value or ''))
    for ri in range(2, len(META_JSON) + 2):
        val = str(ws_md.cell(row=ri, column=ci).value or '')
        for line in val.split('\n'):
            if len(line) > max_len:
                max_len = len(line)
    width = min(max_len + 4, 80)
    ws_md.column_dimensions[get_column_letter(ci)].width = max(width, 12)

# ============================================================
# SAVE
# ============================================================
outpath = os.path.join(SCRIPT_DIR, FILENAME)
wb.save(outpath)
print(f"Workbook saved: {outpath}")
print(f"Filename: {FILENAME}")
print(f"TestPlan rows: {len(TESTPLAN_JSON)}")
print(f"MetaData rows: {len(META_JSON)}")

# ============================================================
# POST-SAVE VALIDATION
# ============================================================
assert os.path.exists(outpath), "Workbook file does not exist"
assert os.path.getsize(outpath) > 0, "Workbook file is empty"

wb2 = load_workbook(outpath)
assert 'TestPlan' in wb2.sheetnames, "TestPlan sheet missing"
assert 'MetaData' in wb2.sheetnames, "MetaData sheet missing"
assert wb2['MetaData'].sheet_state == 'veryHidden', "MetaData not veryHidden"
assert wb2['TestPlan'].max_row - 1 == len(TESTPLAN_JSON), f"TestPlan row count mismatch: {wb2['TestPlan'].max_row - 1} vs {len(TESTPLAN_JSON)}"
assert wb2['MetaData'].max_row - 1 == len(META_JSON), f"MetaData row count mismatch: {wb2['MetaData'].max_row - 1} vs {len(META_JSON)}"

# Verify Code Generation column header
cg_header = wb2['TestPlan'].cell(row=1, column=cg_col_idx).value
assert cg_header == 'Code Generation (Required / Not)', f"Code Generation header mismatch: {cg_header}"

# Verify Code Generation cells are blank
for ri in range(2, len(TESTPLAN_JSON) + 2):
    val = wb2['TestPlan'].cell(row=ri, column=cg_col_idx).value
    assert val == '' or val is None, f"Code Generation cell row {ri} not blank: {val}"

# Verify data validations exist
assert len(wb2['TestPlan'].data_validations.dataValidation) > 0, "No data validations found"

print("ALL VALIDATIONS PASSED")
print(f"Output: {outpath}")
