#!/usr/bin/env python3
"""
MIPI_DSI TestPlan Excel Generator - Run this script to generate the XLSX file.
Usage: python3 generate_mipi_dsi_testplan.py
Output: MIPI_DSI_TestPlan_20260922_203551.xlsx
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter
import base64, os, sys

FILENAME = "MIPI_DSI_TestPlan_20260922_203551.xlsx"

json_data = [
    {"Index":"1","SS / Module":"MIPI_DSI","Test Case Name":"mipi_dsi_basic_test","Feature":"NA","Speed":"NA","Mode":"NA","Memory Start Offset":"NA","Memory End Offset":"NA",
     "Test Description":"This testcase validates basic MIPI DSI initialization and interrupt-driven DMA transfer. It enables DMAC interrupts and subsystem-level interrupt forwarding via the interrupt_enable register. It configures the DSI host PHY interface through PHY_IF_CFG, sets up the packet handler via PCKHDL_CFG, and configures the clock manager via CLKMGR_CFG. It disables DPI output through the dpi_control register. It then programs and executes DMA channel instructions through DMAC debug instruction registers. The test verifies interrupt propagation by reading the interrupt_mask register to confirm GDMA interrupt masking status and reading the DMAC masked interrupt status register. Finally, it clears both DMAC and subsystem-level interrupts via the respective clear and interrupt_raw registers.",
     "Remarks":"Six DMAC register tokens (DMAC INTEN, DBGINST0, DBGINST1, DBGCMD, INTMIS, INTCLR) could not be mapped to canonical register names in the available register specification documents. The MIPI DSI subsystem base address was not fully resolved by Agent 3 (partially resolved). Source files found in the repository folder did not contain MIPI DSI testcase code; upstream Agent 2, 3, and 4 outputs were used as the authoritative data source for register access and mapping information.",
     "Test Steps / Procedure":"1. Enable DMAC channel interrupts by writing the interrupt enable value to the DMAC interrupt enable register. 2. Enable subsystem-level GDMA interrupt forwarding by configuring the interrupt_enable register. 3. Configure the DSI host PHY interface stop wait time through the PHY_IF_CFG register. 4. Configure the packet handler settings via the PCKHDL_CFG register. 5. Configure the clock manager via the CLKMGR_CFG register. 6. Disable DPI output by writing to the dpi_control register. 7. Program DMA channel debug instructions by writing instruction opcode and channel information to the DMAC debug instruction registers. 8. Write the DMA instruction payload to the DMAC debug instruction payload register. 9. Execute the programmed DMA debug instruction by triggering the DMAC debug command register. 10. Repeat DMA instruction programming and execution for all required DMA operations. 11. Read the interrupt_mask register and verify that the GDMA interrupt mask bit reflects the expected state. 12. Read the DMAC masked interrupt status register to confirm DMA channel completion. 13. Clear the DMAC channel interrupt by writing to the DMAC interrupt clear register. 14. Clear the subsystem raw interrupt by writing to the interrupt_raw register.",
     "Impacted Registers":"interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; interrupt_mask; interrupt_raw",
     "Validation / Acceptance Criteria":"After enabling DMAC and subsystem interrupts and executing DMA instructions, the interrupt_mask register must reflect the expected GDMA interrupt mask state. The DMAC masked interrupt status register must indicate successful DMA channel completion. After clearing interrupts through the DMAC interrupt clear register and the interrupt_raw register, the interrupt status should be cleared. The test passes if interrupt propagation from DMAC through the subsystem interrupt path is verified correctly.",
     "Meta Headers":"NA","Meta Macros":"NA","Meta Arrays":"NA",
     "Meta Test Description":"This testcase performs a basic MIPI DSI initialization and interrupt-driven DMA transfer validation. It writes to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC interrupts with value 0x3. It writes to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable subsystem-level GDMA interrupt. It configures the DSI host PHY interface via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask for the PHY_STOP_WAIT_TIME field. It configures the packet handler via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG. It configures the clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. It disables DPI control by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It programs DMA channel instructions by writing to MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and MIZAR_MIPI_DSI_DMAC_DBGCMD to execute DMAC debug instructions. It reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to verify the GDMA interrupt mask status. It reads MIZAR_MIPI_DSI_DMAC_INTMIS to check the masked interrupt status of the DMAC. It clears DMAC interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTCLR. It clears subsystem raw interrupts by writing to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.",
     "Meta Test Steps / Procedure":"1. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC channel interrupts. 2. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR bit set to enable subsystem GDMA interrupt forwarding. 3. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field to configure PHY stop wait time. 4. Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handler settings. 5. Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager. 6. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control. 7. Write DMA instruction bytes to MIZAR_MIPI_DSI_DMAC_DBGINST0 (channel number and debug instruction opcode). 8. Write DMA instruction payload to MIZAR_MIPI_DSI_DMAC_DBGINST1. 9. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the debug instruction. 10. Repeat steps 7-9 for additional DMA instructions as needed. 11. Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and compare with MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR to verify GDMA interrupt mask status. 12. Read MIZAR_MIPI_DSI_DMAC_INTMIS to check DMAC masked interrupt status and determine channel completion. 13. Write channel mask to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMAC interrupts. 14. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW with GDMA interrupt bit to clear subsystem raw interrupt.",
     "Meta Impacted Registers":"MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
     "Meta Validation / Acceptance Criteria":"After enabling DMAC and subsystem interrupts and executing DMA instructions, read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and verify the GDMA interrupt mask bit (MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) is set as expected. Read MIZAR_MIPI_DSI_DMAC_INTMIS to verify the DMAC masked interrupt status indicates channel completion. After clearing interrupts via MIZAR_MIPI_DSI_DMAC_INTCLR and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, the interrupt status should be cleared. The test passes if the interrupt mask and masked interrupt status registers reflect the expected interrupt state after DMA operations complete."},
    {"Index":"2","SS / Module":"MIPI_DSI","Test Case Name":"mipi_dsi_dbi_random_payload_test","Feature":"NA","Speed":"NA","Mode":"NA","Memory Start Offset":"NA","Memory End Offset":"NA",
     "Test Description":"This testcase validates MIPI DSI DBI random payload transfer using DMA. It configures the DSI host PHY interface through the PHY_IF_CFG register, sets up the packet handler via the PCKHDL_CFG register, and configures the clock manager via the CLKMGR_CFG register. It disables DPI output through the dpi_control register. It then enables DMAC interrupts and programs DMA channel debug instructions for two channels to initiate DBI payload transfers. The test polls the DMAC masked interrupt status register to wait for DMA completion and clears the DMAC interrupts upon completion.",
     "Remarks":"Six DMAC register tokens (DMAC INTEN, DBGINST0, DBGINST1, DBGCMD, INTMIS, INTCLR) could not be mapped to canonical register names in the available register specification documents. The MIPI DSI HOST and SUBSYS base addresses were not fully resolved by Agent 3 (partially resolved). Source files found in the repository folder contained unrelated PCIe code and a timezone Makefile, not MIPI DSI DBI random payload test code; upstream Agent 2, 3, and 4 outputs were used as the authoritative data source for register access and mapping information. The test uses a polling loop on the DMAC masked interrupt status register to wait for DMA completion.",
     "Test Steps / Procedure":"1. Configure the DSI host PHY interface stop wait time through the PHY_IF_CFG register. 2. Configure the packet handler settings via the PCKHDL_CFG register. 3. Configure the clock manager via the CLKMGR_CFG register. 4. Disable DPI output by writing to the dpi_control register. 5. Enable DMAC channel interrupts for both DMA channels by writing to the DMAC interrupt enable register. 6. Program DMA channel 0 debug instructions by writing the instruction opcode and descriptor address to the DMAC debug instruction registers. 7. Execute the DMA channel 0 debug instruction by triggering the DMAC debug command register. 8. Program DMA channel 1 debug instructions by writing the instruction opcode and descriptor address to the DMAC debug instruction registers. 9. Execute the DMA channel 1 debug instruction by triggering the DMAC debug command register. 10. Poll the DMAC masked interrupt status register until DMA channel completion is indicated. 11. Clear the DMAC channel interrupts by writing to the DMAC interrupt clear register.",
     "Impacted Registers":"PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control",
     "Validation / Acceptance Criteria":"The DMAC masked interrupt status register must indicate successful completion of both DMA channels after the DBI random payload transfer. After polling confirms channel completion, the DMAC interrupts are cleared via the interrupt clear register. The test passes if both DMA channels complete their transfers and the interrupt status reflects the expected completion state.",
     "Meta Headers":"NA","Meta Macros":"NA","Meta Arrays":"NA",
     "Meta Test Description":"This testcase performs a MIPI DSI DBI random payload transfer test. It configures the DSI host PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask for the PHY_STOP_WAIT_TIME field. It configures the packet handler by writing to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with value 0x3d. It configures the clock manager by writing to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with value 0x107. It disables DPI control by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It enables DMAC interrupts by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN. It programs DMA channel instructions by writing to MIZAR_MIPI_DSI_DMAC_DBGINST0 with channel number and debug instruction opcode, writing the descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST1, and executing via MIZAR_MIPI_DSI_DMAC_DBGCMD with value 0x0. This sequence is repeated for two DMA channels (ch0 and ch1). It polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop to wait for DMA channel completion. It clears DMAC interrupts by writing the read data to MIZAR_MIPI_DSI_DMAC_INTCLR.",
     "Meta Test Steps / Procedure":"1. Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field to configure PHY stop wait time. 2. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handler settings. 3. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager. 4. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control. 5. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC channel interrupts for channels 0 and 1. 6. Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 0 debug instruction opcode. 7. Write ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 0 descriptor address. 8. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute channel 0 debug instruction. 9. Write channel 1 debug instruction opcode to MIZAR_MIPI_DSI_DMAC_DBGINST0. 10. Write ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 1 descriptor address. 11. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute channel 1 debug instruction. 12. Poll MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until the expected channel completion bits are set. 13. Write the read interrupt status value to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMAC interrupts.",
     "Meta Impacted Registers":"MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
     "Meta Validation / Acceptance Criteria":"Poll MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until the expected DMA channel completion interrupt bits are set, indicating both channel 0 and channel 1 have completed their DBI random payload transfers. After completion, write the read interrupt status to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the interrupts. The test passes if the DMAC masked interrupt status register indicates successful completion of both DMA channels."},
    {"Index":"3","SS / Module":"MIPI_DSI","Test Case Name":"mipi_dsi_subsys_reg_wr_rd_test","Feature":"Subsystem Register Write-Read Verification","Speed":"NA","Mode":"NA","Memory Start Offset":"NA","Memory End Offset":"NA",
     "Test Description":"This testcase validates the register write-read integrity of MIPI DSI subsystem registers. It reads each register and verifies the reset default value is correct. For writable registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv), it writes a test pattern and reads back to confirm the value was correctly stored. The interrupt_raw register is verified for its reset default value only and is skipped for write operations. The test ensures that all targeted subsystem registers are accessible and retain written values as expected.",
     "Remarks":"The MIPI DSI subsystem base address was not fully resolved by Agent 3 (partially resolved for all five tokens). Source files found in the repository folder contained unrelated PCIe device enumeration code and a timezone Makefile rather than MIPI DSI subsystem register write-read test code; upstream Agent 2, 3, and 4 outputs were used as the authoritative data source for register access and mapping information. The testcase name and Agent 2 token analysis indicate a register write-read verification pattern using addr_array, default_val_array, read_mask_array, write_mask_array, and skip_array to drive an iterative register check loop. The interrupt_raw register is skipped for write operations in this test.",
     "Test Steps / Procedure":"1. Initialize the test with the list of MIPI DSI subsystem register addresses, their expected default values, read masks, write masks, and skip indicators. 2. For each register under test, read the current register value and verify it matches the expected reset default value. 3. For each writable register (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv), write a test data pattern to the register. 4. Read back the written register value and verify it matches the expected written pattern after applying the appropriate masks. 5. Skip write-read verification for the interrupt_raw register as it is marked read-only in this test. 6. Accumulate error counts for reset-value mismatches and write-read mismatches. 7. Report the overall test result as pass if no errors are detected, or fail if any mismatch is found.",
     "Impacted Registers":"data_fifo_threshold_val; low_pwr; dbite; dbi_fdiv; interrupt_raw",
     "Validation / Acceptance Criteria":"Each subsystem register must return its expected reset default value when read after initialization. For writable registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv), the value read back after a write must match the written test pattern after applying the appropriate read and write masks. The interrupt_raw register must return its expected default value on read. The test passes if no reset-value mismatches and no write-read mismatches are detected across all tested registers.",
     "Meta Headers":"NA","Meta Macros":"NA","Meta Arrays":"NA",
     "Meta Test Description":"This testcase performs a register write-read verification test on MIPI DSI subsystem registers. It iterates over an array of subsystem register addresses and performs reset-value checking and write-read-back verification. The registers under test are MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL (offset 0x0), MIZAR_MIPI_DSI_SUBSYS_LOW_PWR (offset 0x4), MIZAR_MIPI_DSI_SUBSYS_DBITE (offset 0x8), MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV (offset 0xC), and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (offset 0x10). For each register, the test reads the current value and compares it against the expected default value. For writable registers (DATA_FIFO_THRESHOLD_VAL, LOW_PWR, DBITE, DBI_FDIV), the test writes a data pattern and reads back to verify the written value matches. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is read-only in this test and is skipped for write operations. The test uses addr_array, default_val_array, read_mask_array, write_mask_array, and skip_array to drive the register verification loop.",
     "Meta Test Steps / Procedure":"1. Initialize the test by loading addr_array with register address macros: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. 2. Load default_val_array with expected reset default values for each register. 3. Load read_mask_array and write_mask_array with the applicable read and write masks for each register. 4. Load skip_array to indicate which registers should be skipped for write operations (MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW has skip=1). 5. For each register in addr_array, call chk_rst_val(): read the register using read_reg(addr), apply read_mask, and compare against the expected default value from default_val_array. 6. For each register in addr_array where skip_array is 0, call chk_rd_wr(): write a test data pattern using write_reg(addr, data_wr) with write_mask applied, then read back using read_reg(addr) with read_mask applied, and compare the read-back value against the expected written value. 7. Track errors in err1 (reset value check errors) and err2 (write-read errors). 8. Report pass or fail based on accumulated error counts.",
     "Meta Impacted Registers":"MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
     "Meta Validation / Acceptance Criteria":"For each register in addr_array, read the register value using read_reg(addr), apply the read_mask from read_mask_array, and compare against the expected default value from default_val_array. If the masked read value does not match the expected default, increment err1. For each register where skip_array is 0, write a test pattern using write_reg(addr, data_wr) with write_mask applied, then read back using read_reg(addr) with read_mask applied, and compare against the expected written value. If the masked read-back value does not match the expected written value, increment err2. The test passes if both err1 and err2 remain zero after all registers have been verified."}
]

TESTPLAN_COLS = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation"]
METADATA_COLS = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]

header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
header_alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
cell_alignment = Alignment(vertical="top", wrap_text=True)
thin_border = Border(left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin"))

def create_workbook():
    wb = openpyxl.Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    for col_idx, col_name in enumerate(TESTPLAN_COLS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment
        cell.border = thin_border
    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(TESTPLAN_COLS, 1):
            value = row_data.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_alignment
            cell.border = thin_border
    ws_tp.freeze_panes = "A2"
    for col_idx in range(1, len(TESTPLAN_COLS) + 1):
        max_len = len(TESTPLAN_COLS[col_idx - 1])
        for row_idx in range(2, len(json_data) + 2):
            val = ws_tp.cell(row=row_idx, column=col_idx).value
            if val:
                max_len = max(max_len, min(len(str(val)), 80))
        ws_tp.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 4, 60)

    ws_md = wb.create_sheet("MetaData")
    for col_idx, col_name in enumerate(METADATA_COLS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment
        cell.border = thin_border
    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(METADATA_COLS, 1):
            value = row_data.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_alignment
            cell.border = thin_border
    ws_md.freeze_panes = "A2"
    for col_idx in range(1, len(METADATA_COLS) + 1):
        max_len = len(METADATA_COLS[col_idx - 1])
        for row_idx in range(2, len(json_data) + 2):
            val = ws_md.cell(row=row_idx, column=col_idx).value
            if val:
                max_len = max(max_len, min(len(str(val)), 80))
        ws_md.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 4, 60)
    ws_md.sheet_state = "veryHidden"

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_path = os.path.join(script_dir, FILENAME)
    wb.save(output_path)
    print(f"Saved: {output_path} ({os.path.getsize(output_path)} bytes)")

    wb2 = openpyxl.load_workbook(output_path)
    assert "TestPlan" in wb2.sheetnames
    assert "MetaData" in wb2.sheetnames
    assert wb2["MetaData"].sheet_state == "veryHidden"
    print("Validation: PASSED")

    with open(output_path, "rb") as f:
        b64_content = base64.b64encode(f.read()).decode("utf-8")
    print(f"BASE64_START\n{b64_content}\nBASE64_END")
    return output_path

if __name__ == "__main__":
    create_workbook()
