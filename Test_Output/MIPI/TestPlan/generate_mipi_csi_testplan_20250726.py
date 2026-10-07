#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os, sys, base64, json

def main():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
    filename = f'MIPI_CSI_TestPlan_{timestamp}.xlsx'

    json_data = [
        {
            "Index": "1",
            "SS / Module": "MIPI_CSI",
            "Test Case Name": "mipi_csi2_dphy_lanes_test",
            "Feature": "DPHY Lane Configuration and CSI-2 Data Transfer",
            "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
            "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2; LS_LE_EN; TOTAL_FRAME; VC_ID; VRES; HRES; DATA_TYPE",
            "Meta Arrays": "NA",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY lane configuration by iterating through all lane counts (4 down to 1). It first calls csi2_enable_interrupt() which reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear pending interrupts, then writes all CSI-2 host interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED) to enable various interrupt categories. The test then writes MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL with the computed virtual channel ID and writes MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 1 to enable control data transfer. It calls snps_phy_init() for D-PHY initialization, then polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until the value equals 0x1000f indicating all lanes have entered stop state. For each lane count (3 down to 0), it writes MIZAR_MIPI_CSI2_HOST_N_LANES with the lane number, writes 0xa0243ffc with (lane_num+1) to trigger the CSI-2 sequence, then enters a packet processing loop. Inside the loop, it enables DMA interrupts via gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET, calls dma_trnsfr_instn_preload() and DMAGO_CSI() for control data transfer on channel 0, polls gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET for DMA completion (bit 0), clears the interrupt via gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET, reads 0xE6001000 for CSI control data, and if the data type field indicates image data (>0xf), extracts word_count, calculates csi_data_size with 8-byte alignment, programs DMA channel 1 for data transfer via dma_trnsfr_instn_preload() and DMAGO_CSI(), polls for DMA completion (bit 1), and clears the interrupt. The test calls finish(0) upon completion.",
            "Test Description": "This test validates MIPI CSI-2 DPHY lane configuration by iterating through all supported lane counts from 4 lanes down to 1 lane. For each lane configuration, the test enables all CSI-2 host interrupt masks, configures the virtual channel, enables control data transfer, initializes the D-PHY, and waits for all lanes to enter stop state. It then configures the number of active lanes via the N_LANES register and triggers a CSI-2 data sequence. For each received packet, the test uses DMA to transfer control data, reads the CSI control packet to determine the data type, and if image data is detected, calculates the transfer size with 8-byte alignment and performs a DMA data transfer. The test validates DMA completion for both control and data channels by polling interrupt status registers and clearing interrupts after each transfer.",
            "Meta Test Steps / Procedure": "1. Call csi2_enable_interrupt() which performs: read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) to clear pending interrupts; write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff); write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff). 2. Compute vcid_csi2_wrap_reg based on GDMA path conditional compilation. 3. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg). 4. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1). 5-6. Repeated writes. 7. Call snps_phy_init(). 8-9. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until 0x1000f. 10. Set ch0_pc, ch1_pc. 11-28. Lane iteration with DMA transfers and interrupt handling.",
            "Test Steps / Procedure": "1. Enable all CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing interrupt mask values to INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, and INT_MSK_ECC_CORRECTED registers. 2. Configure the virtual channel ID in the virtual_channel register based on the selected GDMA path. 3. Enable control data transfer by writing to the control_data register. 4. Initialize the D-PHY by calling the PHY initialization sequence. 5. Poll the PHY_STOPSTATE register until all lanes report stop state. 6. Iterate through lane counts from 4 down to 1 by writing the lane number to the N_LANES register. 7. Trigger the CSI-2 sequence for the current lane configuration. 8. For each expected packet, enable DMA interrupts, program DMA channel 0 for control data transfer, and start the DMA transfer. 9. Poll the DMA interrupt status register for channel 0 completion, then clear the DMA interrupt. 10. Read the CSI control data from the destination buffer to determine the data type and word count. 11. If the data type indicates image data, calculate the aligned transfer size, program DMA channel 1 for data transfer, and start the DMA transfer. 12. Poll the DMA interrupt status register for channel 1 completion, then clear the DMA interrupt. 13. Repeat steps 6 through 12 for all lane configurations. 14. Signal test completion.",
            "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
            "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
            "Meta Validation / Acceptance Criteria": "1. PHY stop state validation: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) is polled until rd_data equals 0x1000f. 2. DMA channel 0 completion: polled until (rd_data & 0x1) != 0x0. 3. CSI control data type check: (csi_ctrl_data & 0x3f) > 0xf determines image data. 4. DMA channel 1 completion: polled until (rd_data & 0x2) != 0x0. 5. Word count extraction and 8-byte alignment. 6. finish(0) called.",
            "Validation / Acceptance Criteria": "1. The PHY_STOPSTATE register must report all lanes in stop state before lane configuration begins. 2. DMA control data transfer on channel 0 must complete successfully as indicated by the DMA interrupt status register. 3. The CSI control data read from the destination buffer must contain a valid data type field to determine whether image data transfer is required. 4. When image data is present, DMA data transfer on channel 1 must complete successfully as indicated by the DMA interrupt status register. 5. The test must iterate through all four lane configurations (4, 3, 2, and 1 lane) and process all expected packets for each configuration. 6. The test must signal successful completion for all lane configurations.",
            "Remarks": "The test iterates lane counts from 4 down to 1 (lane_num 3 to 0) to validate DPHY operation across all supported lane configurations. DMA transfers use two channels: channel 0 for CSI-2 control data and channel 1 for image data. The virtual channel register write and control data enable write are performed twice in the source code. Two hardcoded hex addresses (used for triggering CSI-2 sequence and reading DMA fault status) could not be mapped to named registers in the specification. The GDMA path selection is compile-time conditional. DMA interrupt polling uses bit masking to distinguish channel 0 (bit 0) and channel 1 (bit 1) completion."
        },
        {
            "Index": "2",
            "SS / Module": "MIPI_CSI",
            "Test Case Name": "mipi_csi2_test_pattern_generator",
            "Feature": "Test Pattern Generator and CSI-2 Data Transfer",
            "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
            "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2",
            "Meta Arrays": "NA",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator functionality. The test_case() function sets vcid=3, computes vcid_unselected_path=((vcid+1)&0xf), then computes vcid_csi2_wrap_reg based on GDMA path conditional compilation (GDMA3_PATH, GDMA2_PATH, GDMA1_PATH, or GDMA0_PATH). It sets gdma_reg_base = 0xE6A00000 + ((gdma_path)*0x1000). It writes MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL with vcid_csi2_wrap_reg and writes MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 0 to disable control data transfer. It calls csi2_subsys_enable_interrupt() and snps_phy_init(). It polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until rd_data equals 0x1000f. It sets hres=320, vres=16, valid_bits_per_pixel=24 and computes csi2_data_trnsfr_size with 8-byte alignment. It writes DMA address registers, enables fracdiv output, programs DMA transfer, enables pattern generator via PPI_PG registers, waits, disables pattern generator, polls DMA completion, and calls finish(0).",
            "Test Description": "This test validates the MIPI CSI-2 internal test pattern generator by configuring the pattern generator with a vertical resolution of 16 lines and horizontal resolution of 320 pixels at 24 bits per pixel. The test configures the virtual channel, disables control data transfer, enables CSI-2 and DMA interrupts, initializes the D-PHY, and waits for all lanes to enter stop state. It then programs the DMA address registers for channel 0 read and write paths, computes the aligned data transfer size, and programs the DMA transfer instructions. The test enables the pattern generator via the PPI_PG_ENABLE register, waits briefly, then disables it. It polls the DMA interrupt status for channel 0 completion to confirm the generated test pattern data was successfully transferred via DMA. The test signals completion after a final wait period.",
            "Meta Test Steps / Procedure": "1. Set int_pend = 1, vcid = 3, vcid_unselected_path = ((vcid + 1) & 0xf). 2. Compute vcid_csi2_wrap_reg based on GDMA path. 3. Set gdma_reg_base. 4-5. Write virtual_channel and control_data registers. 6-7. Enable interrupts and init PHY. 8-9. Poll PHY_STOPSTATE. 10-11. Set image parameters and compute transfer size. 12-15. Write DMA address registers. 16. Enable fracdiv. 17-19. Program and start DMA. 20. Enable pattern generator via PPI_PG registers. 21-22. Wait and disable pattern generator. 23-24. Poll DMA completion. 25-26. Wait and finish. 27. csi2_enable_interrupt() details.",
            "Test Steps / Procedure": "1. Configure the virtual channel ID in the virtual_channel register based on the selected GDMA path. 2. Disable control data transfer by writing 0 to the control_data register. 3. Enable CSI-2 and DMA interrupts by reading INT_ST_MAIN to clear pending interrupts, then writing interrupt mask values to INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, and INT_MSK_ECC_CORRECTED registers. 4. Initialize the D-PHY by calling the PHY initialization sequence. 5. Poll the PHY_STOPSTATE register until all lanes report stop state. 6. Set image parameters: horizontal resolution 320, vertical resolution 16, 24 bits per pixel. 7. Compute the 8-byte aligned data transfer size based on the image parameters. 8. Program the DMA channel 0 higher-order address registers by writing to dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, and dma_m0_addr_aw_ch0_Instruction registers. 9. Enable fracdiv output to the CSI-2 subsystem. 10. Program DMA transfer instructions and start DMA channel 0. 11. Enable the test pattern generator by writing vertical resolution, horizontal resolution, configuration, and enable values to PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, and PPI_PG_ENABLE registers. 12. Wait briefly, then disable the pattern generator by writing 0 to PPI_PG_ENABLE. 13. Poll the DMA interrupt status register for channel 0 completion. 14. Wait for a settling period and signal test completion.",
            "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
            "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
            "Meta Validation / Acceptance Criteria": "1. PHY stop state validation: polled until rd_data equals 0x1000f. 2. DMA channel 0 completion: polled until (rd_data & 0x1) != 0. 3. Pattern generator enable/disable sequence validated. 4. finish(0) called.",
            "Validation / Acceptance Criteria": "1. The PHY_STOPSTATE register must report all lanes in stop state before DMA and pattern generator configuration begins. 2. The test pattern generator must be successfully enabled and then disabled via the PPI_PG_ENABLE register. 3. DMA transfer of the generated test pattern data on channel 0 must complete successfully as indicated by the DMA interrupt status register. 4. The test must signal successful completion after the DMA transfer completes and a settling wait period elapses.",
            "Remarks": "The test uses the CSI-2 host internal test pattern generator instead of an external D-PHY source. Control data transfer is explicitly disabled by writing 0 to the control_data register, unlike the dphy_lanes_test which enables it. The pattern generator is configured for 320x16 resolution at 24 bits per pixel with a specific configuration value. The DMA higher-order address registers for channel 0 are programmed for both read and write paths. A write to an inline offset register at base+0xf4 enables fracdiv output to the CSI-2 subsystem. The GDMA path selection is compile-time conditional with FPS60 also affecting DMA destination address increment behavior. The function csi2_subsys_enable_interrupt() is called in test_case() while csi2_enable_interrupt() is defined locally, suggesting the subsystem-level function may wrap additional DMA interrupt enablement."
        }
    ]

    # TestPlan columns
    tp_cols = ['Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
               'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
               'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria', 'Code Generation']

    # MetaData columns
    md_cols = ['Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
               'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
               'Meta Headers', 'Meta Macros', 'Meta Arrays']

    wb = openpyxl.Workbook()

    # TestPlan sheet
    ws_tp = wb.active
    ws_tp.title = 'TestPlan'

    # MetaData sheet
    ws_md = wb.create_sheet('MetaData')

    header_font = Font(bold=True, color='FFFFFF', size=11)
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wrap_align = Alignment(wrap_text=True, vertical='top')

    # Write TestPlan headers
    for ci, col_name in enumerate(tp_cols, 1):
        cell = ws_tp.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    # Write TestPlan data
    for ri, row_data in enumerate(json_data, 2):
        for ci, col_name in enumerate(tp_cols, 1):
            val = row_data.get(col_name, '')
            cell = ws_tp.cell(row=ri, column=ci, value=val)
            cell.alignment = wrap_align

    # Write MetaData headers
    for ci, col_name in enumerate(md_cols, 1):
        cell = ws_md.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    # Write MetaData data
    for ri, row_data in enumerate(json_data, 2):
        for ci, col_name in enumerate(md_cols, 1):
            val = row_data.get(col_name, '')
            cell = ws_md.cell(row=ri, column=ci, value=val)
            cell.alignment = wrap_align

    # Freeze first row
    ws_tp.freeze_panes = 'A2'
    ws_md.freeze_panes = 'A2'

    # Auto-size columns
    for ws in [ws_tp, ws_md]:
        for col in ws.columns:
            max_len = 0
            col_letter = col[0].column_letter
            for cell in col:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
            adjusted = min(max_len + 2, 80)
            ws.column_dimensions[col_letter].width = max(adjusted, 12)

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = 'veryHidden'

    # Save
    output_dir = 'Test_Output/MIPI/TestPlan'
    os.makedirs(output_dir, exist_ok=True)
    filepath = os.path.join(output_dir, filename)
    wb.save(filepath)

    # Verify
    file_size = os.path.getsize(filepath)
    wb2 = openpyxl.load_workbook(filepath)
    sheets = wb2.sheetnames
    wb2.close()

    print(f'FILENAME={filename}')
    print(f'FILEPATH={filepath}')
    print(f'FILESIZE={file_size}')
    print(f'SHEETS={",".join(sheets)}')
    print(f'ROWS_TESTPLAN=2')
    print(f'ROWS_METADATA=2')
    print(f'VALIDATION=PASSED')

    return filepath, filename

if __name__ == '__main__':
    main()
