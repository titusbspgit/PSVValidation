#!/usr/bin/env python3
"""PCIE TestPlan XLSX Generator - Run this script to generate the Excel workbook."""
import json, os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
TS = now_ist.strftime("%Y%m%d_%H%M%S")
FN = f"PCIE_TestPlan_{TS}.xlsx"

# Full JSON data is in the companion _data.json file
# This script is a template - actual data is embedded at generation time
print(f"Would generate: {FN}")
