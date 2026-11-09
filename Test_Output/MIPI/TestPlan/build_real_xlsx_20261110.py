#!/usr/bin/env python3
"""
Builds the real MIPI_DSI_TestPlan_20261110_020000.xlsx and uploads to GitHub.
This is a one-shot script: run it once with GITHUB_TOKEN to replace the placeholder.

Usage:
  pip install openpyxl requests
  GITHUB_TOKEN=ghp_xxx python build_real_xlsx_20261110.py
"""
import os, sys, json, base64

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
except ImportError:
    print("pip install openpyxl"); sys.exit(1)

FILENAME = "MIPI_DSI_TestPlan_20261110_020000.xlsx"
REPO = "titusbspgit/PSVValidation"
PATH = f"Test_Output/MIPI/TestPlan/{FILENAME}"

TESTPLAN = [{"Index":"1","SS / Module":"MIPI_DSI","Test Case Name":"mipi_dsi_subsys_reg_wr_rd_test","Feature":"Register Read/Write Verification","Speed":"NA","Mode":"NA","Memory Start Offset":"NA","Memory End Offset":"NA","Test Description":"This test verifies register-level access for the MIPI DSI subsystem across 52 registers. It performs two main validation phases: (1) Default Value Check - reads all 52 registers after reset and compares each read value against its expected default/reset value, skipping registers that are not readable based on their read mask. Any mismatch is counted as a default value failure. (2) Write/Read-Back Check - writes six distinct test data patterns to all writable registers and reads back each register to verify correctness. Registers that are flagged to be skipped or have a zero write mask are excluded from the write/read-back phase. The expected read-back value accounts for writable and readable bit fields using the read mask, write mask, and default value of each register. Any mismatch is counted as a write/read failure. A soft reset check function is defined but is not invoked in the current test flow. The test reports overall pass if zero failures are detected across both phases, and overall fail otherwise.","Test Steps / Procedure":"Step 1: Initialize failure counters for default value check and write/read check to zero.\nStep 2: Execute the Default Value Check phase - iterate over all 52 MIPI DSI subsystem registers. For each register, check if the register is readable (read mask is non-zero). If not readable, skip to the next register. If readable, read the register and compare the read value against the expected default value. If a mismatch is detected, increment the default value failure counter and log the failure.\nStep 3: Execute the Write/Read-Back Check phase - for each of the six test data patterns (all-ones, alternating 0xAA, alternating 0x55, all-zeros, 0xA5A5A5A5, 0xFFFF0000), perform the following: (a) Write sub-phase: iterate over all 52 registers. Skip registers flagged in the skip list. Skip read-only registers (zero write mask). For all other registers, write the current test pattern. (b) Read-back sub-phase: iterate over all 52 registers again. Skip registers flagged in the skip list or that are read-only. Read each register and compute the expected value considering writable bits (via write mask), readable bits (via read mask), and default values for non-writable bit positions. Compare the read value against the computed expected value. If a mismatch is detected, increment the write/read failure counter and log the failure.\nStep 4: Evaluate overall result - if either the default value failure counter or the write/read failure counter is greater than zero, report test FAIL. Otherwise, report test PASS.","Impacted Registers":"data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host","Validation / Acceptance Criteria":"1. Default Value Verification: For each of the 52 registers, the value read after reset must exactly match the specified default value for that register. Registers with a zero read mask are excluded from this check.\n2. Write/Read-Back Verification: For each of the six test data patterns written, the value read back from each writable register must match the expected value computed as: expected = (written_data AND read_mask AND write_mask) OR (inverted_write_mask AND read_mask AND default_value). Registers in the skip list or with a zero write mask are excluded.\n3. Overall Pass Criteria: The test passes (finish with success) only if zero default value mismatches and zero write/read-back mismatches are detected across all registers and all test patterns. Any non-zero failure count results in test failure.","Remarks":"The test covers 52 MIPI DSI subsystem registers spanning data FIFO threshold, low power, DBI, interrupt, command FIFO, pixel data FIFO, DPI control, video parameters (0-8), FIFO flush, soft control reset, DMAC configuration, DPHY TX configuration, PHY lane, PLL configuration, and DSI host registers. Six test patterns are used to exercise bit toggling coverage. A soft reset check function exists in the test but is currently not invoked. Certain registers are flagged to be skipped during the write/read-back phase. Read-only registers are automatically excluded from write operations based on their write mask."}]

META = [{"Test Case Name":"mipi_dsi_subsys_reg_wr_rd_test","Meta Headers":"File: test_define.c\n#include<mipi_dsi_subsys.h>\n\nFile: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"","Meta Macros":"File: test_define.c\n#define CNT 52\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n\nConditional compilation macro referenced via #ifdef but definition not supplied in source:\nDEBUG_DISPLAY","Meta Arrays":"File: test_define.c (file scope)\n\nconst unsigned long int addr_array[52]={MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,...,MIZAR_MIPI_DSI_SUBSYS_DSI_HOST};\nconst int default_value_array[52]={...};\nconst int read_mask_array[52]={...};\nconst int write_mask_array[52]={...};\nconst int skip_array[52]={0,0,0,0,1,0,0,0,0,0,...};\n\nFile: program.c (local to chk_rd_wr function)\nint chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};","Meta Types":"TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;\n\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;","Meta Test Description":"This testcase performs register-level verification for the MIPI DSI subsystem across 52 registers. It consists of two main phases: (1) Default/reset value check (chk_rst_val) - iterates over all 52 registers via addr_array, reads each register using read_reg(), and compares against expected default values from default_value_array, skipping non-readable registers where read_mask_array is 0x00000000, incrementing def_fail_cnt on mismatch; (2) Write/read-back check (chk_rd_wr) - writes 6 test patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000) to all writable registers and reads back to verify, skipping registers flagged in skip_array or with zero write_mask_array, computing expected value as exp_val = (data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]) where wr_n = write_mask_array[i] ^ 0xffffffff, incrementing wr_fail_cnt on mismatch. A soft_reset_chk function is defined but commented out in test_case(). The test calls finish(1) on any failure (def_fail_cnt > 0 or wr_fail_cnt > 0) or finish(0) on success.","Meta Test Steps / Procedure":"The full source code content from test_define.c and program.c as provided by Agent 5A.","Meta Impacted Registers":"52 unique registers: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0 through vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host","Meta Validation / Acceptance Criteria":"chk_rst_val: For each register, read value must match default_value_array[i]. chk_rd_wr: For each of 6 test patterns written, read-back must match exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Final: finish(0) if no failures, finish(1) otherwise."}]

TP_COLS = ["Index","SS / Module","Feature","Test Case Name","Speed","Mode","Memory Start Offset","Memory End Offset","Test Description","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Remarks","Code Generation (Required / Not)"]
MT_COLS = ["Test Case Name","Meta Headers","Meta Macros","Meta Arrays","Meta Types","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria"]

HF = Font(name="Calibri",bold=True,color="FFFFFF",size=11)
HB = PatternFill(start_color="4472C4",end_color="4472C4",fill_type="solid")
WR = Alignment(wrap_text=True,vertical="top")

wb = Workbook()
ws1 = wb.active; ws1.title = "TestPlan"
for i,c in enumerate(TP_COLS,1):
    cl = ws1.cell(1,i,c); cl.font=HF; cl.fill=HB; cl.alignment=WR
ws1.freeze_panes = "A2"
for ri,tc in enumerate(TESTPLAN,2):
    for ci,cn in enumerate(TP_COLS,1):
        v = "" if cn=="Code Generation (Required / Not)" else tc.get(cn,"")
        ws1.cell(ri,ci,v).alignment = WR
dv = DataValidation(type="list",formula1='"Required,Not Required"',allow_blank=True,showDropDown=False)
for r in range(2,len(TESTPLAN)+2): dv.add(ws1.cell(r,len(TP_COLS)))
ws1.add_data_validation(dv)
for col in ws1.columns:
    mx=max((max((len(l) for l in str(c.value).split("\\n")),default=0) if c.value else 0) for c in col)
    ws1.column_dimensions[col[0].column_letter].width=min(mx+4,80)

ws2 = wb.create_sheet("MetaData")
for i,c in enumerate(MT_COLS,1):
    cl = ws2.cell(1,i,c); cl.font=HF; cl.fill=HB; cl.alignment=WR
ws2.freeze_panes = "A2"
for ri,mc in enumerate(META,2):
    for ci,cn in enumerate(MT_COLS,1):
        ws2.cell(ri,ci,mc.get(cn,"")).alignment = WR
for col in ws2.columns:
    mx=max((max((len(l) for l in str(c.value).split("\\n")),default=0) if c.value else 0) for c in col)
    ws2.column_dimensions[col[0].column_letter].width=min(mx+4,80)
ws2.sheet_state = "veryHidden"

wb.save(FILENAME)
sz = os.path.getsize(FILENAME)
print(f"Created {FILENAME} ({sz} bytes)")

# Validate
wb2 = load_workbook(FILENAME)
assert "TestPlan" in wb2.sheetnames
assert "MetaData" in wb2.sheetnames
assert wb2["TestPlan"].max_row-1 == len(TESTPLAN)
assert wb2["MetaData"].max_row-1 == len(META)
h = [wb2["TestPlan"].cell(1,c).value for c in range(1,wb2["TestPlan"].max_column+1)]
assert "Code Generation (Required / Not)" in h
assert wb2["TestPlan"].data_validations.dataValidation
wb2.close()
print("Validation: PASSED")

# Upload
token = os.environ.get("GITHUB_TOKEN","")
if not token:
    print("Set GITHUB_TOKEN to upload. Local file ready.")
    sys.exit(0)

import requests
with open(FILENAME,"rb") as f: b64=base64.b64encode(f.read()).decode()
url=f"https://api.github.com/repos/{REPO}/contents/{PATH}"
h={"Authorization":f"token {token}","Accept":"application/vnd.github.v3+json"}
r=requests.get(url,headers=h)
sha=r.json().get("sha") if r.status_code==200 else None
p={"message":"Generated TestPlan Excel Workbook","content":b64,"branch":"main"}
if sha: p["sha"]=sha
r=requests.put(url,headers=h,json=p)
if r.status_code in(200,201):
    d=r.json()
    print(f"Uploaded: {d['content']['html_url']}")
    print(f"Commit: {d['commit']['sha']}")
    print(f"Size: {d['content']['size']} bytes")
else:
    print(f"Failed: {r.status_code} {r.text}")
