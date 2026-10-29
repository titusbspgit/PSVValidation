#!/usr/bin/env python3
\"\"\"
Self-contained: generates XLSX and pushes to GitHub in one step.
Requires: pip install openpyxl requests
Run: GITHUB_TOKEN=<token> python build_and_push.py
\"\"\"
import json, os, sys, datetime, base64, io

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter
import requests

TESTPLAN = json.loads(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'testplan_data.json')).read())
META = json.loads(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'metadata_data.json')).read())

OWNER = 'titusbspgit'
REPO = 'PSVValidation'
BRANCH = 'main'
DIR = 'Test_Output/Ethernet/Ethernet1/TestPlan'

ist = datetime.timezone(datetime.timedelta(hours=5, minutes=30))
ts = datetime.datetime.now(ist).strftime('%Y%m%d_%H%M%S')
FILENAME = f'Ethernet1_TestPlan_{ts}.xlsx'

def aw(ws, mw=80):
    for col in ws.columns:
        mx = 0
        cl = get_column_letter(col[0].column)
        for cell in col:
            if cell.value:
                for ln in str(cell.value).split('\\n'):
                    mx = max(mx, len(ln))
        ws.column_dimensions[cl].width = min(mx + 4, mw)

wb = Workbook()
hf = Font(bold=True, color='FFFFFF')
hfill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap = Alignment(wrap_text=True, vertical='top')

ws = wb.active
ws.title = 'TestPlan'
cols = ['Index','SS / Module','Feature','Test Case Name','Speed','Mode',
        'Memory Start Offset','Memory End Offset','Test Description',
        'Test Steps / Procedure','Impacted Registers',
        'Validation / Acceptance Criteria','Remarks',
        'Code Generation (Required / Not)']
for c, n in enumerate(cols, 1):
    cell = ws.cell(row=1, column=c, value=n)
    cell.font = hf; cell.fill = hfill; cell.alignment = wrap
for r, rec in enumerate(TESTPLAN, 2):
    for c, key in enumerate(['Index','SS / Module','Feature','Test Case Name','Speed','Mode',
        'Memory Start Offset','Memory End Offset','Test Description',
        'Test Steps / Procedure','Impacted Registers',
        'Validation / Acceptance Criteria','Remarks'], 1):
        ws.cell(row=r, column=c, value=rec.get(key, '')).alignment = wrap
dv = DataValidation(type='list', formula1='\"Required,Not Required\"', allow_blank=True, showDropDown=False)
dv.add(f'N2:N{len(TESTPLAN)+1}')
ws.add_data_validation(dv)
ws.freeze_panes = 'A2'
aw(ws)

ws2 = wb.create_sheet('MetaData')
mcols = ['Test Case Name','Meta Headers','Meta Macros','Meta Arrays',
         'Meta Types','Meta Test Description','Meta Test Steps / Procedure',
         'Meta Impacted Registers','Meta Validation / Acceptance Criteria']
for c, n in enumerate(mcols, 1):
    cell = ws2.cell(row=1, column=c, value=n)
    cell.font = hf; cell.fill = hfill; cell.alignment = wrap
for r, rec in enumerate(META, 2):
    for c, key in enumerate(mcols, 1):
        ws2.cell(row=r, column=c, value=rec.get(key, '')).alignment = wrap
ws2.sheet_state = 'veryHidden'
ws2.freeze_panes = 'A2'
aw(ws2)

buf = io.BytesIO()
wb.save(buf)
buf.seek(0)
binary = buf.getvalue()

# Validate
buf2 = io.BytesIO(binary)
wb2 = load_workbook(buf2)
assert 'TestPlan' in wb2.sheetnames
assert 'MetaData' in wb2.sheetnames
assert wb2['TestPlan'].max_row - 1 == len(TESTPLAN)
assert wb2['MetaData'].max_row - 1 == len(META)

# Upload
token = os.environ.get('GITHUB_TOKEN')
url = f'https://api.github.com/repos/{OWNER}/{REPO}/contents/{DIR}/{FILENAME}'
headers = {'Authorization': f'token {token}', 'Accept': 'application/vnd.github.v3+json'}
payload = {'message': 'Generated TestPlan Excel Workbook', 'content': base64.b64encode(binary).decode(), 'branch': BRANCH}
resp = requests.put(url, json=payload, headers=headers)
d = resp.json()
print(json.dumps({
    'status': 'SUCCESS',
    'output_file_path': f'{DIR}/{FILENAME}',
    'github_url': d['content']['html_url'],
    'filename': FILENAME,
    'rows_testplan': len(TESTPLAN),
    'rows_metadata': len(META),
    'commit_sha': d['commit']['sha'],
    'validation': 'PASSED'
}, indent=2))
