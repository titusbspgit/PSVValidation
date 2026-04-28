#!/usr/bin/env python3
import os, json, sys, io
from datetime import datetime, timezone, timedelta
from pathlib import Path
import subprocess

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print("[ERROR] openpyxl is required: ", e)
    sys.exit(1)

# ===== User/Workflow constants =====
IP_NAME = "I2C"
OUTPUT_DIR = Path("Test_Output/I2C/TestPlan")
BRANCH = os.getenv("GITHUB_REF_NAME", "main")

# JSON payload embedded (Stage-1 compliant)
JSON_DATA = r'''{
  "META_DATA": {
    "IP": "I2C",
    "repo_url": "https://github.com/titusbspgit/PSVValidation",
    "repo": "titusbspgit/PSVValidation",
    "branch": "main",
    "source_path_base": "TestRepo/i2c",
    "generation_timestamp_ist": "2026-04-28T00:00:00+05:30"
  },
  "test_cases": [
    {
      "Index": "1",
      "SS / Module": "I2C",
      "Feature": "AHB interface to access its register space",
      "Test Case Name": "i2c_reg_rd_wr_test",
      "Test Description": "Checks reset defaults and masked write/read behavior for the I2C/SMBus register map using multiple data patterns, then reports pass/fail based on mismatches observed.",
      "Speed": "NA",
      "Mode": "Polling",
      "Memory Start Offset": "NA",
      "Memory End Offset": "NA",
      "Remarks": "Registers marked non-readable or non-writable by masks are skipped; any addresses listed in the skip list are not accessed.",
      "Test Steps / Procedure": [
        "Start the test entry and run the default-value check for all configured registers.",
        "Iterate through the register list; for each entry marked readable, read the value and compare to the expected default for that register; count a failure if any mismatch is detected.",
        "For each of the predefined data patterns, iterate through the register list and write the pattern only to entries marked writable and not flagged to skip.",
        "After each pattern write pass, iterate through the register list again and for each readable and previously written entry, read back the value and compute the expected result from the written pattern combined with preserved default bits for non-writable fields; count a failure if any mismatch is detected.",
        "After completing all patterns, determine pass/fail based on whether any default or write/read mismatches were counted; finalize the test accordingly."
      ],
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": [
        "All readable registers return their documented default values during the reset-value check.",
        "For each data pattern and for each register with read/write access, the observed value equals the combination of written data in writable fields and preserved defaults in non-writable fields.",
        "The test passes only if no mismatches are counted across all checks; otherwise it fails."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "i2c_reg_rd_wr_test",
      "Hidden_Test_Description": "Performs chk_rst_val() over CNT=43 registers (addr_array[]) to compare read values against default_value_array[], skipping entries with read_mask_array[i]==0. Then performs chk_rd_wr() for 6 patterns (0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000): writes to each addr_array[i] if skip_array[i]==0 and write_mask_array[i]!=0, then reads back for entries with read_mask_array[i]!=0 and write_mask_array[i]!=0; expected value exp_val=((data_wr & read_mask & write_mask) | (~write_mask & read_mask & default_value)); increments wr_fail_cnt on mismatch. At end, finish(1) if def_fail_cnt>0 || wr_fail_cnt>0 else finish(0).",
      "Hidden_Remarks": "Addresses with read_mask_array[i]==0 are skipped as not readable; addresses with write_mask_array[i]==0 are skipped as not writable; addresses with skip_array[i]==1 are explicitly skipped in both write and readback phases.",
      "Hidden_Test_Steps_Procedure": [
        "Entry: test_case()",
        "Call chk_rst_val()",
        "Loop i=0..CNT-1 (CNT=43): addr=addr_array[i] where addr_array includes: MIZAR_I2C_DEV_CTRL, MIZAR_I2C_TSFR_CTRL, MIZAR_I2C_SLV_ADDR, MIZAR_I2C_TGT_SLV_ADDR, MIZAR_I2C_I2C_MSTR_CODE, MIZAR_I2C_I2C_BYTE_CNT, MIZAR_I2C_SF_HCNT, MIZAR_I2C_SF_LCNT, MIZAR_I2C_I2C_HS_HCNT, MIZAR_I2C_I2C_HS_LCNT, MIZAR_I2C_RIS, MIZAR_I2C_MASK_INTR, MIZAR_I2C_INTR_STS, MIZAR_I2C_INTR_CLR, MIZAR_I2C_TAS, MIZAR_I2C_TX_FIFO_THLD, MIZAR_I2C_RX_FIFO_THLD, MIZAR_I2C_DMA_CTRL, MIZAR_I2C_FF, MIZAR_I2C_TX_FIFO_LVL, MIZAR_I2C_RX_FIFO_LVL, MIZAR_I2C_I2C_MSTR_STS, MIZAR_I2C_I2C_FLTR_SEL, MIZAR_I2C_I2C_CURRENT_BYTECNT, MIZAR_I2C_I2C_SMB_SFTRST, MIZAR_I2C_SMB_HST_STS, MIZAR_I2C_SMB_HST_CTRL, MIZAR_I2C_SMB_HST_CMD, MIZAR_I2C_SMB_HST_DATA0, MIZAR_I2C_SMB_HST_DATA1, MIZAR_I2C_SMB_HST_BLOCK_DATA, MIZAR_I2C_SMB_PEC_DATA, MIZAR_I2C_SMB_SLAVE_WDATA, MIZAR_I2C_SMB_SLAVE_CMD, MIZAR_I2C_SMB_SLAVE_CTS, MIZAR_I2C_SMB_SLV, MIZAR_I2C_SMB_NOTIFY_ADDR, MIZAR_I2C_SMB_NOTIFY_LOW_BYTE, MIZAR_I2C_SMB_NOTIFY_HIGH_BYTE, MIZAR_I2C_SMB_DATA_HLDTIME, MIZAR_I2C_SMB_TIMEOUT_CNT, MIZAR_I2C_SMB_TMEXT_CNT, MIZAR_I2C_I2CSMB_DATA_SETUP.",
        "If read_mask_array[i]==0x00000000: continue (skip).",
        "READ data_rd = read_reg(addr).",
        "If data_rd == default_value_array[i]: OK else def_fail_cnt++ and log failure.",
        "Return from chk_rst_val().",
        "Call chk_rd_wr().",
        "Define chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}.",
        "For each pattern j=0..5: data_wr = chk_val[j].",
        "Write pass: Loop i=0..CNT-1: addr=addr_array[i]; if skip_array[i]==1 continue; if write_mask_array[i]==0x00000000 continue; else WRITE write_reg(addr,data_wr).",
        "Read/verify pass: Loop i=0..CNT-1: addr=addr_array[i]; if skip_array[i]==1 continue; if write_mask_array[i]==0x00000000 continue; if read_mask_array[i]==0x00000000 continue; else READ data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd==exp_val: OK else wr_fail_cnt++ and log failure.",
        "After all patterns complete: if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0)."
      ],
      "Hidden_Impacted_Registers": "MIZAR_I2C_DEV_CTRL, MIZAR_I2C_TSFR_CTRL, MIZAR_I2C_SLV_ADDR, MIZAR_I2C_TGT_SLV_ADDR, MIZAR_I2C_I2C_MSTR_CODE, MIZAR_I2C_I2C_BYTE_CNT, MIZAR_I2C_SF_HCNT, MIZAR_I2C_SF_LCNT, MIZAR_I2C_I2C_HS_HCNT, MIZAR_I2C_I2C_HS_LCNT, MIZAR_I2C_RIS, MIZAR_I2C_MASK_INTR, MIZAR_I2C_INTR_STS, MIZAR_I2C_INTR_CLR, MIZAR_I2C_TAS, MIZAR_I2C_TX_FIFO_THLD, MIZAR_I2C_RX_FIFO_THLD, MIZAR_I2C_DMA_CTRL, MIZAR_I2C_FF, MIZAR_I2C_TX_FIFO_LVL, MIZAR_I2C_RX_FIFO_LVL, MIZAR_I2C_I2C_MSTR_STS, MIZAR_I2C_I2C_FLTR_SEL, MIZAR_I2C_I2C_CURRENT_BYTECNT, MIZAR_I2C_I2C_SMB_SFTRST, MIZAR_I2C_SMB_HST_STS, MIZAR_I2C_SMB_HST_CTRL, MIZAR_I2C_SMB_HST_CMD, MIZAR_I2C_SMB_HST_DATA0, MIZAR_I2C_SMB_HST_DATA1, MIZAR_I2C_SMB_HST_BLOCK_DATA, MIZAR_I2C_SMB_PEC_DATA, MIZAR_I2C_SMB_SLAVE_WDATA, MIZAR_I2C_SMB_SLAVE_CMD, MIZAR_I2C_SMB_SLAVE_CTS, MIZAR_I2C_SMB_SLV, MIZAR_I2C_SMB_NOTIFY_ADDR, MIZAR_I2C_SMB_NOTIFY_LOW_BYTE, MIZAR_I2C_SMB_NOTIFY_HIGH_BYTE, MIZAR_I2C_SMB_DATA_HLDTIME, MIZAR_I2C_SMB_TIMEOUT_CNT, MIZAR_I2C_SMB_TMEXT_CNT, MIZAR_I2C_I2CSMB_DATA_SETUP",
      "Hidden_Validation_Acceptance_Criteria": "Default check: for all i with read_mask_array[i]!=0, read_reg(addr_array[i]) == default_value_array[i]. Write/Read check: for each pattern and for all i with write_mask_array[i]!=0 and read_mask_array[i]!=0 and skip_array[i]==0, read_reg(addr_array[i]) equals exp_val=((data_wr & read_mask & write_mask) | (~write_mask & read_mask & default_value)). Pass if def_fail_cnt==0 and wr_fail_cnt==0 leading to finish(0); otherwise finish(1)."
    },
    {
      "Index": "2",
      "SS / Module": "I2C",
      "Feature": "High-speed mode (3.4 Mb/s)",
      "Test Case Name": "test_i2c0_mst_i2c1_slv_dma_hs_md",
      "Test Description": "Configures a master–slave transfer from I2C0 to I2C1 using DMA in a high-speed setup, moves a small block of data through the host block data window, and validates data integrity and interrupt handling.",
      "Speed": "High-speed mode",
      "Mode": "DMA, Interrupt",
      "Memory Start Offset": "0xA0243E00",
      "Memory End Offset": "0xA0243F10",
      "Remarks": "Requires enabling SoC system access for the I2C interrupt path and enabling platform IRQs; uses a fixed transfer length and expects a single transfer-complete interrupt.",
      "Test Steps / Procedure": [
        "Initialize platform system access control to permit non-secure operation for the target interconnect interface.",
        "Enable the platform interrupt lines associated with the two I2C instances.",
        "Enable top-level interrupt sources for both I2C controllers.",
        "Flush controller FIFOs and clear any pending interrupt status at both ends.",
        "Program device control for both controllers; set the slave address and the master's target address; configure identical transfer byte counts.",
        "Program timing for the serial clock including standard/fast counters and high-speed counters; program the master code; unmask the transfer-complete interrupt at the master.",
        "Set transmit and receive FIFO thresholds on both controllers.",
        "Enable DMA on the master side and prepare the source SRAM buffer with test data.",
        "Configure the DMA transmit channel to move the prepared words from SRAM to the master's host block data register and enable the channel.",
        "Wait until DMA hardware reports completion.",
        "Initiate the I2C transfer at the master.",
        "Poll the master's current byte count until all bytes have been transmitted, inserting short waits between reads.",
        "Wait for the transfer-complete interrupt to be serviced.",
        "Enable DMA on the slave side, configure the receive channel to move words from the slave's host block data register to the destination SRAM region, and enable the channel.",
        "Wait until the receive DMA channel completes.",
        "Compare each word in the source and destination SRAM regions to confirm data integrity across the transfer.",
        "Finalize the test with a pass if no mismatches or interrupt handling errors are detected, otherwise report failure.",
        "Interrupt service: upon entry, recognize the transfer-complete source, clear the master's interrupt, clear platform IRQs, clear the raw system status, briefly wait, and verify both controller and system interrupt status are deasserted; record an error if any condition fails."
      ],
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": [
        "The transfer-complete interrupt is observed during the transaction and is acknowledged and cleared at both the controller and system levels.",
        "After completion, the contents of the destination SRAM exactly match the contents of the source SRAM for all words transferred.",
        "No error conditions are recorded during interrupt handling or data verification; the test passes only if the error counter remains zero."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_i2c0_mst_i2c1_slv_dma_hs_md",
      "Hidden_Test_Description": "I2C0 master to I2C1 slave DMA high-speed mode: preloads 5 words at SRAM_ADDR_1 (0xA0243F00), moves via DMA CH0 to MIZAR_I2C0_SMB_HST_BLOCK_DATA, starts I2C0 transfer, waits for int via Default_IRQHandler(), then enables I2C1 DMA CH1 to pull from MIZAR_I2C1_SMB_HST_BLOCK_DATA to SRAM_ADDR_2 (0xA0243E00); compares data and sets finish(test_err).",
      "Hidden_Remarks": "Requires configuring 0xA1700008..0xA1700054 for non-secure access; uses GIC IRQs 80 and 81; byte count set to 0x5; uses thresholds of 0x5; interrupt mask 0xFFFFFFEF at master; master code set via I2C_MSTR_CODE; uses wait_on(100) and wait_on(10) delays.",
      "Hidden_Test_Steps_Procedure": [
        "Entry: test_case()",
        "Set int_pend=1.",
        "WRITE 0xA1700008=0x1; 0xA170000C=0x1; 0xA1700014=0x1; 0xA1700018=0x1; 0xA170001C=0x1; 0xA1700020=0x1; 0xA1700024=0x1; 0xA1700028=0x1; 0xA170002C=0x1; 0xA1700030=0x1; 0xA1700034=0x1; 0xA1700038=0x1; 0xA170003C=0x1; 0xA1700044=0x1; 0xA1700048=0x1; 0xA1700050=0x1; 0xA1700054=0x1.",
        "GIC_EnableIRQ(80); GIC_EnableIRQ(81).",
        "WRITE MIZAR_LSS_SYSREG_INTR_EN0 = LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT | LSS_SYSREG_INTR_EN0_I2C1_INTERRUPT.",
        "WRITE MIZAR_I2C0_FF=0x3; WRITE MIZAR_I2C1_FF=0x3.",
        "WRITE MIZAR_I2C0_INTR_CLR=0xFFFFFFFF; WRITE MIZAR_I2C1_INTR_CLR=0xFFFFFFFF.",
        "WRITE MIZAR_I2C0_DEV_CTRL=0x38A; WRITE MIZAR_I2C1_DEV_CTRL=0x38A.",
        "WRITE MIZAR_I2C1_SLV_ADDR=0x37; WRITE MIZAR_I2C0_TGT_SLV_ADDR=0x6E.",
        "WRITE MIZAR_I2C0_I2C_BYTE_CNT=0x5; WRITE MIZAR_I2C1_I2C_BYTE_CNT=0x5.",
        "WRITE MIZAR_I2C0_SF_LCNT=0x2C; WRITE MIZAR_I2C0_SF_HCNT=0x18.",
        "WRITE MIZAR_I2C0_I2C_HS_LCNT=0xC; WRITE MIZAR_I2C0_I2C_HS_HCNT=0x8.",
        "WRITE MIZAR_I2C0_I2C_MSTR_CODE=0x2.",
        "WRITE MIZAR_I2C0_MASK_INTR=0xFFFFFFEF.",
        "WRITE MIZAR_I2C0_TX_FIFO_THLD=0x5; WRITE MIZAR_I2C1_TX_FIFO_THLD=0x5.",
        "WRITE MIZAR_I2C0_RX_FIFO_THLD=0x5; WRITE MIZAR_I2C1_RX_FIFO_THLD=0x5.",
        "WRITE MIZAR_I2C0_DMA_CTRL=0x2.",
        "Set src_addr=SRAM_ADDR_1(0xA0243F00); dest_addr=MIZAR_I2C0_SMB_HST_BLOCK_DATA.",
        "For i=0..4: WRITE (0xA0243F00 + i*4) = (i*5).",
        "DMA CH0 TX config: WRITE MIZAR_DMA_CH0_CTRL=0x8028028; WRITE MIZAR_DMA_CH0_SRC_ADDR=0xA0243F00; WRITE MIZAR_DMA_CH0_DEST_ADDR=MIZAR_I2C0_SMB_HST_BLOCK_DATA; WRITE MIZAR_DMA_CH0_SRC_XCNT=0x5; WRITE MIZAR_DMA_CH0_SRC_XMDFY=0x4; WRITE MIZAR_DMA_CH0_DEST_XMDFY=0x0; WRITE MIZAR_DMA_CH0_SRC_REQ=0x5; WRITE MIZAR_DMA_DMA_CH_EN=0x1.",
        "Call dma_disable(): Poll READ MIZAR_DMA_DMA_CH_EN until 0; wait_on(100) within loop.",
        "Start I2C transfer: WRITE MIZAR_I2C0_TSFR_CTRL=0x2.",
        "Poll READ MIZAR_I2C0_I2C_CURRENT_BYTECNT until ==0; in loop: printf and wait_on(100).",
        "Wait for interrupt: while(int_pend) { wait_on(10); }",
        "Slave side DMA: WRITE MIZAR_I2C1_DMA_CTRL=0x1; set src_addr=MIZAR_I2C1_SMB_HST_BLOCK_DATA; dest_addr=SRAM_ADDR_2(0xA0243E00).",
        "DMA CH1 RX config: WRITE MIZAR_DMA_CH1_CTRL=0x8024028; WRITE MIZAR_DMA_CH1_SRC_ADDR=MIZAR_I2C1_SMB_HST_BLOCK_DATA; WRITE MIZAR_DMA_CH1_DEST_ADDR=0xA0243E00; WRITE MIZAR_DMA_CH1_SRC_XCNT=0x5; WRITE MIZAR_DMA_CH1_SRC_XMDFY=0x0; WRITE MIZAR_DMA_CH1_DEST_XMDFY=0x4; WRITE MIZAR_DMA_CH1_SRC_REQ=0x6; WRITE MIZAR_DMA_DMA_CH_EN=0x2.",
        "Call dma_disable(): Poll READ MIZAR_DMA_DMA_CH_EN until 0; wait_on(100) within loop.",
        "For i=0..4: READ data_sent = READ(0xA0243F00 + i*4); READ data_rcvd = READ(0xA0243E00 + i*4); if equal: log success else test_err++ and log failure.",
        "wait_on(100); finish(test_err).",
        "ISR Default_IRQHandler(): set int_pend=0; READ int_status=READ MIZAR_I2C0_INTR_STS; if int_status==0x0010 then WRITE MIZAR_I2C0_INTR_CLR=0x00000010; GIC_ClearIRQ(80); GIC_ClearIRQ(81); WRITE MIZAR_LSS_SYSREG_RAW_STCR0 = LSS_SYSREG_RAW_STCR0_I2C0_INTERRUPT; wait_on(100); READ int_status=READ MIZAR_I2C0_INTR_STS; READ int_status_lss=READ MIZAR_LSS_SYSREG_RAW_STCR0 & LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT; if int_status==0 && int_status_lss==0: OK else test_err++. Else: test_err++."
      ],
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_I2C0_FF, MIZAR_I2C1_FF, MIZAR_I2C0_INTR_CLR, MIZAR_I2C1_INTR_CLR, MIZAR_I2C0_DEV_CTRL, MIZAR_I2C1_DEV_CTRL, MIZAR_I2C1_SLV_ADDR, MIZAR_I2C0_TGT_SLV_ADDR, MIZAR_I2C0_I2C_BYTE_CNT, MIZAR_I2C1_I2C_BYTE_CNT, MIZAR_I2C0_SF_LCNT, MIZAR_I2C0_SF_HCNT, MIZAR_I2C0_I2C_HS_LCNT, MIZAR_I2C0_I2C_HS_HCNT, MIZAR_I2C0_I2C_MSTR_CODE, MIZAR_I2C0_MASK_INTR, MIZAR_I2C0_TX_FIFO_THLD, MIZAR_I2C1_TX_FIFO_THLD, MIZAR_I2C0_RX_FIFO_THLD, MIZAR_I2C1_RX_FIFO_THLD, MIZAR_I2C0_DMA_CTRL, MIZAR_I2C0_SMB_HST_BLOCK_DATA, MIZAR_DMA_CH0_CTRL, MIZAR_DMA_CH0_SRC_ADDR, MIZAR_DMA_CH0_DEST_ADDR, MIZAR_DMA_CH0_SRC_XCNT, MIZAR_DMA_CH0_SRC_XMDFY, MIZAR_DMA_CH0_DEST_XMDFY, MIZAR_DMA_CH0_SRC_REQ, MIZAR_DMA_DMA_CH_EN, MIZAR_I2C0_TSFR_CTRL, MIZAR_I2C0_I2C_CURRENT_BYTECNT, MIZAR_I2C1_DMA_CTRL, MIZAR_I2C1_SMB_HST_BLOCK_DATA, MIZAR_DMA_CH1_CTRL, MIZAR_DMA_CH1_SRC_ADDR, MIZAR_DMA_CH1_DEST_ADDR, MIZAR_DMA_CH1_SRC_XCNT, MIZAR_DMA_CH1_SRC_XMDFY, MIZAR_DMA_CH1_DEST_XMDFY, MIZAR_DMA_CH1_SRC_REQ, MIZAR_I2C0_INTR_STS, MIZAR_LSS_SYSREG_RAW_STCR0",
      "Hidden_Validation_Acceptance_Criteria": "ISR: int_status==0x0010 indicates transfer complete; after WRITE MIZAR_I2C0_INTR_CLR=0x10 and WRITE MIZAR_LSS_SYSREG_RAW_STCR0=LSS_SYSREG_RAW_STCR0_I2C0_INTERRUPT and GIC clears, subsequent reads yield MIZAR_I2C0_INTR_STS==0x00 and (MIZAR_LSS_SYSREG_RAW_STCR0 & LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT)==0x00. Data integrity: for i=0..4, READ(SRAM_ADDR_1+i*4)==READ(SRAM_ADDR_2+i*4). No increments to test_err; finish(0) for pass else finish(test_err>0) fail."
    },
    {
      "Index": "3",
      "SS / Module": "I2C",
      "Feature": "Standard mode (100 Kb/s)",
      "Test Case Name": "test_i2c0_mst_i2c1_slv_dma_st_md",
      "Test Description": "Configures a master–slave transfer from I2C0 to I2C1 using DMA in standard mode, moves a small block of data through the host block data window, verifies interrupt handling, and checks end-to-end data integrity.",
      "Speed": "Standard mode",
      "Mode": "DMA, Interrupt",
      "Memory Start Offset": "0xA0243FC0",
      "Memory End Offset": "0xA0243FF0",
      "Remarks": "Requires system configuration for the interrupt path and enabling platform IRQs; uses a fixed-length transfer with a single expected transfer-complete interrupt.",
      "Test Steps / Procedure": [
        "Initialize platform access control for the interconnect interface to allow the intended access mode.",
        "Enable the platform interrupt lines corresponding to the I2C controllers.",
        "Enable top-level interrupt sources for both I2C instances.",
        "Flush FIFOs and clear any pending interrupts on both controllers.",
        "Program device control for both controllers; set the slave address and the master's target address; set the same transfer byte count on both sides.",
        "Program standard-mode serial clock timing; unmask the transfer-complete interrupt at the master.",
        "Configure transmit and receive FIFO thresholds on both controllers.",
        "Enable DMA on the master side and preload the source SRAM buffer with test words.",
        "Program and enable the DMA transmit channel to move data from the source SRAM to the master's host block data register.",
        "Wait for the DMA hardware to indicate completion.",
        "Initiate the I2C transfer at the master.",
        "Poll the master's current byte count until all bytes are transmitted, inserting waits between reads.",
        "Wait for the transfer-complete interrupt service routine to run.",
        "Enable DMA on the slave side, configure and enable the DMA receive channel to move data into the destination SRAM region.",
        "Wait for the receive DMA channel to complete.",
        "Compare the source and destination SRAM words to confirm data integrity.",
        "Conclude with pass if all integrity checks and interrupt handling complete successfully; otherwise record failure.",
        "Interrupt service: upon interrupt, verify the transfer-complete source, clear the controller interrupt, clear platform IRQs, clear the raw system status, briefly wait, and verify that both controller and system interrupt status are deasserted; log an error if any step fails."
      ],
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": [
        "A transfer-complete interrupt is observed and successfully cleared at the controller and system levels.",
        "All words received in the destination SRAM match the source SRAM contents.",
        "The test passes only if no interrupt-clearing or data-integrity errors are recorded."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_i2c0_mst_i2c1_slv_dma_st_md",
      "Hidden_Test_Description": "I2C0 master to I2C1 slave DMA standard mode: preloads 5 words at SRAM_ADDR_1 (0xA0243FC0), DMA CH0 to MIZAR_I2C0_SMB_HST_BLOCK_DATA, starts transfer, waits for ISR, then DMA CH1 from MIZAR_I2C1_SMB_HST_BLOCK_DATA to SRAM_ADDR_2 (0xA0243FE0); compares data; finish(test_err).",
      "Hidden_Remarks": "Requires configuring 0xA1700008..0xA1700054 for non-secure access; uses GIC IRQs 80 and 81; byte count=0x5; FIFO thresholds=0x5; unmask via 0xFFFFFFEF at master; uses waits wait_on(5000), wait_on(10), wait_on(100).",
      "Hidden_Test_Steps_Procedure": [
        "Entry: test_case()",
        "WRITE 0xA1700008..0xA1700054 each = 0x1 to set non-secure access.",
        "GIC_EnableIRQ(80); GIC_EnableIRQ(81).",
        "WRITE MIZAR_LSS_SYSREG_INTR_EN0 = LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT | LSS_SYSREG_INTR_EN0_I2C1_INTERRUPT.",
        "WRITE MIZAR_I2C0_FF=0x3; WRITE MIZAR_I2C1_FF=0x3.",
        "WRITE MIZAR_I2C0_INTR_CLR=0xFFFFFFFF; WRITE MIZAR_I2C1_INTR_CLR=0xFFFFFFFF.",
        "WRITE MIZAR_I2C0_DEV_CTRL=0x382; WRITE MIZAR_I2C1_DEV_CTRL=0x382.",
        "WRITE MIZAR_I2C1_SLV_ADDR=0x37; WRITE MIZAR_I2C0_TGT_SLV_ADDR=0x6E.",
        "WRITE MIZAR_I2C0_I2C_BYTE_CNT=0x5; WRITE MIZAR_I2C1_I2C_BYTE_CNT=0x5.",
        "WRITE MIZAR_I2C0_SF_LCNT=0x12C; WRITE MIZAR_I2C0_SF_HCNT=0xC8.",
        "WRITE MIZAR_I2C0_MASK_INTR=0xFFFFFFEF.",
        "WRITE MIZAR_I2C0_TX_FIFO_THLD=0x5; WRITE MIZAR_I2C1_TX_FIFO_THLD=0x5.",
        "WRITE MIZAR_I2C0_RX_FIFO_THLD=0x5; WRITE MIZAR_I2C1_RX_FIFO_THLD=0x5.",
        "WRITE MIZAR_I2C0_DMA_CTRL=0x2.",
        "Set src_addr=SRAM_ADDR_1(0xA0243FC0); dest_addr=MIZAR_I2C0_SMB_HST_BLOCK_DATA.",
        "For i=0..4: WRITE (0xA0243FC0 + i*4) = (i*5).",
        "DMA CH0 TX: WRITE MIZAR_DMA_CH0_CTRL=0x8028028; WRITE MIZAR_DMA_CH0_SRC_ADDR=0xA0243FC0; WRITE MIZAR_DMA_CH0_DEST_ADDR=MIZAR_I2C0_SMB_HST_BLOCK_DATA; WRITE MIZAR_DMA_CH0_SRC_XCNT=0x5; WRITE MIZAR_DMA_CH0_SRC_XMDFY=0x4; WRITE MIZAR_DMA_CH0_DEST_XMDFY=0x0; WRITE MIZAR_DMA_CH0_SRC_REQ=0x5; WRITE MIZAR_DMA_DMA_CH_EN=0x1.",
        "dma_disable(): Poll READ MIZAR_DMA_DMA_CH_EN until 0; wait_on(100).",
        "WRITE MIZAR_I2C0_TSFR_CTRL=0x2 to start transfer.",
        "Poll READ MIZAR_I2C0_I2C_CURRENT_BYTECNT until 0; inside loop wait_on(5000).",
        "Set int_pend=1; while(int_pend) { wait_on(10); }",
        "WRITE MIZAR_I2C1_DMA_CTRL=0x1; set src_addr=MIZAR_I2C1_SMB_HST_BLOCK_DATA; dest_addr=SRAM_ADDR_2(0xA0243FE0).",
        "DMA CH1 RX: WRITE MIZAR_DMA_CH1_CTRL=0x8024028; WRITE MIZAR_DMA_CH1_SRC_ADDR=MIZAR_I2C1_SMB_HST_BLOCK_DATA; WRITE MIZAR_DMA_CH1_DEST_ADDR=0xA0243FE0; WRITE MIZAR_DMA_CH1_SRC_XCNT=0x5; WRITE MIZAR_DMA_CH1_SRC_XMDFY=0x0; WRITE MIZAR_DMA_CH1_DEST_XMDFY=0x4; WRITE MIZAR_DMA_CH1_SRC_REQ=0x6; WRITE MIZAR_DMA_DMA_CH_EN=0x2.",
        "dma_disable(): Poll READ MIZAR_DMA_DMA_CH_EN until 0; wait_on(100).",
        "Compare loop: addr=0xA0243FC0; addr1=0xA0243FE0; for i=0..4: READ data_sent=READ(addr); READ data_rcvd=READ(addr1); if equal OK else test_err++; addr+=4; addr1+=4.",
        "wait_on(100); finish(test_err).",
        "ISR Default_IRQHandler(): set int_pend=0; READ int_status=READ MIZAR_I2C0_INTR_STS; if int_status==0x0010 then WRITE MIZAR_I2C0_INTR_CLR=0x00000010; GIC_ClearIRQ(80); GIC_ClearIRQ(81); WRITE MIZAR_LSS_SYSREG_RAW_STCR0=LSS_SYSREG_RAW_STCR0_I2C0_INTERRUPT; wait_on(100); READ int_status=READ MIZAR_I2C0_INTR_STS; READ int_status_lss=READ MIZAR_LSS_SYSREG_RAW_STCR0 & LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT; if int_status==0 && int_status_lss==0 OK else test_err++; else test_err++."
      ],
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_I2C0_FF, MIZAR_I2C1_FF, MIZAR_I2C0_INTR_CLR, MIZAR_I2C1_INTR_CLR, MIZAR_I2C0_DEV_CTRL, MIZAR_I2C1_DEV_CTRL, MIZAR_I2C1_SLV_ADDR, MIZAR_I2C0_TGT_SLV_ADDR, MIZAR_I2C0_I2C_BYTE_CNT, MIZAR_I2C1_I2C_BYTE_CNT, MIZAR_I2C0_SF_LCNT, MIZAR_I2C0_SF_HCNT, MIZAR_I2C0_MASK_INTR, MIZAR_I2C0_TX_FIFO_THLD, MIZAR_I2C1_TX_FIFO_THLD, MIZAR_I2C0_RX_FIFO_THLD, MIZAR_I2C1_RX_FIFO_THLD, MIZAR_I2C0_DMA_CTRL, MIZAR_I2C0_SMB_HST_BLOCK_DATA, MIZAR_DMA_CH0_CTRL, MIZAR_DMA_CH0_SRC_ADDR, MIZAR_DMA_CH0_DEST_ADDR, MIZAR_DMA_CH0_SRC_XCNT, MIZAR_DMA_CH0_SRC_XMDFY, MIZAR_DMA_CH0_DEST_XMDFY, MIZAR_DMA_CH0_SRC_REQ, MIZAR_DMA_DMA_CH_EN, MIZAR_I2C0_TSFR_CTRL, MIZAR_I2C0_I2C_CURRENT_BYTECNT, MIZAR_I2C1_DMA_CTRL, MIZAR_I2C1_SMB_HST_BLOCK_DATA, MIZAR_DMA_CH1_CTRL, MIZAR_DMA_CH1_SRC_ADDR, MIZAR_DMA_CH1_DEST_ADDR, MIZAR_DMA_CH1_SRC_XCNT, MIZAR_DMA_CH1_SRC_XMDFY, MIZAR_DMA_CH1_DEST_XMDFY, MIZAR_DMA_CH1_SRC_REQ, MIZAR_I2C0_INTR_STS, MIZAR_LSS_SYSREG_RAW_STCR0",
      "Hidden_Validation_Acceptance_Criteria": "ISR: int_status==0x0010 then after clearing controller and system, MIZAR_I2C0_INTR_STS==0x00 and (MIZAR_LSS_SYSREG_RAW_STCR0 & LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT)==0x00; else error. Data integrity: for i=0..4, READ(0xA0243FC0+i*4)==READ(0xA0243FE0+i*4). Pass if test_err remains 0 and finish(0)."
    }
  ]
}'''

# Columns definitions
MAIN_COLUMNS = [
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
    "Code Generation (Required / Not)",
]

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]


def to_ist_now():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(tz=ist)


def join_if_list(val):
    if isinstance(val, list):
        return "\n".join(str(x) for x in val)
    return val


def build_workbook(test_cases):
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Build union of keys preserving first appearance order
    seen = []
    for tc in test_cases:
        for k in tc.keys():
            if k not in seen:
                seen.append(k)

    # Write header
    for c, k in enumerate(seen, start=1):
        cell = ws.cell(row=1, column=c, value=k)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
    ws.freeze_panes = "A2"

    # Write rows
    for r, tc in enumerate(test_cases, start=2):
        for c, k in enumerate(seen, start=1):
            val = tc.get(k, "")
            ws.cell(row=r, column=c, value=join_if_list(val))

    # Create Meta_data_sheet and copy META columns
    meta = wb.create_sheet("Meta_data_sheet")
    for c, k in enumerate(META_COLUMNS, start=1):
        cell = meta.cell(row=1, column=c, value=k)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
    for r, tc in enumerate(test_cases, start=2):
        for c, k in enumerate(META_COLUMNS, start=1):
            meta.cell(row=r, column=c, value=join_if_list(tc.get(k, "")))

    # Very hide meta sheet
    meta.sheet_state = 'veryHidden'

    # Prepare TestPlan from Data: keep only MAIN columns, same order
    # Rebuild Data content into TestPlan
    data = wb["Data"]
    plan = data
    plan.title = "TestPlan"

    # Map existing columns by header name
    header_map = {}
    max_col = plan.max_column
    for c in range(1, max_col + 1):
        key = str(plan.cell(row=1, column=c).value)
        header_map[key] = c

    # Build new table content for TestPlan
    rows = []
    rows.append(MAIN_COLUMNS)
    max_row = plan.max_row
    for r in range(2, max_row + 1):
        row_vals = []
        for k in MAIN_COLUMNS:
            if k in header_map:
                val = plan.cell(row=r, column=header_map[k]).value
            else:
                val = ""
            row_vals.append(val)
        rows.append(row_vals)

    # Clear existing cells and rewrite only MAIN columns
    for row in plan[1:plan.max_row]:
        for cell in row:
            cell.value = None
    
    for r, row_vals in enumerate(rows, start=1):
        for c, v in enumerate(row_vals, start=1):
            plan.cell(row=r, column=c, value=v)

    # Formatting for TestPlan
    header_fill = PatternFill(start_color="FF1F4E79", end_color="FF1F4E79", fill_type="solid")
    thin = Side(style='thin', color='FF000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    # Apply header styles
    for c in range(1, len(MAIN_COLUMNS) + 1):
        cell = plan.cell(row=1, column=c)
        cell.font = Font(bold=True, color="FFFFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=False)
        cell.fill = header_fill
        cell.border = border

    # Wrap specific columns and set alignment/borders for all data cells
    wrap_cols = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }
    col_index = {MAIN_COLUMNS[i]: i+1 for i in range(len(MAIN_COLUMNS))}

    data_rows = plan.max_row
    data_cols = len(MAIN_COLUMNS)

    for r in range(2, data_rows + 1):
        for c in range(1, data_cols + 1):
            cell = plan.cell(row=r, column=c)
            key = MAIN_COLUMNS[c-1]
            # Alignment
            if key == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=(key in wrap_cols))
            elif key in wrap_cols:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=True)
            else:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=False)
            cell.border = border

    # AutoFilter across header
    plan.auto_filter.ref = plan.dimensions

    # Freeze top row
    plan.freeze_panes = "A2"

    # Approximate autofit column widths
    for idx, col_name in enumerate(MAIN_COLUMNS, start=1):
        max_len = len(str(col_name)) + 2
        for r in range(2, data_rows + 1):
            v = plan.cell(row=r, column=idx).value
            if v is None:
                continue
            s = str(v)
            # consider wrapped text newlines
            for part in s.split("\n"):
                if len(part) + 2 > max_len:
                    max_len = len(part) + 2
        # cap width
        max_len = min(max_len, 80)
        plan.column_dimensions[chr(64+idx) if idx<=26 else (chr(64+(idx-1)//26) + chr(64+(idx-1)%26+1))].width = max_len

    # Data validation for Code Generation (Required / Not)
    code_col = col_index["Code Generation (Required / Not)"]
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=True)
    plan.add_data_validation(dv)
    dv.add(f"{plan.cell(row=2, column=code_col).coordinate}:{plan.cell(row=data_rows, column=code_col).coordinate}")

    return wb


def commit_file(file_path: Path, message: str):
    # Configure git and commit the file
    subprocess.run(["git", "config", "user.email", "github-actions[bot]@users.noreply.github.com"], check=True)
    subprocess.run(["git", "config", "user.name", "github-actions[bot]"], check=True)
    subprocess.run(["git", "add", str(file_path)], check=True)
    subprocess.run(["git", "commit", "-m", message], check=True)
    subprocess.run(["git", "push", "origin", f"HEAD:{BRANCH}"], check=True)


def main():
    try:
        payload = json.loads(JSON_DATA)
    except Exception as e:
        print("[FAILURE] Invalid JSON input:", e)
        sys.exit(2)

    if not isinstance(payload, dict) or "test_cases" not in payload or not isinstance(payload["test_cases"], list):
        print("[FAILURE] JSON must contain 'test_cases' array")
        sys.exit(3)

    test_cases = payload["test_cases"]

    # Pre-normalize arrays to newline-joined strings for Excel cells for required fields only
    normalized = []
    for tc in test_cases:
        norm = {}
        for k, v in tc.items():
            if isinstance(v, list):
                norm[k] = "\n".join(str(x) for x in v)
            else:
                norm[k] = v
        normalized.append(norm)

    # Build workbook
    wb = build_workbook(normalized)

    # Timestamp in IST for naming
    now_ist = to_ist_now()
    fname = f"{IP_NAME}_TestPlan_{now_ist.strftime('%Y%m%d')}_{now_ist.strftime('%H%M%S')}.xlsx"

    # Ensure output directory exists
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / fname

    # Save final Excel file only
    wb.save(str(out_path))

    # Commit with IST timestamp in message
    commit_msg = f"Add I2C TestPlan autogenerated by Stage1 [{now_ist.isoformat()}]"
    commit_file(out_path, commit_msg)

    print("STATUS=SUCCESS")
    print(f"ROWS={len(test_cases)}")
    print(f"COLS={len(MAIN_COLUMNS)}")
    print(f"OUTPUT={out_path}")

if __name__ == "__main__":
    main()
