#!/usr/bin/env python3
"""MIPI_CSI TestPlan XLSX Generator - Agent 7 Direct Execution
Generates: MIPI_CSI_TestPlan_20250710_223000.xlsx
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from datetime import datetime, timezone, timedelta
import os
import json

# IST Timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp_str}.xlsx'

# JSON Data
json_data = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_dphy_lanes_test",
    "Feature": "DPHY Lane Configuration and Interrupt Masking",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase configures the MIPI CSI2 DPHY lanes and verifies PHY stop state. It writes to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel, writes to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data, polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait for the PHY stop state condition, writes to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the number of active DPHY lanes, writes to a hardcoded address 0xa0243ffc, reads from a hardcoded address 0xE6001000, reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check the main interrupt status, and writes to multiple interrupt mask registers (MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED) to configure interrupt masks for various fatal and non-fatal interrupt sources.",
    "Test Description": "This test validates the MIPI CSI2 DPHY lane configuration and interrupt masking functionality. It configures the virtual channel and control data registers, sets the number of active DPHY lanes via the N_LANES register, polls the PHY_STOPSTATE register to confirm the PHY has entered stop state, reads the INT_ST_MAIN register to check the main interrupt status, and writes to all interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED) to enable or disable specific interrupt sources. Additionally, it performs a write to an external address and a read from another external address as part of the test setup.",
    "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL (base 0xE6A04000, offset 0x0) to configure the virtual channel setting. 2. Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA (base 0xE6A04000, offset 0x20) to set the control data configuration. 3. Read MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE (base 0xE6A05000, offset 0x4C) and poll until the expected PHY stop state condition is met. 4. Write to MIZAR_MIPI_CSI2_HOST_N_LANES (base 0xE6A05000, offset 0x4) to configure the number of active DPHY data lanes. 5. Write to hardcoded address 0xa0243ffc as part of test setup or external configuration. 6. Read from hardcoded address 0xE6001000 as part of test setup or status check. 7. Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN (base 0xE6A05000, offset 0xC) to check the main interrupt status register. 8. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL (base 0xE6A05000, offset 0xE4) to configure the PHY fatal interrupt mask. 9. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL (base 0xE6A05000, offset 0xF4) to configure the packet fatal interrupt mask. 10. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY (base 0xE6A05000, offset 0x114) to configure the PHY interrupt mask. 11. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE (base 0xE6A05000, offset 0x134) to configure the line interrupt mask. 12. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL (base 0xE6A05000, offset 0x284) to configure the boundary frame fatal interrupt mask. 13. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL (base 0xE6A05000, offset 0x294) to configure the sequence frame fatal interrupt mask. 14. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL (base 0xE6A05000, offset 0x2A4) to configure the CRC frame fatal interrupt mask. 15. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL (base 0xE6A05000, offset 0x2B4) to configure the payload CRC fatal interrupt mask. 16. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID (base 0xE6A05000, offset 0x2C4) to configure the data ID interrupt mask. 17. Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED (base 0xE6A05000, offset 0x2D4) to configure the ECC corrected interrupt mask.",
    "Test Steps / Procedure": "1. Configure the virtual channel by writing to the virtual_channel register. 2. Configure the control data by writing to the control_data register. 3. Poll the PHY_STOPSTATE register until the PHY stop state condition is satisfied. 4. Configure the number of active DPHY data lanes by writing to the N_LANES register. 5. Perform an external write operation as part of test setup. 6. Perform an external read operation as part of test setup or status verification. 7. Read the INT_ST_MAIN register to verify the main interrupt status. 8. Configure the PHY fatal interrupt mask by writing to the INT_MSK_PHY_FATAL register. 9. Configure the packet fatal interrupt mask by writing to the INT_MSK_PKT_FATAL register. 10. Configure the PHY interrupt mask by writing to the INT_MSK_PHY register. 11. Configure the line interrupt mask by writing to the INT_MSK_LINE register. 12. Configure the boundary frame fatal interrupt mask by writing to the INT_MSK_BNDRY_FRAME_FATAL register. 13. Configure the sequence frame fatal interrupt mask by writing to the INT_MSK_SEQ_FRAME_FATAL register. 14. Configure the CRC frame fatal interrupt mask by writing to the INT_MSK_CRC_FRAME_FATAL register. 15. Configure the payload CRC fatal interrupt mask by writing to the INT_MSK_PLD_CRC_FATAL register. 16. Configure the data ID interrupt mask by writing to the INT_MSK_DATA_ID register. 17. Configure the ECC corrected interrupt mask by writing to the INT_MSK_ECC_CORRECTED register.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE (base 0xE6A05000, offset 0x4C) until the expected PHY stop state condition is met, confirming that the DPHY lanes have entered stop state after lane configuration via MIZAR_MIPI_CSI2_HOST_N_LANES. The test reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN (base 0xE6A05000, offset 0xC) to verify the main interrupt status after configuring all interrupt mask registers. Successful completion requires the PHY stop state poll to resolve and the interrupt status to reflect the expected masked/unmasked state.",
    "Validation / Acceptance Criteria": "The PHY_STOPSTATE register must indicate that the DPHY lanes have entered stop state after lane configuration via the N_LANES register. The INT_ST_MAIN register must reflect the expected interrupt status after all interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED) have been configured. The polling of PHY_STOPSTATE must complete successfully without timeout.",
    "Remarks": "Source code in the repository folder does not contain the actual MIPI CSI2 DPHY lanes test implementation; testcase details are derived from upstream Agent 2, Agent 3, and Agent 4 outputs. Two hardcoded addresses (0xa0243ffc for write, 0xE6001000 for read) could not be mapped to canonical register names. The PHY_STOPSTATE register is polled, indicating a wait-for-condition pattern. Ten interrupt mask registers are configured, covering PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt sources."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_test_pattern_generator",
    "Feature": "Test Pattern Generation",
    "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
    "Meta Macros": "NA",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase configures and enables the MIPI CSI2 host internal test pattern generator (PPI PG). It writes to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES (base 0xE6A05000, offset 0x60) to set the vertical resolution of the test pattern, writes to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES (base 0xE6A05000, offset 0x64) to set the horizontal resolution, writes to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG (base 0xE6A05000, offset 0x68) to configure the pattern generator parameters, and writes to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE (base 0xE6A05000, offset 0x6C) to enable the pattern generator. It then configures the virtual channel via MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL (base 0xE6A04000, offset 0x0) and control data via MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA (base 0xE6A04000, offset 0x20). The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE (base 0xE6A05000, offset 0x4C) to wait for the PHY stop state condition. DMA channel 0 read and write address registers are configured: MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA (base 0xE6A04000, offset 0x6C), MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION (base 0xE6A04000, offset 0x70), MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA (base 0xE6A04000, offset 0x8C), and MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION (base 0xE6A04000, offset 0x90). A write to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 is performed. The test reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN (base 0xE6A05000, offset 0xC) to check the main interrupt status. Ten interrupt mask registers are written to configure interrupt masking.",
    "Test Description": "This test validates the MIPI CSI2 host internal test pattern generator functionality. It configures the pattern generator vertical and horizontal resolution via the PPI_PG_PATTERN_VRES and PPI_PG_PATTERN_HRES registers, sets the pattern configuration via PPI_PG_CONFIG, and enables the pattern generator via PPI_PG_ENABLE. The virtual channel and control data are configured through the virtual_channel and control_data registers. The test polls PHY_STOPSTATE to confirm the PHY has entered stop state. DMA channel 0 address registers (dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction) are configured for data transfer. The INT_ST_MAIN register is read to check the main interrupt status, and all interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED) are configured. Finally, the pattern generator is disabled via PPI_PG_ENABLE.",
    "Meta Test Steps / Procedure": "1. Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES (base 0xE6A05000, offset 0x60) with value 0x10 to set the vertical resolution of the test pattern. 2. Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES (base 0xE6A05000, offset 0x64) to set the horizontal resolution of the test pattern. 3. Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG (base 0xE6A05000, offset 0x68) to configure the pattern generator mode and parameters. 4. Write 1 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE (base 0xE6A05000, offset 0x6C) to enable the test pattern generator. 5. Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL (base 0xE6A04000, offset 0x0) to configure the virtual channel. 6. Write 0 to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA (base 0xE6A04000, offset 0x20) to set the control data. 7. Read MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE (base 0xE6A05000, offset 0x4C) and poll in a while loop until the expected PHY stop state condition is met. 8-25. Configure DMA and interrupt mask registers.",
    "Test Steps / Procedure": "1. Configure the test pattern vertical resolution by writing to the PPI_PG_PATTERN_VRES register. 2. Configure the test pattern horizontal resolution by writing to the PPI_PG_PATTERN_HRES register. 3. Configure the test pattern generator parameters by writing to the PPI_PG_CONFIG register. 4. Enable the test pattern generator by writing to the PPI_PG_ENABLE register. 5. Configure the virtual channel by writing to the virtual_channel register. 6. Set the control data by writing to the control_data register. 7. Poll the PHY_STOPSTATE register until the PHY stop state condition is satisfied. 8. Configure the DMA channel 0 read address data by writing to the dma_m0_addr_ar_ch0_data register. 9. Configure the DMA channel 0 read address instruction by writing to the dma_m0_addr_ar_ch0_Instruction register. 10. Configure the DMA channel 0 write address data by writing to the dma_m0_addr_aw_ch0_data register. 11. Configure the DMA channel 0 write address instruction by writing to the dma_m0_addr_aw_ch0_Instruction register. 12. Trigger a control operation by writing to a register in the RB REG block. 13. Disable the test pattern generator by writing to the PPI_PG_ENABLE register. 14. Read the DMA interrupt masked status to check DMA interrupt state. 15. Read the INT_ST_MAIN register to verify the main interrupt status. 16-25. Configure all interrupt mask registers.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE (base 0xE6A05000, offset 0x4C) until the expected PHY stop state condition is met. The test reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN (base 0xE6A05000, offset 0xC) to verify the main interrupt status after configuring all interrupt mask registers. The pattern generator is enabled via MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with value 1 and subsequently disabled with value 0, validating the enable/disable cycle.",
    "Validation / Acceptance Criteria": "The PHY_STOPSTATE register must indicate that the DPHY lanes have entered stop state after the test pattern generator is configured and enabled via PPI_PG_ENABLE. The INT_ST_MAIN register must reflect the expected interrupt status after all interrupt mask registers have been configured. The polling of PHY_STOPSTATE must complete successfully without timeout. The pattern generator must be successfully enabled and then disabled via the PPI_PG_ENABLE register.",
    "Remarks": "The source code in the repository folder contains PCIe-related test code rather than MIPI CSI2 test pattern generator code; testcase register-level details are derived from upstream Agent 2, Agent 3, and Agent 4 outputs. The PHY_STOPSTATE register is polled indicating a wait-for-condition pattern. The test pattern generator is enabled and then disabled via PPI_PG_ENABLE, validating the enable/disable lifecycle. DMA channel 0 read and write address registers are configured for data transfer."
  }
]

# TestPlan columns
tp_cols = ['Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
           'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
           'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
           'Code Generation']

# MetaData columns
md_cols = ['Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
           'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
           'Meta Headers', 'Meta Macros', 'Meta Arrays']

# Create workbook
wb = openpyxl.Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# MetaData sheet
ws_md = wb.create_sheet('MetaData')

# Formatting
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')
thin_border = Border(
    left=Side(style='thin'),
    right=Side(style='thin'),
    top=Side(style='thin'),
    bottom=Side(style='thin')
)

def populate_sheet(ws, columns, data):
    # Write headers
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align
        cell.border = thin_border
    
    # Write data rows
    for row_idx, row_data in enumerate(data, 2):
        for col_idx, col_name in enumerate(columns, 1):
            value = row_data.get(col_name, '')
            cell = ws.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_align
            cell.border = thin_border
    
    # Auto-size columns
    for col_idx, col_name in enumerate(columns, 1):
        max_len = len(str(col_name))
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    max_len = max(max_len, min(len(str(cell.value)), 80))
        adjusted_width = min(max_len + 4, 60)
        ws.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = adjusted_width
    
    # Freeze first row
    ws.freeze_panes = 'A2'

# Populate sheets
populate_sheet(ws_tp, tp_cols, json_data)
populate_sheet(ws_md, md_cols, json_data)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save
script_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(script_dir, filename)
wb.save(output_path)

# Validate
wb2 = openpyxl.load_workbook(output_path)
assert 'TestPlan' in wb2.sheetnames
assert 'MetaData' in wb2.sheetnames
assert wb2['MetaData'].sheet_state == 'veryHidden'
file_size = os.path.getsize(output_path)
assert file_size > 0

print(f'SUCCESS: Generated {filename}')
print(f'Path: {output_path}')
print(f'Size: {file_size} bytes')
print(f'TestPlan rows: {len(json_data)}')
print(f'MetaData rows: {len(json_data)}')
print(f'Sheets: {wb2.sheetnames}')
