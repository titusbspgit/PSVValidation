#!/usr/bin/env python3
"""
Standalone XLSX generator - creates MIPI_CSI_TestPlan Excel and pushes to GitHub.
Run locally: python create_xlsx_now.py
"""
import os, sys, json, base64, io
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"

json_data = [{"Index":"1","SS / Module":"MIPI CSI2 Subsystem","Test Case Name":"mipi_csi2_test_pattern_generator","Feature":"NA","Meta Headers":"#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"","Meta Macros":"#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000","Meta Arrays":"NA","Speed":"NA","Mode":"NA","Memory Start Offset":"NA","Memory End Offset":"NA","Meta Test Description":"This testcase configures and enables the MIPI CSI-2 internal test pattern generator (PG) to generate a known image pattern, routes the generated data through the CSI-2 subsystem via a selected virtual channel and GDMA path, programs the DMA engine to transfer the received pixel data from the CSI-2 receive buffer to a destination memory address, waits for the DMA transfer completion interrupt, and then finishes.","Test Description":"Configure and enable the MIPI CSI-2 internal test pattern generator to produce a known image pattern (320x16 pixels, 24 bits per pixel). Route the generated data through a selected virtual channel and GDMA path. Program the DMA engine to transfer received pixel data to destination memory. Enable all relevant CSI-2 and DMA interrupts. Initialize the D-PHY and wait for stop-state. Start the pattern generator, wait, then disable it. Poll for DMA transfer completion interrupt. Wait and finish the test.","Meta Test Steps / Procedure":"1. Global variable declarations\n2. Function void csi2_ctrlr_pg_enable()\n3. Function int test_case()\n4. Function void csi2_enable_interrupt()","Test Steps / Procedure":"1. Set interrupt pending flag.\n2. Set virtual channel ID (vcid=3) and compute the unselected path value.\n3. Based on the selected GDMA path, compute the virtual channel wrapper register value.\n4. Compute gdma_reg_base address.\n5. Write virtual channel value to register.\n6. Disable control data transfer.\n7. Enable CSI-2 and DMA interrupts.\n8. Initialize the SNPS D-PHY.\n9. Poll PHY_STOPSTATE register until 0x1000f.\n10. Set image parameters.\n11. Compute DMA transfer size.\n12. Program DMA higher-order address registers.\n13. Enable fracdiv output.\n14. Set DMA channel 0 program counter.\n15. Configure DMA transfer instruction preload.\n16. Issue DMAGO command.\n17. Enable pattern generator.\n18. Wait 100 cycles.\n19. Disable pattern generator.\n20. Poll DMA interrupt status.\n21. Wait 10000 cycles.\n22. Call finish(0).","Meta Impacted Registers":"PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, PPI_PG_ENABLE, virtual_channel, control_data, PHY_STOPSTATE, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, INT_ST_MAIN, INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED","Impacted Registers":"PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, PPI_PG_ENABLE, virtual_channel, control_data, PHY_STOPSTATE, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, INT_ST_MAIN, INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED","Meta Validation / Acceptance Criteria":"1. PHY stop-state polling must return 0x1000f.\n2. DMA transfer completion polling bit 0 must be set.\n3. Test completes by calling finish(0).","Validation / Acceptance Criteria":"1. The PHY_STOPSTATE register must read 0x1000f.\n2. The DMA interrupt masked status register bit 0 must be set.\n3. The test finishes with finish(0).","Remarks":"The testcase uses conditional compilation for GDMA path selection and FPS60 flag.","Code Generation":""}]

testplan_columns = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation"]
metadata_columns = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]

wb = Workbook()
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')

ws_tp = wb.active
ws_tp.title = "TestPlan"
for ci, cn in enumerate(testplan_columns, 1):
    c = ws_tp.cell(row=1, column=ci, value=cn)
    c.font = header_font; c.fill = header_fill; c.alignment = wrap_align
for ri, rd in enumerate(json_data, 2):
    for ci, cn in enumerate(testplan_columns, 1):
        c = ws_tp.cell(row=ri, column=ci, value=rd.get(cn, ""))
        c.alignment = wrap_align
ws_tp.freeze_panes = 'A2'
for ci, cn in enumerate(testplan_columns, 1):
    ml = len(cn)
    for r in range(2, len(json_data)+2):
        v = ws_tp.cell(row=r, column=ci).value
        if v:
            for ln in str(v).split('\n'): ml = max(ml, len(ln))
    ws_tp.column_dimensions[get_column_letter(ci)].width = max(min(ml+2, 60), 12)

ws_md = wb.create_sheet(title="MetaData")
for ci, cn in enumerate(metadata_columns, 1):
    c = ws_md.cell(row=1, column=ci, value=cn)
    c.font = header_font; c.fill = header_fill; c.alignment = wrap_align
for ri, rd in enumerate(json_data, 2):
    for ci, cn in enumerate(metadata_columns, 1):
        c = ws_md.cell(row=ri, column=ci, value=rd.get(cn, ""))
        c.alignment = wrap_align
ws_md.freeze_panes = 'A2'
for ci, cn in enumerate(metadata_columns, 1):
    ml = len(cn)
    for r in range(2, len(json_data)+2):
        v = ws_md.cell(row=r, column=ci).value
        if v:
            for ln in str(v).split('\n'): ml = max(ml, len(ln))
    ws_md.column_dimensions[get_column_letter(ci)].width = max(min(ml+2, 60), 12)
ws_md.sheet_state = 'veryHidden'

# Save to BytesIO
buf = io.BytesIO()
wb.save(buf)
buf.seek(0)
xlsx_bytes = buf.read()

# Save to file
filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
with open(filepath, 'wb') as f:
    f.write(xlsx_bytes)

# Validate
assert os.path.exists(filepath) and os.path.getsize(filepath) > 0
wb2 = load_workbook(filepath)
assert "TestPlan" in wb2.sheetnames and "MetaData" in wb2.sheetnames
print(f"SUCCESS: {filename} ({os.path.getsize(filepath)} bytes)")
print(f"TestPlan rows: {wb2['TestPlan'].max_row - 1}")
print(f"MetaData rows: {wb2['MetaData'].max_row - 1}")
print(f"B64:{base64.b64encode(xlsx_bytes).decode()}")
