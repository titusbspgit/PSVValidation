#!/usr/bin/env python3
"""One-shot Excel generator - creates MIPI_DSI TestPlan workbook."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os, sys, base64

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_DSI_TestPlan_{timestamp}.xlsx"

json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "NA",
        "Test Description": "This testcase validates basic MIPI DSI initialization and interrupt-driven DMA transfer functionality. It enables DMAC and subsystem-level interrupts, configures the DSI host PHY interface (PHY_IF_CFG), packet handler (PCKHDL_CFG), and clock manager (CLKMGR_CFG). It disables DPI control (dpi_control) and programs the DMAC debug instruction and command registers to initiate a DMA operation. The test then polls the subsystem interrupt mask register (interrupt_mask) for the GDMA interrupt bit. Upon interrupt assertion, it reads the DMAC interrupt status, clears the DMAC interrupt, and clears the subsystem raw interrupt (interrupt_raw) to complete the test.",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "Six DMAC registers (DMAC interrupt enable, DMAC debug instruction 0, DMAC debug instruction 1, DMAC debug command, DMAC masked interrupt status, DMAC interrupt clear) could not be mapped to canonical register names because no DMAC specification document was provided. The test involves two sequential DMAC debug instruction and interrupt verification cycles. The polling loop on the subsystem interrupt mask register has no timeout, which means the test will hang if the interrupt is never asserted.",
        "Test Steps / Procedure": "1. Enable DMAC interrupts by configuring the DMAC interrupt enable register. 2. Enable the subsystem-level GDMA interrupt via the interrupt_enable register. 3. Configure the DSI host PHY interface parameters in the PHY_IF_CFG register (PHY stop wait time). 4. Configure the DSI host packet handler via the PCKHDL_CFG register. 5. Configure the DSI host clock manager via the CLKMGR_CFG register. 6. Disable DPI control by writing to the dpi_control register. 7. Program the DMAC debug instruction registers with the desired DMA channel and instruction encoding. 8. Execute the DMAC debug command to initiate the DMA transfer. 9. Poll the subsystem interrupt_mask register until the GDMA interrupt bit is asserted. 10. Read the DMAC masked interrupt status register to verify the interrupt source. 11. Clear the DMAC interrupt by writing to the DMAC interrupt clear register. 12. Clear the subsystem raw interrupt by writing to the interrupt_raw register. 13. Repeat the DMAC debug instruction and interrupt verification sequence for a second transfer. 14. Verify test completion with a pass indication.",
        "Impacted Registers": "interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; interrupt_mask; interrupt_raw",
        "Validation / Acceptance Criteria": "The test passes when the subsystem interrupt_mask register indicates the GDMA interrupt bit is asserted after the DMAC debug command execution. The DMAC masked interrupt status register must confirm the interrupt source. After clearing both the DMAC interrupt and the subsystem raw interrupt (interrupt_raw), the test completes with a pass status via finish(0). If the GDMA interrupt is never asserted, the test will hang in the polling loop, indicating a failure.",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Meta Test Description": "This testcase performs a basic MIPI DSI initialization and interrupt-driven DMA transfer test. It enables DMAC interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTEN with value 0x3. It enables subsystem-level GDMA interrupt by writing to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE using the MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR mask. It configures the DSI host PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. It configures the packet handler via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG and the clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. It disables DPI control by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It programs the DMAC debug instruction registers MIZAR_MIPI_DSI_DMAC_DBGINST0 and MIZAR_MIPI_DSI_DMAC_DBGINST1 and executes the debug command via MIZAR_MIPI_DSI_DMAC_DBGCMD. It then polls MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK checking for the GDMA interrupt bit using MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR. Upon interrupt detection, it reads MIZAR_MIPI_DSI_DMAC_INTMIS to check the DMAC interrupt status, clears the DMAC interrupt by writing to MIZAR_MIPI_DSI_DMAC_INTCLR, and clears the subsystem interrupt by writing to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.",
        "Meta Test Steps / Procedure": "1. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC interrupts. 2. Write MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable subsystem GDMA interrupt. 3. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME to configure PHY stop wait time. 4. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handler. 5. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager. 6. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control. 7. Write debug instruction bytes to MIZAR_MIPI_DSI_DMAC_DBGINST0 (channel and instruction encoding). 8. Write debug instruction payload to MIZAR_MIPI_DSI_DMAC_DBGINST1. 9. Write 0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the debug command. 10. Poll MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK in a while loop checking for MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit. 11. Read MIZAR_MIPI_DSI_DMAC_INTMIS to check DMAC masked interrupt status. 12. Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMAC interrupt. 13. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem raw interrupt. 14. Repeat steps 7-13 for a second DMAC debug instruction sequence. 15. Call finish(0) to indicate test completion.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK in a while loop, checking for the MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit to be set. Once the GDMA interrupt is detected, it reads MIZAR_MIPI_DSI_DMAC_INTMIS to verify the DMAC interrupt status. The DMAC interrupt is then cleared via MIZAR_MIPI_DSI_DMAC_INTCLR and the subsystem interrupt is cleared via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. The test completes successfully by calling finish(0), indicating a pass condition. If the GDMA interrupt bit is never asserted in the interrupt mask register, the test will remain in the polling loop indefinitely.",
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_dbi_random_payload_test",
        "Feature": "NA",
        "Test Description": "This testcase validates MIPI DSI DBI random payload transfer using the DMAC engine. It configures the DSI host PHY interface (PHY_IF_CFG), packet handler (PCKHDL_CFG), and clock manager (CLKMGR_CFG). It disables DPI control (dpi_control) and enables DMAC interrupts. The test programs the DMAC debug instruction and command registers to initiate a DMA transfer for a channel descriptor. It then polls the DMAC masked interrupt status register until both channel interrupts are asserted. Upon completion, it clears the DMAC interrupt and repeats the sequence for a second DMAC channel to verify multi-channel DBI random payload transfer.",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "Six DMAC registers (DMAC interrupt enable, DMAC debug instruction 0, DMAC debug instruction 1, DMAC debug command, DMAC masked interrupt status, DMAC interrupt clear) could not be mapped to canonical register names because no DMAC specification document was provided. The source files in the testcase folder do not contain MIPI DSI content; the testcase details are derived from upstream agent outputs. The polling loop on the DMAC masked interrupt status register has no timeout, which means the test will hang if the expected interrupt status is never reached. The test involves two sequential DMAC channel transfer and interrupt verification cycles.",
        "Test Steps / Procedure": "1. Configure the DSI host PHY interface parameters in the PHY_IF_CFG register (PHY stop wait time). 2. Configure the DSI host packet handler via the PCKHDL_CFG register. 3. Configure the DSI host clock manager via the CLKMGR_CFG register. 4. Disable DPI control by writing to the dpi_control register. 5. Enable DMAC interrupts by configuring the DMAC interrupt enable register. 6. Program the DMAC debug instruction registers with the desired DMA channel and instruction encoding. 7. Load the channel descriptor address into the DMAC debug instruction payload register. 8. Execute the DMAC debug command to initiate the DMA transfer. 9. Poll the DMAC masked interrupt status register until both channel interrupts are asserted. 10. Clear the DMAC interrupt by writing to the DMAC interrupt clear register. 11. Repeat the DMAC debug instruction and interrupt verification sequence for a second channel. 12. Verify test completion with a pass indication.",
        "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control",
        "Validation / Acceptance Criteria": "The test passes when the DMAC masked interrupt status register indicates both channel interrupts are asserted (value equals expected pattern) after the DMAC debug command execution. After clearing the DMAC interrupt via the interrupt clear register, the sequence is verified for a second channel. The test completes with a pass status via finish(0). If the expected interrupt status is never reached, the test will hang in the polling loop, indicating a failure.",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Meta Test Description": "This testcase performs a MIPI DSI DBI random payload transfer test using DMAC. It configures the DSI host PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. It configures the packet handler by writing 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG and the clock manager by writing 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. It disables DPI control by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It enables DMAC interrupts by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN. It programs the DMAC debug instruction registers MIZAR_MIPI_DSI_DMAC_DBGINST0 with 0x00A00000 and MIZAR_MIPI_DSI_DMAC_DBGINST1 with the channel 0 descriptor address, then executes the debug command by writing 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD. It polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until the read value equals 0x3. Upon completion, it clears the DMAC interrupt by writing the read data to MIZAR_MIPI_DSI_DMAC_INTCLR. The sequence is repeated for a second DMAC channel.",
        "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME to configure PHY stop wait time. 2. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handler. 3. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager. 4. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control. 5. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC interrupts. 6. Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 to set debug instruction 0 with channel and instruction encoding. 7. Write channel 0 descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST1. 8. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the debug command. 9. Read MIZAR_MIPI_DSI_DMAC_INTMIS and poll in a while loop until value equals 0x3. 10. Write the read data to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMAC interrupt. 11. Repeat steps 6-10 for a second DMAC channel with updated descriptor address. 12. Call finish(0) to indicate test completion.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop, checking until the read value equals 0x3 (both channel interrupts asserted). Once the expected interrupt status is detected, it writes the read data to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMAC interrupt. This polling and clearing sequence is performed twice, once for each DMAC channel. The test completes successfully by calling finish(0), indicating a pass condition. If the DMAC interrupt status never reaches 0x3, the test will remain in the polling loop indefinitely.",
    },
    {
        "Index": "3",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "Subsystem Register Write-Read Verification",
        "Test Description": "This testcase validates the MIPI DSI subsystem registers by performing a default value check and a write-read-back verification. It reads each subsystem register and compares the value against the expected reset default. It then writes a test pattern to each writable register and reads it back to verify data integrity. The registers under test are data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv (all write-read-back verified), and interrupt_raw (default value check only, skipped for write-read-back). A soft reset is performed between the two test phases. The test passes if all default value comparisons and write-read-back comparisons succeed.",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "All five subsystem registers were successfully mapped to canonical register names from the MIPI DSI subsystem specification. The interrupt_raw register is treated as read-only for write-read-back purposes (skip_array entry is set to 1). The source files in the testcase folder contain PCIe-related code rather than MIPI DSI code; the testcase details are derived from upstream agent outputs which extracted valid MIPI DSI register-access information. A soft reset is performed between the default value check and write-read-back phases, but the soft reset register is excluded from tracking per instructions.",
        "Test Steps / Procedure": "1. Read each MIPI DSI subsystem register and verify the value matches the expected reset default value. 2. Registers checked for default values: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, and interrupt_raw. 3. Record any mismatches as errors during the default value verification phase. 4. Perform a soft reset of the subsystem. 5. Write a test data pattern to each writable subsystem register: data_fifo_threshold_val, low_pwr, dbite, and dbi_fdiv. 6. Read back each written register and compare the read value against the written value. 7. Skip the interrupt_raw register during the write-read-back phase (read-only verification only). 8. Record any mismatches as errors during the write-read-back verification phase. 9. If all default value checks and write-read-back checks pass with no errors, report test pass. 10. If any errors are detected, report test failure.",
        "Impacted Registers": "data_fifo_threshold_val; low_pwr; dbite; dbi_fdiv; interrupt_raw",
        "Validation / Acceptance Criteria": "The test passes when all subsystem register default value reads match their expected reset values, and all write-read-back operations on data_fifo_threshold_val, low_pwr, dbite, and dbi_fdiv return the written test pattern. The interrupt_raw register must match its expected default value but is not required to pass write-read-back verification. The test fails if any default value comparison or write-read-back comparison produces a mismatch. Pass is indicated by finish(0) and failure by finish(1).",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Meta Test Description": "This testcase performs a register write-read verification test on MIPI DSI subsystem registers. It iterates over an array of subsystem register addresses (addr_array) and a corresponding array of default values (rst_val_array). For each register, the function chk_rst_val() reads the register using read_reg(addr_array[i]) and compares the read value against the expected default value from rst_val_array[i]. If the values do not match, an error counter (err1) is incremented. Next, the function chk_rd_wr() writes a test data pattern (data_wr) to each register using write_reg(addr_array[i], data_wr), then reads it back using read_reg(addr_array[i]) and compares the read-back value against the written value. Registers with skip_array[i] == 1 are skipped during the write-read-back phase. The registers tested are: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL (read_modify_write), MIZAR_MIPI_DSI_SUBSYS_LOW_PWR (read_modify_write), MIZAR_MIPI_DSI_SUBSYS_DBITE (read_modify_write), MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV (read_modify_write), and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (read only, skipped for write-read-back). A soft reset is performed between the default value check and the write-read-back test using SOFT_RST_REG_ADDRESS (excluded from register tracking). The test calls finish(0) on success or finish(1) on failure.",
        "Meta Test Steps / Procedure": "1. Initialize error counters err1 and err2 to 0. 2. Call chk_rst_val() which iterates over addr_array containing MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. 3. For each register in addr_array, read the register value using data_rd = read_reg(addr_array[i]). 4. Compare data_rd against rst_val_array[i] (expected default value). 5. If mismatch, increment err1 error counter. 6. Perform a soft reset by reading and writing SOFT_RST_REG_ADDRESS (excluded from tracking). 7. Call chk_rd_wr() which iterates over addr_array. 8. For each register where skip_array[i] != 1, write data_wr to the register using write_reg(addr_array[i], data_wr). 9. Read back the register value using data_rd = read_reg(addr_array[i]). 10. Compare data_rd against data_wr. 11. If mismatch, increment err2 error counter. 12. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is skipped during write-read-back (skip_array[4] == 1). 13. If err1 == 0 and err2 == 0, call finish(0) indicating pass. 14. If any errors, call finish(1) indicating failure.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Meta Validation / Acceptance Criteria": "In chk_rst_val(), each register read via read_reg(addr_array[i]) is compared against rst_val_array[i]. If data_rd != rst_val_array[i], err1 is incremented. In chk_rd_wr(), each register (where skip_array[i] != 1) is written with data_wr via write_reg(addr_array[i], data_wr), then read back via data_rd = read_reg(addr_array[i]). If data_rd != data_wr, err2 is incremented. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW has skip_array[4] == 1 and is not write-read-back tested. The test passes (finish(0)) if err1 == 0 and err2 == 0. The test fails (finish(1)) if any error counter is non-zero.",
    },
]

# TestPlan columns
tp_cols = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Code Generation"
]

# MetaData columns
md_cols = [
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria", "Meta Headers",
    "Meta Macros", "Meta Arrays"
]

wb = openpyxl.Workbook()

# --- TestPlan sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_align = Alignment(wrap_text=True, vertical="top")

for col_idx, col_name in enumerate(tp_cols, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(tp_cols, 1):
        val = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

ws_tp.freeze_panes = "A2"

# Auto-size columns
for col_idx, col_name in enumerate(tp_cols, 1):
    max_len = len(col_name)
    for row in range(2, len(json_data) + 2):
        val = ws_tp.cell(row=row, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_tp.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = min(max_len + 4, 60)

# --- MetaData sheet ---
ws_md = wb.create_sheet("MetaData")

for col_idx, col_name in enumerate(md_cols, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(md_cols, 1):
        val = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

ws_md.freeze_panes = "A2"

for col_idx, col_name in enumerate(md_cols, 1):
    max_len = len(col_name)
    for row in range(2, len(json_data) + 2):
        val = ws_md.cell(row=row, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_md.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = min(max_len + 4, 60)

ws_md.sheet_state = "veryHidden"

# Save
script_dir = os.path.dirname(os.path.abspath(__file__))
filepath = os.path.join(script_dir, filename)
wb.save(filepath)

# Validate
assert os.path.exists(filepath), "File not created"
assert os.path.getsize(filepath) > 0, "File is empty"
vwb = openpyxl.load_workbook(filepath)
assert "TestPlan" in vwb.sheetnames, "TestPlan sheet missing"
assert "MetaData" in vwb.sheetnames, "MetaData sheet missing"
vwb.close()

# Output base64 for upload
with open(filepath, "rb") as f:
    b64 = base64.b64encode(f.read()).decode("utf-8")

print(f"FILENAME={filename}")
print(f"FILEPATH={filepath}")
print(f"FILESIZE={os.path.getsize(filepath)}")
print(f"BASE64_START")
print(b64)
print(f"BASE64_END")
print("VALIDATION=PASSED")
