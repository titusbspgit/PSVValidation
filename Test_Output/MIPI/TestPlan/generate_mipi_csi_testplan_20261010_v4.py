#!/usr/bin/env python3
"""
Agent 7 - Excel Generator Script
Generates MIPI_CSI_TestPlan XLSX workbook using openpyxl
Run: python generate_mipi_csi_testplan_20261010_v4.py
Output: MIPI_CSI_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"

# Input JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "mipi_csi2_subsys",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "Test Pattern Generator",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates the MIPI CSI-2 subsystem internal test pattern generator (PG) functionality. The test configures the CSI-2 subsystem-level virtual channel routing registers, enables CSI-2 and DMA interrupts via csi2_subsys_enable_interrupt(), initializes the SNPS D-PHY via snps_phy_init(), and polls PHY_STOPSTATE register until the PHY enters stop state (value 0x1000f). It then computes the DMA transfer size based on hres=320, vres=16, valid_bits_per_pixel=24, programs higher-order ARM DMA address bits for channel 0 (AR data=0x100, AR instruction=0x0, AW data=0x0, AW instruction=0x0), enables fracdiv output to CSI2 subsystem via MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 = 0x1, preloads DMA transfer instructions via dma_trnsfr_instn_preload_incr_addr(), issues DMAGO via DMAGO_CSI(), enables the pattern generator by writing PG_PATTERN_VRES=0x10, PG_PATTERN_HRES=0x70140, PG_CONFIG=0xe401, PG_ENABLE=1, waits 100 cycles via wait_on(100), disables the pattern generator by writing PG_ENABLE=0, polls the DMA interrupt masked status register (gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) until bit 0 is set indicating DMA transfer completion, waits 10000 cycles via wait_on(10000), and finishes with finish(0). A local function csi2_enable_interrupt() is also defined which reads INT_ST_MAIN to clear interrupts and writes interrupt mask registers for PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, and ECC_CORRECTED. The test_case() function calls csi2_subsys_enable_interrupt() (external) rather than the locally defined csi2_enable_interrupt().",
        "Test Description": "Validates the MIPI CSI-2 internal test pattern generator by configuring virtual channel routing, initializing the D-PHY, programming DMA channel 0 for data transfer, enabling the pattern generator with specific vertical and horizontal resolution settings, and verifying DMA transfer completion through interrupt status polling.",
        "Meta Test Steps / Procedure": "1. Set global variable int_pend = 1.\n2. printf(\"start line\\n\").\n3. Set local variable vcid = 3.\n4. Compute vcid_unselected_path = ((vcid + 1) & 0xf), which equals 4.\n5. Conditional compilation for GDMA path selection:\n - If GDMA3_PATH defined: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+vcid); gdma_path = 3.\n - Elif GDMA2_PATH defined: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid<<4)+(vcid_unselected_path)); gdma_path = 2.\n - Elif GDMA1_PATH defined: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path)); gdma_path = 1.\n - Else (GDMA0_PATH default): vcid_csi2_wrap_reg = ((vcid<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path)); gdma_path = 0.\n6. Compute gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000).\n7. printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg).\n8. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\n9. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0).\n10. Call csi2_subsys_enable_interrupt().\n11. Call snps_phy_init().\n12. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE).\n13. while(!(rd_data == 0x1000f)) { rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE); }\n14. Set hres = 320.\n15. Set vres = 16.\n16. Set valid_bits_per_pixel = 24.\n17. Compute csi2_data_trnsfr_size = 15360 bytes.\n18. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100).\n19. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0).\n20. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0).\n21. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0).\n22. write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1).\n23-27. DMA preload and DMAGO.\n28. csi2_ctrlr_pg_enable().\n29. wait_on(100).\n30. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0).\n31-32. Poll DMA interrupt.\n33. wait_on(10000).\n34. finish(0).",
        "Test Steps / Procedure": "1. Initialize interrupt pending flag and set virtual channel ID to 3.\n2. Compute virtual channel routing register value based on the selected GDMA path (GDMA0/1/2/3) and write the virtual channel register.\n3. Disable control data transfer by writing 0 to the control data register.\n4. Enable CSI-2 subsystem and DMA interrupts.\n5. Initialize the SNPS D-PHY.\n6. Poll the PHY stop state register until all lanes enter stop state (expected value 0x1000f).\n7. Set image parameters: horizontal resolution = 320, vertical resolution = 16, bits per pixel = 24.\n8. Compute DMA transfer size based on image parameters (byte-aligned to 8-byte boundary).\n9. Program higher-order ARM DMA address bits for channel 0 read and write paths.\n10. Enable fracdiv output to CSI2 subsystem.\n11. Preload DMA transfer instructions with source address 0x00, destination address 0xE6001000, and computed transfer size.\n12. Issue DMAGO command for DMA channel 0.\n13. Enable the pattern generator by configuring vertical resolution, horizontal resolution, pattern configuration, and PG enable registers.\n14. Wait 100 cycles.\n15. Disable the pattern generator.\n16. Poll the DMA interrupt masked status register until bit 0 is set, indicating DMA transfer completion.\n17. Wait 10000 cycles.\n18. Finish the test with pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE polling: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) must return 0x1000f to exit the while loop.\n2. DMA interrupt polling: read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled until (rd_data & 0x1) != 0.\n3. Test completion: finish(0) is called with argument 0, indicating test pass.\n4. The locally defined csi2_enable_interrupt() function validates interrupt configuration by reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN.",
        "Validation / Acceptance Criteria": "1. The D-PHY must enter stop state on all lanes, confirmed by the PHY stop state register reading the expected value.\n2. DMA channel 0 transfer must complete successfully, confirmed by the DMA interrupt masked status register bit 0 being set.\n3. The test must complete with a pass status via the finish call.",
        "Remarks": "The test_case() function calls csi2_subsys_enable_interrupt() which is an external function (not the locally defined csi2_enable_interrupt()). The locally defined csi2_enable_interrupt() is present in program.c but is not invoked from the test entry point. External functions snps_phy_init(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), and finish() are called but their implementations are not available in the testcase folder. The GDMA path is selected via conditional compilation (GDMA0_PATH/GDMA1_PATH/GDMA2_PATH/GDMA3_PATH). The FPS60 macro controls whether destination address increment is enabled. The DMA interrupt status register is accessed via gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET, where gdma_reg_base is computed at runtime and MIPI_CSI2_DMA_INTMIS_OFFSET is defined externally. MIZAR_MIPI_CSI2_RB_REG_BASE is used with offset 0xf4 for fracdiv enable but its Agent 4 mapping is unresolved.",
        "Code Generation": ""
    }
]

# TestPlan sheet columns
testplan_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# MetaData sheet columns
metadata_columns = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Header formatting
header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# Write TestPlan headers
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns with max width cap
MAX_WIDTH = 60
for col_idx, col_name in enumerate(testplan_columns, 1):
    max_len = len(col_name)
    for row in ws_tp.iter_rows(min_row=2, max_row=ws_tp.max_row, min_col=col_idx, max_col=col_idx):
        for cell in row:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
    adjusted_width = min(max_len + 2, MAX_WIDTH)
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

# --- MetaData Sheet ---
ws_md = wb.create_sheet(title="MetaData")

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_md.freeze_panes = "A2"

# Auto-size columns
for col_idx, col_name in enumerate(metadata_columns, 1):
    max_len = len(col_name)
    for row in ws_md.iter_rows(min_row=2, max_row=ws_md.max_row, min_col=col_idx, max_col=col_idx):
        for cell in row:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
    adjusted_width = min(max_len + 2, MAX_WIDTH)
    ws_md.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save workbook
output_path = filename
wb.save(output_path)
print(f"SUCCESS: Workbook saved as {output_path}")
print(f"File size: {os.path.getsize(output_path)} bytes")

# Validation
try:
    wb_check = load_workbook(output_path)
    sheets = wb_check.sheetnames
    print(f"Sheets: {sheets}")
    assert "TestPlan" in sheets, "TestPlan sheet missing"
    assert "MetaData" in sheets, "MetaData sheet missing"
    tp_rows = wb_check["TestPlan"].max_row - 1
    md_rows = wb_check["MetaData"].max_row - 1
    print(f"TestPlan rows: {tp_rows}")
    print(f"MetaData rows: {md_rows}")
    print(f"MetaData state: {wb_check['MetaData'].sheet_state}")
    print("VALIDATION: PASSED")
    wb_check.close()
except Exception as e:
    print(f"VALIDATION: FAILED - {e}")

if __name__ == "__main__":
    print(f"\nGenerated: {filename}")
    print(f"Timestamp (IST): {now_ist.strftime('%Y-%m-%d %H:%M:%S %Z')}")
