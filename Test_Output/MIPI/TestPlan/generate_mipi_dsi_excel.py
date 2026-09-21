#!/usr/bin/env python3
"""One-shot Excel generator - creates MIPI_DSI TestPlan workbook with current data."""
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
        "Feature": "DBI Data Transfer with DMAC and Interrupt Handling",
        "Test Description": "This testcase validates basic MIPI DSI DBI data transfer using DMAC. It enables DMAC and subsystem interrupts, configures the PHY interface through PHY_IF_CFG, sets up packet handling via PCKHDL_CFG, and configures the clock manager through CLKMGR_CFG. DPI control is disabled via dpi_control. Two DMAC channels are programmed with descriptor addresses using DMAC debug instruction registers. The test polls the interrupt_mask register for GDMA interrupt assertion and reads the DMAC masked interrupt status to identify which channel completed. Upon completion, DMAC and subsystem interrupts are cleared through the respective interrupt clear and interrupt_raw registers.",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "Six DMAC registers (DMAC interrupt enable, DMAC masked interrupt status, DMAC interrupt clear, DMAC debug instruction 0, DMAC debug instruction 1, DMAC debug command) could not be mapped to canonical register names because no DMAC register specification document was provided. The source files in the repository folder contain PCIe content rather than MIPI DSI content; testcase behavior was derived from upstream agent register-access outputs. The test uses polling-based interrupt detection on the subsystem interrupt mask register.",
        "Test Steps / Procedure": "1. Enable DMAC channel 0 and channel 1 interrupts by writing to the DMAC interrupt enable register. 2. Enable the GDMA interrupt in the subsystem interrupt_enable register. 3. Configure the PHY interface by setting the stop wait time field in PHY_IF_CFG. 4. Configure packet handling options (EoTp TX/RX, BTA, ECC, CRC) by writing to PCKHDL_CFG. 5. Configure clock manager escape clock division and timeout clock division by writing to CLKMGR_CFG. 6. Disable DPI control by writing to dpi_control. 7. Program DMAC channel 0 by writing the Go instruction and descriptor address through the DMAC debug instruction registers, then execute the instruction via the DMAC debug command register. 8. Program DMAC channel 1 by writing the Go instruction and descriptor address through the DMAC debug instruction registers, then execute the instruction via the DMAC debug command register. 9. Poll the interrupt_mask register for GDMA interrupt assertion to detect transfer completion. 10. Read the DMAC masked interrupt status register to identify which channel completed. 11. Clear the DMAC channel interrupt by writing to the DMAC interrupt clear register. 12. Clear the subsystem interrupt by writing to the interrupt_raw register.",
        "Impacted Registers": "interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; interrupt_raw; interrupt_mask",
        "Validation / Acceptance Criteria": "The test passes when the GDMA interrupt is asserted in the interrupt_mask register, confirming DMAC transfer completion. The DMAC masked interrupt status register is read to verify which channel completed. Both DMAC channel interrupts and the subsystem interrupt must be successfully cleared. The test validates that DBI data transfer via DMAC completes without errors for both channels, and that the interrupt signaling and clearing mechanism works correctly through the interrupt_enable, interrupt_mask, interrupt_raw, PHY_IF_CFG, PCKHDL_CFG, and CLKMGR_CFG registers.",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Meta Test Description": "This testcase performs a basic MIPI DSI DBI data transfer using DMAC. It enables DMAC interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTEN with value 0x3. It enables the subsystem GDMA interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE using MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR. It configures the PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG with a computed phy_if_cfg value derived from set_data_mask using MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. It configures packet handling via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with value 0x3d and clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with value 0x107. It disables DPI control by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It programs DMAC channels by writing to MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and MIZAR_MIPI_DSI_DMAC_DBGCMD for both channel 0 and channel 1 descriptor addresses. It then polls MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK checking for MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR and reads MIZAR_MIPI_DSI_DMAC_INTMIS to determine channel completion status. Upon completion, it clears DMAC interrupts via MIZAR_MIPI_DSI_DMAC_INTCLR and subsystem interrupts via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.",
        "Meta Test Steps / Procedure": "1. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC channel 0 and channel 1 interrupts. 2. Write MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable GDMA interrupt in the subsystem. 3. Compute phy_if_cfg using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME and write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. 4. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling (eotp_tx_en, eotp_rx_en, bta_en, ecc_rx_en, crc_rx_en, eotp_tx_lp_en). 5. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager (tx_esc_clk_division and to_clk_division). 6. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control. 7. Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 0 DMAC Go instruction. 8. Write ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 0 descriptor address. 9. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute channel 0 DMAC instruction. 10. Write 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 1 DMAC Go instruction. 11. Write ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 1 descriptor address. 12. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute channel 1 DMAC instruction. 13. Poll MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and check for MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit assertion. 14. Read MIZAR_MIPI_DSI_DMAC_INTMIS to determine which DMAC channel triggered the interrupt. 15. Write ch_mask_st to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMAC channel interrupt. 16. Write dsi_subsys_mask_st to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear the subsystem interrupt.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and checks if the MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit is asserted, indicating GDMA interrupt has fired. It then reads MIZAR_MIPI_DSI_DMAC_INTMIS to determine the channel mask status (ch_mask_st). If ch_mask_st indicates channel completion, the test clears the DMAC interrupt via MIZAR_MIPI_DSI_DMAC_INTCLR and the subsystem interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. The test passes if both DMAC channels complete their DBI data transfers and all interrupts are successfully cleared.",
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_dbi_random_payload_test",
        "Feature": "DBI Random Payload Data Transfer with DMAC",
        "Test Description": "This testcase validates MIPI DSI DBI data transfer with random payload sizes using DMAC. It configures the PHY interface through PHY_IF_CFG, sets up packet handling via PCKHDL_CFG, and configures the clock manager through CLKMGR_CFG. DPI control is disabled via dpi_control. DMAC interrupts are enabled, and two DMAC channels are programmed with descriptor addresses using DMAC debug instruction registers. The test polls the DMAC masked interrupt status register to detect transfer completion. Upon completion, the DMAC interrupt is cleared. This transfer process is repeated 10 times with randomly generated payload sizes to validate robustness of the DBI data path.",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "Six DMAC registers (DMAC interrupt enable, DMAC masked interrupt status, DMAC interrupt clear, DMAC debug instruction 0, DMAC debug instruction 1, DMAC debug command) could not be mapped to canonical register names because no DMAC register specification document was provided. The source files in the repository folder contain PCIe content rather than MIPI DSI content; testcase behavior was derived from upstream agent register-access outputs. The test uses polling-based completion detection on the DMAC masked interrupt status register and iterates 10 times with random payload sizes.",
        "Test Steps / Procedure": "1. Configure the PHY interface by setting the stop wait time field in PHY_IF_CFG. 2. Configure packet handling options (EoTp TX/RX, BTA, ECC, CRC) by writing to PCKHDL_CFG. 3. Configure clock manager escape clock division and timeout clock division by writing to CLKMGR_CFG. 4. Disable DPI control by writing to dpi_control. 5. Enable DMAC channel 0 and channel 1 interrupts by writing to the DMAC interrupt enable register. 6. Begin iteration loop for 10 random payload transfer cycles. 7. Program DMAC channel 0 by writing the Go instruction and descriptor address through the DMAC debug instruction registers, then execute the instruction via the DMAC debug command register. 8. Program DMAC channel 1 by writing the Go instruction and descriptor address through the DMAC debug instruction registers, then execute the instruction via the DMAC debug command register. 9. Poll the DMAC masked interrupt status register until transfer completion is detected. 10. Clear the DMAC channel interrupt by writing the read-back status to the DMAC interrupt clear register. 11. Repeat steps 7 through 10 for all 10 iterations with different random payload sizes.",
        "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control",
        "Validation / Acceptance Criteria": "The test passes when the DMAC masked interrupt status register indicates transfer completion for each of the 10 random payload iterations. The DMAC interrupt must be successfully cleared after each iteration. All 10 DBI data transfers with random payload sizes must complete without errors, validating that the PHY_IF_CFG, PCKHDL_CFG, CLKMGR_CFG, and dpi_control configurations support reliable data transfer across varying payload sizes.",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Meta Test Description": "This testcase performs MIPI DSI DBI random payload data transfer using DMAC. It configures the PHY interface by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG with a computed phy_if_cfg value derived from set_data_mask using MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. It configures packet handling via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with value 0x3d and clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with value 0x107. It disables DPI control by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It enables DMAC interrupts by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN. It programs DMAC channel 0 by writing 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0, ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1, and 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD. It programs DMAC channel 1 by writing 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0, ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1, and 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD. It polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop to detect DMAC channel completion. Upon completion, it clears the DMAC interrupt by writing rd_data to MIZAR_MIPI_DSI_DMAC_INTCLR. The test iterates this transfer process 10 times with random payload sizes.",
        "Meta Test Steps / Procedure": "1. Compute phy_if_cfg using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME and write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. 2. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling (eotp_tx_en, eotp_rx_en, bta_en, ecc_rx_en, crc_rx_en, eotp_tx_lp_en). 3. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager (tx_esc_clk_division and to_clk_division). 4. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control. 5. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC channel 0 and channel 1 interrupts. 6. Begin iteration loop (10 iterations with random payload sizes). 7. Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 0 DMAC Go instruction. 8. Write ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 0 descriptor address. 9. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute channel 0 DMAC instruction. 10. Write 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 1 DMAC Go instruction. 11. Write ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 1 descriptor address. 12. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute channel 1 DMAC instruction. 13. Poll MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until DMAC channel completion is detected. 14. Read rd_data from MIZAR_MIPI_DSI_DMAC_INTMIS to capture interrupt status. 15. Write rd_data to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMAC channel interrupt. 16. End iteration loop after 10 iterations.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop to detect when the DMAC channel transfer completes. The read value (rd_data) from MIZAR_MIPI_DSI_DMAC_INTMIS is used to determine completion status. Once a non-zero value is detected indicating channel completion, the test writes rd_data to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the interrupt. The test passes if all 10 iterations of random payload DBI transfers complete successfully with DMAC interrupts properly asserted and cleared each time.",
    },
    {
        "Index": "3",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "MIPI DSI Subsystem Register Write/Read Verification",
        "Test Description": "This testcase validates the MIPI DSI subsystem registers by performing register write and read-back operations on five subsystem registers: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, and interrupt_raw. The test first verifies that each register contains its expected reset default value. It then writes six different data patterns to each register and reads back the values to confirm correct write/read behavior. Write and read masks are applied to account for reserved or read-only bit fields. The test tracks error counts for both the default value verification phase and the write/read-back verification phase.",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "The source files in the repository folder contain PCIe content rather than MIPI DSI content; testcase behavior was derived from upstream agent register-access outputs and prior workflow context. All 5 registers are in the MIPI DSI SUBSYS block at base address 0xE6A41000 and were successfully mapped to canonical register names from the mipi_dsi_subsys_autoreg specification. The test uses write and read masks to handle reserved and read-only bit fields in each register. Six data patterns are used to exercise all-ones, all-zeros, alternating-bit, and mixed-bit scenarios.",
        "Test Steps / Procedure": "1. Initialize error tracking for default value verification and write/read-back verification. 2. Read each of the five subsystem registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw) and verify that each contains its expected reset default value. 3. Record any mismatches found during default value verification. 4. Perform a soft reset of the subsystem to restore registers to a known state. 5. For each of six data patterns, write the pattern to each of the five subsystem registers with appropriate write masks applied. 6. Read back each register after writing and compare the read value (with read mask applied) against the expected value. 7. Record any mismatches found during write/read-back verification. 8. Evaluate the total error counts from both verification phases to determine overall test pass or fail.",
        "Impacted Registers": "data_fifo_threshold_val; low_pwr; dbite; dbi_fdiv; interrupt_raw",
        "Validation / Acceptance Criteria": "The test passes when all five subsystem registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw) contain their expected reset default values, and all write/read-back operations across six data patterns produce matching results after applying write and read masks. Zero errors must be recorded in both the default value verification phase and the write/read-back verification phase for the test to pass.",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "debug_print(...); LOGI(...); CTRL_REG_ADDR; SYNC_HANDSHAKE_VALUE; SII_LINK_STATUS_OFFSET; SII_LINK_UP_MASK; PCIE_SLV_VENDOR_ID_OFFSET; PCIE_SLV_CMD_STATUS_OFFSET; PCIE_CMD_MEM_IO_BUSMASTER; BAR0_REG_OFFSET; BAR1_REG_OFFSET; SEC_LAT_TIMER_OFFSET; SEC_STAT_IO_OFFSET; MEM_LIMIT_OFFSET; PREF_MEM_LIMIT_OFFSET; BAR_ENUM_PATTERN; BAR0_BASE_ADDR; BAR1_BASE_ADDR; BAR2_BASE_ADDR; BAR3_BASE_ADDR; BAR4_BASE_ADDR; BAR5_BASE_ADDR; SYS_REG_0; SYS_REG_1; SYS_REG_2; SYS_REG_3; SYS_REG_4; SYS_REG_5; TESTS_ITEM_DEFINED; TEST_OUTPUT_DEFINED",
        "Meta Arrays": "NA",
        "Meta Test Description": "This testcase validates the MIPI DSI subsystem registers by performing register write and read-back operations. It targets 5 SUBSYS registers accessed via addr_array: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL (base 0xE6A41000, offset 0x0), MIZAR_MIPI_DSI_SUBSYS_LOW_PWR (base 0xE6A41000, offset 0x4), MIZAR_MIPI_DSI_SUBSYS_DBITE (base 0xE6A41000, offset 0x8), MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV (base 0xE6A41000, offset 0xC), and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (base 0xE6A41000, offset 0x10). The test first reads each register via read_reg(addr) in a chk_rst_val() function to verify reset default values against expected default value arrays. It then performs write/read-back verification in a chk_rd_wr() function by iterating over 6 data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xFFFF0000), writing each pattern via write_reg(addr, data_wr) with write masks applied, then reading back via read_reg(addr) and comparing with read masks applied. A soft reset is performed between default value check and write/read phases using SOFT_RST_REG_ADDRESS (excluded per instruction). Error counters err1 and err2 track mismatches for default value checks and write/read-back checks respectively.",
        "Meta Test Steps / Procedure": "1. Initialize error counters err1 and err2 to 0. 2. Call chk_rst_val() which iterates over addr_array containing MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. 3. For each register in addr_array, call read_reg(addr) and compare the read value against the corresponding default value from the default value array. 4. If the read value does not match the expected default value, increment err1. 5. Perform a soft reset sequence (SOFT_RST_REG_ADDRESS operations excluded per instruction). 6. Call chk_rd_wr() which iterates over 6 data patterns: 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xFFFF0000. 7. For each data pattern, iterate over all 5 registers in addr_array. 8. For each register, compute data_wr by applying the write mask to the data pattern. 9. Call write_reg(addr, data_wr) to write the masked pattern to the register. 10. Call read_reg(addr) to read back the register value. 11. Apply the read mask to the read-back value and compare against the expected value (data pattern masked with both write and read masks). 12. If the masked read-back value does not match the expected value, increment err2. 13. After all patterns and registers are tested, evaluate err1 and err2 to determine pass or fail.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Meta Validation / Acceptance Criteria": "In chk_rst_val(), each register read via read_reg(addr) is compared against its corresponding default value from the default value array. A mismatch increments err1. In chk_rd_wr(), for each of the 6 data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xFFFF0000), the value written via write_reg(addr, data_wr) is read back via read_reg(addr), and the masked read-back value is compared against the expected masked write value. A mismatch increments err2. The test passes if both err1 == 0 and err2 == 0, meaning all 5 registers have correct default values and all 30 write/read-back operations (5 registers x 6 patterns) produce matching results.",
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

print(f"FILENAME={filename}")
print(f"FILEPATH={filepath}")
print(f"FILESIZE={os.path.getsize(filepath)}")
print("VALIDATION=PASSED")
