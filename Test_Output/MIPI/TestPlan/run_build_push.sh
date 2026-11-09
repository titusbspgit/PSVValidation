#!/bin/bash
# Run this script to generate and push the MIPI_DSI xlsx workbook
# Prerequisites: pip install openpyxl requests
# Set GITHUB_TOKEN environment variable before running
cd "$(dirname "$0")"
python3 build_mipi_dsi_xlsx_20261109.py
