#!/usr/bin/env python3
"""Agent 7 - MIPI_CSI TestPlan XLSX Generator
Generates MIPI_CSI_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx with TestPlan and MetaData sheets.
"""
import os, sys
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
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_dphy_lanes_test",
    "Feature": "D-PHY Lane Configuration",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "0xE6000000; 0xE6000500; 0xE6001000; 0xE6002000",
    "Memory End Offset": "NA",
    "Test Description": "Verify MIPI CSI-2 D-PHY lane configuration by iterating through 4-lane, 3-lane, 2-lane, and 1-lane modes. For each lane configuration, enable CSI-2 host interrupts, configure virtual channel routing, initialize the D-PHY, poll for PHY stop state, configure the lane count, trigger the CSI-2 sequence, and perform DMA-based control and data packet transfers with interrupt-driven completion for each received packet. Validate that DMA transfers complete successfully for all lane configurations.",
    "Test Steps / Procedure": "1. Enable all CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing enable masks to PHY Fatal, Packet Fatal, PHY, Line, Boundary Frame Fatal, Sequence Frame Fatal, CRC Frame Fatal, Payload CRC Fatal, Data ID, and ECC Corrected interrupt mask registers.\n2. Configure the virtual channel routing register based on the selected GDMA path and set the virtual channel ID.\n3. Enable control data transfer by writing to the control data register.\n4. Write the virtual channel and control data registers a second time to confirm configuration.\n5. Initialize the D-PHY by calling the PHY initialization sequence.\n6. Poll the PHY Stop State register until all lanes report stop state (expected value 0x1000f).\n7. Set DMA channel 0 program counter to 0xE6000000 and channel 1 program counter to 0xE6000500.\n8. Begin outer loop iterating lane count from 4 lanes down to 1 lane.\n9. Write the N_LANES register with the current lane configuration value.\n10. Calculate the control packet count as (VRES * 3) + 2.\n11. Write the trigger register to start the CSI-2 sequence with the current lane count.\n12. Begin inner loop iterating through each expected control packet.\n13. Enable DMA interrupts for both channel 0 and channel 1.\n14. Program a control data DMA transfer on channel 0 (source: CSI-2 control data FIFO, destination: control data buffer, transfer size: 8 bytes).\n15. Start DMA channel 0 and poll the DMA interrupt status register until channel 0 transfer completes.\n16. Clear the DMA interrupt for channel 0.\n17. Read the received control data from the destination buffer.\n18. If the control data indicates a long packet (data type > 0xf), extract the word count, calculate the 8-byte aligned transfer size, program a data DMA transfer on channel 1, start DMA channel 1, poll the DMA interrupt status register until channel 1 transfer completes, and clear the DMA interrupt for channel 1.\n19. Repeat for all packets in the inner loop.\n20. Repeat for all lane configurations in the outer loop.\n21. End test with pass status by calling finish(0).",
    "Impacted Registers": "INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED; virtual_channel; control_data; PHY_STOPSTATE; N_LANES",
    "Validation / Acceptance Criteria": "1. The PHY Stop State register must report all data lanes and clock lane in stop state (value 0x1000f) before lane configuration begins.\n2. DMA channel 0 transfer must complete for each control packet, confirmed by the DMA interrupt status register bit 0 being set.\n3. For long packets (data type > 0xf), DMA channel 1 transfer must complete, confirmed by the DMA interrupt status register bit 1 being set.\n4. The test must successfully iterate through all four lane configurations (4-lane, 3-lane, 2-lane, 1-lane) and process all expected packets for each configuration.\n5. The test completes with pass status via finish(0) after all lane iterations.",
    "Remarks": "The testcase uses conditional compilation (#ifdef GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) to select the GDMA path and virtual channel shift. Default VRES=3 and HRES=64 are used unless GDMA0_FULL_MEM is defined (which would set VRES=1080, HRES=1920). The snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() functions are external and not defined within the testcase folder. The MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL and MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA registers are written twice in sequence. Two hex addresses (0xa0243ffc and 0xE6001000) could not be mapped to named registers in the specification documents.",
    "Meta Headers": "#include <stdio.h>; #include <stdlib.h>; #include \"test_common.h\"; #include \"mipi_csi2.h\"",
    "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 3\n#define HRES 64\n#define DATA_TYPE CSI2_RGB888",
    "Meta Arrays": "NA",
    "Meta Test Description": "This testcase verifies MIPI CSI-2 D-PHY lane configuration by iterating through lane counts from 4 lanes (lane_num=3) down to 1 lane (lane_num=0). For each lane configuration, the test performs the following sequence: (1) enables all CSI-2 host interrupt masks by calling csi2_enable_interrupt(), which reads INT_ST_MAIN to clear pending interrupts and then writes enable masks to 10 interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED); (2) configures the virtual channel register with VC_ID=3 shifted based on the GDMA path; (3) enables control data transfer; (4) calls snps_phy_init() for D-PHY initialization; (5) polls PHY_STOPSTATE until 0x1000f; (6) iterates lane configurations with DMA transfers.",
    "Meta Test Steps / Procedure": "1. Enter test_case() function.\n2. Declare local variables: rx_desc, tx_desc, gdma_tx_trnsfr_size, gdma_trnsfr_size, cntrl_pkt_cnt.\n3. Call csi2_enable_interrupt().\n4. Read INT_ST_MAIN to clear pending interrupts.\n5. Write enable masks to 10 interrupt mask registers.\n6. Configure virtual channel register based on GDMA path.\n7. Write control data register.\n8. Call snps_phy_init().\n9. Poll PHY_STOPSTATE until 0x1000f.\n10. Iterate lane_num from 3 to 0.\n11. Write N_LANES register.\n12. Calculate cntrl_pkt_cnt = (VRES*3)+2.\n13. Write trigger register 0xa0243ffc.\n14. For each packet: enable DMA interrupts, program DMA transfer, start DMA, poll completion, clear interrupt.\n15. For long packets: program data DMA on channel 1, poll completion.\n16. Call finish(0).",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000",
    "Meta Validation / Acceptance Criteria": "1. PHY Stop State polling: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) polled until rd_data == 0x1000f.\n2. DMA Channel 0 completion: INTMIS bit 0 set.\n3. Control data type check: (csi_ctrl_data & 0x3f) > 0xf for long packets.\n4. DMA Channel 1 completion: INTMIS bit 1 set.\n5. Word count extraction and 8-byte alignment.\n6. finish(0) called after all iterations."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_test_pattern_generator",
    "Feature": "Test Pattern Generator",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "0xE6000000; 0xE6001000",
    "Memory End Offset": "NA",
    "Test Description": "Verify the MIPI CSI-2 internal test pattern generator by configuring the PG block with RGB888 data type, VC=3, HRES=320, VRES=16, enabling CSI-2 host interrupts, initializing the D-PHY, polling for PHY stop state, configuring DMA AXI address registers, enabling the fracdiv clock, programming a DMA data transfer, enabling the pattern generator, waiting briefly, disabling the pattern generator, and polling for DMA transfer completion. Validate that the DMA transfer completes successfully.",
    "Test Steps / Procedure": "1. Set interrupt pending flag and initialize virtual channel ID to 3.\n2. Configure the virtual channel routing register based on the selected GDMA path to route VC=3 to the appropriate DMA channel.\n3. Disable control data transfer by writing 0 to the control data register.\n4. Enable all CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing enable masks to PHY Fatal, Packet Fatal, PHY, Line, Boundary Frame Fatal, Sequence Frame Fatal, CRC Frame Fatal, Payload CRC Fatal, Data ID, and ECC Corrected interrupt mask registers.\n5. Initialize the D-PHY by calling the PHY initialization sequence.\n6. Poll the PHY Stop State register until all lanes report stop state (expected value 0x1000f).\n7. Set horizontal resolution to 320, vertical resolution to 16, and bits per pixel to 24.\n8. Calculate the total DMA data transfer size with 8-byte alignment (960 bytes per line x 16 lines = 15360 bytes).\n9. Configure DMA AXI higher-order address registers for channel 0: AR data = 0x100, AR instruction = 0x0, AW data = 0x0, AW instruction = 0x0.\n10. Enable the fracdiv clock output to the CSI-2 subsystem by writing to the clock gating register.\n11. Program the DMA transfer on channel 0 with source address 0x00, destination address 0xE6001000, and the calculated transfer size.\n12. Start DMA channel 0.\n13. Enable the internal test pattern generator by configuring PG vertical resolution (16), horizontal resolution (320 with 7 beats), PG configuration (RGB888 data type, VC=3), and PG enable.\n14. Wait 100 time units for pattern generation.\n15. Disable the pattern generator.\n16. Poll the DMA interrupt status register until channel 0 transfer completes (bit 0 set).\n17. Wait 10000 time units for completion.\n18. End test with pass status by calling finish(0).",
    "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Validation / Acceptance Criteria": "1. The PHY Stop State register must report all data lanes and clock lane in stop state (value 0x1000f) before DMA and pattern generator configuration begins.\n2. DMA channel 0 transfer must complete, confirmed by the DMA interrupt status register bit 0 being set.\n3. The pattern generator must be configured with RGB888 data type, VC=3, HRES=320, VRES=16 and must be enabled then disabled in sequence.\n4. The total DMA data transfer size must be 15360 bytes (960 bytes per line x 16 lines, 8-byte aligned).\n5. The test completes with pass status via finish(0) after DMA transfer completion and a final wait period.",
    "Remarks": "The testcase uses conditional compilation (#ifdef GDMA3_PATH, GDMA2_PATH, GDMA1_PATH, default GDMA0_PATH) to select the GDMA path and virtual channel routing. The default path is GDMA0_PATH (gdma_path=0, gdma_reg_base=0xE6A00000). An additional conditional compilation (#ifdef FPS60) controls whether the DMA destination address increments. The PPI_PG_CONFIG value 0xe401 encodes VC=3, data_type=0x24 (RGB888), and mode=1. The MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 write targets the enableclkgating_csiphy register in the wrapper specification.",
    "Meta Headers": "#include <stdio.h>; #include <stdlib.h>; #include \"test_common.h\"; #include \"mipi_csi2.h\"",
    "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
    "Meta Arrays": "NA",
    "Meta Test Description": "This testcase verifies the MIPI CSI-2 internal test pattern generator (PG) functionality. The test configures the CSI-2 subsystem to receive internally generated test pattern data using the PPI PG block with RGB888 data type (0x24), virtual channel VC=3, horizontal resolution HRES=320, vertical resolution VRES=16, and valid_bits_per_pixel=24.",
    "Meta Test Steps / Procedure": "1. Enter test_case() function.\n2. Initialize vcid=3, vcid_unselected_path=4.\n3. Configure virtual channel register based on GDMA path.\n4. Disable control data transfer.\n5. Call csi2_subsys_enable_interrupt().\n6. Call snps_phy_init().\n7. Poll PHY_STOPSTATE until 0x1000f.\n8. Calculate csi2_data_trnsfr_size = 15360 bytes.\n9. Configure DMA AXI address registers.\n10. Enable fracdiv clock (write 0x1 to RB_REG_BASE+0xf4).\n11. Program DMA transfer.\n12. Start DMA channel 0.\n13. Enable pattern generator (VRES=0x10, HRES=0x70140, CONFIG=0xe401, ENABLE=1).\n14. Wait 100 time units.\n15. Disable pattern generator.\n16. Poll DMA INTMIS until bit 0 set.\n17. Wait 10000 time units.\n18. Call finish(0).",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "1. PHY Stop State polling until rd_data == 0x1000f.\n2. DMA Channel 0 completion: INTMIS bit 0 set.\n3. PG configuration: VRES=0x10, HRES=0x70140, CONFIG=0xe401, ENABLE toggled.\n4. Transfer size = 15360 bytes.\n5. finish(0) called after completion."
  }
]

# TestPlan columns
tp_cols = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
           "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
           "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
           "Code Generation"]

# Metadata columns
md_cols = ["Index", "SS / Module", "Test Case Name", "Feature", "Meta Headers", "Meta Macros",
           "Meta Arrays", "Meta Test Description", "Meta Test Steps / Procedure",
           "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"]

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
