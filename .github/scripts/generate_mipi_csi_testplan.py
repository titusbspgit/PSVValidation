#!/usr/bin/env python3
"""Generate MIPI_CSI TestPlan Excel workbook using openpyxl."""
import json
import os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# IST timezone offset
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_CSI_TestPlan_{timestamp_str}.xlsx"
output_dir = "Test_Output/MIPI/TestPlan"
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# Input JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "NA",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs a basic MIPI DSI test involving DMA controller interrupt enable, subsystem interrupt enable, DSI host PHY interface configuration, packet handler configuration, clock manager configuration, DPI control configuration, DMA debug instruction programming and execution, subsystem interrupt mask read-back, DMA interrupt status read-back, DMA interrupt clear, and subsystem interrupt raw status clear. The test writes to MIZAR_MIPI_DSI_DMAC_INTEN, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, MIZAR_MIPI_DSI_DMAC_DBGCMD, MIZAR_MIPI_DSI_DMAC_INTCLR, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. It reads from MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and MIZAR_MIPI_DSI_DMAC_INTMIS.",
        "Test Description": "This testcase performs a basic MIPI DSI validation by enabling DMA controller interrupts, enabling subsystem-level interrupts, configuring the DSI host PHY interface, configuring the packet handler, configuring the clock manager, setting DPI control, programming DMA debug instructions and executing a DMA command, reading back the subsystem interrupt mask and DMA interrupt status, clearing DMA interrupts, and clearing the subsystem interrupt raw status.",
        "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA controller interrupts. 2. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable subsystem-level interrupts. 3. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the DSI host PHY interface. 4. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure the packet handler. 5. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager. 6. Write to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to set DPI control. 7. Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 to program DMA debug instruction 0. 8. Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 to program DMA debug instruction 1. 9. Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA debug command. 10. Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to verify subsystem interrupt mask status. 11. Read MIZAR_MIPI_DSI_DMAC_INTMIS to check DMA masked interrupt status. 12. Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMA interrupts. 13. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem interrupt raw status.",
        "Test Steps / Procedure": "1. Enable DMA controller interrupts by writing to the DMA interrupt enable register. 2. Enable subsystem-level interrupts by writing to the subsystem interrupt enable register. 3. Configure the DSI host PHY interface by writing to the PHY interface configuration register. 4. Configure the packet handler by writing to the packet handler configuration register. 5. Configure the clock manager by writing to the clock manager configuration register. 6. Set DPI control by writing to the DPI control register. 7. Program DMA debug instruction 0 by writing to the DMA debug instruction 0 register. 8. Program DMA debug instruction 1 by writing to the DMA debug instruction 1 register. 9. Execute the DMA debug command by writing to the DMA debug command register. 10. Read back the subsystem interrupt mask register to verify interrupt mask status. 11. Read the DMA masked interrupt status register to check DMA interrupt status. 12. Clear DMA interrupts by writing to the DMA interrupt clear register. 13. Clear subsystem interrupt raw status by writing to the subsystem interrupt raw register.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "NA",
        "Validation / Acceptance Criteria": "NA",
        "Remarks": "All 13 register-access macros are MIPI DSI macros which could not be resolved by Agent 3 (no DSI header definitions available) and could not be mapped to canonical register names by Agent 4 (no DSI register specifications available). The actual MIPI DSI test source code was not accessible in the testcase folder; source-derived fields are marked NA accordingly."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_dsi_dbi_random_payload_test",
        "Feature": "NA",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs a MIPI DSI DBI random payload transfer test. It configures the DSI host PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, configures the packet handler by writing to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, configures the clock manager by writing to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, disables DPI control by writing to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, enables DMA controller interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTEN, programs DMA debug instructions by writing to MIZAR_MIPI_DSI_DMAC_DBGINST0 and MIZAR_MIPI_DSI_DMAC_DBGINST1, executes the DMA debug command by writing to MIZAR_MIPI_DSI_DMAC_DBGCMD, polls the DMA masked interrupt status by reading MIZAR_MIPI_DSI_DMAC_INTMIS, and clears DMA interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTCLR.",
        "Test Description": "This testcase validates MIPI DSI DBI interface random payload transfer. It configures the DSI host PHY interface, packet handler, and clock manager. It disables DPI control to operate in DBI mode. It enables DMA controller interrupts, programs DMA debug instructions for a DMA transfer, executes the DMA command, polls the DMA masked interrupt status register to wait for transfer completion, and clears the DMA interrupts after completion.",
        "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the DSI host PHY interface. 2. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure the packet handler. 3. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager. 4. Write to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control for DBI mode operation. 5. Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA controller interrupts. 6. Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 to program DMA debug instruction 0. 7. Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 to program DMA debug instruction 1. 8. Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA debug command. 9. Read MIZAR_MIPI_DSI_DMAC_INTMIS to poll DMA masked interrupt status for transfer completion. 10. Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMA interrupts after completion.",
        "Test Steps / Procedure": "1. Configure the DSI host PHY interface by writing to the PHY interface configuration register. 2. Configure the packet handler by writing to the packet handler configuration register. 3. Configure the clock manager by writing to the clock manager configuration register. 4. Disable DPI control by writing to the DPI control register to operate in DBI mode. 5. Enable DMA controller interrupts by writing to the DMA interrupt enable register. 6. Program DMA debug instruction 0 by writing to the DMA debug instruction 0 register. 7. Program DMA debug instruction 1 by writing to the DMA debug instruction 1 register. 8. Execute the DMA debug command by writing to the DMA debug command register. 9. Poll the DMA masked interrupt status register to wait for DMA transfer completion. 10. Clear DMA interrupts by writing to the DMA interrupt clear register.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_DMAC_INTMIS to detect DMA transfer completion. Upon detecting the interrupt, it clears the interrupt via MIZAR_MIPI_DSI_DMAC_INTCLR. The test passes if the DMA transfer completes and the interrupt is successfully detected and cleared.",
        "Validation / Acceptance Criteria": "The test passes if the DMA masked interrupt status register indicates successful DMA transfer completion after polling, and the DMA interrupt is successfully cleared via the DMA interrupt clear register.",
        "Remarks": "All 10 register-access macros are MIPI DSI macros which could not be resolved by Agent 3 (no DSI header definitions available in the knowledge base) and could not be mapped to canonical register names by Agent 4 (no DSI register specifications available). The actual MIPI DSI DBI random payload test source code was not accessible in the testcase folder; the main.c found is an unrelated STM32 motor controller file. Source-derived fields are marked NA accordingly. The DMA masked interrupt status register is polled in a loop to wait for transfer completion."
    },
    {
        "Index": "3",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "NA",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs a MIPI DSI subsystem register write-read verification test. It targets 5 DSI subsystem registers using read-modify-write operations: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. Each register undergoes a write followed by a read-back to verify the written value was correctly stored.",
        "Test Description": "This testcase validates the write and read-back functionality of MIPI DSI subsystem registers. It performs read-modify-write operations on five DSI subsystem registers covering data FIFO threshold configuration, low power mode configuration, DBI timing configuration, DBI frequency divider configuration, and interrupt raw status. For each register, a value is written and then read back to confirm correct register accessibility and data integrity.",
        "Meta Test Steps / Procedure": "1. Perform read-modify-write on MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL to verify data FIFO threshold register accessibility. 2. Perform read-modify-write on MIZAR_MIPI_DSI_SUBSYS_LOW_PWR to verify low power register accessibility. 3. Perform read-modify-write on MIZAR_MIPI_DSI_SUBSYS_DBITE to verify DBI timing register accessibility. 4. Perform read-modify-write on MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV to verify DBI frequency divider register accessibility. 5. Perform read-modify-write on MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to verify interrupt raw status register accessibility.",
        "Test Steps / Procedure": "1. Write a test value to the data FIFO threshold register and read it back to verify correct storage. 2. Write a test value to the low power configuration register and read it back to verify correct storage. 3. Write a test value to the DBI timing configuration register and read it back to verify correct storage. 4. Write a test value to the DBI frequency divider register and read it back to verify correct storage. 5. Write a test value to the interrupt raw status register and read it back to verify correct storage.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "For each of the 5 registers, the value read back after writing must match the written value, accounting for applicable read and write masks. If any read-back value does not match the expected value, the test fails for that register.",
        "Validation / Acceptance Criteria": "The test passes if all five DSI subsystem registers return the expected values upon read-back after write operations. Each register's read-back data must match the written data within the applicable read and write mask constraints. Any mismatch indicates a register access failure.",
        "Remarks": "All 5 register-access macros are MIPI DSI subsystem macros which could not be resolved by Agent 3 (no DSI header definitions available in the knowledge base) and could not be mapped to canonical register names by Agent 4 (no DSI register specifications available). The actual MIPI DSI subsystem register write-read test source code was not accessible in the testcase folder; the program.c found is an unrelated PCIe device enumeration test and the test_define.c is an unrelated PCIe definitions file. Source-derived fields are marked NA accordingly."
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
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Header formatting
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# Write TestPlan headers
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = "A2"

# --- MetaData Sheet ---
ws_md = wb.create_sheet(title="MetaData")

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_md.freeze_panes = "A2"

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Auto-size columns for both sheets
for ws in [ws_tp, ws_md]:
    for col_cells in ws.columns:
        max_length = 0
        col_letter = get_column_letter(col_cells[0].column)
        for cell in col_cells:
            try:
                if cell.value:
                    max_length = max(max_length, len(str(cell.value)))
            except Exception:
                pass
        # Cap at 60 for readability
        adjusted_width = min(max(max_length + 2, 12), 60)
        ws.column_dimensions[col_letter].width = adjusted_width

# Save workbook
wb.save(output_path)
print(f"SUCCESS: Workbook saved to {output_path}")
print(f"Filename: {filename}")
print(f"Rows TestPlan: {len(json_data)}")
print(f"Rows MetaData: {len(json_data)}")
