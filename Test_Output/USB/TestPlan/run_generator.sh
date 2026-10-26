#!/bin/bash
# Run the Excel generator script
cd "$(dirname "$0")"
python3 generate_excel_v2.py
echo "Done. Check output for generated .xlsx file."
