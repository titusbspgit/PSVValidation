#!/usr/bin/env python3
"""
Agent 7 Runner - Generates and commits the Excel workbook.
Execute this script in the repo root to generate the workbook.
"""
import subprocess
import sys
import os

# Ensure dependencies
subprocess.check_call([sys.executable, "-m", "pip", "install", "openpyxl", "pytz", "-q"])

script_dir = os.path.join("Test_Output", "Ethernet", "Ethernet0", "TestPlan")
gen_script = os.path.join(script_dir, "Ethernet_TestPlan_Generator_Final.py")

result = subprocess.run([sys.executable, gen_script], capture_output=True, text=True)
print(result.stdout)
if result.returncode != 0:
    print(result.stderr)
    sys.exit(1)
print("Generation complete.")
