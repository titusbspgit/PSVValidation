#!/usr/bin/env python3
import json, os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# IST timestamp
ist = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(ist)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp}.xlsx'
output_dir = 'Test_Output/MIPI/TestPlan'
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# Input data
json_data = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_dphy_lanes_test",
    "Feature": "DPHY Lane Configuration",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates MIPI CSI2 DPHY lane configuration. It writes to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel, writes to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data, writes to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the number of active DPHY lanes, and polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait for the PHY stop state condition. It writes to a direct hex address 0xa0243ffc and reads from 0xE6001000. It also enables CSI2 host interrupts by reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN and writing to all interrupt mask registers: MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, and MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED.",
    "Test Description": "This test validates MIPI CSI2 DPHY lane configuration and interrupt setup. It configures the virtual channel and control data in the CSI2 register block, sets the number of active DPHY lanes via the N_LANES register, and polls the PHY_STOPSTATE register to confirm the PHY has entered the stop state. It also reads the INT_ST_MAIN register and enables all CSI2 host interrupt masks including PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt masks. Additionally, it performs a write and a read to external hardware addresses outside the CSI2 host register space.",
    "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel. 2. Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data parameters. 3. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a while loop to wait for the PHY stop state condition. 4. Write to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the number of active DPHY lanes. 5. Write to 0xa0243ffc (direct hex address). 6. Read from 0xE6001000 (direct hex address). 7. Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check main interrupt status. 8. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to enable PHY fatal interrupt mask. 9. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to enable packet fatal interrupt mask. 10. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to enable PHY interrupt mask. 11. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to enable line interrupt mask. 12. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to enable boundary frame fatal interrupt mask. 13. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to enable sequence frame fatal interrupt mask. 14. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to enable CRC frame fatal interrupt mask. 15. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to enable payload CRC fatal interrupt mask. 16. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to enable data ID interrupt mask. 17. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to enable ECC corrected interrupt mask.",
    "Test Steps / Procedure": "1. Configure the virtual channel by writing to the virtual_channel register. 2. Set control data parameters by writing to the control_data register. 3. Poll the PHY_STOPSTATE register until the PHY enters the stop state condition. 4. Configure the number of active DPHY lanes by writing to the N_LANES register. 5. Write to an external hardware register outside the CSI2 host register space. 6. Read from an external hardware register outside the CSI2 host register space. 7. Read the INT_ST_MAIN register to check the main interrupt status. 8. Enable all CSI2 host interrupt masks by writing to INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, and INT_MSK_ECC_CORRECTED registers. 9. Verify the test completes successfully with all lane configuration and interrupt setup operations completed without errors.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The PHY_STOPSTATE register is polled in a while loop until the expected stop state condition is met, confirming that the DPHY lanes have entered stop state after lane configuration. The INT_ST_MAIN register is read to verify the main interrupt status. All interrupt mask registers are written to enable the corresponding interrupt sources. The test passes if the PHY stop state polling completes successfully and all register writes complete without error.",
    "Validation / Acceptance Criteria": "The PHY_STOPSTATE register must indicate that the DPHY lanes have entered the stop state after lane configuration via the N_LANES register. The INT_ST_MAIN register read must return a valid interrupt status. All interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED) must be successfully written to enable the corresponding interrupts. The test passes if all configuration and polling operations complete without errors.",
    "Remarks": "The testcase polls PHY_STOPSTATE in a loop which may have a timeout dependency. Two direct hex addresses are accessed that could not be mapped to known CSI2 registers in the specification documents. The actual testcase source file could not be accessed for detailed source-level analysis; the output is based on upstream agent register-access extraction data."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Feature": "Register Block Read/Write Verification",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates the read and write accessibility of all MIPI CSI2 Register Block (RB) registers. It performs read_modify_write operations on the following register address macros: MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL (offset 0x0), MIZAR_MIPI_CSI2_RB_REG_LINE_INFO (offset 0x4), MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL (offset 0x8), MIZAR_MIPI_CSI2_RB_REG_LANE_CLK (offset 0xC), MIZAR_MIPI_CSI2_RB_REG_MEM (offset 0x10), MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW (offset 0x14), MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK (offset 0x18), MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE (offset 0x1C), MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA (offset 0x20), and MIZAR_MIPI_CSI2_RB_REG_FLUSH (offset 0x24). All registers reside in the RB register block at base 0xE6A00000+0x4000. The test verifies that each register can be written to and read back correctly, and also checks reset default values and performs soft reset verification.",
    "Test Description": "This test validates the read and write accessibility of all MIPI CSI2 Register Block registers. It performs write-then-read-back verification on each register in the RB block, including virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, and flush. The test checks that each register can be written with a known value and the same value can be read back correctly. It also verifies reset default values for all registers and performs soft reset verification to ensure registers return to their default state after reset.",
    "Meta Test Steps / Procedure": "1. The chk_rst_val() function iterates over addr_array[] containing all 10 RB register address macros (MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL through MIZAR_MIPI_CSI2_RB_REG_FLUSH) and calls read_reg(addr) on each to read and verify the reset default value against the expected default value from default_val_array[]. 2. The chk_rd_wr() function iterates over addr_array[] and for each register: writes a test value using write_reg(addr, data_wr) where data_wr is computed using the write mask, then reads back using read_reg(addr) and compares the read value (masked with read mask) against the expected written value. 3. The soft_reset_chk() function triggers a soft reset via SOFT_RST_REG_ADDRESS (excluded per instructions), then re-reads all registers to verify they have returned to their default values. 4. Error counters err1 and err2 track mismatches during reset value check and read/write verification respectively.",
    "Test Steps / Procedure": "1. Read all RB registers to verify their reset default values match expected defaults. 2. For each RB register (virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush), write a known test value using the register's write mask. 3. Read back each register after writing and compare the read value (using the read mask) against the expected written value. 4. Trigger a soft reset of the register block. 5. After soft reset, read all RB registers again to verify they have returned to their default reset values. 6. Verify that no mismatches occurred during reset value verification, write-read-back verification, or post-reset verification.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_LINE_INFO; MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL; MIZAR_MIPI_CSI2_RB_REG_LANE_CLK; MIZAR_MIPI_CSI2_RB_REG_MEM; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_RB_REG_FLUSH",
    "Impacted Registers": "virtual_channel; line_info; fifo_threshold_val; lane_clk; mem; interrupt_raw; interrupt_mask; interrupt_enable; control_data; flush",
    "Meta Validation / Acceptance Criteria": "For each register in addr_array[], the reset default value read via read_reg() must match the corresponding entry in default_val_array[]. During write-read-back verification, the value read back from each register (masked with read_mask_array[]) must match the written value (masked with write_mask_array[]). After soft reset, all registers must return to their default values. Error counters err1 (reset value mismatches) and err2 (read/write mismatches) must both be zero for the test to pass.",
    "Validation / Acceptance Criteria": "Each RB register must return its expected reset default value when read after power-on. After writing a known test value to each register, the read-back value must match the written value when applying the appropriate read and write masks. After a soft reset, all registers must return to their default reset values. The test passes only if no mismatches are detected during reset value verification, write-read-back verification, and post-reset verification.",
    "Remarks": "All 10 RB registers are accessed via an array-driven loop pattern using address, default value, read mask, and write mask arrays. The SOFT_RST_REG_ADDRESS macro is excluded per instructions. The actual testcase source files in the repository folder did not match the MIPI CSI2 RB register test content; the output is based on upstream agent register-access extraction data and the earlier pipeline analysis of the source code."
  },
  {
    "Index": "3",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_test_pattern_generator",
    "Feature": "Test Pattern Generation",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates the MIPI CSI2 internal test pattern generator functionality. It configures the pattern generator by writing vertical resolution to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, horizontal resolution to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, pattern configuration to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, and enables the pattern generator via MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE. It configures the virtual channel by writing to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL and sets control data via MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA. It polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait for the PHY stop state condition. DMA channel 0 read and write addresses are configured by writing to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, and MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION. It reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN for main interrupt status and enables all CSI2 host interrupt masks. The pattern generator is then disabled by writing 0 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE after the test completes.",
    "Test Description": "This test validates the MIPI CSI2 internal test pattern generator. It configures the pattern generator with vertical resolution, horizontal resolution, and pattern configuration parameters, then enables it via the PPI_PG_ENABLE register. The virtual channel and control data are configured in the RB register block. The PHY_STOPSTATE register is polled to confirm the PHY has entered the stop state. DMA channel 0 read and write address registers are configured for data and instruction paths via dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, and dma_m0_addr_aw_ch0_Instruction. The INT_ST_MAIN register is read for interrupt status, and all CSI2 host interrupt masks are enabled. After the test pattern transfer completes, the pattern generator is disabled by clearing the PPI_PG_ENABLE register.",
    "Meta Test Steps / Procedure": "1. Write vertical resolution value to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES. 2. Write horizontal resolution value to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES. 3. Write pattern configuration to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG. 4. Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to enable the test pattern generator. 5. Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL. 6. Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA. 7. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE. 8-11. Write DMA channel 0 address registers. 12. Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN. 13-22. Write all interrupt mask registers. 23. Write 0 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to disable.",
    "Test Steps / Procedure": "1. Configure the test pattern generator vertical resolution by writing to the PPI_PG_PATTERN_VRES register. 2. Configure horizontal resolution via PPI_PG_PATTERN_HRES. 3. Set pattern type via PPI_PG_CONFIG. 4. Enable pattern generator via PPI_PG_ENABLE. 5. Configure virtual_channel. 6. Set control_data. 7. Poll PHY_STOPSTATE. 8-9. Configure DMA channel 0 read address registers. 10-11. Configure DMA channel 0 write address registers. 12. Read INT_ST_MAIN. 13. Enable all CSI2 host interrupt masks. 14. Disable pattern generator. 15. Verify test completes without errors.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The PHY_STOPSTATE register is polled until stop state is met. The PPI_PG_ENABLE register is written to enable and later disable the pattern generator. The INT_ST_MAIN register is read. All interrupt mask registers are written. DMA channel 0 address registers are configured. The test passes if all operations complete without error.",
    "Validation / Acceptance Criteria": "The PHY_STOPSTATE register must indicate stop state. The test pattern generator must be successfully enabled and disabled via PPI_PG_ENABLE. DMA channel 0 registers must be configured. INT_ST_MAIN must return valid status. All interrupt mask registers must be written. The test passes if all operations complete without errors.",
    "Remarks": "The testcase polls PHY_STOPSTATE in a loop which may have a timeout dependency. The pattern generator is explicitly disabled at the end of the test by writing 0 to PPI_PG_ENABLE. DMA channel 0 is configured for both read (AR) and write (AW) address paths with separate data and instruction registers. The actual testcase source files in the repository folder did not match the MIPI CSI2 test pattern generator content; the output is based on upstream agent register-access extraction data."
  },
  {
    "Index": "4",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_dphy_idi_test",
    "Feature": "DPHY IDI Interface",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates the MIPI CSI2 DPHY IDI (Image Data Interface) functionality. It polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait for the PHY stop state condition. It writes to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data parameters, writes to MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK to configure null/blank packet insertion, and writes to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel. It writes to a direct hex address 0xa0243ffc and reads from 0xE6001000. It also enables CSI2 host interrupts by reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN and writing to all interrupt mask registers.",
    "Test Description": "This test validates the MIPI CSI2 DPHY IDI (Image Data Interface) functionality. It polls the PHY_STOPSTATE register to confirm the PHY has entered the stop state. It configures the control_data register for data control parameters, the null_blank register for null/blank packet insertion, and the virtual_channel register for virtual channel selection. It reads the INT_ST_MAIN register for main interrupt status and enables all CSI2 host interrupt masks including PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt masks. Additionally, it performs a write and a read to external hardware addresses outside the CSI2 host register space.",
    "Meta Test Steps / Procedure": "1. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE. 2. Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA (value 1). 3. Write to MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK (value 1). 4. Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL. 5. Write to 0xa0243ffc. 6. Read from 0xE6001000. 7. Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN. 8-17. Write to all interrupt mask registers.",
    "Test Steps / Procedure": "1. Poll the PHY_STOPSTATE register until the PHY enters the stop state condition. 2. Configure the control_data register. 3. Configure the null_blank register. 4. Configure the virtual_channel register. 5. Write to an external hardware register. 6. Read from an external hardware register. 7. Read the INT_ST_MAIN register. 8. Enable all CSI2 host interrupt masks. 9. Verify the test completes successfully.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PHY_STOPSTATE; control_data; null_blank; virtual_channel; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The PHY_STOPSTATE register is polled until stop state is met. MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA is written with value 1. MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK is written with value 1. MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL is written. INT_ST_MAIN is read. All interrupt mask registers are written. The test passes if all operations complete without error.",
    "Validation / Acceptance Criteria": "The PHY_STOPSTATE register must indicate stop state. The control_data register must be written. The null_blank register must be written. The virtual_channel register must be configured. INT_ST_MAIN must return valid status. All interrupt mask registers must be written. The test passes if all operations complete without errors.",
    "Remarks": "The testcase polls PHY_STOPSTATE in a loop which may have a timeout dependency. The null_blank register configuration distinguishes this test from other MIPI CSI2 tests by enabling null/blank packet insertion for IDI data path validation. Two direct hex addresses are accessed that could not be mapped to known CSI2 registers in the specification documents. The actual testcase source files in the repository folder did not match the MIPI CSI2 DPHY IDI test content; the output is based on upstream agent register-access extraction data."
  }
]

# TestPlan sheet columns
tp_cols = ['Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
           'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
           'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
           'Code Generation']

# MetaData sheet columns
md_cols = ['Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
           'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
           'Meta Headers', 'Meta Macros', 'Meta Arrays']

# Create workbook
wb = Workbook()
ws_tp = wb.active
ws_tp.title = 'TestPlan'
ws_md = wb.create_sheet('MetaData')

# Formatting
header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')

def write_sheet(ws, columns, data):
    # Write headers
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align
    # Write data rows
    for row_idx, row_data in enumerate(data, 2):
        for col_idx, col_name in enumerate(columns, 1):
            val = row_data.get(col_name, '')
            cell = ws.cell(row=row_idx, column=col_idx, value=val)
            cell.alignment = wrap_align
    # Freeze first row
    ws.freeze_panes = 'A2'
    # Auto-size columns
    for col_idx, col_name in enumerate(columns, 1):
        max_len = len(str(col_name))
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    max_len = max(max_len, min(len(str(cell.value)), 80))
        adjusted_width = min(max_len + 4, 60)
        ws.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

write_sheet(ws_tp, tp_cols, json_data)
write_sheet(ws_md, md_cols, json_data)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save workbook
wb.save(output_path)
print(f'Workbook saved: {output_path}')
print(f'Filename: {filename}')

# Validate
wb2 = load_workbook(output_path)
assert 'TestPlan' in wb2.sheetnames, 'TestPlan sheet missing'
assert 'MetaData' in wb2.sheetnames, 'MetaData sheet missing'
assert wb2['MetaData'].sheet_state == 'veryHidden', 'MetaData not veryHidden'
tp_rows = wb2['TestPlan'].max_row - 1
md_rows = wb2['MetaData'].max_row - 1
assert tp_rows == 4, f'Expected 4 TestPlan rows, got {tp_rows}'
assert md_rows == 4, f'Expected 4 MetaData rows, got {md_rows}'
file_size = os.path.getsize(output_path)
assert file_size > 0, 'File size is 0'
print(f'Validation PASSED: {tp_rows} TestPlan rows, {md_rows} MetaData rows, size={file_size} bytes')
