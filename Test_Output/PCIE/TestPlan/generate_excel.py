import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import json

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'PCIE_TestPlan_{timestamp}.xlsx'

# Create workbook
wb = openpyxl.Workbook()

# ===== TestPlan Sheet =====
ws_tp = wb.active
ws_tp.title = 'TestPlan'

tp_headers = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# Header formatting
header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

for col_idx, header in enumerate(tp_headers, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Data row
tp_data = [
    1,
    'PCIE',
    'Register Read/Write Validation',
    'pcie_reg_wr_rd_test',
    'This test validates the reset default values and read-write accessibility of PCIe controller registers across both PCIE0 and PCIE1 instances. It covers three register domains: DBI DSP controller registers (MSI_CAP_OFF_08H_REG, MSI_CAP_OFF_10H_REG, FILTER_MASK_2_OFF, AXI_MSTR_MSG_ADDR_HIGH_OFF, UTILITY_OFF), SII wrapper registers (Transmit Header2, Transmit Header3, PHY Control 23), and PHY lane registers accessed via hardcoded addresses. The test first reads all registers and verifies they contain the expected default value of 0x0. It then performs write-read-back verification using multiple data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555) for controller and SII registers, and PHY-specific patterns (0x7BAF, 0x1, 0x003B) for PHY lane registers. SII registers are written with instance-specific write masks, and PHY registers use 16-bit aligned access with a 13-bit write mask. PHY reset control registers are written to de-assert PHY reset before PHY register access. The test passes only if all reset-value checks and write-read-back checks produce zero errors.',
    'NA',
    'NA',
    '0x58',
    '0xE68A64B8',
    'Test covers two PCIe controller instances (PCIE0 and PCIE1) across three register domains: DBI DSP controller registers, SII wrapper registers, and PHY lane registers. SII registers use instance-specific write masks; PHY registers use 16-bit aligned access with a 13-bit write mask (0x1FFF). PHY reset control registers are written with value 0x01203000 to de-assert PHY reset before PHY register access. Three data patterns are used for write-read-back verification of controller and SII registers, and three separate PHY-specific patterns are used for PHY lane registers. The outer loop in the write-read-back function iterates only over the first 3 of 6 declared check values. PCIE1 DBI DSP registers, all SII registers, PHY reset control registers, and PHY lane hex-addressed registers could not be mapped to specification register names due to missing header or specification data.',
    '1. Read all five PCIE0 DBI DSP controller registers (MSI_CAP_OFF_08H_REG, MSI_CAP_OFF_10H_REG, FILTER_MASK_2_OFF, AXI_MSTR_MSG_ADDR_HIGH_OFF, UTILITY_OFF) and verify each returns the expected default value of 0x00000000.\n2. Read all five PCIE1 DBI DSP controller registers (same register set on the second PCIe instance) and verify each returns the expected default value of 0x00000000.\n3. Read all three PCIE0 SII registers (Transmit Header2, Transmit Header3, PHY Control 23) and verify each returns the expected default value of 0x00000000.\n4. Read all three PCIE1 SII registers (Transmit Header2, Transmit Header3, PHY Control 23) and verify each returns the expected default value of 0x00000000.\n5. Write the PHY reset de-assertion value 0x01203000 to both PCIE0 and PCIE1 PHY Reset Control registers.\n6. Read all three PCIE0 PHY lane registers using 16-bit aligned access and verify each returns the expected default value of 0x0.\n7. Read all three PCIE1 PHY lane registers using 16-bit aligned access and verify each returns the expected default value of 0x0.\n8. For each of three data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555), write the pattern to all five PCIE0 and all five PCIE1 DBI DSP controller registers.\n9. For each data pattern, write the pattern masked with the appropriate write mask to all three PCIE0 SII registers and all three PCIE1 SII registers (full 32-bit mask for Transmit Header registers, 0xF000F mask for PHY Control 23).\n10. Write the PHY reset de-assertion value to both PCIE0 and PCIE1 PHY Reset Control registers before each PHY write iteration.\n11. For each of three PHY-specific data patterns (0x7BAF, 0x1, 0x003B), write the pattern masked with 0x1FFF to all three PCIE0 and all three PCIE1 PHY lane registers.\n12. Read back all five PCIE0 and all five PCIE1 DBI DSP controller registers and verify the read data matches the written pattern exactly.\n13. Read back all three PCIE0 and all three PCIE1 SII registers and verify the read data matches the written pattern masked with the register-specific write mask.\n14. Read back all three PCIE0 and all three PCIE1 PHY lane registers using 16-bit aligned access and verify the read data within the 13-bit write mask (0x1FFF) matches the expected PHY pattern.\n15. Verify that both error counters are zero. Report pass if no mismatches were detected across all reset-value checks and write-read-back checks; report fail otherwise.',
    'MSI_CAP_OFF_08H_REG; MSI_CAP_OFF_10H_REG; FILTER_MASK_2_OFF; AXI_MSTR_MSG_ADDR_HIGH_OFF; UTILITY_OFF',
    '1. All DBI DSP controller registers on both PCIE0 and PCIE1 instances must read back their expected default value of 0x00000000 after reset.\n2. All SII registers on both PCIE0 and PCIE1 instances must read back their expected default value of 0x00000000 after reset.\n3. All PHY lane registers on both PCIE0 and PCIE1 instances must read back their expected default value of 0x0 after PHY reset de-assertion.\n4. For each write pattern (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555), all DBI DSP controller registers must read back the exact written value.\n5. For each write pattern, all SII registers must read back the written value masked with the register-specific write mask (full 32-bit for Transmit Header registers, 0xF000F for PHY Control 23).\n6. For each PHY-specific write pattern (0x7BAF, 0x1, 0x003B), all PHY lane registers must read back the written value within the 13-bit write mask (0x1FFF) using 16-bit aligned access.\n7. Both error counters (err1 and err2) must be zero for the test to pass.',
    ''
]

for col_idx, value in enumerate(tp_data, 1):
    cell = ws_tp.cell(row=2, column=col_idx, value=value)
    cell.alignment = wrap_alignment

# ===== MetaData Sheet =====
ws_md = wb.create_sheet('MetaData')

md_headers = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

for col_idx, header in enumerate(md_headers, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

md_data = [
    1,
    'pcie_reg_wr_rd_test',
    'This testcase validates reset default values and read-write functionality of PCIe registers for both PCIE0 and PCIE1 controller instances across three register domains. Global variables: data_rd, data_wr, data1_rd, err1=0, err2=0. (1) DBI DSP controller registers stored in rc0_ctl_addr[5]={mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG, mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF, mizar_PCIE0_DBI_DSP_UTILITY_OFF} and rc1_ctl_addr[5]={mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG, mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF, mizar_PCIE1_DBI_DSP_UTILITY_OFF}, all with ctl_default[5]={0x0,0x0,0x0,0x0,0x0}. (2) SII registers in sii0_addr[3]={mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3, mizar_PCIE0_SII_PHY_CONTROL_23} and sii1_addr[3]={mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2, mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3, mizar_PCIE1_SII_PHY_CONTROL_23}, with sii_default[3]={0x0,0x0,0x0}, sii0_write_mask[3]={0xFFFFFFFF,0xFFFFFFFF,0xF000F}, sii1_write_mask[3]={0xFFFFFFFF,0xFFFFFFFF,0xF000F}. (3) PHY lane registers in phy0_addr[3]={0xE68860B8,0xE68862B8,0xE68864B8} and phy1_addr[3]={0xE68A60B8,0xE68A62B8,0xE68A64B8}, with phy0_default[3]={0x0,0x0,0x0}, phy1_default[3]={0x0,0x0,0x0}, phy0_write_mask[3]={0x1FFF,0x1FFF,0x1FFF}, phy1_write_mask[3]={0x1FFF,0x1FFF,0x1FFF}. (4) PHY reset control via write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000) and write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000). The test_case() entry point calls chk_rst_val(), then chk_rd_wr(), then finish(err2 || err1). PHY registers use 16-bit access with alignment-based shifting: if address % 4 != 0, data is shifted right by 16 bits; otherwise masked with 0x0000FFFF.',
    '1. Global variables initialized: data_rd, data_wr, data1_rd, err1=0, err2=0. Arrays declared: rc0_ctl_addr[5]={mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG, mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF, mizar_PCIE0_DBI_DSP_UTILITY_OFF}, rc1_ctl_addr[5]={mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG, mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF, mizar_PCIE1_DBI_DSP_UTILITY_OFF}, ctl_default[5]={0x0,0x0,0x0,0x0,0x0}, sii0_addr[3]={mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3, mizar_PCIE0_SII_PHY_CONTROL_23}, sii1_addr[3]={mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2, mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3, mizar_PCIE1_SII_PHY_CONTROL_23}, sii_default[3]={0x0,0x0,0x0}, sii0_write_mask[3]={0xFFFFFFFF,0xFFFFFFFF,0xF000F}, sii1_write_mask[3]={0xFFFFFFFF,0xFFFFFFFF,0xF000F}, phy0_addr[3]={0xE68860B8,0xE68862B8,0xE68864B8}, phy1_addr[3]={0xE68A60B8,0xE68A62B8,0xE68A64B8}, phy0_default[3]={0x0,0x0,0x0}, phy1_default[3]={0x0,0x0,0x0}, phy0_write_mask[3]={0x1FFF,0x1FFF,0x1FFF}, phy1_write_mask[3]={0x1FFF,0x1FFF,0x1FFF}.\n2. test_case() entry: calls chk_rst_val(), then chk_rd_wr(), then finish(err2 || err1).\n3. chk_rst_val(): Loop i=0..4: data_rd = read_reg(rc0_ctl_addr[i]); if(data_rd != ctl_default[i]) err1++, printf FAILED. Loop i=0..4: data_rd = read_reg(rc1_ctl_addr[i]); if(data_rd != ctl_default[i]) err2++, printf FAILED. Loop i=0..2: data_rd = read_reg(sii0_addr[i]); if(data_rd != sii_default[i]) err2++, printf FAILED. Loop i=0..2: data_rd = read_reg(sii1_addr[i]); if(data_rd != sii_default[i]) err2++, printf FAILED. write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000); write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000). Loop i=0..2: data_rd = read_reg(phy0_addr[i]); if(phy0_addr[i]%4) data_rd = data_rd >> 16; else data_rd = data_rd & 0x0000FFFF; if(data_rd != phy0_default[i]) err2++, printf FAILED. Loop i=0..2: data_rd = read_reg(phy1_addr[i]); if(phy1_addr[i]%4) data_rd = data_rd >> 16; else data_rd = data_rd & 0x0000FFFF; if(data_rd != phy1_default[i]) err2++, printf FAILED.\n4. chk_rd_wr(): chk_val[6]={0xFFFFFFFF,0xAAAAAAAA,0x55555555,0x00000000,0xA5A5A5A5,0xFFFF0000}; chk_val_phy[3]={0x7BAF,0x1,0x003B}. Loop j=0..2: Write phase: Loop i=0..4: write_reg(rc0_ctl_addr[i], chk_val[j]). Loop i=0..4: write_reg(rc1_ctl_addr[i], chk_val[j]). Loop i=0..2: write_reg(sii0_addr[i], chk_val[j] & sii0_write_mask[i]). Loop i=0..2: write_reg(sii1_addr[i], chk_val[j] & sii1_write_mask[i]). write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000); write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000). Loop i=0..2: write_reg(phy0_addr[i], chk_val_phy[j] & phy0_write_mask[i]). Loop i=0..2: write_reg(phy1_addr[i], chk_val_phy[j] & phy1_write_mask[i]). Read-back phase: Loop i=0..4: data_rd = read_reg(rc0_ctl_addr[i]); if(data_rd != chk_val[j]) err1++, printf FAILED. Loop i=0..4: data_rd = read_reg(rc1_ctl_addr[i]); if(data_rd != chk_val[j]) err1++, printf FAILED. Loop i=0..2: data_rd = read_reg(sii0_addr[i]); if(data_rd != (chk_val[j] & sii0_write_mask[i])) err1++, printf FAILED. Loop i=0..2: data_rd = read_reg(sii1_addr[i]); if(data_rd != (chk_val[j] & sii1_write_mask[i])) err1++, printf FAILED. Loop i=0..2: data_rd = read_reg(phy0_addr[i]); if(phy0_addr[i]%4) data_rd >>= 16; else data_rd &= 0x0000FFFF; if((data_rd & phy0_write_mask[i]) != (chk_val_phy[j] & 0x00001FFF)) err1++, printf FAILED. Loop i=0..2: data_rd = read_reg(phy1_addr[i]); if(phy1_addr[i]%4) data_rd >>= 16; else data_rd &= 0x0000FFFF; if((data_rd & phy1_write_mask[i]) != (chk_val_phy[j] & 0x00001FFF)) err1++, printf FAILED.\n5. finish(err2 || err1): pass if both err1 and err2 are 0, fail otherwise.',
    'mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE0_DBI_DSP_UTILITY_OFF; mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE1_DBI_DSP_UTILITY_OFF; mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2; mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3; mizar_PCIE0_SII_PHY_CONTROL_23; mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2; mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3; mizar_PCIE1_SII_PHY_CONTROL_23; mizar_PCIE0_SII_PHY_RST_CONTROL; mizar_PCIE1_SII_PHY_RST_CONTROL; 0xE68860B8; 0xE68862B8; 0xE68864B8; 0xE68A60B8; 0xE68A62B8; 0xE68A64B8',
    '1. All DBI DSP controller registers on both PCIE0 and PCIE1 instances must read back their expected default value of 0x00000000 after reset.\n2. All SII registers on both PCIE0 and PCIE1 instances must read back their expected default value of 0x00000000 after reset.\n3. All PHY lane registers on both PCIE0 and PCIE1 instances must read back their expected default value of 0x0 after PHY reset de-assertion.\n4. For each write pattern (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555), all DBI DSP controller registers must read back the exact written value.\n5. For each write pattern, all SII registers must read back the written value masked with the register-specific write mask (full 32-bit for Transmit Header registers, 0xF000F for PHY Control 23).\n6. For each PHY-specific write pattern (0x7BAF, 0x1, 0x003B), all PHY lane registers must read back the written value within the 13-bit write mask (0x1FFF) using 16-bit aligned access.\n7. Both error counters (err1 and err2) must be zero for the test to pass.',
    '#include <stdlib.h>; #include <stdio.h>; #include <test_common.h>; #include <pcie.h>',
    'mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE0_DBI_DSP_UTILITY_OFF; mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE1_DBI_DSP_UTILITY_OFF; mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2; mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3; mizar_PCIE0_SII_PHY_CONTROL_23; mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2; mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3; mizar_PCIE1_SII_PHY_CONTROL_23; mizar_PCIE0_SII_PHY_RST_CONTROL; mizar_PCIE1_SII_PHY_RST_CONTROL; 0xE68860B8; 0xE68862B8; 0xE68864B8; 0xE68A60B8; 0xE68A62B8; 0xE68A64B8',
    'rc0_ctl_addr[5]={mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG, mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF, mizar_PCIE0_DBI_DSP_UTILITY_OFF}; rc1_ctl_addr[5]={mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG, mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF, mizar_PCIE1_DBI_DSP_UTILITY_OFF}; sii0_addr[3]={mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3, mizar_PCIE0_SII_PHY_CONTROL_23}; sii1_addr[3]={mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2, mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3, mizar_PCIE1_SII_PHY_CONTROL_23}; phy0_addr[3]={0xE68860B8, 0xE68862B8, 0xE68864B8}; phy1_addr[3]={0xE68A60B8, 0xE68A62B8, 0xE68A64B8}'
]

for col_idx, value in enumerate(md_data, 1):
    cell = ws_md.cell(row=2, column=col_idx, value=value)
    cell.alignment = wrap_alignment

# Freeze first row on both sheets
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# Auto-size columns with max width cap
for ws in [ws_tp, ws_md]:
    for col in ws.columns:
        max_length = 0
        col_letter = col[0].column_letter
        for cell in col:
            if cell.value:
                max_length = max(max_length, len(str(cell.value)))
        adjusted_width = min(max_length + 2, 60)
        ws.column_dimensions[col_letter].width = max(adjusted_width, 12)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
wb.save(output_path)

print(f'FILENAME={filename}')
print(f'OUTPUT_PATH={output_path}')
print(f'FILE_SIZE={os.path.getsize(output_path)}')
print('GENERATION=SUCCESS')
