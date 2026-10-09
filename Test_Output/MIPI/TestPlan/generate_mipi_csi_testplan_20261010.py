#!/usr/bin/env python3
"""
Agent 7 - Excel Generator: MIPI_CSI TestPlan XLSX Generator
Generates a real Office Open XML workbook (.xlsx) using openpyxl.
Triggered: 2026-10-09T14:15:00+05:30
"""
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

# ============================================================
# IST Timestamp
# ============================================================
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
IP_NAME = "MIPI_CSI"
filename = f"{IP_NAME}_TestPlan_{timestamp_str}.xlsx"
output_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(output_dir, filename)

# ============================================================
# Input JSON Data
# ============================================================
json_data = [
    {
        "Index": "2",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "Test Pattern Generator",
        "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "test_common.h"\n#include "mipi_csi2.h"',
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "0xE6000000",
        "Memory End Offset": "0xE6001000",
        "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator (PG) functionality. The test configures the CSI-2 subsystem virtual channel register, disables control data transfer, enables CSI-2 and DMA interrupts via csi2_subsys_enable_interrupt(), initializes the D-PHY via snps_phy_init(), and polls PHY_STOPSTATE until the value equals 0x1000f. It then computes the total data transfer size based on hres=320, vres=16, valid_bits_per_pixel=24, yielding csi2_data_trnsfr_size = (((24*320/8) aligned to 8-byte boundary) * 16) = 15360 bytes. The DMA higher-order address registers are programmed: DMA_M0_ADDR_AR_CH0_DATA=0x100, DMA_M0_ADDR_AR_CH0_INSTRUCTION=0x0, DMA_M0_ADDR_AW_CH0_DATA=0x0, DMA_M0_ADDR_AW_CH0_INSTRUCTION=0x0. The fracdiv output enable register at MIZAR_MIPI_CSI2_RB_REG_BASE+0xf4 is written with 0x1. DMA channel 0 is programmed via dma_trnsfr_instn_preload_incr_addr() with src_addr=0x00, dest_addr=0xE6001000, trnsfr_size=csi2_data_trnsfr_size, src_incr_addr_flag=0, dest_incr_addr_flag=1, irq_num=0. DMA channel 0 is started via DMAGO_CSI(). The pattern generator is then enabled by writing PPI_PG_PATTERN_VRES=0x10, PPI_PG_PATTERN_HRES=0x70140, PPI_PG_CONFIG=0xe401, PPI_PG_ENABLE=1. After wait_on(100), the pattern generator is disabled by writing PPI_PG_ENABLE=0. The test then polls gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET for bit 0 to confirm DMA channel 0 transfer completion. After completion, wait_on(10000) is called, followed by finish(0) to indicate test pass. The file also defines csi2_enable_interrupt() which reads INT_ST_MAIN to clear pending interrupts and writes enable values to 10 interrupt mask registers: INT_MSK_PHY_FATAL=0x0000000f, INT_MSK_PKT_FATAL=0x00000003, INT_MSK_PHY=0x000f000f, INT_MSK_LINE=0x000f000f, INT_MSK_BNDRY_FRAME_FATAL=0x0000ffff, INT_MSK_SEQ_FRAME_FATAL=0x0000ffff, INT_MSK_CRC_FRAME_FATAL=0x0000ffff, INT_MSK_PLD_CRC_FATAL=0x0000ffff, INT_MSK_DATA_ID=0x0000ffff, INT_MSK_ECC_CORRECTED=0x0000ffff.",
        "Test Description": "Validate the MIPI CSI-2 internal test pattern generator by configuring the virtual channel, initializing the D-PHY, programming DMA for data reception, enabling the pattern generator with specific vertical resolution, horizontal resolution, and configuration parameters, then disabling the pattern generator and polling for DMA transfer completion to confirm successful data reception.",
        "Meta Test Steps / Procedure": "1. Enter test_case() function.\n2. Declare local variables: rx_desc, tx_desc (long long int), csi2_data_trnsfr_size, single_axi_trnsfr_size (long long int), vcid_unselected_path, vcid (int), num_beats (int), dma_ch0_pc, dma_ch0_instn_preload_addr (unsigned long int), dmago_instn_0, dmago_instn_1, instn (unsigned int), total_loop_cnt, lc1_iter, lc0_iter (int), dma_dest_addr_incr_flag (int), valid_bits_per_pixel (int), rd_data (unsigned int), vres, hres (int).\n3. int_pend = 1.\n4. printf(\"start line\\n\").\n5. vcid = 3.\n6. vcid_unselected_path = ((vcid + 1) & 0xf) \u2014 evaluates to 4.\n7. Conditional compilation for GDMA path selection. Default GDMA0_PATH: vcid_csi2_wrap_reg = ((vcid << 12) + (vcid_unselected_path << 8) + (vcid_unselected_path << 4) + (vcid_unselected_path)) \u2014 evaluates to ((3 << 12) + (4 << 8) + (4 << 4) + 4) = 0x3444.\n8. gdma_path = 0.\n9. gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000) \u2014 evaluates to 0xE6A00000.\n10. printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg).\n11. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) \u2014 write virtual channel register with 0x3444.\n12. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0) \u2014 disable control data transfer.\n13. Call csi2_subsys_enable_interrupt() \u2014 external function, enables CSI-2 and DMA interrupts at subsystem level.\n14. Call snps_phy_init() \u2014 external function, D-PHY initialization sequence.\n15. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) \u2014 first read of PHY_STOPSTATE.\n16. while(!(rd_data == 0x1000f)): poll loop \u2014 rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) \u2014 repeated read until rd_data equals 0x1000f.\n17. hres = 320.\n18. vres = 16.\n19. valid_bits_per_pixel = 24.\n20. csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres) / 8) % 8) ? (((valid_bits_per_pixel * hres) / 8) + 8 - (((valid_bits_per_pixel * hres) / 8) % 8)) : ((valid_bits_per_pixel * hres) / 8)) * vres) \u2014 computes to (((24 * 320 / 8) aligned to 8-byte boundary) * 16) = (960 * 16) = 15360 bytes.\n21. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100) \u2014 program DMA higher-order AXI read address for channel 0 data.\n22. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0) \u2014 program DMA higher-order AXI read address for channel 0 instruction.\n23. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0) \u2014 program DMA higher-order AXI write address for channel 0 data.\n24. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0) \u2014 program DMA higher-order AXI write address for channel 0 instruction.\n25. write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1) \u2014 enable sending fracdiv output to CSI-2 subsystem.\n26. dma_ch0_pc = 0xE6000000.\n27. dma_ch0_instn_preload_addr = dma_ch0_pc.\n28. Conditional compilation for FPS60. Default (non-FPS60): dma_dest_addr_incr_flag = 1.\n29. Call dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base, 0x00, 0xE6001000, csi2_data_trnsfr_size, 0, dma_dest_addr_incr_flag, 0) \u2014 program DMA channel 0 transfer: src_addr=0x00, dest_addr=0xE6001000, trnsfr_size=15360, src_incr_addr_flag=0, dest_incr_addr_flag=1, irq_num=0.\n30. Call DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0) \u2014 start DMA channel 0.\n31. Call csi2_ctrlr_pg_enable().\n32. [Inside csi2_ctrlr_pg_enable()] write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10) \u2014 set pattern generator vertical resolution to 16.\n33. [Inside csi2_ctrlr_pg_enable()] write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140) \u2014 set pattern generator horizontal resolution: {16'd7, 16'd320}.\n34. [Inside csi2_ctrlr_pg_enable()] write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401) \u2014 set pattern generator configuration: {14'h0, 2'h0, 2'h3, 6'h24, 7'h0, 1'h1} = {16'h0, 8'hE4, 8'h01}.\n35. [Inside csi2_ctrlr_pg_enable()] write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1) \u2014 enable pattern generator.\n36. [Return from csi2_ctrlr_pg_enable()].\n37. Call wait_on(100) \u2014 wait 100 units after pattern generator enable.\n38. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0) \u2014 disable pattern generator.\n39. rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) \u2014 first read of DMA interrupt masked status.\n40. while((rd_data & 0x1) == 0): poll loop \u2014 rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) \u2014 poll DMA interrupt status for channel 0 completion (bit 0).\n41. Call wait_on(10000) \u2014 wait 10000 units after DMA completion.\n42. Call finish(0) \u2014 testcase completion with pass status.\n43. [csi2_enable_interrupt() function defined in source but called externally via csi2_subsys_enable_interrupt()]:\n44. [Inside csi2_enable_interrupt()] Declare local int rd_data.\n45. [Inside csi2_enable_interrupt()] rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) \u2014 read INT_ST_MAIN to clear pending interrupts.\n46. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f) \u2014 enable phy_fatal interrupts.\n47. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003) \u2014 enable pkt_fatal interrupts.\n48. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f) \u2014 enable phy interrupts.\n49. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f) \u2014 enable line interrupts.\n50. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff) \u2014 enable boundary frame fatal interrupts.\n51. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff) \u2014 enable seq frame fatal interrupts.\n52. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff) \u2014 enable crc frame fatal interrupts.\n53. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff) \u2014 enable pld crc fatal interrupts.\n54. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff) \u2014 enable data_id interrupts.\n55. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff) \u2014 enable ecc corrected interrupts.",
        "Test Steps / Procedure": "1. Configure the virtual channel register with the appropriate VC_ID mapping for the selected GDMA path.\n2. Disable control data transfer by writing 0 to the control_data register.\n3. Enable CSI-2 and DMA interrupts at the subsystem level.\n4. Perform D-PHY initialization sequence.\n5. Poll the PHY_STOPSTATE register until the D-PHY enters stop state (expected value 0x1000f).\n6. Compute the total data transfer size based on horizontal resolution (320), vertical resolution (16), and bits per pixel (24), yielding 15360 bytes.\n7. Program DMA higher-order address registers for channel 0 read and write paths.\n8. Enable fracdiv output to the CSI-2 subsystem.\n9. Program DMA channel 0 for data transfer with source address, destination address, computed transfer size, and address increment configuration.\n10. Start DMA channel 0.\n11. Enable the internal test pattern generator by configuring vertical resolution (16), horizontal resolution (320 with attribute 7), configuration (RGB888, VC3), and setting the enable bit.\n12. Wait for pattern generation to complete.\n13. Disable the pattern generator by clearing the enable bit.\n14. Poll DMA interrupt status for channel 0 completion (bit 0).\n15. Wait for post-completion settling.\n16. Complete the test with pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE poll: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) must eventually return 0x1000f indicating D-PHY stop state is reached on all data lanes and clock lane.\n2. DMA channel 0 completion poll: (rd_data & 0x1) != 0 \u2014 bit 0 of gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET must be set, indicating DMA channel 0 transfer of csi2_data_trnsfr_size (15360) bytes is complete.\n3. Pattern generator configuration validation: PPI_PG_PATTERN_VRES written with 0x10 (vres=16), PPI_PG_PATTERN_HRES written with 0x70140 ({16'd7, 16'd320}), PPI_PG_CONFIG written with 0xe401 ({14'h0, 2'h0, 2'h3, 6'h24, 7'h0, 1'h1} representing VC3, RGB888 data type, pattern enabled), PPI_PG_ENABLE written with 1 to enable then 0 to disable.\n4. DMA address register programming: DMA_M0_ADDR_AR_CH0_DATA=0x100, DMA_M0_ADDR_AR_CH0_INSTRUCTION=0x0, DMA_M0_ADDR_AW_CH0_DATA=0x0, DMA_M0_ADDR_AW_CH0_INSTRUCTION=0x0.\n5. DMA transfer parameters: src_addr=0x00, dest_addr=0xE6001000, trnsfr_size=15360 (csi2_data_trnsfr_size), src_incr_addr_flag=0, dest_incr_addr_flag=1, irq_num=0.\n6. Fracdiv enable: MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 written with 0x1.\n7. Virtual channel register: MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL written with vcid_csi2_wrap_reg (0x3444 for default GDMA0_PATH with vcid=3).\n8. Control data disabled: MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA written with 0.\n9. Test completion: finish(0) is called indicating test pass after DMA completion and post-completion wait.",
        "Validation / Acceptance Criteria": "1. D-PHY enters stop state successfully as indicated by PHY_STOPSTATE register returning the expected value.\n2. DMA channel 0 completes the full data transfer of pattern generator output (15360 bytes), confirmed by DMA interrupt status bit 0 assertion.\n3. Pattern generator is successfully enabled and then disabled without errors.\n4. Test completes with pass status via finish(0).",
        "Remarks": "External functions snps_phy_init(), csi2_subsys_enable_interrupt(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), and finish() are not defined within the testcase folder and their implementations are not inspectable. The function csi2_enable_interrupt() is defined in this source file but is not directly called from test_case(); test_case() calls csi2_subsys_enable_interrupt() which is external and may internally call csi2_enable_interrupt(). The GDMA path is selected via conditional compilation; default path assumed is GDMA0_PATH. The FPS60 define controls dma_dest_addr_incr_flag; default (non-FPS60) sets it to 1. The macro SOFT_RST_REG_ADDRESS was ignored per instruction. The register at MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 (fracdiv enable) is accessed via a base+offset expression and could not be mapped to a named register via Agent 4. PPI_PG_CONFIG value 0xe401 encodes: VC=3 (bits[17:16]=2'h3), data_type=0x24/RGB888 (bits[13:8]=6'h24), pattern_type enabled (bit[0]=1'h1)."
    }
]

# ============================================================
# TestPlan Sheet Columns
# ============================================================
testplan_columns = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation"
]

# ============================================================
# MetaData Sheet Columns
# ============================================================
metadata_columns = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays"
]

# ============================================================
# Create Workbook
# ============================================================
wb = Workbook()

# Rename default sheet to TestPlan
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Create MetaData sheet
ws_md = wb.create_sheet("MetaData")

# ============================================================
# Formatting
# ============================================================
header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

def write_header(ws, columns):
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

def auto_size_columns(ws, columns, max_width=60):
    for col_idx, col_name in enumerate(columns, 1):
        max_len = len(str(col_name))
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_len:
                            max_len = len(line)
        adjusted_width = min(max_len + 4, max_width)
        ws.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

# ============================================================
# Write TestPlan Sheet
# ============================================================
write_header(ws_tp, testplan_columns)

for row_data in json_data:
    row_values = [
        row_data.get("Index", ""),
        row_data.get("SS / Module", ""),
        row_data.get("Feature", ""),
        row_data.get("Test Case Name", ""),
        row_data.get("Test Description", ""),
        row_data.get("Speed", ""),
        row_data.get("Mode", ""),
        row_data.get("Memory Start Offset", ""),
        row_data.get("Memory End Offset", ""),
        row_data.get("Remarks", ""),
        row_data.get("Test Steps / Procedure", ""),
        row_data.get("Impacted Registers", ""),
        row_data.get("Validation / Acceptance Criteria", ""),
        ""  # Code Generation - empty
    ]
    ws_tp.append(row_values)

# Apply wrap text to all data cells in TestPlan
for row in ws_tp.iter_rows(min_row=2, max_row=ws_tp.max_row, max_col=len(testplan_columns)):
    for cell in row:
        cell.alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns
auto_size_columns(ws_tp, testplan_columns)

# ============================================================
# Write MetaData Sheet
# ============================================================
write_header(ws_md, metadata_columns)

for row_data in json_data:
    row_values = [
        row_data.get("Index", ""),
        row_data.get("Test Case Name", ""),
        row_data.get("Meta Test Description", ""),
        row_data.get("Meta Test Steps / Procedure", ""),
        row_data.get("Meta Impacted Registers", ""),
        row_data.get("Meta Validation / Acceptance Criteria", ""),
        row_data.get("Meta Headers", ""),
        row_data.get("Meta Macros", ""),
        row_data.get("Meta Arrays", "")
    ]
    ws_md.append(row_values)

# Apply wrap text to all data cells in MetaData
for row in ws_md.iter_rows(min_row=2, max_row=ws_md.max_row, max_col=len(metadata_columns)):
    for cell in row:
        cell.alignment = wrap_alignment

# Freeze first row
ws_md.freeze_panes = "A2"

# Auto-size columns
auto_size_columns(ws_md, metadata_columns)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# ============================================================
# Save Workbook
# ============================================================
wb.save(output_path)
print(f"Workbook saved: {output_path}")

# ============================================================
# Post-Save Validation
# ============================================================
if not os.path.exists(output_path):
    print("VALIDATION FAILED: File does not exist")
    sys.exit(1)

file_size = os.path.getsize(output_path)
if file_size == 0:
    print("VALIDATION FAILED: File size is 0")
    sys.exit(1)

# Verify workbook can be reopened
try:
    wb_check = load_workbook(output_path)
    sheet_names = wb_check.sheetnames
    if "TestPlan" not in sheet_names:
        print("VALIDATION FAILED: TestPlan sheet missing")
        sys.exit(1)
    if "MetaData" not in sheet_names:
        print("VALIDATION FAILED: MetaData sheet missing")
        sys.exit(1)
    tp_rows = wb_check["TestPlan"].max_row - 1  # exclude header
    md_rows = wb_check["MetaData"].max_row - 1  # exclude header
    wb_check.close()
except Exception as e:
    print(f"VALIDATION FAILED: Cannot reopen workbook: {e}")
    sys.exit(1)

print(f"VALIDATION PASSED")
print(f"  Filename: {filename}")
print(f"  File size: {file_size} bytes")
print(f"  TestPlan rows: {tp_rows}")
print(f"  MetaData rows: {md_rows}")
print(f"  Sheets: {sheet_names}")
print(f"OUTPUT_FILE={output_path}")
print(f"OUTPUT_FILENAME={filename}")
