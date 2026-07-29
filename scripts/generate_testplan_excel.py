#!/usr/bin/env python3
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# Embedded final aggregated JSON from previous steps
JSON_TEXT = r'''[
  {
    "Index": "1",
    "SS / Module": "PCIE",
    "Feature": "PCIe Link Training and Enumeration",
    "Test Case Name": "pcie_enumerate_test",
    "Test Description": "Exercise PCIe link bring-up and enumeration: train the PCIe link, program the DBI DSP Coherency Control 3 registers for both controllers, poll SII status until link-up, enable non-secure NIC, configure and verify SLV0/SLV1 BAR (memory base) registers, and conclude on handshake register completion.",
    "Meta Test Description": "The test initializes and enumerates PCIe by performing: (1) write_reg(0xE6004100, 0x0) to clear a handshake/scratch register; (2) link training via conditional calls link_training_dm0_x4(4) or link_training_dm1_x4(4) depending on build-time flags (DM0_RC/DM1_RC/DM0_EP/DM1_EP); (3) cache coherency programming on both PCIE0 and PCIE1 DBI DSP Coherency Control 3 registers: repeatedly read-modify-write fields [11:14], [3:6], [27:30], [19:22] to 0xF, with wait_on(20) between programming rounds; (4) poll SII0 and SII1 status at offset 0xC0 until (value & 0xD1) == 0xD1; (5) call non_secure_prot_nic(); (6) for DM0_RC: read VENDOR ID at SLV0 offset 0x0, write 0x7 to offset 0x4, call mem_base_program_dm0_x4() and mem_base_program_dm1_x4(), then wait_on(10); (7) write 0x1 to 0xE690000C/0x10/0x14/0x18/0x30/0x34; (8) disable cache programming: on both controllers, set DBI DSP Coherency Control 3 fields [11:14] and [3:6] to 0xF, but drive [19:22] to 0x0 and later set [27:30] to 0x0, with waits (wait_on(10), then additional programming and wait_on(30)); (9) program SLV1 BAR/memory base registers at offsets 0x10..0x24 to 0xFFFFFFFF, read back, then program to {0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000} and read back; (10) repeat same sequence for SLV0; (11) wait_on(10) then poll read_reg(0xE6004100) until it equals 0x12345678; (12) finish(0).",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Requires PCIe link to train successfully before BAR programming and verification. DBI DSP Coherency Control 3 must be programmed on both controllers as specified prior to status polling. The test relies on SII status register (offset 0xC0) bits to indicate link-up and a final handshake value at 0xE6004100. Non-secure NIC configuration must be enabled.",
    "Test Steps / Procedure": "1. Clear the handshake/scratch register at 0xE6004100.\n2. Initiate PCIe link training for the configured device/mode and lane width.\n3. Program the PCIE0 and PCIE1 DBI DSP Coherency Control 3 registers to enable required coherency fields; repeat after a short wait to ensure settings take effect.\n4. Poll SII0 and SII1 status (offset 0xC0) until link-up bits indicate link is up.\n5. Enable non-secure NIC access for subsequent operations.\n6. If running as designated root complex, read and log the endpoint vendor ID and initialize memory base programming, then wait briefly.\n7. Enable required system-side controls by writing 0x1 to the set of system control registers at 0xE690000C/10/14/18/30/34.\n8. Adjust coherency control on both controllers to disable specific fields per sequence; apply waits between phases.\n9. Configure SLV1 BAR/memory base registers (offsets 0x10..0x24), first writing all 0xFFFFFFFF and reading back, then program target base values and re-read to verify.\n10. Repeat the BAR/memory base configuration and verification sequence for SLV0.\n11. After a short delay, poll 0xE6004100 until it reaches 0x12345678 to confirm end-of-test handshake.\n12. End the test successfully.",
    "Meta Test Steps / Procedure": "1) write_reg(0xE6004100, 0x0);\n2) Conditional link training based on build flags:\n   - If DM0_RC: link_training_dm0_x4(4);\n   - If DM1_RC: link_training_dm1_x4(4);\n   - If DM0_EP: link_training_dm0_x4(4);\n   - If DM1_EP: link_training_dm1_x4(4);\n3) DBI coherency programming round 1 (PCIE0):\n   - rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xF);\n   - rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);\n   - write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);\n   - rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xF);\n   - rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF);\n   - write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);\n4) DBI coherency programming round 1 (PCIE1): same field programming [11:14],[3:6],[27:30],[19:22] to 0xF with read-modify-write sequences on mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF.\n5) wait_on(20); then DBI coherency programming round 2 (PCIE0): set [11:14],[3:6],[27:30],[19:22] to 0xF; write back.\n6) DBI coherency programming round 2 (PCIE1): set [11:14],[3:6] to 0xF; then set [27:30],[19:22] to 0xF; write back.\n7) data_rd = read_sii0_reg(0xC0); loop while ((data_rd & 0xD1) != 0xD1) { data_rd = read_sii0_reg(0xC0); }.\n8) data_rd = read_sii1_reg(0xC0); loop while ((data_rd & 0xD1) != 0xD1) { data_rd = read_sii1_reg(0xC0); }.\n9) non_secure_prot_nic();\n10) If DM0_RC: rd_wr_data1 = read_pcie_slv0_reg(0x0) (Vendor ID logged); write_pcie_slv0_reg(0x4, 0x7); mem_base_program_dm0_x4(); mem_base_program_dm1_x4(); wait_on(10);\n11) Write system control enables: write_reg(0xE690000C,0x1); write_reg(0xE6900010,0x1); write_reg(0xE6900014,0x1); write_reg(0xE6900018,0x1); write_reg(0xE6900030,0x1); write_reg(0xE6900034,0x1);\n12) Disable cache programming phase 1 (PCIE0): set [11:14]=0xF,[3:6]=0xF; then set [27:30]=0xF,[19:22]=0x0; write back. Phase 1 (PCIE1): same sequence with [19:22]=0x0.\n13) wait_on(10); then phase 2 (PCIE0): set [11:14]=0xF,[3:6]=0xF,[27:30]=0x0,[19:22]=0x0; write back. Phase 2 (PCIE1): same.\n14) wait_on(30);\n15) SLV1 BARs: write_pcie_slv1_reg(0x10,0xFFFFFFFF); 0x14=0xFFFFFFFF; 0x18=0xFFFFFFFF; 0x1C=0xFFFFFFFF; 0x20=0xFFFFFFFF; 0x24=0xFFFFFFFF; then read back each with read_pcie_slv1_reg().\n16) Program SLV1 BARs to target values: 0x10=0x0; 0x14=0x4; 0x18=0x20000000; 0x1C=0x40000000; 0x20=0x60000000; 0x24=0x80000000; then read back each.\n17) Repeat BAR sequence for SLV0: write all 0xFFFFFFFF (0x10..0x24), read back; then program to {0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000}, read back each.\n18) wait_on(10); then poll data_rd = read_reg(0xE6004100) until data_rd == 0x12345678.\n19) finish(0).",
    "Impacted Registers": "DBI_DSP_COHERENCY_CONTROL_3_OFF (PCIE0), DBI_DSP_COHERENCY_CONTROL_3_OFF (PCIE1)",
    "Meta Impacted Registers": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "Validation / Acceptance Criteria": "Pass if: (a) SII0 and SII1 status at offset 0xC0 indicate link-up (mask 0xD1 equals 0xD1); (b) SLV0 and SLV1 BAR/memory base registers read back match the values written (both the 0xFFFFFFFF pattern and the target base values 0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000); and (c) the handshake register at 0xE6004100 eventually equals 0x12345678.",
    "Meta Validation / Acceptance Criteria": "- Link-up check: while(((read_sii0_reg(0xC0)) & 0xD1) != 0xD1) loop; while(((read_sii1_reg(0xC0)) & 0xD1) != 0xD1) loop; Exit only when both satisfy (value & 0xD1) == 0xD1.\n- BAR verification: After write_pcie_slvX_reg(0x10..0x24, 0xFFFFFFFF), subsequent read_pcie_slvX_reg() calls must return 0xFFFFFFFF for each offset. After reprogramming to 0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000, reads must match exactly for each corresponding offset for both SLV0 and SLV1.\n- Completion handshake: Poll read_reg(0xE6004100) until it equals 0x12345678; reaching this value indicates successful test completion.",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdlib.h>\n#include <stdio.h>\n#include <test_common.h>\n#include \"pcie.h\"",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  },
  {
    "Index": "2",
    "SS / Module": "PCIE",
    "Feature": "PCIe Register Read/Write and Reset Defaults",
    "Test Case Name": "pcie_reg_wr_rd_test",
    "Test Description": "Verify PCIe DBI, SII, and PHY register accessibility by checking reset defaults and performing masked write/read-back tests across both controllers, including 16-bit PHY register handling and PHY reset control sequencing.",
    "Meta Test Description": "The test initializes error counters and performs two phases: (1) Reset default verification and (2) Register write/read-back verification. Phase 1 reads 5 DBI control registers each for PCIE0 and PCIE1 and compares them against 0x0 defaults, then reads 3 SII registers each for PCIE0 and PCIE1 against 0x0 defaults. It asserts SII PHY reset control with 0x01203000 on both controllers, then reads three PHY register addresses per controller; due to 16-bit layout, for addresses with (addr % 4) != 0 the upper 16 bits are used (data >> 16) else lower 16 bits (data & 0x0000FFFF) and compared to 0x0 defaults. Phase 2 iterates over j=0..2 with patterns chk_val[j] in {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555} and PHY patterns chk_val_phy[j] in {0x7BAF, 0x1, 0x003B}. For each iteration: writes all 5 DBI registers for both controllers with chk_val[j]; writes SII registers with (chk_val[j] & mask) using per-register write masks (for SII_PHY_CONTROL_23 mask is 0x000F000F, others 0xFFFFFFFF). Re-asserts SII PHY reset control (0x01203000) on both, then writes three PHY registers per controller with (chk_val_phy[j] & 0x1FFF) honoring per-register write masks (0x1FFF). Read-backs: DBI registers must equal chk_val[j]; SII registers must equal (chk_val[j] & mask); PHY reads use 16-bit half selection as above and must satisfy ((read_val & 0x1FFF) == (chk_val_phy[j] & 0x1FFF)). Any mismatch increments err1/err2 and prints a failure message. Test ends with finish(err2 || err1) meaning pass if no errors.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "0xE68860B8",
    "Memory End Offset": "0xE68A64B8",
    "Remarks": "PHY registers require SII PHY reset control to be asserted before access. PHY read-back compares only 16-bit halves based on address alignment and applies a 0x1FFF mask. SII register writes are masked per register; verify with masks applied. Test passes only if all default and write/read-back comparisons match across both controllers.",
    "Test Steps / Procedure": "1. Initialize the test and error counters.\n2. Read DBI control registers for both controllers and verify reset defaults.\n3. Read SII transmit/PHY control registers for both controllers and verify reset defaults.\n4. Assert the PHY reset control for both controllers to prepare PHY access.\n5. Read PHY registers on both controllers using 16-bit half selection and verify reset defaults.\n6. For each test pattern, write DBI registers on both controllers and verify read-back matches.\n7. For each test pattern, write SII registers on both controllers with the documented write masks and verify read-back matches the masked values.\n8. Re-assert PHY reset control, write masked PHY values on both controllers, and verify 16-bit read-backs match masked expectations.\n9. Report pass if no mismatches were found; otherwise report fail.",
    "Meta Test Steps / Procedure": "1) Initialize: err1=0; err2=0.\n2) For i=0..4: data_rd=read_reg(rc0_ctl_addr[i]); if(data_rd!=ctl_default[i]) {err1++; print fail}.\n3) For i=0..4: data_rd=read_reg(rc1_ctl_addr[i]); if(data_rd!=ctl_default[i]) {err2++; print fail}.\n4) For i=0..2: data_rd=read_reg(sii0_addr[i]); if(data_rd!=sii_default[i]) {err2++; print fail}.\n5) For i=0..2: data_rd=read_reg(sii1_addr[i]); if(data_rd!=sii_default[i]) {err2++; print fail}.\n6) write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000); write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000).\n7) For i=0..2: data_rd=read_reg(phy0_addr[i]); data_rd=(phy0_addr[i]%4)? (data_rd>>16):(data_rd & 0x0000FFFF); if(data_rd!=phy0_default[i]) {err2++; print fail}.\n8) For i=0..2: data_rd=read_reg(phy1_addr[i]); data_rd=(phy1_addr[i]%4)? (data_rd>>16):(data_rd & 0x0000FFFF); if(data_rd!=phy1_default[i]) {err2++; print fail}.\n9) For j=0..2 with chk_val={0xFFFFFFFF,0xAAAAAAAA,0x55555555} and chk_val_phy={0x7BAF,0x1,0x003B}:\n   9.1) For i=0..4: write_reg(rc0_ctl_addr[i], chk_val[j]).\n   9.2) For i=0..4: write_reg(rc1_ctl_addr[i], chk_val[j]).\n   9.3) For i=0..2: write_reg(sii0_addr[i], (chk_val[j] & sii0_write_mask[i])).\n   9.4) For i=0..2: write_reg(sii1_addr[i], (chk_val[j] & sii1_write_mask[i])).\n   9.5) write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000); write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000).\n   9.6) For i=0..2: write_reg(phy0_addr[i], (chk_val_phy[j] & phy0_write_mask[i])).\n   9.7) For i=0..2: write_reg(phy1_addr[i], (chk_val_phy[j] & phy1_write_mask[i])).\n   9.8) For i=0..4: data_rd=read_reg(rc0_ctl_addr[i]); if(data_rd!=chk_val[j]) {err1++; print fail}.\n   9.9) For i=0..4: data_rd=read_reg(rc1_ctl_addr[i]); if(data_rd!=chk_val[j]) {err1++; print fail}.\n   9.10) For i=0..2: data_rd=read_reg(sii0_addr[i]); if(data_rd!=(chk_val[j] & sii0_write_mask[i])) {err1++; print fail}.\n   9.11) For i=0..2: data_rd=read_reg(sii1_addr[i]); if(data_rd!=(chk_val[j] & sii1_write_mask[i])) {err1++; print fail}.\n   9.12) For i=0..2: data_rd=read_reg(phy0_addr[i]); data_rd=(phy0_addr[i]%4)? (data_rd>>16):(data_rd & 0x0000FFFF); if((data_rd & phy0_write_mask[i])!=(chk_val_phy[j] & 0x00001FFF)) {err1++; print fail}.\n   9.13) For i=0..2: data_rd=read_reg(phy1_addr[i]); data_rd=(phy1_addr[i]%4)? (data_rd>>16):(data_rd & 0x0000FFFF); if((data_rd & phy1_write_mask[i])!=(chk_val_phy[j] & 0x00001FFF)) {err1++; print fail}.\n10) finish(err2 || err1).",
    "Impacted Registers": "DBI_DSP_MSI_CAP_OFF_08H_REG; MSI_CAP_OFF_10H_REG; DBI_DSP_FILTER_MASK_2_OFF; DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; DBI_DSP_UTILITY_OFF; SII_PCIE0_TRANSMIT_HEADER2; SII_PCIE0_TRANSMIT_HEADER3; SII_PCIE1_TRANSMIT_HEADER2; SII_PCIE1_TRANSMIT_HEADER3; SII_PHY_CONTROL_23; SII_PHY_RST_CONTROL; PHY registers at 0xE68860B8, 0xE68862B8, 0xE68864B8, 0xE68A60B8, 0xE68A62B8, 0xE68A64B8",
    "Meta Impacted Registers": "mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG, mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF, mizar_PCIE0_DBI_DSP_UTILITY_OFF, mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG, mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF, mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF, mizar_PCIE1_DBI_DSP_UTILITY_OFF, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3, mizar_PCIE0_SII_PHY_CONTROL_23, mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2, mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3, mizar_PCIE1_SII_PHY_CONTROL_23, mizar_PCIE0_SII_PHY_RST_CONTROL, mizar_PCIE1_SII_PHY_RST_CONTROL",
    "Validation / Acceptance Criteria": "Pass if: (a) All DBI and SII registers on both controllers read 0x0 at reset as expected; (b) After writes, DBI registers read back the exact written pattern; (c) SII registers read back the written pattern masked by their write masks; (d) PHY 16-bit read-backs, after asserting PHY reset control, match the masked write patterns (0x1FFF mask) using the correct 16-bit half per address alignment; and (e) overall finish status indicates no errors.",
    "Meta Validation / Acceptance Criteria": "- Reset default checks: For each rc0_ctl_addr[i] and rc1_ctl_addr[i], read_reg()==0x0; for each sii0_addr[i] and sii1_addr[i], read_reg()==0x0; for each phy*_addr[i], selected 16-bit value equals 0x0.\n- Write/read-back checks: For DBI regs, read_reg(rc*_ctl_addr[i])==chk_val[j]. For SII regs, read_reg(sii*_addr[i])==(chk_val[j] & sii*_write_mask[i]) (mask 0xF000F for SII_PHY_CONTROL_23; 0xFFFFFFFF for transmit headers). For PHY regs, after SII_PHY_RST_CONTROL writes, selected 16-bit value satisfies ((read_val & 0x1FFF)==(chk_val_phy[j] & 0x1FFF)).\n- Test completion: finish(err2 || err1) must be invoked with 0 (both err1 and err2 remain 0).",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdlib.h>\n#include <stdio.h>\n#include <test_common.h>\n#include <pcie.h>",
    "Meta Macros": "NA",
    "Meta Arrays": "unsigned int rc0_ctl_addr[5] = {mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,mizar_PCIE0_DBI_DSP_UTILITY_OFF};\nunsigned int rc1_ctl_addr[5] = {mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,mizar_PCIE1_DBI_DSP_UTILITY_OFF};\nunsigned int ctl_default[5] = {0x0, 0x0, 0x0, 0x0, 0x0};\nunsigned int sii0_addr[3] = {mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,mizar_PCIE0_SII_PHY_CONTROL_23};\nunsigned int sii1_addr[3] = {mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2, mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,mizar_PCIE1_SII_PHY_CONTROL_23};\nunsigned int sii_default[3] = {0x0, 0x0,0x0};\n\nunsigned int sii0_write_mask[3] = {0xFFFFFFFF,0xFFFFFFFF,0xF000F};\nunsigned int sii1_write_mask[3] = {0xFFFFFFFF,0xFFFFFFFF,0xF000F};\n\nunsigned int phy0_addr[3] = {0xE68860B8,0xE68862B8,0xE68864B8};\nunsigned int phy1_addr[3] ={0xE68A60B8,0xE68A62B8,0xE68A64B8};\nunsigned int phy0_default[3] = {0x0,0x0,0x0};\nunsigned int phy1_default[3] = {0x0,0x0,0x0};\n\nunsigned int phy0_write_mask[3] = {0x1FFF,0x1FFF,0x1FFF};\n\nunsigned int phy1_write_mask[3] = {0x1FFF,0x1FFF,0x1FFF};"
  }
]'''

# Required columns for sheets
TESTPLAN_COLUMNS = [
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

METADATA_COLUMNS = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]


def validate_json(data):
    if not isinstance(data, list):
        raise ValueError("json_data must be a list (array)")
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            raise ValueError(f"Element at index {i} is not an object/dict")


def auto_width(ws):
    col_widths = {}
    for row in ws.iter_rows(values_only=False):
        for cell in row:
            val = cell.value
            if val is None:
                length = 0
            else:
                sval = str(val)
                length = min(len(sval), 120)
            col = cell.column
            if col not in col_widths or length > col_widths[col]:
                col_widths[col] = length
    for col, width in col_widths.items():
        # add padding; cap maximum width
        adj = min(width + 2, 80)
        ws.column_dimensions[get_column_letter(col)].width = max(adj, 10)


def main():
    # Load and validate JSON
    data = json.loads(JSON_TEXT)
    validate_json(data)

    wb = Workbook()
    ws1 = wb.active
    ws1.title = "TestPlan"
    ws2 = wb.create_sheet("MetaData")

    # Styles
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="1F4E78", end_color="1F4E78", fill_type="solid")
    wrap = Alignment(wrap_text=True, vertical="top")

    # Write headers
    ws1.append(TESTPLAN_COLUMNS)
    ws2.append(METADATA_COLUMNS)
    for cell in ws1[1]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap
    for cell in ws2[1]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap

    # Write rows (preserve order)
    for row in data:
        ws1.append([row.get(col, "") for col in TESTPLAN_COLUMNS])
        ws2.append([row.get(col, "") for col in METADATA_COLUMNS])

    # Apply wrap text to all cells and freeze first row
    for ws in (ws1, ws2):
        ws.freeze_panes = "A2"
        for r in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
            for c in r:
                c.alignment = wrap
        auto_width(ws)

    # Set MetaData sheet to VERY HIDDEN
    ws2.sheet_state = "veryHidden"

    # Build filename with IST timezone
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    ip_name = "PCIE"
    filename = f"{ip_name}_TestPlan_{ts}.xlsx"

    # Output directory from env or default
    output_dir = os.getenv("OUTPUT_DIR", "Test_Output/PCIE/TestPlan/")
    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, filename)

    wb.save(out_path)
    print(out_path)


if __name__ == "__main__":
    main()
