#!/usr/bin/env python3
"""Generate PCIE TestPlan XLSX - Agent 7"""
import openpyxl
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from datetime import datetime, timezone, timedelta
import json, sys, os

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
ts = now_ist.strftime("%Y%m%d_%H%M%S")
fname = f"PCIE_TestPlan_{ts}.xlsx"
print(f"Generating: {fname}")
