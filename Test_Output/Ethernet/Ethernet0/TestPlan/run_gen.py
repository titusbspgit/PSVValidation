#!/usr/bin/env python3
"""Run generate_usb_testplan.py and upload result to GitHub."""
import subprocess, sys, os
script_dir = os.path.dirname(os.path.abspath(__file__))
script = os.path.join(script_dir, "generate_usb_testplan.py")
subprocess.run([sys.executable, script], cwd=script_dir)
