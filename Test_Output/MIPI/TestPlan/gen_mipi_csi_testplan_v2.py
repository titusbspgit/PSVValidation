#!/usr/bin/env python3
"""MIPI_CSI TestPlan XLSX Generator - Agent 7 Direct Excel Generation"""
import json, os, sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl'])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

json_data = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_dphy_lanes_test",
    "Feature": "DPHY Lane Configuration and Interrupt Setup",
    "Meta Headers": "NA",
    "Meta Macros": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED; MIPI_CSI2_DMA_INTEN_OFFSET; MIPI_CSI2_DMA_INTMIS_OFFSET; MIPI_CSI2_DMA_INTCLR_OFFSET",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates MIPI CSI2 DPHY lane configuration and interrupt handling. In the test_case() function, it first writes to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel, then writes to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data. It polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a while-loop to wait for the PHY data lanes to reach stop state. It writes to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the number of active DPHY lanes. A write to hardcoded address 0xa0243ffc is performed for auxiliary configuration. DMA-related registers are accessed via base+offset expressions (gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET for interrupt enable, gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET for masked interrupt status read, gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET for interrupt clear, and gdma_reg_base+0x28 for DMA status). A read from hardcoded address 0xE6001000 is performed. The csi2_enable_interrupt() function reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check main interrupt status, then writes to all CSI2 host interrupt mask registers to enable all interrupt sources.",
    "Test Description": "This testcase validates MIPI CSI2 DPHY lane configuration and comprehensive interrupt setup. It configures the virtual channel and control data registers in the CSI2 register block, then polls the PHY_STOPSTATE register to confirm that the DPHY data lanes have entered stop state. The number of active DPHY lanes is configured via the N_LANES register. DMA interrupt enable, status, and clear operations are performed. The test enables all CSI2 host interrupt masks by writing to the interrupt mask registers for PHY fatal, packet fatal, PHY errors, line errors, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt categories. The main interrupt status register is read to verify the interrupt state.",
    "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL (base 0xE6A04000, offset 0x0) to configure virtual channel. 2. Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA (base 0xE6A04000, offset 0x20) to set control data parameters. 3. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE (base 0xE6A05000, offset 0x4C) in a while-loop waiting for PHY stop state indication on data lanes. 4. Write to MIZAR_MIPI_CSI2_HOST_N_LANES (base 0xE6A05000, offset 0x4) to configure the number of active DPHY lanes. 5. Write to hardcoded address 0xa0243ffc for auxiliary configuration. 6. Write to gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET to enable DMA interrupts. 7. Read gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET to check DMA masked interrupt status. 8. Write to gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET to clear DMA interrupts. 9. Read gdma_reg_base+0x28 for DMA status. 10. Read hardcoded address 0xE6001000. 11. Call csi2_enable_interrupt(): Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN (offset 0xC) to check main interrupt status. 12-21. Write to all 10 interrupt mask registers.",
    "Test Steps / Procedure": "1. Configure the virtual channel register in the CSI2 register block to set the desired virtual channel. 2. Configure the control data register in the CSI2 register block with the required control parameters. 3. Poll the PHY_STOPSTATE register until the DPHY data lanes indicate they have entered stop state. 4. Configure the N_LANES register to set the number of active DPHY lanes for the test. 5. Perform an auxiliary configuration write to an external peripheral register. 6. Enable DMA interrupts by writing to the DMA interrupt enable register. 7. Read the DMA masked interrupt status register to verify DMA interrupt state. 8. Clear any pending DMA interrupts by writing to the DMA interrupt clear register. 9. Read the DMA status register to confirm DMA channel state. 10. Read an external system register for status verification. 11. Read the INT_ST_MAIN register to check the current main interrupt status of the CSI2 host. 12. Enable all CSI2 host interrupt masks by writing to the following interrupt mask registers: PHY fatal, packet fatal, PHY error, line error, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected. 13. Verify that all interrupt mask registers are configured correctly and the DPHY lane configuration is complete.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The test validates that: (1) MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL write completes successfully. (2) MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA write completes successfully. (3) Polling of MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE exits the while-loop when the PHY data lanes reach stop state. (4) MIZAR_MIPI_CSI2_HOST_N_LANES is written with the correct lane count value. (5) DMA interrupt enable, status read, and interrupt clear operations complete without errors. (6) MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN read returns the expected main interrupt status. (7) All 10 interrupt mask registers are written successfully to enable the corresponding interrupt sources.",
    "Validation / Acceptance Criteria": "1. The virtual_channel register is written successfully to configure the desired virtual channel. 2. The control_data register is written successfully with the required control parameters. 3. The PHY_STOPSTATE register polling completes, confirming that the DPHY data lanes have reached stop state. 4. The N_LANES register is configured with the correct number of active lanes. 5. DMA interrupt enable, status read, and clear operations complete without errors. 6. The INT_ST_MAIN register read returns the expected main interrupt status value. 7. All 10 CSI2 host interrupt mask registers are configured correctly to enable the corresponding interrupt sources. 8. The test completes without timeout during the PHY stop state polling.",
    "Remarks": "Two hardcoded register addresses could not be mapped to canonical register names from the specification documents. The PHY_STOPSTATE register is read-only and is polled in a while-loop, which may cause a timeout if the PHY does not reach stop state. DMA registers are accessed via base+offset expressions using a runtime-computed DMA register base address. The test exercises both the CSI2 register block (virtual_channel, control_data) and the CSI2 host block (N_LANES, PHY_STOPSTATE, interrupt mask and status registers)."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Feature": "Register Block Read/Write Verification",
    "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>",
    "Meta Macros": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_LINE_INFO; MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL; MIZAR_MIPI_CSI2_RB_REG_LANE_CLK; MIZAR_MIPI_CSI2_RB_REG_MEM; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_RB_REG_FLUSH",
    "Meta Arrays": "addr_array[]; default_val_array[]; read_mask_array[]; write_mask_array[]; skip_array[]",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase performs register reset-value verification and write/read-back validation for all 10 registers in the MIPI CSI2 RB REG block (base 0xE6A04000). The test defines parallel arrays for addresses, default values, read masks, write masks, and skip flags. The chk_rst_val() function reads each register and compares against expected defaults. The chk_rd_wr() function writes test patterns and verifies read-back for writable registers.",
    "Test Description": "This testcase validates the reset default values and write/read-back behavior of all registers in the MIPI CSI2 register block. It first reads each register and compares the masked value against the expected reset default to verify correct power-on state. Then, for writable registers, it writes a test pattern, reads back the value, and compares the masked result against the written data to confirm write/read-back integrity. Two registers (interrupt_raw and flush) are read-only and are skipped during the write/read-back phase. All 10 registers in the block are covered: virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, and flush.",
    "Meta Test Steps / Procedure": "1. Define addr_array with 10 register address macros. 2. Define default_val_array with expected reset values. 3. Define read_mask_array with valid read bit masks. 4. Define write_mask_array with writable bit masks. 5. Define skip_array with skip flags (indices 5 and 9 set to 1). 6. Call chk_rst_val(): For each register, read and compare against defaults. 7. Call chk_rd_wr(): For writable registers, write test pattern and verify read-back. 8. Check error counters for pass/fail.",
    "Test Steps / Procedure": "1. Initialize the test with arrays containing register addresses, expected reset default values, read masks, write masks, and skip flags for all 10 registers in the MIPI CSI2 register block. 2. Perform reset value verification: Read each register, apply the corresponding read mask, and compare against the expected reset default value. 3. Perform write/read-back verification: For each writable register, write a test pattern, read back the register value, apply the write mask, and compare against the expected written value. 4. Skip write/read-back testing for read-only registers (interrupt_raw and flush). 5. Evaluate the error counters from both verification phases to determine overall test pass or fail.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_LINE_INFO; MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL; MIZAR_MIPI_CSI2_RB_REG_LANE_CLK; MIZAR_MIPI_CSI2_RB_REG_MEM; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_RB_REG_FLUSH",
    "Impacted Registers": "virtual_channel; line_info; fifo_threshold_val; lane_clk; mem; interrupt_raw; interrupt_mask; interrupt_enable; control_data; flush",
    "Meta Validation / Acceptance Criteria": "In chk_rst_val(): For each register, read and mask with read_mask, compare against default_val. In chk_rd_wr(): For writable registers, write data_wr, read back, mask with write_mask, compare. Test passes if both err1 == 0 and err2 == 0. Registers at indices 5 and 9 are excluded from write/read-back.",
    "Validation / Acceptance Criteria": "1. All 10 registers must return their expected reset default values when read and masked. 2. For the 8 writable registers, the write/read-back value masked with the write mask must match the written test pattern. 3. The interrupt_raw and flush registers are read-only and must be correctly skipped. 4. The test passes only if zero errors are detected in both phases.",
    "Remarks": "The test uses parallel arrays to iterate over all 10 registers in the MIPI CSI2 RB REG block. Two registers (interrupt_raw at offset 0x14 and flush at offset 0x24) are marked as read-only via the skip_array. The soft_reset_chk() function referencing SOFT_RST_REG_ADDRESS is commented out and excluded per instructions."
  },
  {
    "Index": "3",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_test_pattern_generator",
    "Feature": "Test Pattern Generation and DMA Configuration",
    "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>",
    "Meta Macros": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED; MIPI_CSI2_DMA_INTMIS_OFFSET",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates the MIPI CSI2 internal test pattern generator functionality along with DMA channel configuration and comprehensive interrupt setup. The csi2_ctrlr_pg_enable() function configures the pattern generator. DMA channel 0 address registers are configured. CSI PHY clock gating is enabled. All 10 interrupt mask registers are written.",
    "Test Description": "This testcase validates the MIPI CSI2 internal test pattern generator along with DMA channel configuration and interrupt setup. It configures the pattern generator vertical and horizontal resolution via the PPI_PG_PATTERN_VRES and PPI_PG_PATTERN_HRES registers, sets the pattern configuration via PPI_PG_CONFIG, and enables the generator via PPI_PG_ENABLE. The virtual_channel and control_data registers are configured. The PHY_STOPSTATE register is polled. DMA channel 0 read and write address registers are configured. The enableclkgating_csiphy register is written. After test pattern transmission, the pattern generator is disabled. All CSI2 host interrupt mask registers are enabled.",
    "Meta Test Steps / Procedure": "1. Call csi2_ctrlr_pg_enable(): Write to PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, PPI_PG_ENABLE. 2. Write to virtual_channel and control_data. 3. Poll PHY_STOPSTATE. 4. Configure DMA channel 0 address registers. 5. Write to enableclkgating_csiphy. 6. Disable pattern generator. 7. Read DMA interrupt status. 8. Enable all interrupt masks.",
    "Test Steps / Procedure": "1. Configure the test pattern generator by setting the vertical resolution in PPI_PG_PATTERN_VRES and horizontal resolution in PPI_PG_PATTERN_HRES. 2. Set the pattern configuration in PPI_PG_CONFIG. 3. Enable the test pattern generator by writing to PPI_PG_ENABLE. 4. Configure virtual_channel with the desired virtual channel ID. 5. Write to control_data to set control parameters. 6. Poll PHY_STOPSTATE until DPHY data lanes enter stop state. 7. Configure DMA channel 0 read address registers. 8. Configure DMA channel 0 write address registers. 9. Enable CSI PHY clock gating. 10. Disable the test pattern generator. 11. Read DMA masked interrupt status. 12. Read INT_ST_MAIN. 13. Enable all CSI2 host interrupt masks. 14. Verify all configurations complete without errors.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; enableclkgating_csiphy; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The test validates that all PPI_PG registers are written correctly, DMA channel 0 address registers are configured, CSI PHY clock gating is enabled, pattern generator is toggled, and all 10 interrupt mask registers are written successfully.",
    "Validation / Acceptance Criteria": "1. PPI_PG_PATTERN_VRES written successfully. 2. PPI_PG_PATTERN_HRES written successfully. 3. PPI_PG_CONFIG written successfully. 4. PPI_PG_ENABLE toggled correctly. 5. virtual_channel configured. 6. control_data written. 7. PHY_STOPSTATE polling completes. 8. All four DMA channel 0 address registers configured. 9. enableclkgating_csiphy written. 10. DMA interrupt status read completes. 11. INT_ST_MAIN read returns expected status. 12. All 10 interrupt mask registers configured. 13. Test completes without timeout.",
    "Remarks": "The test pattern generator is enabled and then disabled within the same test flow. The PHY_STOPSTATE register is polled in a while-loop. DMA masked interrupt status is accessed via a runtime-computed base address. The test exercises three functional areas: test pattern generation, DMA channel configuration, and interrupt mask setup."
  },
  {
    "Index": "4",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_dphy_idi_test",
    "Feature": "DPHY IDI Data Transfer with Virtual Channel Iteration",
    "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>",
    "Meta Macros": "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED; MIPI_CSI2_DMA_INTEN_OFFSET; MIPI_CSI2_DMA_INTMIS_OFFSET; MIPI_CSI2_DMA_INTCLR_OFFSET; GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates MIPI CSI2 DPHY IDI data transfer with virtual channel iteration, null/blank packet configuration, DMA interrupt handling, and comprehensive CSI2 host interrupt setup. PHY stop state is polled. Control data and null/blank packet insertion are configured. Virtual channel is iterated. DMA interrupts are managed. All 10 interrupt mask registers are written.",
    "Test Description": "This testcase validates MIPI CSI2 DPHY IDI data transfer with virtual channel iteration and comprehensive interrupt configuration. It polls the PHY_STOPSTATE register to confirm that the DPHY data lanes have entered stop state. The control_data register is configured to enable data control, and the null_blank register is configured to enable null/blank packet insertion. The virtual_channel register is written with the virtual channel ID for each iteration. DMA interrupt enable, masked status read, and interrupt clear operations are performed. All CSI2 host interrupt masks are enabled.",
    "Meta Test Steps / Procedure": "1. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE. 2. Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA. 3. Write to MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK. 4. Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL. 5. Write to 0xa0243ffc. 6-8. DMA interrupt operations. 9-10. External reads. 11-21. Enable all interrupt masks.",
    "Test Steps / Procedure": "1. Poll the PHY_STOPSTATE register until DPHY data lanes enter stop state. 2. Configure control_data register. 3. Configure null_blank register. 4. Write virtual channel ID to virtual_channel register. 5. Write virtual channel ID to external peripheral register. 6. Enable DMA interrupts. 7. Read DMA masked interrupt status. 8. Clear DMA interrupts. 9. Read DMA status register. 10. Read external system register. 11. Read INT_ST_MAIN. 12. Enable all CSI2 host interrupt masks. 13. Verify all configurations complete without errors.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PHY_STOPSTATE; control_data; null_blank; virtual_channel; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The test validates that PHY stop state polling completes, control_data and null_blank are written correctly, virtual_channel is iterated, DMA interrupt operations complete, and all 10 interrupt mask registers are written successfully.",
    "Validation / Acceptance Criteria": "1. PHY_STOPSTATE polling completes. 2. control_data written successfully. 3. null_blank written successfully. 4. virtual_channel configured for each iteration. 5. External peripheral register write completes. 6. DMA interrupt operations complete. 7. External system register read completes. 8. INT_ST_MAIN read returns expected status. 9. All 10 interrupt mask registers configured. 10. Test completes without timeout.",
    "Remarks": "Two hardcoded register addresses could not be mapped. The PHY_STOPSTATE register is polled in a while-loop. DMA registers are accessed via base+offset expressions. The null_blank register at offset 0x38 is unique to this testcase. Two macros (GDMA_CSI2_DATA_DEST_ADDR2 and GDMA_CTRL_DATA_DEST_ADDR2) are defined but not used in register access calls."
  }
]

# TestPlan sheet columns
tp_cols = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# MetaData sheet columns
md_cols = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

def generate_xlsx():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    ts = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"MIPI_CSI_TestPlan_{ts}.xlsx"
    output_dir = os.environ.get("OUTPUT_DIR", ".")
    filepath = os.path.join(output_dir, filename)

    wb = Workbook()

    # --- TestPlan Sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_tp.append(tp_cols)

    for row in json_data:
        r = []
        for col in tp_cols:
            if col == "Code Generation":
                r.append("")
            else:
                r.append(row.get(col, ""))
        ws_tp.append(r)

    # --- MetaData Sheet ---
    ws_md = wb.create_sheet("MetaData")
    ws_md.append(md_cols)

    for row in json_data:
        r = []
        for col in md_cols:
            r.append(row.get(col, ""))
        ws_md.append(r)

    # --- Formatting ---
    header_font = Font(bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_align = Alignment(wrap_text=True, vertical="top")

    for ws in [ws_tp, ws_md]:
        # Format header row
        for cell in ws[1]:
            cell.font = header_font
            cell.fill = header_fill
            cell.alignment = wrap_align

        # Wrap text on all data cells
        for row in ws.iter_rows(min_row=2, max_row=ws.max_row, max_col=ws.max_column):
            for cell in row:
                cell.alignment = wrap_align

        # Auto-size columns
        for col_idx in range(1, ws.max_column + 1):
            max_len = 0
            col_letter = get_column_letter(col_idx)
            for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=col_idx, max_col=col_idx):
                for cell in row:
                    if cell.value:
                        lines = str(cell.value).split('\n')
                        for line in lines:
                            max_len = max(max_len, len(line))
            width = min(max_len + 4, 60)
            ws.column_dimensions[col_letter].width = max(width, 12)

        # Freeze first row
        ws.freeze_panes = "A2"

    # MetaData sheet: veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save
    os.makedirs(output_dir, exist_ok=True)
    wb.save(filepath)
    print(f"SAVED: {filepath}")

    # Validate
    assert os.path.exists(filepath), "File does not exist"
    assert os.path.getsize(filepath) > 0, "File is empty"
    wb2 = load_workbook(filepath)
    assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in wb2.sheetnames, "MetaData sheet missing"
    tp_rows = wb2["TestPlan"].max_row - 1
    md_rows = wb2["MetaData"].max_row - 1
    print(f"VALIDATION PASSED: TestPlan={tp_rows} rows, MetaData={md_rows} rows")
    print(f"FILENAME={filename}")
    return filename

if __name__ == "__main__":
    generate_xlsx()
