#!/usr/bin/env python3
"""
Excel Workbook Generator for USB TestPlan
Generates USB_TestPlan_YYYYMMDD_HHMMSS.xlsx using openpyxl
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    import openpyxl
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    print("ERROR: openpyxl not available")
    sys.exit(1)

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_hs",
        "Feature": "USB Host Enumeration - High Speed",
        "Speed": "High-Speed (HS)",
        "Mode": "Host",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "Verify USB host enumeration of a high-speed device...",
        "Test Steps / Procedure": "1. Call NIC programming initialization...",
        "Impacted Registers": "CONFIG, DCBAAP_LO, ...",
        "Validation / Acceptance Criteria": "1. After starting the host controller...",
        "Remarks": "The high-speed enumeration test...",
        "Meta Test Description": "USB host enumeration test for high-speed...",
        "Meta Test Steps / Procedure": "1. extern int int_pend...",
        "Meta Impacted Registers": "CONFIG, DCBAAP_LO, ...",
        "Meta Validation / Acceptance Criteria": "1. After port status change...",
        "Meta Headers": '#include <stdio.h>...',
        "Meta Macros": "MIZAR_USB_BASE, ...",
        "Meta Arrays": "int data_in[512];..."
    }
]

print(f"Filename: {filename}")
print(f"Timestamp IST: {now_ist.isoformat()}")
print("openpyxl version:", openpyxl.__version__)
print("Generation script ready")
