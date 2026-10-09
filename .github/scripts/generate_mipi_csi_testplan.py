#!/usr/bin/env python3
import os
import json
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp}.xlsx'
output_dir = 'Test_Output/MIPI/TestPlan'
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

json_data = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_dphy_lanes_test",
    "Feature": "D-PHY Lane Configuration",
    "Meta Headers": '#include <stdio.h>; #include <stdlib.h>; #include "test_common.h"; #include "mipi_csi2.h"',
    "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 3\n#define HRES 64\n#define DATA_TYPE CSI2_RGB888",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "0xE6000000",
    "Memory End Offset": "0xE6002000",
    "Meta Test Description": "This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through all lane counts from 4 lanes down to 1 lane. For each lane configuration, the test performs a complete CSI-2 receive sequence: it first enables all CSI-2 host interrupts by reading INT_ST_MAIN to clear pending interrupts and then writing mask registers (INT_MSK_PHY_FATAL with 0x0000000f, INT_MSK_PKT_FATAL with 0x00000003, INT_MSK_PHY with 0x000f000f, INT_MSK_LINE with 0x000f000f, INT_MSK_BNDRY_FRAME_FATAL with 0x0000ffff, INT_MSK_SEQ_FRAME_FATAL with 0x0000ffff, INT_MSK_CRC_FRAME_FATAL with 0x0000ffff, INT_MSK_PLD_CRC_FATAL with 0x0000ffff, INT_MSK_DATA_ID with 0x0000ffff, INT_MSK_ECC_CORRECTED with 0x0000ffff). It then configures the virtual channel register with VC_ID (3) shifted according to the GDMA path, enables control data transfer by writing 1 to the control_data register, performs SNPS D-PHY initialization via snps_phy_init(), and polls PHY_STOPSTATE until it reads 0x1000f. For each lane count (3 down to 0), it writes the lane_num to N_LANES, signals the lane count to address 0xa0243ffc, and then loops through cntrl_pkt_cnt = ((VRES*3)+2) packets. In each packet iteration, it enables DMA interrupts by writing 0x3 to gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET, programs a control-data DMA transfer of 8 bytes from source 0x8000 to destination 0xE6001000 via dma_trnsfr_instn_preload() on channel 0, starts the DMA via DMAGO_CSI(), polls gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET until bit 0 is set, clears the DMA interrupt by writing 0x1 to gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET, reads gdma_reg_base+0x28 for debug status, then reads the control data from 0xE6001000. If the data type field (bits[5:0]) is greater than 0xf (indicating a long packet), it extracts word_count from bits[21:6], computes csi_data_size aligned to 8 bytes, programs a data DMA transfer of csi_data_size bytes from source 0x0000 to destination 0xE6002000 via dma_trnsfr_instn_preload() on channel 1, starts the DMA via DMAGO_CSI(), polls gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET until bit 1 is set, reads the INTMIS register once more, clears the DMA interrupt by writing 0x2 to gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET. After all lane iterations complete, finish(0) is called.",
    "Test Description": "Verify MIPI CSI-2 D-PHY lane configuration by iterating from 4 lanes down to 1 lane. For each lane count, enable CSI-2 host interrupts, configure virtual channel, initialize D-PHY, wait for PHY stop state, then perform DMA-based control and data packet transfers for all expected packets per frame. Validate DMA completion via interrupt polling for both control packet (channel 0) and data packet (channel 1) transfers at each lane configuration.",
    "Meta Test Steps / Procedure": "1. Enter test_case() function.\n2. Declare local variables: rx_desc, tx_desc (long long int), gdma_tx_trnsfr_size, gdma_trnsfr_size (long long int), cntrl_pkt_cnt (int).\n3. printf(\"start line\\n\").\n4. Call csi2_enable_interrupt().\n5. [Inside csi2_enable_interrupt()]: Declare local int rd_data.\n6. [Inside csi2_enable_interrupt()]: rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) \u2014 read INT_ST_MAIN to clear pending interrupts.\n7. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f) \u2014 enable phy_fatal interrupts.\n8. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003) \u2014 enable pkt_fatal interrupts.\n9. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f) \u2014 enable phy interrupts.\n10. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f) \u2014 enable line interrupts.\n11. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff) \u2014 enable boundary frame fatal interrupts.\n12. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff) \u2014 enable seq frame fatal interrupts.\n13. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff) \u2014 enable CRC frame fatal interrupts.\n14. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff) \u2014 enable payload CRC fatal interrupts.\n15. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff) \u2014 enable data ID interrupts.\n16. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff) \u2014 enable ECC corrected interrupts.\n17. [Return from csi2_enable_interrupt()].\n18. Conditional compilation: set vcid_csi2_wrap_reg based on GDMA path.\n19. gdma_reg_base = 0xE6A00000.\n20. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\n21. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1).\n22-24. Repeat VC and control data writes.\n25. Call snps_phy_init().\n26-29. Poll PHY_STOPSTATE until 0x1000f.\n30-31. Set ch0_pc and ch1_pc.\n32-46. Outer loop (lane_num 3 to 0): write N_LANES, signal lane count, inner loop for packets: enable DMA IRQ, program DMA, start DMA, poll INTMIS, clear IRQ, read control data, if long packet: program data DMA, start, poll, clear.\n47. Call finish(0).",
    "Test Steps / Procedure": "1. Enable all CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing enable masks to PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt mask registers.\n2. Configure the CSI-2 virtual channel register with the appropriate VC ID based on the selected GDMA path.\n3. Enable control data transfer by writing to the control data register.\n4. Repeat the virtual channel and control data register writes a second time.\n5. Initialize the SNPS D-PHY.\n6. Poll the PHY stop state register until all lanes and the clock lane report stop state (expected value 0x1000f).\n7. For each lane configuration from 4 lanes down to 1 lane (lane_num = 3, 2, 1, 0):\n   a. Write the lane count to the N_LANES register.\n   b. Signal the lane count to the external trigger address to start the CSI-2 sequence.\n   c. For each expected packet in the frame (total = (VRES*3)+2 packets):\n      i. Enable DMA interrupts for both channels.\n      ii. Program a control-data DMA transfer of 8 bytes on channel 0.\n      iii. Start the DMA transfer on channel 0.\n      iv. Poll the DMA interrupt status register until channel 0 completion (bit 0 set).\n      v. Clear the channel 0 DMA interrupt.\n      vi. Read the received CSI-2 control data.\n      vii. If the control data indicates a long packet (data type > 0xf), extract the word count, compute the 8-byte aligned transfer size, program a data DMA transfer on channel 1, start the DMA, poll for channel 1 completion (bit 1 set), read interrupt status, and clear the channel 1 DMA interrupt.\n8. Signal test completion.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE polling: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) is polled in a while loop until rd_data == 0x1000f.\n2. DMA channel 0 completion polling: read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled with condition (rd_data & 0x1) == 0x0.\n3. Control data type check: (csi_ctrl_data & 0x3f) > 0xf determines long packet.\n4. Word count extraction: word_count = ((csi_ctrl_data >> 6) & 0xffff).\n5. Data size alignment: csi_data_size = (word_count % 8) ? (word_count / 8 + 1) * 8 : word_count.\n6. DMA channel 1 completion polling: (rd_data & 0x2) == 0x0.\n7. DMA interrupt clear channel 0: write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1).\n8. DMA interrupt clear channel 1: write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2).\n9. Test completion: finish(0).",
    "Validation / Acceptance Criteria": "1. The PHY stop state register must read back the expected value indicating all configured data lanes and the clock lane have entered stop state.\n2. DMA channel 0 interrupt status must indicate completion (bit 0 set) for each control packet transfer before proceeding.\n3. The received CSI-2 control data must be readable and its data type field must be correctly parsed to distinguish short packets from long packets.\n4. For long packets, DMA channel 1 interrupt status must indicate completion (bit 1 set) for each data packet transfer.\n5. DMA interrupts must be successfully cleared after each transfer completion.\n6. The test must complete successfully across all four lane configurations (4, 3, 2, and 1 lane) by calling finish with a pass status of 0.",
    "Remarks": "External functions snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() are called but their implementations are not located inside the testcase folder. The virtual channel register and control data register are each written twice in sequence. The GDMA path selection (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) is compile-time conditional. Default VRES=3 yields cntrl_pkt_cnt=11 packets per lane iteration. Two hex addresses (0xa0243ffc and 0xE6001000) could not be mapped to named registers in the provided specification documents."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_test_pattern_generator",
    "Feature": "Internal Test Pattern Generator",
    "Meta Headers": '#include <stdio.h>; #include <stdlib.h>; #include "test_common.h"; #include "mipi_csi2.h"',
    "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "0xE6000000",
    "Memory End Offset": "0xE6001000",
    "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator (PG) functionality. The test configures the CSI-2 subsystem virtual channel register based on the compile-time selected GDMA path, disables control data transfer by writing 0 to the control_data register, enables CSI-2 and DMA interrupts, initializes the SNPS D-PHY, and polls PHY_STOPSTATE until it reads 0x1000f. It then computes csi2_data_trnsfr_size for a 320x16 image with 24 bits per pixel (RGB888), yielding 15360 bytes. It programs the higher-order AXI DMA address registers, enables the FracDiv clock output, programs a DMA data transfer, starts the DMA, enables the pattern generator, waits, disables the pattern generator, polls for DMA completion, and calls finish(0).",
    "Test Description": "Verify the MIPI CSI-2 internal test pattern generator by configuring a 320x16 RGB888 pattern, enabling the pattern generator, receiving the generated data via DMA, and validating DMA transfer completion. The test configures virtual channel, enables CSI-2 host interrupts, initializes D-PHY, waits for PHY stop state, programs higher-order AXI address registers, enables FracDiv clock, programs and starts a DMA data transfer, enables then disables the pattern generator, and polls for DMA completion.",
    "Meta Test Steps / Procedure": "1. Enter test_case() function.\n2. Declare local variables.\n3-9. Configure GDMA path and virtual channel.\n10. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\n11. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0).\n12-25. Enable interrupts via csi2_enable_interrupt().\n26-30. Poll PHY_STOPSTATE until 0x1000f.\n31-34. Compute csi2_data_trnsfr_size = 15360.\n35-38. Write DMA higher-order AXI address registers.\n39. Write MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 with 0x1.\n40-44. Program DMA transfer, start DMA channel 0.\n45-50. Call csi2_ctrlr_pg_enable() to configure and enable PG.\n51. wait_on(100).\n52. write_reg(PPI_PG_ENABLE, 0).\n53-56. Poll DMA INTMIS until bit 0 set.\n57. wait_on(10000).\n58. finish(0).",
    "Test Steps / Procedure": "1. Set the interrupt pending flag and configure the virtual channel ID (VC=3) with the appropriate GDMA path selection.\n2. Compute the virtual channel register value based on the selected GDMA path and calculate the GDMA register base address.\n3. Write the virtual channel register with the computed VC configuration.\n4. Disable control data transfer by writing 0 to the control data register.\n5. Enable all CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing enable masks to PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt mask registers.\n6. Initialize the SNPS D-PHY.\n7. Poll the PHY stop state register until all data lanes and the clock lane report stop state.\n8. Set image parameters: horizontal resolution = 320, vertical resolution = 16, bits per pixel = 24 (RGB888).\n9. Compute the total DMA data transfer size with 8-byte alignment (15360 bytes for 320x16 RGB888).\n10. Program the higher-order AXI DMA address registers for read and write channels.\n11. Enable the FracDiv clock output to the CSI-2 subsystem by writing to the clock gating register.\n12. Program the DMA data transfer with source address, destination address, and computed transfer size on channel 0.\n13. Start the DMA transfer on channel 0.\n14. Enable the internal pattern generator by configuring vertical resolution (16), horizontal resolution (320 with 7 bytes per line encoding), PG configuration (RGB888, VC=0, mode=1), and enabling PG.\n15. Wait 100 cycles for pattern generation.\n16. Disable the pattern generator.\n17. Poll the DMA interrupt status register until channel 0 completion is indicated (bit 0 set).\n18. Wait 10000 cycles for completion settling.\n19. Signal test completion with pass status.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; enableclkgating_csiphy; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE polling: polled until rd_data == 0x1000f.\n2. DMA channel 0 completion polling: polled until bit 0 set.\n3. csi2_data_trnsfr_size = 15360 bytes for 320x16 RGB888.\n4. Pattern generator configuration: PPI_PG_PATTERN_VRES=0x10, PPI_PG_PATTERN_HRES=0x70140, PPI_PG_CONFIG=0xe401, PPI_PG_ENABLE=1 then 0.\n5. DMA higher-order addresses: ar_ch0_data=0x100, ar_ch0_Instruction=0x0, aw_ch0_data=0x0, aw_ch0_Instruction=0x0.\n6. FracDiv clock enable: write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1).\n7. DMA transfer: src=0x00, dest=0xE6001000, size=15360, channel=0.\n8. Test completion: finish(0).",
    "Validation / Acceptance Criteria": "1. The PHY stop state register must read back the expected value indicating all data lanes and the clock lane have entered stop state before proceeding with DMA and pattern generator configuration.\n2. The DMA interrupt status register must indicate channel 0 completion (bit 0 set) after the pattern generator has been enabled and then disabled, confirming that the generated test pattern data was successfully transferred via DMA.\n3. The pattern generator must be correctly configured for a 320x16 RGB888 image before being enabled, and must be disabled after a wait period.\n4. The total DMA transfer size must be correctly computed as 15360 bytes for the configured image parameters.\n5. The test must complete successfully by calling finish with a pass status of 0.",
    "Remarks": "External functions snps_phy_init(), csi2_subsys_enable_interrupt(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), and finish() are called but their implementations are not located inside the testcase folder. The csi2_enable_interrupt() function is defined locally in the same file. The GDMA path selection (GDMA0_PATH through GDMA3_PATH) and FPS60 flag are compile-time conditional. The control_data register is written with 0 (disabled) in this testcase, unlike the dphy_lanes_test which writes 1 (enabled). The PPI_PG_ENABLE register is written twice: first with 1 inside csi2_ctrlr_pg_enable() to enable the pattern generator, then with 0 in test_case() to disable it after wait_on(100)."
  }
]

# TestPlan columns
tp_cols = ['Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
           'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
           'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
           'Code Generation']

# MetaData columns
md_cols = ['Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
           'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
           'Meta Headers', 'Meta Macros', 'Meta Arrays']

wb = Workbook()
ws_tp = wb.active
ws_tp.title = 'TestPlan'
ws_md = wb.create_sheet('MetaData')

header_font = Font(bold=True, color='FFFFFF')
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

# Auto-size columns
for ws in [ws_tp, ws_md]:
    for col_cells in ws.columns:
        max_len = 0
        col_letter = get_column_letter(col_cells[0].column)
        for cell in col_cells:
            if cell.value:
                lines = str(cell.value).split('\n')
                max_line = max(len(l) for l in lines)
                if max_line > max_len:
                    max_len = max_line
        adjusted = min(max_len + 2, 60)
        ws.column_dimensions[col_letter].width = max(adjusted, 12)

# Freeze first row
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

wb.save(output_path)
print(f'Generated: {output_path}')
print(f'Filename: {filename}')
