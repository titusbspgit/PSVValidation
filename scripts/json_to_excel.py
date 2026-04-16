#!/usr/bin/env python3
import json, os, sys
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

# Embedded JSON input
JSON_INPUT = r'''[
  {
    "state": "Tamil Nadu",
    "capital": "Chennai",
    "population_millions": 72,
    "area_sq_km": 130058,
    "literacy_rate_percent": 80.1,
    "official_language": "Tamil",
    "vehicle_code": "TN",
    "coastal_state": true,
    "formation_year": 1956,
    "major_industry": "Manufacturing"
  },
  {
    "state": "Karnataka",
    "capital": "Bengaluru",
    "population_millions": 61,
    "area_sq_km": 191791,
    "literacy_rate_percent": 75.4,
    "official_language": "Kannada",
    "vehicle_code": "KA",
    "coastal_state": true,
    "formation_year": 1956,
    "major_industry": "IT"
  },
  {
    "state": "Maharashtra",
    "capital": "Mumbai",
    "population_millions": 124,
    "area_sq_km": 307713,
    "literacy_rate_percent": 82.3,
    "official_language": "Marathi",
    "vehicle_code": "MH",
    "coastal_state": true,
    "formation_year": 1960,
    "major_industry": "Finance"
  },
  {
    "state": "Kerala",
    "capital": "Thiruvananthapuram",
    "population_millions": 35,
    "area_sq_km": 38863,
    "literacy_rate_percent": 96.2,
    "official_language": "Malayalam",
    "vehicle_code": "KL",
    "coastal_state": true,
    "formation_year": 1956,
    "major_industry": "Tourism"
  },
  {
    "state": "Gujarat",
    "capital": "Gandhinagar",
    "population_millions": 63,
    "area_sq_km": 196024,
    "literacy_rate_percent": 78.0,
    "official_language": "Gujarati",
    "vehicle_code": "GJ",
    "coastal_state": true,
    "formation_year": 1960,
    "major_industry": "Petrochemicals"
  },
  {
    "state": "Rajasthan",
    "capital": "Jaipur",
    "population_millions": 80,
    "area_sq_km": 342239,
    "literacy_rate_percent": 66.1,
    "official_language": "Hindi",
    "vehicle_code": "RJ",
    "coastal_state": false,
    "formation_year": 1949,
    "major_industry": "Mining"
  },
  {
    "state": "West Bengal",
    "capital": "Kolkata",
    "population_millions": 91,
    "area_sq_km": 88752,
    "literacy_rate_percent": 76.3,
    "official_language": "Bengali",
    "vehicle_code": "WB",
    "coastal_state": true,
    "formation_year": 1950,
    "major_industry": "Agriculture"
  }
]'''

def fail(msg):
    print(msg, file=sys.stderr)
    sys.exit(2)

def main():
    try:
        data = json.loads(JSON_INPUT)
    except Exception as e:
        fail(f"Invalid JSON: {e}")

    # Normalize to list of dicts
    if isinstance(data, dict):
        rows = [data]
    elif isinstance(data, list) and all(isinstance(x, dict) for x in data):
        rows = data
    else:
        fail("Unsupported JSON structure: expected object or array of objects")

    if not rows:
        fail("Empty JSON: no data rows found")

    # Preserve first-seen key order across the union of keys
    columns = list(rows[0].keys())
    seen = set(columns)
    for obj in rows[1:]:
        for k in obj.keys():
            if k not in seen:
                columns.append(k)
                seen.add(k)

    # Build workbook
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header row
    ws.append(columns)
    for cell in ws[1]:
        cell.font = Font(bold=True)

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Data rows
    for obj in rows:
        ws.append([obj.get(col, "") for col in columns])

    # Auto-fit approximate column widths based on content length
    for idx, col_name in enumerate(columns, start=1):
        max_len = len(str(col_name))
        for row in ws.iter_rows(min_row=2, min_col=idx, max_col=idx, values_only=True):
            v = row[0]
            if v is None:
                l = 0
            else:
                l = len(str(v))
            if l > max_len:
                max_len = l
        width = min(60, max(10, max_len + 2))
        ws.column_dimensions[get_column_letter(idx)].width = width

    # Ensure output directory exists
    out_path = os.path.join('TestRepo', 'gpio', 'json_testing.xlsx')
    os.makedirs(os.path.dirname(out_path), exist_ok=True)

    # Save workbook
    wb.save(out_path)
    print(f"Wrote Excel file to {out_path}")

if __name__ == '__main__':
    main()
