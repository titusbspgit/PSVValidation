#!/usr/bin/env python3
"""MIPI_CSI TestPlan XLSX Generator - Agent 7 Automated Output
Generates MIPI_CSI_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx with IST timestamp.
Run: python generate_mipi_csi_testplan_agent7.py
"""
import os, json
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    import subprocess, sys
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl'])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment

JSON_DATA = [
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
    "Meta Test Description": "This testcase validates the MIPI CSI2 DPHY lane configuration. It writes to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel, writes to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data, polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait for the PHY stop state condition, and writes to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the number of active DPHY lanes. It also enables all CSI2 host interrupts by writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, and MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED. It reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check the main interrupt status. Additionally, it writes to address 0xa0243ffc and reads from address 0xE6001000 for auxiliary operations.",
    "Test Description": "This test validates the MIPI CSI2 DPHY lane configuration. It configures the virtual channel via the virtual_channel register, sets control data via the control_data register, polls the PHY_STOPSTATE register to confirm the PHY has entered the stop state, and configures the number of active DPHY lanes via the N_LANES register. The test enables all CSI2 host interrupt masks including INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, and INT_MSK_ECC_CORRECTED. It reads the INT_ST_MAIN register to verify the main interrupt status. Additional auxiliary register accesses are performed for system-level configuration.",
    "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel setting. 2. Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set the control data configuration. 3. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a loop until the PHY stop state condition is met. 4. Write to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the number of active DPHY lanes. 5. Write to 0xa0243ffc for auxiliary system-level configuration. 6. Call csi2_enable_interrupt() to enable all CSI2 host interrupts: 6a. Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check the current main interrupt status. 6b. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to unmask PHY fatal interrupts. 6c. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to unmask packet fatal interrupts. 6d. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to unmask PHY interrupts. 6e. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to unmask line interrupts. 6f. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to unmask boundary frame fatal interrupts. 6g. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to unmask sequence frame fatal interrupts. 6h. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to unmask CRC frame fatal interrupts. 6i. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to unmask payload CRC fatal interrupts. 6j. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to unmask data ID interrupts. 6k. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to unmask ECC corrected interrupts. 7. Read from 0xE6001000 for auxiliary status check.",
    "Test Steps / Procedure": "1. Configure the virtual channel by writing to the virtual_channel register. 2. Set the control data configuration by writing to the control_data register. 3. Poll the PHY_STOPSTATE register until the PHY stop state condition is satisfied. 4. Configure the number of active DPHY lanes by writing to the N_LANES register. 5. Perform auxiliary system-level register write for configuration. 6. Enable all CSI2 host interrupts by performing the following: 6a. Read the INT_ST_MAIN register to check the current main interrupt status. 6b. Unmask PHY fatal interrupts via INT_MSK_PHY_FATAL. 6c. Unmask packet fatal interrupts via INT_MSK_PKT_FATAL. 6d. Unmask PHY interrupts via INT_MSK_PHY. 6e. Unmask line interrupts via INT_MSK_LINE. 6f. Unmask boundary frame fatal interrupts via INT_MSK_BNDRY_FRAME_FATAL. 6g. Unmask sequence frame fatal interrupts via INT_MSK_SEQ_FRAME_FATAL. 6h. Unmask CRC frame fatal interrupts via INT_MSK_CRC_FRAME_FATAL. 6i. Unmask payload CRC fatal interrupts via INT_MSK_PLD_CRC_FATAL. 6j. Unmask data ID interrupts via INT_MSK_DATA_ID. 6k. Unmask ECC corrected interrupts via INT_MSK_ECC_CORRECTED. 7. Read auxiliary status register to verify system-level state.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The testcase polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a while loop until the expected PHY stop state value is reached. The read of MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN validates the main interrupt status after interrupt mask configuration. Successful completion requires the PHY stop state polling condition to be met and all interrupt masks to be written without error.",
    "Validation / Acceptance Criteria": "The PHY_STOPSTATE register must reach the expected stop state value during polling. The INT_ST_MAIN register must reflect the correct main interrupt status after all interrupt masks are configured. All interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED) must accept the written values. The N_LANES register must be configured with the correct lane count. The test passes when all register operations complete successfully and the PHY stop state condition is satisfied.",
    "Remarks": "The actual MIPI CSI2 testcase source code was not found in the repository folder; the files present (program.c and main.c) contain unrelated PCIe and STM32 code. All register-related information is derived from upstream Agent 2, Agent 3, and Agent 4 outputs. Two register addresses (0xa0243ffc and 0xE6001000) could not be mapped to canonical register names. The testcase includes a DMA-related flow using offset-based register accesses (MIPI_CSI2_DMA_INTEN_OFFSET, MIPI_CSI2_DMA_INTMIS_OFFSET, MIPI_CSI2_DMA_INTCLR_OFFSET) that were excluded from Agent 2 extraction as they are base+offset expressions."
  },
  {
    "Index": "2",
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
    "Meta Test Description": "This testcase validates the MIPI CSI2 internal test pattern generator functionality. It configures the pattern generator by writing vertical resolution to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, horizontal resolution to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, pattern configuration to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, and enables the pattern generator via MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE. It configures the virtual channel via MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL and sets control data via MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA. It polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait for the PHY stop state condition. DMA address registers are configured by writing to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, and MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION. All CSI2 host interrupts are enabled by reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN and writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, and MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED. After pattern generation, MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE is disabled and DMA interrupt status is polled to confirm transfer completion.",
    "Test Description": "This test validates the MIPI CSI2 internal test pattern generator. It configures the pattern generator vertical resolution via PPI_PG_PATTERN_VRES, horizontal resolution via PPI_PG_PATTERN_HRES, and pattern configuration via PPI_PG_CONFIG, then enables the generator via PPI_PG_ENABLE. The virtual channel is configured via the virtual_channel register and control data is set via the control_data register. The test polls PHY_STOPSTATE to confirm the PHY has entered the stop state. DMA channel 0 read and write address registers are configured via dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, and dma_m0_addr_aw_ch0_Instruction. All CSI2 host interrupt masks are enabled including INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, and INT_MSK_ECC_CORRECTED. The INT_ST_MAIN register is read to verify the main interrupt status. After the pattern transfer, the pattern generator is disabled and DMA completion is verified.",
    "Meta Test Steps / Procedure": "1. Call csi2_ctrlr_pg_enable() to configure and enable the test pattern generator: 1a. Write vertical resolution value (0x10) to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES. 1b. Write horizontal resolution value (0x70140) to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES. 1c. Write pattern configuration value (0xe401) to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG. 1d. Write 1 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to enable the pattern generator. 2. Write virtual channel ID value to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel. 3. Write 0 to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data configuration. 4. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a while loop until the PHY stop state condition is met. 5. Write DMA read address for data channel (0x100) to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA. 6. Write DMA read address for instruction channel (0x0) to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION. 7. Write DMA write address for data channel (0x0) to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA. 8. Write DMA write address for instruction channel (0x0) to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION. 9. Write 0x1 to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 to trigger an additional RB_REG configuration. 10. Call csi2_enable_interrupt() to enable all CSI2 host interrupts: 10a. Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check the current main interrupt status. 10b. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to unmask PHY fatal interrupts. 10c. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to unmask packet fatal interrupts. 10d. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to unmask PHY interrupts. 10e. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to unmask line interrupts. 10f. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to unmask boundary frame fatal interrupts. 10g. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to unmask sequence frame fatal interrupts. 10h. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to unmask CRC frame fatal interrupts. 10i. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to unmask payload CRC fatal interrupts. 10j. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to unmask data ID interrupts. 10k. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to unmask ECC corrected interrupts. 11. Write 0 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to disable the pattern generator after transfer. 12. Poll DMA interrupt status register (gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) to wait for DMA transfer completion. 13. Verify DMA transfer completion status.",
    "Test Steps / Procedure": "1. Configure the test pattern generator by setting vertical resolution via PPI_PG_PATTERN_VRES, horizontal resolution via PPI_PG_PATTERN_HRES, and pattern configuration via PPI_PG_CONFIG. 2. Enable the test pattern generator by writing to PPI_PG_ENABLE. 3. Configure the virtual channel by writing to the virtual_channel register. 4. Set the control data configuration by writing to the control_data register. 5. Poll the PHY_STOPSTATE register until the PHY stop state condition is satisfied. 6. Configure DMA channel 0 read addresses by writing to dma_m0_addr_ar_ch0_data and dma_m0_addr_ar_ch0_Instruction. 7. Configure DMA channel 0 write addresses by writing to dma_m0_addr_aw_ch0_data and dma_m0_addr_aw_ch0_Instruction. 8. Trigger additional subsystem configuration for DMA enablement. 9. Enable all CSI2 host interrupts: 9a. Read INT_ST_MAIN to check the current main interrupt status. 9b. Unmask PHY fatal interrupts via INT_MSK_PHY_FATAL. 9c. Unmask packet fatal interrupts via INT_MSK_PKT_FATAL. 9d. Unmask PHY interrupts via INT_MSK_PHY. 9e. Unmask line interrupts via INT_MSK_LINE. 9f. Unmask boundary frame fatal interrupts via INT_MSK_BNDRY_FRAME_FATAL. 9g. Unmask sequence frame fatal interrupts via INT_MSK_SEQ_FRAME_FATAL. 9h. Unmask CRC frame fatal interrupts via INT_MSK_CRC_FRAME_FATAL. 9i. Unmask payload CRC fatal interrupts via INT_MSK_PLD_CRC_FATAL. 9j. Unmask data ID interrupts via INT_MSK_DATA_ID. 9k. Unmask ECC corrected interrupts via INT_MSK_ECC_CORRECTED. 10. Disable the test pattern generator by writing to PPI_PG_ENABLE. 11. Poll the DMA interrupt status to confirm DMA transfer completion. 12. Verify the test pattern data was transferred successfully.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The testcase polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a while loop until the expected PHY stop state value is reached. The read of MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN validates the main interrupt status after interrupt mask configuration. The DMA interrupt status register (gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled to confirm DMA transfer completion. MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE is written to 0 to disable the pattern generator after the transfer. Successful completion requires the PHY stop state polling condition to be met, all interrupt masks to be written without error, and the DMA transfer to complete as indicated by the DMA interrupt status.",
    "Validation / Acceptance Criteria": "The PHY_STOPSTATE register must reach the expected stop state value during polling. The INT_ST_MAIN register must reflect the correct main interrupt status after all interrupt masks are configured. All interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED) must accept the written values. The PPI_PG_ENABLE register must successfully enable and then disable the pattern generator. The DMA transfer must complete as indicated by the DMA interrupt status. The test pattern data generated by the PPI pattern generator must be transferred correctly through the DMA channel.",
    "Remarks": "The actual MIPI CSI2 test pattern generator source code was not found in the repository folder; the files present (program.c and Makefile) contain unrelated PCIe and timezone code. All register-related information is derived from upstream Agent 2, Agent 3, and Agent 4 outputs. The testcase includes DMA-related flow using offset-based register accesses (MIPI_CSI2_DMA_INTMIS_OFFSET) that were excluded from Agent 2 extraction as they are base+offset expressions. An additional base+offset write (MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4) was also excluded from Agent 2 extraction. All 22 Agent 4 register mappings were successfully matched to canonical register names."
  }
]

TP_COLS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

MD_COLS = [
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria", "Meta Headers",
    "Meta Macros", "Meta Arrays"
]

def create_workbook():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    ts = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"MIPI_CSI_TestPlan_{ts}.xlsx"
    
    wb = Workbook()
    
    # --- TestPlan sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_align = Alignment(wrap_text=True, vertical="top")
    
    for col_idx, col_name in enumerate(TP_COLS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align
    
    for row_idx, row_data in enumerate(JSON_DATA, 2):
        for col_idx, col_name in enumerate(TP_COLS, 1):
            val = row_data.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
            cell.alignment = wrap_align
    
    ws_tp.freeze_panes = "A2"
    
    # Auto-size columns
    for col_idx, col_name in enumerate(TP_COLS, 1):
        max_len = len(col_name)
        for row_idx in range(2, len(JSON_DATA) + 2):
            val = str(ws_tp.cell(row=row_idx, column=col_idx).value or "")
            max_len = max(max_len, min(len(val), 80))
        ws_tp.column_dimensions[ws_tp.cell(row=1, column=col_idx).column_letter].width = min(max_len + 2, 60)
    
    # --- MetaData sheet ---
    ws_md = wb.create_sheet("MetaData")
    
    for col_idx, col_name in enumerate(MD_COLS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align
    
    for row_idx, row_data in enumerate(JSON_DATA, 2):
        for col_idx, col_name in enumerate(MD_COLS, 1):
            val = row_data.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
            cell.alignment = wrap_align
    
    ws_md.freeze_panes = "A2"
    
    for col_idx, col_name in enumerate(MD_COLS, 1):
        max_len = len(col_name)
        for row_idx in range(2, len(JSON_DATA) + 2):
            val = str(ws_md.cell(row=row_idx, column=col_idx).value or "")
            max_len = max(max_len, min(len(val), 80))
        ws_md.column_dimensions[ws_md.cell(row=1, column=col_idx).column_letter].width = min(max_len + 2, 60)
    
    ws_md.sheet_state = "veryHidden"
    
    # Save
    script_dir = os.path.dirname(os.path.abspath(__file__))
    filepath = os.path.join(script_dir, filename)
    wb.save(filepath)
    
    # Validate
    assert os.path.exists(filepath), f"File not found: {filepath}"
    assert os.path.getsize(filepath) > 0, "File is empty"
    wb2 = load_workbook(filepath)
    assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in wb2.sheetnames, "MetaData sheet missing"
    wb2.close()
    
    print(f"SUCCESS: {filename}")
    print(f"Path: {filepath}")
    print(f"Size: {os.path.getsize(filepath)} bytes")
    print(f"Rows TestPlan: {len(JSON_DATA)}")
    print(f"Rows MetaData: {len(JSON_DATA)}")
    print(f"Validation: PASSED")
    return filename, filepath

if __name__ == "__main__":
    create_workbook()
