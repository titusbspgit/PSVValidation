#!/usr/bin/env python3
import openpyxl,base64,os,json,sys
from openpyxl.styles import Font,PatternFill,Alignment
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation
try:
    import requests
except ImportError:
    requests=None
T=os.environ.get('GITHUB_TOKEN','')
O='titusbspgit';R='PSVValidation';B='main'
D='Test_Output/MIPI/TestPlan'
F='MIPI_CSI_TestPlan_20261105_213000.xlsx'
P=f'{D}/{F}'
def go():
    wb=openpyxl.Workbook()
    hf=Font(name='Calibri',bold=True,color='FFFFFF',size=11)
    hfl=PatternFill(start_color='4472C4',end_color='4472C4',fill_type='solid')
    wa=Alignment(wrap_text=True,vertical='top')
    ws=wb.active;ws.title='TestPlan'
    th=['Index','SS / Module','Feature','Test Case Name','Speed','Mode','Memory Start Offset','Memory End Offset','Test Description','Test Steps / Procedure','Impacted Registers','Validation / Acceptance Criteria','Remarks','Code Generation (Required / Not)']
    for i,h in enumerate(th,1):
        c=ws.cell(row=1,column=i,value=h);c.font=hf;c.fill=hfl;c.alignment=wa
    td=['1','MIPI_CSI','Register Read/Write Verification','mipi_csi2_rb_reg_wr_rd_test','NA','NA','NA','NA',
        'This testcase verifies the register read/write functionality of 10 MIPI CSI2 RB registers: VIRTUAL_CHANNEL, LINE_INFO, FIFO_THRESHOLD_VAL, LANE_CLK, MEM, INTERRUPT_RAW, INTERRUPT_MASK, INTERRUPT_ENABLE, CONTROL_DATA, and FLUSH. The test executes in two phases. Phase 1 (Default Value Check) reads all registers after reset and verifies that each register holds its expected default/reset value. Phase 2 (Write and Read Check) writes a test data pattern to each writable register, then reads back and compares the read data against an expected value that accounts for the register\'s read mask, write mask, and default value. Registers INTERRUPT_RAW and FLUSH are explicitly skipped during the write/read phase. Read-only registers are also skipped. The test reports overall pass or fail based on accumulated failure counters from both phases.',
        'Step 1: Initialize the test environment and set failure counters to zero.\nStep 2: Execute the Default Value Check phase - read each of the 10 MIPI CSI2 RB registers and compare the read value against the expected default/reset value for that register. Increment the default-fail counter for any mismatch.\nStep 3: Execute the Write and Read Check phase:\n  Step 3a: Iterate over all registers in the register list.\n  Step 3b: For each register, check if it is flagged in the skip list (INTERRUPT_RAW and FLUSH are skipped). If flagged, skip to the next register.\n  Step 3c: Check if the register is writable. If not writable, skip writing to that register.\n  Step 3d: For writable and non-skipped registers, write the test data pattern to the register.\n  Step 3e: After completing all writes, iterate over all registers again for the read-back phase.\n  Step 3f: For each register, skip if flagged in the skip list, if not writable, or if not readable.\n  Step 3g: For eligible registers, read the register value and compute the expected value using the write mask, read mask, and default value. The expected value accounts for writable bits reflecting the written data and non-writable bits retaining their default values.\n  Step 3h: Compare the read value against the computed expected value. Increment the write-fail counter for any mismatch.\nStep 4: Evaluate overall test result - if either the default-fail counter or the write-fail counter is greater than zero, report test FAIL. Otherwise, report test PASS.',
        'virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush',
        '1. Default Value Check: Each register must return its expected default/reset value when read after reset. The default-fail counter must remain zero.\n2. Write/Read Check: For each writable and non-skipped register, the read-back value after writing the test pattern must match the computed expected value. The expected value is determined by combining the written data (masked by both the read mask and write mask) with the default value (masked by the read mask and the inverse of the write mask). The write-fail counter must remain zero.\n3. Skip Verification: Registers INTERRUPT_RAW and FLUSH must be correctly skipped during the write/read phase. Read-only registers must be skipped for writing.\n4. Overall Pass Criteria: Both the default-fail counter and the write-fail counter must be zero for the test to pass. Any non-zero counter results in test failure.',
        'The soft reset check function exists in the test but is currently disabled. Only 10 of the 60 array slots are populated with valid register entries. Registers INTERRUPT_RAW and FLUSH are explicitly excluded from write/read verification via the skip configuration.',
        '']
    for i,v in enumerate(td,1):
        c=ws.cell(row=2,column=i,value=v if v else None);c.alignment=wa
    dv=DataValidation(type='list',formula1='"Required,Not Required"',allow_blank=True,showDropDown=False,showErrorMessage=True)
    dv.add('N2:N1048576');ws.add_data_validation(dv)
    cw=[8,15,30,35,8,8,20,20,60,60,45,60,45,30]
    for ci in range(1,15):
        ws.column_dimensions[get_column_letter(ci)].width=cw[ci-1]
    ws.freeze_panes='A2'
    wm=wb.create_sheet('MetaData')
    mh=['Test Case Name','Meta Headers','Meta Macros','Meta Arrays','Meta Types','Meta Function Inventory','Meta Test Description','Meta Test Steps / Procedure','Meta Impacted Registers','Meta Validation / Acceptance Criteria']
    for i,h in enumerate(mh,1):
        c=wm.cell(row=1,column=i,value=h);c.font=hf;c.fill=hfl;c.alignment=wa
    md=['mipi_csi2_rb_reg_wr_rd_test',
        'File: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include "test_define.c"\n#include "test_common.h"\n\nFile: test_define.c\n#include<mipi_csi2_rb_reg.h>',
        'File: test_define.c\n#define CNT 60\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS\t0x00000000\n#define SOFT_RST_REG_DATA\t0x00000000\n\nConditional compilation directive used in program.c:\n#ifdef DEBUG_DISPLAY\n(DEBUG_DISPLAY is not defined in the supplied source; used as a conditional guard around printf statements.)',
        'File: test_define.c\n\nconst unsigned long int addr_array[60]={\n    MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,\n    MIZAR_MIPI_CSI2_RB_REG_LINE_INFO,\n    MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL,\n    MIZAR_MIPI_CSI2_RB_REG_LANE_CLK,\n    MIZAR_MIPI_CSI2_RB_REG_MEM,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE,\n    MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA,\n    MIZAR_MIPI_CSI2_RB_REG_FLUSH\n};\n\nconst int default_value_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_LINE_INFO_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_LANE_CLK_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_MEM_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_FLUSH_DEFAULT_VAL\n};\n\nconst int read_mask_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_READ_MASK,\n    MIPI_CSI2_RB_REG_LINE_INFO_READ_MASK,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_READ_MASK,\n    MIPI_CSI2_RB_REG_LANE_CLK_READ_MASK,\n    MIPI_CSI2_RB_REG_MEM_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_READ_MASK,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_READ_MASK,\n    MIPI_CSI2_RB_REG_FLUSH_READ_MASK\n};\n\nconst int write_mask_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_WRITE_MASK,\n    MIPI_CSI2_RB_REG_LINE_INFO_WRITE_MASK,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_WRITE_MASK,\n    MIPI_CSI2_RB_REG_LANE_CLK_WRITE_MASK,\n    MIPI_CSI2_RB_REG_MEM_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_WRITE_MASK,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_WRITE_MASK,\n    MIPI_CSI2_RB_REG_FLUSH_WRITE_MASK\n};\n\nconst int skip_array[60]={0,0,0,0,0,1,0,0,0,1};',
        'TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;\n\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;',
        'File: program.c\n\n--- Function Definition: test_case ---\nSignature: int test_case()\nReturn type: int\nParameters: none\nBody supplied: yes (complete)\nCalls in source order:\n  1. chk_rst_val()\n  2. printf("********* Default value check end ************\\n") [inside #ifdef DEBUG_DISPLAY]\n  3. chk_rd_wr()\n  4. printf("********* Write & Read from registers end ************\\n") [inside #ifdef DEBUG_DISPLAY]\n  5. finish(1) [inside if(def_fail_cnt > 0 || wr_fail_cnt > 0)]\n  6. finish(0) [inside else branch]\nCommented-out calls: soft_reset_chk(), printf("********* Soft reset end ************\\n"), chk_rst_val()\n\n--- Function Definition: chk_rd_wr ---\nSignature: void chk_rd_wr() [inferred from supplied excerpt]\nReturn type: void (inferred)\nParameters: none\nBody supplied: partial\n\n--- Function: chk_rst_val ---\nCalled from: test_case()\nBody supplied: no.\n\n--- Function: soft_reset_chk ---\nReferenced in: test_case() (commented out)\nBody supplied: no.\n\n--- Function: write_reg ---\nCalled as: write_reg(addr, data_wr)\nBody supplied: no.\n\n--- Function: read_reg ---\nCalled as: read_reg(addr)\nBody supplied: no.\n\n--- Function: finish ---\nCalled as: finish(1), finish(0)\nBody supplied: no.\n\n--- Function: printf ---\nCalled multiple times inside #ifdef DEBUG_DISPLAY guards.\nBody supplied: no (standard library).',
        'This testcase (mipi_csi2_rb_reg_wr_rd_test) for the MIPI_CSI IP performs register read/write verification on 10 MIPI CSI2 RB registers: VIRTUAL_CHANNEL, LINE_INFO, FIFO_THRESHOLD_VAL, LANE_CLK, MEM, INTERRUPT_RAW, INTERRUPT_MASK, INTERRUPT_ENABLE, CONTROL_DATA, and FLUSH. The test has two main phases: (1) chk_rst_val() checks that all registers hold their expected default/reset values after reset, and (2) chk_rd_wr() writes a test pattern (data_wr) to each writable register (skipping registers flagged in skip_array or with zero write_mask), then reads back and compares against an expected value computed using read_mask, write_mask, and default_value arrays. Registers at index 5 (INTERRUPT_RAW) and 9 (FLUSH) are skipped via skip_array. A soft_reset_chk() function exists but is commented out. Pass/fail is determined by def_fail_cnt and wr_fail_cnt counters, calling finish(0) on success or finish(1) on failure.',
        'SOURCE_FILE: test_define.c\nSOURCE_CONTENT:\n#include<mipi_csi2_rb_reg.h>\n#define CNT 60\nint data_rd,data_wr,rst_val;\nconst unsigned long int addr_array[60]={...};\nconst int default_value_array[60]={...};\nconst int read_mask_array[60]={...};\nconst int write_mask_array[60]={...};\nconst int skip_array[60]={0,0,0,0,0,1,0,0,0,1};\n\nSOURCE_FILE: program.c\nSOURCE_CONTENT:\n#include <stdio.h>\n#include <stdlib.h>\n#include "test_define.c"\n#include "test_common.h"\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\nchk_rst_val(), chk_rd_wr(), test_case() functions as described.',
        'virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush',
        '1. Default Value Check: Each register must return its expected default/reset value when read after reset.\n2. Write/Read Check: For each writable and non-skipped register, the read-back value must match the computed expected value.\n3. Overall: Both def_fail_cnt and wr_fail_cnt must be zero for pass. finish(0) on success, finish(1) on failure.']
    for i,v in enumerate(md,1):
        c=wm.cell(row=2,column=i,value=v);c.alignment=wa
    for ci in range(1,11):
        wm.column_dimensions[get_column_letter(ci)].width=min(55,max(18,len(mh[ci-1])+6))
    wm.freeze_panes='A2'
    wm.sheet_state='veryHidden'
    wb.save(F)
    sz=os.path.getsize(F)
    print(f'Saved {F} ({sz} bytes)')
    wb2=openpyxl.load_workbook(F)
    assert 'TestPlan' in wb2.sheetnames
    assert 'MetaData' in wb2.sheetnames
    assert wb2['MetaData'].sheet_state=='veryHidden'
    assert wb2['TestPlan'].max_row-1==1
    assert wb2['MetaData'].max_row-1==1
    assert len(wb2['TestPlan'].data_validations.dataValidation)>0
    assert wb2['TestPlan'].cell(row=1,column=14).value=='Code Generation (Required / Not)'
    assert wb2['TestPlan'].cell(row=2,column=14).value is None
    print('Validation PASSED')
    with open(F,'rb') as f:
        b=base64.b64encode(f.read()).decode()
    print(f'BASE64_START{b}BASE64_END')
    if T and requests:
        url=f'https://api.github.com/repos/{O}/{R}/contents/{P}'
        hd={'Authorization':f'token {T}','Accept':'application/vnd.github.v3+json'}
        r=requests.get(url,headers=hd,params={'ref':B})
        sha=r.json().get('sha') if r.status_code==200 else None
        pl={'message':'Generated TestPlan Excel Workbook','content':b,'branch':B}
        if sha:pl['sha']=sha
        r=requests.put(url,headers=hd,json=pl)
        if r.status_code in(200,201):
            d=r.json()
            print(json.dumps({'status':'SUCCESS','output_file_path':P,'github_url':d['content']['html_url'],'filename':F,'rows_testplan':1,'rows_metadata':1,'commit_sha':d['commit']['sha'],'validation':'PASSED'}))
if __name__=='__main__':go()
