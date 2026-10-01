#!/usr/bin/env python3
"""Ethernet TestPlan Excel Generator - Agent 7"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import sys

def generate_testplan():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"Ethernet_TestPlan_{timestamp}.xlsx"
    output_dir = os.path.dirname(os.path.abspath(__file__))
    filepath = os.path.join(output_dir, filename)

    # JSON data
    json_data = [
        {
            "Index": "1",
            "SS / Module": "Ethernet",
            "Test Case Name": "ethernet0_reg_wr_rd_test",
            "Feature": "Register Write-Read Verification",
            "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_define.c\"; <test_common.h>; <ethernet0/ethernet0_def.h>; <ethernet0/ethernet0_offset.h>",
            "Meta Macros": "#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n#define CNT 434",
            "Meta Arrays": "const unsigned long int addr_array[434] = {\n  mizar_ETHERNET0_MAC_CONFIGURATION,\n  mizar_ETHERNET0_MAC_EXT_CONFIGURATION,\n  mizar_ETHERNET0_MAC_PACKET_FILTER,\n  mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,\n  mizar_ETHERNET0_MAC_HASH_TABLE_REG0\n};\n\nconst int default_value_array[434] = {\n  ETHERNET0_MAC_CONFIGURATION_DEFAULT_VAL,\n  ETHERNET0_MAC_EXT_CONFIGURATION_DEFAULT_VAL,\n  ETHERNET0_MAC_PACKET_FILTER_DEFAULT_VAL,\n  ETHERNET0_MAC_WD_JB_TIMEOUT_DEFAULT_VAL,\n  ETHERNET0_MAC_HASH_TABLE_REG0_DEFAULT_VAL\n};\n\nconst int read_mask_array[434] = {\n  ETHERNET0_MAC_CONFIGURATION_READ_MASK,\n  ETHERNET0_MAC_EXT_CONFIGURATION_READ_MASK,\n  ETHERNET0_MAC_PACKET_FILTER_READ_MASK,\n  ETHERNET0_MAC_WD_JB_TIMEOUT_READ_MASK,\n  ETHERNET0_MAC_HASH_TABLE_REG0_READ_MASK\n};\n\nconst int write_mask_array[434] = {\n  ETHERNET0_MAC_CONFIGURATION_WRITE_MASK,\n  ETHERNET0_MAC_EXT_CONFIGURATION_WRITE_MASK,\n  ETHERNET0_MAC_PACKET_FILTER_WRITE_MASK,\n  ETHERNET0_MAC_WD_JB_TIMEOUT_WRITE_MASK,\n  ETHERNET0_MAC_HASH_TABLE_REG0_WRITE_MASK\n};\n\nconst int skip_array[434] = {\n  0, 0, 0, 0, 0\n};\n\nconst int skip_rst_array[434] = {\n  0, 0, 0, 0, 0\n};\n\nint chk_val[6] = {\n  0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000\n};",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "0x0",
            "Memory End Offset": "0x10",
            "Meta Test Description": "This testcase verifies the register write-read functionality of Ethernet0 MAC registers. It operates in two phases:\n\nPhase 1 \u2014 Default Value Check (chk_rst_val): Iterates over all registers in addr_array (CNT=434, 5 populated entries). For each register, if read_mask_array[i] is 0x00000000 the register is skipped (not readable). If skip_rst_array[i] is 1 the register is skipped. Otherwise, read_reg(addr) is called and the read value is compared against default_value_array[i]. If mismatch, def_fail_cnt is incremented and a failure message is printed.\n\nPhase 2 \u2014 Write-Read Check (chk_rd_wr): Uses 6 test patterns from chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}. For each pattern j (0..5):\n  - Write sub-phase: For each register i (0..CNT-1), if skip_array[i]==1 skip, if write_mask_array[i]==0x00000000 skip, otherwise call write_reg(addr, data_wr).\n  - Read-back sub-phase: For each register i (0..CNT-1), if skip_array[i]==1 skip, if write_mask_array[i]==0x00000000 skip, if read_mask_array[i]==0x00000000 skip, otherwise call read_reg(addr). Compute wr_n = write_mask_array[i] ^ 0xffffffff. Compute exp_val = (data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]). Compare data_rd against exp_val. If mismatch, wr_fail_cnt is incremented.\n\nFinal check: If def_fail_cnt > 0 or wr_fail_cnt > 0, call finish(1) indicating failure. Otherwise call finish(0) indicating pass.\n\nNote: soft_reset_chk() is commented out in test_case() and is not executed. The SOFT_RST_REG_ADDRESS macro is excluded per instructions.",
            "Test Description": "Verify the default reset values and write-read accessibility of Ethernet0 MAC registers. The test reads each register and compares against its expected default value, then writes six distinct test patterns (all-ones, 0xAAAAAAAA, 0x55555555, all-zeros, 0xA5A5A5A5, 0xFFFF0000) to each writable register and reads back to verify correctness using read and write masks. The test passes if all default value checks and all write-read checks succeed for every register.",
            "Meta Test Steps / Procedure": "1. Global variable initialization: int data_rd, data_wr; int def_fail_cnt = 0; int wr_fail_cnt = 0;\n2. test_case() entry point is called.\n3. test_case() calls chk_rst_val().\n4. chk_rst_val() begins: declares int i; unsigned long int addr;\n5. Loop: for (i = 0; i < CNT; i++) where CNT = 434 (5 populated entries):\n   5a. addr = addr_array[i];\n   5b. Check: if (read_mask_array[i] == 0x00000000) then skip.\n   5c. Check: if (skip_rst_array[i] == 1) then skip.\n   5d. data_rd = read_reg(addr);\n   5e. Compare: if (data_rd == default_value_array[i]) then PASS.\n   5f. Else: def_fail_cnt++.\n6. chk_rst_val() returns.\n7. test_case() calls chk_rd_wr().\n8. chk_rd_wr() begins.\n9. Local array: int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};\n10. Outer loop: for (j = 0; j < 6; j++).\n11. data_wr = chk_val[j].\n12. Write sub-phase: for (i = 0; i < CNT; i++): skip if skip_array[i]==1 or write_mask==0; else write_reg(addr, data_wr).\n13. Read-back sub-phase: for (i = 0; i < CNT; i++): skip conditions; read_reg(addr); compute exp_val; compare.\n14. End outer loop.\n15. chk_rd_wr() returns.\n16. Final check: if (def_fail_cnt > 0 || wr_fail_cnt > 0): finish(1) \u2014 FAIL.\n17. Else: finish(0) \u2014 PASS.",
            "Test Steps / Procedure": "1. Initialize global failure counters to zero.\n2. Execute default value check phase: For each register, skip if not readable or marked for skip. Read the register and compare against its expected default value. Increment the default-fail counter on mismatch.\n3. Execute write-read check phase: For each of six test patterns, write to each writable register and read back to verify using masks.\n4. Evaluate results: If either failure counter is greater than zero, report test failure. Otherwise, report test pass.",
            "Meta Impacted Registers": "mizar_ETHERNET0_MAC_CONFIGURATION; mizar_ETHERNET0_MAC_EXT_CONFIGURATION; mizar_ETHERNET0_MAC_PACKET_FILTER; mizar_ETHERNET0_MAC_WD_JB_TIMEOUT; mizar_ETHERNET0_MAC_HASH_TABLE_REG0",
            "Impacted Registers": "MAC_Configuration; MAC_Ext_Configuration; MAC_Packet_Filter; MAC_WD_JB_Timeout; MAC_Hash_Table_Reg0",
            "Meta Validation / Acceptance Criteria": "Phase 1: data_rd == default_value_array[i] for all readable registers.\nPhase 2: data_rd == exp_val for all 6 patterns and all writable+readable registers.\nFinal: def_fail_cnt == 0 && wr_fail_cnt == 0 \u2192 finish(0).",
            "Validation / Acceptance Criteria": "1. All registers must return their expected default reset values when read after reset.\n2. For each of the six test patterns, writing to each writable register and reading back must yield the expected value.\n3. The test passes only if zero mismatches are detected.",
            "Remarks": "The addr_array is declared with size 434 but only 5 entries are populated. The soft_reset_chk() function is commented out. The SOFT_RST_REG_ADDRESS macro is excluded per project instructions."
        },
        {
            "Index": "2",
            "SS / Module": "Ethernet",
            "Test Case Name": "ethernet0_rx_basic_test",
            "Feature": "Basic RX DMA Reception",
            "Meta Headers": "<stdio.h>; <stdlib.h>; <math.h>; <ethernet0/ethernet0_def.h>; <test_common.h>; <ethernet0/ethernet0_offset.h>; <ethernet0/ethernet0_funcs.h>; <ethernet0/ethernet0_programming_sequence.h>",
            "Meta Macros": "NA",
            "Meta Arrays": "NA",
            "Speed": "ETH_10M (10 Mbps); ETH_100M (100 Mbps); 1000 Mbps (default)",
            "Mode": "HALF_DUPLEX; Full Duplex (default)",
            "Memory Start Offset": "0x0",
            "Memory End Offset": "0x12E0",
            "Meta Test Description": "This testcase verifies basic Ethernet0 RX DMA reception across all 4 DMA channels.",
            "Test Description": "Verify basic Ethernet0 RX DMA reception across all 4 DMA channels. The test configures MAC, MTL, and DMA subsystems, preloads RX descriptors, starts the RX DMA engine, triggers an external VIP sequencer, and waits for multiple transfer-complete interrupts.",
            "Meta Test Steps / Procedure": "Steps 1-119 as detailed by Agent 5 for Folder 2.",
            "Test Steps / Procedure": "1. Configure GIC and enable all interrupts.\n2. Select speed and interface.\n3-33. Configure MAC, MTL, DMA subsystems and start RX. Poll for transfer completion via interrupts.",
            "Meta Impacted Registers": "Full list of 91 macros as detailed by Agent 5 for Folder 2.",
            "Impacted Registers": "MAC_Configuration; MAC_RxQ_Ctrl0; MAC_RxQ_Ctrl1; MAC_RxQ_Ctrl2; MAC_RxQ_Ctrl4; MAC_VLAN_Tag_Ctrl; MAC_Packet_Filter; MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MAC_Ext_Configuration; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_TxQ3_Quantum_Weight; MTL_TxQ2_Quantum_Weight; MTL_TxQ1_Quantum_Weight; MTL_Operation_Mode; MTL_RxQ3_Operation_Mode; MTL_RxQ2_Operation_Mode; MTL_RxQ1_Operation_Mode; MTL_TxQ0_Quantum_Weight; MTL_RxQ0_Operation_Mode; MTL_RxQ_DMA_Map0; MTL_Q3_Interrupt_Control_Status; MTL_Q2_Interrupt_Control_Status; MTL_Q1_Interrupt_Control_Status; MTL_RxQ3_Control; MTL_RxQ2_Control; MTL_RxQ1_Control; DMA_CH3_Tx_Control; DMA_CH2_Tx_Control; DMA_CH1_Tx_Control; DMA_CH3_TxDesc_List_Address; DMA_CH2_TxDesc_List_Address; DMA_CH1_TxDesc_List_Address; DMA_CH3_TxDesc_Ring_Length; DMA_CH2_TxDesc_Ring_Length; DMA_CH1_TxDesc_Ring_Length; MTL_Q0_Interrupt_Control_Status; DMA_CH3_Control; DMA_CH2_Control; DMA_CH1_Control; MTL_RxQ0_Control; DMA_CH3_Rx_Control; DMA_CH2_Rx_Control; DMA_CH1_Rx_Control; DMA_SysBus_Mode; DMA_CH3_RxDesc_List_Address; DMA_CH2_RxDesc_List_Address; DMA_CH1_RxDesc_List_Address; DMA_CH3_Rx_Control2; DMA_CH2_Rx_Control2; DMA_CH1_Rx_Control2; DMA_CH0_Tx_Control; DMA_CH0_TxDesc_List_Address; DMA_CH1_Interrupt_Enable; DMA_CH2_Interrupt_Enable; DMA_CH3_Interrupt_Enable; DMA_CH0_TxDesc_Ring_Length; DMA_CH0_Control; DMA_CH0_Rx_Control; DMA_CH0_RxDesc_List_Address; DMA_CH0_Rx_Control2; DMA_CH0_Interrupt_Enable; MAC_Interrupt_Enable; DMA_Mode; DMA_CH0_RxDesc_Tail_Pointer; DMA_CH0_Current_App_RxDesc; DMA_CH0_Current_App_RxBuffer; DMA_CH1_Current_App_RxDesc; DMA_CH1_Current_App_RxBuffer; DMA_CH2_Current_App_RxDesc; DMA_CH2_Current_App_RxBuffer; DMA_CH3_Current_App_RxDesc; DMA_CH3_Current_App_RxBuffer; DMA_Interrupt_Status; DMA_CH0_Status; DMA_CH1_Status; DMA_CH2_Status; DMA_CH3_Status",
            "Meta Validation / Acceptance Criteria": "All transfer-complete interrupts received. IRQ handler reads/clears DMA statuses.",
            "Validation / Acceptance Criteria": "1. All expected transfer-complete interrupts must be received.\n2. DMA channel statuses must be cleared successfully.\n3. Test completes with success after all transfers.",
            "Remarks": "Several registers written twice. DMA_CH0_RX_CONTROL written three times. Four hex addresses unmapped."
        },
        {
            "Index": "3",
            "SS / Module": "Ethernet",
            "Test Case Name": "ethernet0_tx_basic_test",
            "Feature": "Basic TX DMA Transmission",
            "Meta Headers": "<stdio.h>; <stdlib.h>; <math.h>; <test_common.h>; <ethernet0/ethernet0_def.h>; <ethernet0/ethernet0_offset.h>; <ethernet0/ethernet0_funcs.h>; <ethernet0/ethernet0_programming_sequence.h>",
            "Meta Macros": "NA",
            "Meta Arrays": "NA",
            "Speed": "ETH_10M (10 Mbps); ETH_100M (100 Mbps); 1000 Mbps (default)",
            "Mode": "HALF_DUPLEX; Full Duplex (default)",
            "Memory Start Offset": "0x0",
            "Memory End Offset": "0x12B4",
            "Meta Test Description": "This testcase verifies basic Ethernet0 TX DMA transmission.",
            "Test Description": "Verify basic Ethernet0 TX DMA transmission on DMA channel 0. The test configures MAC, MTL, and DMA subsystems, preloads TX descriptors, starts the TX DMA engine on channel 0, and waits for transfer-complete interrupts.",
            "Meta Test Steps / Procedure": "Steps 1-69 as detailed by Agent 5 for Folder 3.",
            "Test Steps / Procedure": "1. Configure GIC and enable all interrupts.\n2. Select speed and interface.\n3-23. Configure MAC, MTL, DMA subsystems, preload TX descriptors, start TX, poll for completion via interrupts.",
            "Meta Impacted Registers": "Full list of 51 macros as detailed by Agent 5 for Folder 3.",
            "Impacted Registers": "MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MAC_Packet_Filter; MAC_Configuration; MAC_Ext_Configuration; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_TxQ3_Quantum_Weight; MTL_TxQ2_Quantum_Weight; MTL_TxQ1_Quantum_Weight; MTL_TxQ0_Quantum_Weight; MTL_Q3_Interrupt_Control_Status; MTL_Q2_Interrupt_Control_Status; MTL_Q1_Interrupt_Control_Status; MTL_Q0_Interrupt_Control_Status; DMA_SysBus_Mode; DMA_CH3_TxDesc_Ring_Length; DMA_CH2_TxDesc_Ring_Length; DMA_CH1_TxDesc_Ring_Length; DMA_CH0_TxDesc_Ring_Length; DMA_CH3_TxDesc_List_Address; DMA_CH2_TxDesc_List_Address; DMA_CH1_TxDesc_List_Address; DMA_CH0_TxDesc_List_Address; DMA_CH3_TxDesc_Tail_Pointer; DMA_CH2_TxDesc_Tail_Pointer; DMA_CH1_TxDesc_Tail_Pointer; DMA_CH0_TxDesc_Tail_Pointer; DMA_CH3_Control; DMA_CH2_Control; DMA_CH1_Control; DMA_CH0_Control; DMA_CH1_Interrupt_Enable; DMA_CH2_Interrupt_Enable; DMA_CH3_Interrupt_Enable; DMA_CH0_Interrupt_Enable; DMA_CH0_Tx_Control; DMA_Interrupt_Status; DMA_CH0_Status",
            "Meta Validation / Acceptance Criteria": "All transfer-complete interrupts received. IRQ handler reads/clears DMA CH0 status.",
            "Validation / Acceptance Criteria": "1. All expected transfer-complete interrupts must be received.\n2. DMA channel 0 status must be cleared successfully.\n3. Test completes with success after all transfers.",
            "Remarks": "Only DMA CH0 TX Control started. DMA CH3/2/1 TX Control writes commented out. Only DMA CH0 Status cleared in IRQ handler. VIP trigger occurs before MTL/DMA configuration."
        },
        {
            "Index": "4",
            "SS / Module": "Ethernet",
            "Test Case Name": "ethernet0_tx_rx_multi_chnl_test",
            "Feature": "Multi-Channel TX and RX DMA Transfer",
            "Meta Headers": "<stdio.h>; <stdlib.h>; <math.h>; <test_common.h>; <ethernet0/ethernet0_def.h>; <ethernet0/ethernet0_offset.h>; <ethernet0/ethernet0_funcs.h>; <ethernet0/ethernet0_programming_sequence.h>",
            "Meta Macros": "NA",
            "Meta Arrays": "NA",
            "Speed": "ETH_10M (10 Mbps); ETH_100M (100 Mbps); 1000 Mbps (default)",
            "Mode": "HALF_DUPLEX; Full Duplex (default)",
            "Memory Start Offset": "0x0",
            "Memory End Offset": "0x12E0",
            "Meta Test Description": "This testcase verifies Ethernet0 multi-channel TX and RX DMA transfer across all 4 DMA channels.",
            "Test Description": "Verify Ethernet0 multi-channel TX and RX DMA transfer across all 4 DMA channels. Channels are activated sequentially (CH3\u2192CH2\u2192CH1\u2192CH0), each time updating the RX queue-to-DMA mapping and polling for cumulative TX/RX packet counts.",
            "Meta Test Steps / Procedure": "Steps 1-162 as detailed by Agent 5 for Folder 4.",
            "Test Steps / Procedure": "1. Configure GIC and enable all interrupts.\n2-28. Configure MAC, MTL, DMA subsystems, preload TX/RX descriptors, sequentially activate channels, poll for cumulative packet counts, manage RX tail pointers in IRQ handler.",
            "Meta Impacted Registers": "Full list of 95 macros as detailed by Agent 5 for Folder 4.",
            "Impacted Registers": "MAC_Configuration; MAC_Ext_Configuration; MAC_RxQ_Ctrl0; MAC_RxQ_Ctrl1; MAC_RxQ_Ctrl2; MAC_VLAN_Tag_Ctrl; MAC_Packet_Filter; MMC_IPC_Rx_Interrupt_Mask; MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_TxQ3_Quantum_Weight; MTL_TxQ2_Quantum_Weight; MTL_TxQ1_Quantum_Weight; MTL_Operation_Mode; MTL_TxQ0_Quantum_Weight; MTL_RxQ3_Operation_Mode; MTL_RxQ2_Operation_Mode; MTL_RxQ1_Operation_Mode; MTL_RxQ0_Operation_Mode; MTL_Q3_Interrupt_Control_Status; MTL_Q2_Interrupt_Control_Status; MTL_Q1_Interrupt_Control_Status; MTL_Q0_Interrupt_Control_Status; MTL_RxQ_DMA_Map0; MTL_RxQ3_Control; MTL_RxQ2_Control; MTL_RxQ1_Control; MTL_RxQ0_Control; DMA_CH3_Tx_Control; DMA_CH2_Tx_Control; DMA_CH1_Tx_Control; DMA_CH0_Tx_Control; DMA_CH3_TxDesc_List_Address; DMA_CH2_TxDesc_List_Address; DMA_CH1_TxDesc_List_Address; DMA_CH0_TxDesc_List_Address; DMA_CH3_TxDesc_Ring_Length; DMA_CH2_TxDesc_Ring_Length; DMA_CH1_TxDesc_Ring_Length; DMA_CH3_Control; DMA_CH2_Control; DMA_CH1_Control; DMA_SysBus_Mode; DMA_CH3_Rx_Control; DMA_CH2_Rx_Control; DMA_CH1_Rx_Control; DMA_CH0_Rx_Control; DMA_CH3_RxDesc_List_Address; DMA_CH2_RxDesc_List_Address; DMA_CH1_RxDesc_List_Address; DMA_CH0_RxDesc_List_Address; DMA_CH3_Rx_Control2; DMA_CH2_Rx_Control2; DMA_CH1_Rx_Control2; DMA_CH0_TxDesc_Ring_Length; DMA_CH3_Interrupt_Enable; DMA_CH2_Interrupt_Enable; DMA_CH1_Interrupt_Enable; DMA_CH2_TxDesc_Tail_Pointer; DMA_CH3_TxDesc_Tail_Pointer; DMA_CH0_Control; DMA_CH0_Rx_Control2; DMA_CH0_Interrupt_Enable; DMA_Mode; DMA_CH3_RxDesc_Tail_Pointer; DMA_CH2_RxDesc_Tail_Pointer; DMA_CH1_RxDesc_Tail_Pointer; DMA_CH0_RxDesc_Tail_Pointer; DMA_CH1_TxDesc_Tail_Pointer; DMA_CH0_TxDesc_Tail_Pointer; Rx_Packets_Count_Good_Bad; Tx_Packet_Count_Good_Bad; DMA_CH0_Current_App_RxDesc; DMA_CH1_Current_App_RxDesc; DMA_CH2_Current_App_RxDesc; DMA_CH3_Current_App_RxDesc; DMA_Interrupt_Status; DMA_CH0_Status; DMA_CH1_Status; DMA_CH2_Status; DMA_CH3_Status",
            "Meta Validation / Acceptance Criteria": "Cumulative packet counts must reach 10/20/30/40 for each phase. IRQ handler manages RX tail pointers with wrap-around.",
            "Validation / Acceptance Criteria": "1. After each phase, cumulative TX/RX packet counts must reach expected values (10, 20, 30, 40).\n2. IRQ handler must correctly manage RX descriptor tail pointer advancement with wrap-around.\n3. All DMA channel statuses cleared successfully.\n4. Test completes with success after all 40 TX and 40 RX packets transferred.",
            "Remarks": "Channels activated in reverse order CH3\u2192CH2\u2192CH1\u2192CH0. MTL_RXQ_DMA_MAP0 written 5 times. preload_descriptor called 8 times, preload_descriptor_rx called 4 times. Complex per-channel RX tail pointer wrap-around logic in IRQ handler."
        }
    ]

    # TestPlan columns
    tp_cols = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
               "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
               "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
               "Code Generation"]

    # MetaData columns
    md_cols = ["Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
               "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
               "Meta Headers", "Meta Macros", "Meta Arrays"]

    # Create workbook
    wb = openpyxl.Workbook()

    # --- TestPlan Sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    header_font = Font(bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_align = Alignment(wrap_text=True, vertical="top")

    # Write headers
    for col_idx, col_name in enumerate(tp_cols, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    # Write data
    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(tp_cols, 1):
            val = row_data.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
            cell.alignment = wrap_align

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # Auto-size columns
    for col_idx, col_name in enumerate(tp_cols, 1):
        max_len = len(col_name)
        for row_idx in range(2, len(json_data) + 2):
            cell_val = str(ws_tp.cell(row=row_idx, column=col_idx).value or "")
            lines = cell_val.split("\n")
            for line in lines:
                if len(line) > max_len:
                    max_len = len(line)
        width = min(max_len + 2, 80)
        ws_tp.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = max(width, 12)

    # --- MetaData Sheet ---
    ws_md = wb.create_sheet("MetaData")

    for col_idx, col_name in enumerate(md_cols, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(md_cols, 1):
            val = row_data.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
            cell.alignment = wrap_align

    ws_md.freeze_panes = "A2"

    for col_idx, col_name in enumerate(md_cols, 1):
        max_len = len(col_name)
        for row_idx in range(2, len(json_data) + 2):
            cell_val = str(ws_md.cell(row=row_idx, column=col_idx).value or "")
            lines = cell_val.split("\n")
            for line in lines:
                if len(line) > max_len:
                    max_len = len(line)
        width = min(max_len + 2, 80)
        ws_md.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = max(width, 12)

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save
    wb.save(filepath)
    print(f"Workbook saved: {filepath}")
    print(f"File size: {os.path.getsize(filepath)} bytes")

    # Validate
    wb2 = openpyxl.load_workbook(filepath)
    assert "TestPlan" in wb2.sheetnames
    assert "MetaData" in wb2.sheetnames
    print(f"Validation PASSED")
    print(f"GENERATED_FILE: {filename}")
    return filepath, filename

if __name__ == "__main__":
    generate_testplan()
