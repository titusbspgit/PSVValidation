#!/usr/bin/env python3
"""Standalone XLSX generator for MIPI_DSI TestPlan - executed locally or via GitHub Actions."""
import os, sys, base64
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_DSI_TestPlan_{timestamp}.xlsx'

json_data = [
  {
    "Index": "1",
    "SS / Module": "MIPI_DSI",
    "Test Case Name": "mipi_dsi_basic_test",
    "Feature": "DBI Command Mode Transfer via DMAC",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This testcase validates a basic MIPI DSI DBI command mode data transfer using the DMA controller. It enables DMAC and subsystem interrupts, configures the DSI host PHY interface, packet handling, and clock manager registers, disables DPI control for command mode operation, programs and executes a DMA transfer via DMAC debug instruction registers, polls the subsystem interrupt mask and DMAC interrupt status registers for transfer completion, and then clears both DMAC and subsystem interrupt flags.",
    "Test Steps / Procedure": "1. Enable DMA controller channel interrupts by writing to the INTEN register.\n2. Enable the GDMA interrupt in the subsystem interrupt_enable register.\n3. Configure the DSI host PHY interface parameters by writing to the PHY_IF_CFG register.\n4. Configure packet handling behavior by writing to the PCKHDL_CFG register.\n5. Configure the clock manager settings by writing to the CLKMGR_CFG register.\n6. Disable DPI control for command mode operation by writing to the dpi_control register.\n7. Program the first DMA debug instruction word by writing to the DBGINST0 register.\n8. Program the second DMA debug instruction word by writing to the DBGINST1 register.\n9. Execute the DMA debug command to initiate the data transfer by writing to the DBGCMD register.\n10. Poll the subsystem interrupt_mask register to detect transfer completion interrupt.\n11. Read the DMAC INTMIS register to verify the DMA interrupt status.\n12. Clear the DMA interrupt by writing to the INTCLR register.\n13. Clear the subsystem interrupt by writing to the interrupt_raw register.",
    "Impacted Registers": "INTEN; interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; DBGINST0; DBGINST1; DBGCMD; interrupt_mask; INTMIS; INTCLR; interrupt_raw",
    "Validation / Acceptance Criteria": "The test passes when the subsystem interrupt_mask register indicates the GDMA interrupt is asserted after the DMA transfer completes. The DMAC INTMIS register must reflect the expected masked interrupt status. Both the DMAC interrupt (via INTCLR) and subsystem interrupt (via interrupt_raw) must be successfully cleared. Failure to detect the interrupt or inability to clear it indicates a test failure.",
    "Remarks": "This testcase exercises the MIPI DSI DBI command mode path using the integrated DMA controller. It involves three register blocks: DSI Host (PHY_IF_CFG, PCKHDL_CFG, CLKMGR_CFG), DSI Subsystem (interrupt_enable, interrupt_mask, interrupt_raw, dpi_control), and DMAC (INTEN, INTMIS, INTCLR, DBGINST0, DBGINST1, DBGCMD). The source files in the repository folder did not contain matching MIPI DSI code; the testcase details are derived from upstream Agent 2, Agent 3, and Agent 4 outputs.",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA",
    "Meta Test Description": "This testcase performs a basic MIPI DSI DBI command mode transfer using the DMAC. It writes to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC interrupts, writes to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable subsystem-level GDMA interrupt, configures the DSI host PHY interface via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, configures packet handling via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, configures clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, disables DPI control via MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, programs DMAC debug instructions via MIZAR_MIPI_DSI_DMAC_DBGINST0 and MIZAR_MIPI_DSI_DMAC_DBGINST1, executes the DMAC debug command via MIZAR_MIPI_DSI_DMAC_DBGCMD, polls MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and MIZAR_MIPI_DSI_DMAC_INTMIS for interrupt status, clears DMAC interrupt via MIZAR_MIPI_DSI_DMAC_INTCLR, and clears subsystem interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.",
    "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_DSI_DMAC_INTEN with value 0x3 to enable DMAC channel interrupts.\n2. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable the GDMA interrupt bit in the subsystem.\n3. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the PHY interface (set PHY_STOP_WAIT_TIME field using set_data_mask).\n4. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling.\n5. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager.\n6. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control for command mode.\n7. Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 with the first debug instruction word (DMA channel thread and instruction encoding).\n8. Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 with the second debug instruction word.\n9. Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the debug instruction and start the DMA transfer.\n10. Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to poll for subsystem interrupt assertion.\n11. Read MIZAR_MIPI_DSI_DMAC_INTMIS to check DMAC masked interrupt status.\n12. Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMAC interrupt.\n13. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear the subsystem raw interrupt.",
    "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
    "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to check that the GDMA interrupt bit is asserted after the DMA transfer. It reads MIZAR_MIPI_DSI_DMAC_INTMIS to verify the DMAC masked interrupt status indicates transfer completion. After successful interrupt detection, MIZAR_MIPI_DSI_DMAC_INTCLR is written to clear the DMAC interrupt and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is written to clear the subsystem interrupt. The test passes if the interrupt is detected and cleared successfully."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_DSI",
    "Test Case Name": "mipi_dsi_dbi_random_payload_test",
    "Feature": "DBI Random Payload Transfer via DMAC",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This testcase validates MIPI DSI DBI random payload data transfers using the DMA controller across 10 iterations. It configures the DSI host PHY interface, packet handling, and clock manager registers, disables DPI control for command mode operation, and enables DMAC interrupts. In each iteration, random payload data is generated, the DMAC is programmed and executed via debug instruction registers, the DMAC interrupt status register is polled for transfer completion, and the interrupt is cleared before proceeding to the next iteration.",
    "Test Steps / Procedure": "1. Configure the DSI host PHY interface parameters by writing to the PHY_IF_CFG register.\n2. Configure packet handling behavior by writing to the PCKHDL_CFG register.\n3. Configure the clock manager settings by writing to the CLKMGR_CFG register.\n4. Disable DPI control for command mode operation by writing to the dpi_control register.\n5. Enable DMA controller channel interrupts by writing to the INTEN register.\n6. Begin a 10-iteration loop generating random payload data for each iteration.\n7. Program the first DMA debug instruction word by writing to the DBGINST0 register.\n8. Program the second DMA debug instruction word by writing to the DBGINST1 register.\n9. Execute the DMA debug command to initiate the data transfer by writing to the DBGCMD register.\n10. Poll the INTMIS register until the expected DMA transfer completion interrupt status is detected.\n11. Clear the DMA interrupt by writing to the INTCLR register.\n12. Repeat steps 6 through 11 for all 10 iterations to validate random payload transfers.",
    "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; INTEN; DBGINST0; DBGINST1; DBGCMD; INTMIS; INTCLR",
    "Validation / Acceptance Criteria": "For each of the 10 iterations, the INTMIS register must indicate DMA transfer completion with the expected interrupt status value. The INTCLR register must successfully clear the interrupt after each iteration. The test passes if all 10 random payload transfer iterations complete successfully with the expected interrupt status detected and cleared each time. Failure to detect the expected interrupt status or inability to clear the interrupt in any iteration indicates a test failure.",
    "Remarks": "This testcase exercises the MIPI DSI DBI command mode path with random payload data across 10 iterations using the integrated DMA controller. It involves two register blocks: DSI Host (PHY_IF_CFG, PCKHDL_CFG, CLKMGR_CFG), DSI Subsystem (dpi_control), and DMAC (INTEN, INTMIS, INTCLR, DBGINST0, DBGINST1, DBGCMD). Unlike the basic test, this testcase does not use subsystem-level interrupt enable, mask, or raw registers. The polling-based completion check uses the DMAC INTMIS register directly. The source files in the repository folder did not contain matching MIPI DSI code; the testcase details are derived from upstream Agent 2, Agent 3, and Agent 4 outputs.",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA",
    "Meta Test Description": "This testcase performs MIPI DSI DBI random payload transfers using the DMAC in a 10-iteration loop. It configures the DSI host PHY interface via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG (write), configures packet handling via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG (write), configures the clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG (write), disables DPI control via MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL (write), enables DMAC interrupts via MIZAR_MIPI_DSI_DMAC_INTEN (write). In each iteration of the 10-iteration loop, random payload data is generated, the DMAC is programmed via MIZAR_MIPI_DSI_DMAC_DBGINST0 (write) and MIZAR_MIPI_DSI_DMAC_DBGINST1 (write), the transfer is initiated via MIZAR_MIPI_DSI_DMAC_DBGCMD (write), MIZAR_MIPI_DSI_DMAC_INTMIS is polled for DMA completion, and MIZAR_MIPI_DSI_DMAC_INTCLR is written to clear the interrupt before the next iteration.",
    "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the PHY interface (set PHY_STOP_WAIT_TIME field).\n2. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling.\n3. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager.\n4. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control for command mode.\n5. Write to MIZAR_MIPI_DSI_DMAC_INTEN with value 0x3 to enable DMAC channel interrupts.\n6. Begin 10-iteration loop: generate random payload data for the current iteration.\n7. Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 with the first debug instruction word (DMA channel thread and instruction encoding).\n8. Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 with the second debug instruction word (descriptor address).\n9. Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the debug instruction and start the DMA transfer.\n10. Poll MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until the expected interrupt status value (0x3) is detected, indicating DMA transfer completion.\n11. Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMAC interrupt.\n12. End of loop iteration; repeat steps 6-11 for all 10 iterations.",
    "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
    "Meta Validation / Acceptance Criteria": "In each of the 10 iterations, MIZAR_MIPI_DSI_DMAC_INTMIS is polled in a while loop until the read value equals 0x3, indicating that both DMA channels have completed the transfer. After successful detection, MIZAR_MIPI_DSI_DMAC_INTCLR is written to clear the interrupt. The test passes if all 10 iterations complete successfully with the expected interrupt status detected and cleared each time."
  }
]

testplan_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

metadata_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

wb = Workbook()
ws_tp = wb.active
ws_tp.title = 'TestPlan'

header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, '')
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

ws_tp.freeze_panes = 'A2'

for col_idx, col_name in enumerate(testplan_columns, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        val = ws_tp.cell(row=row_idx, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 60)

ws_md = wb.create_sheet('MetaData')

for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, '')
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

ws_md.freeze_panes = 'A2'

for col_idx, col_name in enumerate(metadata_columns, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        val = ws_md.cell(row=row_idx, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_md.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 60)

ws_md.sheet_state = 'veryHidden'

output_path = os.path.join('/tmp', filename)
wb.save(output_path)

# Validate
wb2 = load_workbook(output_path)
assert 'TestPlan' in wb2.sheetnames
assert 'MetaData' in wb2.sheetnames
file_size = os.path.getsize(output_path)
assert file_size > 0

# Output base64 for GitHub push
with open(output_path, 'rb') as f:
    b64 = base64.b64encode(f.read()).decode('utf-8')

print(f'FILENAME={filename}')
print(f'FILE_SIZE={file_size}')
print(f'VALIDATION=PASSED')
print(f'BASE64_START')
print(b64)
print(f'BASE64_END')
