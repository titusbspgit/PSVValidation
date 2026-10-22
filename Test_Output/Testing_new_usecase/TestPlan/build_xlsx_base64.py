#!/usr/bin/env python3
"""
Agent 7 - Excel Generation Script
Generates MIPI_CSI TestPlan workbook with 2 testcases.
Outputs base64-encoded xlsx to stdout.
"""
import json, base64, io, sys
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side

META_JSON = [
    {
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"",
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#ifdef DPHY_LANES_TEST\n#ifndef LS_LE_EN\n#define LS_LE_EN 1\n#endif\n#ifndef TOTAL_FRAME\n#define TOTAL_FRAME 1\n#endif\n#ifndef VC_ID\n#define VC_ID 3\n#endif\n#ifndef VRES\n#ifdef GDMA0_FULL_MEM\n#define VRES 1080\n#else\n#define VRES 3\n#endif\n#endif\n#ifndef HRES\n#ifdef GDMA0_FULL_MEM\n#define HRES 1920\n#else\n#define HRES 64\n#endif\n#endif\n#ifndef DATA_TYPE\n#define DATA_TYPE CSI2_RGB888\n#endif",
        "Meta Arrays": "NA",
        "Meta Types": "int data_rd, data_wr;\nint def_fail_cnt = 0, wr_fail_cnt = 0;\nint vcid_csi2_wrap_reg;\nint gdma_path;\nunsigned int ch0_pc, ch0_preload_loc;\nunsigned int ch1_pc, ch1_preload_loc;\nunsigned int gdma_reg_base;\nunsigned int rd_data;\nlong int csi_ctrl_data;\nint gdma_int_rsts;\nlong int word_count;\nint csi_data_size;\nlong long int rx_desc, tx_desc;\nlong long int gdma_tx_trnsfr_size, gdma_trnsfr_size;\nint cntrl_pkt_cnt;",
        "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY lane configurations by iterating from 4 active lanes down to 1 active lane. For each lane configuration, the test configures the CSI-2 host controller N_LANES register, sets up DMA transfers using dma_trnsfr_instn_preload for both control data (channel 0) and CSI data (channel 1), triggers the CSI-2 sequence, and validates DMA transfer completion via interrupt status polling on both DMA IRQ channels. The test initializes the D-PHY via snps_phy_init(), polls PHY_STOPSTATE until all lanes and clock lane enter stop state (0x1000F), configures the virtual channel register and control data register, and then loops through lane configurations performing DMA data transfers with data comparison validation using failure counters (def_fail_cnt, wr_fail_cnt).",
        "Meta Test Steps / Procedure": "Step 1: Call printf(\"start line\\n\").\nStep 2: Call csi2_enable_interrupt() to enable CSI-2 interrupt.\nStep 3: GDMA path selection via preprocessor conditionals:\n  - #ifdef GDMA3_PATH: vcid_csi2_wrap_reg = VC_ID; gdma_path = 3;\n  - #elif GDMA2_PATH: vcid_csi2_wrap_reg = (VC_ID << 4); gdma_path = 2;\n  - #elif GDMA1_PATH: vcid_csi2_wrap_reg = (VC_ID << 8); gdma_path = 1;\n  - #elif GDMA0_PATH: vcid_csi2_wrap_reg = (VC_ID << 12); printf(\"VC_ID=%d\\n\", VC_ID); gdma_path = 0;\nStep 4: Set gdma_reg_base = 0xE6A00000.\nStep 5: write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) \u2014 Write CSI-2 virtual channel register.\nStep 6: write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1) \u2014 Enable control data transfer.\nStep 7: printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg).\nStep 8: csi2_subsys_enable_interrupt() \u2014 commented out.\nStep 9: Call snps_phy_init() \u2014 D-PHY initialization sequence.\nStep 10: rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) \u2014 Read PHY stop state.\nStep 11: Polling loop: while(!(rd_data == 0x1000f)) \u2014 Wait for PHY to enter stop state on all lanes.\nStep 12: Set ch0_pc = 0xE6000000.\nStep 13: Set ch1_pc = 0xE6000500.\nStep 14: for(int lane_num=3; lane_num>=0; lane_num--) \u2014 Loop from 4 lanes down to 1 lane:\n  Step 14a: write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num).\n  Step 14b: cntrl_pkt_cnt = ((VRES * 3) + 2).\n  Step 14c: printf(\"DEBUG: cntrl_pkt_cnt=%d\\n\", cntrl_pkt_cnt).\n  Step 14d: write_reg(0xa0243ffc, (lane_num + 1)).\n  Step 14e: for(int i = 0; i < cntrl_pkt_cnt; i++) \u2014 Inner loop for each packet:\n    Step 14e-i through 14e-xiii: DMA transfer programming, interrupt polling, and clearing.\nStep 15: finish(0) \u2014 End test.",
        "Meta Impacted Registers": "DATA_IDS_VC_1\nPPI_PG_PATTERN_VRES\nPPI_PG_PATTERN_HRES",
        "Meta Validation / Acceptance Criteria": "Validation 1: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until rd_data == 0x1000f.\nValidation 2: DMA interrupt status polling for control packet (bit 0).\nValidation 3: DMA interrupt status polling for data packet (bit 1).\nValidation 4: DMA interrupt clear for control packet.\nValidation 5: DMA interrupt clear for data packet.\nValidation 6: DMA status read.\nValidation 7: Data comparison \u2014 def_fail_cnt == 0 and wr_fail_cnt == 0.\nValidation 8: Control packet count computation.\nValidation 9: Lane iteration from 4 to 1.\nValidation 10: Test completion \u2014 finish(0)."
    },
    {
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"",
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
        "Meta Arrays": "NA",
        "Meta Types": "int data_rd, data_wr;\nint def_fail_cnt = 0, wr_fail_cnt = 0;\nint vcid_csi2_wrap_reg;\nint gdma_path;\nlong int csi_ctrl_data;\nint gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;\nunsigned int rd_data;\nunsigned int tx_trnsfr_size;\nlong long int rx_desc, tx_desc;\nlong long int gdma_tx_trnsfr_size, gdma_trnsfr_size;\nint cntrl_pkt_cnt;\nint int_pend;\nint vcid;\nint vcid_unselected_path;\nint hres, vres;\nint valid_bits_per_pixel;\nunsigned int dma_ch0_pc;\nunsigned int dma_ch0_instn_preload_addr;\nint dma_dest_addr_incr_flag;\nunsigned int csi2_data_trnsfr_size;\nunsigned int gdma_reg_base;",
        "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator functionality. The test configures virtual channel IDs and GDMA path selection via preprocessor conditionals, writes the CSI-2 wrapper virtual channel register and disables control data transfer. It computes the CSI-2 data transfer size based on hres=320, vres=16, and valid_bits_per_pixel=24 with 8-byte alignment. It programs DMA transfer instructions, starts DMA channel 0, enables the pattern generator, waits, disables the pattern generator, polls DMA interrupt status for transfer completion, and finishes the test.",
        "Meta Test Steps / Procedure": "Step 1: Call printf(\"start line\\n\").\nStep 2: Call csi2_enable_interrupt().\nStep 3: Set int_pend = 1.\nStep 4: Set vcid = 3.\nStep 5: Compute vcid_unselected_path = ((vcid + 1) & 0xf).\nStep 6: GDMA path selection via preprocessor conditionals.\nStep 7: Compute gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000).\nStep 8: write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\nStep 9: write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0).\nStep 10: Set hres = 320.\nStep 11: Set vres = 16.\nStep 12: Set valid_bits_per_pixel = 24.\nStep 13: Compute csi2_data_trnsfr_size.\nStep 14: Set dma_ch0_pc = 0xE6000000.\nStep 15: Set dma_ch0_instn_preload_addr = dma_ch0_pc.\nStep 16: Conditional dma_dest_addr_incr_flag.\nStep 17: Call dma_trnsfr_instn_preload_incr_addr().\nStep 18: Call DMAGO_CSI().\nStep 19: Call csi2_ctrlr_pg_enable() \u2014 writes PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, PPI_PG_ENABLE.\nStep 20: Call wait_on(100).\nStep 21: write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0).\nStep 22-23: Poll DMA interrupt status.\nStep 24: Call wait_on(10000).\nStep 25: Call finish(0).",
        "Meta Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "Validation 1: DMA interrupt status polling for data transfer completion (bit 0).\nValidation 2: Pattern generator enable.\nValidation 3: Pattern generator disable.\nValidation 4: CSI-2 data transfer size computation = 15360 bytes.\nValidation 5: Virtual channel register configuration.\nValidation 6: Control data disabled.\nValidation 7: DMA transfer programming.\nValidation 8: DMA channel start.\nValidation 9: Post-completion wait.\nValidation 10: Test completion \u2014 finish(0)."
    }
]

TESTPLAN_JSON = [
    {
        "Index": "1",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lane Configuration and DMA Data Transfer Validation",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase validates MIPI CSI-2 DPHY lane configurations by iterating from 4 active lanes down to 1 active lane. For each lane configuration, the test configures the CSI-2 host controller N_LANES register, sets up DMA transfers for both control data and CSI pixel data on separate DMA channels, triggers the CSI-2 receive sequence, and validates DMA transfer completion via interrupt status polling on both DMA IRQ channels. The test initializes the D-PHY, polls PHY_STOPSTATE until all data lanes and the clock lane enter stop state, configures the virtual channel register and enables control data transfer, and then loops through each lane configuration performing DMA data transfers with data comparison validation using failure counters.",
        "Test Steps / Procedure": "1. Enable CSI-2 interrupts.\n2. Select the GDMA path and configure the virtual channel ID mapping based on the selected path.\n3. Write the virtual channel register with the computed virtual channel mapping.\n4. Enable control data transfer by writing to the control data register.\n5. Initialize the D-PHY.\n6. Poll the PHY stop state register until all data lanes and the clock lane report stop state.\n7. Begin lane iteration loop from 4 active lanes down to 1 active lane.\n8. For each lane configuration, write the N_LANES register with the current lane count.\n9. Calculate the control packet count based on vertical resolution.\n10. Trigger the CSI-2 receive sequence for the current lane configuration.\n11. For each control packet iteration, enable DMA interrupts for both channels.\n12. Program DMA transfer instructions for control data on channel 0 and CSI pixel data on channel 1.\n13. Start the DMA transfer.\n14. Poll the DMA interrupt status register until the control packet transfer completes (IRQ channel 0).\n15. Clear the control packet DMA interrupt.\n16. Poll the DMA interrupt status register until the data packet transfer completes (IRQ channel 1).\n17. Clear the data packet DMA interrupt.\n18. Repeat for all packets in the current lane configuration.\n19. Repeat for all lane configurations (4 lanes to 1 lane).\n20. End the test.",
        "Impacted Registers": "DATA_IDS_VC_1, PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES",
        "Validation / Acceptance Criteria": "1. PHY stop state register must report all data lanes and clock lane in stop state before proceeding with lane configuration.\n2. DMA interrupt for control packet transfer (IRQ channel 0) must assert for each packet iteration.\n3. DMA interrupt for data packet transfer (IRQ channel 1) must assert for each packet iteration.\n4. DMA interrupts must be successfully cleared after each transfer completion.\n5. Data comparison between written and read-back values must pass with zero failure counts.\n6. The test must successfully complete DMA transfers for all lane configurations from 4 lanes down to 1 lane.\n7. The test must complete successfully with a pass indication.",
        "Remarks": "The test uses compile-time configuration for vertical resolution, horizontal resolution, virtual channel ID, data type (RGB888), and frame count. The GDMA path is selected via preprocessor conditionals. No timeout mechanism is implemented for PHY stop state or DMA interrupt polling loops."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "Internal Test Pattern Generator and DMA Data Transfer Validation",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator functionality. The test configures virtual channel IDs and GDMA path selection, writes the CSI-2 wrapper virtual channel register, and disables control data transfer. It computes the CSI-2 data transfer size based on a horizontal resolution of 320, vertical resolution of 16, and 24 valid bits per pixel with 8-byte alignment, resulting in a total transfer size of 15360 bytes. The test programs DMA transfer instructions with conditional destination address increment, starts DMA channel 0, enables the pattern generator, waits for data generation, disables the pattern generator, polls the DMA interrupt status for transfer completion, and finishes the test.",
        "Test Steps / Procedure": "1. Enable CSI-2 interrupts.\n2. Set the virtual channel ID and compute the unselected path virtual channel value.\n3. Select the GDMA path and configure the virtual channel ID mapping based on the selected path.\n4. Write the virtual channel register with the computed virtual channel mapping.\n5. Disable control data transfer by writing to the control data register.\n6. Set horizontal resolution to 320, vertical resolution to 16, and valid bits per pixel to 24.\n7. Compute the CSI-2 data transfer size with 8-byte alignment.\n8. Configure the DMA destination address increment flag based on frame rate configuration.\n9. Program DMA transfer instructions for channel 0 with the computed transfer size.\n10. Start DMA channel 0.\n11. Enable the pattern generator by writing the vertical resolution register, horizontal resolution register, pattern generator configuration register, and pattern generator enable register.\n12. Wait for the pattern generator to produce data.\n13. Disable the pattern generator by clearing the pattern generator enable register.\n14. Poll the DMA interrupt status register until the data transfer completes (IRQ channel 0 asserts).\n15. Wait for a post-completion cooldown period.\n16. End the test.",
        "Impacted Registers": "NA",
        "Validation / Acceptance Criteria": "1. DMA interrupt for data transfer completion (IRQ channel 0) must assert.\n2. The pattern generator must be successfully enabled.\n3. The pattern generator must be successfully disabled after the data generation wait period.\n4. The CSI-2 data transfer size must be correctly computed as 15360 bytes.\n5. The virtual channel register must be correctly configured.\n6. Control data transfer must be disabled.\n7. DMA channel 0 must be correctly programmed and started.\n8. The test must complete successfully with a pass indication.",
        "Remarks": "The test uses a fixed resolution of 320x16 with 24 bits per pixel (RGB888 format). The GDMA path is selected via compile-time preprocessor conditionals. No timeout mechanism is implemented for the DMA interrupt polling loop. All macros resolved to non-register addresses."
    }
]

TESTPLAN_COLUMNS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks"
]

META_COLUMNS = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

def build_workbook():
    wb = Workbook()

    # --- TestPlan Sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=True)
    cell_align = Alignment(vertical="top", wrap_text=True)
    thin_border = Border(
        left=Side(style="thin"),
        right=Side(style="thin"),
        top=Side(style="thin"),
        bottom=Side(style="thin")
    )

    # Write TestPlan headers
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align
        cell.border = thin_border

    # Write TestPlan data
    for row_idx, record in enumerate(TESTPLAN_JSON, 2):
        for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
            value = record.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_align
            cell.border = thin_border

    # Auto-size TestPlan columns
    tp_widths = {
        "Index": 8, "SS / Module": 15, "Feature": 35, "Test Case Name": 38,
        "Speed": 10, "Mode": 10, "Memory Start Offset": 20, "Memory End Offset": 20,
        "Test Description": 60, "Test Steps / Procedure": 70,
        "Impacted Registers": 35, "Validation / Acceptance Criteria": 60, "Remarks": 50
    }
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        ws_tp.column_dimensions[chr(64 + col_idx) if col_idx <= 26 else None].width = tp_widths.get(col_name, 20)

    # Fix column letter for columns > 26
    from openpyxl.utils import get_column_letter
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        ws_tp.column_dimensions[get_column_letter(col_idx)].width = tp_widths.get(col_name, 20)

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # --- MetaData Sheet ---
    ws_meta = wb.create_sheet("MetaData")

    # Write MetaData headers
    for col_idx, col_name in enumerate(META_COLUMNS, 1):
        cell = ws_meta.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align
        cell.border = thin_border

    # Write MetaData data
    for row_idx, record in enumerate(META_JSON, 2):
        for col_idx, col_name in enumerate(META_COLUMNS, 1):
            value = record.get(col_name, "")
            cell = ws_meta.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_align
            cell.border = thin_border

    # Auto-size MetaData columns
    meta_widths = {
        "Test Case Name": 38, "Meta Headers": 40, "Meta Macros": 50,
        "Meta Arrays": 20, "Meta Types": 50, "Meta Test Description": 60,
        "Meta Test Steps / Procedure": 70, "Meta Impacted Registers": 35,
        "Meta Validation / Acceptance Criteria": 60
    }
    for col_idx, col_name in enumerate(META_COLUMNS, 1):
        ws_meta.column_dimensions[get_column_letter(col_idx)].width = meta_widths.get(col_name, 20)

    # Freeze first row
    ws_meta.freeze_panes = "A2"

    # Set MetaData sheet to veryHidden
    ws_meta.sheet_state = "veryHidden"

    return wb

def main():
    from openpyxl.utils import get_column_letter
    wb = build_workbook()
    buf = io.BytesIO()
    wb.save(buf)
    buf.seek(0)
    b64 = base64.b64encode(buf.read()).decode("ascii")
    sys.stdout.write(b64)

if __name__ == "__main__":
    main()
