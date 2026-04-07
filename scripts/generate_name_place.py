import openpyxl

wb = openpyxl.Workbook()
ws = wb.active
ws.title = 'Sheet1'

# Headers
ws.append(['NAME', 'PLACE'])

# Data rows
ws.append(['Varsha', 'Chennai'])
ws.append(['Varsha', 'Bangalore'])

wb.save('Name_Place.xlsx')
