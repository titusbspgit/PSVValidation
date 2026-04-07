from openpyxl import Workbook

# Create workbook and Sheet 1 (Visible)
wb = Workbook()
ws1 = wb.active
ws1.title = "Sheet 1"
ws1.append(["NAME"])  # header
ws1.append(["Varsha"])  # row 1
ws1.append(["Varsha"])  # row 2

# Create Sheet 2 (Hidden) with NAME and PLACE
ws2 = wb.create_sheet(title="Sheet 2")
ws2.append(["NAME", "PLACE"])  # headers
ws2.append(["Varsha", "Chennai"])  # row 1
ws2.append(["Varsha", "Bangalore"])  # row 2

# Hide Sheet 2 and set Sheet 1 as active
ws2.sheet_state = "hidden"
wb.active = 0

# Save Excel file at repository root
wb.save("Name_Place.xlsx")
print("Generated Name_Place.xlsx with Sheet 2 hidden and Sheet 1 visible.")
