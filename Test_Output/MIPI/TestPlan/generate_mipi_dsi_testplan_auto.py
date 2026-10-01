#!/usr/bin/env python3
"""Auto-generated MIPI DSI TestPlan Excel Generator
Generates: MIPI_DSI_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx
Timezone: IST (GMT+05:30)
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter

# IST Timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_DSI_TestPlan_{timestamp}.xlsx"

# Input JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "DBI Data Transfer with DMA",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_dsi.h"',
        "Meta Macros": "NA",
        "Meta Arrays": "data_tdbdcb[2]; data_rebdcb[2]; cmd_tdbdcb[2]; cmd_rebdcb[2]",
        "Speed": "NA",
        "Mode": "DBI Mode",
        "Memory Start Offset": "0x0000; 0x10000; 0x3F000; 0x3F800",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs a basic MIPI DSI DBI (Display Bus Interface) data transfer using the integrated DMA controller (DMA330). It enables GIC interrupt DSI_INTR_NO, sets int_pend=1 and int_pend1=0x3 (bitmask for 2 DMA channels). It configures phy_stop_wait_time=0x40 and n_lanes=3. It writes MIZAR_MIPI_DSI_DMAC_INTEN with 0x3 to enable DMA interrupts for channels 0 and 1. It writes MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR to enable GDMA interrupt at subsystem level. It constructs phy_if_cfg using n_lanes and set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field and writes to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. It writes MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with 0x3d and MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with 0x107. It writes MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL with 0 to enable DBI mode. It calls phy_init() for PHY initialization. It configures DBI parameters: dbi_vcid=0x3, load_cmd_or_data_to_sram=1, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x1, allowed_cmd_size=0x7, num_of_pixel=40. It calls pixel_to_bytes_wr_cmd_size(40) to compute wr_cmd_size and num_bytes. It sets various DSI command type flags to 0x0, tear_fx_en=0x1, generic_vc_id=0x2. It programs DPI clock with dpi_clk_time_period=16.012400ns. It calls dbi_config() to apply DBI configuration. It sets up DMA descriptors for 1 iteration: CH0 (data) uses data_tdbdcb with addr=RAM_BASE+0x10000 and data_rebdcb with addr=0x10000000000; CH1 (command) uses cmd_tdbdcb with addr=RAM_BASE+0x0000 and cmd_rebdcb with addr=0x10000008000. For each channel it programs DMAMOV for SAR/DAR, program_data_num_bytes for transfer length, DMAWMB, DMASEV, and DMAEND instructions. It loads random data via load_rand_data() and write command via load_wr_command() with DSI_WRITE_MEMORY_START. It starts DMA CH0 by writing MIZAR_MIPI_DSI_DMAC_DBGINST0=0x00A00000, MIZAR_MIPI_DSI_DMAC_DBGINST1=ch0_desc_addr_act, MIZAR_MIPI_DSI_DMAC_DBGCMD=0x0. It starts DMA CH1 similarly with DBGINST0=0x01A00000 and DBGINST1=ch1_desc_addr_act. It polls int_pend1 in a while loop with wait_on(10) until both channels complete. It waits wait_on(10000) then calls finish(err0). The ISR Default_IRQHandler reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK, checks for GDMA interrupt, reads MIZAR_MIPI_DSI_DMAC_INTMIS to get channel status, clears int_pend1 bits, writes MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMA interrupt, writes MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem interrupt, and calls GIC_ClearIRQ.",
        "Test Description": "This test validates a basic MIPI DSI DBI (Display Bus Interface) data transfer using the integrated DMA controller. It configures the DSI host PHY interface with 3 lanes and a stop wait time, sets up packet handling and clock manager registers, and enables DBI mode by disabling DPI control. After PHY and DBI configuration, it prepares DMA descriptors for two channels: one for pixel data and one for DBI write commands. The DMA controller is started for both channels using debug instruction registers. The test uses interrupt-driven completion: an ISR reads the subsystem interrupt mask register to identify the GDMA interrupt source, reads the DMA masked interrupt status register to determine which channel completed, clears the DMA and subsystem-level interrupts, and decrements a pending channel counter. The test waits for both DMA channels to complete and then reports pass or fail based on the accumulated error count.",
        "Meta Test Steps / Procedure": "1. Entry: test_case() is called. 2. GIC_EnableIRQ(DSI_INTR_NO) is called to enable the DSI interrupt in the GIC. 3. int_pend is set to 1. 4. int_pend1 is set to 0x3 (bitmask indicating both CH0 and CH1 are pending). 5. phy_stop_wait_time is set to 0x40. 6. n_lanes is set to 3. 7. write_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3) is called to enable DMA interrupts for channels 0 and 1. 8. write_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR) is called to enable GDMA interrupt at subsystem level. 9. phy_if_cfg is set to n_lanes (3). 10. phy_if_cfg is modified using set_data_mask(phy_if_cfg, MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME, phy_stop_wait_time) to insert phy_stop_wait_time=0x40 into the PHY_STOP_WAIT_TIME field. 11. write_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg) is called to configure PHY interface. 12. write_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3d) is called to configure packet handling. 13. write_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107) is called to configure clock manager. 14. write_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0) is called to disable DPI and enable DBI mode. 15. phy_init() is called to initialize the DSI PHY. 16-84. (Full DBI config, DMA setup, ISR handling steps as documented).",
        "Test Steps / Procedure": "1. Enable the DSI interrupt in the GIC. 2. Enable DMA interrupts for channels 0 and 1 by writing to the INTEN register. 3. Enable GDMA interrupt at the subsystem level by writing to the interrupt_enable register. 4. Configure the PHY interface with 3 lanes and a stop wait time by writing to the PHY_IF_CFG register. 5. Configure packet handling by writing to the PCKHDL_CFG register. 6. Configure the clock manager by writing to the CLKMGR_CFG register. 7. Disable DPI control to enable DBI mode by writing to the dpi_control register. 8. Initialize the DSI PHY. 9. Configure DBI parameters including virtual channel ID, LUT size, DBI input/output configuration, partitioning, command size, and pixel count (40 pixels). 10. Program the DPI clock frequency. 11. Apply DBI configuration. 12. Set up DMA descriptor memory regions for CH0 (data) and CH1 (command). 13. Program DMA microcode instructions for both channels including source address, destination address, transfer length, write memory barrier, send event, and end. 14. Load random pixel data into SRAM for the data channel. 15. Load the DBI write memory start command into SRAM for the command channel. 16. Start DMA CH0 by writing the GO debug instruction via DBGINST0, DBGINST1, and DBGCMD registers. 17. Start DMA CH1 by writing the GO debug instruction via DBGINST0, DBGINST1, and DBGCMD registers. 18. Poll for both DMA channel completions via interrupt-driven mechanism. 19. Verify that the ISR correctly identifies the GDMA interrupt source from the interrupt_mask register, reads channel completion status from the INTMIS register, clears channel interrupts via the INTCLR register, and clears subsystem interrupts via the interrupt_raw register. 20. Wait for a final settling delay after all transfers complete. 21. Report test result based on accumulated error count.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "INTEN; interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; DBGINST0; DBGINST1; DBGCMD; interrupt_mask; INTMIS; INTCLR; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "The test validates via interrupt-driven completion. In Default_IRQHandler, dsi_subsys_mask_st is read from MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and compared against MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR. If it does not match, err0 is incremented and an error message is printed. If it matches, ch_mask_st is read from MIZAR_MIPI_DSI_DMAC_INTMIS. If ch_mask_st is non-zero, int_pend1 bits are cleared (int_pend1 & ~ch_mask_st) and MIZAR_MIPI_DSI_DMAC_INTCLR is written with ch_mask_st to clear the DMA interrupt. If ch_mask_st is zero, err0 is incremented (unexpected interrupt). MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is written with dsi_subsys_mask_st to clear subsystem interrupt. The main loop polls int_pend1 until it becomes 0 (both channels completed). The test passes when finish(err0) is called with err0==0. Any unexpected interrupt source or missing channel status increments err0, causing test failure.",
        "Validation / Acceptance Criteria": "The test passes when both DMA channels (CH0 and CH1) complete their transfers successfully via interrupt-driven completion. The ISR must correctly identify the GDMA interrupt source by reading the interrupt_mask register and matching the expected GDMA interrupt value. The DMA channel completion status must be non-zero when read from the INTMIS register. Each channel interrupt must be successfully cleared via the INTCLR register, and the subsystem interrupt must be cleared via the interrupt_raw register. The pending channel bitmask must reach zero indicating both channels completed. The test fails if any unexpected interrupt source is detected at the subsystem level, if the DMA masked interrupt status is zero during an expected GDMA interrupt, or if any error increments the error counter. The final result is reported as pass only when the accumulated error count is zero.",
        "Remarks": "The test uses an interrupt-driven model with a polling loop in the main function waiting on int_pend1 bitmask. Two DMA channels are used: CH0 for pixel data transfer and CH1 for DBI write command transfer. DMA microcode is programmed in SRAM using DMAMOV, DMAWMB, DMASEV, and DMAEND instructions. The DMA is started via the debug instruction interface (DBGINST0/DBGINST1/DBGCMD). The test relies on external functions phy_init(), dbi_config(), pixel_to_bytes_wr_cmd_size(), program_dpi_clock(), load_rand_data(), and load_wr_command() whose implementations are not in the supplied testcase folder. DBI mode is selected by writing 0 to the dpi_control register."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_dbi_random_payload_test",
        "Feature": "DBI Random Payload Data Transfer with DMA",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_dsi.h"; <math.h>',
        "Meta Macros": "NA",
        "Meta Arrays": "data_tdbdcb[10]; data_rebdcb[10]; cmd_tdbdcb[10]; cmd_rebdcb[10]; wr_cmd_arr[27]; WR_CMD_SIZE[27]",
        "Speed": "NA",
        "Mode": "DBI Mode",
        "Memory Start Offset": "0x0000; 0x10000; 0x3F000; 0x3F800",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs MIPI DSI DBI data transfers with randomized pixel payload sizes using the integrated DMA controller (DMA330). It configures phy_stop_wait_time=0x40 and n_lanes=3. It constructs phy_if_cfg using n_lanes and set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field and writes to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. It writes MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with 0x3d and MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with 0x107. It writes MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL with 0 to enable DBI mode. It calls phy_init() for PHY initialization. It configures DBI parameters: dbi_vcid=0x3, load_cmd_or_data_to_sram=1, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x0, allowed_cmd_size=0x25, initial wr_cmd_size=193. It sets num_of_pixel=40 initially and calls pixel_to_bytes_wr_cmd_size(40) to compute wr_cmd_size and num_bytes. It sets all DSI command type flags to 0x0, tear_fx_en=0x1, generic_vc_id=0x2. It programs DPI clock with dpi_clk_time_period=16.012400ns. It calls dbi_config() to apply initial DBI configuration. It writes MIZAR_MIPI_DSI_DMAC_INTEN with 0x3 to enable DMA interrupts for channels 0 and 1. It then enters a loop of 10 iterations. In each iteration: num_of_pixel is randomized as ((rand() % 1024)) * 8, pixel_to_bytes_wr_cmd_size() recomputes wr_cmd_size and num_bytes, dbi_config() is called again with updated parameters. DMA descriptors are set up for 1 descriptor: CH0 (data) uses data_tdbdcb with addr=RAM_BASE+0x10000 and data_rebdcb with addr=0x10000000000; CH1 (command) uses cmd_tdbdcb with addr=RAM_BASE+0x0000 and cmd_rebdcb with addr=0x10000008000. For each channel it programs DMAMOV for SAR/DAR, program_data_num_bytes for transfer length (CH0 uses ceil(len/8)*8), DMAWMB, DMASEV, and DMAEND instructions. It loads random data via load_rand_data() and write command via load_wr_command() with DSI_WRITE_MEMORY_START. It starts DMA CH0 by writing MIZAR_MIPI_DSI_DMAC_DBGINST0=0x00A00000, MIZAR_MIPI_DSI_DMAC_DBGINST1=ch0_desc_addr_act, MIZAR_MIPI_DSI_DMAC_DBGCMD=0x0. It starts DMA CH1 similarly with DBGINST0=0x01A00000 and DBGINST1=ch1_desc_addr_act. It polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until rd_data equals 0x3 (both channels complete). It then writes MIZAR_MIPI_DSI_DMAC_INTCLR with rd_data to clear interrupts. It calls wait_on(100000) before proceeding to the next iteration. After all 10 iterations, finish(0) is called.",
        "Test Description": "This test validates MIPI DSI DBI data transfers with randomized pixel payload sizes using the integrated DMA controller. It configures the DSI host PHY interface with 3 lanes and a stop wait time, sets up packet handling and clock manager registers, and enables DBI mode by disabling DPI control. After PHY and DBI configuration, the test enters a loop of 10 iterations. In each iteration, a random pixel count (multiples of 8, up to 8192) is generated, the DBI configuration is updated with the new payload size, DMA descriptors are prepared for two channels (one for pixel data and one for DBI write commands), and both DMA channels are started via the debug instruction registers. The test polls the DMA masked interrupt status register until both channels report completion, then clears the interrupts via the interrupt clear register. After all 10 iterations complete successfully, the test reports pass.",
        "Meta Test Steps / Procedure": "1. Entry: test_case() is called. 2. phy_stop_wait_time is set to 0x40. 3. n_lanes is set to 3. 4-81. (Full PHY config, DBI config, DMA setup, polling, and iteration steps as documented).",
        "Test Steps / Procedure": "1. Configure the PHY interface with 3 lanes and a stop wait time by writing to the PHY_IF_CFG register. 2. Configure packet handling by writing to the PCKHDL_CFG register. 3. Configure the clock manager by writing to the CLKMGR_CFG register. 4. Disable DPI control to enable DBI mode by writing to the dpi_control register. 5. Initialize the DSI PHY. 6. Configure DBI parameters including virtual channel ID, LUT size, DBI input/output configuration, partitioning disabled, and allowed command size. 7. Compute initial write command size and byte count from pixel count (40 pixels). 8. Program the DPI clock frequency. 9. Apply initial DBI configuration. 10. Enable DMA interrupts for channels 0 and 1 by writing to the INTEN register. 11. Begin iterating 10 times with randomized pixel payload sizes (multiples of 8, up to 8192 pixels). 12. In each iteration, recompute write command size and byte count from the random pixel count. 13. Reapply DBI configuration with the updated payload size. 14. Set up DMA descriptor memory regions for CH0 (data) and CH1 (command). 15. Program DMA microcode instructions for both channels including source address, destination address, transfer length (8-byte aligned for data channel), write memory barrier, send event, and end. 16. Load random pixel data into SRAM for the data channel. 17. Load the DBI write memory start command into SRAM for the command channel. 18. Start DMA CH0 by writing the GO debug instruction via DBGINST0, DBGINST1, and DBGCMD registers. 19. Start DMA CH1 by writing the GO debug instruction via DBGINST0, DBGINST1, and DBGCMD registers. 20. Poll the INTMIS register until both DMA channels report completion (value equals expected bitmask). 21. Clear DMA interrupts by writing to the INTCLR register. 22. Wait for a settling delay before the next iteration. 23. After all 10 iterations complete, report test pass.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; INTEN; DBGINST0; DBGINST1; DBGCMD; INTMIS; INTCLR",
        "Meta Validation / Acceptance Criteria": "The test validates via polling-based completion. In each of the 10 iterations, after starting both DMA channels, rd_data is read from MIZAR_MIPI_DSI_DMAC_INTMIS and compared against 0x3 in a while loop. The loop continues polling until rd_data equals 0x3, indicating both CH0 (bit 0) and CH1 (bit 1) have completed. Once rd_data == 0x3, MIZAR_MIPI_DSI_DMAC_INTCLR is written with rd_data to clear the DMA interrupts. After all 10 iterations complete without hanging in the polling loop, finish(0) is called with a hardcoded 0, indicating unconditional pass if all polling loops complete. The test implicitly validates that the DMA controller can handle varying random payload sizes across multiple iterations without stalling.",
        "Validation / Acceptance Criteria": "The test passes when all 10 iterations of randomized DBI data transfers complete successfully. In each iteration, the INTMIS register must report completion for both DMA channels (both channel bits set). The polling loop must not hang, confirming the DMA controller successfully transferred the randomized payload. After polling confirms completion, the interrupts must be successfully cleared via the INTCLR register. The test reports pass unconditionally after all iterations complete, implicitly validating that the DMA and DBI subsystem can handle varying random payload sizes across multiple consecutive transfers without stalling or errors.",
        "Remarks": "This test uses a polling-based completion model (not interrupt-driven) by directly polling the INTMIS register in a while loop. Unlike the basic test, this test does not use GIC interrupts or an ISR. The test runs 10 iterations with randomized pixel counts computed as ((rand() % 1024)) * 8, producing multiples of 8 up to 8184 pixels. The data channel transfer length is 8-byte aligned using ceil(len/8)*8. The test always calls finish(0) with a hardcoded pass value, meaning it does not detect data corruption - it only validates that DMA transfers complete. The test relies on external functions phy_init(), dbi_config(), pixel_to_bytes_wr_cmd_size(), program_dpi_clock(), load_rand_data(), and load_wr_command() whose implementations are not in the supplied testcase folder. DBI mode is selected by writing 0 to the dpi_control register. Partitioning is disabled (partitioning_en=0x0) unlike the basic test."
    }
]

# TestPlan sheet columns
testplan_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# MetaData sheet columns
metadata_columns = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Header formatting
header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
header_alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
cell_alignment = Alignment(vertical="top", wrap_text=True)
thin_border = Border(
    left=Side(style="thin"),
    right=Side(style="thin"),
    top=Side(style="thin"),
    bottom=Side(style="thin")
)

# Write TestPlan headers
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, "")
        if value is None:
            value = ""
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns with max width
for col_idx in range(1, len(testplan_columns) + 1):
    max_length = len(str(ws_tp.cell(row=1, column=col_idx).value or ""))
    for row_idx in range(2, len(json_data) + 2):
        cell_value = str(ws_tp.cell(row=row_idx, column=col_idx).value or "")
        # Use first 100 chars for width estimation
        max_length = max(max_length, min(len(cell_value), 100))
    adjusted_width = min(max_length + 2, 60)
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = max(adjusted_width, 12)

# --- MetaData Sheet ---
ws_md = wb.create_sheet(title="MetaData")

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, "")
        if value is None:
            value = ""
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_md.freeze_panes = "A2"

# Auto-size MetaData columns
for col_idx in range(1, len(metadata_columns) + 1):
    max_length = len(str(ws_md.cell(row=1, column=col_idx).value or ""))
    for row_idx in range(2, len(json_data) + 2):
        cell_value = str(ws_md.cell(row=row_idx, column=col_idx).value or "")
        max_length = max(max_length, min(len(cell_value), 100))
    adjusted_width = min(max_length + 2, 60)
    ws_md.column_dimensions[get_column_letter(col_idx)].width = max(adjusted_width, 12)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save workbook
output_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(output_dir, filename)
wb.save(output_path)

# Validate
try:
    wb_check = load_workbook(output_path)
    assert "TestPlan" in wb_check.sheetnames
    assert "MetaData" in wb_check.sheetnames
    file_size = os.path.getsize(output_path)
    assert file_size > 0
    print(f"SUCCESS: {filename}")
    print(f"Path: {output_path}")
    print(f"Size: {file_size} bytes")
    print(f"Sheets: {wb_check.sheetnames}")
    print(f"TestPlan rows: {ws_tp.max_row - 1}")
    print(f"MetaData rows: {ws_md.max_row - 1}")
except Exception as e:
    print(f"VALIDATION FAILED: {e}")
    sys.exit(1)
