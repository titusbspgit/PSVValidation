#!/bin/bash
cd "$(dirname "$0")"
pip install openpyxl 2>/dev/null || pip3 install openpyxl 2>/dev/null
python generate_excel.py 2>&1 || python3 generate_excel.py 2>&1
