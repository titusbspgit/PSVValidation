import sys
from pathlib import Path
from openpyxl import load_workbook

SRC = Path("TestRepo/gpio/GPIO_TestCase.xlsx")
DST = Path("TestRepo/gpio/GPIO_TestCase_Update.xlsx")
COLUMN_NAME = "Code Generation - Required/Not Required"
NEW_VALUE = "Required"

if not SRC.exists():
    print(f"Source file not found: {SRC}")
    sys.exit(0)

wb = load_workbook(SRC)

updated_any = False
for ws in wb.worksheets:
    # Build header map from the first row
    headers = {}
    for cell in ws[1]:
        val = str(cell.value).strip() if cell.value is not None else None
        if val:
            headers[val] = cell.column
    if COLUMN_NAME in headers:
        col = headers[COLUMN_NAME]
        max_row = ws.max_row or 1
        # Start from row 2 to skip header
        for r in range(2, max_row + 1):
            ws.cell(row=r, column=col, value=NEW_VALUE)
        updated_any = True
        print(f"Updated sheet '{ws.title}' column '{COLUMN_NAME}' to '{NEW_VALUE}' for rows 2..{max_row}")
    else:
        print(f"Column '{COLUMN_NAME}' not found in sheet '{ws.title}', skipping.")

# Save to destination path, keeping source intact
DST.parent.mkdir(parents=True, exist_ok=True)
wb.save(DST)

if updated_any:
    print(f"Saved updated workbook to {DST}")
else:
    print("No sheets contained the target column; saved copy without changes.")
