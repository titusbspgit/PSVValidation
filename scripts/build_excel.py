import openpyxl
from openpyxl import Workbook

# Create workbook
wb = Workbook()

# Sheet 1 (Visible)
ws1 = wb.active
ws1.title = "Sheet 1"
ws1.append(["NAME"])  # Header
ws1.append(["Varsha"])  # Row 1
ws1.append(["Varsha"])  # Row 2

# Sheet 2 (Hidden from UI)
ws2 = wb.create_sheet(title="Sheet 2")
ws2.append(["NAME", "PLACE"])  # Headers
ws2.append(["Varsha", "Chennai"])  # Row 1
ws2.append(["Varsha", "Bangalore"])  # Row 2

# Hide Sheet 2 and ensure Sheet 1 is active/visible
ws2.sheet_state = "hidden"
wb.active = 0  # Keep Sheet 1 active

# Save Excel file in repo root
wb.save("Name_Place.xlsx")
