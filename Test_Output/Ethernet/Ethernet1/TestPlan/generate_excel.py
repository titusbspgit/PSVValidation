#!/usr/bin/env python3
"""Generate Ethernet1 TestPlan Excel workbook."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import base64
import json
import os

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"Ethernet1_TestPlan_{timestamp_str}.xlsx"

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "Ethernet1",
        "Test Case Name": "ethernet1_reg_wr_rd_test",
        "Feature": "Register Write-Read Verification",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This test verifies the default reset values and write-read accessibility of Ethernet1 MAC registers.",
        "Test Steps / Procedure": "1. Read each Ethernet1 MAC register and verify the value matches its expected default reset value.\n2. Skip registers that are not readable based on their read mask configuration.\n3. For each of six distinct test data patterns, write the pattern to each writable register.\n4. Skip registers that are not writable based on their write mask configuration.\n5. Read back each written register and compute the expected value by applying the read mask, write mask, and default value.\n6. Compare the read-back value against the computed expected value for each register and each pattern.\n7. Track any mismatches in default value checks and write-read checks separately.\n8. Report test pass if all default value and write-read verifications succeed; report test fail if any mismatch is detected.",
        "Impacted Registers": "MAC_Configuration; MAC_Ext_Configuration; MAC_Packet_Filter; MAC_WD_JB_Timeout; MAC_Hash_Table_Reg0; MAC_Hash_Table_Reg1",
        "Validation / Acceptance Criteria": "1. All registers must return their expected default reset values when read after reset.\n2. For each of six test data patterns written to each writable register, the read-back value must match the expected value computed using the applicable read and write masks and default values.\n3. The test passes (finish with success) only if zero default-value mismatches and zero write-read mismatches are detected across all registers and all patterns.\n4. Any single mismatch in either the default value check or the write-read check causes the test to fail.",
        "Remarks": "The soft_reset_chk() function is commented out in the source and is not executed during this test. The addr_array is declared with size 434 but only 6 register entries are populated in the source. Six write patterns are used for comprehensive bit-level coverage. Registers are conditionally skipped based on read mask, write mask, skip array, and skip reset array configurations.",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_define.c\"; <test_common.h>; <ethernet1/ethernet1_def.h>; <ethernet1/ethernet1_offset.h>",
        "Meta Macros": "SOFT_RST_REG_ADDRESS; SOFT_RST_REG_DATA; CNT",
        "Meta Arrays": "addr_array[434]; default_value_array[434]; read_mask_array[434]; write_mask_array[434]; skip_array[434]; skip_rst_array[434]; chk_val[6]",
        "Meta Test Description": "This testcase performs register default-value verification and write-read verification for Ethernet1 MAC registers.",
        "Meta Test Steps / Procedure": "1. test_case() is the entry point. 2. chk_rst_val() is called...",
        "Meta Impacted Registers": "mizar_ETHERNET1_MAC_CONFIGURATION; mizar_ETHERNET1_MAC_EXT_CONFIGURATION; mizar_ETHERNET1_MAC_PACKET_FILTER; mizar_ETHERNET1_MAC_WD_JB_TIMEOUT; mizar_ETHERNET1_MAC_HASH_TABLE_REG0; mizar_ETHERNET1_MAC_HASH_TABLE_REG1",
        "Meta Validation / Acceptance Criteria": "In chk_rst_val(): data_rd = read_reg(addr) must equal default_value_array[i]..."
    },
    {
        "Index": "2",
        "SS / Module": "Ethernet1",
        "Test Case Name": "ethernet1_tx_basic_test",
        "Feature": "Basic TX DMA Transmission",
        "Speed": "NA",
        "Mode": "Interrupt Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This test verifies basic Ethernet1 TX DMA packet transmission using interrupt-driven completion.",
        "Test Steps / Procedure": "1. Configure four MAC addresses...",
        "Impacted Registers": "MAC_Address3_High; MAC_Address2_High; ...",
        "Validation / Acceptance Criteria": "1. The test must successfully receive 10 transfer-complete interrupts...",
        "Remarks": "Only DMA channel 0 TX transmission is started...",
        "Meta Headers": "<stdio.h>; <stdlib.h>; <ethernet1/ethernet1_def.h>; <ethernet1/ethernet1_offset.h>; <test_common.h>; <ethernet0/ethernet0_programming_sequence.h>",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Meta Test Description": "This testcase performs a basic Ethernet1 TX DMA transmission...",
        "Meta Test Steps / Procedure": "1. Set int_pend = 1...",
        "Meta Impacted Registers": "mizar_ETHERNET1_MAC_ADDRESS3_HIGH; ...",
        "Meta Validation / Acceptance Criteria": "The test uses an interrupt-driven validation model..."
    }
]

print(f"Filename: {filename}")
print(f"Timestamp: {timestamp_str}")
print("Script ready for execution")
