#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import json
import os
import sys

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'Ethernet_TestPlan_{timestamp}.xlsx'

# Complete JSON data with all Meta fields
json_data = [
  {
    "Index": "1",
    "SS / Module": "Ethernet",
    "Test Case Name": "ethernet0_reg_wr_rd_test",
    "Feature": "Register Write-Read Verification",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_define.c\"; <test_common.h>; <ethernet0/ethernet0_def.h>; <ethernet0/ethernet0_offset.h>",
    "Meta Macros": "#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n#define CNT 434",
    "Meta Arrays": "const unsigned long int addr_array[434] = {\n  mizar_ETHERNET0_MAC_CONFIGURATION,\n  mizar_ETHERNET0_MAC_EXT_CONFIGURATION,\n  mizar_ETHERNET0_MAC_PACKET_FILTER,\n  mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,\n  mizar_ETHERNET0_MAC_HASH_TABLE_REG0\n};",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This test verifies the default reset values and write-read accessibility of Ethernet0 MAC registers.",
    "Test Steps / Procedure": "1. Initialize global fail counters.\n2. Execute default value check phase.\n3. Execute write-read check phase.\n4. Evaluate overall result.",
    "Impacted Registers": "MAC_Configuration; MAC_Ext_Configuration; MAC_Packet_Filter; MAC_WD_JB_Timeout; MAC_Hash_Table_Reg0",
    "Validation / Acceptance Criteria": "1. All registers must return expected default reset values.\n2. Write-read patterns must match expected values.\n3. Test passes only if zero mismatches detected.",
    "Remarks": "The addr_array is declared with size 434 but only 5 register entries are initialized."
  }
]

print(f'Generated filename: {filename}')
print('Script ready - run to generate Excel')
