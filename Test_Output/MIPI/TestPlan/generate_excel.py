import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
import datetime
import os
import json

# IST timestamp
ist_now = datetime.datetime.utcnow() + datetime.timedelta(hours=5, minutes=30)
timestamp = ist_now.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"

print(f"FILENAME={filename}")
print(f"TIMESTAMP={timestamp}")
