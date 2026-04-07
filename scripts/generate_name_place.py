import openpyxl
from openpyxl import Workbook


def main():
    wb = Workbook()
    ws = wb.active
    ws.title = "Sheet1"
    # Headers
    ws.append(["NAME", "PLACE"])
    # Rows
    ws.append(["Varsha", "Chennai"])
    ws.append(["Varsha", "Bangalore"])
    wb.save("Name_Place.xlsx")


if __name__ == "__main__":
    main()
