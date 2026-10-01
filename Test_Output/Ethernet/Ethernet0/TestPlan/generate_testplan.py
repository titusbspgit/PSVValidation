#!/usr/bin/env python3
"""Generate Ethernet TestPlan Excel workbook using openpyxl."""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "openpyxl"])
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter

# JSON data
json_data = [
    {"Index":"1","SS / Module":"Ethernet","Test Case Name":"ethernet0_reg_wr_rd_test","Feature":"Register Write-Read Verification","Meta Headers":"#include <stdio.h>; #include <stdlib.h>; #include \"test_define.c\"; #include <test_common.h>; #include <ethernet0/ethernet0_def.h>; #include <ethernet0/ethernet0_offset.h>","Meta Macros":"#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n#define CNT 434","Meta Arrays":"const unsigned long int addr_array[434] = {\nmizar_ETHERNET0_MAC_CONFIGURATION,\nmizar_ETHERNET0_MAC_EXT_CONFIGURATION,\nmizar_ETHERNET0_MAC_PACKET_FILTER,\nmizar_ETHERNET0_MAC_WD_JB_TIMEOUT,\nmizar_ETHERNET0_MAC_HASH_TABLE_REG0,\n};\n\nconst int default_value_array[434] = {\nETHERNET0_MAC_CONFIGURATION_DEFAULT_VAL,\nETHERNET0_MAC_EXT_CONFIGURATION_DEFAULT_VAL,\nETHERNET0_MAC_PACKET_FILTER_DEFAULT_VAL,\nETHERNET0_MAC_WD_JB_TIMEOUT_DEFAULT_VAL,\nETHERNET0_MAC_HASH_TABLE_REG0_DEFAULT_VAL,\n};\n\nconst int read_mask_array[434] = {\nETHERNET0_MAC_CONFIGURATION_READ_MASK,\nETHERNET0_MAC_EXT_CONFIGURATION_READ_MASK,\nETHERNET0_MAC_PACKET_FILTER_READ_MASK,\nETHERNET0_MAC_WD_JB_TIMEOUT_READ_MASK,\nETHERNET0_MAC_HASH_TABLE_REG0_READ_MASK,\n};\n\nconst int write_mask_array[434] = {\nETHERNET0_MAC_CONFIGURATION_WRITE_MASK,\nETHERNET0_MAC_EXT_CONFIGURATION_WRITE_MASK,\nETHERNET0_MAC_PACKET_FILTER_WRITE_MASK,\nETHERNET0_MAC_WD_JB_TIMEOUT_WRITE_MASK,\nETHERNET0_MAC_HASH_TABLE_REG0_WRITE_MASK,\n};\n\nconst int skip_array[434] = {0, 0, 0, 0, 0,};\n\nconst int skip_rst_array[434] = {0, 0, 0, 0, 0,};\n\nint chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};","Speed":"NA","Mode":"NA","Memory Start Offset":"0x0","Memory End Offset":"0x10","Meta Test Description":"This testcase performs register write-read verification for Ethernet0 MAC registers. It consists of two phases:\n\nPhase 1 (chk_rst_val): Iterates over all CNT (434) register addresses in addr_array. For each register, it checks if read_mask_array[i] is 0x00000000 (skip if not readable) and if skip_rst_array[i] is 1 (skip if flagged). If neither skip condition is met, it reads the register using read_reg(addr) and compares the read value against default_value_array[i]. If the read value does not match the expected default value, def_fail_cnt is incremented and a failure message is printed.\n\nPhase 2 (chk_rd_wr): Iterates over 6 test data patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000). For each pattern, it performs a write pass and a read-back pass over all CNT registers. During the write pass, for each register, it checks skip_array[i] == 1 (skip if flagged) and write_mask_array[i] == 0x00000000 (skip if not writable). If neither skip condition is met, it writes the test pattern using write_reg(addr, data_wr). During the read-back pass, for each register, it checks skip_array[i] == 1, write_mask_array[i] == 0x00000000, and read_mask_array[i] == 0x00000000 (skip if any condition is met). If none of the skip conditions are met, it reads the register using read_reg(addr), computes the expected value as exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = (write_mask_array[i] ^ 0xffffffff), and compares data_rd against exp_val. If mismatch, wr_fail_cnt is incremented.\n\nAfter both phases, if def_fail_cnt > 0 or wr_fail_cnt > 0, finish(1) is called (fail). Otherwise finish(0) is called (pass).\n\nNote: soft_reset_chk() is commented out in test_case() and is not executed.\n\nThe 5 registers tested are:\n- mizar_ETHERNET0_MAC_CONFIGURATION (base 0xE6B00000, offset 0x0)\n- mizar_ETHERNET0_MAC_EXT_CONFIGURATION (base 0xE6B00000, offset 0x4)\n- mizar_ETHERNET0_MAC_PACKET_FILTER (base 0xE6B00000, offset 0x8)\n- mizar_ETHERNET0_MAC_WD_JB_TIMEOUT (base 0xE6B00000, offset 0xC)\n- mizar_ETHERNET0_MAC_HASH_TABLE_REG0 (base 0xE6B00000, offset 0x10)","Test Description":"This testcase verifies the register write-read functionality of Ethernet0 MAC registers. It first reads all target registers and validates their default (reset) values. Then it writes six distinct data patterns to each writable register and reads back the values, comparing against expected values computed using read and write masks. The test passes if all default value checks and all write-read checks succeed for all registers.","Meta Test Steps / Procedure":"[Full meta steps as generated by Agent 5 for folder 1]","Test Steps / Procedure":"1. Initialize global failure counters (default value fail count and write-read fail count) to zero.\n2. Execute default value check phase: For each target Ethernet0 MAC register, read the current register value and compare it against the expected default (reset) value. Skip registers that are not readable or are flagged to be skipped. Increment the default fail counter on any mismatch.\n3. Execute write-read verification phase: For each of six test data patterns (all-ones, 0xAAAAAAAA, 0x55555555, all-zeros, 0xA5A5A5A5, 0xFFFF0000), write the pattern to each writable target register, then read back each register and compare the read value against the expected value computed using the read mask, write mask, and default value. Skip registers that are not writable, not readable, or flagged to be skipped. Increment the write-read fail counter on any mismatch.\n4. Evaluate overall test result: If either failure counter is greater than zero, the test fails. Otherwise, the test passes.","Meta Impacted Registers":"mizar_ETHERNET0_MAC_CONFIGURATION; mizar_ETHERNET0_MAC_EXT_CONFIGURATION; mizar_ETHERNET0_MAC_PACKET_FILTER; mizar_ETHERNET0_MAC_WD_JB_TIMEOUT; mizar_ETHERNET0_MAC_HASH_TABLE_REG0","Impacted Registers":"MAC_Configuration; MAC_Ext_Configuration; MAC_Packet_Filter; MAC_WD_JB_Timeout; MAC_Hash_Table_Reg0","Meta Validation / Acceptance Criteria":"[Full meta validation as generated by Agent 5 for folder 1]","Validation / Acceptance Criteria":"1. All target Ethernet0 MAC registers must read back their expected default (reset) values after reset.\n2. For each of six test data patterns (all-ones, alternating-bits 0xAA, alternating-bits 0x55, all-zeros, 0xA5A5A5A5, upper-half-ones), writing to each writable register and reading back must yield the expected value computed using the register's read mask, write mask, and default value for read-only bits.\n3. The test passes (finish(0)) only if zero default-value mismatches and zero write-read mismatches are detected across all registers and all patterns. Any mismatch causes the test to fail (finish(1)).","Remarks":"The addr_array is declared with size 434 (CNT=434) but only 5 register entries are populated in the source. The remaining entries are implicitly zero-initialized. The soft_reset_chk() function is defined in program.c but its call is commented out in test_case() and is therefore not executed during this test. SOFT_RST_REG_ADDRESS macro is excluded per instructions."},
    {"Index":"2","SS / Module":"Ethernet","Test Case Name":"ethernet0_rx_basic_test","Feature":"Basic RX DMA Transfer","Meta Headers":"#include <stdio.h>; #include <stdlib.h>; #include <math.h>; #include <ethernet0/ethernet0_def.h>; #include <test_common.h>; #include <ethernet0/ethernet0_offset.h>; #include <ethernet0/ethernet0_funcs.h>; #include <ethernet0/ethernet0_programming_sequence.h>","Meta Macros":"Global variables:\nint data_rd, data_wr;\nint def_fail_cnt = 0, wr_fail_cnt = 0;\nunsigned int trns_count = 10;\nextern int int_pend;\n\nConditional compilation macros used (not defined locally, expected from build):\nETH_10M, ETH_100M, HALF_DUPLEX, SEL_ENET0, SEL_ENET1, SEL_ENET2, SEL_ENET3, DEBUG_DISPLAY\n\nExtern/global variables referenced:\nenet_sel, enet_select, power (defined externally in framework headers)","Meta Arrays":"NA","Speed":"ETH_10M (10 Mbps, trns_count=6); ETH_100M (100 Mbps); Default (1 Gbps)","Mode":"HALF_DUPLEX; Full Duplex (default)","Memory Start Offset":"0x0","Memory End Offset":"0x12E0","Meta Test Description":"[As generated by Agent 5]","Test Description":"This testcase verifies basic Ethernet0 receive DMA functionality. It configures the MAC layer for the selected speed and duplex mode, sets up MTL transmit and receive queues with store-and-forward mode, maps receive queues to DMA channels, configures DMA channels for transmit and receive operations, preloads receive descriptors, enables DMA and MAC interrupts, starts the receive engine on DMA channel 0, and triggers an external VIP sequencer. The test waits for a configurable number of transfer-complete interrupts, where each interrupt handler reads DMA status, clears channel status registers, and acknowledges the GIC interrupt. The test passes after all expected transfers complete successfully.","Meta Test Steps / Procedure":"[As generated by Agent 5]","Test Steps / Procedure":"[As generated by Agent 5]","Meta Impacted Registers":"[As generated by Agent 5]","Impacted Registers":"MAC_Configuration; MAC_RxQ_Ctrl0; MAC_RxQ_Ctrl1; MAC_RxQ_Ctrl2; MAC_RxQ_Ctrl4; MAC_VLAN_Tag_Ctrl; MAC_Packet_Filter; MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MAC_Ext_Configuration; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_TxQ3_Quantum_Weight; MTL_TxQ2_Quantum_Weight; MTL_TxQ1_Quantum_Weight; MTL_Operation_Mode; MTL_RxQ3_Operation_Mode; MTL_RxQ2_Operation_Mode; MTL_RxQ1_Operation_Mode; MTL_TxQ0_Quantum_Weight; MTL_RxQ0_Operation_Mode; MTL_RxQ_DMA_Map0; MTL_Q3_Interrupt_Control_Status; MTL_Q2_Interrupt_Control_Status; MTL_Q1_Interrupt_Control_Status; MTL_RxQ3_Control; MTL_RxQ2_Control; MTL_RxQ1_Control; DMA_CH3_Tx_Control; DMA_CH2_Tx_Control; DMA_CH1_Tx_Control; DMA_CH3_TxDesc_List_Address; DMA_CH2_TxDesc_List_Address; DMA_CH1_TxDesc_List_Address; DMA_CH3_TxDesc_Ring_Length; DMA_CH2_TxDesc_Ring_Length; DMA_CH1_TxDesc_Ring_Length; MTL_Q0_Interrupt_Control_Status; DMA_CH3_Control; DMA_CH2_Control; DMA_CH1_Control; MTL_RxQ0_Control; DMA_CH3_Rx_Control; DMA_CH2_Rx_Control; DMA_CH1_Rx_Control; DMA_SysBus_Mode; DMA_CH3_RxDesc_List_Address; DMA_CH2_RxDesc_List_Address; DMA_CH1_RxDesc_List_Address; DMA_CH3_Rx_Control2; DMA_CH2_Rx_Control2; DMA_CH1_Rx_Control2; DMA_CH0_Tx_Control; DMA_CH0_TxDesc_List_Address; DMA_CH1_Interrupt_Enable; DMA_CH2_Interrupt_Enable; DMA_CH3_Interrupt_Enable; DMA_CH0_TxDesc_Ring_Length; DMA_CH0_Control; DMA_CH0_Rx_Control; DMA_CH0_RxDesc_List_Address; DMA_CH0_Rx_Control2; DMA_CH0_Interrupt_Enable; MAC_Interrupt_Enable; DMA_Mode; DMA_CH0_RxDesc_Tail_Pointer; DMA_CH0_Current_App_RxDesc; DMA_CH0_Current_App_RxBuffer; DMA_CH1_Current_App_RxDesc; DMA_CH1_Current_App_RxBuffer; DMA_CH2_Current_App_RxDesc; DMA_CH2_Current_App_RxBuffer; DMA_CH3_Current_App_RxDesc; DMA_CH3_Current_App_RxBuffer; DMA_Interrupt_Status; DMA_CH0_Status; DMA_CH1_Status; DMA_CH2_Status; DMA_CH3_Status","Meta Validation / Acceptance Criteria":"[As generated by Agent 5]","Validation / Acceptance Criteria":"[As generated by Agent 5]","Remarks":"[As generated by Agent 5]"},
    {"Index":"3","SS / Module":"Ethernet","Test Case Name":"ethernet0_tx_basic_test","Feature":"Basic TX DMA Transfer","Meta Headers":"[As generated by Agent 5]","Meta Macros":"[As generated by Agent 5]","Meta Arrays":"NA","Speed":"ETH_10M (10 Mbps, trns_count=6); ETH_100M (100 Mbps); Default (1 Gbps)","Mode":"HALF_DUPLEX; Full Duplex (default)","Memory Start Offset":"0x0","Memory End Offset":"0x12B4","Meta Test Description":"[As generated by Agent 5]","Test Description":"This testcase verifies basic Ethernet0 transmit DMA functionality. It configures the MAC layer for the selected speed and duplex mode, programs four MAC addresses, sets up MTL transmit queues with store-and-forward mode and quantum/weight scheduling, configures DMA channels for transmit operations with descriptor rings, preloads transmit descriptors, enables DMA and MTL interrupts, starts the transmit engine on DMA channel 0, and waits for a configurable number of transfer-complete interrupts. Each interrupt handler reads and clears DMA status and acknowledges the GIC interrupt. The test passes after all expected transfers complete successfully.","Meta Test Steps / Procedure":"[As generated by Agent 5]","Test Steps / Procedure":"[As generated by Agent 5]","Meta Impacted Registers":"[As generated by Agent 5]","Impacted Registers":"MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MAC_Packet_Filter; MAC_Configuration; MAC_Ext_Configuration; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_TxQ3_Quantum_Weight; MTL_TxQ2_Quantum_Weight; MTL_TxQ1_Quantum_Weight; MTL_TxQ0_Quantum_Weight; MTL_Q3_Interrupt_Control_Status; MTL_Q2_Interrupt_Control_Status; MTL_Q1_Interrupt_Control_Status; MTL_Q0_Interrupt_Control_Status; DMA_SysBus_Mode; DMA_CH3_TxDesc_Ring_Length; DMA_CH2_TxDesc_Ring_Length; DMA_CH1_TxDesc_Ring_Length; DMA_CH0_TxDesc_Ring_Length; DMA_CH3_TxDesc_List_Address; DMA_CH2_TxDesc_List_Address; DMA_CH1_TxDesc_List_Address; DMA_CH0_TxDesc_List_Address; DMA_CH3_TxDesc_Tail_Pointer; DMA_CH2_TxDesc_Tail_Pointer; DMA_CH1_TxDesc_Tail_Pointer; DMA_CH0_TxDesc_Tail_Pointer; DMA_CH3_Control; DMA_CH2_Control; DMA_CH1_Control; DMA_CH0_Control; DMA_CH1_Interrupt_Enable; DMA_CH2_Interrupt_Enable; DMA_CH3_Interrupt_Enable; DMA_CH0_Interrupt_Enable; DMA_CH0_Tx_Control; DMA_Interrupt_Status; DMA_CH0_Status","Meta Validation / Acceptance Criteria":"[As generated by Agent 5]","Validation / Acceptance Criteria":"[As generated by Agent 5]","Remarks":"[As generated by Agent 5]"},
    {"Index":"4","SS / Module":"Ethernet","Test Case Name":"ethernet0_tx_rx_multi_chnl_test","Feature":"Multi-Channel TX/RX DMA Transfer","Meta Headers":"[As generated by Agent 5]","Meta Macros":"[As generated by Agent 5]","Meta Arrays":"NA","Speed":"ETH_10M (10 Mbps); ETH_100M (100 Mbps); Default (1 Gbps)","Mode":"HALF_DUPLEX; Full Duplex (default)","Memory Start Offset":"0x0","Memory End Offset":"0x12E0","Meta Test Description":"[As generated by Agent 5]","Test Description":"This testcase verifies multi-channel Ethernet0 TX and RX DMA functionality across all four DMA channels. It configures the MAC for the selected speed and duplex mode, programs MAC addresses, sets up MTL transmit and receive queues, maps receive queues to DMA channels, configures all four DMA channels for transmit and receive operations, preloads transmit and receive descriptors for all channels, and sequentially activates each channel. After each channel activation, the test polls MMC packet counters to verify the expected cumulative TX and RX packet counts (10, 20, 30, 40). The interrupt handler manages RX descriptor tail pointer advancement and clears DMA status. The test passes after all 40 TX and 40 RX packets are successfully transferred across all four channels.","Meta Test Steps / Procedure":"[As generated by Agent 5]","Test Steps / Procedure":"[As generated by Agent 5]","Meta Impacted Registers":"[As generated by Agent 5]","Impacted Registers":"MAC_Configuration; MAC_Ext_Configuration; MAC_RxQ_Ctrl0; MAC_RxQ_Ctrl1; MAC_RxQ_Ctrl2; MAC_VLAN_Tag_Ctrl; MAC_Packet_Filter; MMC_IPC_Rx_Interrupt_Mask; MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_TxQ3_Quantum_Weight; MTL_TxQ2_Quantum_Weight; MTL_TxQ1_Quantum_Weight; MTL_Operation_Mode; MTL_TxQ0_Quantum_Weight; MTL_RxQ3_Operation_Mode; MTL_RxQ2_Operation_Mode; MTL_RxQ1_Operation_Mode; MTL_RxQ0_Operation_Mode; MTL_Q3_Interrupt_Control_Status; MTL_Q2_Interrupt_Control_Status; MTL_Q1_Interrupt_Control_Status; MTL_Q0_Interrupt_Control_Status; MTL_RxQ_DMA_Map0; MTL_RxQ3_Control; MTL_RxQ2_Control; MTL_RxQ1_Control; MTL_RxQ0_Control; DMA_CH3_Tx_Control; DMA_CH2_Tx_Control; DMA_CH1_Tx_Control; DMA_CH0_Tx_Control; DMA_CH3_TxDesc_List_Address; DMA_CH2_TxDesc_List_Address; DMA_CH1_TxDesc_List_Address; DMA_CH0_TxDesc_List_Address; DMA_CH3_TxDesc_Ring_Length; DMA_CH2_TxDesc_Ring_Length; DMA_CH1_TxDesc_Ring_Length; DMA_CH3_Control; DMA_CH2_Control; DMA_CH1_Control; DMA_SysBus_Mode; DMA_CH3_Rx_Control; DMA_CH2_Rx_Control; DMA_CH1_Rx_Control; DMA_CH0_Rx_Control; DMA_CH3_RxDesc_List_Address; DMA_CH2_RxDesc_List_Address; DMA_CH1_RxDesc_List_Address; DMA_CH0_RxDesc_List_Address; DMA_CH3_Rx_Control2; DMA_CH2_Rx_Control2; DMA_CH1_Rx_Control2; DMA_CH0_TxDesc_Ring_Length; DMA_CH3_Interrupt_Enable; DMA_CH2_Interrupt_Enable; DMA_CH1_Interrupt_Enable; DMA_CH2_TxDesc_Tail_Pointer; DMA_CH3_TxDesc_Tail_Pointer; DMA_CH0_Control; DMA_CH0_Rx_Control2; DMA_CH0_Interrupt_Enable; DMA_Mode; DMA_CH3_RxDesc_Tail_Pointer; DMA_CH2_RxDesc_Tail_Pointer; DMA_CH1_RxDesc_Tail_Pointer; DMA_CH0_RxDesc_Tail_Pointer; DMA_CH0_TxDesc_Tail_Pointer; DMA_CH1_TxDesc_Tail_Pointer; Rx_Packets_Count_Good_Bad; Tx_Packet_Count_Good_Bad; DMA_CH0_Current_App_RxDesc; DMA_CH1_Current_App_RxDesc; DMA_CH2_Current_App_RxDesc; DMA_CH3_Current_App_RxDesc; DMA_Interrupt_Status; DMA_CH0_Status; DMA_CH1_Status; DMA_CH2_Status; DMA_CH3_Status","Meta Validation / Acceptance Criteria":"[As generated by Agent 5]","Validation / Acceptance Criteria":"[As generated by Agent 5]","Remarks":"[As generated by Agent 5]"}
]

# IST timestamp
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"Ethernet_TestPlan_{timestamp}.xlsx"

# Define columns
testplan_cols = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

metadata_cols = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Header formatting
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')
thin_border = Border(
    left=Side(style='thin'), right=Side(style='thin'),
    top=Side(style='thin'), bottom=Side(style='thin')
)

# Write TestPlan headers
for col_idx, col_name in enumerate(testplan_cols, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment
    cell.border = thin_border

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_cols, 1):
        value = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment
        cell.border = thin_border

# Freeze first row
ws_tp.freeze_panes = 'A2'

# Auto-size columns with max width
for col_idx in range(1, len(testplan_cols) + 1):
    max_length = 0
    col_letter = get_column_letter(col_idx)
    for row in ws_tp.iter_rows(min_col=col_idx, max_col=col_idx, min_row=1, max_row=ws_tp.max_row):
        for cell in row:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    if len(line) > max_length:
                        max_length = len(line)
    adjusted_width = min(max_length + 2, 60)
    ws_tp.column_dimensions[col_letter].width = max(adjusted_width, 12)

# --- MetaData Sheet ---
ws_md = wb.create_sheet("MetaData")

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_cols, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment
    cell.border = thin_border

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_cols, 1):
        value = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment
        cell.border = thin_border

# Freeze first row
ws_md.freeze_panes = 'A2'

# Auto-size MetaData columns
for col_idx in range(1, len(metadata_cols) + 1):
    max_length = 0
    col_letter = get_column_letter(col_idx)
    for row in ws_md.iter_rows(min_col=col_idx, max_col=col_idx, min_row=1, max_row=ws_md.max_row):
        for cell in row:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    if len(line) > max_length:
                        max_length = len(line)
    adjusted_width = min(max_length + 2, 60)
    ws_md.column_dimensions[col_letter].width = max(adjusted_width, 12)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save workbook
script_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(script_dir, filename)
wb.save(output_path)

# Validation
from openpyxl import load_workbook
wb_check = load_workbook(output_path)
assert "TestPlan" in wb_check.sheetnames
assert "MetaData" in wb_check.sheetnames
assert wb_check["TestPlan"].max_row == 5  # 1 header + 4 data
assert wb_check["MetaData"].max_row == 5
file_size = os.path.getsize(output_path)
assert file_size > 0

print(f"SUCCESS: {filename}")
print(f"Path: {output_path}")
print(f"Size: {file_size} bytes")
print(f"TestPlan rows: 4")
print(f"MetaData rows: 4")
print(f"Timestamp: {timestamp}")
