#!/usr/bin/env python3
"""
Agent 7 - Excel Generator Script
Generates MIPI_CSI_TestPlan_20261009_184530.xlsx
Run: python3 generate_mipi_csi_testplan_20261009_runner.py
"""
import os, sys, base64, json
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# IST timestamp
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"

# JSON data
json_data = [
    {"Index":"1","SS / Module":"MIPI_CSI","Test Case Name":"mipi_csi2_dphy_lanes_test","Feature":"D-PHY Lane Configuration and CSI-2 Data Reception","Test Description":"Verify MIPI CSI-2 D-PHY lane configuration by iterating through all lane counts (4 to 1 lane), enabling CSI-2 interrupts, initializing the D-PHY, waiting for PHY stop state, and performing DMA-based control and data packet transfers for each lane configuration. Validate DMA completion via interrupt polling and verify control packet reception for each lane count.","Speed":"NA","Mode":"NA","Memory Start Offset":"0xE6000000","Memory End Offset":"0xE6040080","Remarks":"The testcase uses external functions snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() whose implementations are not available in the testcase folder. The VRES and HRES macros have conditional definitions depending on GDMA0_FULL_MEM. The GDMA path selection (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) is compile-time conditional. Two hex register tokens (0xa0243ffc and 0xE6001000) could not be mapped to named registers in the specification documents.","Test Steps / Procedure":"1. Call the CSI-2 interrupt enable routine to clear pending interrupts by reading the main interrupt status register, then enable all CSI-2 interrupt masks including PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupts.\n2. Configure the CSI-2 virtual channel register with the selected virtual channel ID based on the active GDMA path.\n3. Enable control data transfer by writing to the control data register.\n4. Write the virtual channel register and control data register a second time to confirm configuration.\n5. Initialize the D-PHY by calling the SNPS PHY initialization routine.\n6. Poll the PHY stop state register until the PHY enters stop state (expected value indicating all lanes and clock in stop state).\n7. Set DMA channel 0 program counter to the designated memory location and DMA channel 1 program counter to a separate designated memory location.\n8. For each lane count from 4 lanes down to 1 lane, configure the number of active lanes in the CSI-2 host controller.\n9. Trigger the CSI-2 sequence by writing the lane count to the sequence trigger register.\n10. For each control packet in the frame (total count based on vertical resolution), enable DMA interrupts for both channels.\n11. Program and start a DMA transfer on channel 0 to receive the 8-byte control packet.\n12. Poll the DMA interrupt status register until the channel 0 transfer completion interrupt is asserted.\n13. Clear the channel 0 DMA interrupt and read the DMA status register.\n14. Read the received control data from the DMA destination address.\n15. If the data type field in the control data indicates a long packet (data type > 0xF), extract the word count, calculate the 8-byte aligned transfer size, program and start a DMA transfer on channel 1 for the data payload.\n16. Poll the DMA interrupt status register until the channel 1 transfer completion interrupt is asserted.\n17. Read the DMA interrupt status register again after channel 1 completion, then clear the channel 1 DMA interrupt.\n18. Repeat steps 10-17 for all packets in the current lane configuration.\n19. Repeat steps 8-18 for all lane counts (4, 3, 2, 1).\n20. Signal test completion with pass status.","Impacted Registers":"virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED","Validation / Acceptance Criteria":"1. The PHY stop state register must read the expected value indicating all data lanes and clock lane are in stop state before proceeding with lane configuration.\n2. DMA channel 0 transfer completion must be confirmed by polling the DMA interrupt status register until the channel 0 completion bit is asserted.\n3. The received control data must be read and its data type field evaluated to determine whether a data payload transfer is required.\n4. When a long packet is detected, the word count must be correctly extracted and the transfer size must be 8-byte aligned before initiating the DMA data transfer on channel 1.\n5. DMA channel 1 transfer completion must be confirmed by polling the DMA interrupt status register until the channel 1 completion bit is asserted.\n6. DMA interrupts must be properly cleared after each channel transfer completion.\n7. The test must iterate through all four lane configurations (4, 3, 2, 1 lanes) and process all packets for each configuration.\n8. The test must complete successfully by calling the finish routine with a pass indication.","Code Generation":""},
    {"Index":"2","SS / Module":"MIPI_CSI","Test Case Name":"mipi_csi2_test_pattern_generator","Feature":"Test Pattern Generator and CSI-2 Data Reception via DMA","Test Description":"Verify the MIPI CSI-2 internal test pattern generator by configuring the CSI-2 subsystem virtual channel, enabling CSI-2 and DMA interrupts, initializing the D-PHY, waiting for PHY stop state, programming DMA address registers and transfer parameters, enabling the pattern generator with configured vertical resolution, horizontal resolution, and pattern configuration, then disabling the pattern generator and polling for DMA transfer completion to confirm successful data reception.","Speed":"NA","Mode":"NA","Memory Start Offset":"0xE6000000","Memory End Offset":"0xE6040080","Remarks":"The testcase uses external functions snps_phy_init(), csi2_subsys_enable_interrupt(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), and finish() whose implementations are not available in the testcase folder. The csi2_enable_interrupt() function is defined locally but is called indirectly via the external csi2_subsys_enable_interrupt() function. The GDMA path selection (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) and FPS60 are compile-time conditionals. The MIZAR_MIPI_CSI2_RB_REG_BASE token used with offset +0xf4 could not be mapped to a named register by Agent 4 (status: unresolved). The register at offset 0xf4 in the csi_reg block corresponds to enableclkgating_csictrl per the specification document.","Test Steps / Procedure":"1. Set the interrupt pending flag and configure the virtual channel ID (vcid=3) and compute the unselected path value.\n2. Select the GDMA path via compile-time conditional and compute the virtual channel wrap register value accordingly.\n3. Compute the GDMA register base address from the selected GDMA path.\n4. Write the virtual channel register with the computed virtual channel wrap value.\n5. Disable control data transfer by writing 0 to the control data register.\n6. Enable CSI-2 and DMA interrupts by calling the subsystem interrupt enable routine, which reads the main interrupt status register to clear pending interrupts and then enables all CSI-2 interrupt masks (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, ECC corrected).\n7. Initialize the D-PHY by calling the SNPS PHY initialization routine.\n8. Poll the PHY stop state register until the PHY enters stop state (all data lanes and clock lane in stop state).\n9. Set horizontal resolution to 320, vertical resolution to 16, and valid bits per pixel to 24.\n10. Calculate the 8-byte aligned total DMA transfer size based on the resolution and pixel depth.\n11. Program the higher-order ARM DMA address registers for channel 0 read and write paths.\n12. Enable the fractional divider output to the CSI-2 subsystem by writing to the clock gating control register.\n13. Set the DMA channel 0 program counter address and configure the destination address increment flag.\n14. Program the DMA transfer instructions with source address, destination address, transfer size, and address increment flags.\n15. Start DMA channel 0 transfer.\n16. Enable the test pattern generator by writing vertical resolution, horizontal resolution, pattern configuration, and pattern enable.\n17. Wait for 100 cycles to allow pattern generation.\n18. Disable the test pattern generator by writing 0 to the pattern enable register.\n19. Poll the DMA interrupt status register until the channel 0 transfer completion interrupt (bit 0) is asserted.\n20. Wait for 10000 cycles for post-transfer settling.\n21. Signal test completion with pass status.","Impacted Registers":"PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED","Validation / Acceptance Criteria":"1. The PHY stop state register must read the expected value indicating all data lanes and clock lane are in stop state before proceeding with DMA and pattern generator configuration.\n2. The DMA transfer size must be correctly calculated as 8-byte aligned based on the configured horizontal resolution, vertical resolution, and bits per pixel.\n3. DMA channel 0 transfer completion must be confirmed by polling the DMA interrupt status register until the channel 0 completion bit (bit 0) is asserted.\n4. The test pattern generator must be properly configured with vertical resolution, horizontal resolution, and pattern configuration before being enabled.\n5. The pattern generator must be disabled after the generation window to stop pattern output.\n6. The higher-order DMA address registers must be correctly programmed for channel 0 read and write paths.\n7. The fractional divider output to the CSI-2 subsystem must be enabled.\n8. The test must complete successfully by calling the finish routine with a pass indication.","Code Generation":""}
]

# TestPlan columns
tp_cols = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation"]

# MetaData columns
md_cols = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]

# Full metadata from input
meta_data = [
    {"Index":"1","Test Case Name":"mipi_csi2_dphy_lanes_test","Meta Test Description":"This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through all lane counts (4 lanes down to 1 lane). For each lane configuration, it programs the CSI-2 host controller with the number of active lanes, triggers a CSI-2 sequence, and then performs repeated DMA transfers to receive both control packets and data packets from the CSI-2 interface.\n\nThe test begins by calling csi2_enable_interrupt() which reads INT_ST_MAIN to clear pending interrupts, then enables all CSI-2 interrupt masks (PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, ECC_CORRECTED). It then configures the virtual channel register and enables control data transfer. The D-PHY is initialized via snps_phy_init() and the test polls PHY_STOPSTATE until the PHY enters stop state (value 0x1000f).\n\nFor each lane count (3 down to 0), the test configures N_LANES, triggers the CSI-2 sequence by writing (lane_num+1) to address 0xa0243ffc, then enters an inner loop iterating (VRES*3)+2 times. Each inner iteration performs a DMA control-packet transfer on channel 0 (8 bytes from src 0x8000 to dest 0xE6001000), polls DMA interrupt status until bit 0 is set, clears the DMA interrupt, reads the control data from 0xE6001000, and if the data type field (bits[5:0]) is greater than 0xf, calculates the word count and performs a DMA data transfer on channel 1 (variable size from src 0x0000 to dest 0xE6002000), polls DMA interrupt status until bit 1 is set, reads interrupt status again, and clears the DMA interrupt. The test completes by calling finish(0).","Meta Test Steps / Procedure":"1. Entry: test_case() is called.\n2. Declare local variables.\n3. Call csi2_enable_interrupt().\n4-17. Enable all CSI-2 interrupt masks.\n18-24. Configure virtual channel and control data.\n25. Call snps_phy_init().\n26-29. Poll PHY_STOPSTATE until 0x1000f.\n30-31. Set DMA program counters.\n32-69. Lane iteration loop with DMA transfers and interrupt polling.","Meta Impacted Registers":"MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED","Meta Validation / Acceptance Criteria":"1. PHY Stop State Polling until rd_data == 0x1000f.\n2. DMA Channel 0 Completion Polling.\n3. Control Data Type Check.\n4. Word Count Extraction.\n5. Data Size Alignment.\n6. DMA Channel 1 Completion Polling.\n7-10. Interrupt clearing and test completion.","Meta Headers":"<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"","Meta Macros":"#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 1080 (if GDMA0_FULL_MEM defined), else 3\n#define HRES 1920 (if GDMA0_FULL_MEM defined), else 64\n#define DATA_TYPE CSI2_RGB888","Meta Arrays":"NA"},
    {"Index":"2","Test Case Name":"mipi_csi2_test_pattern_generator","Meta Test Description":"This testcase validates the MIPI CSI-2 internal test pattern generator (PPI PG) functionality. It configures the CSI-2 subsystem virtual channel, enables CSI-2 and DMA interrupts, initializes the D-PHY, polls PHY_STOPSTATE until the PHY enters stop state (expected value 0x1000f), programs the DMA address registers for higher-order ARM DMA address bits, enables the fractional divider output to the CSI-2 subsystem by writing to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, programs a DMA transfer using dma_trnsfr_instn_preload_incr_addr() with calculated transfer size based on hres=320, vres=16, valid_bits_per_pixel=24, starts the DMA via DMAGO_CSI(), enables the pattern generator by calling csi2_ctrlr_pg_enable() which writes PG_VRES=0x10, PG_HRES=0x70140, PG_CONFIG=0xe401, PG_ENABLE=1, waits 100 cycles via wait_on(100), disables the pattern generator by writing PG_ENABLE=0, polls the DMA interrupt status register until bit 0 is set indicating DMA channel 0 transfer completion, waits 10000 cycles via wait_on(10000), and calls finish(0) to signal test pass.","Meta Test Steps / Procedure":"1. Entry: test_case() is called.\n2-9. Configure VCID and GDMA path.\n10-11. Write virtual channel and control data registers.\n12-13. Enable interrupts and init PHY.\n14-17. Poll PHY_STOPSTATE.\n18-21. Set resolution and calculate transfer size.\n22-26. Program DMA address registers and enable fracdiv.\n27-31. Program and start DMA transfer.\n32-37. Enable pattern generator.\n38-39. Wait and disable pattern generator.\n40-45. Poll DMA completion and finish.","Meta Impacted Registers":"MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED","Meta Validation / Acceptance Criteria":"1. PHY Stop State Polling until rd_data == 0x1000f.\n2. DMA Transfer Size Calculation validation.\n3. DMA Channel 0 Completion Polling.\n4. Pattern Generator Enable/Disable Sequence.\n5-9. Configuration validation and test completion.","Meta Headers":"<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"","Meta Macros":"#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000","Meta Arrays":"NA"}
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_align = Alignment(wrap_text=True, vertical="top")

# Write headers
for col_idx, col_name in enumerate(tp_cols, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Write data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(tp_cols, 1):
        val = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns
for col_idx, col_name in enumerate(tp_cols, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        val = str(ws_tp.cell(row=row_idx, column=col_idx).value or "")
        lines = val.split("\n")
        for line in lines:
            if len(line) > max_len:
                max_len = len(line)
    width = min(max_len + 2, 80)
    ws_tp.column_dimensions[ws_tp.cell(row=1, column=col_idx).column_letter].width = width

# --- MetaData Sheet ---
ws_md = wb.create_sheet("MetaData")

for col_idx, col_name in enumerate(md_cols, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

for row_idx, row_data in enumerate(meta_data, 2):
    for col_idx, col_name in enumerate(md_cols, 1):
        val = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

ws_md.freeze_panes = "A2"

for col_idx, col_name in enumerate(md_cols, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(meta_data) + 2):
        val = str(ws_md.cell(row=row_idx, column=col_idx).value or "")
        lines = val.split("\n")
        for line in lines:
            if len(line) > max_len:
                max_len = len(line)
    width = min(max_len + 2, 80)
    ws_md.column_dimensions[ws_md.cell(row=1, column=col_idx).column_letter].width = width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save
wb.save(filename)
print(f"SUCCESS: {filename}")
print(f"SIZE: {os.path.getsize(filename)}")

# Verify
from openpyxl import load_workbook
wb2 = load_workbook(filename)
print(f"SHEETS: {wb2.sheetnames}")
print(f"TP_ROWS: {ws_tp.max_row - 1}")
print(f"MD_ROWS: {ws_md.max_row - 1}")

# Output base64 for upload
with open(filename, "rb") as f:
    b64 = base64.b64encode(f.read()).decode("ascii")
print(f"BASE64_LENGTH: {len(b64)}")

# Write base64 to a text file for retrieval
with open(filename + ".b64", "w") as f:
    f.write(b64)
print(f"B64_FILE: {filename}.b64")
