#!/bin/bash
# Run the XLSX generator
cd "$(dirname "$0")"
pip install openpyxl 2>/dev/null
python3 generate_PCIE_TestPlan_20250710.py
echo "Done. Check for PCIE_TestPlan_20250710_033000.xlsx"
