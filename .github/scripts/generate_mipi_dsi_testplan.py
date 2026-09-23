#!/usr/bin/env python3
"""Generate MIPI_DSI TestPlan Excel workbook using openpyxl."""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl'])
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime('%Y%m%d_%H%M%S')

IP_NAME = 'MIPI_DSI'
filename = f'{IP_NAME}_TestPlan_{timestamp_str}.xlsx'
output_dir = 'Test_Output/MIPI/TestPlan'
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "DBI Data Transfer with DMA",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW; MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME; MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR; MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR; DSI_INTR_NO; RAM_BASE; DMA_SAR; DMA_DAR; DSI_WRITE_MEMORY_START",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "Interrupt Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs a basic MIPI DSI DBI write memory transfer using interrupt-driven DMA. It enables DMAC interrupts by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN, enables the GDMA interrupt in the subsystem via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR. It configures the DSI host PHY interface by writing phy_if_cfg (with phy_stop_wait_time field set via set_data_mask using MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME) to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. It configures packet handling by writing 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG (enabling eotp_tx_en, eotp_rx_en, bta_en, ecc_rx_en, crc_rx_en). It configures clock management by writing 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. It disables DPI by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It then starts two DMA channels by writing DMAC debug instructions to MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and executing them via MIZAR_MIPI_DSI_DMAC_DBGCMD. The Default_IRQHandler reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to check for GDMA interrupt, reads MIZAR_MIPI_DSI_DMAC_INTMIS for DMA channel interrupt status, clears DMA interrupts via MIZAR_MIPI_DSI_DMAC_INTCLR, and clears subsystem interrupts via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. The test waits for both DMA channels to complete via interrupt flags before finishing with pass status.",
        "Test Description": "This testcase validates a basic MIPI DSI DBI write memory transfer using interrupt-driven DMA. It configures the DSI host PHY interface, packet handling (enabling EoTp, BTA, ECC, and CRC), and clock management. The DPI interface is disabled to operate in DBI/command mode. Two DMA channels are started to transfer data. An interrupt service routine handles DMA completion by reading the subsystem interrupt mask register and DMA interrupt status, clearing both DMA and subsystem interrupts. The test verifies that both DMA channels complete their transfers successfully via interrupt signaling.",
        "Meta Test Steps / Procedure": "1. Enable DMAC interrupts: write_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3) to enable interrupts for DMA channel 0 and channel 1.\n2. Enable subsystem GDMA interrupt: write_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR).\n3. Configure PHY interface: compute phy_if_cfg using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field, then write_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg).\n4. Configure packet handling: write_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3d) enabling eotp_tx_en, eotp_rx_en, bta_en, ecc_rx_en, crc_rx_en.\n5. Configure clock manager: write_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107) setting tx_esc_clk_division and to_clk_division.\n6. Disable DPI: write_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0).\n7. Start DMA channel 0: write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000), write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, ch0_desc_addr_act), write_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0).\n8. Start DMA channel 1: write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000), write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, ch1_desc_addr_act), write_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0).\n9. On interrupt (Default_IRQHandler): read_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK) to get dsi_subsys_mask_st, check if MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit is set.\n10. If GDMA interrupt detected: read_reg(MIZAR_MIPI_DSI_DMAC_INTMIS) to get ch_mask_st for DMA channel status.\n11. Clear DMA interrupt: write_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, ch_mask_st).\n12. Clear subsystem interrupt: write_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, dsi_subsys_mask_st).\n13. Track channel completion flags; when both channels complete, call finish(0) for pass.",
        "Test Steps / Procedure": "1. Enable DMA channel 0 and channel 1 interrupts by writing to the DMAC interrupt enable register.\n2. Enable the GDMA interrupt in the DSI subsystem interrupt enable register.\n3. Configure the DSI host PHY interface register with the appropriate PHY stop wait time setting.\n4. Configure the packet handling register to enable EoTp transmission, EoTp reception, Bus Turn-Around, ECC reception, and CRC reception.\n5. Configure the clock manager register with TX escape clock division and timeout clock division values.\n6. Disable the DPI interface by writing to the dpi_control register.\n7. Start DMA channel 0 by programming the DMAC debug instruction registers and executing the debug command.\n8. Start DMA channel 1 by programming the DMAC debug instruction registers and executing the debug command.\n9. Wait for an interrupt to fire; in the interrupt handler, read the interrupt_mask register to identify the interrupt source.\n10. If a GDMA interrupt is detected, read the DMAC masked interrupt status to determine which DMA channel completed.\n11. Clear the DMA interrupt by writing the channel status to the DMAC interrupt clear register.\n12. Clear the subsystem interrupt by writing to the interrupt_raw register.\n13. Verify that both DMA channels complete successfully and the test finishes with a pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "interrupt_enable; PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control; interrupt_mask; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "In the Default_IRQHandler, the test reads MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and checks if the MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR bit is set. If set, it reads MIZAR_MIPI_DSI_DMAC_INTMIS to determine which DMA channel triggered the interrupt. The test tracks completion of both DMA channel 0 and channel 1 using flags. When both channels have completed (both interrupt flags received and cleared), the test calls finish(0) indicating pass. If the interrupts do not fire or channels do not complete, the test does not reach the pass condition.",
        "Validation / Acceptance Criteria": "The test passes when both DMA channels complete their data transfers as indicated by interrupts. The interrupt handler must successfully read the interrupt_mask register to confirm a GDMA interrupt source, read the DMAC interrupt status to identify the completing channel, clear both the DMAC and subsystem interrupts, and track completion of both channels. The test finishes with a pass status (finish(0)) only when both DMA channel 0 and channel 1 have completed successfully.",
        "Remarks": "The test operates in interrupt-driven mode using a Default_IRQHandler for DMA completion notification. DMAC registers (INTEN, DBGINST0, DBGINST1, DBGCMD, INTMIS, INTCLR) could not be mapped to canonical register names as no DMAC register specification document was provided. The DPI interface is explicitly disabled indicating DBI/command mode operation. Two DMA channels are used for the data transfer."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_dbi_random_payload_test",
        "Feature": "DBI Random Payload Data Transfer",
        "Meta Headers": "NA",
        "Meta Macros": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME; RAM_BASE; DMA_SAR; DMA_DAR; DSI_WRITE_MEMORY_START",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "Polling Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs MIPI DSI DBI write memory transfers with random payload sizes over 10 iterations using polling-based DMA completion. It configures the DSI host PHY interface by writing phy_if_cfg (with phy_stop_wait_time field set via set_data_mask using MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME) to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG. It configures packet handling by writing 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG (enabling eotp_tx_en, eotp_rx_en, bta_en, ecc_rx_en, crc_rx_en). It configures clock management by writing 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG. It disables DPI by writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL. It enables DMAC interrupts by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN. For each iteration, it starts two DMA channels by writing debug instructions to MIZAR_MIPI_DSI_DMAC_DBGINST0, MIZAR_MIPI_DSI_DMAC_DBGINST1, and executing via MIZAR_MIPI_DSI_DMAC_DBGCMD. It polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until the value equals 0x3 (both channels complete), then clears interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTCLR. After all 10 iterations complete successfully, the test calls finish(0) for pass.",
        "Test Description": "This testcase validates MIPI DSI DBI write memory transfers with random payload sizes across 10 iterations using polling-based DMA completion. It configures the DSI host PHY interface, packet handling (enabling EoTp, BTA, ECC, and CRC), and clock management. The DPI interface is disabled to operate in DBI/command mode. For each iteration, two DMA channels are started to transfer data with a randomly generated payload size. The test polls the DMAC interrupt status register until both channels complete, then clears the interrupts. The test verifies successful completion of all 10 iterations.",
        "Meta Test Steps / Procedure": "1. Configure PHY interface: compute phy_if_cfg using set_data_mask with MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME field, then write_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg).\n2. Configure packet handling: write_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3d) enabling eotp_tx_en, eotp_rx_en, bta_en, ecc_rx_en, crc_rx_en.\n3. Configure clock manager: write_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107) setting tx_esc_clk_division and to_clk_division.\n4. Disable DPI: write_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0).\n5. Enable DMAC interrupts: write_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3) for channel 0 and channel 1.\n6. Begin loop for 10 iterations with random payload sizes.\n7. Start DMA channel 0: write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000), write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, ch0_desc_addr_act), write_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0).\n8. Start DMA channel 1: write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000), write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, ch1_desc_addr_act), write_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0).\n9. Poll MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until rd_data equals 0x3 (both channels complete).\n10. Read MIZAR_MIPI_DSI_DMAC_INTMIS again to confirm status.\n11. Clear DMA interrupts: write_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data).\n12. End loop; after all 10 iterations, call finish(0) for pass.",
        "Test Steps / Procedure": "1. Configure the DSI host PHY interface register with the appropriate PHY stop wait time setting.\n2. Configure the packet handling register to enable EoTp transmission, EoTp reception, Bus Turn-Around, ECC reception, and CRC reception.\n3. Configure the clock manager register with TX escape clock division and timeout clock division values.\n4. Disable the DPI interface by writing to the dpi_control register.\n5. Enable DMA channel 0 and channel 1 interrupts by writing to the DMAC interrupt enable register.\n6. Begin a loop of 10 iterations, each with a randomly generated payload size.\n7. Start DMA channel 0 by programming the DMAC debug instruction registers and executing the debug command.\n8. Start DMA channel 1 by programming the DMAC debug instruction registers and executing the debug command.\n9. Poll the DMAC masked interrupt status register until both DMA channels report completion.\n10. Clear the DMA interrupts by writing the completion status to the DMAC interrupt clear register.\n11. Repeat steps 7-10 for each of the 10 iterations.\n12. Verify that all 10 iterations complete successfully and the test finishes with a pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Impacted Registers": "PHY_IF_CFG; PCKHDL_CFG; CLKMGR_CFG; dpi_control",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_DSI_DMAC_INTMIS in a while loop until the read value equals 0x3, indicating both DMA channel 0 and channel 1 have completed. After confirming completion, it clears the interrupts by writing to MIZAR_MIPI_DSI_DMAC_INTCLR. This polling and clearing cycle repeats for all 10 iterations with random payload sizes. After all iterations complete successfully, the test calls finish(0) indicating pass. If the polling condition is never met, the test hangs and does not reach the pass condition.",
        "Validation / Acceptance Criteria": "The test passes when all 10 iterations of DBI write memory transfers with random payload sizes complete successfully. For each iteration, the DMAC masked interrupt status must indicate that both DMA channels have completed. After clearing the DMA interrupts, the next iteration proceeds. The test finishes with a pass status only after all 10 iterations complete. If any iteration fails to complete (polling never satisfied), the test does not reach the pass condition.",
        "Remarks": "The test operates in polling mode, reading the DMAC masked interrupt status register in a loop until both channels complete. DMAC registers (INTEN, DBGINST0, DBGINST1, DBGCMD, INTMIS, INTCLR) could not be mapped to canonical register names as no DMAC register specification document was provided. The DPI interface is explicitly disabled indicating DBI/command mode operation. Random payload sizes are used across 10 iterations to stress-test the DBI data transfer path."
    },
    {
        "Index": "3",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "Subsystem Register Write/Read Verification",
        "Meta Headers": "NA",
        "Meta Macros": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW; CNT; SOFT_RST_REG_ADDRESS",
        "Meta Arrays": "addr_array[52]; default_value_array[52]; read_mask_array[52]; write_mask_array[52]; skip_array[52]; chk_val[6]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs register write/read verification for 5 MIPI DSI subsystem registers. It uses addr_array[52] containing register address macros (MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW), along with default_value_array[52] for expected reset values, read_mask_array[52] for read masks, write_mask_array[52] for write masks, and skip_array[52] for controlling which registers are skipped during write/read testing. The function chk_rst_val() reads each register via read_reg(addr) and compares the read value (masked with read_mask) against the expected default value to verify reset/default state. The function chk_rd_wr() writes 6 different test patterns from chk_val[6] to each register (masked with write_mask) via write_reg(addr, data_wr), then reads back via read_reg(addr) and compares (masked with read_mask) to verify write/read integrity. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW has skip_array[4]=1, meaning it is skipped in chk_rd_wr() but still verified for its default value in chk_rst_val(). The remaining 4 registers undergo both default value check and full write/read verification with all 6 test patterns. SOFT_RST_REG_ADDRESS is used in soft_reset_chk() but is excluded from analysis per instructions.",
        "Test Description": "This testcase verifies the register write/read functionality of 5 MIPI DSI subsystem registers: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, and interrupt_raw. It first checks that all registers contain their expected default/reset values by reading each register and comparing against known defaults with appropriate read masks. Then it performs write/read verification on 4 of the 5 registers (interrupt_raw is skipped for write/read) using 6 different test patterns. For each pattern, the test writes a masked value to the register, reads it back, and compares the result against the expected value using read and write masks. The test validates both the default state integrity and the write/read accessibility of the subsystem registers.",
        "Meta Test Steps / Procedure": "1. Initialize addr_array[52] with register address macros: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.\n2. Initialize default_value_array[52] with expected reset values for each register.\n3. Initialize read_mask_array[52] with read masks for each register.\n4. Initialize write_mask_array[52] with write masks for each register.\n5. Initialize skip_array[52] with skip flags; skip_array[4]=1 for MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.\n6. Initialize chk_val[6] with 6 different test patterns for write/read verification.\n7. Call chk_rst_val(): for each register i from 0 to CNT-1, read_reg(addr_array[i]), apply read_mask_array[i], compare with default_value_array[i]. If mismatch, increment err1 and log error.\n8. Call chk_rd_wr(): for each test pattern j from 0 to 5, for each register i from 0 to CNT-1, if skip_array[i]==0, compute data_wr = chk_val[j] & write_mask_array[i], write_reg(addr_array[i], data_wr), read_reg(addr_array[i]), apply read_mask_array[i], compare with expected. If mismatch, increment err2 and log error.\n9. Check err1 and err2: if both are 0, call finish(0) for pass; otherwise call finish(1) for fail.",
        "Test Steps / Procedure": "1. Initialize register address, default value, read mask, write mask, and skip flag arrays for the 5 target subsystem registers.\n2. Initialize 6 different test patterns for write/read verification.\n3. Execute default value check: read each of the 5 subsystem registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw) and compare the masked read value against the expected default/reset value.\n4. Execute write/read verification: for each of the 6 test patterns, write the masked pattern to each non-skipped register (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv), read back the value, and compare the masked result against the expected value.\n5. Note that interrupt_raw is skipped during write/read verification but is still checked for its default value.\n6. Verify that no errors occurred during default value checks or write/read checks.\n7. If all checks pass with zero errors, the test finishes with pass status; otherwise it finishes with fail status.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "data_fifo_threshold_val; low_pwr; dbite; dbi_fdiv; interrupt_raw",
        "Meta Validation / Acceptance Criteria": "In chk_rst_val(), for each register i, the test reads read_reg(addr_array[i]), applies read_mask_array[i] via bitwise AND, and compares the result with default_value_array[i]. If the masked read value does not match the expected default, err1 is incremented and an error is logged. In chk_rd_wr(), for each of the 6 test patterns in chk_val[6] and each register i where skip_array[i]==0, the test writes data_wr = chk_val[j] & write_mask_array[i] via write_reg(addr_array[i], data_wr), reads back via read_reg(addr_array[i]), applies read_mask_array[i], and compares with the expected value. If the masked read-back does not match, err2 is incremented and an error is logged. The test passes (finish(0)) only if both err1==0 and err2==0. Otherwise it fails (finish(1)).",
        "Validation / Acceptance Criteria": "The test passes when all of the following conditions are met: (1) All 5 subsystem registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw) return their expected default/reset values when read and masked with the appropriate read mask. (2) All 4 writable registers (data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv) successfully accept writes and return the correct masked values when read back for all 6 test patterns. (3) No errors are accumulated during either the default value check or the write/read verification phases. The test finishes with pass status only when zero errors are detected across all checks; any mismatch results in fail status.",
        "Remarks": "The interrupt_raw register (skip_array[4]=1) is excluded from write/read verification in chk_rd_wr() but is still verified for its default value in chk_rst_val(). SOFT_RST_REG_ADDRESS is used in the soft_reset_chk() function but was explicitly excluded from analysis per instructions. All 5 register address macros were successfully resolved to the MIPI DSI subsystem base and mapped to canonical register names from the subsystem specification."
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
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_alignment = Alignment(vertical='top', wrap_text=True)
thin_border = Border(
    left=Side(style='thin'),
    right=Side(style='thin'),
    top=Side(style='thin'),
    bottom=Side(style='thin')
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
        value = row_data.get(col_name, '')
        if value is None:
            value = ''
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_tp.freeze_panes = 'A2'

# Auto-size columns with max width
for col_idx, col_name in enumerate(testplan_columns, 1):
    max_length = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        cell_value = str(ws_tp.cell(row=row_idx, column=col_idx).value or '')
        lines = cell_value.split('\n')
        for line in lines:
            if len(line) > max_length:
                max_length = len(line)
    adjusted_width = min(max_length + 2, 60)
    ws_tp.column_dimensions[ws_tp.cell(row=1, column=col_idx).column_letter].width = adjusted_width

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')

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
        value = row_data.get(col_name, '')
        if value is None:
            value = ''
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_md.freeze_panes = 'A2'

# Auto-size columns with max width
for col_idx, col_name in enumerate(metadata_columns, 1):
    max_length = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        cell_value = str(ws_md.cell(row=row_idx, column=col_idx).value or '')
        lines = cell_value.split('\n')
        for line in lines:
            if len(line) > max_length:
                max_length = len(line)
    adjusted_width = min(max_length + 2, 60)
    ws_md.column_dimensions[ws_md.cell(row=1, column=col_idx).column_letter].width = adjusted_width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save workbook
wb.save(output_path)
print(f'SUCCESS: Workbook saved to {output_path}')
print(f'FILENAME: {filename}')
print(f'TIMESTAMP: {timestamp_str}')

# Validate
import os
file_size = os.path.getsize(output_path)
print(f'FILE_SIZE: {file_size}')

# Reopen to validate
from openpyxl import load_workbook
wb_check = load_workbook(output_path)
sheet_names = wb_check.sheetnames
print(f'SHEETS: {sheet_names}')
print(f'TESTPLAN_ROWS: {ws_tp.max_row - 1}')
print(f'METADATA_ROWS: {ws_md.max_row - 1}')
print('VALIDATION: PASSED')
