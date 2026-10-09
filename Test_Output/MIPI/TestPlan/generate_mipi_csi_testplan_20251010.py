#!/usr/bin/env python3
"""Agent 7 - MIPI_CSI TestPlan XLSX Generator.
Run: python3 generate_mipi_csi_testplan_20251010.py
Requires: pip install openpyxl
Generates MIPI_CSI_TestPlan_<IST_TIMESTAMP>.xlsx and outputs base64 for GitHub push.
Trigger: workflow_dispatch or push event
"""
import json, os, sys, base64
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"

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
    "Meta Test Steps / Procedure": "1. Set global variable int_pend = 1.\n2. printf(\"start line\\n\").\n3. Set local variable vcid = 3.\n4. Compute vcid_unselected_path = ((vcid + 1) & 0xf), which equals 4.\n5. Conditional compilation for GDMA path selection:\n - If GDMA3_PATH defined: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+vcid); gdma_path = 3.\n - Elif GDMA2_PATH defined: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid<<4)+(vcid_unselected_path)); gdma_path = 2.\n - Elif GDMA1_PATH defined: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path)); gdma_path = 1.\n - Else (GDMA0_PATH default): vcid_csi2_wrap_reg = ((vcid<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path)); gdma_path = 0.\n6. Compute gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000).\n7. printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg).\n8. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) \u2014 Write CSI-2 virtual channel register.\n9. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0) \u2014 Disable control data transfer.\n10. Call csi2_subsys_enable_interrupt() \u2014 Enable CSI-2 and DMA interrupts (external function, implementation not locally available).\n11. Call snps_phy_init() \u2014 D-PHY initialization sequence (external function, implementation not locally available).\n12. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) \u2014 Read PHY stop state register.\n13. Enter while loop: while(!(rd_data == 0x1000f)) { rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE); } \u2014 Poll PHY_STOPSTATE until value equals 0x1000f.\n14. Set hres = 320.\n15. Set vres = 16.\n16. Set valid_bits_per_pixel = 24.\n17. Compute csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres)/8) % 8) ? (((valid_bits_per_pixel * hres)/8) + 8 - (((valid_bits_per_pixel * hres)/8) % 8)) : ((valid_bits_per_pixel * hres)/8)) * vres) \u2014 Number of bytes of transfer. With valid_bits_per_pixel=24, hres=320, vres=16: (24*320)/8 = 960, 960%8=0, so csi2_data_trnsfr_size = 960 * 16 = 15360 bytes.\n18. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100) \u2014 Program higher order ARM DMA AR channel 0 data address bits.\n19. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0) \u2014 Program higher order ARM DMA AR channel 0 instruction address bits.\n20. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0) \u2014 Program higher order ARM DMA AW channel 0 data address bits.\n21. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0) \u2014 Program higher order ARM DMA AW channel 0 instruction address bits.\n22. write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1) \u2014 Enable sending fracdiv output to CSI2 subsystem.\n23. Set dma_ch0_pc = 0xE6000000.\n24. Set dma_ch0_instn_preload_addr = dma_ch0_pc (= 0xE6000000).\n25. Conditional compilation for FPS60:\n - If FPS60 defined: dma_dest_addr_incr_flag = 0.\n - Else: dma_dest_addr_incr_flag = 1.\n26. Call dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base, 0x00 /src_addr/, 0xE6001000 /dest_addr/, csi2_data_trnsfr_size /trnsfr_size/, 0 /src_incr_addr_flag/, dma_dest_addr_incr_flag /dest_incr_addr_flag/, 0 /irq_num/) \u2014 Preload DMA transfer instructions with incremental address support (external function, implementation not locally available).\n27. Call DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0 /ch_num/) \u2014 Issue DMAGO command for channel 0 (external function, implementation not locally available).\n28. Call csi2_ctrlr_pg_enable() \u2014 Enable pattern generator. Internal implementation:\n 28a. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10) \u2014 Set PG vertical resolution to 0x10 (16).\n 28b. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140) \u2014 Set PG horizontal resolution to 0x70140 ({16'd7, 16'd320}).\n 28c. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401) \u2014 Set PG configuration to 0xe401 ({14'h0, 2'h0, 2'h3, 6'h24, 7'h0, 1'h1}, i.e., 16'h0, 8'hE4, 8'h01).\n 28d. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1) \u2014 Enable pattern generator.\n29. Call wait_on(100) \u2014 Wait 100 cycles.\n30. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0) \u2014 Disable pattern generator.\n31. rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) \u2014 Read DMA interrupt masked status register.\n32. Enter while loop: while((rd_data & 0x1) == 0) { rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET); } \u2014 Poll DMA interrupt masked status until bit 0 is set (DMA transfer complete). Conditional debug printf inside loop: printf(\"Polling dma_irq[0]=%0d\\n\", rd_data).\n33. Call wait_on(10000) \u2014 Wait 10000 cycles.\n34. Call finish(0) \u2014 End test with pass status.\n\nLocally defined function csi2_enable_interrupt() (defined in program.c but NOT called from test_case(); test_case() calls csi2_subsys_enable_interrupt() instead):\n L1. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) \u2014 Read INT_ST_MAIN register to clear interrupts.\n L2. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f) \u2014 Enable phy_fatal interrupts.\n L3. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003) \u2014 Enable pkt_fatal interrupts.\n L4. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f) \u2014 Enable phy interrupts.\n L5. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f) \u2014 Enable line interrupts.\n L6. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff) \u2014 Enable boundary frame fatal interrupts.\n L7. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff) \u2014 Enable seq frame fatal interrupts.\n L8. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff) \u2014 Enable CRC frame fatal interrupts.\n L9. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff) \u2014 Enable payload CRC fatal interrupts.\n L10. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff) \u2014 Enable data ID interrupts.\n L11. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff) \u2014 Enable ECC corrected interrupts.\n\nGlobal variable declarations:\n - int data_rd, data_wr;\n - int def_fail_cnt = 0, wr_fail_cnt = 0;\n - int vcid_csi2_wrap_reg;\n - int gdma_path;\n - long int csi_ctrl_data;\n - int gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;\n - extern int_pend;\n - unsigned int tx_trnsfr_size;",
    "Test Steps / Procedure": "1. Initialize interrupt pending flag and set virtual channel ID to 3.\n2. Compute virtual channel routing register value based on the selected GDMA path (GDMA0/1/2/3) and write the virtual channel register.\n3. Disable control data transfer by writing 0 to the control data register.\n4. Enable CSI-2 subsystem and DMA interrupts.\n5. Initialize the SNPS D-PHY.\n6. Poll the PHY stop state register until all lanes enter stop state (expected value 0x1000f).\n7. Set image parameters: horizontal resolution = 320, vertical resolution = 16, bits per pixel = 24.\n8. Compute DMA transfer size based on image parameters (byte-aligned to 8-byte boundary).\n9. Program higher-order ARM DMA address bits for channel 0 read and write paths.\n10. Enable fracdiv output to CSI2 subsystem.\n11. Preload DMA transfer instructions with source address 0x00, destination address 0xE6001000, and computed transfer size.\n12. Issue DMAGO command for DMA channel 0.\n13. Enable the pattern generator by configuring vertical resolution, horizontal resolution, pattern configuration, and PG enable registers.\n14. Wait 100 cycles.\n15. Disable the pattern generator.\n16. Poll the DMA interrupt masked status register until bit 0 is set, indicating DMA transfer completion.\n17. Wait 10000 cycles.\n18. Finish the test with pass status.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE polling: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) must return 0x1000f to exit the while loop. The test polls continuously until rd_data == 0x1000f, indicating all D-PHY lanes have entered stop state.\n2. DMA interrupt polling: read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled in a while loop until (rd_data & 0x1) != 0, i.e., bit 0 of the DMA interrupt masked status register must be set to 1, indicating DMA channel 0 transfer completion.\n3. Test completion: finish(0) is called with argument 0, indicating test pass. No explicit fail path is present in test_case(); the test will hang in the polling loops if the expected conditions are never met.\n4. The locally defined csi2_enable_interrupt() function (not called from test_case()) validates interrupt configuration by reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear pending interrupts before writing mask registers.",
    "Validation / Acceptance Criteria": "1. The D-PHY must enter stop state on all lanes, confirmed by the PHY stop state register reading the expected value.\n2. DMA channel 0 transfer must complete successfully, confirmed by the DMA interrupt masked status register bit 0 being set.\n3. The test must complete with a pass status via the finish call.",
    "Remarks": "The test_case() function calls csi2_subsys_enable_interrupt() which is an external function (not the locally defined csi2_enable_interrupt()). The locally defined csi2_enable_interrupt() is present in program.c but is not invoked from the test entry point. External functions snps_phy_init(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), and finish() are called but their implementations are not available in the testcase folder. The GDMA path is selected via conditional compilation (GDMA0_PATH/GDMA1_PATH/GDMA2_PATH/GDMA3_PATH). The FPS60 macro controls whether destination address increment is enabled. The DMA interrupt status register is accessed via gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET, where gdma_reg_base is computed at runtime and MIPI_CSI2_DMA_INTMIS_OFFSET is defined externally. MIZAR_MIPI_CSI2_RB_REG_BASE is used with offset 0xf4 for fracdiv enable but its Agent 4 mapping is unresolved."
  }
]

# TestPlan columns
tp_cols = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
           "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
           "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
           "Code Generation"]

# MetaData columns
md_cols = ["Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
           "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
           "Meta Headers", "Meta Macros", "Meta Arrays"]

wb = Workbook()
ws_tp = wb.active
ws_tp.title = "TestPlan"
ws_md = wb.create_sheet("MetaData")

header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_align = Alignment(wrap_text=True, vertical="top")

def write_sheet(ws, columns, data):
    for ci, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align
    for ri, row_data in enumerate(data, 2):
        for ci, col_name in enumerate(columns, 1):
            val = row_data.get(col_name, "")
            cell = ws.cell(row=ri, column=ci, value=val)
            cell.alignment = wrap_align
    ws.freeze_panes = "A2"
    for ci, col_name in enumerate(columns, 1):
        max_len = len(col_name)
        for ri in range(2, len(data) + 2):
            val = str(ws.cell(row=ri, column=ci).value or "")
            lines = val.split("\n")
            for line in lines:
                max_len = max(max_len, len(line))
        width = min(max_len + 2, 60)
        ws.column_dimensions[get_column_letter(ci)].width = width

write_sheet(ws_tp, tp_cols, json_data)
write_sheet(ws_md, md_cols, json_data)
ws_md.sheet_state = "veryHidden"

script_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(script_dir, filename)
wb.save(output_path)

# Validate
assert os.path.exists(output_path), "File not created"
assert os.path.getsize(output_path) > 0, "File is empty"
wb2 = load_workbook(output_path)
assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing"
assert "MetaData" in wb2.sheetnames, "MetaData sheet missing"
print(f"SUCCESS: Generated {filename} ({os.path.getsize(output_path)} bytes)")
print(f"Path: {output_path}")
print(f"FILENAME:{filename}")
