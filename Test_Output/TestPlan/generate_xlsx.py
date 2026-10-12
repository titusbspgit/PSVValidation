#!/usr/bin/env python3
"""Temporary generator script - will be deleted after execution."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta

# IST timestamp
ist = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(ist)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp_str}.xlsx"
print(f"FILENAME={filename}")
print(f"TIMESTAMP={timestamp_str}")
