#!/usr/bin/env python3
"""Agent 7 - Inline xlsx generator. Run: python agent7_generate_xlsx_inline.py
Requires: pip install openpyxl
This script generates the xlsx locally in the current directory."""
import io, os, sys, json, zipfile, base64
from datetime import datetime, timezone, timedelta

# Build xlsx from raw XML components (no openpyxl needed)
def build_xlsx_raw():
    """Construct a genuine xlsx (ZIP of XML) with TestPlan + veryHidden MetaData sheets."""
    
    # Helper to escape XML
    def esc(s):
        return str(s).replace('&','&amp;').replace('<','&lt;').replace('>','&gt;').replace('"','&quot;').replace("'",'&apos;')
    
    # === DATA ===
    tp_headers = ["Index","SS / Module","Feature","Test Case Name","Speed","Mode","Memory Start Offset","Memory End Offset","Test Description","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Remarks","Code Generation (Required / Not)"]
    
    tp_row = ["1","MIPI_CSI","Register Read/Write Validation (RB Register Block)","mipi_csi2_rb_reg_wr_rd_test","NA","NA","NA","NA",
        "This test performs register-level read/write validation on 60 MIPI CSI2 Receive Block (RB) registers. The test executes in two active phases: Phase 1 verifies that all 60 registers contain their expected default/reset values after power-on, skipping any registers that are not readable. Phase 2 writes six distinct test data patterns to each writable register, reads back the values, and compares the read data against expected values computed using the register read mask, write mask, and default value properties. Certain registers are excluded from the write/read phase via a skip list. A soft reset check function is defined but not actively invoked in the current test flow.",
        "Step 1: Execute the default value check routine. For each of the 60 RB registers, read the register value. If the register is non-readable (read mask is zero), skip it. Compare the read value against the expected default value for that register. If a mismatch is detected, increment the default-fail counter and log the failure.\n\nStep 2: Execute the write/read validation routine. Iterate over six test data patterns (all-ones, alternating 0xAA, alternating 0x55, all-zeros, checkerboard 0xA5, and upper-half 0xFFFF0000). For each pattern, write the pattern to each of the 60 registers, skipping registers that are in the skip list or have a zero write mask. Then read back each register, skipping non-writable and non-readable registers. Compute the expected read-back value using the write mask, read mask, and default value. Compare the actual read value against the computed expected value. If a mismatch is detected, increment the write-fail counter and log the failure.\n\nStep 3: Evaluate overall test result. If both the default-fail counter and write-fail counter are zero, the test passes. If either counter is greater than zero, the test fails.",
        "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0, boot_addr_VC1, boot_addr_VC2, boot_addr_VC3, boot_irq_ns_VC0, boot_irq_ns_VC1, boot_irq_ns_VC2, boot_irq_ns_VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m1_addr_ar_ch0_data, dma_m1_addr_ar_ch0_Instruction, dma_m2_addr_ar_ch0_data, dma_m2_addr_ar_ch0_Instruction, dma_m3_addr_ar_ch0_data, dma_m3_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, dma_m1_addr_aw_ch0_data, dma_m1_addr_aw_ch0_Instruction, dma_m2_addr_aw_ch0_data, dma_m2_addr_aw_ch0_Instruction, dma_m3_addr_aw_ch0_data, dma_m3_addr_aw_ch0_Instruction, dma_m0_addr_ar_ch1_data, dma_m0_addr_ar_ch1_Instruction, dma_m1_addr_ar_ch1_data, dma_m1_addr_ar_ch1_Instruction, dma_m2_addr_ar_ch1_data, dma_m2_addr_ar_ch1_Instruction, dma_m3_addr_ar_ch1_data, dma_m3_addr_ar_ch1_Instruction, dma_m0_addr_aw_ch1_data, dma_m0_addr_aw_ch1_Instruction, dma_m1_addr_aw_ch1_data, dma_m1_addr_aw_ch1_Instruction, dma_m2_addr_aw_ch1_data, dma_m2_addr_aw_ch1_Instruction, dma_m3_addr_aw_ch1_data, dma_m3_addr_aw_ch1_Instruction, dphy_rx",
        "1. All 60 readable registers must return their expected default values after reset; the default-fail counter must remain zero.\n2. For each of the six test data patterns, every writable and readable register (excluding those in the skip list) must return a read-back value that matches the computed expected value based on write mask, read mask, and default value; the write-fail counter must remain zero.\n3. The test passes (finish with success) if and only if both the default-fail counter and the write-fail counter are zero. Any non-zero counter results in test failure.\n4. Three registers at skip list indices 5, 9, and 10 are intentionally excluded from the write/read validation phase and are not evaluated for write-back correctness.",
        "The soft reset check function is defined in the test source but is commented out and not executed in the current test flow. Six test data patterns are used for comprehensive bit-level coverage: all-ones, alternating-AA, alternating-55, all-zeros, checkerboard-A5, and upper-half-ones. The expected read-back value computation accounts for read-only bits retaining their default values while writable bits reflect the written pattern. A total of 60 registers spanning the MIPI CSI2 RB register block are covered, including virtual channel configuration, interrupt control, FIFO threshold, DPHY configuration, DMA address mapping, and boot configuration registers.",
        ""  # Code Generation - blank default
    ]
    
    meta_headers = ["Test Case Name","Meta Headers","Meta Macros","Meta Arrays","Meta Types","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria"]
    
    meta_row = [
        "mipi_csi2_rb_reg_wr_rd_test",
        "SOURCE_FILE: program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include \"test_define.c\"\n4. #include \"test_common.h\"\n\nSOURCE_FILE: test_define.c\n1. #include<mipi_csi2_rb_reg.h>",
        "SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA, CNT, DEBUG_DISPLAY, and 245 register macros (address, read_mask, write_mask, default_val) for 60 MIPI CSI2 RB registers. Total: 249 macros.",
        "addr_array[60], read_mask_array[60], write_mask_array[60], default_value_array[60], skip_array[60], chk_val[6]",
        "int, unsigned long int, const int",
        "Register-level read/write validation on 60 MIPI CSI2 RB registers. Phase 1: Default value check. Phase 2: Write/Read with 6 test patterns. Phase 3: soft_reset_chk (commented out).",
        "1. chk_rst_val() - Read all 60 registers, compare against default_value_array. 2. chk_rd_wr() - Write 6 patterns, read back, verify using mask computation. 3. test_case() evaluates def_fail_cnt and wr_fail_cnt, calls finish(0) on success or finish(1) on failure.",
        "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0-VC3, boot_irq_ns_VC0-VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m[0-3]_addr_ar/aw_ch[0-1]_data/Instruction, dphy_rx",
        "Pass: def_fail_cnt == 0 AND wr_fail_cnt == 0 -> finish(0). Fail: def_fail_cnt > 0 OR wr_fail_cnt > 0 -> finish(1)."
    ]
    
    # === Build shared strings ===
    all_strings = tp_headers + tp_row + meta_headers + meta_row
    ss_xml = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="' + str(len(all_strings)) + '" uniqueCount="' + str(len(all_strings)) + '">'
    for s in all_strings:
        ss_xml += '<si><t xml:space="preserve">' + esc(s) + '</t></si>'
    ss_xml += '</sst>'
    
    # === Build styles (header: bold white on blue, body: wrap text) ===
    styles_xml = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
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
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
</styleSheet>'''
    
    # === Build Sheet1 (TestPlan) ===
    def col_ref(c): return chr(64+c) if c<=26 else chr(64+(c-1)//26)+chr(65+(c-1)%26)
    
    sheet1 = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
    sheet1 += '<sheetViews><sheetView tabSelected="1" workbookViewId="0"><pane ySplit="1" topLeftCell="A2" activePane="bottomLeft" state="frozen"/></sheetView></sheetViews>'
    sheet1 += '<sheetData>'
    # Header row
    sheet1 += '<row r="1">'
    for ci, h in enumerate(tp_headers):
        si = ci  # shared string index
        cr = col_ref(ci+1)
        sheet1 += f'<c r="{cr}1" t="s" s="1"><v>{si}</v></c>'
    sheet1 += '</row>'
    # Data row
    sheet1 += '<row r="2">'
    base_si = len(tp_headers)
    for ci, v in enumerate(tp_row):
        si = base_si + ci
        cr = col_ref(ci+1)
        sheet1 += f'<c r="{cr}2" t="s" s="2"><v>{si}</v></c>'
    sheet1 += '</row>'
    sheet1 += '</sheetData>'
    # Data validation for Code Generation column (column N = col 14)
    cg_ref = col_ref(len(tp_headers))
    sheet1 += f'<dataValidations count="1"><dataValidation type="list" allowBlank="1" showInputMessage="1" showErrorMessage="1" sqref="{cg_ref}2:{cg_ref}2"><formula1>"Required,Not Required"</formula1></dataValidation></dataValidations>'
    sheet1 += '</worksheet>'
    
    # === Build Sheet2 (MetaData) ===
    sheet2 = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
    sheet2 += '<sheetViews><sheetView workbookViewId="0"><pane ySplit="1" topLeftCell="A2" activePane="bottomLeft" state="frozen"/></sheetView></sheetViews>'
    sheet2 += '<sheetData>'
    meta_base = len(tp_headers) + len(tp_row)
    sheet2 += '<row r="1">'
    for ci, h in enumerate(meta_headers):
        si = meta_base + ci
        cr = col_ref(ci+1)
        sheet2 += f'<c r="{cr}1" t="s" s="1"><v>{si}</v></c>'
    sheet2 += '</row>'
    sheet2 += '<row r="2">'
    meta_data_base = meta_base + len(meta_headers)
    for ci, v in enumerate(meta_row):
        si = meta_data_base + ci
        cr = col_ref(ci+1)
        sheet2 += f'<c r="{cr}2" t="s" s="2"><v>{si}</v></c>'
    sheet2 += '</row>'
    sheet2 += '</sheetData></worksheet>'
    
    # === Workbook ===
    wb_xml = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets>
<sheet name="TestPlan" sheetId="1" r:id="rId1"/>
<sheet name="MetaData" sheetId="2" state="veryHidden" r:id="rId2"/>
</sheets>
</workbook>'''
    
    # === Relationships ===
    wb_rels = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet2.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>'''
    
    root_rels = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''
    
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
    
    # === Assemble ZIP ===
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, 'w', zipfile.ZIP_DEFLATED) as zf:
        zf.writestr('[Content_Types].xml', content_types)
        zf.writestr('_rels/.rels', root_rels)
        zf.writestr('xl/workbook.xml', wb_xml)
        zf.writestr('xl/_rels/workbook.xml.rels', wb_rels)
        zf.writestr('xl/worksheets/sheet1.xml', sheet1)
        zf.writestr('xl/worksheets/sheet2.xml', sheet2)
        zf.writestr('xl/styles.xml', styles_xml)
        zf.writestr('xl/sharedStrings.xml', ss_xml)
    
    return buf.getvalue()

def main():
    IST = timezone(timedelta(hours=5, minutes=30))
    ts = datetime.now(IST).strftime('%Y%m%d_%H%M%S')
    fn = f'MIPI_CSI_TestPlan_{ts}.xlsx'
    
    xlsx_bytes = build_xlsx_raw()
    
    with open(fn, 'wb') as f:
        f.write(xlsx_bytes)
    
    sz = os.path.getsize(fn)
    print(f'Generated {fn} ({sz} bytes)')
    print(f'Base64 length: {len(base64.b64encode(xlsx_bytes))}')
    
    # Print base64 for upload
    print('---BASE64_START---')
    print(base64.b64encode(xlsx_bytes).decode('ascii'))
    print('---BASE64_END---')
    
    return fn, sz

if __name__ == '__main__':
    main()
