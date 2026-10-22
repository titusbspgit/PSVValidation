#!/usr/bin/env python3
"""
Creates MIPI_CSI TestPlan xlsx and outputs base64 to stdout.
Usage: python3 create_xlsx_binary.py | base64 -d > MIPI_CSI_TestPlan.xlsx
Or:    python3 create_xlsx_binary.py --save
"""
import zipfile, io, base64, sys, xml.etree.ElementTree as ET
from xml.dom.minidom import parseString

def esc(s):
    if s is None: return ""
    return str(s).replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace('"',"&quot;")

# Shared strings collector
ss_list = []
ss_map = {}
def ss_idx(val):
    v = str(val) if val is not None else ""
    if v not in ss_map:
        ss_map[v] = len(ss_list)
        ss_list.append(v)
    return ss_map[v]

# DATA
TP_COLS = ["Index","SS / Module","Feature","Test Case Name","Speed","Mode","Memory Start Offset","Memory End Offset","Test Description","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Remarks"]
TP_DATA = [
["1","MIPI_CSI","DPHY Lane Configuration and DMA Data Transfer Validation","mipi_csi2_dphy_lanes_test","NA","NA","NA","NA","This testcase validates MIPI CSI-2 DPHY lane configurations by iterating from 4 active lanes down to 1 active lane. For each lane configuration, the test configures the CSI-2 host controller N_LANES register, sets up DMA transfers for both control data and CSI pixel data on separate DMA channels, triggers the CSI-2 receive sequence, and validates DMA transfer completion via interrupt status polling on both DMA IRQ channels. The test initializes the D-PHY, polls PHY_STOPSTATE until all data lanes and the clock lane enter stop state, configures the virtual channel register and enables control data transfer, and then loops through each lane configuration performing DMA data transfers with data comparison validation using failure counters.","1. Enable CSI-2 interrupts.\n2. Select the GDMA path and configure the virtual channel ID mapping based on the selected path.\n3. Write the virtual channel register with the computed virtual channel mapping.\n4. Enable control data transfer by writing to the control data register.\n5. Initialize the D-PHY.\n6. Poll the PHY stop state register until all data lanes and the clock lane report stop state.\n7. Begin lane iteration loop from 4 active lanes down to 1 active lane.\n8. For each lane configuration, write the N_LANES register with the current lane count.\n9. Calculate the control packet count based on vertical resolution.\n10. Trigger the CSI-2 receive sequence for the current lane configuration.\n11. For each control packet iteration, enable DMA interrupts for both channels.\n12. Program DMA transfer instructions for control data on channel 0 and CSI pixel data on channel 1.\n13. Start the DMA transfer.\n14. Poll the DMA interrupt status register until the control packet transfer completes (IRQ channel 0).\n15. Clear the control packet DMA interrupt.\n16. Poll the DMA interrupt status register until the data packet transfer completes (IRQ channel 1).\n17. Clear the data packet DMA interrupt.\n18. Repeat for all packets in the current lane configuration.\n19. Repeat for all lane configurations (4 lanes to 1 lane).\n20. End the test.","DATA_IDS_VC_1, PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES","1. PHY stop state register must report all data lanes and clock lane in stop state before proceeding with lane configuration.\n2. DMA interrupt for control packet transfer (IRQ channel 0) must assert for each packet iteration, confirming successful control data DMA completion.\n3. DMA interrupt for data packet transfer (IRQ channel 1) must assert for each packet iteration, confirming successful CSI pixel data DMA completion.\n4. DMA interrupts must be successfully cleared after each transfer completion.\n5. Data comparison between written and read-back values must pass with zero default failure count and zero write failure count.\n6. The test must successfully complete DMA transfers for all lane configurations from 4 lanes down to 1 lane.\n7. The test must complete successfully with a pass indication.","The test uses compile-time configuration for vertical resolution, horizontal resolution, virtual channel ID, data type (RGB888), and frame count. The GDMA path is selected via preprocessor conditionals. The control packet count is derived from the vertical resolution. No timeout mechanism is implemented for PHY stop state or DMA interrupt polling loops."],
["2","MIPI_CSI","Internal Test Pattern Generator and DMA Data Transfer Validation","mipi_csi2_test_pattern_generator","NA","NA","NA","NA","This testcase validates the MIPI CSI-2 internal test pattern generator functionality. The test configures virtual channel IDs and GDMA path selection, writes the CSI-2 wrapper virtual channel register, and disables control data transfer. It computes the CSI-2 data transfer size based on a horizontal resolution of 320, vertical resolution of 16, and 24 valid bits per pixel with 8-byte alignment, resulting in a total transfer size of 15360 bytes. The test programs DMA transfer instructions with conditional destination address increment, starts DMA channel 0, enables the pattern generator by configuring the vertical resolution, horizontal resolution, pattern generator configuration, and pattern generator enable registers, waits for data generation, disables the pattern generator, polls the DMA interrupt status for transfer completion, and finishes the test.","1. Enable CSI-2 interrupts.\n2. Set the virtual channel ID and compute the unselected path virtual channel value.\n3. Select the GDMA path and configure the virtual channel ID mapping based on the selected path.\n4. Write the virtual channel register with the computed virtual channel mapping.\n5. Disable control data transfer by writing to the control data register.\n6. Set horizontal resolution to 320, vertical resolution to 16, and valid bits per pixel to 24.\n7. Compute the CSI-2 data transfer size with 8-byte alignment.\n8. Configure the DMA destination address increment flag based on frame rate configuration.\n9. Program DMA transfer instructions for channel 0 with the computed transfer size.\n10. Start DMA channel 0.\n11. Enable the pattern generator by writing the vertical resolution register, horizontal resolution register, pattern generator configuration register, and pattern generator enable register.\n12. Wait for the pattern generator to produce data.\n13. Disable the pattern generator by clearing the pattern generator enable register.\n14. Poll the DMA interrupt status register until the data transfer completes (IRQ channel 0 asserts).\n15. Wait for a post-completion cooldown period.\n16. End the test.","NA","1. DMA interrupt for data transfer completion (IRQ channel 0) must assert, confirming successful DMA channel 0 transfer of pattern generator data.\n2. The pattern generator must be successfully enabled by writing the correct values to the vertical resolution, horizontal resolution, configuration, and enable registers.\n3. The pattern generator must be successfully disabled after the data generation wait period.\n4. The CSI-2 data transfer size must be correctly computed as 15360 bytes based on the configured resolution and bits per pixel with 8-byte alignment.\n5. The virtual channel register must be correctly configured based on the selected GDMA path.\n6. Control data transfer must be disabled for the pattern generator test flow.\n7. DMA channel 0 must be correctly programmed and started before the pattern generator is enabled.\n8. The test must complete successfully with a pass indication.","The test uses a fixed resolution of 320x16 with 24 bits per pixel (RGB888 format) for pattern generation. The GDMA path is selected via compile-time preprocessor conditionals. The DMA destination address increment behavior is conditional on the FPS60 compile-time configuration. No timeout mechanism is implemented for the DMA interrupt polling loop. All macros in this testcase resolved to non-register addresses and are therefore not mapped to any IP registers."]
]

MD_COLS = ["Test Case Name","Meta Headers","Meta Macros","Meta Arrays","Meta Types","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria"]
MD_DATA = [
["mipi_csi2_dphy_lanes_test","#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"","#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#ifdef DPHY_LANES_TEST\n#ifndef LS_LE_EN\n#define LS_LE_EN 1\n#endif\n#ifndef TOTAL_FRAME\n#define TOTAL_FRAME 1\n#endif\n#ifndef VC_ID\n#define VC_ID 3\n#endif\n#ifndef VRES\n#ifdef GDMA0_FULL_MEM\n#define VRES 1080\n#else\n#define VRES 3\n#endif\n#endif\n#ifndef HRES\n#ifdef GDMA0_FULL_MEM\n#define HRES 1920\n#else\n#define HRES 64\n#endif\n#endif\n#ifndef DATA_TYPE\n#define DATA_TYPE CSI2_RGB888\n#endif","NA","int data_rd, data_wr;\nint def_fail_cnt = 0, wr_fail_cnt = 0;\nint vcid_csi2_wrap_reg;\nint gdma_path;\nunsigned int ch0_pc, ch0_preload_loc;\nunsigned int ch1_pc, ch1_preload_loc;\nunsigned int gdma_reg_base;\nunsigned int rd_data;\nlong int csi_ctrl_data;\nint gdma_int_rsts;\nlong int word_count;\nint csi_data_size;\nlong long int rx_desc, tx_desc;\nlong long int gdma_tx_trnsfr_size, gdma_trnsfr_size;\nint cntrl_pkt_cnt;","This testcase validates MIPI CSI-2 DPHY lane configurations by iterating from 4 active lanes down to 1 active lane. For each lane configuration, the test configures the CSI-2 host controller N_LANES register, sets up DMA transfers using dma_trnsfr_instn_preload for both control data (channel 0) and CSI data (channel 1), triggers the CSI-2 sequence, and validates DMA transfer completion via interrupt status polling on both DMA IRQ channels. The test initializes the D-PHY via snps_phy_init(), polls PHY_STOPSTATE until all lanes and clock lane enter stop state (0x1000F), configures the virtual channel register and control data register, and then loops through lane configurations performing DMA data transfers with data comparison validation using failure counters (def_fail_cnt, wr_fail_cnt).","Step 1: Call printf(\"start line\\n\").\nStep 2: Call csi2_enable_interrupt() to enable CSI-2 interrupt.\nStep 3: GDMA path selection via preprocessor conditionals.\nStep 4: Set gdma_reg_base = 0xE6A00000.\nStep 5: write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\nStep 6: write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1).\nStep 7-8: Debug print and subsystem interrupt.\nStep 9: Call snps_phy_init().\nStep 10-11: Poll PHY_STOPSTATE until 0x1000f.\nStep 12-13: Set ch0_pc and ch1_pc.\nStep 14: Lane iteration loop (4 to 1 lane) with DMA transfers.\nStep 15: finish(0).","DATA_IDS_VC_1\nPPI_PG_PATTERN_VRES\nPPI_PG_PATTERN_HRES","Validation 1: PHY_STOPSTATE polling until 0x1000f.\nValidation 2: DMA IRQ[0] polling for control packet.\nValidation 3: DMA IRQ[1] polling for data packet.\nValidation 4-5: DMA interrupt clear.\nValidation 6: DMA status read.\nValidation 7: Data comparison (def_fail_cnt==0, wr_fail_cnt==0).\nValidation 8: Control packet count computation.\nValidation 9: Lane iteration validation.\nValidation 10: Test completion via finish(0)."],
["mipi_csi2_test_pattern_generator","#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"","#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000","NA","int data_rd, data_wr;\nint def_fail_cnt = 0, wr_fail_cnt = 0;\nint vcid_csi2_wrap_reg;\nint gdma_path;\nlong int csi_ctrl_data;\nint gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;\nunsigned int rd_data;\nunsigned int tx_trnsfr_size;\nlong long int rx_desc, tx_desc;\nlong long int gdma_tx_trnsfr_size, gdma_trnsfr_size;\nint cntrl_pkt_cnt;\nint int_pend;\nint vcid;\nint vcid_unselected_path;\nint hres, vres;\nint valid_bits_per_pixel;\nunsigned int dma_ch0_pc;\nunsigned int dma_ch0_instn_preload_addr;\nint dma_dest_addr_incr_flag;\nunsigned int csi2_data_trnsfr_size;\nunsigned int gdma_reg_base;","This testcase validates the MIPI CSI-2 internal test pattern generator functionality. The test configures virtual channel IDs and GDMA path selection via preprocessor conditionals, writes the CSI-2 wrapper virtual channel register and disables control data transfer. It computes the CSI-2 data transfer size based on hres=320, vres=16, and valid_bits_per_pixel=24 with 8-byte alignment. It programs DMA transfer instructions, starts DMA channel 0, enables the pattern generator, waits, disables it, polls DMA interrupt status for transfer completion, and finishes the test.","Step 1: printf(\"start line\\n\").\nStep 2: csi2_enable_interrupt().\nStep 3-5: Set int_pend, vcid, vcid_unselected_path.\nStep 6-7: GDMA path selection and gdma_reg_base computation.\nStep 8-9: Write virtual channel and control data registers.\nStep 10-13: Set hres=320, vres=16, bpp=24, compute transfer size.\nStep 14-16: Set DMA PC and dest addr incr flag.\nStep 17: Program DMA transfer instructions.\nStep 18: Start DMA channel 0.\nStep 19: Enable pattern generator (VRES, HRES, CONFIG, ENABLE).\nStep 20-21: Wait and disable pattern generator.\nStep 22-23: Poll DMA INTMIS for completion.\nStep 24-25: Post-wait and finish(0).","NA","Validation 1: DMA IRQ[0] polling for transfer completion.\nValidation 2: Pattern generator enable verification.\nValidation 3: Pattern generator disable verification.\nValidation 4: Transfer size computation (15360 bytes).\nValidation 5: Virtual channel register configuration.\nValidation 6: Control data disabled.\nValidation 7: DMA transfer programming.\nValidation 8: DMA channel start.\nValidation 9: Post-completion wait.\nValidation 10: Test completion via finish(0)."]
]

# Pre-populate shared strings
for row in [TP_COLS] + TP_DATA: 
    for v in row: ss_idx(v)
for row in [MD_COLS] + MD_DATA:
    for v in row: ss_idx(v)

def make_sheet_xml(cols, data, name):
    rows_xml = []
    # Header row
    cells = []
    for ci, v in enumerate(cols):
        col_letter = chr(65 + ci) if ci < 26 else chr(64 + ci//26) + chr(65 + ci%26)
        ref = f"{col_letter}1"
        cells.append(f'<c r="{ref}" t="s" s="1"><v>{ss_idx(v)}</v></c>')
    rows_xml.append(f'<row r="1">{"".join(cells)}</row>')
    # Data rows
    for ri, row in enumerate(data, start=2):
        cells = []
        for ci, v in enumerate(row):
            col_letter = chr(65 + ci) if ci < 26 else chr(64 + ci//26) + chr(65 + ci%26)
            ref = f"{col_letter}{ri}"
            cells.append(f'<c r="{ref}" t="s" s="2"><v>{ss_idx(v)}</v></c>')
        rows_xml.append(f'<row r="{ri}">{"".join(cells)}</row>')
    
    return f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheetViews><sheetView tabSelected="{"1" if name=="TestPlan" else "0"}" workbookViewId="0"><pane ySplit="1" topLeftCell="A2" activePane="bottomLeft" state="frozen"/></sheetView></sheetViews>
<sheetData>{"".join(rows_xml)}</sheetData>
</worksheet>'''

# Build shared strings XML
def make_shared_strings():
    items = []
    for s in ss_list:
        items.append(f"<si><t xml:space=\"preserve\">{esc(s)}</t></si>")
    return f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="{len(ss_list)}" uniqueCount="{len(ss_list)}">{"".join(items)}</sst>'''

content_types = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/worksheets/sheet2.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>
</Types>'''

rels = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

workbook = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets>
<sheet name="TestPlan" sheetId="1" r:id="rId1"/>
<sheet name="MetaData" sheetId="2" state="veryHidden" r:id="rId2"/>
</sheets>
</workbook>'''

wb_rels = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet2.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>'''

styles = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="2">
<font><sz val="11"/><name val="Calibri"/></font>
<font><b/><sz val="11"/><color rgb="FFFFFFFF"/><name val="Calibri"/></font>
</fonts>
<fills count="3">
<fill><patternFill patternType="none"/></fill>
<fill><patternFill patternType="gray125"/></fill>
<fill><patternFill patternType="solid"><fgColor rgb="FF4472C4"/></patternFill></fill>
</fills>
<borders count="2">
<border><left/><right/><top/><bottom/><diagonal/></border>
<border><left style="thin"><color auto="1"/></left><right style="thin"><color auto="1"/></right><top style="thin"><color auto="1"/></top><bottom style="thin"><color auto="1"/></bottom><diagonal/></border>
</borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="3">
<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>
<xf numFmtId="0" fontId="1" fillId="2" borderId="1" xfId="0" applyFont="1" applyFill="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center" wrapText="1"/></xf>
<xf numFmtId="0" fontId="0" fillId="0" borderId="1" xfId="0" applyBorder="1" applyAlignment="1"><alignment vertical="top" wrapText="1"/></xf>
</cellXfs>
</styleSheet>'''

sheet1 = make_sheet_xml(TP_COLS, TP_DATA, "TestPlan")
sheet2 = make_sheet_xml(MD_COLS, MD_DATA, "MetaData")
shared_strings = make_shared_strings()

buf = io.BytesIO()
with zipfile.ZipFile(buf, 'w', zipfile.ZIP_DEFLATED) as zf:
    zf.writestr("[Content_Types].xml", content_types)
    zf.writestr("_rels/.rels", rels)
    zf.writestr("xl/workbook.xml", workbook)
    zf.writestr("xl/_rels/workbook.xml.rels", wb_rels)
    zf.writestr("xl/worksheets/sheet1.xml", sheet1)
    zf.writestr("xl/worksheets/sheet2.xml", sheet2)
    zf.writestr("xl/styles.xml", styles)
    zf.writestr("xl/sharedStrings.xml", shared_strings)

xlsx_b64 = base64.b64encode(buf.getvalue()).decode("ascii")

if "--save" in sys.argv:
    from datetime import datetime, timezone, timedelta
    IST = timezone(timedelta(hours=5, minutes=30))
    ts = datetime.now(IST).strftime("%Y%m%d_%H%M%S")
    fn = f"MIPI_CSI_TestPlan_{ts}.xlsx"
    with open(fn, "wb") as f:
        f.write(buf.getvalue())
    print(f"Saved: {fn}", file=sys.stderr)

print(xlsx_b64)
