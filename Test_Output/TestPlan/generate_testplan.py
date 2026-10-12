#!/usr/bin/env python3
"""
Temporary script to generate LPDDR4 TestPlan XLSX.
This file will be removed after generation.
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from datetime import datetime, timezone, timedelta

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"

print(f"Generated filename: {filename}")
print(f"IST time: {now_ist.isoformat()}")
