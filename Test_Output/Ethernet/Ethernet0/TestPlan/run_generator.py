#!/usr/bin/env python3
"""Quick runner to verify the generator works"""
import subprocess
import sys
result = subprocess.run([sys.executable, "Test_Output/Ethernet/Ethernet0/TestPlan/generate_testplan.py"], 
                       capture_output=True, text=True)
print("STDOUT:", result.stdout)
print("STDERR:", result.stderr)
print("Return code:", result.returncode)
