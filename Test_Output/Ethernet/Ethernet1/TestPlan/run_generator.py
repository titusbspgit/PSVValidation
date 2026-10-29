#!/usr/bin/env python3
"""
Minimal runner: generates the XLSX and prints its base64 to stdout.
Pipe to a file or use in automation.

Usage:
    python run_generator.py > workbook_b64.txt
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from create_and_upload_workbook import generate
import base64, datetime

ist = datetime.timezone(datetime.timedelta(hours=5, minutes=30))
ts = datetime.datetime.now(ist).strftime('%Y%m%d_%H%M%S')
fn = f'Ethernet1_TestPlan_{ts}.xlsx'

binary = generate()
# Save locally
with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), fn), 'wb') as f:
    f.write(binary)

print(f'Generated: {fn}')
print(f'Size: {len(binary)} bytes')
print(f'Base64 length: {len(base64.b64encode(binary))}')
