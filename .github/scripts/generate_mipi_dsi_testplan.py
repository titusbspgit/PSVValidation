#!/usr/bin/env python3
import os
import json
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_DSI_TestPlan_{timestamp}.xlsx'
output_dir = 'Test_Output/MIPI/TestPlan'
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# Input data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "DBI DMA Transfer with Interrupt Handling",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase configures the MIPI DSI subsystem for a DBI-mode DMA transfer with interrupt handling. It writes to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA channel interrupts, writes to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable subsystem-level GDMA interrupt, configures the PHY interface via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG (lane count and stop wait time), configures packet handling via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, configures clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, disables DPI control via MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL (set to 0 for DBI mode), issues DMA channel instructions by writing to MIZAR_MIPI_DSI_DMAC_DBGINST0 and MIZAR_MIPI_DSI_DMAC_DBGINST1 followed by executing via MIZAR_MIPI_DSI_DMAC_DBGCMD, reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to poll for subsystem interrupt status, reads MIZAR_MIPI_DSI_DMAC_INTMIS to check DMA channel interrupt status, clears DMA interrupts via MIZAR_MIPI_DSI_DMAC_INTCLR, and clears subsystem interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.",
        "Test Description": "This testcase validates a basic MIPI DSI DBI-mode DMA transfer with interrupt handling. It enables DMA channel and subsystem-level interrupts, configures the PHY interface for lane count and stop wait time via PHY_IF_CFG, sets packet handling configuration via PCKHDL_CFG, configures the clock manager via CLKMGR_CFG, disables DPI control via dpi_control to select DBI mode, issues DMA channel transfer instructions through DMAC debug instruction and command registers, polls the interrupt_mask and DMAC interrupt status registers to detect transfer completion, and clears both DMA and subsystem interrupts after completion.",
        "Meta Test Steps / Procedure": "1. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA channel 0 and channel 1 interrupts. 2. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR bit set to enable GDMA interrupt at subsystem level. 3. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure PHY interface with lane count and stop wait time using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field. 4. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling. 5. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager. 6. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control (select DBI mode). 7. Write DMA instruction bytes to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 0. 8. Write DMA instruction data to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 0. 9. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA instruction for channel 0. 10. Repeat steps 7-9 for channel 1 with appropriate instruction bytes. 11. Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to poll for GDMA interrupt assertion (check MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit). 12. Read MIZAR_MIPI_DSI_DMAC_INTMIS to determine which DMA channel triggered the interrupt. 13. Write channel mask to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA channel interrupt. 14. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR bit to clear the subsystem GDMA interrupt.",
        "Test Steps / Procedure": "1. Enable DMA channel 0 and channel 1 interrupts by writing to the DMAC interrupt enable register. 2. Enable the GDMA interrupt at the subsystem level by writing to the interrupt_enable register. 3. Configure the PHY interface with the desired lane count and stop wait time by writing to PHY_IF_CFG. 4. Configure packet handling parameters by writing to PCKHDL_CFG. 5. Configure the clock manager by writing to CLKMGR_CFG. 6. Disable DPI control to select DBI mode by writing zero to dpi_control. 7. Issue DMA transfer instructions for channel 0 by writing instruction bytes and data to the DMAC debug instruction registers and executing via the DMAC debug command register. 8. Issue DMA transfer instructions for channel 1 using the same debug instruction mechanism. 9. Poll the interrupt_mask register to detect GDMA interrupt assertion at the subsystem level. 10. Read the DMAC interrupt status register to identify which DMA channel completed. 11. Clear the DMA channel interrupt by writing to the DMAC interrupt clear register. 12. Clear the subsystem GDMA interrupt by writing to the interrupt_raw register.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; interrupt_mask; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "The test validates DMA transfer completion by polling MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK for the MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit to be asserted, indicating the GDMA interrupt has been triggered. It then reads MIZAR_MIPI_DSI_DMAC_INTMIS to confirm which DMA channel completed. After confirmation, it clears the DMA interrupt via MIZAR_MIPI_DSI_DMAC_INTCLR and the subsystem interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. The test passes if both DMA channels complete their transfers and interrupts are properly asserted and cleared.",
        "Validation / Acceptance Criteria": "The test passes when the GDMA interrupt is detected in the interrupt_mask register after DMA transfer initiation, the DMAC interrupt status register confirms the specific DMA channel completion, and both the DMA channel interrupt and subsystem GDMA interrupt are successfully cleared. Failure occurs if the GDMA interrupt is not asserted after the DMA transfer or if interrupt clearing does not succeed.",
        "Remarks": "Six DMAC registers (DMAC interrupt enable, DMAC debug instruction 0, DMAC debug instruction 1, DMAC debug command, DMAC interrupt status, DMAC interrupt clear) could not be mapped to canonical register names because no DMAC register specification document was provided. Source files in the testcase folder do not contain MIPI DSI content; testcase details are derived from upstream agent register-access analysis."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_dbi_random_payload_test",
        "Feature": "DBI Random Payload DMA Transfer",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs a MIPI DSI DBI random payload transfer using DMA. It configures the PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, sets packet handling configuration by writing to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, configures the clock manager by writing to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, disables DPI control by writing to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to select DBI mode, enables DMA channel interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTEN, issues DMA channel transfer instructions by writing to MIZAR_MIPI_DSI_DMAC_DBGINST0 and MIZAR_MIPI_DSI_DMAC_DBGINST1 and executing via MIZAR_MIPI_DSI_DMAC_DBGCMD, polls MIZAR_MIPI_DSI_DMAC_INTMIS for DMA transfer completion, and clears the DMA interrupt by writing to MIZAR_MIPI_DSI_DMAC_INTCLR. The source files in the testcase folder do not contain MIPI DSI code; details are derived from upstream agent register-access analysis.",
        "Test Description": "This testcase validates a MIPI DSI DBI random payload DMA transfer. It configures the PHY interface via PHY_IF_CFG, sets packet handling parameters via PCKHDL_CFG, configures the clock manager via CLKMGR_CFG, disables DPI control via dpi_control to select DBI mode, enables DMA channel interrupts, issues DMA transfer instructions through DMAC debug instruction and command registers, polls the DMAC interrupt status register for transfer completion, and clears the DMA interrupt after completion.",
        "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the PHY interface with lane count and stop wait time. 2. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling. 3. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager. 4. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control and select DBI mode. 5. Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA channel interrupts. 6. Write DMA instruction bytes to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 0. 7. Write DMA instruction data to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 0. 8. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA instruction for channel 0. 9. Repeat steps 6-8 for channel 1 with appropriate instruction bytes. 10. Poll MIZAR_MIPI_DSI_DMAC_INTMIS until both DMA channels report completion (expected value 0x3). 11. Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA channel interrupts.",
        "Test Steps / Procedure": "1. Configure the PHY interface with the desired lane count and stop wait time by writing to PHY_IF_CFG. 2. Configure packet handling parameters by writing to PCKHDL_CFG. 3. Configure the clock manager by writing to CLKMGR_CFG. 4. Disable DPI control to select DBI mode by writing zero to dpi_control. 5. Enable DMA channel interrupts by writing to the DMAC interrupt enable register. 6. Issue DMA transfer instructions for channel 0 by writing instruction bytes and data to the DMAC debug instruction registers and executing via the DMAC debug command register. 7. Issue DMA transfer instructions for channel 1 using the same debug instruction mechanism. 8. Poll the DMAC interrupt status register until both DMA channels report transfer completion. 9. Clear the DMA channel interrupts by writing to the DMAC interrupt clear register.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control",
        "Meta Validation / Acceptance Criteria": "The test validates DMA transfer completion by polling MIZAR_MIPI_DSI_DMAC_INTMIS until the read value equals 0x3, indicating both DMA channel 0 and channel 1 have completed their transfers. After confirmation, the DMA interrupt is cleared by writing the polled value to MIZAR_MIPI_DSI_DMAC_INTCLR. The test passes if both DMA channels complete their transfers and the interrupt status register reflects the expected completion value.",
        "Validation / Acceptance Criteria": "The test passes when the DMAC interrupt status register indicates both DMA channels have completed their transfers. The DMA channel interrupts are then successfully cleared via the DMAC interrupt clear register. Failure occurs if the DMAC interrupt status register does not reach the expected completion value indicating both channels are done.",
        "Remarks": "The source files in the testcase folder contain PCIe code and do not contain MIPI DSI content. Testcase details are derived from upstream agent register-access analysis. Six DMAC registers (DMAC interrupt enable, DMAC debug instruction 0, DMAC debug instruction 1, DMAC debug command, DMAC interrupt status, DMAC interrupt clear) could not be mapped to canonical register names because no DMAC register specification document was provided."
    },
    {
        "Index": "3",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "Subsystem Register Write-Read Verification",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs register write-read verification on a set of MIPI DSI subsystem registers. It reads the default (reset) values of each register and compares them against expected defaults. For writable registers, it writes a series of test data patterns, reads back each register, and compares the read-back value against the written value masked with the register write mask. The registers under test are MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is read-only and is skipped during write operations. The test uses addr_array, rst_val_array, rd_mask_array, wr_mask_array, and skip_array to drive the verification loop. The source files in the testcase folder do not contain MIPI DSI code; testcase details are derived from upstream agent register-access analysis and testcase naming convention.",
        "Test Description": "This testcase verifies the register write-read behavior of five MIPI DSI subsystem registers: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, and interrupt_raw. It first reads each register and validates the default reset value. Then for each writable register, it writes a series of test data patterns, reads back the register value, and compares the read-back against the expected value using the register write mask. The interrupt_raw register is read-only and is verified for its default value only, with write operations skipped.",
        "Meta Test Steps / Procedure": "1. Initialize addr_array with register address macros: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. 2. Initialize rst_val_array with expected default values, rd_mask_array with read masks, wr_mask_array with write masks, and skip_array to mark MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW as skip for write operations. 3. Execute chk_rst_val(): For each register in addr_array, call read_reg(addr) and compare the read value (masked with rd_mask) against the expected default value (masked with rd_mask). Report pass or fail for each register. 4. Execute chk_rd_wr(): For each test data pattern in the data pattern set, iterate over each register in addr_array. If skip_array entry is 0, call write_reg(addr, data_wr) to write the test pattern. Then call read_reg(addr) and compare the read-back value (masked with wr_mask) against the written value (masked with wr_mask). Report pass or fail for each register and pattern combination. 5. If skip_array entry is 1 (MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW), skip the write and read-back verification for that register.",
        "Test Steps / Procedure": "1. Initialize the register address list, expected default values, read masks, write masks, and skip flags for the five subsystem registers under test. 2. Read each register and verify that the default reset value matches the expected value using the read mask. 3. For each writable register (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv), write a test data pattern to the register. 4. Read back the register value and compare it against the written value using the write mask. 5. Repeat steps 3-4 for multiple test data patterns to cover different bit combinations. 6. Skip write-read verification for the interrupt_raw register since it is read-only. 7. Report pass if all default value checks and write-read comparisons match; report fail otherwise.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "data_fifo_threshold_val; low_pwr; dbite; dbi_fdiv; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "For chk_rst_val: Each register read value masked with rd_mask_array must equal the corresponding rst_val_array entry masked with rd_mask_array. If mismatch, the test reports failure with the register address, expected value, and actual value. For chk_rd_wr: For each non-skipped register, the read-back value masked with wr_mask_array must equal the written data pattern masked with wr_mask_array. If mismatch, the test reports failure with the register address, written value, and read-back value. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (skip_array=1) is only validated for its default reset value and is not subjected to write-read verification. The test passes only if all default value checks and all write-read pattern checks succeed for all registers.",
        "Validation / Acceptance Criteria": "The test passes when all five subsystem registers return their expected default reset values during the initial read verification. For the four writable registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv), the test passes when every write-read cycle across all test data patterns produces a read-back value that matches the written value after applying the write mask. The interrupt_raw register is validated for its default value only. The test fails if any default value mismatch or any write-read mismatch is detected.",
        "Remarks": "The source files in the testcase folder contain PCIe device enumeration code and do not contain MIPI DSI content. Testcase details are derived from upstream agent register-access analysis, the testcase folder name, and the register write-read verification pattern indicated by the Agent 2 operations. All five registers are in the MIPI DSI subsystem block (base 0xE6A41000) and were successfully mapped to canonical register names from the subsystem specification. The interrupt_raw register is treated as read-only based on the Agent 2 operation classification."
    }
]

# TestPlan sheet columns
testplan_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# MetaData sheet columns
metadata_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# Header formatting
header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

# Write TestPlan headers
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, '')
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = 'A2'

# Auto-size columns with max width
for col_idx, col_name in enumerate(testplan_columns, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        cell_val = ws_tp.cell(row=row_idx, column=col_idx).value
        if cell_val:
            max_len = max(max_len, min(len(str(cell_val)), 80))
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 60)

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, '')
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_md.freeze_panes = 'A2'

# Auto-size columns with max width
for col_idx, col_name in enumerate(metadata_columns, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        cell_val = ws_md.cell(row=row_idx, column=col_idx).value
        if cell_val:
            max_len = max(max_len, min(len(str(cell_val)), 80))
    ws_md.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 60)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save workbook
wb.save(output_path)

# Validation
from openpyxl import load_workbook
wb_check = load_workbook(output_path)
assert 'TestPlan' in wb_check.sheetnames, 'TestPlan sheet missing'
assert 'MetaData' in wb_check.sheetnames, 'MetaData sheet missing'
assert wb_check['TestPlan'].max_row == 4, f'Expected 4 rows in TestPlan, got {wb_check["TestPlan"].max_row}'
assert wb_check['MetaData'].max_row == 4, f'Expected 4 rows in MetaData, got {wb_check["MetaData"].max_row}'
file_size = os.path.getsize(output_path)
assert file_size > 0, 'File size is 0'

print(f'SUCCESS: Generated {output_path}')
print(f'File size: {file_size} bytes')
print(f'TestPlan rows: {wb_check["TestPlan"].max_row - 1}')
print(f'MetaData rows: {wb_check["MetaData"].max_row - 1}')
print(f'Filename: {filename}')
