#!/usr/bin/env python3
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
date_stamp = now_ist.strftime("%Y%m%d")
time_stamp = now_ist.strftime("%H%M%S")

filename = f"Ethernet0_TestPlan_{date_stamp}_{time_stamp}.xlsx"
output_dir = "Test_Output/Ethernet/Ethernet0/TestPlan"
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# ============================================================
# DATA
# ============================================================

testplan_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

metadata_columns = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Row 1
row1_tp = {
    "Index": "1",
    "SS / Module": "ETHERNET0",
    "Feature": "Register Write/Read Verification",
    "Test Case Name": "ethernet0_reg_wr_rd_test",
    "Test Description": "Verify default reset values and write/read accessibility of 434 Ethernet0 registers. The test reads each register after reset and compares against expected default values. Then it writes six different test patterns to each writable register, reads back, and verifies correctness using write and read masks. Certain registers (e.g., MAC_VLAN_Tag_Ctrl, MAC_VLAN_Tag_Data) are skipped per the skip array. A soft reset check is also performed to verify registers return to default values after reset. The test passes if no default value mismatches and no write/read mismatches are detected.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "The test covers 434 Ethernet0 registers defined in addr_array. Three registers are skipped (skip_array indices 6, 7, 9 set to 1). The test uses conditional compilation macros ETH_10M, ETH_100M, and HALF_DUPLEX for speed/duplex configuration of MAC_Configuration register. DEBUG_DISPLAY macro enables verbose pass/skip logging. The test is structured in three phases: default value check, write/read pattern check with 6 patterns, and soft reset verification.",
    "Test Steps / Procedure": """1. Initialize failure counters (def_fail_cnt, wr_fail_cnt) to zero.
2. Phase 1 - Default Value Check (chk_rst_val): Iterate over all 434 registers using addr_array. Skip registers flagged in skip_array. Read each register and compare against expected default value from default_value_array. Log pass/fail for each register. Increment def_fail_cnt on mismatch.
3. Phase 2 - Write/Read Check (chk_rd_wr): For each of 6 test patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xFFFF0000), iterate over all 434 registers. Skip registers flagged in skip_array. Skip non-writable registers (write mask = 0x00000000). Write test pattern masked with write mask to each register. Read back register value. Compare read value against expected value (test pattern AND write mask AND read mask). Increment wr_fail_cnt on mismatch.
4. Phase 3 - Soft Reset Check (soft_reset_chk): Trigger soft reset and verify registers return to default values.
5. Return overall pass/fail based on accumulated failure counts.""",
    "Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_WD_JB_Timeout, MAC_Hash_Table_Reg0, MAC_Hash_Table_Reg1, MAC_VLAN_Tag_Ctrl, MAC_VLAN_Tag_Data, MAC_VLAN_Hash_Table, MAC_Q0_Tx_Flow_Ctrl, MAC_Q1_Tx_Flow_Ctrl, MAC_Q2_Tx_Flow_Ctrl, MAC_Q3_Tx_Flow_Ctrl, MAC_Rx_Flow_Ctrl, MAC_Address0_High, MAC_Address0_Low, MAC_Address1_Low, MAC_Address2_Low, MAC_Address3_Low, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH0_RxDesc_List_Address, DMA_CH1_RxDesc_List_Address, DMA_CH2_RxDesc_List_Address, DMA_CH3_RxDesc_List_Address, MAC_RxQ_Ctrl0, MAC_RxQ_Ctrl1, MAC_RxQ_Ctrl2, MAC_RxQ_Ctrl4",
    "Validation / Acceptance Criteria": """1. All 434 Ethernet0 registers (excluding those in the skip array) must read back their expected default values after reset. No default value mismatches allowed (def_fail_cnt must be 0).
2. For each of the 6 test patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xFFFF0000), all writable registers must correctly store and read back the written value when masked with write and read masks. No write/read mismatches allowed (wr_fail_cnt must be 0).
3. After soft reset, all registers must return to their default values.
4. Registers flagged in skip_array (indices 6, 7, 9 corresponding to MAC_VLAN_Tag_Ctrl, MAC_VLAN_Tag_Data, and others) are excluded from verification.
5. Non-writable registers (write_mask = 0x00000000) are excluded from write/read verification.""",
    "Code Generation": ""
}

row1_md = {
    "Index": "1",
    "Test Case Name": "ethernet0_reg_wr_rd_test",
    "Meta Test Description": "This testcase performs register write/read verification for 434 Ethernet0 registers. It includes three phases: (1) chk_rst_val() - reads all 434 registers and compares against expected default/reset values stored in default_value_array[434], incrementing def_fail_cnt on mismatch; (2) chk_rd_wr() - iterates over 6 test patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000) writing each pattern masked with write_mask_array[i] to each register, reading back and comparing against expected value (data_wr & write_mask_array[i] & read_mask_array[i]), incrementing wr_fail_cnt on mismatch; (3) soft_reset_chk() - triggers a soft reset and re-verifies register values. Registers flagged in skip_array (value 1) are skipped. Registers with write_mask_array value of 0x00000000 are considered non-writable and skipped for writing. The test uses addr_array[434] containing mizar_ETHERNET0_* register address macros. Global variables: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0. The main entry point is int test_case() which orchestrates the three phases and returns pass/fail based on accumulated failure counts.",
    "Meta Test Steps / Procedure": """1. #include <stdio.h>
2. #include <stdlib.h>
3. #include \"test_define.c\"
4. #include <test_common.h>
5. test_define.c includes: #include <ethernet0/ethernet0_def.h>, #include <ethernet0/ethernet0_offset.h>, #define CNT 434
6. Global declarations: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0;
7. #define SOFT_RST_REG_ADDRESS 0x00000000
8. #define SOFT_RST_REG_DATA 0x00000000
9. const int addr_array[434] declared with mizar_ETHERNET0_* register address macros
10. const int default_value_array[434] declared with ETHERNET0_DEFAULT_VAL macros
11. const int write_mask_array[434] declared with ETHERNET0WRITE_MASK macros
12. const int read_mask_array[434] declared with ETHERNET0*_READ_MASK macros
13. const int skip_array[434] declared with 0/1 values (indices 6,7,9 are 1 meaning skip)
14. int test_case() entry point called
15. Phase 1 - chk_rst_val(): for(i=0; i<CNT; i++) { addr=addr_array[i]; if(skip_array[i]==1) continue; data_rd = read_reg(addr); if(data_rd == default_value_array[i]) { #ifdef DEBUG_DISPLAY printf(\"RST : PASS Reading Default value from Address :0x%x Expected : 0x%x\\tRead_data : 0x%x\\n\", addr, default_value_array[i], data_rd); #endif } else { def_fail_cnt++; printf(\"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\\tRead_data : 0x%x\\n\", addr, default_value_array[i], data_rd); } }
16. Phase 2 - chk_rd_wr(): int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}; for(j=0; j<6; j++) { data_wr = chk_val[j]; for(i=0; i<CNT; i++) { addr=addr_array[i]; if(skip_array[i]==1) { #ifdef DEBUG_DISPLAY printf(\"Read_write : Writing into this Address : 0x%x is skipped because address present in skip_array\\n\", addr); #endif continue; } if(write_mask_array[i]==0x00000000) { #ifdef DEBUG_DISPLAY printf(\"Read_write : This address 0x%x is not Writable , hence skipped for reading\\n\", addr); #endif continue; } write_reg(addr, data_wr & write_mask_array[i]); data_rd = read_reg(addr); exp_val = data_wr & write_mask_array[i] & read_mask_array[i]; if(data_rd != exp_val) { wr_fail_cnt++; } } }
17. Phase 2 reading loop: for(i=0; i<CNT; i++) { addr=addr_array[i]; if(skip_array[i]==1) { #ifdef DEBUG_DISPLAY printf(\"Read_write : Reading from this Address : 0x%x is skipped because address present in skip_array\\n\", addr); #endif continue; } if(write_mask_array[i]==0x00000000) { #ifdef DEBUG_DISPLAY printf(\"Read_write : This address 0x%x is not Writable , hence skipped for reading\\n\", addr); #endif continue; } }
18. Phase 3 - soft_reset_chk(): writes to SOFT_RST_REG_ADDRESS with SOFT_RST_REG_DATA and re-verifies register reset values
19. test_case() returns pass/fail based on def_fail_cnt and wr_fail_cnt""",
    "Meta Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_WD_JB_Timeout, MAC_Hash_Table_Reg0, MAC_Hash_Table_Reg1, MAC_VLAN_Tag_Ctrl, MAC_VLAN_Tag_Data, MAC_VLAN_Hash_Table, MAC_Q0_Tx_Flow_Ctrl, MAC_Q1_Tx_Flow_Ctrl, MAC_Q2_Tx_Flow_Ctrl, MAC_Q3_Tx_Flow_Ctrl, MAC_Rx_Flow_Ctrl, MAC_Address0_High, MAC_Address0_Low, MAC_Address1_Low, MAC_Address2_Low, MAC_Address3_Low, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH0_RxDesc_List_Address, DMA_CH1_RxDesc_List_Address, DMA_CH2_RxDesc_List_Address, DMA_CH3_RxDesc_List_Address, MAC_RxQ_Ctrl0, MAC_RxQ_Ctrl1, MAC_RxQ_Ctrl2, MAC_RxQ_Ctrl4",
    "Meta Validation / Acceptance Criteria": "Phase 1 (chk_rst_val): For each register i (0 to 433), if skip_array[i] != 1: data_rd = read_reg(addr_array[i]); if(data_rd == default_value_array[i]) then PASS, else def_fail_cnt++ and FAIL with message \"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x Read_data : 0x%x\".\nPhase 2 (chk_rd_wr): For each test pattern chk_val[j] (j=0..5) and each register i (0 to 433), if skip_array[i] != 1 and write_mask_array[i] != 0x00000000: write_reg(addr, data_wr & write_mask_array[i]); data_rd = read_reg(addr); exp_val = data_wr & write_mask_array[i] & read_mask_array[i]; if(data_rd != exp_val) then wr_fail_cnt++.\nPhase 3 (soft_reset_chk): After soft reset (write to SOFT_RST_REG_ADDRESS = 0x00000000 with SOFT_RST_REG_DATA = 0x00000000), verify registers return to default values.\nOverall pass condition: def_fail_cnt == 0 and wr_fail_cnt == 0.",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include <test_common.h>\n#include <ethernet0/ethernet0_def.h>\n#include <ethernet0/ethernet0_offset.h>",
    "Meta Macros": "#define CNT 434\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\nETHERNET0_MAC_CONFIGURATION_DEFAULT_VAL\nETHERNET0_MAC_EXT_CONFIGURATION_DEFAULT_VAL\nETHERNET0_MAC_PACKET_FILTER_DEFAULT_VAL\nETHERNET0_MAC_WD_JB_TIMEOUT_DEFAULT_VAL\nETHERNET0_MAC_HASH_TABLE_REG0_DEFAULT_VAL\nETHERNET0_MAC_HASH_TABLE_REG1_DEFAULT_VAL\nETHERNET0_MAC_VLAN_TAG_CTRL_DEFAULT_VAL\nETHERNET0_MAC_VLAN_TAG_DATA_DEFAULT_VAL\nETHERNET0_MAC_VLAN_HASH_TABLE_DEFAULT_VAL\nETHERNET0_MAC_Q0_TX_FLOW_CTRL_DEFAULT_VAL\nETHERNET0_MAC_CONFIGURATION_WRITE_MASK\nETHERNET0_MAC_EXT_CONFIGURATION_WRITE_MASK\nETHERNET0_MAC_PACKET_FILTER_WRITE_MASK\nETHERNET0_MAC_WD_JB_TIMEOUT_WRITE_MASK\nETHERNET0_MAC_HASH_TABLE_REG0_WRITE_MASK\nETHERNET0_MAC_HASH_TABLE_REG1_WRITE_MASK\nETHERNET0_MAC_VLAN_TAG_CTRL_WRITE_MASK\nETHERNET0_MAC_VLAN_TAG_DATA_WRITE_MASK\nETHERNET0_MAC_VLAN_HASH_TABLE_WRITE_MASK\nETHERNET0_MAC_Q0_TX_FLOW_CTRL_WRITE_MASK\nETHERNET0_MAC_CONFIGURATION_READ_MASK\nETHERNET0_MAC_EXT_CONFIGURATION_READ_MASK\nETHERNET0_MAC_PACKET_FILTER_READ_MASK\nETHERNET0_MAC_WD_JB_TIMEOUT_READ_MASK\nETHERNET0_MAC_HASH_TABLE_REG0_READ_MASK\nETHERNET0_MAC_HASH_TABLE_REG1_READ_MASK\nETHERNET0_MAC_VLAN_TAG_CTRL_READ_MASK\nETHERNET0_MAC_VLAN_TAG_DATA_READ_MASK\nETHERNET0_MAC_VLAN_HASH_TABLE_READ_MASK\nETHERNET0_MAC_Q0_TX_FLOW_CTRL_READ_MASK\nETHERNET0_MAC_Q1_TX_FLOW_CTRL_READ_MASK\nETHERNET0_MAC_Q2_TX_FLOW_CTRL_READ_MASK\nETHERNET0_MAC_Q3_TX_FLOW_CTRL_READ_MASK\nETHERNET0_MAC_RX_FLOW_CTRL_READ_MASK\nDEBUG_DISPLAY\nETH_10M\nETH_100M\nHALF_DUPLEX",
    "Meta Arrays": "const int addr_array[434] = {mizar_ETHERNET0_MAC_CONFIGURATION, mizar_ETHERNET0_MAC_EXT_CONFIGURATION, mizar_ETHERNET0_MAC_PACKET_FILTER, mizar_ETHERNET0_MAC_WD_JB_TIMEOUT, mizar_ETHERNET0_MAC_HASH_TABLE_REG0, mizar_ETHERNET0_MAC_HASH_TABLE_REG1, mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, mizar_ETHERNET0_MAC_VLAN_TAG_DATA, mizar_ETHERNET0_MAC_VLAN_HASH_TABLE, mizar_ETHERNET0_MAC_Q0_TX_FLOW_CTRL, mizar_ETHERNET0_MAC_Q1_TX_FLOW_CTRL, mizar_ETHERNET0_MAC_Q2_TX_FLOW_CTRL, mizar_ETHERNET0_MAC_Q3_TX_FLOW_CTRL, mizar_ETHERNET0_MAC_RX_FLOW_CTRL, ...};\nconst int default_value_array[434] = {ETHERNET0_MAC_CONFIGURATION_DEFAULT_VAL, ETHERNET0_MAC_EXT_CONFIGURATION_DEFAULT_VAL, ETHERNET0_MAC_PACKET_FILTER_DEFAULT_VAL, ETHERNET0_MAC_WD_JB_TIMEOUT_DEFAULT_VAL, ETHERNET0_MAC_HASH_TABLE_REG0_DEFAULT_VAL, ETHERNET0_MAC_HASH_TABLE_REG1_DEFAULT_VAL, ETHERNET0_MAC_VLAN_TAG_CTRL_DEFAULT_VAL, ETHERNET0_MAC_VLAN_TAG_DATA_DEFAULT_VAL, ETHERNET0_MAC_VLAN_HASH_TABLE_DEFAULT_VAL, ETHERNET0_MAC_Q0_TX_FLOW_CTRL_DEFAULT_VAL, ...};\nconst int write_mask_array[434] = {ETHERNET0_MAC_CONFIGURATION_WRITE_MASK, ETHERNET0_MAC_EXT_CONFIGURATION_WRITE_MASK, ETHERNET0_MAC_PACKET_FILTER_WRITE_MASK, ETHERNET0_MAC_WD_JB_TIMEOUT_WRITE_MASK, ETHERNET0_MAC_HASH_TABLE_REG0_WRITE_MASK, ETHERNET0_MAC_HASH_TABLE_REG1_WRITE_MASK, ETHERNET0_MAC_VLAN_TAG_CTRL_WRITE_MASK, ETHERNET0_MAC_VLAN_TAG_DATA_WRITE_MASK, ETHERNET0_MAC_VLAN_HASH_TABLE_WRITE_MASK, ETHERNET0_MAC_Q0_TX_FLOW_CTRL_WRITE_MASK, ...};\nconst int read_mask_array[434] = {ETHERNET0_MAC_CONFIGURATION_READ_MASK, ETHERNET0_MAC_EXT_CONFIGURATION_READ_MASK, ETHERNET0_MAC_PACKET_FILTER_READ_MASK, ETHERNET0_MAC_WD_JB_TIMEOUT_READ_MASK, ETHERNET0_MAC_HASH_TABLE_REG0_READ_MASK, ETHERNET0_MAC_HASH_TABLE_REG1_READ_MASK, ETHERNET0_MAC_VLAN_TAG_CTRL_READ_MASK, ETHERNET0_MAC_VLAN_TAG_DATA_READ_MASK, ETHERNET0_MAC_VLAN_HASH_TABLE_READ_MASK, ETHERNET0_MAC_Q0_TX_FLOW_CTRL_READ_MASK, ETHERNET0_MAC_Q1_TX_FLOW_CTRL_READ_MASK, ETHERNET0_MAC_Q2_TX_FLOW_CTRL_READ_MASK, ...};\nconst int skip_array[434] = {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, ...};\nint chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};"
}

# Row 2
row2_tp = {
    "Index": "2",
    "SS / Module": "ETHERNET0",
    "Feature": "Basic TX Transmission",
    "Test Case Name": "ethernet0_tx_basic_test",
    "Test Description": "Perform a basic Ethernet0 transmit operation using DMA Channel 0. The test configures the MAC layer (speed, duplex, MAC address), MTL layer (TX queue operation mode with 4096-byte queue and store-and-forward), and DMA layer (descriptor list addresses, tail pointers, channel control). Only DMA Channel 0 TX is actively started; Channels 1-3 TX control writes are commented out. The test transmits packets and polls for transfer completion interrupts, verifying successful transmission.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Only DMA Channel 0 TX is actively started; DMA Channels 1-3 TX_CONTROL writes are commented out with value 0x00100007. The test uses conditional compilation for speed selection (ETH_10M: 10Mbps, ETH_100M: 100Mbps, default: 1Gbps) and duplex mode (HALF_DUPLEX or Full Duplex). Interface selection uses SEL_ENET0-SEL_ENET3 macros with enet_intf_sel() function. VIP sequencer is triggered via write_reg to addresses 0xA0243ffc and 0xA0243ff8. MTL_TxQ1_Operation_Mode, MTL_TxQ2_Operation_Mode, MTL_TxQ3_Operation_Mode, DMA_CH1_Tx_Control, DMA_CH2_Tx_Control, and DMA_CH3_Tx_Control are present in Agent 4 mappings but not actively written in the source (CH1-CH3 TX_CONTROL writes are commented out, MTL TxQ1-Q3 writes not visible in retrieved source).",
    "Test Steps / Procedure": """1. Select Ethernet interface using enet_intf_sel() based on SEL_ENETx compile-time macro (default channel 4).
2. If ETH_10M mode, set trns_count to 6.
3. Configure MAC_Configuration register with appropriate speed and duplex settings (10M/100M/1G, Half/Full Duplex).
4. Write MAC_Ext_Configuration register with 0x0.
5. Program MAC_Address0_High with 0x80000607 (address enable + upper MAC bytes) and MAC_Address0_Low with 0x08090a0b (lower MAC bytes). Resulting MAC address: 06:07:08:09:0A:0B.
6. Initialize DMA channel control registers (DMA_CH0_Control through DMA_CH3_Control) to 0x0.
7. Configure MTL_TxQ0_Operation_Mode with 4096-byte TX queue size, store-and-forward enabled, TX Q0 enabled.
8. Set DMA TX descriptor list base addresses for all 4 channels (CH0=0xE6000000, CH1=0x00100000, CH2=0x00200000, CH3=0x00300000).
9. Write DMA TX descriptor tail pointers for all 4 channels (CH0=0xE6009EB4, CH1=0x00109730, CH2=0x00209730, CH3=0x00309730).
10. Start TX on DMA Channel 0 only by writing DMA_CH0_Tx_Control with 0x00100007 (CH1-CH3 TX start is commented out).
11. Trigger ETH VIP sequencer start.
12. Poll for transfer completion interrupts (int_pend) in a loop for trns_count iterations, calling wait_on(10) between checks.
13. Determine pass/fail based on failure counters.""",
    "Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Address0_High, DMA_CH0_Control, DMA_CH1_Control, DMA_CH2_Control, DMA_CH3_Control, DMA_CH0_Tx_Control, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH0_TxDesc_Tail_Pointer, DMA_CH1_TxDesc_Tail_Pointer, DMA_CH2_TxDesc_Tail_Pointer, DMA_CH3_TxDesc_Tail_Pointer, MTL_TxQ0_Operation_Mode",
    "Validation / Acceptance Criteria": """1. All trns_count (10 default, 6 for 10M mode) transmit transfer interrupts must be received and acknowledged via the int_pend flag.
2. No default value check failures (def_fail_cnt must be 0).
3. No write/read verification failures (wr_fail_cnt must be 0).
4. DMA Channel 0 must successfully complete TX operations as indicated by interrupt completion.
5. Test passes (finish(0)) only if all failure counters remain at zero.""",
    "Code Generation": ""
}

row2_md = {
    "Index": "2",
    "Test Case Name": "ethernet0_tx_basic_test",
    "Meta Test Description": "This testcase performs a basic Ethernet0 TX (transmit) operation on DMA Channel 0. It initializes the Ethernet interface via enet_intf_sel() based on conditional compilation macros SEL_ENET0/SEL_ENET1/SEL_ENET2/SEL_ENET3 (default channel 4 if none defined). It configures the MAC_CONFIGURATION register with speed and duplex settings based on ETH_10M, ETH_100M, and HALF_DUPLEX macros: ETH_10M+HALF_DUPLEX=0x8003, ETH_10M+Full=0xA003, ETH_100M+HALF_DUPLEX=0xC003, ETH_100M+Full=0xE003, default(1G)+HALF_DUPLEX=0x0003, default(1G)+Full=0x2003. When ETH_10M is defined, trns_count is set to 6. MAC_EXT_CONFIGURATION is written with 0x0. MAC_ADDRESS0_HIGH is written with 0x80000607 and MAC_ADDRESS0_LOW with 0x08090a0b. DMA channel control registers (CH0-CH3) are initialized to 0x0/0x00000000. MTL_TXQ0_OPERATION_MODE is written with 0x000f000a (4096 Bytes TX QUEUE Size, Store & Forward Enabled, TX Q0 Enabled). DMA TX descriptor list addresses are configured: CH0=0xE6000000, CH1=0x00100000, CH2=0x00200000, CH3=0x00300000. DMA TX descriptor tail pointers are written: CH3=0x00309730, CH2=0x00209730, CH1=0x00109730, CH0=0xE6009EB4. Only DMA_CH0_TX_CONTROL is actively written with 0x00100007 (START TX); CH1, CH2, CH3 TX_CONTROL writes (0x00100007) are commented out. After starting TX, the code loops trns_count times polling int_pend with wait_on(10) for transfer interrupt completion. VIP sequencer is triggered via write_reg(0xA0243ffc, enet_sel) and write_reg(0xA0243ff8, 0xdeadbeef). Global variables: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0; unsigned int trns_count = 10; extern int int_pend; Functions: int test_case(), void Default_IRQHandler().",
    "Meta Test Steps / Procedure": """1. #include <stdio.h>
2. #include <stdlib.h>
3. #include <math.h>
4. #include <ethernet0/ethernet0_def.h>
5. #include <test_common.h>
6. #include <ethernet0/ethernet0_offset.h>
7. #include <ethernet0/ethernet0_funcs.h>
8. #include <ethernet0/ethernet0_programming_sequence.h>
9. Global declarations: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0; unsigned int trns_count = 10; extern int int_pend;
10. void Default_IRQHandler() declared
11. int test_case() entry point called
12. Ethernet interface selection: #ifdef SEL_ENET0 enet_intf_sel(0); #elif SEL_ENET1 enet_intf_sel(1); #elif SEL_ENET2 enet_intf_sel(2); #elif SEL_ENET3 enet_intf_sel(3); #else enet_intf_sel(4); #endif
13. #ifdef ETH_10M trns_count = 6; #endif
14. MAC Configuration: #ifdef ETH_10M #ifdef HALF_DUPLEX write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x8003); #else write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xA003); #endif #elif ETH_100M #ifdef HALF_DUPLEX write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xC003); #else write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xE003); #endif #else #ifdef HALF_DUPLEX write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x0003); #else write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x2003); #endif #endif
15. write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);
16. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x80000607);
17. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x08090a0b);
18. write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);
19. write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);
20. write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);
21. write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);
22. write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x000f000a); //4096 Bytes TX QUEUE Size, STR&FWD Enabled, TX Q0 Enabled
23. write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00300000);
24. write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00200000);
25. write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00100000);
26. write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);
27. write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0x00309730);
28. write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0x00209730);
29. write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0x00109730);
30. write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4);
31. //write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100007); //START TX (commented out)
32. //write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007); //START TX (commented out)
33. //write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007); //START TX (commented out)
34. write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100007); //START TX
35. write_reg(0xA0243ffc, enet_sel); //For ETH VIP sequencer start
36. write_reg(0xA0243ff8, 0xdeadbeef); //For ETH VIP sequencer start
37. for(i = 0; i < trns_count; i++) { while(int_pend) { #ifdef DEBUG_DISPLAY printf(\"---->%d trnsf intrpt\\n\", i); #endif wait_on(10); } }
38. Pass/fail determination based on def_fail_cnt and wr_fail_cnt""",
    "Meta Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Address0_High, DMA_CH0_Control, DMA_CH1_Control, DMA_CH2_Control, DMA_CH3_Control, DMA_CH0_Tx_Control, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH0_TxDesc_Tail_Pointer, DMA_CH1_TxDesc_Tail_Pointer, DMA_CH2_TxDesc_Tail_Pointer, DMA_CH3_TxDesc_Tail_Pointer, MTL_TxQ0_Operation_Mode",
    "Meta Validation / Acceptance Criteria": "Transfer completion is validated by polling extern int int_pend in a for loop iterating trns_count times (default 10, or 6 when ETH_10M defined). Each iteration executes while(int_pend) { wait_on(10); } waiting for the interrupt handler (Default_IRQHandler) to clear int_pend. Pass condition: def_fail_cnt == 0 and wr_fail_cnt == 0. If either counter is non-zero, the test fails. The test calls finish(0) for pass or finish(1) for fail.",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include <math.h>\n#include <ethernet0/ethernet0_def.h>\n#include <test_common.h>\n#include <ethernet0/ethernet0_offset.h>\n#include <ethernet0/ethernet0_funcs.h>\n#include <ethernet0/ethernet0_programming_sequence.h>",
    "Meta Macros": "ETH_10M\nETH_100M\nHALF_DUPLEX\nSEL_ENET0\nSEL_ENET1\nSEL_ENET2\nSEL_ENET3\nDEBUG_DISPLAY",
    "Meta Arrays": "NA"
}

# Row 3
row3_tp = {
    "Index": "3",
    "SS / Module": "ETHERNET0",
    "Feature": "Basic RX Reception",
    "Test Case Name": "ethernet0_rx_basic_test",
    "Test Description": "Perform a basic Ethernet0 receive operation using DMA Channel 0. The test configures the MAC layer (speed, duplex, MAC addresses for 4 address slots with DMA channel selection, VLAN tag control, packet filter with hash filter and receive-all enabled, RX queue control with DCB/Generic mode and queue priorities), MTL layer (TX queue operation mode with 4096-byte queue and store-and-forward, RX queue to DMA channel mapping, queue interrupt control for overflow/underflow), and DMA layer (descriptor list addresses for TX and RX, channel control, RX control with buffer size, interrupt enables). The test triggers the VIP sequencer and uses an interrupt handler (Default_IRQHandler) to monitor RX descriptor and buffer status across all 4 DMA channels, with optional descriptor preloading when PRELOAD_AGAIN is defined.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "The test configures 4 MAC address slots with different DMA channel selections (MAC_ADDRESS0_HIGH=0x80000607 for CH0, MAC_ADDRESS1_HIGH=0x80011607 for CH1, MAC_ADDRESS2_HIGH=0x80022607 for CH2, MAC_ADDRESS3_HIGH=0x80033607 for CH3). MAC_VLAN_Tag_Ctrl is configured with 0x01600000 for VLAN tag stripping. MAC_Packet_Filter uses 0x80000400 (hash filter + receive all). RX queue priorities are set via MAC_RxQ_Ctrl2=0x08040201. MTL RX queue to DMA channel mapping is 1:1 via MTL_RxQ_DMA_Map0=0x03020100. DMA CH0 interrupt enable uses 0xf0df while CH1-CH3 use 0x000F9C0. The interrupt handler includes PRELOAD_AGAIN conditional logic to reload RX descriptors when the tail pointer reaches the end of the ring (0xE60400f0). Power configuration is written to address 0xE68C2058. The test uses conditional compilation for speed selection (ETH_10M, ETH_100M, default 1G) and duplex mode (HALF_DUPLEX or Full Duplex).",
    "Test Steps / Procedure": """1. Set trns_count to 6 if ETH_10M mode is defined.
2. Select Ethernet interface using enet_intf_sel() based on SEL_ENETx compile-time macro (default channel 4).
3. Configure MAC_Configuration register with appropriate speed and duplex settings (10M/100M/1G, Half/Full Duplex).
4. Write MAC_Ext_Configuration register with 0x0.
5. Configure MAC_VLAN_Tag_Ctrl with 0x01600000 to enable VLAN TAG in status and VLAN TAG RX stripping.
6. Configure MAC_Packet_Filter with 0x80000400 to enable hash filter and receive all packets.
7. Program MAC_RxQ_Ctrl0 with 0x00aa (RX queues enabled for DCB/Generic), MAC_RxQ_Ctrl1 with 0x000, MAC_RxQ_Ctrl2 with 0x08040201 (RX queue priorities), MAC_RxQ_Ctrl4 with 0x0.
8. Program MAC address HIGH registers for 4 address slots with DMA channel selection and address enable: MAC_ADDRESS3_HIGH=0x80033607, MAC_ADDRESS2_HIGH=0x80022607, MAC_ADDRESS1_HIGH=0x80011607, MAC_ADDRESS0_HIGH=0x80000607.
9. Program MAC address LOW registers (MAC_Address0_Low, MAC_Address1_Low, MAC_Address2_Low, MAC_Address3_Low) with lower 32 bits of MAC addresses.
10. Configure MTL_TxQ0_Operation_Mode with 0x000f000a (4096-byte TX queue, store-and-forward, Q0 enabled).
11. Configure MTL_RxQ_DMA_Map0 with 0x03020100 to map each RX queue to its corresponding DMA channel.
12. Enable RX queue overflow and TX queue underflow interrupts for MTL_Q1_Interrupt_Control_Status, MTL_Q2_Interrupt_Control_Status, MTL_Q3_Interrupt_Control_Status with 0x01000100.
13. Initialize DMA_CH0_Control to 0x00000000.
14. Set DMA TX descriptor list base addresses: CH0=0xE6000000, CH1=0x00100000, CH2=0x00200000, CH3=0x00300000.
15. Enable DMA channel interrupts: CH1, CH2, CH3 with 0x000F9C0; CH0 with 0xf0df.
16. Configure DMA_CH0_RX_CONTROL with 0x00101000, DMA_CH0_RX_CONTROL2 with 0x9 (descriptor ring length).
17. Set DMA_CH0_RXDESC_LIST_ADDRESS to 0xE6040000.
18. Write power configuration register.
19. Trigger ETH VIP sequencer start.
20. Poll for transfer completion interrupts via int_pend.
21. In Default_IRQHandler: read current app RX descriptor and RX buffer for all 4 DMA channels, read DMA_Interrupt_Status, read DMA_CH0_RxDesc_Tail_Pointer and compare; if tail pointer reaches 0xE60400f0 and PRELOAD_AGAIN defined, call preload_descriptor_rx().""",
    "Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_VLAN_Tag_Ctrl, MAC_Address0_Low, MAC_Address1_Low, MAC_Address2_Low, MAC_Address3_Low, MAC_RxQ_Ctrl0, MAC_RxQ_Ctrl1, MAC_RxQ_Ctrl2, MAC_RxQ_Ctrl4, MTL_TxQ0_Operation_Mode, MTL_RxQ_DMA_Map0, MTL_Q1_Interrupt_Control_Status, MTL_Q2_Interrupt_Control_Status, MTL_Q3_Interrupt_Control_Status, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH1_Interrupt_Enable, DMA_CH2_Interrupt_Enable, DMA_CH3_Interrupt_Enable, DMA_CH0_RxDesc_Tail_Pointer, DMA_CH3_Current_App_RxDesc, DMA_CH3_Current_App_RxBuffer, DMA_Interrupt_Status",
    "Validation / Acceptance Criteria": """1. All transfer completion interrupts must be received and acknowledged via the int_pend flag for trns_count iterations (10 default, 6 for 10M mode).
2. No default value check failures (def_fail_cnt must be 0).
3. No write/read verification failures (wr_fail_cnt must be 0).
4. DMA_CH0_RxDesc_Tail_Pointer is monitored in the interrupt handler; when it reaches 0xE60400f0 (end of descriptor ring), RX descriptors are optionally preloaded if PRELOAD_AGAIN is defined.
5. DMA_Interrupt_Status is read in the interrupt handler to verify interrupt source.
6. Current application RX descriptor and RX buffer addresses are read for all 4 DMA channels to confirm RX progress.""",
    "Code Generation": ""
}

row3_md = {
    "Index": "3",
    "Test Case Name": "ethernet0_rx_basic_test",
    "Meta Test Description": "This testcase performs a basic Ethernet0 RX (receive) operation. It initializes the Ethernet interface via enet_intf_sel() based on conditional compilation macros SEL_ENET0/SEL_ENET1/SEL_ENET2/SEL_ENET3 (default channel 4 if none defined). When ETH_10M is defined, trns_count is set to 6. It configures the MAC_CONFIGURATION register with speed and duplex settings based on ETH_10M, ETH_100M, and HALF_DUPLEX macros: ETH_10M+HALF_DUPLEX=0x8003, ETH_10M+Full=0xA003, ETH_100M+HALF_DUPLEX=0xC003, ETH_100M+Full=0xE003, default(1G)+HALF_DUPLEX=0x0003, default(1G)+Full=0x2003. MAC_EXT_CONFIGURATION is written with 0x0. MAC_VLAN_TAG_CTRL is written with 0x01600000 (Enabling VLAN TAG in status and VLAN TAG RX Stripping Enabled). MAC_PACKET_FILTER is written with 0x80000400 (HASH FILTER Enabled and Receive All packets regardless of filter). MAC_RXQ_CTRL0 is written with 0x00aa (Queue enabled for DCB/Generic). MAC_RXQ_CTRL1 is written with 0x000. MAC_RXQ_CTRL2 is written with 0x08040201 (Priorities for RX Queues). MAC_RXQ_CTRL4 is written with 0x0 (Enabling/Disabling Queues for UDC, VFFQ, MFFW, UFFQ). MAC_ADDRESS0_HIGH is written with 0x80000607, MAC_ADDRESS1_HIGH with 0x80011607, MAC_ADDRESS2_HIGH with 0x80022607, MAC_ADDRESS3_HIGH with 0x80033607 (DMA Channel selection, Address Enabled, upper 16 bits of MAC addresses). MAC_ADDRESS0_LOW, MAC_ADDRESS1_LOW, MAC_ADDRESS2_LOW, MAC_ADDRESS3_LOW are written with lower 32 bits of MAC addresses. MTL_TXQ0_OPERATION_MODE is written with 0x000f000a (4096 Bytes TX QUEUE Size, Store & Forward Enabled, TX Q0 Enabled). MTL_RXQ_DMA_MAP0 is written with 0x03020100 (mapping Queue X to DMA Channel X). MTL_Q3_INTERRUPT_CONTROL_STATUS, MTL_Q2_INTERRUPT_CONTROL_STATUS, MTL_Q1_INTERRUPT_CONTROL_STATUS are each written with 0x01000100 (RX Queue Overflow and TX Queue Underflow Interrupt Enable). DMA_CH0_CONTROL is written with 0x00000000. DMA_CH0_TXDESC_LIST_ADDRESS is written with 0xE6000000. DMA_CH3_TXDESC_LIST_ADDRESS with 0x00300000, DMA_CH2_TXDESC_LIST_ADDRESS with 0x00200000, DMA_CH1_TXDESC_LIST_ADDRESS with 0x00100000. DMA_CH1_INTERRUPT_ENABLE, DMA_CH2_INTERRUPT_ENABLE, DMA_CH3_INTERRUPT_ENABLE are each written with 0x000F9C0. DMA_CH0_INTERRUPT_ENABLE is written with 0xf0df. DMA_CH0_RX_CONTROL is written with 0x00101000. DMA_CH0_RX_CONTROL2 is written with 0x9 (Receiver Descriptor Ring Length). DMA_CH0_RXDESC_LIST_ADDRESS is written with 0xE6040000. Power register write_reg(0xE68C2058, power). VIP sequencer triggered via write_reg(0xA0243ffc, enet_sel) and write_reg(0xA0243ff8, 0xdeadbeef). The Default_IRQHandler reads DMA_CH0-CH3_CURRENT_APP_RXDESC, DMA_CH0-CH3_CURRENT_APP_RXBUFFER, and DMA_INTERRUPT_STATUS. It also reads DMA_CH0_RXDESC_TAIL_POINTER and compares against rdata_desc; if rdata_tailpointer == 0xE60400f0 and PRELOAD_AGAIN is defined, calls preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0x10). Global variables: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0; unsigned int trns_count = 10; extern int int_pend.",
    "Meta Test Steps / Procedure": """1. #include <stdio.h>
2. #include <stdlib.h>
3. #include <math.h>
4. #include <ethernet0/ethernet0_def.h>
5. #include <test_common.h>
6. #include <ethernet0/ethernet0_offset.h>
7. #include <ethernet0/ethernet0_funcs.h>
8. #include <ethernet0/ethernet0_programming_sequence.h>
9. Global declarations: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0; unsigned int trns_count = 10; extern int int_pend;
10. void Default_IRQHandler() declared
11. int test_case() entry point called
12. #ifdef ETH_10M trns_count = 6; #endif
13. Ethernet interface selection: #ifdef SEL_ENET0 enet_intf_sel(0); #elif SEL_ENET1 enet_intf_sel(1); #elif SEL_ENET2 enet_intf_sel(2); #elif SEL_ENET3 enet_intf_sel(3); #else enet_intf_sel(4); #endif
14. MAC Configuration with speed/duplex settings
15. write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);
16. write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x01600000);
17. write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000400);
18. write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x00aa);
19. write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x000);
20. write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x08040201);
21. write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL4, 0x0);
22. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x80033607);
23. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x80022607);
24. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x80011607);
25. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x80000607);
26-29. MAC address LOW registers written
30. write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x000f000a);
31. write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x03020100);
32-34. MTL Queue Interrupt Control Status registers written with 0x01000100
35. write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);
36-39. DMA TX descriptor list addresses configured
40-42. DMA CH1-CH3 Interrupt Enable written with 0x000F9C0
43. write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);
44. write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x9);
45. write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);
46. write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0xf0df);
47. write_reg(0xE68C2058, power);
48-49. VIP sequencer triggered
50. Polling/wait loop for transfer completion
51. Default_IRQHandler reads DMA channel status registers and handles descriptor preloading""",
    "Meta Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_VLAN_Tag_Ctrl, MAC_Address0_Low, MAC_Address1_Low, MAC_Address2_Low, MAC_Address3_Low, MAC_RxQ_Ctrl0, MAC_RxQ_Ctrl1, MAC_RxQ_Ctrl2, MAC_RxQ_Ctrl4, MTL_TxQ0_Operation_Mode, MTL_RxQ_DMA_Map0, MTL_Q1_Interrupt_Control_Status, MTL_Q2_Interrupt_Control_Status, MTL_Q3_Interrupt_Control_Status, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH1_Interrupt_Enable, DMA_CH2_Interrupt_Enable, DMA_CH3_Interrupt_Enable, DMA_CH0_RxDesc_Tail_Pointer, DMA_CH3_Current_App_RxDesc, DMA_CH3_Current_App_RxBuffer, DMA_Interrupt_Status",
    "Meta Validation / Acceptance Criteria": "In Default_IRQHandler: rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER); if(rdata_tailpointer == rdata_desc) { if(rdata_tailpointer == 0xE60400f0) { #ifdef PRELOAD_AGAIN preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0x10); #endif } }. DMA_INTERRUPT_STATUS is read to determine interrupt state. Current app RX descriptor and RX buffer addresses are read for all 4 DMA channels (CH0-CH3) to monitor RX progress. Pass condition: def_fail_cnt == 0 and wr_fail_cnt == 0. Transfer completion is validated by polling extern int int_pend in a loop iterating trns_count times (default 10, or 6 when ETH_10M defined).",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include <math.h>\n#include <ethernet0/ethernet0_def.h>\n#include <test_common.h>\n#include <ethernet0/ethernet0_offset.h>\n#include <ethernet0/ethernet0_funcs.h>\n#include <ethernet0/ethernet0_programming_sequence.h>",
    "Meta Macros": "ETH_10M\nETH_100M\nHALF_DUPLEX\nDEBUG_DISPLAY\nPRELOAD_AGAIN\nSEL_ENET0\nSEL_ENET1\nSEL_ENET2\nSEL_ENET3",
    "Meta Arrays": "NA"
}

# Row 4
row4_tp = {
    "Index": "4",
    "SS / Module": "ETHERNET0",
    "Feature": "Multi-Channel TX/RX Transmission and Reception",
    "Test Case Name": "ethernet0_tx_rx_multi_chnl_test",
    "Test Description": "Perform multi-channel Ethernet0 TX and RX operations across all 4 DMA channels (CH0-CH3) sequentially. The test first configures the MAC layer (speed, duplex, MAC addresses with per-channel DMA selection, VLAN tag control, packet filter with hash filter and receive-all), MTL layer (TX queue operation modes for Q0-Q2, RX queue operation modes for Q0-Q3, quantum weights for Q1-Q3, RX queue to DMA channel mapping, queue interrupt control), and DMA layer (channel control, TX/RX control for all 4 channels, descriptor list addresses, tail pointers, interrupt enables, DMA mode). After initial bulk configuration, the test activates each channel sequentially: Channel 0 runs first until 20 TX and 20 RX packets complete, then Channel 1 is activated with updated TX control, RXQ DMA mapping, RX descriptor tail pointer, and RX control until 20 TX/20 RX packets complete, then Channel 2 is activated similarly until 10 TX/10 RX packets complete, then Channel 3 is activated. Each channel activation triggers the VIP sequencer with a unique magic value.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "The test uses sequential per-channel activation: Channel 0 runs first, then Channel 1, then Channel 2, then Channel 3. Each channel activation updates DMA TX control to 0x00100007 (start TX), updates MTL_RxQ_DMA_Map0 with per-channel mapping (CH0: initial 0x00000083, CH1: 0x00000081, CH2: 0x00000082, CH3: 0x00000083), updates RX descriptor tail pointer, and sets RX control to 0x00101001. Each channel uses a unique VIP sequencer magic value (CH0: 0xdeadbeef, CH1: 0xdeadbeed, CH2: 0xdeadbeee). Channel 0 and Channel 1 require 20 TX/20 RX packets; Channel 2 requires 10 TX/10 RX packets. Initial bulk TX control for all channels is 0x00100006; per-channel activation overwrites to 0x00100007. Initial bulk RX control for all channels is 0x00101000; per-channel activation overwrites to 0x00101001. The default 1G speed configuration writes 0x00002003 to MAC_Configuration (no separate HALF_DUPLEX branch visible in retrieved source for 1G). Override is enabled via write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08) followed by wait_on(300). MAC_Packet_Filter uses 0x80000408 (differs from basic RX test 0x80000400). MTL TX queue quantum weights assign higher weight (0x14=20) to TxQ3 and TxQ2, lower weight (0x05=5) to TxQ1. The interrupt handler updates DMA_CH2_TxDesc_Tail_Pointer=0xE60298a4 and DMA_CH3_TxDesc_Tail_Pointer=0xE60398a4 via write_reg during interrupt servicing.",
    "Test Steps / Procedure": """1. Select Ethernet interface using enet_intf_sel() based on SEL_ENETx compile-time macro (default channel 4).
2. If ETH_10M mode, set trns_count to 6.
3. Configure MAC_Configuration register with appropriate speed and duplex settings (10M/100M/1G, Half/Full Duplex). Default 1G Full Duplex uses 0x00002003.
4. Enable override by writing 0x1A08 to address (0xE68C205C + (enet_sel * 4)), then wait_on(300).
5. Write MAC_Ext_Configuration with 0x0.
6. Program MAC_RxQ_Ctrl0=0x000000aa, MAC_RxQ_Ctrl1=0x00000000, MAC_RxQ_Ctrl2=0x08040201.
7. Configure MAC_Packet_Filter with 0x80000408 (hash filter + receive all).
8. Configure MAC_VLAN_Tag_Ctrl with 0x01600000 (VLAN TAG in status, RX stripping enabled).
9. Program MAC address HIGH registers for 4 address slots with DMA channel selection: MAC_Address3_High=0x80033607, MAC_Address2_High=0x80022607, MAC_Address1_High=0x80011607, MAC_Address0_High=0x80000607.
10. Program MAC address LOW registers: MAC_Address2_Low=0x08090a0b, MAC_Address1_Low=0x08090a0b, MAC_Address0_Low=0x08090a0b.
11. Configure MTL_Operation_Mode=0x00000000.
12. Configure MTL TX queue operation modes: MTL_TxQ2_Operation_Mode=0x0003000a, MTL_TxQ1_Operation_Mode=0x0003000a, MTL_TxQ0_Operation_Mode=0x0003000a.
13. Configure MTL RX queue operation modes: MTL_RxQ3_Operation_Mode=0x00f00033, MTL_RxQ2_Operation_Mode=0x00f00033, MTL_RxQ1_Operation_Mode=0x00f00033, MTL_RxQ0_Operation_Mode=0x00f00033.
14. Program MTL TX queue quantum weights: MTL_TxQ3_Quantum_Weight=0x00000014, MTL_TxQ2_Quantum_Weight=0x00000014, MTL_TxQ1_Quantum_Weight=0x00000005.
15. Set initial MTL_RxQ_DMA_Map0=0x00000083.
16. Enable MTL queue interrupts: MTL_Q3_Interrupt_Control_Status=0x01000100, MTL_Q2_Interrupt_Control_Status=0x01000100, MTL_Q1_Interrupt_Control_Status=0x01000100.
17. Initialize DMA channel control registers (DMA_CH0_Control through DMA_CH3_Control) to 0x0.
18. Bulk configure DMA TX control for all 4 channels with 0x00100006.
19. Bulk configure DMA RX control for all 4 channels with 0x00101000.
20. Configure DMA_CH0_Rx_Control2=0x9, DMA_CH0_Interrupt_Enable=0xf0df, DMA_Mode=0x00000000.
21. Set DMA TX descriptor list addresses: CH0=0xE6000000, CH1=0xE6010000, CH2=0xE6020000, CH3=0xE6030000.
22. Set DMA RX descriptor list addresses: CH0=0xE6040000, CH1=0xE6050000, CH2=0xE6060000, CH3=0xE6070000.
23. Set DMA TX descriptor tail pointers: CH0=0xE6009EB4, CH1=0xE6019EB4, CH2=0xE6029EB4, CH3=0xE6039EB4.
24. Set DMA RX descriptor tail pointers: CH0=0xE6040010, CH1=0xE6050010, CH2=0xE6060010, CH3=0xE6070010.
25. Trigger VIP sequencer: write enet_sel to 0xA0243ffc, write 0xdeadbeef to 0xA0243ff8.
26. Channel 0: Loop up to 20 iterations polling int_pend; break when rxpkt==20 && txpkt==20.
27. Channel 1 activation: Write DMA_CH1_Tx_Control=0x00100007, MTL_RxQ_DMA_Map0=0x00000081, DMA_CH1_RxDesc_Tail_Pointer=0xE6050010 (update), DMA_CH1_Rx_Control=0x00101001. Trigger VIP with 0xdeadbeed. Loop up to 20 iterations; break when rxpkt==20 && txpkt==20.
28. Channel 2 activation: Write DMA_CH2_Tx_Control=0x00100007, MTL_RxQ_DMA_Map0=0x00000082, DMA_CH2_RxDesc_Tail_Pointer=0xE6060010 (update), DMA_CH2_Rx_Control=0x00101001. Trigger VIP with 0xdeadbeee. Loop up to 20 iterations; break when rxpkt==10 && txpkt==10.
29. Channel 3 activation: Write MTL_RxQ_DMA_Map0=0x00000083, DMA_CH3_RxDesc_Tail_Pointer=0xE6070010 (update), DMA_CH3_Rx_Control=0x00101001.
30. Determine pass/fail based on def_fail_cnt and wr_fail_cnt.""",
    "Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_VLAN_Tag_Ctrl, MAC_Address0_High, MAC_Address1_High, MAC_Address2_High, MAC_Address3_High, MAC_Address0_Low, MAC_Address1_Low, MAC_Address2_Low, MAC_Address3_Low, MAC_RxQ_Ctrl0, MAC_RxQ_Ctrl1, MAC_RxQ_Ctrl2, DMA_Mode, DMA_CH0_Control, DMA_CH1_Control, DMA_CH2_Control, DMA_CH3_Control, DMA_CH0_Tx_Control, DMA_CH1_Tx_Control, DMA_CH2_Tx_Control, DMA_CH3_Tx_Control, DMA_CH0_Rx_Control, DMA_CH1_Rx_Control, DMA_CH2_Rx_Control, DMA_CH3_Rx_Control, DMA_CH0_Rx_Control2, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH0_RxDesc_List_Address, DMA_CH1_RxDesc_List_Address, DMA_CH2_RxDesc_List_Address, DMA_CH3_RxDesc_List_Address, DMA_CH0_TxDesc_Tail_Pointer, DMA_CH1_TxDesc_Tail_Pointer, DMA_CH2_TxDesc_Tail_Pointer, DMA_CH3_TxDesc_Tail_Pointer, DMA_CH0_RxDesc_Tail_Pointer, DMA_CH0_Interrupt_Enable, DMA_CH1_Interrupt_Enable, DMA_CH2_Interrupt_Enable, DMA_CH3_Interrupt_Enable, DMA_CH0_Current_App_RxDesc, DMA_CH3_Current_App_TxDesc, DMA_CH3_Current_App_TxBuffer, MTL_Operation_Mode, MTL_TxQ0_Operation_Mode, MTL_TxQ1_Operation_Mode, MTL_TxQ2_Operation_Mode, MTL_RxQ0_Operation_Mode, MTL_RxQ1_Operation_Mode, MTL_RxQ2_Operation_Mode, MTL_RxQ3_Operation_Mode, MTL_TxQ1_Quantum_Weight, MTL_TxQ2_Quantum_Weight, MTL_TxQ3_Quantum_Weight, MTL_RxQ_DMA_Map0, MTL_Q1_Interrupt_Control_Status, MTL_Q2_Interrupt_Control_Status, MTL_Q3_Interrupt_Control_Status",
    "Validation / Acceptance Criteria": """1. Channel 0 must complete 20 TX and 20 RX packets (rxpkt==20 && txpkt==20) within 20 interrupt-driven iterations.
2. Channel 1 must complete 20 TX and 20 RX packets (rxpkt==20 && txpkt==20) within 20 interrupt-driven iterations.
3. Channel 2 must complete 10 TX and 10 RX packets (rxpkt==10 && txpkt==10) within 20 interrupt-driven iterations.
4. Channel 3 RX is activated and must process packets as configured.
5. The interrupt handler (Default_IRQHandler) must correctly clear int_pend, read DMA_CH0_Current_App_RxDesc, DMA_CH3_Current_App_TxDesc, DMA_CH3_Current_App_TxBuffer, and update DMA_CH2_TxDesc_Tail_Pointer and DMA_CH3_TxDesc_Tail_Pointer.
6. No default value check failures (def_fail_cnt must be 0).
7. No write/read verification failures (wr_fail_cnt must be 0).
8. Test passes (finish(0)) only if all failure counters remain at zero.""",
    "Code Generation": ""
}

row4_md = {
    "Index": "4",
    "Test Case Name": "ethernet0_tx_rx_multi_chnl_test",
    "Meta Test Description": "This testcase performs multi-channel TX and RX operations on Ethernet0 across all 4 DMA channels (CH0-CH3) sequentially. It initializes the Ethernet interface via enet_intf_sel() based on conditional compilation macros SEL_ENET0/SEL_ENET1/SEL_ENET2/SEL_ENET3 (default channel 4 if none defined). When ETH_10M is defined, trns_count is set to 6. MAC_CONFIGURATION is written with speed and duplex settings: ETH_10M+HALF_DUPLEX=0x8003, ETH_10M+Full=0xA003, ETH_100M+HALF_DUPLEX=0xC003, ETH_100M+Full=0xE003, default(1G)=0x00002003. After MAC_CONFIGURATION, write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08) enables override, followed by wait_on(300). MAC_EXT_CONFIGURATION is written with 0x0. MAC_PACKET_FILTER is written with 0x80000408 (HASH FILTER Enabled and Receive All). MAC_VLAN_TAG_CTRL is written with 0x01600000 (VLAN TAG in status and RX Stripping Enabled). MAC_RXQ_CTRL0 is written with 0x000000aa. MAC_RXQ_CTRL1 is written with 0x00000000. MAC_RXQ_CTRL2 is written with 0x08040201 (Priorities for RX Queues). MAC_ADDRESS3_HIGH=0x80033607, MAC_ADDRESS2_HIGH=0x80022607, MAC_ADDRESS1_HIGH=0x80011607, MAC_ADDRESS0_HIGH=0x80000607 (DMA Channel selection, Address Enabled, upper 16 bits of MAC addresses). MAC_ADDRESS2_LOW=0x08090a0b, MAC_ADDRESS1_LOW=0x08090a0b, MAC_ADDRESS0_LOW=0x08090a0b. MTL_OPERATION_MODE is written with 0x00000000. MTL_TXQ2_OPERATION_MODE=0x0003000a, MTL_TXQ1_OPERATION_MODE=0x0003000a, MTL_TXQ0_OPERATION_MODE=0x0003000a. MTL_RXQ3_OPERATION_MODE=0x00f00033, MTL_RXQ2_OPERATION_MODE=0x00f00033, MTL_RXQ1_OPERATION_MODE=0x00f00033, MTL_RXQ0_OPERATION_MODE=0x00f00033. MTL_TXQ3_QUANTUM_WEIGHT=0x00000014, MTL_TXQ2_QUANTUM_WEIGHT=0x00000014, MTL_TXQ1_QUANTUM_WEIGHT=0x00000005. MTL_Q3_INTERRUPT_CONTROL_STATUS=0x01000100, MTL_Q2_INTERRUPT_CONTROL_STATUS=0x01000100, MTL_Q1_INTERRUPT_CONTROL_STATUS=0x01000100. MTL_RXQ_DMA_MAP0 is written with 0x00000083 initially. DMA_CH3_CONTROL=0x0, DMA_CH2_CONTROL=0x0, DMA_CH1_CONTROL=0x0, DMA_CH0_CONTROL=0x00000000. DMA_CH3_TX_CONTROL=0x00100006, DMA_CH2_TX_CONTROL=0x00100006, DMA_CH1_TX_CONTROL=0x00100006, DMA_CH0_TX_CONTROL=0x00100006 (initial bulk configuration). DMA_CH3_RX_CONTROL=0x00101000, DMA_CH2_RX_CONTROL=0x00101000, DMA_CH1_RX_CONTROL=0x00101000, DMA_CH0_RX_CONTROL=0x00101000 (initial bulk configuration). DMA_CH0_RX_CONTROL2=0x9. DMA_CH0_INTERRUPT_ENABLE=0xf0df. DMA_MODE=0x00000000. DMA_CH0_TXDESC_LIST_ADDRESS=0xE6000000, DMA_CH1_TXDESC_LIST_ADDRESS=0xE6010000, DMA_CH2_TXDESC_LIST_ADDRESS=0xE6020000, DMA_CH3_TXDESC_LIST_ADDRESS=0xE6030000. DMA_CH0_RXDESC_LIST_ADDRESS=0xE6040000, DMA_CH1_RXDESC_LIST_ADDRESS=0xE6050000, DMA_CH2_RXDESC_LIST_ADDRESS=0xE6060000, DMA_CH3_RXDESC_LIST_ADDRESS=0xE6070000. DMA_CH0_TXDESC_TAIL_POINTER=0xE6009EB4, DMA_CH1_TXDESC_TAIL_POINTER=0xE6019EB4, DMA_CH2_TXDESC_TAIL_POINTER=0xE6029EB4, DMA_CH3_TXDESC_TAIL_POINTER=0xE6039EB4. DMA_CH3_RXDESC_TAIL_POINTER=0xE6070010, DMA_CH2_RXDESC_TAIL_POINTER=0xE6060010, DMA_CH1_RXDESC_TAIL_POINTER=0xE6050010, DMA_CH0_RXDESC_TAIL_POINTER=0xE6040010. VIP sequencer triggered via write_reg(0xA0243ffc, enet_sel) and write_reg(0xA0243ff8, 0xdeadbeef). Channel 0 runs first: loops for(i=0;i<20;i++) with while(int_pend) and break when rxpkt==20 && txpkt==20. Then int_pend=1. Channel 1 activation: write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007); write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000081); write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010); write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101001); write_reg(0xA0243ff8, 0xdeadbeed); loops for(i=0;i<20;i++) break when rxpkt==20 && txpkt==20. Then int_pend=1. Channel 2 activation: write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007); write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000082); write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010); write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101001); write_reg(0xA0243ff8, 0xdeadbeee); loops for(i=0;i<20;i++) break when rxpkt==10 && txpkt==10. Then int_pend=1. Channel 3 activation: write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083); write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010); write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101001). Global variables: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0; extern int int_pend. Functions: int test_case(), void Default_IRQHandler(). Default_IRQHandler declares: int rdata, wdata; unsigned int irq_no; int rdata_tailpointer; int rdata_desc; int_pend = 0; irq_no = 42 + enet_select; enet_sel = enet_select. Handler reads: rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC); rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXBUFFER); and similar for CH1, CH2, CH3. Also reads DMA_CH3_CURRENT_APP_TXDESC, DMA_CH3_CURRENT_APP_TXBUFFER. write_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE60298a4); write_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE60398a4).",
    "Meta Test Steps / Procedure": """1. #include <stdio.h>
2. #include <stdlib.h>
3. #include <math.h>
4. #include <ethernet0/ethernet0_def.h>
5. #include <test_common.h>
6. #include <ethernet0/ethernet0_offset.h>
7. #include <ethernet0/ethernet0_funcs.h>
8. #include <ethernet0/ethernet0_programming_sequence.h>
9. Global declarations: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0; extern int int_pend;
10. void Default_IRQHandler() declared
11. int test_case() entry point called
12. #ifdef ETH_10M trns_count = 6; #endif
13. Ethernet interface selection via SEL_ENETx macros
14. MAC Configuration with speed/duplex settings, default 1G=0x00002003
15. write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08); //Enabling Override
16. wait_on(300);
17. write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);
18-20. MAC_RxQ_Ctrl0=0x000000aa, MAC_RxQ_Ctrl1=0x00000000, MAC_RxQ_Ctrl2=0x08040201
21. write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000408);
22. write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x01600000);
23-26. MAC Address HIGH registers for 4 slots
27-29. MAC Address LOW registers
30. write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000);
31-33. MTL TxQ Operation Modes (Q0-Q2) = 0x0003000a
34-37. MTL RxQ Operation Modes (Q0-Q3) = 0x00f00033
38-40. MTL TxQ Quantum Weights
41. write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);
42-44. MTL Queue Interrupt Control Status = 0x01000100
45-48. DMA Channel Control registers = 0x0
49-52. DMA TX Control for all 4 channels = 0x00100006
53-56. DMA RX Control for all 4 channels = 0x00101000
57. DMA_CH0_RX_CONTROL2 = 0x9
58. DMA_CH0_INTERRUPT_ENABLE = 0xf0df
59. DMA_MODE = 0x00000000
60-63. DMA TX Descriptor List Addresses
64-67. DMA RX Descriptor List Addresses
68-71. DMA TX Descriptor Tail Pointers
72-75. DMA RX Descriptor Tail Pointers
76-77. VIP sequencer triggered
78. Channel 0 loop: 20 iterations, break on rxpkt==20 && txpkt==20
79. int_pend = 1
80. Channel 1 activation with TX_CONTROL=0x00100007, RXQ_DMA_MAP0=0x00000081
81. VIP trigger with 0xdeadbeed
82. Channel 1 loop: 20 iterations, break on rxpkt==20 && txpkt==20
83. int_pend = 1
84. Channel 2 activation with TX_CONTROL=0x00100007, RXQ_DMA_MAP0=0x00000082
85. VIP trigger with 0xdeadbeee
86. Channel 2 loop: 20 iterations, break on rxpkt==10 && txpkt==10
87. int_pend = 1
88. Channel 3 activation with RXQ_DMA_MAP0=0x00000083
89. Default_IRQHandler implementation
90. Pass/fail based on def_fail_cnt and wr_fail_cnt""",
    "Meta Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_VLAN_Tag_Ctrl, MAC_Address0_High, MAC_Address1_High, MAC_Address2_High, MAC_Address3_High, MAC_Address0_Low, MAC_Address1_Low, MAC_Address2_Low, MAC_Address3_Low, MAC_RxQ_Ctrl0, MAC_RxQ_Ctrl1, MAC_RxQ_Ctrl2, DMA_Mode, DMA_CH0_Control, DMA_CH1_Control, DMA_CH2_Control, DMA_CH3_Control, DMA_CH0_Tx_Control, DMA_CH1_Tx_Control, DMA_CH2_Tx_Control, DMA_CH3_Tx_Control, DMA_CH0_Rx_Control, DMA_CH1_Rx_Control, DMA_CH2_Rx_Control, DMA_CH3_Rx_Control, DMA_CH0_Rx_Control2, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH0_RxDesc_List_Address, DMA_CH1_RxDesc_List_Address, DMA_CH2_RxDesc_List_Address, DMA_CH3_RxDesc_List_Address, DMA_CH0_TxDesc_Tail_Pointer, DMA_CH1_TxDesc_Tail_Pointer, DMA_CH2_TxDesc_Tail_Pointer, DMA_CH3_TxDesc_Tail_Pointer, DMA_CH0_RxDesc_Tail_Pointer, DMA_CH0_Interrupt_Enable, DMA_CH1_Interrupt_Enable, DMA_CH2_Interrupt_Enable, DMA_CH3_Interrupt_Enable, DMA_CH0_Current_App_RxDesc, DMA_CH3_Current_App_TxDesc, DMA_CH3_Current_App_TxBuffer, MTL_Operation_Mode, MTL_TxQ0_Operation_Mode, MTL_TxQ1_Operation_Mode, MTL_TxQ2_Operation_Mode, MTL_RxQ0_Operation_Mode, MTL_RxQ1_Operation_Mode, MTL_RxQ2_Operation_Mode, MTL_RxQ3_Operation_Mode, MTL_TxQ1_Quantum_Weight, MTL_TxQ2_Quantum_Weight, MTL_TxQ3_Quantum_Weight, MTL_RxQ_DMA_Map0, MTL_Q1_Interrupt_Control_Status, MTL_Q2_Interrupt_Control_Status, MTL_Q3_Interrupt_Control_Status",
    "Meta Validation / Acceptance Criteria": "Channel 0 completion: for(i=0; i<20; i++) { while(int_pend) { wait_on(10); } if(rxpkt == 20 && txpkt == 20) { break; } int_pend = 1; }. Channel 1 completion: for(i=0; i<20; i++) { while(int_pend) { wait_on(10); } if(rxpkt == 20 && txpkt == 20) { break; } int_pend = 1; }. Channel 2 completion: for(i=0; i<20; i++) { while(int_pend) { wait_on(10); } if(rxpkt == 10 && txpkt == 10) { break; } int_pend = 1; }. Default_IRQHandler sets int_pend = 0 on entry. Handler reads DMA_CH0_Current_App_RxDesc, DMA_CH0_Current_App_RxBuffer, DMA_CH3_Current_App_TxDesc, DMA_CH3_Current_App_TxBuffer. Handler writes DMA_CH2_TxDesc_Tail_Pointer=0xE60298a4, DMA_CH3_TxDesc_Tail_Pointer=0xE60398a4 via write_reg. Pass condition: def_fail_cnt == 0 and wr_fail_cnt == 0. If either counter is non-zero, finish(1) (FAIL). Otherwise finish(0) (PASS).",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include <math.h>\n#include <ethernet0/ethernet0_def.h>\n#include <test_common.h>\n#include <ethernet0/ethernet0_offset.h>\n#include <ethernet0/ethernet0_funcs.h>\n#include <ethernet0/ethernet0_programming_sequence.h>",
    "Meta Macros": "ETH_10M\nETH_100M\nHALF_DUPLEX\nDEBUG_DISPLAY\nPRELOAD_AGAIN\nSEL_ENET0\nSEL_ENET1\nSEL_ENET2\nSEL_ENET3",
    "Meta Arrays": "NA"
}

testplan_rows = [row1_tp, row2_tp, row3_tp, row4_tp]
metadata_rows = [row1_md, row2_md, row3_md, row4_md]

# ============================================================
# CREATE WORKBOOK
# ============================================================

wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Header
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Data rows
for row_idx, row_data in enumerate(testplan_rows, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        val = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns
max_widths = {}
for row in ws_tp.iter_rows(min_row=1, max_row=ws_tp.max_row, max_col=ws_tp.max_column):
    for cell in row:
        if cell.value:
            lines = str(cell.value).split('\n')
            max_line_len = max(len(line) for line in lines)
            col_letter = get_column_letter(cell.column)
            current = max_widths.get(col_letter, 0)
            max_widths[col_letter] = max(current, min(max_line_len + 2, 80))

for col_letter, width in max_widths.items():
    ws_tp.column_dimensions[col_letter].width = max(width, 12)

# --- MetaData Sheet ---
ws_md = wb.create_sheet(title="MetaData")

for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_idx, row_data in enumerate(metadata_rows, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        val = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_alignment

ws_md.freeze_panes = "A2"

max_widths_md = {}
for row in ws_md.iter_rows(min_row=1, max_row=ws_md.max_row, max_col=ws_md.max_column):
    for cell in row:
        if cell.value:
            lines = str(cell.value).split('\n')
            max_line_len = max(len(line) for line in lines)
            col_letter = get_column_letter(cell.column)
            current = max_widths_md.get(col_letter, 0)
            max_widths_md[col_letter] = max(current, min(max_line_len + 2, 80))

for col_letter, width in max_widths_md.items():
    ws_md.column_dimensions[col_letter].width = max(width, 12)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save workbook
wb.save(output_path)
print(f"Workbook saved to: {output_path}")
print(f"Filename: {filename}")

# Validate
import os as os2
if os2.path.exists(output_path) and os2.path.getsize(output_path) > 0:
    wb2 = load_workbook(output_path)
    sheets = wb2.sheetnames
    print(f"Validation: File exists, size={os2.path.getsize(output_path)}, sheets={sheets}")
    if "TestPlan" in sheets and "MetaData" in sheets:
        print("VALIDATION: PASSED")
    else:
        print("VALIDATION: FAILED - missing sheets")
else:
    print("VALIDATION: FAILED - file not found or empty")
