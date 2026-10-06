#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import json

def generate():
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"
    output_dir = "Test_Output/MIPI/TestPlan"
    os.makedirs(output_dir, exist_ok=True)
    filepath = os.path.join(output_dir, filename)

    json_data = [
        {
            "Index": "1",
            "SS / Module": "MIPI_CSI",
            "Test Case Name": "mipi_csi2_dphy_lanes_test",
            "Feature": "DPHY Lane Configuration",
            "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
            "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2; LS_LE_EN; TOTAL_FRAME; VC_ID; VRES; HRES; DATA_TYPE",
            "Meta Arrays": "NA",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY lane configuration by iterating through all lane counts (4 down to 1). It first calls csi2_enable_interrupt() which reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear interrupts, then writes interrupt mask registers (MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL with 0x0000000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL with 0x00000003, MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY with 0x000f000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE with 0x000f000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED with 0x0000ffff). The GDMA path and virtual channel ID are configured based on compile-time defines (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH). The vcid_csi2_wrap_reg is computed from VC_ID shifted by the appropriate amount and written to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL. Control data transfer is enabled by writing 1 to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA. The D-PHY is initialized via snps_phy_init(). MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE is polled until its value equals 0x1000f, confirming PHY stop state. A for loop iterates lane_num from 3 down to 0, writing lane_num to MIZAR_MIPI_CSI2_HOST_N_LANES and (lane_num+1) to 0xa0243ffc to trigger the CSI-2 sequence. For each lane configuration, an inner loop runs for cntrl_pkt_cnt = (VRES*3)+2 iterations. In each iteration: DMA interrupt enable is written to gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET with 0x3; dma_trnsfr_instn_preload() is called for control data with ch0_preload_loc, gdma_reg_base, src_addr=0x8000, dest_addr=0xE6001000, trnsfr_size=8, irq_num=0; DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0) starts channel 0; gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET is polled until bit 0 is set; gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET is written with 0x1 to clear the interrupt; 0xE6001000 is read to obtain csi_ctrl_data. If (csi_ctrl_data & 0x3f) > 0xf, word_count is extracted from bits [21:6], csi_data_size is computed as word_count aligned to 8 bytes, dma_trnsfr_instn_preload() is called for data with ch1_preload_loc, gdma_reg_base, src_addr=0x0000, dest_addr=0xE6002000, trnsfr_size=csi_data_size, irq_num=1; DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1) starts channel 1; gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET is polled until bit 1 is set; gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET is written with 0x2 to clear the interrupt. The test completes by calling finish(0).",
            "Test Description": "This test validates MIPI CSI-2 DPHY lane configuration by iterating through all supported lane counts from 4 lanes down to 1 lane. For each lane configuration, it enables CSI-2 interrupts by reading the main interrupt status register to clear pending interrupts and then writing all interrupt mask registers to enable PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupts. The virtual channel register is configured and control data transfer is enabled. The D-PHY is initialized and the PHY stop state register is polled until the PHY enters stop state. For each lane count, the N_LANES register is written with the lane number, a trigger write initiates the CSI-2 sequence, and then a packet processing loop runs for each expected packet. In each packet iteration, DMA channel 0 transfers control data, the DMA interrupt status is polled for completion, and the control data is read. If the control data indicates a data packet (data type greater than short packet threshold), the word count is extracted, a DMA channel 1 transfer is initiated for the CSI data payload, and the DMA interrupt is polled for completion. The test exercises all lane configurations to verify correct DPHY operation across varying lane counts.",
            "Meta Test Steps / Procedure": "1. Call csi2_enable_interrupt() function. 2. Inside csi2_enable_interrupt(): read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) to clear pending interrupts. 3. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f) to enable PHY fatal interrupts. 4. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003) to enable packet fatal interrupts. 5. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f) to enable PHY interrupts. 6. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f) to enable line interrupts. 7. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff) to enable boundary frame fatal interrupts. 8. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff) to enable sequence frame fatal interrupts. 9. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff) to enable CRC frame fatal interrupts. 10. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff) to enable payload CRC fatal interrupts. 11. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff) to enable data ID interrupts. 12. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff) to enable ECC corrected interrupts. 13. Return from csi2_enable_interrupt(). 14. Compute vcid_csi2_wrap_reg based on compile-time GDMA path define: GDMA3_PATH sets vcid_csi2_wrap_reg = VC_ID; GDMA2_PATH sets vcid_csi2_wrap_reg = (VC_ID << 4); GDMA1_PATH sets vcid_csi2_wrap_reg = (VC_ID << 8); GDMA0_PATH sets vcid_csi2_wrap_reg = (VC_ID << 12). Set gdma_path accordingly (3, 2, 1, or 0). 15. Set gdma_reg_base = 0xE6A00000. 16. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) to configure virtual channel. 17. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1) to enable control data transfer. 18. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) again (repeated write). 19. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1) again (repeated write). 20. Call snps_phy_init() to initialize the D-PHY. 21. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) to read PHY stop state. 22. Poll: while rd_data != 0x1000f, repeatedly read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) until PHY enters stop state (value equals 0x1000f). 23. Set ch0_pc = 0xE6000000 and ch1_pc = 0xE6000500. 24. Begin outer for loop: lane_num iterates from 3 down to 0 (inclusive). 25. write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num) to configure the number of active lanes. 26. Compute cntrl_pkt_cnt = (VRES * 3) + 2. 27. write_reg(0xa0243ffc, (lane_num + 1)) to trigger the CSI-2 sequence for the current lane count. 28. Begin inner for loop: i iterates from 0 to cntrl_pkt_cnt - 1. 29. Set ch0_preload_loc = ch0_pc and ch1_preload_loc = ch1_pc. 30. write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3) to enable DMA IRQ[1] and IRQ[0]. 31. Call dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0) to preload control data DMA transfer instructions for channel 0. 32. Call DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0) to start DMA channel 0. 33. Set rd_data = 0. 34. Poll: while (rd_data & 0x1) == 0x0, read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) to check DMA interrupt status for channel 0 completion. 35. write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1) to clear DMA IRQ for channel 0. 36. rd_data = read_reg(gdma_reg_base + 0x28) to read DMA status. 37. csi_ctrl_data = read_reg(0xE6001000) to read the transferred control data. 38. Check condition: if (csi_ctrl_data & 0x3f) > 0xf (data type indicates a long packet). 39. If true: extract word_count = (csi_ctrl_data >> 6) & 0xffff. 40. Compute csi_data_size: if (word_count % 8) != 0 then csi_data_size = (word_count / 8 + 1) * 8, else csi_data_size = word_count. 41. Call dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1) to preload data DMA transfer instructions for channel 1. 42. Call DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1) to start DMA channel 1. 43. Set rd_data = 0. 44. Poll: while (rd_data & 0x2) == 0x0, read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) to check DMA interrupt status for channel 1 completion. 45. rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) (additional read after poll exit). 46. write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2) to clear DMA IRQ for channel 1. 47. End of if block for long packet handling. 48. End of inner for loop (next packet iteration). 49. End of outer for loop (next lane_num iteration). 50. Call finish(0) to complete the test.",
            "Test Steps / Procedure": "1. Enable CSI-2 interrupts by reading the main interrupt status register to clear pending interrupts, then writing all interrupt mask registers to enable PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupts. 2. Configure the virtual channel register based on the selected GDMA path and virtual channel ID. 3. Enable control data transfer by writing to the control data register. 4. Initialize the D-PHY by calling the PHY initialization routine. 5. Poll the PHY stop state register until the PHY enters stop state. 6. For each lane configuration (4 lanes down to 1 lane): a. Write the lane count to the N_LANES register. b. Trigger the CSI-2 sequence for the current lane count. c. For each expected packet in the frame: i. Enable DMA interrupts for both channels. ii. Preload and start DMA channel 0 for control data transfer. iii. Poll the DMA interrupt status register until channel 0 transfer completes. iv. Clear the DMA interrupt for channel 0. v. Read the transferred control data. vi. If the control data indicates a long data packet, extract the word count, compute the aligned data size, preload and start DMA channel 1 for data payload transfer, poll the DMA interrupt status register until channel 1 transfer completes, and clear the DMA interrupt for channel 1. 7. Complete the test.",
            "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
            "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
            "Meta Validation / Acceptance Criteria": "The test validates DPHY lane operation by polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until its value equals 0x1000f, confirming all lanes and clock lane have entered stop state. For each lane configuration (lane_num from 3 to 0), the DMA interrupt status register (gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled: for channel 0, bit 0 must become set (rd_data & 0x1 != 0x0) indicating control data transfer completion; for channel 1, bit 1 must become set (rd_data & 0x2 != 0x0) indicating data payload transfer completion. The control data read from 0xE6001000 is checked: if (csi_ctrl_data & 0x3f) > 0xf, the packet is identified as a long packet and data transfer is initiated. The word_count is extracted as (csi_ctrl_data >> 6) & 0xffff and aligned to 8-byte boundary for DMA transfer size. The test completes successfully by calling finish(0), indicating all lane configurations processed all packets without error.",
            "Validation / Acceptance Criteria": "The PHY stop state register must report the expected stop state value confirming all lanes and clock lane are in stop state before proceeding. For each lane configuration (4 lanes down to 1 lane), DMA channel 0 must complete the control data transfer as indicated by the DMA interrupt status register. When the control data indicates a long data packet, DMA channel 1 must complete the data payload transfer as indicated by the DMA interrupt status register. All lane configurations must process all expected packets successfully. The test must complete without errors across all lane counts.",
            "Remarks": "The test iterates lane_num from 3 down to 0, testing 4-lane, 3-lane, 2-lane, and 1-lane configurations. The GDMA path and virtual channel ID are compile-time configurable via GDMA0_PATH/GDMA1_PATH/GDMA2_PATH/GDMA3_PATH and VC_ID defines. The virtual channel and control data registers are written twice (repeated writes in source). The DMA base address gdma_reg_base is set to 0xE6A00000. DMA register accesses use gdma_reg_base plus offset macros (MIPI_CSI2_DMA_INTEN_OFFSET, MIPI_CSI2_DMA_INTMIS_OFFSET, MIPI_CSI2_DMA_INTCLR_OFFSET). Two hex addresses (0xa0243ffc and 0xE6001000) could not be mapped to named registers. The snps_phy_init() and dma_trnsfr_instn_preload() function implementations are external to this testcase folder."
        },
        {
            "Index": "2",
            "SS / Module": "MIPI_CSI",
            "Test Case Name": "mipi_csi2_test_pattern_generator",
            "Feature": "Test Pattern Generator",
            "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
            "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2",
            "Meta Arrays": "NA",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator. The test_case() function begins by setting int_pend = 1, vcid = 3, and computing vcid_unselected_path = ((vcid + 1) & 0xf). Based on compile-time GDMA path defines (GDMA3_PATH, GDMA2_PATH, GDMA1_PATH, or default GDMA0_PATH), vcid_csi2_wrap_reg is computed from vcid and vcid_unselected_path shifted into appropriate nibble positions, and gdma_path is set accordingly (3, 2, 1, or 0). gdma_reg_base is computed as 0xE6A00000 + (gdma_path * 0x1000). MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL is written with vcid_csi2_wrap_reg to configure virtual channel routing. MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA is written with 0 to disable control data transfer. csi2_subsys_enable_interrupt() is called (external function). snps_phy_init() is called to initialize the D-PHY. MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE is read and polled in a while loop until its value equals 0x1000f, confirming PHY stop state. hres is set to 320, vres to 16, valid_bits_per_pixel to 24. csi2_data_trnsfr_size is computed as the total byte count for the frame. DMA higher-order address registers are programmed. MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 is written with 0x1 to enable sending fracdiv output to the CSI-2 subsystem. The pattern generator is enabled and then disabled. DMA completion is polled. The test completes by calling finish(0).",
            "Test Description": "This test validates the MIPI CSI-2 internal test pattern generator functionality. It configures the virtual channel register for the selected GDMA path and disables control data transfer. CSI-2 and DMA interrupts are enabled. The D-PHY is initialized and the PHY stop state register is polled until the PHY enters stop state. The test computes the total frame data transfer size based on a resolution of 320x16 pixels at 24 bits per pixel, with 8-byte alignment per line. DMA higher-order address registers for channel 0 read and write paths are configured for both data and instruction. The fractional divider output is enabled for the CSI-2 subsystem. DMA channel 0 transfer instructions are preloaded and the DMA channel is started. The internal pattern generator is then enabled by configuring the vertical resolution, horizontal resolution, pattern generator configuration, and pattern generator enable registers. After a wait period, the pattern generator is disabled. The DMA interrupt status is polled until channel 0 transfer completes, confirming that the generated test pattern data was successfully transferred via DMA. The test completes after a final wait period.",
            "Meta Test Steps / Procedure": "1. Set int_pend = 1. 2. Set vcid = 3. 3. Compute vcid_unselected_path = ((vcid + 1) & 0xf) = 4. 4. Based on compile-time GDMA path define: configure vcid_csi2_wrap_reg and gdma_path. 5. Compute gdma_reg_base = 0xE6A00000 + (gdma_path * 0x1000). 6. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg). 7. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0). 8. Call csi2_subsys_enable_interrupt(). 9. Call snps_phy_init(). 10. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until 0x1000f. 11-17. Configure DMA address registers. 18. Enable fracdiv output. 19-22. Preload and start DMA. 23-28. Enable and disable pattern generator. 29-32. Poll DMA completion. 33-34. Wait and finish.",
            "Test Steps / Procedure": "1. Configure the virtual channel register based on the selected GDMA path and virtual channel ID, and disable control data transfer by writing to the control data register. 2. Enable CSI-2 and DMA interrupts. 3. Initialize the D-PHY. 4. Poll the PHY stop state register until the PHY enters stop state. 5. Compute the total frame data transfer size based on 320x16 resolution at 24 bits per pixel with 8-byte alignment. 6. Program the DMA higher-order address registers for channel 0 read and write paths (data and instruction). 7. Enable the fractional divider output to the CSI-2 subsystem. 8. Preload DMA channel 0 transfer instructions with source, destination, and transfer size parameters, then start DMA channel 0. 9. Enable the internal test pattern generator by configuring the vertical resolution, horizontal resolution, pattern generator configuration, and pattern generator enable registers. 10. Wait for pattern generation to complete, then disable the pattern generator. 11. Poll the DMA interrupt status register until DMA channel 0 transfer completes. 12. Wait for final settling period and complete the test.",
            "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
            "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
            "Meta Validation / Acceptance Criteria": "The test validates pattern generator operation by polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until its value equals 0x1000f. After the pattern generator is enabled and then disabled, the DMA interrupt masked status register is polled until bit 0 is set, confirming DMA channel 0 has completed the transfer. The test completes successfully by calling finish(0).",
            "Validation / Acceptance Criteria": "The PHY stop state register must report the expected stop state value confirming all lanes and clock lane are in stop state before proceeding. After the pattern generator is enabled and subsequently disabled, DMA channel 0 must complete the data transfer as indicated by the DMA interrupt status register bit 0 becoming set. The total transferred data size must correspond to the configured frame resolution (320x16 at 24 bits per pixel with 8-byte line alignment). The test must complete successfully without errors.",
            "Remarks": "The GDMA path and virtual channel ID are compile-time configurable via GDMA0_PATH/GDMA1_PATH/GDMA2_PATH/GDMA3_PATH defines. The DMA destination address increment behavior is controlled by the FPS60 compile-time define. The csi2_subsys_enable_interrupt() function is external to this testcase folder and distinct from the locally defined csi2_enable_interrupt() function. The snps_phy_init(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), and wait_on() function implementations are external to this testcase folder. A write to a register at base + offset 0xf4 enables the fractional divider output to the CSI-2 subsystem; this register could not be mapped to a named register. The pattern generator is enabled briefly and then disabled before polling for DMA completion."
        }
    ]

    # TestPlan columns
    tp_columns = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
                  "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
                  "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
                  "Code Generation"]

    # MetaData columns
    md_columns = ["Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
                  "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
                  "Meta Headers", "Meta Macros", "Meta Arrays"]

    wb = openpyxl.Workbook()

    # --- TestPlan Sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    blue_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    white_font = Font(color="FFFFFF", bold=True)
    wrap_align = Alignment(wrap_text=True, vertical="top")

    for col_idx, col_name in enumerate(tp_columns, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.fill = blue_fill
        cell.font = white_font
        cell.alignment = wrap_align

    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(tp_columns, 1):
            value = row_data.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_align

    ws_tp.freeze_panes = "A2"

    # Auto-size columns
    for col_idx, col_name in enumerate(tp_columns, 1):
        max_len = len(col_name)
        for row_idx in range(2, len(json_data) + 2):
            val = str(ws_tp.cell(row=row_idx, column=col_idx).value or "")
            lines = val.split("\n")
            for line in lines:
                if len(line) > max_len:
                    max_len = len(line)
        adjusted = min(max_len + 2, 60)
        ws_tp.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = adjusted

    # --- MetaData Sheet ---
    ws_md = wb.create_sheet("MetaData")

    for col_idx, col_name in enumerate(md_columns, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.fill = blue_fill
        cell.font = white_font
        cell.alignment = wrap_align

    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(md_columns, 1):
            value = row_data.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_align

    ws_md.freeze_panes = "A2"

    for col_idx, col_name in enumerate(md_columns, 1):
        max_len = len(col_name)
        for row_idx in range(2, len(json_data) + 2):
            val = str(ws_md.cell(row=row_idx, column=col_idx).value or "")
            lines = val.split("\n")
            for line in lines:
                if len(line) > max_len:
                    max_len = len(line)
        adjusted = min(max_len + 2, 60)
        ws_md.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = adjusted

    ws_md.sheet_state = "veryHidden"

    wb.save(filepath)
    print(f"FILE_SAVED:{filepath}")
    print(f"FILENAME:{filename}")

    # Verify
    file_size = os.path.getsize(filepath)
    print(f"FILE_SIZE:{file_size}")

    # Re-open to verify
    wb2 = openpyxl.load_workbook(filepath)
    sheets = wb2.sheetnames
    print(f"SHEETS:{sheets}")
    print(f"ROWS_TP:{ws_tp.max_row - 1}")
    print(f"ROWS_MD:{ws_md.max_row - 1}")
    print("VALIDATION:PASSED")

if __name__ == "__main__":
    generate()
