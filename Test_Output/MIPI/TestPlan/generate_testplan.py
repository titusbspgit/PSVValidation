#!/usr/bin/env python3
"""
Agent 7 - Excel Generator Script
Generates MIPI_DSI TestPlan Excel workbook using openpyxl
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import os
import base64
import json

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_DSI_TestPlan_{timestamp}.xlsx"

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "NA",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase configures the MIPI DSI Host, Subsystem, and DMAC blocks. It writes to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA interrupts, writes to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable subsystem-level GDMA interrupts, writes to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the PHY interface, writes to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling, writes to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager, writes to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to configure DPI control, writes to MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and MIZAR_MIPI_DSI_DMAC_DBGCMD to issue DMA channel instructions via the debug interface. In the interrupt service routine, it reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and MIZAR_MIPI_DSI_DMAC_INTMIS to identify interrupt sources, then writes to MIZAR_MIPI_DSI_DMAC_INTCLR and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear the interrupts.",
        "Test Description": "This testcase configures the MIPI DSI Host, Subsystem, and DMA Controller blocks for a basic DSI transfer operation. It configures the PHY interface via PHY_IF_CFG, sets up packet handling via PCKHDL_CFG, configures the clock manager via CLKMGR_CFG, configures DPI control via dpi_control, enables DMA and subsystem interrupts via interrupt_enable and DMAC interrupt enable registers, and initiates DMA channel operations via DMAC debug instruction registers. An interrupt service routine reads the interrupt_mask and DMAC masked interrupt status to identify interrupt sources, then clears interrupts by writing to the DMAC interrupt clear register and interrupt_raw.",
        "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_DSI_DMAC_INTEN with value 0x3 to enable DMA channel interrupts. 2. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR to enable GDMA interrupt at subsystem level. 3. Configure PHY interface by computing phy_if_cfg using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME and writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. 4. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling. 5. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager. 6. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to configure DPI control. 7. Write DMA channel 0 descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and execute via MIZAR_MIPI_DSI_DMAC_DBGCMD. 8. Write DMA channel 1 descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and execute via MIZAR_MIPI_DSI_DMAC_DBGCMD. 9. In ISR (Default_IRQHandler): read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to get subsystem interrupt mask status. 10. If GDMA interrupt is indicated, read MIZAR_MIPI_DSI_DMAC_INTMIS to get DMA channel masked interrupt status. 11. Write channel mask to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMA interrupt. 12. Write subsystem mask to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem interrupt.",
        "Test Steps / Procedure": "1. Enable DMA channel interrupts by writing to the DMAC interrupt enable register. 2. Enable GDMA interrupt at the subsystem level by writing to the interrupt_enable register. 3. Configure the DSI Host PHY interface parameters by writing to the PHY_IF_CFG register. 4. Configure packet handling behavior by writing to the PCKHDL_CFG register. 5. Configure the clock manager by writing to the CLKMGR_CFG register. 6. Disable DPI control by writing to the dpi_control register. 7. Issue DMA channel 0 transfer instructions via the DMAC debug instruction registers and execute the command. 8. Issue DMA channel 1 transfer instructions via the DMAC debug instruction registers and execute the command. 9. On interrupt, read the interrupt_mask register to identify the subsystem interrupt source. 10. If GDMA interrupt is active, read the DMAC masked interrupt status register to identify the DMA channel source. 11. Clear the DMA interrupt by writing to the DMAC interrupt clear register. 12. Clear the subsystem interrupt by writing to the interrupt_raw register.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; interrupt_mask; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "The ISR reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and checks if the value matches MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR to confirm the GDMA interrupt source. It then reads MIZAR_MIPI_DSI_DMAC_INTMIS to identify which DMA channel triggered the interrupt. Interrupts are cleared by writing to MIZAR_MIPI_DSI_DMAC_INTCLR and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. Successful completion implies that the DMA transfer completes and the interrupt is serviced and cleared without errors.",
        "Validation / Acceptance Criteria": "The interrupt service routine must correctly identify the GDMA interrupt source by reading the interrupt_mask register. The DMAC masked interrupt status must be read to determine the triggering DMA channel. DMA interrupts must be successfully cleared via the DMAC interrupt clear register. Subsystem interrupts must be successfully cleared via the interrupt_raw register. The test passes if the DMA transfer completes and all interrupts are properly serviced and cleared.",
        "Remarks": "Six DMAC registers (interrupt enable, masked interrupt status, interrupt clear, debug instruction 0, debug instruction 1, debug command) could not be mapped to canonical register names because the DMAC (PL330) register specification was not provided. Only DSI Host and DSI Subsystem register specifications were available. The source code in the repository folder does not contain the expected MIPI DSI testcase source; Agent 2, Agent 3, and Agent 4 outputs were used as the primary source for register-related fields."
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
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria", "Meta Headers",
    "Meta Macros", "Meta Arrays"
]

# Create workbook
wb = openpyxl.Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# Write headers
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns
for col_idx, col_name in enumerate(testplan_columns, 1):
    max_len = len(col_name)
    for row in ws_tp.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
        for cell in row:
            if cell.value:
                max_len = max(max_len, min(len(str(cell.value)), 80))
    adjusted_width = min(max_len + 4, 60)
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

# --- MetaData Sheet ---
ws_md = wb.create_sheet("MetaData")

# Write headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_md.freeze_panes = "A2"

# Auto-size columns
for col_idx, col_name in enumerate(metadata_columns, 1):
    max_len = len(col_name)
    for row in ws_md.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
        for cell in row:
            if cell.value:
                max_len = max(max_len, min(len(str(cell.value)), 80))
    adjusted_width = min(max_len + 4, 60)
    ws_md.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save workbook
output_path = filename
wb.save(output_path)

# Validate
wb2 = openpyxl.load_workbook(output_path)
assert "TestPlan" in wb2.sheetnames
assert "MetaData" in wb2.sheetnames
file_size = os.path.getsize(output_path)
assert file_size > 0

# Output base64 for upload
with open(output_path, "rb") as f:
    b64_content = base64.b64encode(f.read()).decode("utf-8")

result = {
    "filename": filename,
    "file_size": file_size,
    "sheets": wb2.sheetnames,
    "testplan_rows": ws_tp.max_row - 1,
    "metadata_rows": ws_md.max_row - 1,
    "validation": "PASSED",
    "base64_length": len(b64_content)
}

print(json.dumps(result, indent=2))
print("BASE64_START")
print(b64_content)
print("BASE64_END")
