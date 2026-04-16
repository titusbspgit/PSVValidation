# Auto-generated JSON to Excel converter (fallback automation)
# Deterministic conversion preserving key order, values, and basic formatting

from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

# Embedded JSON data (exactly as provided)
DATA = [
  {
    "name": "Adeel Solangi",
    "language": "Sindhi",
    "id": "V59OF92YF627HFY0",
    "bio": "Donec lobortis eleifend condimentum. Cras dictum dolor lacinia lectus vehicula rutrum. Maecenas quis nisi nunc. Nam tristique feugiat est vitae mollis. Maecenas quis nisi nunc.",
    "version": 6.1
  },
  {
    "name": "Afzal Ghaffar",
    "language": "Sindhi",
    "id": "ENTOCR13RSCLZ6KU",
    "bio": "Aliquam sollicitudin ante ligula, eget malesuada nibh efficitur et. Pellentesque massa sem, scelerisque sit amet odio id, cursus tempor urna. Etiam congue dignissim volutpat. Vestibulum pharetra libero et velit gravida euismod.",
    "version": 1.88
  },
  {
    "name": "Aamir Solangi",
    "language": "Sindhi",
    "id": "IAKPO3R4761JDRVG",
    "bio": "Vestibulum pharetra libero et velit gravida euismod. Quisque mauris ligula, efficitur porttitor sodales ac, lacinia non ex. Fusce eu ultrices elit, vel posuere neque.",
    "version": 7.27
  }
]

# Target output path inside repository
OUTPUT_PATH = Path("TestRepo/gpio/json_Tester.xlsx")


def validate_input(data):
    if not isinstance(data, list) or len(data) == 0:
        raise ValueError("Invalid or empty JSON input: expected a non-empty array of objects")
    for i, row in enumerate(data, start=1):
        if not isinstance(row, dict):
            raise ValueError(f"Unsupported JSON structure at index {i-1}: each item must be an object")


def collect_headers(data):
    headers = []
    seen = set()
    for row in data:
        for k in row.keys():  # preserves original key order as given
            if k not in seen:
                seen.add(k)
                headers.append(k)
    return headers


def auto_fit_columns(ws, headers, rows):
    for col_idx, key in enumerate(headers, start=1):
        values = [str(key)] + [str(r.get(key, "")) for r in rows]
        max_len = max(len(v) for v in values)
        ws.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 80)


def build_workbook(data):
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    headers = collect_headers(data)

    # Header row
    ws.append(headers)
    for cell in ws[1]:
        cell.font = Font(bold=True)

    # Data rows
    for row in data:
        ws.append([row.get(h, "") for h in headers])

    # Freeze header row
    ws.freeze_panes = "A2"

    # Auto-fit widths
    auto_fit_columns(ws, headers, data)

    return wb


def main():
    validate_input(DATA)
    wb = build_workbook(DATA)

    # Ensure directory exists
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)

    # Save workbook
    wb.save(OUTPUT_PATH)


if __name__ == "__main__":
    main()
