#!/usr/bin/env python3
"""Excel TestPlan Generator - Agent 7"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import json
import os

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"Ethernet_TestPlan_{timestamp}.xlsx"

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "Ethernet",
        "Test Case Name": "ethernet0_reg_wr_rd_test",
        "Feature": "Register Write-Read Verification",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_define.c\"; <test_common.h>; <ethernet0/ethernet0_def.h>; <ethernet0/ethernet0_offset.h>",
        "Meta Macros": "#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n#define CNT 434",
        "Meta Arrays": "const unsigned long int addr_array[434] = { ... }; ...",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "0x0",
        "Memory End Offset": "0x10",
        "Meta Test Description": "This testcase verifies the register write-read functionality...",
        "Test Description": "Verify the default reset values and write-read accessibility of Ethernet0 MAC registers...",
        "Meta Test Steps / Procedure": "1. Global variable initialization...",
        "Test Steps / Procedure": "1. Initialize global failure counters to zero...",
        "Meta Impacted Registers": "mizar_ETHERNET0_MAC_CONFIGURATION; ...",
        "Impacted Registers": "MAC_Configuration; MAC_Ext_Configuration; MAC_Packet_Filter; MAC_WD_JB_Timeout; MAC_Hash_Table_Reg0",
        "Meta Validation / Acceptance Criteria": "Phase 1: data_rd == default_value_array[i]...",
        "Validation / Acceptance Criteria": "1. All registers must return their expected default reset values...",
        "Remarks": "The addr_array is declared with size 434 but only 5 entries are populated..."
    }
]

print(f"Generated filename: {filename}")
print(f"IST timestamp: {now_ist.isoformat()}")
print(f"TIMESTAMP_ONLY: {timestamp}")
print(f"FILENAME_ONLY: {filename}")
