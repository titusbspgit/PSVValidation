#!/usr/bin/env python3
"""MIPI CSI TestPlan XLSX Generator - Agent7 Auto-execution
Generates a real openpyxl workbook and commits it back to the repository.
"""
import os
import sys
import base64
import json
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment

def generate_workbook():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"

    # Input data
    data = {
        "Index": "1",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "D-PHY Lane Configuration",
        "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "test_common.h"\n#include "mipi_csi2.h"',
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 3\n#define HRES 64\n#define DATA_TYPE CSI2_RGB888",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "0xE6000000",
        "Memory End Offset": "0xE6002000",
        "Meta Test Description": "This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through lane counts from 4 lanes down to 1 lane. For each lane configuration, the test performs a complete CSI-2 data reception sequence including control packet DMA transfers and data packet DMA transfers. The test begins by enabling all CSI-2 host interrupt masks (PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, ECC_CORRECTED). It then configures the virtual channel register and enables control data transfer. D-PHY initialization is performed via snps_phy_init(), followed by polling PHY_STOPSTATE until the value equals 0x1000f. The test then iterates lane_num from 3 down to 0, writing N_LANES register with lane_num, triggering the CSI-2 sequence by writing (lane_num+1) to address 0xa0243ffc, and then looping through cntrl_pkt_cnt = ((VRES3) + 2) packets. For each packet iteration, DMA channel 0 is programmed for control data transfer (8 bytes from src 0x8000 to dest 0xE6001000), DMA is started via DMAGO_CSI, and the test polls MIPI_CSI2_DMA_INTMIS_OFFSET for bit 0 completion. After clearing the interrupt, the control data is read from 0xE6001000. If the data type field (bits[5:0]) is greater than 0xf, the word_count is extracted from bits[21:6], csi_data_size is computed as 8-byte aligned word_count, and DMA channel 1 is programmed for data transfer (csi_data_size bytes from src 0x0000 to dest 0xE6002000). DMA channel 1 completion is polled via INTMIS bit 1, followed by interrupt clear. The test completes by calling finish(0).",
        "Test Description": "Validate MIPI CSI-2 D-PHY lane configuration by iterating from 4 lanes down to 1 lane. For each lane count, enable CSI-2 host interrupts, configure virtual channel, initialize D-PHY, poll for PHY stop state, trigger CSI-2 sequence, and perform DMA-based control and data packet transfers with interrupt-driven completion polling. Verify successful reception across all lane configurations.",
        "Meta Test Steps / Procedure": "1. Enter test_case() function.\n2. Declare local variables: rx_desc, tx_desc (long long int), gdma_tx_trnsfr_size, gdma_trnsfr_size (long long int), cntrl_pkt_cnt (int).\n3. printf(\"start line\\n\").\n4. Call csi2_enable_interrupt().\n5. [Inside csi2_enable_interrupt()] Declare local int rd_data.\n6. [Inside csi2_enable_interrupt()] rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) \u2014 read INT_ST_MAIN to clear interrupts.\n7. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f) \u2014 enable phy_fatal interrupts.\n8. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003) \u2014 enable pkt_fatal interrupts.\n9. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f) \u2014 enable phy interrupts.\n10. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f) \u2014 enable line interrupts.\n11. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff) \u2014 enable boundary frame fatal interrupts.\n12. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff) \u2014 enable seq frame fatal interrupts.\n13. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff) \u2014 enable crc frame fatal interrupts.\n14. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff) \u2014 enable pld crc fatal interrupts.\n15. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff) \u2014 enable data_id interrupts.\n16. [Inside csi2_enable_interrupt()] write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff) \u2014 enable ecc corrected interrupts.\n17. [Return from csi2_enable_interrupt()].\n18. Conditional compilation: set vcid_csi2_wrap_reg based on GDMA path. Default GDMA0_PATH: vcid_csi2_wrap_reg = (VC_ID << 12), printf(\"VC_ID=%d\\n\", VC_ID), gdma_path = 0.\n19. gdma_reg_base = 0xE6A00000.\n20. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) \u2014 write virtual channel register (first occurrence).\n21. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1) \u2014 enable control data transfer (first occurrence).\n22. printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg).\n23. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) \u2014 write virtual channel register (second occurrence).\n24. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1) \u2014 enable control data transfer (second occurrence).\n25. Call snps_phy_init() \u2014 D-PHY initialization sequence (external function, implementation not in testcase folder).\n26. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) \u2014 first read of PHY_STOPSTATE.\n27. while(!(rd_data == 0x1000f)): poll loop \u2014 rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) \u2014 repeated read until rd_data equals 0x1000f.\n28. ch0_pc = 0xE6000000.\n29. ch1_pc = 0xE6000500.\n30. Begin outer for loop: lane_num = 3; lane_num >= 0; lane_num-- (iterates 4 times: lane_num=3, 2, 1, 0).\n31. write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num) \u2014 configure number of lanes in CSI-2 controller.\n32. cntrl_pkt_cnt = ((VRES * 3) + 2). With default VRES=3: cntrl_pkt_cnt = 11.\n33. printf(\"DEBUG: cntrl_pkt_cnt=%d\\n\", cntrl_pkt_cnt).\n34. write_reg(0xa0243ffc, (lane_num + 1)) \u2014 trigger CSI-2 sequence for current lane count.\n35. Begin inner for loop: i = 0; i < cntrl_pkt_cnt; i++ (iterates cntrl_pkt_cnt times per lane configuration).\n36. ch0_preload_loc = ch0_pc.\n37. ch1_preload_loc = ch1_pc.\n38. write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3) \u2014 enable dma_irq[1] and dma_irq[0].\n39. Call dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0) \u2014 program DMA channel 0 control data transfer: src=0x8000, dest=0xE6001000, size=8 bytes, irq_num=0.\n40. Call DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0) \u2014 start DMA channel 0.\n41. rd_data = 0.\n42. while((rd_data & 0x1) == 0x0): poll loop \u2014 rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) \u2014 poll DMA interrupt status for channel 0 completion (bit 0). printf(\"polling irq; irq_status_reg rd_data =%0x\\n\", rd_data) inside loop.\n43. write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1) \u2014 clear DMA channel 0 interrupt.\n44. rd_data = read_reg(gdma_reg_base + 0x28) \u2014 read DMA interrupt status register after clear.\n45. printf(\"DEBUG : irq polling completed ch0; rd_data = %0x \\n\", rd_data).\n46. csi_ctrl_data = read_reg(0xE6001000) \u2014 read received control data.\n47. printf(\"DEBUG: csi_ctrl_data=%0x\\n\", csi_ctrl_data).\n48. Condition: if((csi_ctrl_data & 0x3f) > 0xf) \u2014 check if data type field indicates a long packet (data type > 0xf).\n49. [If true] word_count = ((csi_ctrl_data >> 6) & 0xffff) \u2014 extract word count from bits[21:6].\n50. [If true] csi_data_size = (word_count % 8) ? (word_count / 8 + 1) * 8 : word_count \u2014 compute 8-byte aligned transfer size.\n51. [If true] Call dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1) \u2014 program DMA channel 1 data transfer: src=0x0000, dest=0xE6002000, size=csi_data_size, irq_num=1.\n52. [If true] Call DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1) \u2014 start DMA channel 1.\n53. [If true] rd_data = 0.\n54. [If true] while((rd_data & 0x2) == 0x0): poll loop \u2014 rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) \u2014 poll DMA interrupt status for channel 1 completion (bit 1). printf(\"polling irq; irq_status_reg rd_data =%0x\\n\", rd_data) inside loop.\n55. [If true] rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) \u2014 additional read of DMA interrupt status after poll exit.\n56. [If true] write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2) \u2014 clear DMA channel 1 interrupt.\n57. [If true] printf(\"DEBUG : irq polling completed ch1; rd_data=%0x\\n\", rd_data).\n58. End of if block (data transfer branch).\n59. End of inner for loop (packet iteration).\n60. End of outer for loop (lane iteration).\n61. Call finish(0) \u2014 testcase completion with pass status.",
        "Test Steps / Procedure": "1. Enable all CSI-2 host interrupt masks by reading the main interrupt status register to clear pending interrupts, then writing enable values to PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, and ECC_CORRECTED interrupt mask registers.\n2. Configure the virtual channel register with the appropriate VC_ID based on the selected GDMA path.\n3. Enable control data transfer by writing to the control_data register.\n4. Repeat virtual channel and control data configuration (second write).\n5. Perform D-PHY initialization sequence.\n6. Poll the PHY_STOPSTATE register until the D-PHY enters stop state (expected value 0x1000f).\n7. Iterate lane configurations from 4 lanes down to 1 lane (lane_num = 3 to 0):\n a. Write the N_LANES register with the current lane count.\n b. Trigger the CSI-2 sequence for the current lane configuration.\n c. For each control/data packet in the frame:\n i. Enable DMA interrupts for both channels.\n ii. Program and start DMA channel 0 for control data transfer (8 bytes).\n iii. Poll DMA interrupt status for channel 0 completion.\n iv. Clear channel 0 DMA interrupt.\n v. Read the received control data.\n vi. If the data type indicates a long packet, extract word count, compute aligned transfer size, program and start DMA channel 1 for data transfer.\n vii. Poll DMA interrupt status for channel 1 completion.\n viii. Clear channel 1 DMA interrupt.\n8. Complete the test with pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000",
        "Impacted Registers": "INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED; virtual_channel; control_data; PHY_STOPSTATE; N_LANES",
        "Meta Validation / Acceptance Criteria": "1. After csi2_enable_interrupt(): read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) completes successfully to clear pending interrupts. All 10 interrupt mask registers are written with their respective enable values: INT_MSK_PHY_FATAL=0x0000000f, INT_MSK_PKT_FATAL=0x00000003, INT_MSK_PHY=0x000f000f, INT_MSK_LINE=0x000f000f, INT_MSK_BNDRY_FRAME_FATAL=0x0000ffff, INT_MSK_SEQ_FRAME_FATAL=0x0000ffff, INT_MSK_CRC_FRAME_FATAL=0x0000ffff, INT_MSK_PLD_CRC_FATAL=0x0000ffff, INT_MSK_DATA_ID=0x0000ffff, INT_MSK_ECC_CORRECTED=0x0000ffff.\n2. PHY_STOPSTATE poll: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) must eventually return 0x1000f indicating D-PHY stop state is reached on all lanes and clock lane.\n3. DMA channel 0 completion poll: (rd_data & 0x1) != 0x0 \u2014 bit 0 of MIPI_CSI2_DMA_INTMIS_OFFSET must be set, indicating DMA channel 0 transfer complete. This poll occurs once per packet iteration per lane configuration.\n4. Control data validation: csi_ctrl_data = read_reg(0xE6001000). The data type field (csi_ctrl_data & 0x3f) is checked against 0xf. If > 0xf, it is a long packet requiring data transfer.\n5. Word count extraction: word_count = ((csi_ctrl_data >> 6) & 0xffff). csi_data_size is computed as 8-byte aligned: (word_count % 8) ? (word_count / 8 + 1) * 8 : word_count.\n6. DMA channel 1 completion poll: (rd_data & 0x2) != 0x0 \u2014 bit 1 of MIPI_CSI2_DMA_INTMIS_OFFSET must be set, indicating DMA channel 1 transfer complete. This poll occurs once per long-packet iteration per lane configuration.\n7. DMA interrupt clear: channel 0 cleared with write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1), channel 1 cleared with write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2).\n8. Test completion: finish(0) is called indicating test pass. The test iterates through all 4 lane configurations (lane_num=3,2,1,0) and for each processes cntrl_pkt_cnt = ((VRES3)+2) packets successfully.",
        "Validation / Acceptance Criteria": "1. D-PHY enters stop state successfully as indicated by PHY_STOPSTATE register returning the expected value.\n2. DMA channel 0 completes control data transfer for every packet, confirmed by DMA interrupt status bit 0 assertion.\n3. For long packets (data type > 0xf), DMA channel 1 completes data transfer, confirmed by DMA interrupt status bit 1 assertion.\n4. All four lane configurations (4-lane, 3-lane, 2-lane, 1-lane) complete the full packet sequence without errors.\n5. Test completes with pass status via finish(0).",
        "Remarks": "External functions snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() are not defined within the testcase folder and their implementations are not inspectable. The GDMA path is selected via conditional compilation; default path assumed is GDMA0_PATH. VRES and HRES have two possible values depending on GDMA0_FULL_MEM define: VRES=1080/HRES=1920 or VRES=3/HRES=64 (default). The macro SOFT_RST_REG_ADDRESS was ignored per instruction. Two hex addresses 0xa0243ffc and 0xE6001000 could not be mapped to named registers."
    }

    # Create workbook
    wb = Workbook()

    # ===== TestPlan Sheet =====
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    tp_columns = [
        "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
        "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
        "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
        "Code Generation"
    ]

    # Header row
    header_font = Font(bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_alignment = Alignment(wrap_text=True, vertical="top")

    for col_idx, col_name in enumerate(tp_columns, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    # Data row
    tp_row_data = [
        data.get("Index", ""),
        data.get("SS / Module", ""),
        data.get("Feature", ""),
        data.get("Test Case Name", ""),
        data.get("Test Description", ""),
        data.get("Speed", ""),
        data.get("Mode", ""),
        data.get("Memory Start Offset", ""),
        data.get("Memory End Offset", ""),
        data.get("Remarks", ""),
        data.get("Test Steps / Procedure", ""),
        data.get("Impacted Registers", ""),
        data.get("Validation / Acceptance Criteria", ""),
        ""  # Code Generation - empty
    ]

    for col_idx, val in enumerate(tp_row_data, 1):
        cell = ws_tp.cell(row=2, column=col_idx, value=val)
        cell.alignment = wrap_alignment

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # Auto-size columns with max width cap
    max_width = 60
    for col_idx, col_name in enumerate(tp_columns, 1):
        max_len = len(str(col_name))
        for row in ws_tp.iter_rows(min_row=2, max_row=ws_tp.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
        adjusted_width = min(max_len + 4, max_width)
        ws_tp.column_dimensions[ws_tp.cell(row=1, column=col_idx).column_letter].width = adjusted_width

    # ===== MetaData Sheet =====
    ws_md = wb.create_sheet(title="MetaData")

    md_columns = [
        "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
        "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
        "Meta Headers", "Meta Macros", "Meta Arrays"
    ]

    for col_idx, col_name in enumerate(md_columns, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    md_row_data = [
        data.get("Index", ""),
        data.get("Test Case Name", ""),
        data.get("Meta Test Description", ""),
        data.get("Meta Test Steps / Procedure", ""),
        data.get("Meta Impacted Registers", ""),
        data.get("Meta Validation / Acceptance Criteria", ""),
        data.get("Meta Headers", ""),
        data.get("Meta Macros", ""),
        data.get("Meta Arrays", "")
    ]

    for col_idx, val in enumerate(md_row_data, 1):
        cell = ws_md.cell(row=2, column=col_idx, value=val)
        cell.alignment = wrap_alignment

    # Freeze first row
    ws_md.freeze_panes = "A2"

    # Auto-size columns
    for col_idx, col_name in enumerate(md_columns, 1):
        max_len = len(str(col_name))
        for row in ws_md.iter_rows(min_row=2, max_row=ws_md.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
        adjusted_width = min(max_len + 4, max_width)
        ws_md.column_dimensions[ws_md.cell(row=1, column=col_idx).column_letter].width = adjusted_width

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save workbook
    output_path = os.path.join(os.getcwd(), filename)
    wb.save(output_path)

    # Validate
    assert os.path.exists(output_path), "File does not exist after save"
    assert os.path.getsize(output_path) > 0, "File size is 0"
    vwb = load_workbook(output_path)
    assert "TestPlan" in vwb.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in vwb.sheetnames, "MetaData sheet missing"
    vwb.close()

    print(f"SUCCESS: Generated {filename} at {output_path}")
    print(f"File size: {os.path.getsize(output_path)} bytes")

    # Output base64 for GitHub upload
    with open(output_path, 'rb') as f:
        b64_content = base64.b64encode(f.read()).decode('utf-8')

    # Write base64 to a file for the workflow to pick up
    with open(os.path.join(os.getcwd(), 'xlsx_base64.txt'), 'w') as f:
        f.write(b64_content)

    with open(os.path.join(os.getcwd(), 'xlsx_filename.txt'), 'w') as f:
        f.write(filename)

    return filename, output_path, b64_content

if __name__ == "__main__":
    generate_workbook()
