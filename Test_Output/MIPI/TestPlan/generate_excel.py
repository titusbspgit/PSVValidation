#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import json
import os
import sys

json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "DSI Host and DMAC Basic Interrupt-Driven Data Transfer",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW; MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR; MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME; MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "Interrupt Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs a basic MIPI DSI data transfer using the DMAC (DMA Controller) with interrupt-driven completion. It first enables DMAC interrupts by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN. It then enables the subsystem-level GDMA interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE using the MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR bit. The DSI host PHY interface is configured by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME. The packet handler is configured by writing 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG. The clock manager is configured by writing 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. DPI control is disabled by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. DMAC channel 0 and channel 1 descriptor addresses are loaded via MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and MIZAR_MIPI_DSI_DMAC_DBGCMD debug instruction registers. The test then enters a polling loop reading MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and checking for the MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit to detect GDMA interrupt assertion. Upon interrupt detection, MIZAR_MIPI_DSI_DMAC_INTMIS is read to determine the DMAC channel interrupt status. Interrupts are cleared by writing to MIZAR_MIPI_DSI_DMAC_INTCLR and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.",
        "Test Description": "This test validates basic MIPI DSI interrupt-driven data transfer using the DMAC. It configures the DSI host by setting up the PHY interface (PHY_IF_CFG), packet handler (PCKHDL_CFG), and clock manager (CLKMGR_CFG). It enables DMAC and subsystem-level GDMA interrupts via the interrupt_enable register. DPI control is disabled via the dpi_control register. DMAC channels are programmed with descriptor addresses through debug instruction registers. The test polls the interrupt_mask register for GDMA interrupt assertion, reads the DMAC masked interrupt status to identify the triggering channel, and clears interrupts through the DMAC interrupt clear register and the subsystem interrupt_raw register.",
        "Meta Test Steps / Procedure": "1. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC interrupts for channels 0 and 1. 2. Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR bit set to enable GDMA interrupt at subsystem level. 3. Configure PHY interface by calling set_data_mask on MIZAR_MIPI_DSI_HOST_PHY_IF_CFG with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field and writing the result. 4. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handler. 5. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager. 6. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control. 7. Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 0 descriptor setup. 8. Write channel 0 descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST1. 9. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the debug instruction for channel 0. 10. Repeat steps 7-9 for channel 1 descriptor setup. 11. Enter polling loop: read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and check if MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit is set. 12. On GDMA interrupt detection, read MIZAR_MIPI_DSI_DMAC_INTMIS to get DMAC channel interrupt status. 13. Write channel mask status to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMAC interrupts. 14. Write subsystem mask status to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem interrupts.",
        "Test Steps / Procedure": "1. Enable DMAC interrupts for channels 0 and 1 by writing to the DMAC interrupt enable register. 2. Enable the GDMA interrupt at the subsystem level by writing to the interrupt_enable register. 3. Configure the DSI host PHY interface by setting the PHY stop wait time field in the PHY_IF_CFG register. 4. Configure the packet handler by writing to the PCKHDL_CFG register. 5. Configure the clock manager by writing to the CLKMGR_CFG register. 6. Disable DPI control by writing to the dpi_control register. 7. Program DMAC channel 0 descriptor address using the DMAC debug instruction registers. 8. Execute the DMAC debug command to start channel 0. 9. Program DMAC channel 1 descriptor address using the DMAC debug instruction registers. 10. Execute the DMAC debug command to start channel 1. 11. Poll the interrupt_mask register until the GDMA interrupt bit is asserted. 12. Read the DMAC masked interrupt status register to identify the interrupting channel. 13. Clear the DMAC channel interrupt by writing to the DMAC interrupt clear register. 14. Clear the subsystem interrupt by writing to the interrupt_raw register.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; interrupt_mask; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK in a loop and checks whether the MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit is set. When the GDMA interrupt is detected, MIZAR_MIPI_DSI_DMAC_INTMIS is read to determine the DMAC channel interrupt status. The test passes if the GDMA interrupt is successfully detected via the polling loop, the DMAC channel interrupt status is read, and all interrupts are properly cleared by writing to MIZAR_MIPI_DSI_DMAC_INTCLR and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.",
        "Validation / Acceptance Criteria": "The test passes when the GDMA interrupt is detected by polling the interrupt_mask register. Upon detection, the DMAC masked interrupt status is read to confirm the interrupting channel. All DMAC and subsystem interrupts must be successfully cleared. The test validates that the DSI host configuration (PHY_IF_CFG, PCKHDL_CFG, CLKMGR_CFG), DMAC channel programming, interrupt generation, and interrupt clearing all function correctly in an end-to-end basic data transfer scenario.",
        "Remarks": "Six DMAC registers (interrupt enable, debug instruction 0, debug instruction 1, debug command, masked interrupt status, and interrupt clear) could not be mapped to canonical register names because no DMAC register specification document was provided. The test uses interrupt-driven polling to detect DMAC transfer completion. DPI control is explicitly disabled, indicating the test operates in command mode rather than video mode."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_dbi_random_payload_test",
        "Feature": "DBI Random Payload Data Transfer via DMAC",
        "Meta Headers": "NA",
        "Meta Macros": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME; DSI_WRITE_MEMORY_START; DMA_SAR; DMA_DAR; RAM_BASE",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "DBI Command Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs a MIPI DSI DBI (Display Bus Interface) random payload data transfer using the DMAC. The DSI host PHY interface is configured by writing to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with the MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field. The packet handler is configured by writing 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG. The clock manager is configured by writing 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. DPI control is disabled by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, indicating DBI command mode operation. DMAC interrupts are enabled by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN for channels 0 and 1. A DBI write memory start command is loaded via load_wr_command() with DSI_WRITE_MEMORY_START. DMA source and destination addresses are configured using DMAMOV() with DMA_SAR and DMA_DAR, referencing RAM_BASE as the memory base. DMAC channel 0 descriptor address is programmed by writing to MIZAR_MIPI_DSI_DMAC_DBGINST0 (0x00A00000), MIZAR_MIPI_DSI_DMAC_DBGINST1 (channel 0 descriptor address), and MIZAR_MIPI_DSI_DMAC_DBGCMD (0x0). Channel 1 is similarly programmed with MIZAR_MIPI_DSI_DMAC_DBGINST0 (0x01A00000). The test then polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until the value equals 0x3, indicating both channels have completed. Upon completion, MIZAR_MIPI_DSI_DMAC_INTCLR is written with the read data to clear the DMAC interrupts.",
        "Test Description": "This test validates MIPI DSI DBI random payload data transfer using the DMAC. It configures the DSI host by setting up the PHY interface (PHY_IF_CFG) with the PHY stop wait time field, the packet handler (PCKHDL_CFG), and the clock manager (CLKMGR_CFG). DPI control is disabled via the dpi_control register to operate in DBI command mode. DMAC interrupts are enabled for both channels. A DBI write memory start command is loaded, and DMA source and destination addresses are configured. Both DMAC channels are programmed with descriptor addresses through the DMAC debug instruction registers and started. The test polls the DMAC masked interrupt status register until both channels report completion, then clears the DMAC interrupts through the interrupt clear register.",
        "Meta Test Steps / Procedure": "1. Configure PHY interface by calling set_data_mask on MIZAR_MIPI_DSI_HOST_PHY_IF_CFG with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field and writing the result via write_reg. 2. Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure the packet handler. 3. Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager. 4. Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control for DBI command mode. 5. Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMAC interrupts for channels 0 and 1. 6. Load DBI write memory start command via load_wr_command() with DSI_WRITE_MEMORY_START. 7. Configure DMA source and destination addresses using DMAMOV() with DMA_SAR and DMA_DAR, referencing RAM_BASE. 8. Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 0 descriptor setup. 9. Write channel 0 descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST1. 10. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the debug instruction for channel 0. 11. Write 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 1 descriptor setup. 12. Write channel 1 descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST1. 13. Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the debug instruction for channel 1. 14. Poll MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until the value equals 0x3 (both channels complete). 15. Write the polled value to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMAC interrupts.",
        "Test Steps / Procedure": "1. Configure the DSI host PHY interface by setting the PHY stop wait time field in the PHY_IF_CFG register. 2. Configure the packet handler by writing to the PCKHDL_CFG register. 3. Configure the clock manager by writing to the CLKMGR_CFG register. 4. Disable DPI control by writing to the dpi_control register to operate in DBI command mode. 5. Enable DMAC interrupts for channels 0 and 1 by writing to the DMAC interrupt enable register. 6. Load the DBI write memory start command for the DSI transfer. 7. Configure DMA source and destination addresses for the random payload data. 8. Program DMAC channel 0 descriptor address using the DMAC debug instruction registers and execute the debug command. 9. Program DMAC channel 1 descriptor address using the DMAC debug instruction registers and execute the debug command. 10. Poll the DMAC masked interrupt status register until both channels report transfer completion. 11. Clear the DMAC interrupts by writing to the DMAC interrupt clear register.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop and checks whether the value equals 0x3, indicating that both DMAC channel 0 and channel 1 have completed their transfers. The polling pattern is: rd_data = read_reg(MIZAR_MIPI_DSI_DMAC_INTMIS); while(rd_data != 0x3) { rd_data = read_reg(MIZAR_MIPI_DSI_DMAC_INTMIS); }. The test passes if both DMAC channels complete successfully (polled value equals 0x3) and the DMAC interrupts are properly cleared by writing the polled value to MIZAR_MIPI_DSI_DMAC_INTCLR.",
        "Validation / Acceptance Criteria": "The test passes when the DMAC masked interrupt status register reports that both channels have completed their transfers. The polling loop waits until both channel completion bits are set. Upon successful completion, all DMAC interrupts must be properly cleared through the DMAC interrupt clear register. The test validates that the DSI host configuration (PHY_IF_CFG, PCKHDL_CFG, CLKMGR_CFG), DBI command mode setup via dpi_control, random payload DMA transfer across both channels, and interrupt-based completion detection all function correctly.",
        "Remarks": "Six DMAC registers (interrupt enable, debug instruction 0, debug instruction 1, debug command, masked interrupt status, and interrupt clear) could not be mapped to canonical register names because no DMAC register specification document was provided. The test uses DBI command mode with DPI control explicitly disabled. Random payload data is transferred using a DBI write memory start command. The polling loop checks for value 0x3 indicating both DMAC channel 0 and channel 1 completion. The repository source files do not contain the actual MIPI DSI testcase code; the upstream agent analysis was used as the authoritative source for register access behavior."
    },
    {
        "Index": "3",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "Subsystem Register Write-Read Verification",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW; MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL; MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL; MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL; MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL; MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL; MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WRITE_MASK; MIPI_DSI_SUBSYS_LOW_PWR_WRITE_MASK; MIPI_DSI_SUBSYS_DBITE_WRITE_MASK; MIPI_DSI_SUBSYS_DBI_FDIV_WRITE_MASK; MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_READ_MASK; MIPI_DSI_SUBSYS_LOW_PWR_READ_MASK; MIPI_DSI_SUBSYS_DBITE_READ_MASK; MIPI_DSI_SUBSYS_DBI_FDIV_READ_MASK; MIPI_DSI_SUBSYS_INTERRUPT_RAW_READ_MASK; CNT; SOFT_RST_REG_DATA",
        "Meta Arrays": "addr_array[52]; rst_val_array[52]; wr_mask_array[52]; rd_mask_array[52]; skip_array[52]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase verifies the reset default values and write-read functionality of 5 MIPI DSI subsystem registers. The test is structured in two phases. Phase 1 (chk_rst_val): Iterates over addr_array[] containing MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. For each register, it calls read_reg(addr_array[i]) and compares the read value against rst_val_array[i] (the expected reset default value). Mismatches increment err1. Phase 2 (chk_rd_wr): Performs a soft reset, then iterates over the same addr_array[]. For each register where skip_array[i] is not 1, it writes wr_mask_array[i] via write_reg(addr_array[i], wr_mask_array[i]), reads back via read_reg(addr_array[i]), and compares the read-back value against rd_mask_array[i]. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW has skip_array[4]=1, so it is skipped during the write-read phase and is only verified for its reset default value. Mismatches in phase 2 increment err2. The test reports pass if both err1 and err2 are zero.",
        "Test Description": "This test verifies the reset default values and write-read functionality of five MIPI DSI subsystem registers: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, and interrupt_raw. In the first phase, each register is read and its value is compared against the expected reset default. In the second phase, after a soft reset, each writable register is written with a test pattern and read back to verify correctness. The interrupt_raw register is verified for its reset default value only and is skipped during the write-read phase. The test passes when all reset value checks and all write-read comparisons match their expected values.",
        "Meta Test Steps / Procedure": "1. Initialize error counters err1 and err2 to zero. 2. Phase 1 - chk_rst_val: Loop from i=0 to CNT-1 (5 registers). 3. For each iteration, call read_reg(addr_array[i]) to read the current register value. 4. Compare the read value against rst_val_array[i] (expected reset default value). 5. If mismatch, increment err1 and log the address, expected value, and actual value. 6. Perform a soft reset by writing SOFT_RST_REG_DATA to SOFT_RST_REG_ADDRESS. 7. Phase 2 - chk_rd_wr: Loop from i=0 to CNT-1 (5 registers). 8. Check skip_array[i]; if skip_array[i]==1, skip this register (MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW at index 4 is skipped). 9. For non-skipped registers, call write_reg(addr_array[i], wr_mask_array[i]) to write the test pattern. 10. Call read_reg(addr_array[i]) to read back the written value. 11. Compare the read-back value against rd_mask_array[i] (expected read-back value after write). 12. If mismatch, increment err2 and log the address, expected value, and actual value. 13. Report pass if err1==0 and err2==0; report fail otherwise.",
        "Test Steps / Procedure": "1. Read all five subsystem registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw) and verify each value matches its expected reset default. 2. Record any reset value mismatches. 3. Perform a soft reset of the subsystem. 4. For each writable register (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv), write a test pattern value. 5. Read back each written register and compare the read-back value against the expected value. 6. Skip the write-read verification for the interrupt_raw register as it is flagged as read-only in this test. 7. Record any write-read mismatches. 8. Report pass if all reset default checks and all write-read checks succeed with zero errors.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "data_fifo_threshold_val; low_pwr; dbite; dbi_fdiv; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "Phase 1 validation: For each register in addr_array[], the value returned by read_reg(addr_array[i]) must exactly match rst_val_array[i]. Any mismatch increments err1. Phase 2 validation: For each non-skipped register (skip_array[i]!=1), after write_reg(addr_array[i], wr_mask_array[i]), the value returned by read_reg(addr_array[i]) must exactly match rd_mask_array[i]. Any mismatch increments err2. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (skip_array[4]=1) is excluded from phase 2 validation. Overall pass condition: err1==0 AND err2==0.",
        "Validation / Acceptance Criteria": "The test passes when all five subsystem registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw) return their expected reset default values during the reset value check phase. Additionally, the four writable registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv) must return the expected values after being written with test patterns during the write-read verification phase. The interrupt_raw register is only validated for its reset default value. Both error counters must remain at zero for the test to pass.",
        "Remarks": "All five subsystem registers were successfully mapped to canonical register names from the specification document. The interrupt_raw register is skipped during the write-read phase due to the skip_array flag, indicating it is treated as read-only in this test context. The test uses parallel arrays (addr_array, rst_val_array, wr_mask_array, rd_mask_array, skip_array) each of size 52, but only 5 entries are populated for this subsystem register test. A soft reset is performed between the reset value check phase and the write-read phase. The repository source files do not contain the actual MIPI DSI testcase code; the upstream agent analysis was used as the authoritative source for register access behavior."
    }
]

def generate_excel():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"MIPI_DSI_TestPlan_{timestamp_str}.xlsx"

    wb = openpyxl.Workbook()

    # TestPlan sheet
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    tp_columns = [
        "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
        "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
        "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
        "Code Generation"
    ]

    # MetaData sheet
    ws_md = wb.create_sheet(title="MetaData")

    md_columns = [
        "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
        "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
        "Meta Headers", "Meta Macros", "Meta Arrays"
    ]

    # Formatting
    header_font = Font(bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_alignment = Alignment(wrap_text=True, vertical="top")

    # Write TestPlan headers
    for col_idx, col_name in enumerate(tp_columns, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    # Write TestPlan data
    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(tp_columns, 1):
            value = row_data.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

    # Write MetaData headers
    for col_idx, col_name in enumerate(md_columns, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    # Write MetaData data
    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(md_columns, 1):
            value = row_data.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

    # Auto-size columns
    max_width = 60
    for ws in [ws_tp, ws_md]:
        for col in ws.columns:
            max_len = 0
            col_letter = col[0].column_letter
            for cell in col:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
            adjusted = min(max_len + 2, max_width)
            ws.column_dimensions[col_letter].width = max(adjusted, 12)

    # Freeze first row
    ws_tp.freeze_panes = "A2"
    ws_md.freeze_panes = "A2"

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save
    output_path = os.path.join(os.getcwd(), filename)
    wb.save(output_path)
    wb.close()

    # Validate
    if os.path.exists(output_path) and os.path.getsize(output_path) > 0:
        vb = openpyxl.load_workbook(output_path)
        sheets = vb.sheetnames
        vb.close()
        if "TestPlan" in sheets and "MetaData" in sheets:
            print(f"VALIDATION_PASSED|{filename}|{output_path}")
            return filename, output_path
    print("VALIDATION_FAILED")
    return None, None

if __name__ == "__main__":
    fn, fp = generate_excel()
    if fn:
        print(f"Generated: {fn}")
    else:
        print("Generation failed")
        sys.exit(1)
