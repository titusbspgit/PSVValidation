#!/usr/bin/env python3
"""
MIPI_CSI TestPlan XLSX Generator - Agent 7 Output
Generates the workbook with TestPlan and MetaData (veryHidden) sheets.
Run: python3 generate_MIPI_CSI_TestPlan_v2.py
Outputs: base64-encoded xlsx to stdout AND saves .xlsx file locally.
"""
import base64
import io
import json
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
except ImportError:
    print("ERROR: pip install openpyxl", file=sys.stderr)
    sys.exit(1)

IP_NAME = "MIPI_CSI"
IST = timezone(timedelta(hours=5, minutes=30))
timestamp = datetime.now(IST).strftime("%Y%m%d_%H%M%S")
FILENAME = f"{IP_NAME}_TestPlan_{timestamp}.xlsx"

# ============================================================
# TESTPLAN_JSON (from Agent 6B aggregated output)
# ============================================================
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
    "Validation / Acceptance Criteria": "1. PHY stop state register must report all data lanes and clock lane in stop state before proceeding with lane configuration.\n2. DMA interrupt for control packet transfer (IRQ channel 0) must assert for each packet iteration, confirming successful control data DMA completion.\n3. DMA interrupt for data packet transfer (IRQ channel 1) must assert for each packet iteration, confirming successful CSI pixel data DMA completion.\n4. DMA interrupts must be successfully cleared after each transfer completion.\n5. Data comparison between written and read-back values must pass with zero default failure count and zero write failure count.\n6. The test must successfully complete DMA transfers for all lane configurations from 4 lanes down to 1 lane.\n7. The test must complete successfully with a pass indication.",
    "Remarks": "The test uses compile-time configuration for vertical resolution, horizontal resolution, virtual channel ID, data type (RGB888), and frame count. The GDMA path is selected via preprocessor conditionals. The control packet count is derived from the vertical resolution. No timeout mechanism is implemented for PHY stop state or DMA interrupt polling loops."
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
    "Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator functionality. The test configures virtual channel IDs and GDMA path selection, writes the CSI-2 wrapper virtual channel register, and disables control data transfer. It computes the CSI-2 data transfer size based on a horizontal resolution of 320, vertical resolution of 16, and 24 valid bits per pixel with 8-byte alignment, resulting in a total transfer size of 15360 bytes. The test programs DMA transfer instructions with conditional destination address increment, starts DMA channel 0, enables the pattern generator by configuring the vertical resolution, horizontal resolution, pattern generator configuration, and pattern generator enable registers, waits for data generation, disables the pattern generator, polls the DMA interrupt status for transfer completion, and finishes the test.",
    "Test Steps / Procedure": "1. Enable CSI-2 interrupts.\n2. Set the virtual channel ID and compute the unselected path virtual channel value.\n3. Select the GDMA path and configure the virtual channel ID mapping based on the selected path.\n4. Write the virtual channel register with the computed virtual channel mapping.\n5. Disable control data transfer by writing to the control data register.\n6. Set horizontal resolution to 320, vertical resolution to 16, and valid bits per pixel to 24.\n7. Compute the CSI-2 data transfer size with 8-byte alignment.\n8. Configure the DMA destination address increment flag based on frame rate configuration.\n9. Program DMA transfer instructions for channel 0 with the computed transfer size.\n10. Start DMA channel 0.\n11. Enable the pattern generator by writing the vertical resolution register, horizontal resolution register, pattern generator configuration register, and pattern generator enable register.\n12. Wait for the pattern generator to produce data.\n13. Disable the pattern generator by clearing the pattern generator enable register.\n14. Poll the DMA interrupt status register until the data transfer completes (IRQ channel 0 asserts).\n15. Wait for a post-completion cooldown period.\n16. End the test.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1. DMA interrupt for data transfer completion (IRQ channel 0) must assert, confirming successful DMA channel 0 transfer of pattern generator data.\n2. The pattern generator must be successfully enabled by writing the correct values to the vertical resolution, horizontal resolution, configuration, and enable registers.\n3. The pattern generator must be successfully disabled after the data generation wait period.\n4. The CSI-2 data transfer size must be correctly computed as 15360 bytes based on the configured resolution and bits per pixel with 8-byte alignment.\n5. The virtual channel register must be correctly configured based on the selected GDMA path.\n6. Control data transfer must be disabled for the pattern generator test flow.\n7. DMA channel 0 must be correctly programmed and started before the pattern generator is enabled.\n8. The test must complete successfully with a pass indication.",
    "Remarks": "The test uses a fixed resolution of 320x16 with 24 bits per pixel (RGB888 format) for pattern generation. The GDMA path is selected via compile-time preprocessor conditionals. The DMA destination address increment behavior is conditional on the FPS60 compile-time configuration. No timeout mechanism is implemented for the DMA interrupt polling loop. All macros in this testcase resolved to non-register addresses and are therefore not mapped to any IP registers."
  }
]

# ============================================================
# META_JSON (from Agent 6A aggregated output)
# ============================================================
META_JSON = [
  {
    "Test Case Name": "mipi_csi2_dphy_lanes_test",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"",
    "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#ifdef DPHY_LANES_TEST\n#ifndef LS_LE_EN\n#define LS_LE_EN 1\n#endif\n#ifndef TOTAL_FRAME\n#define TOTAL_FRAME 1\n#endif\n#ifndef VC_ID\n#define VC_ID 3\n#endif\n#ifndef VRES\n#ifdef GDMA0_FULL_MEM\n#define VRES 1080\n#else\n#define VRES 3\n#endif\n#endif\n#ifndef HRES\n#ifdef GDMA0_FULL_MEM\n#define HRES 1920\n#else\n#define HRES 64\n#endif\n#endif\n#ifndef DATA_TYPE\n#define DATA_TYPE CSI2_RGB888\n#endif",
    "Meta Arrays": "NA",
    "Meta Types": "int data_rd, data_wr;\nint def_fail_cnt = 0, wr_fail_cnt = 0;\nint vcid_csi2_wrap_reg;\nint gdma_path;\nunsigned int ch0_pc, ch0_preload_loc;\nunsigned int ch1_pc, ch1_preload_loc;\nunsigned int gdma_reg_base;\nunsigned int rd_data;\nlong int csi_ctrl_data;\nint gdma_int_rsts;\nlong int word_count;\nint csi_data_size;\nlong long int rx_desc, tx_desc;\nlong long int gdma_tx_trnsfr_size, gdma_trnsfr_size;\nint cntrl_pkt_cnt;",
    "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY lane configurations by iterating from 4 active lanes down to 1 active lane. For each lane configuration, the test configures the CSI-2 host controller N_LANES register, sets up DMA transfers using dma_trnsfr_instn_preload for both control data (channel 0) and CSI data (channel 1), triggers the CSI-2 sequence, and validates DMA transfer completion via interrupt status polling on both DMA IRQ channels. The test initializes the D-PHY via snps_phy_init(), polls PHY_STOPSTATE until all lanes and clock lane enter stop state (0x1000F), configures the virtual channel register and control data register, and then loops through lane configurations performing DMA data transfers with data comparison validation using failure counters (def_fail_cnt, wr_fail_cnt).",
    "Meta Test Steps / Procedure": "Step 1: Call printf(\"start line\\n\").\nStep 2: Call csi2_enable_interrupt() to enable CSI-2 interrupt.\nStep 3: GDMA path selection via preprocessor conditionals:\n  - #ifdef GDMA3_PATH: vcid_csi2_wrap_reg = VC_ID; gdma_path = 3;\n  - #elif GDMA2_PATH: vcid_csi2_wrap_reg = (VC_ID << 4); gdma_path = 2;\n  - #elif GDMA1_PATH: vcid_csi2_wrap_reg = (VC_ID << 8); gdma_path = 1;\n  - #elif GDMA0_PATH: vcid_csi2_wrap_reg = (VC_ID << 12); printf(\"VC_ID=%d\\n\", VC_ID); gdma_path = 0;\nStep 4: Set gdma_reg_base = 0xE6A00000.\nStep 5: write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) \u2014 Write CSI-2 virtual channel register.\nStep 6: write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1) \u2014 Enable control data transfer.\nStep 7: printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg).\nStep 8: csi2_subsys_enable_interrupt() \u2014 commented out (//nmitta temp).\nStep 9: Call snps_phy_init() \u2014 D-PHY initialization sequence.\nStep 10: rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) \u2014 Read PHY stop state.\nStep 11: Polling loop: while(!(rd_data == 0x1000f)) { rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE); } \u2014 Wait for PHY to enter stop state on all lanes.\nStep 12: Set ch0_pc = 0xE6000000.\nStep 13: Set ch1_pc = 0xE6000500.\nStep 14: for(int lane_num=3; lane_num>=0; lane_num--) \u2014 Loop from 4 lanes down to 1 lane:\n  Step 14a: write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num) \u2014 Configure number of lanes in CSI-2 controller.\n  Step 14b: cntrl_pkt_cnt = ((VRES * 3) + 2) \u2014 Calculate control packet count.\n  Step 14c: printf(\"DEBUG: cntrl_pkt_cnt=%d\\n\", cntrl_pkt_cnt).\n  Step 14d: write_reg(0xa0243ffc, (lane_num + 1)) \u2014 Trigger start of CSI-2 sequence.\n  Step 14e: for(int i = 0; i < cntrl_pkt_cnt; i++) \u2014 Inner loop for each packet:\n    Step 14e-i: ch0_preload_loc = ch0_pc.\n    Step 14e-ii: ch1_preload_loc = ch1_pc.\n    Step 14e-iii: write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3) \u2014 Enable dma_irq[1] & dma_irq[0].\n    Step 14e-iv: dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0) \u2014 Control data transfer programming for channel 0.\n    Step 14e-v: dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1) \u2014 Data transfer programming for channel 1.\n    Step 14e-vi: DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1) \u2014 Start DMA channel 1.\n    Step 14e-vii: rd_data = 0; while((rd_data & 0x1) == 0x0) { rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET); printf(\"polling irq; irq_status_reg rd_data =%0x\\n\", rd_data); } \u2014 Poll DMA interrupt status for control packet transfer completion (bit 0).\n    Step 14e-viii: write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1) \u2014 Clear dma_irq[0].\n    Step 14e-ix: rd_data = read_reg(gdma_reg_base + 0x28) \u2014 Read DMA status.\n    Step 14e-x: rd_data = 0; while((rd_data & 0x2) == 0x0) { rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET); printf(\"polling irq; irq_status_reg rd_data =%0x\\n\", rd_data); } \u2014 Poll DMA interrupt status for data packet transfer completion (bit 1).\n    Step 14e-xi: rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) \u2014 Final read of interrupt status.\n    Step 14e-xii: write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2) \u2014 Clear dma_irq[1].\n    Step 14e-xiii: printf(\"DEBUG : irq polling completed ch1; rd_data=%0x\\n\", rd_data).\nStep 15: finish(0) \u2014 End test.",
    "Meta Impacted Registers": "DATA_IDS_VC_1\nPPI_PG_PATTERN_VRES\nPPI_PG_PATTERN_HRES",
    "Meta Validation / Acceptance Criteria": "Validation 1: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE \u2014 read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) is polled in a while loop until rd_data == 0x1000f. Expected value: 0x1000F indicates all data lanes (bits [3:0]) and clock lane (bit 16) are in stop state. Fail condition: loop does not terminate if PHY fails to reach stop state (no timeout).\nValidation 2: DMA interrupt status polling for control packet (bit 0) \u2014 rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled in a while loop until (rd_data & 0x1) != 0. Expected value: bit 0 of INTMIS register asserts (dma_irq[0] fires) indicating control packet DMA transfer completion. Fail condition: loop does not terminate if DMA transfer does not complete.\nValidation 3: DMA interrupt status polling for data packet (bit 1) \u2014 rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled in a while loop until (rd_data & 0x2) != 0. Expected value: bit 1 of INTMIS register asserts (dma_irq[1] fires) indicating data packet DMA transfer completion. Fail condition: loop does not terminate if DMA transfer does not complete.\nValidation 4: DMA interrupt clear for control packet \u2014 write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1) clears dma_irq[0] after control packet transfer completion.\nValidation 5: DMA interrupt clear for data packet \u2014 write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2) clears dma_irq[1] after data packet transfer completion.\nValidation 6: DMA status read \u2014 rd_data = read_reg(gdma_reg_base + 0x28) reads DMA status after clearing control packet interrupt.\nValidation 7: Data comparison \u2014 data_rd vs data_wr comparison performed during DMA transfers. def_fail_cnt is incremented when read-back does not match expected default value. wr_fail_cnt is incremented when read-back does not match written value. Pass condition: def_fail_cnt == 0 and wr_fail_cnt == 0.\nValidation 8: Control packet count \u2014 cntrl_pkt_cnt = ((VRES * 3) + 2) is computed and used to control the number of DMA transfer iterations per lane configuration.\nValidation 9: Lane iteration \u2014 test iterates lane_num from 3 down to 0 (4 lanes to 1 lane), validating DMA transfer completion for each lane configuration via write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num) and subsequent DMA operations.\nValidation 10: Test completion \u2014 finish(0) is called indicating successful test completion."
  },
  {
    "Test Case Name": "mipi_csi2_test_pattern_generator",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"",
    "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
    "Meta Arrays": "NA",
    "Meta Types": "int data_rd, data_wr;\nint def_fail_cnt = 0, wr_fail_cnt = 0;\nint vcid_csi2_wrap_reg;\nint gdma_path;\nlong int csi_ctrl_data;\nint gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;\nunsigned int rd_data;\nunsigned int tx_trnsfr_size;\nlong long int rx_desc, tx_desc;\nlong long int gdma_tx_trnsfr_size, gdma_trnsfr_size;\nint cntrl_pkt_cnt;\nint int_pend;\nint vcid;\nint vcid_unselected_path;\nint hres, vres;\nint valid_bits_per_pixel;\nunsigned int dma_ch0_pc;\nunsigned int dma_ch0_instn_preload_addr;\nint dma_dest_addr_incr_flag;\nunsigned int csi2_data_trnsfr_size;\nunsigned int gdma_reg_base;",
    "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator functionality. The test configures virtual channel IDs and GDMA path selection via preprocessor conditionals, writes the CSI-2 wrapper virtual channel register and disables control data transfer (writes 0 to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA). It computes the CSI-2 data transfer size based on hres=320, vres=16, and valid_bits_per_pixel=24 with 8-byte alignment. It programs DMA transfer instructions via dma_trnsfr_instn_preload_incr_addr() with conditional destination address increment based on FPS60 define, starts DMA channel 0 via DMAGO_CSI(), enables the pattern generator via csi2_ctrlr_pg_enable() which writes PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, and PPI_PG_ENABLE registers, waits, disables the pattern generator, polls DMA interrupt status for transfer completion, and finishes the test.",
    "Meta Test Steps / Procedure": "Step 1: Call printf(\"start line\\n\").\nStep 2: Call csi2_enable_interrupt() to enable CSI-2 interrupt.\nStep 3: Set int_pend = 1.\nStep 4: Set vcid = 3.\nStep 5: Compute vcid_unselected_path = ((vcid + 1) & 0xf).\nStep 6: GDMA path selection via preprocessor conditionals:\n  - #ifdef GDMA3_PATH: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+(vcid)); gdma_path = 3;\n  - #elif GDMA2_PATH: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid<<4)+(vcid_unselected_path)); gdma_path = 2;\n  - #elif GDMA1_PATH: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path)); gdma_path = 1;\n  - #else (GDMA0_PATH): vcid_csi2_wrap_reg = ((vcid<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path)); gdma_path = 0.\nStep 7: Compute gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000).\nStep 8: write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) \u2014 Write CSI-2 virtual channel register.\nStep 9: write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0) \u2014 Disable control data transfer.\nStep 10: Set hres = 320.\nStep 11: Set vres = 16.\nStep 12: Set valid_bits_per_pixel = 24.\nStep 13: Compute csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres)/8) % 8) ? (((valid_bits_per_pixel * hres)/8) + 8 - (((valid_bits_per_pixel * hres)/8) % 8)) : ((valid_bits_per_pixel * hres)/8)) * vres).\nStep 14: Set dma_ch0_pc = 0xE6000000.\nStep 15: Set dma_ch0_instn_preload_addr = dma_ch0_pc.\nStep 16: Conditional dma_dest_addr_incr_flag:\n  - #ifdef FPS60: dma_dest_addr_incr_flag = 0;\n  - #else: dma_dest_addr_incr_flag = 1;\nStep 17: Call dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base, 0x00, 0xE6001000, csi2_data_trnsfr_size, 0, dma_dest_addr_incr_flag, 0) \u2014 Program DMA transfer instructions for channel 0.\nStep 18: Call DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0) \u2014 Start DMA channel 0.\nStep 19: Call csi2_ctrlr_pg_enable() \u2014 Enable pattern generator. Internally this function performs:\n  Step 19a: write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10) \u2014 Set vertical resolution.\n  Step 19b: write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140) \u2014 Set horizontal resolution.\n  Step 19c: write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401) \u2014 Configure pattern generator settings.\n  Step 19d: write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1) \u2014 Enable pattern generator.\nStep 20: Call wait_on(100) \u2014 Wait for pattern generator to produce data.\nStep 21: write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0) \u2014 Disable pattern generator.\nStep 22: rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) \u2014 Read DMA interrupt status.\nStep 23: Polling loop: while((rd_data & 0x1) == 0) { rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET); } \u2014 Poll DMA interrupt status for transfer completion (bit 0).\nStep 24: Call wait_on(10000) \u2014 Post-completion wait.\nStep 25: Call finish(0) \u2014 End test.",
    "Meta Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": "Validation 1: DMA interrupt status polling for data transfer completion (bit 0) \u2014 rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled in a while loop until (rd_data & 0x1) != 0. Expected value: bit 0 of INTMIS register asserts (dma_irq[0] fires) indicating DMA channel 0 transfer completion. Fail condition: loop does not terminate if DMA transfer does not complete (no timeout).\nValidation 2: Pattern generator enable \u2014 csi2_ctrlr_pg_enable() writes MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES = 0x10, MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES = 0x70140, MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG = 0xe401, MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE = 1. Expected: pattern generator starts producing data after enable.\nValidation 3: Pattern generator disable \u2014 write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0) disables the pattern generator after wait_on(100). Expected: pattern generator stops producing data.\nValidation 4: CSI-2 data transfer size computation \u2014 csi2_data_trnsfr_size is computed as (((((valid_bits_per_pixel * hres)/8) % 8) ? (((valid_bits_per_pixel * hres)/8) + 8 - (((valid_bits_per_pixel * hres)/8) % 8)) : ((valid_bits_per_pixel * hres)/8)) * vres). With valid_bits_per_pixel=24, hres=320, vres=16, the per-line byte count is 960 (already 8-byte aligned), total = 960 * 16 = 15360 bytes.\nValidation 5: Virtual channel register configuration \u2014 write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) writes the computed vcid_csi2_wrap_reg value based on GDMA path selection. vcid=3, vcid_unselected_path=4.\nValidation 6: Control data disabled \u2014 write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0) disables control data transfer for the test pattern generator flow.\nValidation 7: DMA transfer programming \u2014 dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base, 0x00, 0xE6001000, csi2_data_trnsfr_size, 0, dma_dest_addr_incr_flag, 0) programs DMA channel 0 with source address 0x00, destination address 0xE6001000, transfer size csi2_data_trnsfr_size, source increment disabled, destination increment conditional on FPS60.\nValidation 8: DMA channel start \u2014 DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0) starts DMA channel 0 for CSI data transfer.\nValidation 9: Post-completion wait \u2014 wait_on(10000) provides a drain/cooldown period after DMA completion.\nValidation 10: Test completion \u2014 finish(0) is called indicating successful test completion."
  }
]

# ============================================================
# CROSS-DATASET VALIDATION
# ============================================================
meta_names = {r["Test Case Name"] for r in META_JSON}
tp_names = {r["Test Case Name"] for r in TESTPLAN_JSON}
if not tp_names.issubset(meta_names):
    missing = tp_names - meta_names
    print(f"VALIDATION FAILED: testcases in TESTPLAN but not in META: {missing}", file=sys.stderr)
    sys.exit(1)

# ============================================================
# WORKBOOK GENERATION
# ============================================================
wb = Workbook()

# -- TestPlan Sheet --
ws_tp = wb.active
ws_tp.title = "TestPlan"
tp_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks"
]
ws_tp.append(tp_columns)
for rec in TESTPLAN_JSON:
    ws_tp.append([
        rec.get("Index", ""),
        rec.get("SS / Module", ""),
        rec.get("Feature", ""),
        rec.get("Test Case Name", ""),
        rec.get("Speed", ""),
        rec.get("Mode", ""),
        rec.get("Memory Start Offset", ""),
        rec.get("Memory End Offset", ""),
        rec.get("Test Description", ""),
        rec.get("Test Steps / Procedure", ""),
        rec.get("Impacted Registers", ""),
        rec.get("Validation / Acceptance Criteria", ""),
        rec.get("Remarks", "")
    ])

# -- MetaData Sheet --
ws_md = wb.create_sheet("MetaData")
md_columns = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]
ws_md.append(md_columns)
for rec in META_JSON:
    ws_md.append([
        rec.get("Test Case Name", ""),
        rec.get("Meta Headers", ""),
        rec.get("Meta Macros", ""),
        rec.get("Meta Arrays", ""),
        rec.get("Meta Types", ""),
        rec.get("Meta Test Description", ""),
        rec.get("Meta Test Steps / Procedure", ""),
        rec.get("Meta Impacted Registers", ""),
        rec.get("Meta Validation / Acceptance Criteria", "")
    ])

# ============================================================
# FORMATTING
# ============================================================
hf = Font(bold=True, color="FFFFFF", size=11)
hfill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
ha = Alignment(horizontal="center", vertical="center", wrap_text=True)
ca = Alignment(vertical="top", wrap_text=True)
tb = Border(
    left=Side(style="thin"), right=Side(style="thin"),
    top=Side(style="thin"), bottom=Side(style="thin")
)

for ws in [ws_tp, ws_md]:
    for cell in ws[1]:
        cell.font = hf
        cell.fill = hfill
        cell.alignment = ha
        cell.border = tb
    for row in ws.iter_rows(min_row=2):
        for cell in row:
            cell.alignment = ca
            cell.border = tb
    ws.freeze_panes = "A2"
    for col_cells in ws.columns:
        mx = 0
        cl = col_cells[0].column_letter
        for c in col_cells:
            if c.value:
                for ln in str(c.value).split("\n"):
                    mx = max(mx, len(ln))
        ws.column_dimensions[cl].width = max(min(mx + 4, 65), 14)

# MetaData sheet: veryHidden
ws_md.sheet_state = "veryHidden"

# ============================================================
# SAVE AND ENCODE
# ============================================================
# Save to file
wb.save(FILENAME)
print(f"Saved: {FILENAME}", file=sys.stderr)

# Save to buffer and print base64
buf = io.BytesIO()
wb.save(buf)
b64 = base64.b64encode(buf.getvalue()).decode("ascii")
print(b64)

# ============================================================
# POST-SAVE VALIDATION
# ============================================================
try:
    from openpyxl import load_workbook
    wb2 = load_workbook(FILENAME)
    assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in wb2.sheetnames, "MetaData sheet missing"
    tp_rows = wb2["TestPlan"].max_row - 1  # minus header
    md_rows = wb2["MetaData"].max_row - 1  # minus header
    assert tp_rows == len(TESTPLAN_JSON), f"TestPlan rows mismatch: {tp_rows} vs {len(TESTPLAN_JSON)}"
    assert md_rows == len(META_JSON), f"MetaData rows mismatch: {md_rows} vs {len(META_JSON)}"
    print(f"VALIDATION PASSED: TestPlan={tp_rows} rows, MetaData={md_rows} rows", file=sys.stderr)
    print(json.dumps({
        "status": "SUCCESS",
        "filename": FILENAME,
        "rows_testplan": tp_rows,
        "rows_metadata": md_rows,
        "validation": "PASSED"
    }), file=sys.stderr)
except Exception as e:
    print(f"VALIDATION FAILED: {e}", file=sys.stderr)
    sys.exit(1)
