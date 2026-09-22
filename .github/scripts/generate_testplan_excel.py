#!/usr/bin/env python3
"""Agent 7 - Excel Generator Script for MIPI_DSI TestPlan.
Generates a genuine Office Open XML workbook (.xlsx) using openpyxl.
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
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "openpyxl"])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter

# === INPUT JSON DATA ===
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "DSI Host and Subsystem Basic Initialization and DMA Transfer",
        "Meta Headers": "NA",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs basic MIPI DSI initialization and a DMA-based data transfer. It enables DMAC interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTEN. It enables subsystem-level interrupts (including GDMA interrupt) by writing to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE. It configures the DSI host PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG (setting PHY stop wait time and lane count). It configures packet handling by writing to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG (value 0x3d enabling EOTP TX, EOTP RX, BTA, ECC RX, CRC RX, and EOTP TX LP). It configures the clock manager by writing to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG (value 0x107 setting TX escape clock division and timeout clock division). It disables DPI mode by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It programs DMA instructions via MIZAR_MIPI_DSI_DMAC_DBGINST0 and MIZAR_MIPI_DSI_DMAC_DBGINST1, then executes them by writing to MIZAR_MIPI_DSI_DMAC_DBGCMD. It polls MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to check the GDMA interrupt status. It reads MIZAR_MIPI_DSI_DMAC_INTMIS to check DMA channel interrupt status. It clears DMA interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTCLR. It clears subsystem raw interrupts by writing to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.",
        "Test Description": "This testcase validates the basic initialization and DMA transfer functionality of the MIPI DSI subsystem. It enables DMAC and subsystem-level interrupts, configures the DSI host PHY interface (lane count and stop wait time), sets up packet handling configuration (EOTP, BTA, ECC, CRC), configures the clock manager for escape clock and timeout clock division, disables DPI mode via the dpi_control register, programs and executes DMA instructions through the DMAC debug interface, polls the interrupt_mask register for GDMA interrupt status, reads the DMAC interrupt status, and clears both DMA and subsystem interrupts upon completion.",
        "Meta Test Steps / Procedure": "1. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC interrupts for channels.\n2. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR to enable GDMA interrupt at subsystem level.\n3. Read MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, apply set_data_mask for PHY_STOP_WAIT_TIME field, and write back to configure PHY interface.\n4. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to enable EOTP TX, EOTP RX, BTA, ECC RX, CRC RX, and EOTP TX LP.\n5. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to set TX escape clock division and timeout clock division.\n6. Write 0x0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI mode.\n7. Write DMA instruction bytes to MIZAR_MIPI_DSI_DMAC_DBGINST0 (channel/debug thread and opcode).\n8. Write DMA instruction operand to MIZAR_MIPI_DSI_DMAC_DBGINST1.\n9. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the programmed DMA instruction.\n10. Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and compare with MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR to poll for GDMA interrupt assertion.\n11. Read MIZAR_MIPI_DSI_DMAC_INTMIS to check DMA channel interrupt status.\n12. Write channel mask to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMA interrupts.\n13. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem raw interrupt status.\n14. Repeat steps 7-13 for additional DMA transfer iterations as needed.",
        "Test Steps / Procedure": "1. Enable DMAC interrupts for the required DMA channels.\n2. Enable the GDMA interrupt at the subsystem level via the interrupt_enable register.\n3. Configure the DSI host PHY interface (PHY_IF_CFG register) with the appropriate lane count and PHY stop wait time.\n4. Configure the packet handling register (PCKHDL_CFG) to enable EOTP transmit, EOTP receive, BTA, ECC receive, CRC receive, and EOTP transmit in low-power mode.\n5. Configure the clock manager register (CLKMGR_CFG) with the TX escape clock division and timeout clock division values.\n6. Disable DPI mode by writing to the dpi_control register.\n7. Program DMA transfer instructions via the DMAC debug instruction registers.\n8. Execute the DMA instruction by triggering the DMAC debug command.\n9. Poll the interrupt_mask register to verify that the GDMA interrupt has been asserted.\n10. Read the DMAC interrupt status to confirm DMA channel completion.\n11. Clear the DMA channel interrupts.\n12. Clear the subsystem raw interrupt status via the interrupt_raw register.\n13. Repeat DMA programming and execution for additional transfer iterations as required.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; interrupt_mask; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "After enabling GDMA interrupt and executing DMA instructions, the test reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and checks that the GDMA interrupt bit (MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) is set, confirming the interrupt was asserted. It then reads MIZAR_MIPI_DSI_DMAC_INTMIS to verify the DMA channel interrupt status matches the expected channel mask. Upon successful verification, DMA interrupts are cleared via MIZAR_MIPI_DSI_DMAC_INTCLR and subsystem raw interrupts are cleared via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. The test passes if the GDMA interrupt is correctly asserted and DMA channel completion is confirmed for all transfer iterations.",
        "Validation / Acceptance Criteria": "The test passes if the GDMA interrupt is correctly asserted in the interrupt_mask register after DMA instruction execution. The DMAC interrupt status must confirm DMA channel completion. All DMA and subsystem interrupts must be successfully cleared after each transfer iteration. The PHY_IF_CFG, PCKHDL_CFG, CLKMGR_CFG, and dpi_control registers must accept their configured values without error. Failure occurs if the GDMA interrupt is not asserted or if DMA channel completion is not indicated.",
        "Remarks": "The actual MIPI DSI testcase source code was not found in the repository folder; the files present (program.c, main.c) contain unrelated PCIe and STM32 code. Testcase details are derived from Agent 2, Agent 3, and Agent 4 outputs. Six DMAC registers (DMAC_INTEN, DMAC_DBGINST0, DMAC_DBGINST1, DMAC_DBGCMD, DMAC_INTMIS, DMAC_INTCLR) could not be mapped to canonical register names as no DMAC register specification was provided. The interrupt polling behavior and DMA instruction programming flow are inferred from the register access patterns provided by upstream agents."
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
        "Meta Test Description": "This testcase performs a MIPI DSI DBI random payload transfer using DMA. It configures the DSI host PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. It configures packet handling by writing to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG. It configures the clock manager by writing to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. It disables DPI mode by writing to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It enables DMAC interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTEN. It programs DMA instructions via MIZAR_MIPI_DSI_DMAC_DBGINST0 and MIZAR_MIPI_DSI_DMAC_DBGINST1, then executes them by writing to MIZAR_MIPI_DSI_DMAC_DBGCMD. It polls MIZAR_MIPI_DSI_DMAC_INTMIS to check DMA channel interrupt completion status. It clears DMA interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTCLR.",
        "Test Description": "This testcase validates the MIPI DSI DBI interface random payload transfer using DMA. It configures the DSI host PHY interface via the PHY_IF_CFG register, sets up packet handling via the PCKHDL_CFG register, configures the clock manager via the CLKMGR_CFG register, and disables DPI mode via the dpi_control register. It then enables DMAC interrupts, programs and executes DMA transfer instructions through the DMAC debug interface, polls the DMAC interrupt status for DMA channel completion, and clears the DMA interrupts upon completion.",
        "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the DSI host PHY interface parameters.\n2. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling settings.\n3. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager settings.\n4. Write 0x0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI mode.\n5. Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC interrupts for the required DMA channels.\n6. Write DMA instruction bytes to MIZAR_MIPI_DSI_DMAC_DBGINST0 (channel/debug thread and opcode).\n7. Write DMA instruction operand to MIZAR_MIPI_DSI_DMAC_DBGINST1.\n8. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the programmed DMA instruction.\n9. Poll MIZAR_MIPI_DSI_DMAC_INTMIS to wait for DMA channel interrupt completion.\n10. Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA channel interrupts.",
        "Test Steps / Procedure": "1. Configure the DSI host PHY interface by writing to the PHY_IF_CFG register with the appropriate lane count and PHY stop wait time.\n2. Configure the packet handling register (PCKHDL_CFG) with the required EOTP, BTA, ECC, and CRC settings.\n3. Configure the clock manager register (CLKMGR_CFG) with the TX escape clock division and timeout clock division values.\n4. Disable DPI mode by writing to the dpi_control register.\n5. Enable DMAC interrupts for the required DMA channels.\n6. Program DMA transfer instructions via the DMAC debug instruction registers for random payload data.\n7. Execute the DMA instruction by triggering the DMAC debug command.\n8. Poll the DMAC interrupt status register to verify DMA channel completion.\n9. Clear the DMA channel interrupts after successful transfer completion.\n10. Repeat DMA programming and execution for additional random payload transfer iterations as required.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_DMAC_INTMIS to verify that the DMA channel interrupt is asserted upon completion of the random payload transfer. Once the expected interrupt status is detected, the test writes to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA channel interrupts. The test passes if the DMA channel completion interrupt is correctly asserted for all random payload transfer iterations and all interrupts are successfully cleared.",
        "Validation / Acceptance Criteria": "The test passes if the DMAC interrupt status confirms DMA channel completion after each random payload transfer. All DMA channel interrupts must be successfully cleared after each transfer iteration. The PHY_IF_CFG, PCKHDL_CFG, CLKMGR_CFG, and dpi_control registers must accept their configured values without error. Failure occurs if the DMA channel completion interrupt is not asserted or if interrupt clearing fails.",
        "Remarks": "The source files in the repository folder (program.c, Makefile) contain unrelated PCIe and timezone code, not actual MIPI DSI DBI random payload test code. Testcase details are derived entirely from Agent 2, Agent 3, and Agent 4 outputs. Six DMAC registers (DMAC_INTEN, DMAC_DBGINST0, DMAC_DBGINST1, DMAC_DBGCMD, DMAC_INTMIS, DMAC_INTCLR) could not be mapped to canonical register names as no DMAC register specification was provided. The polling behavior on the DMAC interrupt status register is inferred from the Agent 2 poll operation."
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
        "Meta Test Description": "This testcase performs register write-read verification on MIPI DSI subsystem registers. It performs read-modify-write operations on four subsystem registers: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL (offset 0x0), MIZAR_MIPI_DSI_SUBSYS_LOW_PWR (offset 0x4), MIZAR_MIPI_DSI_SUBSYS_DBITE (offset 0x8), and MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV (offset 0xC). Each of these registers is written with a test value and then read back to verify the written data matches. Additionally, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (offset 0x10) is read to check its reset value. The test iterates through an address array containing these register addresses, writes known data patterns, reads back the values, and compares the read data against expected values using read and write masks to isolate writable and readable bit fields.",
        "Test Description": "This testcase validates the write-read integrity of MIPI DSI subsystem registers. It performs read-modify-write operations on the data_fifo_threshold_val, low_pwr, dbite, and dbi_fdiv registers by writing known test data patterns and reading them back to verify correctness. The interrupt_raw register is read to verify its reset value. The test ensures that all writable bit fields in the targeted subsystem registers retain written values correctly when read back, confirming proper register accessibility and data integrity.",
        "Meta Test Steps / Procedure": "1. Initialize the register address array containing MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.\n2. For each register in the address array, read the current register value using read_reg(addr_array[i]) and compare against the expected default/reset value.\n3. For registers not in the skip array (MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV), write a test data pattern using write_reg(addr_array[i], data_wr) where data_wr is derived from the write mask.\n4. Read back the written register value using read_reg(addr_array[i]) and apply the read mask to isolate readable bits.\n5. Compare the masked read-back value against the expected written value.\n6. For MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (skip_array entry is 1), perform only the reset value read check without writing.\n7. Report pass if all read-back comparisons match expected values, or report fail with the mismatched register address and data.",
        "Test Steps / Procedure": "1. Read the reset values of the data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, and interrupt_raw registers and verify they match expected default values.\n2. Write a known test data pattern to the data_fifo_threshold_val register and read it back to verify the written value is retained correctly.\n3. Write a known test data pattern to the low_pwr register and read it back to verify the written value is retained correctly.\n4. Write a known test data pattern to the dbite register and read it back to verify the written value is retained correctly.\n5. Write a known test data pattern to the dbi_fdiv register and read it back to verify the written value is retained correctly.\n6. Verify the interrupt_raw register reset value by reading it without performing a write operation.\n7. Confirm that all register read-back values match the expected written values after applying the appropriate bit masks.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "data_fifo_threshold_val; low_pwr; dbite; dbi_fdiv; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "For each register in the address array, the test reads the current value and compares it against the expected reset/default value. For writable registers (MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV), a test data pattern is written and then read back. The read-back value is masked with the read mask and compared against the expected value (written data masked with the write mask). If the masked read-back value does not match the expected value, the test reports a failure with the mismatched register address and data values. For MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, only the reset value read is performed and compared. The test passes only if all register comparisons succeed across all iterations.",
        "Validation / Acceptance Criteria": "The test passes if all five subsystem registers return expected values. The data_fifo_threshold_val, low_pwr, dbite, and dbi_fdiv registers must retain written test data patterns correctly when read back, with readable bit fields matching the written values. The interrupt_raw register must return its expected reset value when read. Any mismatch between expected and actual read-back values for any register constitutes a test failure.",
        "Remarks": "The source files in the repository folder (program.c, test_define.c, Makefile) contain unrelated PCIe enumeration code and timezone build configuration, not actual MIPI DSI subsystem register write-read test code. Testcase details are derived entirely from Agent 2, Agent 3, and Agent 4 outputs. All five subsystem register macros were successfully mapped to canonical register names from the mipi_dsi_subsys_autoreg specification. The SOFT_RST_REG_ADDRESS macro was excluded per instructions. The interrupt_raw register is read-only in this testcase context (skip_array indicates write is skipped for this register)."
    }
]

# === CONFIGURATION ===
IP_NAME = "MIPI_DSI"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"

# === IST TIMESTAMP ===
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"{IP_NAME}_TestPlan_{timestamp_str}.xlsx"
output_path = os.path.join(OUTPUT_DIR, filename)

# Ensure output directory exists
os.makedirs(OUTPUT_DIR, exist_ok=True)

# === TESTPLAN SHEET COLUMNS ===
testplan_columns = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation"
]

# === METADATA SHEET COLUMNS ===
metadata_columns = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays"
]

# === CREATE WORKBOOK ===
print(f"Creating workbook: {filename}")
wb = Workbook()

# === TESTPLAN SHEET ===
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Header formatting
header_font = Font(name="Calibri", size=11, bold=True, color="FFFFFF")
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

# Write TestPlan data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        if col_name == "Code Generation":
            value = ""
        else:
            value = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_tp.freeze_panes = "A2"

# === METADATA SHEET ===
ws_md = wb.create_sheet(title="MetaData")

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write MetaData data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_md.freeze_panes = "A2"

# === AUTO-SIZE COLUMNS ===
def auto_size_columns(ws, max_width=60):
    for col in ws.columns:
        max_length = 0
        column_letter = get_column_letter(col[0].column)
        for cell in col:
            try:
                if cell.value:
                    cell_lines = str(cell.value).split("\n")
                    for line in cell_lines:
                        if len(line) > max_length:
                            max_length = len(line)
            except Exception:
                pass
        adjusted_width = min(max(max_length + 2, 12), max_width)
        ws.column_dimensions[column_letter].width = adjusted_width

auto_size_columns(ws_tp, max_width=60)
auto_size_columns(ws_md, max_width=60)

# === SET METADATA SHEET TO VERY HIDDEN ===
ws_md.sheet_state = "veryHidden"

# === SAVE WORKBOOK ===
wb.save(output_path)
print(f"Workbook saved to: {output_path}")

# === POST-SAVE VALIDATION ===
validation_passed = True
errors = []

# Check file exists
if not os.path.exists(output_path):
    validation_passed = False
    errors.append("File does not exist")
else:
    # Check file size
    file_size = os.path.getsize(output_path)
    if file_size == 0:
        validation_passed = False
        errors.append("File size is 0")
    else:
        print(f"File size: {file_size} bytes")

    # Try reopening
    try:
        wb_check = load_workbook(output_path)
        sheet_names = wb_check.sheetnames
        if "TestPlan" not in sheet_names:
            validation_passed = False
            errors.append("TestPlan sheet not found")
        if "MetaData" not in sheet_names:
            validation_passed = False
            errors.append("MetaData sheet not found")
        
        # Verify row counts
        tp_rows = wb_check["TestPlan"].max_row - 1  # minus header
        md_rows = wb_check["MetaData"].max_row - 1  # minus header
        print(f"TestPlan rows: {tp_rows}")
        print(f"MetaData rows: {md_rows}")
        
        # Verify MetaData is veryHidden
        md_state = wb_check["MetaData"].sheet_state
        print(f"MetaData sheet state: {md_state}")
        
        wb_check.close()
    except Exception as e:
        validation_passed = False
        errors.append(f"Failed to reopen workbook: {str(e)}")

if validation_passed:
    print("VALIDATION: PASSED")
else:
    print(f"VALIDATION: FAILED - {'; '.join(errors)}")

# === OUTPUT SUMMARY ===
print(f"\n=== GENERATION SUMMARY ===")
print(f"Filename: {filename}")
print(f"Output path: {output_path}")
print(f"Rows (TestPlan): 3")
print(f"Rows (MetaData): 3")
print(f"Validation: {'PASSED' if validation_passed else 'FAILED'}")
print(f"Timestamp (IST): {now_ist.strftime('%Y-%m-%d %H:%M:%S %Z')}")
