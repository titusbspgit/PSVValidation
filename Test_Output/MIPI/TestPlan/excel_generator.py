#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import json
import sys

def generate_testplan():
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    ip_name = "MIPI_DSI"
    filename = f"{ip_name}_TestPlan_{timestamp}.xlsx"
    output_dir = os.environ.get("OUTPUT_DIR", ".")
    filepath = os.path.join(output_dir, filename)

    json_data = [
        {
            "Index": "1",
            "SS / Module": "MIPI_DSI",
            "Test Case Name": "mipi_dsi_basic_test",
            "Feature": "DBI Data Transfer with DMA",
            "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_dsi.h\"",
            "Meta Macros": "NA",
            "Meta Arrays": "data_tdbdcb[2]; data_rebdcb[2]; cmd_tdbdcb[2]; cmd_rebdcb[2]",
            "Speed": "NA",
            "Mode": "DBI Mode",
            "Memory Start Offset": "0x0000; 0x10000; 0x3F000; 0x3F800",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase performs a basic MIPI DSI DBI (Display Bus Interface) data transfer using a 2-channel DMA engine. Channel 0 (CH0) is used for data transfer and Channel 1 (CH1) is used for command transfer. The test configures the DSI host PHY interface with n_lanes=3 and phy_stop_wait_time=0x40 via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. It writes MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with 0x3d and MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with 0x107. DBI mode is enabled by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. DBI configuration parameters include dbi_vcid=0x3, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x1, allowed_cmd_size=0x7, tear_fx_en=0x1, generic_vc_id=0x2. The DPI clock is programmed with dpi_clk_time_period=16.012400ns. The test uses num_of_pixel=40 and computes wr_cmd_size and num_bytes via pixel_to_bytes_wr_cmd_size(). DMA descriptors are programmed in a loop for 1 descriptor iteration: data source at RAM_BASE+0x10000, data destination at 0x10000000000, command source at RAM_BASE+0x0000, command destination at 0x10000008000. DMA microcode instructions DMAMOV, program_data_num_bytes, DMAWMB, DMASEV, DMAEND are used for both channels. Random data is loaded via load_rand_data() and write command is loaded via load_wr_command() with DSI_WRITE_MEMORY_START. DMA channels are started by writing MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and MIZAR_MIPI_DSI_DMAC_DBGCMD for CH0 (0x00A00000) and CH1 (0x01A00000). The test waits for int_pend1 to become 0 via polling with wait_on(10). The IRQ handler Default_IRQHandler reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and compares against MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR. If matched, it reads MIZAR_MIPI_DSI_DMAC_INTMIS to get channel mask status, clears int_pend1 bits, writes MIZAR_MIPI_DSI_DMAC_INTCLR with ch_mask_st, then writes MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW with dsi_subsys_mask_st to clear subsystem interrupt. Unexpected interrupts increment err0. The test finishes by calling finish(err0).",
            "Test Description": "This testcase validates a basic MIPI DSI DBI (Display Bus Interface) data transfer using a 2-channel DMA engine. Channel 0 transfers pixel data and Channel 1 transfers commands. The test configures the DSI host PHY interface for 3 data lanes with a PHY stop wait time, sets packet handling and clock manager configurations, enables DBI mode via the DPI control register, initializes the PHY, and configures DBI parameters including virtual channel ID, LUT size, output/input DBI configuration, partitioning, command size, and tear effect enable. A DPI clock is programmed. DMA descriptors are built for both data and command channels with source and destination addresses, random data is loaded into SRAM, and a DSI write memory start command is prepared. Both DMA channels are started via the DMA debug instruction registers. The test waits for DMA completion interrupts on both channels. The interrupt handler validates the interrupt source at the subsystem level, reads the DMA masked interrupt status, clears the DMA and subsystem interrupts, and tracks errors for unexpected interrupts. The test passes if no unexpected interrupts occur.",
            "Meta Test Steps / Procedure": "1. Enable GIC IRQ for DSI interrupt number DSI_INTR_NO via GIC_EnableIRQ(DSI_INTR_NO). 2. Initialize int_pend=1, int_pend1=0x3 (both CH0 and CH1 pending), phy_stop_wait_time=0x40, n_lanes=3. 3. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA interrupts for channels 0 and 1. 4. Write MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable GDMA interrupt at subsystem level. 5. Compute phy_if_cfg = n_lanes (3), then apply phy_stop_wait_time (0x40) using set_data_mask() with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME mask. 6. Write computed phy_if_cfg to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. 7. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG. 8. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. 9. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to enable DBI mode. 10. Call phy_init() to initialize the DSI PHY. 11. Set DBI configuration variables: dbi_vcid=0x3, load_cmd_or_data_to_sram=1, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x1, allowed_cmd_size=0x7. 12. Set num_of_pixel=40, call pixel_to_bytes_wr_cmd_size(num_of_pixel) to compute wr_cmd_size and num_bytes from pixel_attr. 13. Set command type variables: max_rd_pkt_size=0, dcs_lw_tx=0, dcs_sr_0p_tx=0, dcs_sw_1p_tx=0, dcs_sw_0p_tx=0, gen_lw_tx=0, gen_sr_2p_tx=0, gen_sr_1p_tx=0, gen_sr_0p_tx=0, gen_sw_2p_tx=0, gen_sw_1p_tx=0, gen_sw_0p_tx=0, ack_rqst_en=0, tear_fx_en=1, generic_vc_id=0x2. 14. Set dpi_clk_time_period=16.012400, compute dpi_clk_freq=1000000000/dpi_clk_time_period, call program_dpi_clock(dpi_clk_freq). 15. Call dbi_config() to apply all DBI configuration settings. 16. Set num_descriptors=1. 17. Set ch0_desc_addr=RAM_BASE+0x3F000, ch1_desc_addr=RAM_BASE+0x3F800, save actual addresses in ch0_desc_addr_act and ch1_desc_addr_act. 18. Enter descriptor loop (itter=0 to num_descriptors-1): a. Set data_tdbdcb[0].addr=RAM_BASE+0x10000, data_tdbdcb[0].len=num_bytes, data_tdbdcb[0].eop=1. b. Set data_rebdcb[0].addr=0x10000000000, data_rebdcb[0].len=num_bytes. c. Set cmd_tdbdcb[0].addr=RAM_BASE+0x0000, cmd_tdbdcb[0].len=4, cmd_tdbdcb[0].eop=1. d. Set cmd_rebdcb[0].addr=0x10000008000, cmd_rebdcb[0].len=4. e. Program CH0 DMA microcode: DMAMOV(&ch0_desc_addr, DMA_SAR, data_tdbdcb[0].addr), DMAMOV(&ch0_desc_addr, DMA_DAR, data_rebdcb[0].addr), program_data_num_bytes(&ch0_desc_addr, data_tdbdcb[0].len), DMAWMB(&ch0_desc_addr), DMASEV(&ch0_desc_addr, 0), DMAEND(&ch0_desc_addr). f. Program CH1 DMA microcode: DMAMOV(&ch1_desc_addr, DMA_SAR, cmd_tdbdcb[0].addr), DMAMOV(&ch1_desc_addr, DMA_DAR, cmd_rebdcb[0].addr), program_data_num_bytes(&ch1_desc_addr, cmd_tdbdcb[0].len), DMAWMB(&ch1_desc_addr), DMASEV(&ch1_desc_addr, 1), DMAEND(&ch1_desc_addr). g. Call load_rand_data(data_tdbdcb[0]) to load random data into SRAM. h. Call load_wr_command(cmd_tdbdcb[0].addr, num_bytes, DSI_WRITE_MEMORY_START) to load write command into SRAM. 19. Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 (CH0 GO instruction). 20. Write ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 (CH0 descriptor address). 21. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute CH0 DMA instruction. 22. Write 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 (CH1 GO instruction). 23. Write ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 (CH1 descriptor address). 24. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute CH1 DMA instruction. 25. Enter polling loop: while int_pend1 is non-zero, call wait_on(10) repeatedly. 26. Call wait_on(10000) for final delay. 27. Call finish(err0) to complete the test with error count. 28. IRQ Handler (Default_IRQHandler): a. Set int_pend=0. b. Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK into dsi_subsys_mask_st. c. Compare dsi_subsys_mask_st with MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR. d. If matched: read MIZAR_MIPI_DSI_DMAC_INTMIS into ch_mask_st. e. If ch_mask_st is non-zero: clear corresponding bits in int_pend1 (int_pend1 = int_pend1 & ~ch_mask_st), write ch_mask_st to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMA interrupt. f. If ch_mask_st is zero: print error, increment err0. g. Write dsi_subsys_mask_st to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem interrupt. h. If dsi_subsys_mask_st does not match GDMA_INTR: print error, increment err0. i. Call GIC_ClearIRQ(DSI_INTR_NO) to clear GIC interrupt. j. Set int_pend=1.",
            "Test Steps / Procedure": "1. Enable the DSI interrupt in the GIC. 2. Configure DMA interrupt enable for channels 0 and 1 by writing to the INTEN register. 3. Enable GDMA interrupt at the subsystem level by writing to the interrupt_enable register. 4. Configure the DSI host PHY interface with 3 data lanes and PHY stop wait time by writing to the PHY_IF_CFG register. 5. Configure packet handling by writing to the PCKHDL_CFG register. 6. Configure the clock manager by writing to the CLKMGR_CFG register. 7. Enable DBI mode by writing to the dpi_control register. 8. Initialize the DSI PHY. 9. Configure DBI parameters including virtual channel ID, LUT size, output/input DBI configuration, partitioning, command size, and tear effect enable. 10. Compute pixel-to-byte conversion for 40 pixels. 11. Program the DPI clock frequency. 12. Apply DBI configuration. 13. Set up DMA descriptor addresses for data channel (CH0) and command channel (CH1) in SRAM. 14. Build DMA descriptors: set source/destination addresses and transfer lengths for both data and command channels. 15. Program DMA microcode for CH0 (data) and CH1 (command) with move, write memory barrier, send event, and end instructions. 16. Load random pixel data into SRAM for the data channel. 17. Load the DSI write memory start command into SRAM for the command channel. 18. Start CH0 DMA transfer by writing GO instruction and descriptor address to DBGINST0, DBGINST1, and executing via DBGCMD. 19. Start CH1 DMA transfer by writing GO instruction and descriptor address to DBGINST0, DBGINST1, and executing via DBGCMD. 20. Poll for DMA completion by waiting until both channel interrupts have been serviced. 21. In the interrupt handler, validate the interrupt source at the subsystem level by reading interrupt_mask, then read the DMA INTMIS register to identify the channel, clear the DMA interrupt via INTCLR, and clear the subsystem interrupt via interrupt_raw. 22. Wait for a final settling delay. 23. Complete the test and report the error count.",
            "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
            "Impacted Registers": "INTEN; interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; DBGINST0; DBGINST1; DBGCMD; interrupt_mask; INTMIS; INTCLR; interrupt_raw",
            "Meta Validation / Acceptance Criteria": "The test validates DMA transfer completion via interrupt-driven flow. In Default_IRQHandler, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK is read and compared against MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR. If matched, MIZAR_MIPI_DSI_DMAC_INTMIS is read to get ch_mask_st. If ch_mask_st is non-zero, the corresponding bits in int_pend1 are cleared (int_pend1 & ~ch_mask_st), and MIZAR_MIPI_DSI_DMAC_INTCLR is written with ch_mask_st to clear the DMA interrupt. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is written with dsi_subsys_mask_st to clear the subsystem-level interrupt. If dsi_subsys_mask_st does not match GDMA_INTR, err0 is incremented (ERROR1). If ch_mask_st is zero when GDMA_INTR is matched, err0 is incremented (ERROR2). The main loop polls int_pend1 via while(int_pend1) with wait_on(10). The test passes when int_pend1 reaches 0 (both CH0 and CH1 interrupts serviced) and err0 remains 0. finish(err0) is called where err0==0 indicates pass.",
            "Validation / Acceptance Criteria": "The test passes when both DMA channel completion interrupts (CH0 and CH1) are received and serviced correctly. The interrupt handler must read the subsystem interrupt_mask register and confirm the source is the GDMA interrupt. The DMA INTMIS register must report a valid channel mask. The DMA interrupt is cleared via the INTCLR register and the subsystem interrupt is cleared via the interrupt_raw register. Both channel pending flags must be cleared. No unexpected interrupts should occur. The error counter must remain zero at test completion.",
            "Remarks": "The test uses interrupt-driven DMA completion with a polling wait loop in the main thread. Two DMA channels operate concurrently: CH0 for pixel data and CH1 for commands. DBI mode is selected by writing 0 to the dpi_control register. The test loads both command and data into local SRAM for data integrity verification. The DMA microcode is programmed inline using DMAMOV, DMAWMB, DMASEV, and DMAEND helper functions. The DPI clock period is set to approximately 16.01 ns. The test depends on external functions phy_init, dbi_config, pixel_to_bytes_wr_cmd_size, program_dpi_clock, load_rand_data, load_wr_command, and set_data_mask whose implementations are outside the supplied testcase folder."
        },
        {
            "Index": "2",
            "SS / Module": "MIPI_DSI",
            "Test Case Name": "mipi_dsi_dbi_random_payload_test",
            "Feature": "DBI Random Payload Data Transfer with DMA",
            "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_dsi.h\"; <math.h>",
            "Meta Macros": "NA",
            "Meta Arrays": "data_tdbdcb[10]; data_rebdcb[10]; cmd_tdbdcb[10]; cmd_rebdcb[10]; wr_cmd_arr[27]; WR_CMD_SIZE[27]",
            "Speed": "NA",
            "Mode": "DBI Mode",
            "Memory Start Offset": "0x0000; 0x10000; 0x3F000; 0x3F800",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase performs MIPI DSI DBI data transfers with randomized pixel payload sizes using a 2-channel DMA engine. Channel 0 (CH0) is used for data transfer and Channel 1 (CH1) is used for command transfer. The test configures the DSI host PHY interface with n_lanes=3 and phy_stop_wait_time=0x40 via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. It writes MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with 0x3d and MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with 0x107. DBI mode is enabled by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. DBI configuration parameters include dbi_vcid=0x3, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x0, allowed_cmd_size=0x25, tear_fx_en=0x1, generic_vc_id=0x2. The DPI clock is programmed with dpi_clk_time_period=16.012400ns. An initial num_of_pixel=40 is used to compute wr_cmd_size and num_bytes via pixel_to_bytes_wr_cmd_size(). DMA interrupt enable is written to MIZAR_MIPI_DSI_DMAC_INTEN with 0x3. The test then enters a loop of 10 iterations. In each iteration, num_of_pixel is randomized as ((rand() % 1024)) * 8, and pixel_to_bytes_wr_cmd_size() recomputes wr_cmd_size and num_bytes. dbi_config() is called each iteration to reconfigure with the new wr_cmd_size. DMA descriptors are programmed for 1 descriptor: data source at RAM_BASE+0x10000+(itter*0x1000), data destination at 0x10000000000, command source at RAM_BASE+0x0000, command destination at 0x10000008000. The data length uses ceil((double)data_tdbdcb[itter].len/8)*8 for 8-byte alignment. DMA microcode instructions DMAMOV, program_data_num_bytes, DMAWMB, DMASEV, DMAEND are used for both channels. Random data is loaded via load_rand_data() and write command is loaded via load_wr_command() with DSI_WRITE_MEMORY_START. DMA channels are started by writing MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and MIZAR_MIPI_DSI_DMAC_DBGCMD for CH0 (0x00A00000) and CH1 (0x01A00000). The test polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until rd_data equals 0x3 (both channels complete). It then writes rd_data to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the interrupt. A wait_on(100000) delay follows each iteration. After all 10 iterations, finish(0) is called.",
            "Test Description": "This testcase validates MIPI DSI DBI data transfers with randomized pixel payload sizes across 10 iterations using a 2-channel DMA engine. Channel 0 transfers pixel data and Channel 1 transfers commands. The test configures the DSI host PHY interface for 3 data lanes with a PHY stop wait time, sets packet handling and clock manager configurations, enables DBI mode via the DPI control register, initializes the PHY, and configures DBI parameters including virtual channel ID, LUT size, output/input DBI configuration, partitioning, command size, and tear effect enable. A DPI clock is programmed. In each of the 10 loop iterations, a random pixel count (multiples of 8, up to 8192) is generated, the byte count and write command size are recomputed, and DBI configuration is reapplied. DMA descriptors are built for both data and command channels, random pixel data is loaded into SRAM, and a DSI write memory start command is prepared. Both DMA channels are started via the DMA debug instruction registers. The test polls the DMA masked interrupt status register until both channels report completion, then clears the DMA interrupt. A settling delay follows each iteration. The test passes unconditionally after all 10 iterations complete.",
            "Meta Test Steps / Procedure": "1. Set phy_stop_wait_time=0x40, n_lanes=3. 2. Compute phy_if_cfg = n_lanes (3), then apply phy_stop_wait_time (0x40) using set_data_mask() with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME mask. 3. Write computed phy_if_cfg to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. 4. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG. 5. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. 6. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to enable DBI mode. 7. Call phy_init() to initialize the DSI PHY. 8. Set DBI configuration variables: dbi_vcid=0x3, load_cmd_or_data_to_sram=1, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x0, allowed_cmd_size=0x25, wr_cmd_size=193. 9. Set num_of_pixel=40, call pixel_to_bytes_wr_cmd_size(num_of_pixel) to compute wr_cmd_size and num_bytes from pixel_attr. 10. Set command type variables: max_rd_pkt_size=0, dcs_lw_tx=0, dcs_sr_0p_tx=0, dcs_sw_1p_tx=0, dcs_sw_0p_tx=0, gen_lw_tx=0, gen_sr_2p_tx=0, gen_sr_1p_tx=0, gen_sr_0p_tx=0, gen_sw_2p_tx=0, gen_sw_1p_tx=0, gen_sw_0p_tx=0, ack_rqst_en=0, tear_fx_en=1, generic_vc_id=0x2. 11. Set dpi_clk_time_period=16.012400, compute dpi_clk_freq=1000000000/dpi_clk_time_period, call program_dpi_clock(dpi_clk_freq). 12. Call dbi_config() to apply initial DBI configuration settings. 13. Set num_descriptors=1. 14. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA interrupts for channels 0 and 1. 15. Enter main loop (i=0 to 9, 10 iterations): a. Generate random num_of_pixel = ((rand() % 1024)) * 8. b. Call pixel_to_bytes_wr_cmd_size(num_of_pixel) to recompute wr_cmd_size and num_bytes. c. Print i and wr_cmd_size via printf. d. Call dbi_config() to reconfigure DBI with new wr_cmd_size. e. Set ch0_desc_addr=RAM_BASE+0x3F000, ch1_desc_addr=RAM_BASE+0x3F800, save actual addresses in ch0_desc_addr_act and ch1_desc_addr_act. f. Enter descriptor loop (itter=0 to num_descriptors-1): i-viii. [DMA descriptor programming as detailed]. g-l. Start DMA channels. m-o. Poll INTMIS and clear via INTCLR. p. wait_on(100000). 16. Call finish(0) to complete the test with no errors.",
            "Test Steps / Procedure": "1. Configure the DSI host PHY interface with 3 data lanes and PHY stop wait time by writing to the PHY_IF_CFG register. 2. Configure packet handling by writing to the PCKHDL_CFG register. 3. Configure the clock manager by writing to the CLKMGR_CFG register. 4. Enable DBI mode by writing to the dpi_control register. 5. Initialize the DSI PHY. 6. Configure DBI parameters including virtual channel ID, LUT size, output/input DBI configuration, partitioning, command size, and tear effect enable. 7. Compute pixel-to-byte conversion for an initial pixel count. 8. Program the DPI clock frequency. 9. Apply initial DBI configuration. 10. Enable DMA interrupts for channels 0 and 1 by writing to the INTEN register. 11. Begin a loop of 10 iterations with randomized pixel counts (multiples of 8, up to 8192 pixels). 12. In each iteration, recompute the byte count and write command size from the random pixel count. 13. Reconfigure DBI settings with the new write command size. 14. Set up DMA descriptor addresses for data channel (CH0) and command channel (CH1) in SRAM. 15. Build DMA descriptors with source/destination addresses and transfer lengths for both channels, using 8-byte aligned data length for CH0. 16. Program DMA microcode for CH0 (data) and CH1 (command) with move, write memory barrier, send event, and end instructions. 17. Load random pixel data into SRAM for the data channel. 18. Load the DSI write memory start command into SRAM for the command channel. 19. Start CH0 DMA transfer by writing GO instruction and descriptor address to DBGINST0, DBGINST1, and executing via DBGCMD. 20. Start CH1 DMA transfer by writing GO instruction and descriptor address to DBGINST0, DBGINST1, and executing via DBGCMD. 21. Poll the INTMIS register until both channel completion bits are set. 22. Clear the DMA interrupt by writing the status to the INTCLR register. 23. Wait for a settling delay before the next iteration. 24. Complete the test after all 10 iterations.",
            "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
            "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; INTEN; DBGINST0; DBGINST1; DBGCMD; INTMIS; INTCLR",
            "Meta Validation / Acceptance Criteria": "The test validates DMA transfer completion via polling. In each of the 10 iterations, MIZAR_MIPI_DSI_DMAC_INTMIS is read in a while loop until rd_data equals 0x3 (both CH0 bit 0 and CH1 bit 1 set). Once rd_data == 0x3, the value is written to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA interrupt status. The test calls finish(0) unconditionally after all 10 iterations, indicating the test passes if the polling loop completes for all iterations without hanging. The randomized pixel count ensures varying payload sizes are tested across iterations.",
            "Validation / Acceptance Criteria": "The test passes when all 10 iterations of DMA transfers complete successfully. In each iteration, the INTMIS register must report both channel 0 and channel 1 completion bits set. The DMA interrupt is cleared via the INTCLR register after each iteration. The test completes successfully if the polling loop resolves for all iterations with varying random payload sizes. The test always reports pass upon completion of all iterations.",
            "Remarks": "The test uses polling-based DMA completion (no interrupt handler) unlike the basic test which uses interrupt-driven completion. Two DMA channels operate concurrently: CH0 for pixel data and CH1 for commands. DBI mode is selected by writing 0 to the dpi_control register. The pixel count is randomized each iteration as ((rand() % 1024)) * 8, producing multiples of 8 up to 8184 pixels. The data transfer length is 8-byte aligned using ceil(). DBI configuration is reapplied each iteration to update the write command size. The test depends on external functions phy_init, dbi_config, pixel_to_bytes_wr_cmd_size, program_dpi_clock, load_rand_data, load_wr_command, and set_data_mask whose implementations are outside the supplied testcase folder. The DPI clock period is set to approximately 16.01 ns. partitioning_en is set to 0x0 in this test unlike the basic test which uses 0x1. allowed_cmd_size is set to 0x25 in this test unlike the basic test which uses 0x7."
        }
    ]

    testplan_columns = [
        "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
        "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
        "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
        "Code Generation"
    ]

    metadata_columns = [
        "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
        "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
        "Meta Headers", "Meta Macros", "Meta Arrays"
    ]

    wb = openpyxl.Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    header_font = Font(bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_alignment = Alignment(wrap_text=True, vertical="top")

    for col_idx, col_name in enumerate(testplan_columns, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(testplan_columns, 1):
            value = row_data.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

    ws_tp.freeze_panes = "A2"

    ws_md = wb.create_sheet("MetaData")
    for col_idx, col_name in enumerate(metadata_columns, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(metadata_columns, 1):
            value = row_data.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

    ws_md.freeze_panes = "A2"
    ws_md.sheet_state = "veryHidden"

    max_col_width = 80
    for ws in [ws_tp, ws_md]:
        for col in ws.columns:
            max_length = 0
            col_letter = col[0].column_letter
            for cell in col:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_length = max(max_length, len(line))
            adjusted = min(max(max_length + 2, 12), max_col_width)
            ws.column_dimensions[col_letter].width = adjusted

    wb.save(filepath)

    assert os.path.exists(filepath), "File does not exist"
    assert os.path.getsize(filepath) > 0, "File is empty"
    wb2 = openpyxl.load_workbook(filepath)
    assert "TestPlan" in wb2.sheetnames
    assert "MetaData" in wb2.sheetnames
    wb2.close()

    print(f"FILE_GENERATED={filepath}")
    print(f"FILENAME={filename}")
    print(f"ROWS_TESTPLAN=2")
    print(f"ROWS_METADATA=2")
    print(f"VALIDATION=PASSED")
    return filepath, filename

if __name__ == "__main__":
    generate_testplan()
