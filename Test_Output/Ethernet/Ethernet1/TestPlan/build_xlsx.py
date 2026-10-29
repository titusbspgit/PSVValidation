#!/usr/bin/env python3
"""Build and upload Ethernet1 TestPlan xlsx to GitHub."""
import os, sys, base64, json, subprocess
subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl', 'PyGithub', '-q'])

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
from github import Github

IP_NAME = "Ethernet1"
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
FILENAME = f"{IP_NAME}_TestPlan_{now_ist.strftime('%Y%m%d_%H%M%S')}.xlsx"
GITHUB_TOKEN = os.environ.get('GITHUB_TOKEN', '')
REPO_NAME = "titusbspgit/PSVValidation"
BRANCH = "main"
OUTPUT_DIR = "Test_Output/Ethernet/Ethernet1/TestPlan/"

# Load data from JSON files in same directory
script_dir = os.path.dirname(os.path.abspath(__file__))
tp_path = os.path.join(script_dir, 'testplan_data.json')
md_path = os.path.join(script_dir, 'metadata_data.json')

with open(tp_path, 'r') as f:
    TESTPLAN = json.load(f)
with open(md_path, 'r') as f:
    META = json.load(f)

TP_COLS = ['Index','SS / Module','Feature','Test Case Name','Speed','Mode',
           'Memory Start Offset','Memory End Offset','Test Description',
           'Test Steps / Procedure','Impacted Registers',
           'Validation / Acceptance Criteria','Remarks',
           'Code Generation (Required / Not)']
MD_COLS = ['Test Case Name','Meta Headers','Meta Macros','Meta Arrays',
           'Meta Types','Meta Test Description','Meta Test Steps / Procedure',
           'Meta Impacted Registers','Meta Validation / Acceptance Criteria']

hf = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
hfill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wa = Alignment(wrap_text=True, vertical='top')

wb = Workbook()
ws = wb.active
ws.title = 'TestPlan'
for ci, cn in enumerate(TP_COLS, 1):
    c = ws.cell(row=1, column=ci, value=cn); c.font=hf; c.fill=hfill; c.alignment=wa
for ri, rd in enumerate(TESTPLAN, 2):
    for ci, cn in enumerate(TP_COLS, 1):
        v = '' if cn == 'Code Generation (Required / Not)' else rd.get(cn, '')
        ws.cell(row=ri, column=ci, value=v or '').alignment = wa

dv = DataValidation(type='list', formula1='"Required,Not Required"', allow_blank=True)
cg = len(TP_COLS)
for ri in range(2, len(TESTPLAN)+2): dv.add(ws.cell(row=ri, column=cg))
ws.add_data_validation(dv)
ws.freeze_panes = 'A2'
for ci in range(1, len(TP_COLS)+1):
    ml = max(len(str(ws.cell(row=1,column=ci).value or '')),
             max((max((len(l) for l in str(ws.cell(row=ri,column=ci).value or '').split('\n')), default=0)
                  for ri in range(2, len(TESTPLAN)+2)), default=0))
    ws.column_dimensions[get_column_letter(ci)].width = min(ml+4, 80)

wm = wb.create_sheet('MetaData')
wm.sheet_state = 'veryHidden'
for ci, cn in enumerate(MD_COLS, 1):
    c = wm.cell(row=1, column=ci, value=cn); c.font=hf; c.fill=hfill; c.alignment=wa
for ri, rd in enumerate(META, 2):
    for ci, cn in enumerate(MD_COLS, 1):
        wm.cell(row=ri, column=ci, value=rd.get(cn, '') or '').alignment = wa
wm.freeze_panes = 'A2'
for ci in range(1, len(MD_COLS)+1):
    ml = max(len(str(wm.cell(row=1,column=ci).value or '')),
             max((max((len(l) for l in str(wm.cell(row=ri,column=ci).value or '').split('\n')), default=0)
                  for ri in range(2, len(META)+2)), default=0))
    wm.column_dimensions[get_column_letter(ci)].width = min(ml+4, 80)

outpath = os.path.join(script_dir, FILENAME)
wb.save(outpath)

# Validate
wb2 = load_workbook(outpath)
assert 'TestPlan' in wb2.sheetnames
assert 'MetaData' in wb2.sheetnames
assert wb2['MetaData'].sheet_state == 'veryHidden'
assert wb2['TestPlan'].max_row - 1 == len(TESTPLAN)
assert wb2['MetaData'].max_row - 1 == len(META)
print(f"VALIDATED: {FILENAME}")

# Upload to GitHub
if GITHUB_TOKEN:
    g = Github(GITHUB_TOKEN)
    repo = g.get_repo(REPO_NAME)
    with open(outpath, 'rb') as f:
        content = f.read()
    path = OUTPUT_DIR + FILENAME
    result = repo.create_file(path, "Generated TestPlan Excel Workbook", content, branch=BRANCH)
    print(f"Uploaded: {result['content'].html_url}")
    print(f"SHA: {result['commit'].sha}")
else:
    print(f"No GITHUB_TOKEN. File saved locally: {outpath}")
    print(f"Upload manually to: {OUTPUT_DIR}{FILENAME}")
