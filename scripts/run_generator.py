#!/usr/bin/env python3
"""Runner script that generates the XLSX and outputs base64 for upload"""
import subprocess, sys, os

# Install openpyxl if needed
subprocess.check_call([sys.executable, "-m", "pip", "install", "openpyxl", "-q"])

# Now run the generator
exec(open(os.path.join(os.path.dirname(__file__), "generate_workbook.py")).read())
