#!/usr/bin/env python3
"""Auto-generated MIPI_DSI TestPlan Excel Generator - Agent 7
Run: python generate_mipi_dsi_xlsx_agent7.py
Requires: pip install openpyxl
"""
import json, os
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    print('ERROR: openpyxl not installed. Run: pip install openpyxl')
    exit(1)

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_DSI_TestPlan_{timestamp}.xlsx'

json_data = [
  {
    "Index": "1",
    "SS / Module": "MIPI_DSI",
    "Test Case Name": "mipi_dsi_basic_test",
    "Feature": "DBI DMA Data Transfer with Interrupt Handling",
    "Test Description": "This testcase validates a basic MIPI DSI DBI-mode data transfer using the DMA controller. It configures the PHY interface with 3 lanes and stop wait time, sets up packet handling and clock manager configurations, and enables DBI mode by disabling DPI control. After PHY initialization and DBI configuration, it prepares DMA descriptors for a data channel and a command channel, loads random pixel data and a write-memory-start command into SRAM, then triggers both DMA channels. The test uses interrupt-driven completion: the ISR reads the subsystem interrupt mask to confirm the GDMA interrupt source, reads the DMA masked interrupt status to identify the completing channel, clears the DMA and subsystem interrupts, and tracks channel completion. The test passes when both DMA channels complete without unexpected interrupts.",
    "Speed": "NA",
    "Mode": "DBI Mode",
    "Memory Start Offset": "0x0000; 0x10000; 0x3F000; 0x3F800",
    "Memory End Offset": "NA",
    "Remarks": "The test operates in DBI mode (DPI control disabled). Two DMA channels are used: CH0 for pixel data transfer and CH1 for command transfer. Interrupt-driven completion is used with a polling loop on int_pend1 in the main function. The ISR handles both channel interrupts and clears them at DMAC and subsystem levels. A final wait of 10000 cycles is applied after both channels complete before calling finish. The test uses 40 pixels with 3 PHY lanes and virtual channel ID 3. DMA descriptors and microcode are stored in SRAM at offsets relative to RAM_BASE.",
    "Test Steps / Procedure": "1. Enable the DSI interrupt in the GIC. 2. Enable DMA channel 0 and channel 1 interrupts by writing to the INTEN register. 3. Enable GDMA interrupt at the DSI subsystem level by writing to the interrupt_enable register. 4. Configure the PHY interface with 3 lanes and stop wait time by writing to the PHY_IF_CFG register. 5. Configure packet handling by writing to the PCKHDL_CFG register. 6. Configure the clock manager by writing to the CLKMGR_CFG register. 7. Select DBI mode by writing zero to the dpi_control register. 8. Initialize the MIPI DSI PHY. 9. Configure DBI parameters including virtual channel ID, LUT size, DBI format, partitioning, command size, and pixel count for 40 pixels. 10. Set command mode parameters and enable tearing effect. 11. Program the DPI clock and apply DBI configuration. 12. Set up DMA descriptors for data channel (CH0) and command channel (CH1) with source and destination addresses and transfer lengths. 13. Program DMA microcode for both channels including source/destination move, data length, write memory barrier, send event, and end instructions. 14. Load random pixel data into SRAM for the data channel. 15. Load the write-memory-start command into SRAM for the command channel. 16. Start DMA channel 0 by writing the debug instruction and command registers (DBGINST0, DBGINST1, DBGCMD). 17. Start DMA channel 1 by writing the debug instruction and command registers (DBGINST0, DBGINST1, DBGCMD). 18. Wait for both DMA channels to complete by polling the interrupt-driven completion flag. 19. Verify that the ISR correctly identifies the GDMA interrupt source by reading the interrupt_mask register. 20. Verify that the ISR reads the INTMIS register to identify the completing DMA channel. 21. Verify that the ISR clears the DMA interrupt via the INTCLR register and the subsystem interrupt via the interrupt_raw register. 22. Confirm the test completes with no errors.",
    "Impacted Registers": "INTEN; interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; DBGINST0; DBGINST1; DBGCMD; interrupt_mask; INTMIS; INTCLR; interrupt_raw",
    "Validation / Acceptance Criteria": "1. Both DMA channels (CH0 and CH1) must complete their transfers and trigger interrupts. 2. The ISR must correctly identify the GDMA interrupt source by reading the interrupt_mask register and matching the expected GDMA interrupt value. 3. The ISR must read the INTMIS register and confirm a non-zero channel status for each interrupt. 4. The ISR must clear the DMA interrupt via the INTCLR register and the subsystem interrupt via the interrupt_raw register. 5. The completion flag must reach zero, indicating both channels have been serviced. 6. No unexpected interrupt sources should be detected; the error counter must remain zero. 7. The test passes when finish is called with an error count of zero.",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_dsi.h\"",
    "Meta Macros": "NA",
    "Meta Arrays": "desc_t data_tdbdcb[2]; desc_t data_rebdcb[2]; desc_t cmd_tdbdcb[2]; desc_t cmd_rebdcb[2]",
    "Meta Test Description": "This testcase performs a basic MIPI DSI DBI (Display Bus Interface) data transfer using a DMA controller (DMAC). It begins by enabling the GIC interrupt for DSI_INTR_NO and setting int_pend=1, int_pend1=0x3 (indicating two DMA channels pending). PHY configuration is set with phy_stop_wait_time=0x40 and n_lanes=3. The DMAC interrupt enable register MIZAR_MIPI_DSI_DMAC_INTEN is written with 0x3 to enable interrupts for both channels. The subsystem interrupt enable register MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE is written with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR to enable GDMA interrupt at subsystem level. The PHY interface configuration register MIZAR_MIPI_DSI_HOST_PHY_IF_CFG is written with n_lanes and phy_stop_wait_time via set_data_mask using MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. The packet handler configuration register MIZAR_MIPI_DSI_HOST_PCKHDL_CFG is written with 0x3d. The clock manager configuration register MIZAR_MIPI_DSI_HOST_CLKMGR_CFG is written with 0x107. The DPI control register MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL is written with 0 to enable DBI mode. phy_init() is called to initialize the PHY. DBI configuration parameters are set: dbi_vcid=0x3, load_cmd_or_data_to_sram=1, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x1, allowed_cmd_size=0x7, num_of_pixel=40. pixel_to_bytes_wr_cmd_size() converts pixel count to byte count and wr_cmd_size. Various command mode parameters are set to 0x0 except tear_fx_en=0x1. generic_vc_id=0x2. DPI clock is programmed via program_dpi_clock(). dbi_config() is called to apply DBI configuration. DMA descriptors are set up in a loop for num_descriptors=1 iteration: CH0 (data channel) source is RAM_BASE+0x10000 with num_bytes length, destination is 0x10000000000; CH1 (command channel) source is RAM_BASE+0x0000 with 4 bytes length, destination is 0x10000008000. DMA microcode instructions DMAMOV, program_data_num_bytes, DMAWMB, DMASEV, DMAEND are programmed for both channels. Random data is loaded via load_rand_data() and write command is loaded via load_wr_command() with DSI_WRITE_MEMORY_START. DMA channel 0 is started by writing MIZAR_MIPI_DSI_DMAC_DBGINST0=0x00A00000, MIZAR_MIPI_DSI_DMAC_DBGINST1=ch0_desc_addr_act, MIZAR_MIPI_DSI_DMAC_DBGCMD=0x0. DMA channel 1 is started similarly with DBGINST0=0x01A00000. The test then polls int_pend1 in a while loop with wait_on(10) until both channel interrupts are serviced. A final wait_on(10000) is executed before calling finish(err0). The ISR Default_IRQHandler reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to check if the interrupt source is GDMA. If so, it reads MIZAR_MIPI_DSI_DMAC_INTMIS to identify which channel completed, clears int_pend1 bits for that channel, writes MIZAR_MIPI_DSI_DMAC_INTCLR with ch_mask_st to clear the DMAC interrupt, then writes MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW with dsi_subsys_mask_st to clear the subsystem interrupt. If unexpected interrupt sources are detected, err0 is incremented. GIC_ClearIRQ is called at the end of the ISR.",
    "Meta Test Steps / Procedure": "1. Enable GIC interrupt for DSI_INTR_NO via GIC_EnableIRQ(DSI_INTR_NO). 2. Set int_pend=1, int_pend1=0x3 (both DMA channels pending), phy_stop_wait_time=0x40, n_lanes=3. 3. Write MIZAR_MIPI_DSI_DMAC_INTEN with 0x3 to enable interrupts for DMA channels 0 and 1. 4. Write MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR to enable GDMA interrupt at subsystem level. 5. Compute phy_if_cfg = n_lanes, then apply phy_stop_wait_time using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME mask. 6. Write MIZAR_MIPI_DSI_HOST_PHY_IF_CFG with the computed phy_if_cfg value. 7. Write MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with 0x3d. 8. Write MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with 0x107. 9. Write MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL with 0 to select DBI mode. 10. Call phy_init() to initialize the MIPI DSI PHY. 11. Set DBI configuration parameters: dbi_vcid=0x3, load_cmd_or_data_to_sram=1, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x1, allowed_cmd_size=0x7. 12. Set num_of_pixel=40, call pixel_to_bytes_wr_cmd_size(num_of_pixel) to compute wr_cmd_size and num_bytes from pixel_attr. 13. Set command mode parameters: max_rd_pkt_size=0x0, dcs_lw_tx=0x0, dcs_sr_0p_tx=0x0, dcs_sw_1p_tx=0x0, dcs_sw_0p_tx=0x0, gen_lw_tx=0x0, gen_sr_2p_tx=0x0, gen_sr_1p_tx=0x0, gen_sr_0p_tx=0x0, gen_sw_2p_tx=0x0, gen_sw_1p_tx=0x0, gen_sw_0p_tx=0x0, ack_rqst_en=0x0, tear_fx_en=0x1, generic_vc_id=0x2. 14. Compute dpi_clk_freq from dpi_clk_time_period=16.012400ns, call program_dpi_clock(dpi_clk_freq). 15. Call dbi_config() to apply all DBI configuration settings. 16. Set num_descriptors=1. 17. Set ch0_desc_addr=RAM_BASE+0x3F000, ch1_desc_addr=RAM_BASE+0x3F800, ch0_desc_addr_act=ch0_desc_addr, ch1_desc_addr_act=ch1_desc_addr. 18. Enter loop for itter=0 to num_descriptors-1 (1 iteration): 19. Set data_tdbdcb[0].addr=RAM_BASE+0x10000, data_tdbdcb[0].len=num_bytes, data_tdbdcb[0].eop=1. 20. Set data_rebdcb[0].addr=0x10000000000, data_rebdcb[0].len=num_bytes. 21. Set cmd_tdbdcb[0].addr=RAM_BASE+0x0000, cmd_tdbdcb[0].len=4, cmd_tdbdcb[0].eop=1. 22. Set cmd_rebdcb[0].addr=0x10000008000, cmd_rebdcb[0].len=4. 23. Program DMA microcode for CH0. 24. Program DMA microcode for CH1. 25. Call load_rand_data(data_tdbdcb[0]). 26. Call load_wr_command(cmd_tdbdcb[0].addr, num_bytes, DSI_WRITE_MEMORY_START). 27. End loop. 28-33. Start DMA channels 0 and 1. 34. Poll int_pend1. 35. wait_on(10000). 36. finish(err0). 37-47. ISR handling.",
    "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
    "Meta Validation / Acceptance Criteria": "The test validates interrupt-driven DMA completion for both channels. In the ISR, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK is read and compared against MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR to confirm the interrupt source is GDMA. If matched, MIZAR_MIPI_DSI_DMAC_INTMIS is read to identify which DMA channel triggered the interrupt. If ch_mask_st is non-zero, the corresponding bits in int_pend1 are cleared and MIZAR_MIPI_DSI_DMAC_INTCLR is written with ch_mask_st to clear the DMAC interrupt. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is written with dsi_subsys_mask_st to clear the subsystem interrupt. If dsi_subsys_mask_st does not match GDMA interrupt, err0 is incremented (ERROR1). If ch_mask_st is zero when GDMA interrupt is indicated, err0 is incremented (ERROR2). The main loop polls int_pend1 until it becomes 0, indicating both channels completed. finish(err0) is called at the end; err0==0 indicates pass, err0>0 indicates failure due to unexpected interrupts."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_DSI",
    "Test Case Name": "mipi_dsi_dbi_random_payload_test",
    "Feature": "DBI Random Payload DMA Transfer with Polling Completion",
    "Test Description": "This testcase validates MIPI DSI DBI-mode data transfers with randomized pixel payload sizes across 10 iterations using the DMA controller. It configures the PHY interface with 3 lanes and stop wait time, sets up packet handling and clock manager configurations, and enables DBI mode by disabling DPI control. After PHY initialization and initial DBI configuration, it enables DMA interrupts for both channels. In each of 10 iterations, a random pixel count (multiples of 8, up to 8192) is generated, the DBI configuration is updated, DMA descriptors are prepared for a data channel and a command channel, random pixel data and a write-memory-start command are loaded into SRAM, and both DMA channels are triggered. The test uses polling-based completion by reading the DMA masked interrupt status register until both channels report completion, then clears the interrupts. The test passes after all 10 iterations complete successfully.",
    "Speed": "NA",
    "Mode": "DBI Mode",
    "Memory Start Offset": "0x0000; 0x10000; 0x3F000; 0x3F800",
    "Memory End Offset": "NA",
    "Remarks": "The test operates in DBI mode (DPI control disabled) and uses polling-based completion instead of interrupt-driven completion. Two DMA channels are used: CH0 for pixel data transfer and CH1 for command transfer. Random pixel counts are generated as ((rand() % 1024)) * 8, producing multiples of 8 up to 8184 pixels. The DBI configuration is re-applied in each iteration to accommodate the new payload size. Data transfer length for CH0 is rounded up to the nearest 8-byte boundary using ceil. A wait of 100000 cycles is applied after each iteration. The test uses 3 PHY lanes, virtual channel ID 3, partitioning disabled, and allowed_cmd_size of 0x25. DMA descriptors and microcode are stored in SRAM at offsets relative to RAM_BASE. The test always passes with finish(0) assuming the polling loop completes.",
    "Test Steps / Procedure": "1. Configure the PHY interface with 3 lanes and stop wait time by writing to the PHY_IF_CFG register. 2. Configure packet handling by writing to the PCKHDL_CFG register. 3. Configure the clock manager by writing to the CLKMGR_CFG register. 4. Select DBI mode by writing zero to the dpi_control register. 5. Initialize the MIPI DSI PHY. 6. Configure DBI parameters including virtual channel ID, LUT size, DBI format, partitioning, and command size. 7. Set command mode parameters and enable tearing effect. 8. Program the DPI clock and apply initial DBI configuration. 9. Enable DMA channel 0 and channel 1 interrupts by writing to the INTEN register. 10. Begin iterative transfer loop (10 iterations). 11. Generate a random pixel count (multiples of 8) and recompute the write command size and byte count. 12. Reconfigure DBI settings with the updated payload size. 13. Set up DMA descriptors for data channel (CH0) and command channel (CH1) with source and destination addresses and transfer lengths. 14. Program DMA microcode for both channels including source/destination move, data length with 8-byte alignment for CH0, write memory barrier, send event, and end instructions. 15. Load random pixel data into SRAM for the data channel. 16. Load the write-memory-start command into SRAM for the command channel. 17. Start DMA channel 0 by writing the debug instruction and command registers (DBGINST0, DBGINST1, DBGCMD). 18. Start DMA channel 1 by writing the debug instruction and command registers (DBGINST0, DBGINST1, DBGCMD). 19. Poll the INTMIS register until both DMA channels report completion. 20. Clear the DMA interrupts by writing to the INTCLR register. 21. Wait before proceeding to the next iteration. 22. Repeat steps 11-21 for all 10 iterations. 23. Confirm the test completes successfully after all iterations.",
    "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; INTEN; DBGINST0; DBGINST1; DBGCMD; INTMIS; INTCLR",
    "Validation / Acceptance Criteria": "1. Both DMA channels (CH0 and CH1) must complete their transfers in each of the 10 iterations, confirmed by polling the INTMIS register until both channel bits are set. 2. The DMA interrupts must be successfully cleared via the INTCLR register after each iteration. 3. The DBI configuration must be successfully updated with the new randomized payload size in each iteration. 4. The data channel transfer length must be correctly aligned to 8-byte boundaries. 5. The command channel must consistently transfer 4 bytes per iteration. 6. All 10 iterations must complete without hanging in the polling loop. 7. The test passes when finish is called with a value of zero after all iterations.",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_dsi.h\"; <math.h>",
    "Meta Macros": "NA",
    "Meta Arrays": "desc_t data_tdbdcb[10]; desc_t data_rebdcb[10]; desc_t cmd_tdbdcb[10]; desc_t cmd_rebdcb[10]; unsigned int wr_cmd_arr[27]; unsigned int WR_CMD_SIZE[27]",
    "Meta Test Description": "This testcase performs MIPI DSI DBI data transfers with randomized pixel payload sizes over 10 iterations using a DMA controller. It begins by setting phy_stop_wait_time=0x40 and n_lanes=3. The PHY interface configuration register MIZAR_MIPI_DSI_HOST_PHY_IF_CFG is written with n_lanes and phy_stop_wait_time via set_data_mask using MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. The packet handler configuration register MIZAR_MIPI_DSI_HOST_PCKHDL_CFG is written with 0x3d. The clock manager configuration register MIZAR_MIPI_DSI_HOST_CLKMGR_CFG is written with 0x107. The DPI control register MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL is written with 0 to enable DBI mode. phy_init() is called to initialize the PHY. DBI configuration parameters are set: dbi_vcid=0x3, load_cmd_or_data_to_sram=1, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x0, allowed_cmd_size=0x25. Initial wr_cmd_size is set to 193. num_of_pixel=40 is used for initial pixel_to_bytes_wr_cmd_size() call to compute wr_cmd_size and num_bytes from pixel_attr. Command mode parameters are set: all to 0x0 except tear_fx_en=0x1. generic_vc_id=0x2. DPI clock is programmed via program_dpi_clock() with dpi_clk_time_period=16.012400ns. dbi_config() is called to apply initial DBI configuration. The DMAC interrupt enable register MIZAR_MIPI_DSI_DMAC_INTEN is written with 0x3 to enable interrupts for both channels. A loop runs for i=0 to 9 (10 iterations). In each iteration: num_of_pixel is randomized as ((rand() % 1024)) * 8, pixel_to_bytes_wr_cmd_size() recomputes wr_cmd_size and num_bytes, dbi_config() is called again with updated parameters. DMA descriptors are set up with num_descriptors=1. DMA channels are started and polled. After polling, MIZAR_MIPI_DSI_DMAC_INTCLR is written to clear interrupts. After all 10 iterations complete, finish(0) is called.",
    "Meta Test Steps / Procedure": "1. Set phy_stop_wait_time=0x40, n_lanes=3. 2-6. Configure PHY, packet handler, clock manager, DPI control. 7. Initialize PHY. 8-13. Configure DBI and command mode parameters. 14-15. Program DPI clock and apply DBI config. 16-43. Iterative DMA transfer loop with polling completion.",
    "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
    "Meta Validation / Acceptance Criteria": "The test validates polling-based DMA completion for both channels across 10 iterations with randomized payload sizes. MIZAR_MIPI_DSI_DMAC_INTMIS is read and compared against 0x3 in a while loop to confirm both DMA channels (CH0 and CH1) have completed. Once rd_data equals 0x3, MIZAR_MIPI_DSI_DMAC_INTCLR is written with rd_data to clear the DMA interrupts. This polling and clearing sequence is repeated for each of the 10 iterations. After all iterations complete, finish(0) is called, indicating unconditional pass."
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

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = 'TestPlan'
ws_tp.freeze_panes = 'A2'

header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')

for col_idx, col_name in enumerate(tp_cols, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(tp_cols, 1):
        val = row_data.get(col_name, '')
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')
ws_md.sheet_state = 'veryHidden'
ws_md.freeze_panes = 'A2'

for col_idx, col_name in enumerate(md_cols, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(md_cols, 1):
        val = row_data.get(col_name, '')
        cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

# Auto-size columns
for ws in [ws_tp, ws_md]:
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            if cell.value:
                max_len = max(max_len, min(len(str(cell.value)), 80))
        ws.column_dimensions[col_letter].width = min(max(max_len + 2, 12), 60)

outdir = os.path.dirname(os.path.abspath(__file__))
fpath = os.path.join(outdir, filename)
wb.save(fpath)
print(f'Workbook saved: {fpath}')
print(f'Filename: {filename}')
print(f'TestPlan rows: {len(json_data)}')
print(f'MetaData rows: {len(json_data)}')

# Verify
wb2 = load_workbook(fpath)
assert 'TestPlan' in wb2.sheetnames
assert 'MetaData' in wb2.sheetnames
print('Validation: PASSED')
print(f'File size: {os.path.getsize(fpath)} bytes')
