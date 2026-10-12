#!/usr/bin/env python3
"""Temporary script - will be deleted after execution"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import json
import os
import base64

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"

print(f"Generating: {filename}")
