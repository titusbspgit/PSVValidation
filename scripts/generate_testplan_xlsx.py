#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import io
import json
import re
import zipfile
from datetime import datetime
from pathlib import Path

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

# -------------------- Input JSON (embedded) --------------------
JSON_INPUT = r'''{
  "TC1": {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "AHB 32-bit register interface.",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Verifies default values and masked read/write behavior across GPIO-related registers using multiple data patterns.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Some registers are intentionally skipped for default checks and write/read due to access restrictions. Default value comparison ignores bit 0.",
    "Test Steps / Procedure": "1) Read each register from the predefined list if readable and not skipped, then compare the masked value against its documented default.\n2) For each of six data patterns, write masked data to each writable register that is not skipped.\n3) Read back each affected register, apply the read mask, and compare with the expected value derived from the written bits and preserved default bits.\n4) Count any mismatches from default checks or read-back comparisons and determine the final pass/fail result accordingly.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "- Default values: For each readable, non-skipped register, the masked read must match the expected default value; otherwise it is a failure.\n- Write/read: For each writable, non-skipped register and data pattern, the masked read must equal the expected value combining written and preserved default bits; otherwise it is a failure.\n- Final result: Test passes only if both default and write/read failure counters are zero.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "Program exercises two phases: (1) Default value check for all entries in addr_array[] subject to skip_rst_array[] and read_mask_array[]; (2) Write/read check using six patterns for all entries in addr_array[] subject to skip_array[], write_mask_array[], and read_mask_array[]. In chk_rst_val(): for i=0..CNT-1, if skip_rst_array[i]==1 then continue; if read_mask_array[i]==0x00000000 then continue; data_rd=read_reg(addr_array[i]); data=(data_rd & 0xfffffffe); compare data == default_value_array[i]; on mismatch, def_fail_cnt++. In chk_rd_wr(): for each data pattern in chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}, write phase: for i, if skip_array[i]==1 continue; if write_mask_array[i]==0x0 continue; write_reg(addr_array[i], (data_wr & write_mask_array[i])). Read/verify phase: for i, if skip_array[i]==1 continue; if write_mask_array[i]==0x0 continue; if read_mask_array[i]==0x0 continue; data_rd = (read_reg(addr_array[i]) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd != exp_val) wr_fail_cnt++. At end of test_case(): finish(1) if (def_fail_cnt>0 || wr_fail_cnt>0) else finish(0).",
    "Hidden_Remarks": "VRRW or other restricted registers are skipped per skip_array and skip_rst_array. Default comparison masks off bit 0 of each read value before comparison. Some addresses may not be readable or writable and are skipped accordingly.",
    "Hidden_Test_Steps_Procedure": "1) Initialize counters: def_fail_cnt=0; wr_fail_cnt=0.\n2) Default value check loop (i=0..CNT-1):\n   - addr = addr_array[i]. If skip_rst_array[i]==1, continue.\n   - If read_mask_array[i]==0x00000000, continue.\n   - data_rd = read_reg(addr). data = (data_rd & 0xfffffffe).\n   - If data == default_value_array[i] then pass for this address; else increment def_fail_cnt and print failure.\n3) Write/read check repeated for chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}:\n   - For each pattern: data_wr = chk_val[j]. Write phase (i=0..CNT-1):\n     • addr = addr_array[i]. If skip_array[i]==1, continue.\n     • If write_mask_array[i]==0x00000000, continue.\n     • write_reg(addr, (data_wr & write_mask_array[i])).\n   - Read/verify phase (i=0..CNT-1):\n     • addr = addr_array[i]. If skip_array[i]==1, continue.\n     • If write_mask_array[i]==0x00000000, continue.\n     • If read_mask_array[i]==0x00000000, continue.\n     • data_rd = (read_reg(addr) & read_mask_array[i]).\n     • wr_n = (write_mask_array[i] ^ 0xffffffff).\n     • exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])).\n     • If data_rd != exp_val then wr_fail_cnt++ and print failure; else optional PASS print.\n4) On completion: if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0).",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "Default phase: For each i where read is permitted and reset check not skipped, (read_reg(addr_array[i]) & 0xfffffffe) must equal default_value_array[i]; otherwise record failure. Write/read phase: For each pattern and i where write/read permitted and not skipped, data_rd=(read_reg(addr)&read_mask_array[i]) must equal exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])); otherwise record failure. Test passes if def_fail_cnt==0 and wr_fail_cnt==0; else fails."
  },
  "TC2": {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "interrupts can be generated based on positive edge or negative edge or level high or level low detection at GPIO input.",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Validates negative-edge interrupt behavior on GPIO pins 8 to 39 including enable, trigger, service, and clear.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Interrupt source selection depends on compile-time flags for the GPIO controller. A bounded wait of 5000 iterations is used to avoid hangs.",
    "Test Steps / Procedure": "1) Enable the appropriate system interrupt for the selected GPIO controller and unmask its GIC line.\n2) Set each pin from 8 to 39 as input and enable negative-edge detection; clear any pending raw status per pin.\n3) For each pin, enable its group interrupt bit, then generate a falling edge on that pin and wait for the interrupt with a timeout.\n4) In the interrupt handler, verify the pin input reads low, confirm the group status bit is set, then clear the per-pin and group interrupt status and the system status.\n5) Re-enable the group interrupt output for the next pin and repeat until all pins are covered.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "- Each pin must generate an interrupt upon a falling edge within the timeout window; timeouts are treated as failures.\n- After servicing, the pin input must read low; otherwise it is a failure.\n- The corresponding group status bit must be set during service and must clear after the clear sequence; failures are counted if not observed.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Negative-edge interrupt enable/validation across GPIO[8..39]. Setup: Optionally enable GIC IRQ 87 (GPIO0) or 88 (GPIO1). Enable system interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO{0/1}_INTR). Drive external pad driver to high via write_reg(0xA0243ffc, 0xffffffff). Configure each per-pin control register: for i=0..31, addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1,(1u<<20)|(1u<<18)|(1u<<16)) to set doe=1 (input), neie=1, iclr=1. For each i=0..31: wr_val=(1u<<i); Pre-clear group raw: write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val). Enable only this bit: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val). Arm wait: int_pend=1. Generate falling edge: write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val). Wait with timeout=5000 while (int_pend && timeout--) wait_on(10). If timeout==0, print error and increment test_err. ISR (Default_IRQHandler): int_pend=0; restore pad high via write_reg(0xA0243ffc,0xffffffff). raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr). Check DIN low: if ((rdata & 0x1)!=0) test_err++. Check raw bit set: if ((rdata & 0x2)!=0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if((rdata_grp & (1u<<i))==0) test_err++; Clear per-pin raw: write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), (1u<<20)|(1u<<16)); Clear group raw: write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u<<i)); Verify group clear: rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0x0) test_err++; Clear sys raw: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO{0/1}_INTR); GIC_ClearIRQ(87/88);} else test_err++.",
    "Hidden_Remarks": "Compile-time selection via GPIO0/GPIO1 controls which system interrupt and GIC line are used. External pad driver register 0xA0243ffc is used to create edges. Timeout is fixed at 5000 iterations.",
    "Hidden_Test_Steps_Procedure": "1) Initialize test_err=0. Conditionally enable GIC IRQ 87 or 88 based on GPIO0/GPIO1 compile defines.\n2) Conditionally enable system interrupt via MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR.\n3) Drive pad driver to known high: write_reg(0xA0243ffc, 0xffffffff).\n4) For i=0..31: addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1,(1u<<20)|(1u<<18)|(1u<<16)) to set doe=1 (input), neie=1, iclr=1 (clear raw).\n5) Loop i=0..31 per pin:\n   - wr_val=(1u<<i);\n   - write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) to clear raw bit for the pin.\n   - write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) to enable only this pin interrupt.\n   - int_pend=1; generate falling edge via write_reg(0xA0243ffc,0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val).\n   - Wait loop with timeout=5000: while(int_pend && timeout--) wait_on(10). If timeout==0 then printf error and test_err++.\n6) Default_IRQHandler():\n   - int_pend=0; write_reg(0xA0243ffc,0xffffffff) to restore high.\n   - raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr). If ((rdata & 0x1)!=0) test_err++.\n   - If ((rdata & 0x2)!=0x0): rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if((rdata_grp & (1u<<i))==0) test_err++.\n   - write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4),(1u<<20)|(1u<<16)) to keep doe=1 and clear raw.\n   - write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,(1u<<i)).\n   - rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0x0) test_err++.\n   - Clear sys raw: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO0_INTR or _GPIO1_INTR). Then GIC_ClearIRQ(87 or 88).\n   - Else (raw bit not set) test_err++.\n7) finish(test_err).",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "For each pin i in [8..39], bounded wait must terminate by ISR clearing int_pend; timeout==0 indicates failure. In ISR: (rdata & 0x1)==0 confirms DIN low for negedge; group status read from MIZAR_GPIO_GP0_INTR1_INTR_STS1 must have bit (1u<<i) set before clearing, and must read back 0x0 after clearing raw via per-pin write and MIZAR_GPIO_GPIO_INTR_RAW_STCLR1. System raw clear via MIZAR_LSS_SYSREG_RAW_STCR1 must be performed. Any violation increments test_err; finish(test_err) indicates pass if zero."
  },
  "TC3": {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "interrupts can be generated based on positive edge or negative edge or level high or level low detection at GPIO input.",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Validates positive-edge interrupt behavior for GPIO pins 8 to 39, including enable, trigger, service, and clearing.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Interrupt source selection depends on compile-time flags for the GPIO controller. A bounded wait of 2000 iterations is used to avoid hangs.",
    "Test Steps / Procedure": "1) Enable the appropriate system interrupt for the selected GPIO controller and unmask its GIC line.\n2) Enable positive-edge detection per pin for 8 to 39 and configure the pins as inputs using group I/O control.\n3) Enable the group interrupt output, then for each pin generate a single rising edge and wait for the interrupt with a timeout.\n4) In the interrupt handler, mask the group, verify the group status is set, clear all per-pin raw status, and confirm the group status clears.\n5) Clear the system-level interrupt status, re-enable the group interrupt output, and continue to the next pin until all pins are tested.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "- Each pin must generate an interrupt on a rising edge within the timeout window; timeouts are treated as failures.\n- Group interrupt status must be set during service and must read as cleared after the clear sequence; failures are counted if not observed.\n- System-level interrupt status must clear after the write to the system clear register; failures are counted if residual status remains.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Positive-edge interrupt enable/validation across GPIO[8..39]. Setup: Conditionally enable GIC IRQ 87 (GPIO0) or 88 (GPIO1). Enable system interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO{0/1}_INTR). For i=0..31, write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00020000) to set posedge enable (PEIE bit17=1). Configure input mode via group I/O control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,0x000000FF); ... GROUP4 likewise. Enable all bits in group interrupt: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc,0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc,0xFFFFFFFF) to create rising edge; bounded wait with timeout=2000 while(int_pend==1){wait_on(10)}; on timeout, print error and test_err++. In Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); mask group via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0x00000000); If any bit set in rdata_grp, success log else error/test_err++. Clear raw per-pin by writing 0x00010000 to each (for j=0..31) at MIZAR_GPIO_GP0_GPIO_8+(j*4). Verify group clear by reading MIZAR_GPIO_GP0_INTR1_INTR_STS1 == 0x0 else error/test_err++. Clear system status via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO{0/1}_INTR) and verify cleared by reading back and checking the bit is 0; if not, increment test_err. Re-enable group interrupt via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0xFFFFFFFF). Clear corresponding GIC line.",
    "Hidden_Remarks": "Compile-time selection via GPIO0/GPIO1 controls which system interrupt and GIC line are used. External pad driver register 0xA0243ffc is used to generate edges. Timeout is fixed at 2000 iterations.",
    "Hidden_Test_Steps_Procedure": "1) Conditionally enable GIC IRQ (87 or 88) based on GPIO0/GPIO1 defines.\n2) Enable system interrupt via MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR.\n3) For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00020000) to set posedge enable (PEIE=1 at bit17).\n4) Configure input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,0x000000FF); GROUP2..4 likewise to 0x000000FF.\n5) Enable all group interrupt bits: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0xFFFFFFFF).\n6) For i=0..31: drive low via write_reg(0xA0243ffc,0x00000000); wait_on(10); set int_pend=1; drive high via write_reg(0xA0243ffc,0xFFFFFFFF). Wait with timeout=2000 while(int_pend==1) wait_on(10). If timeout expires, log error and increment test_err.\n7) Default_IRQHandler():\n   - wr_val=(1<<i); int_pend=0.\n   - rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); mask group via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0x00000000).\n   - If (rdata_grp & 0xffffffff)!=0 then success log else error and test_err++.\n   - For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8+(j*4), 0x00010000) to clear raw per-pin; wait_on(2).\n   - rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0x0) error and test_err++ else success log.\n   - Clear sys raw via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO{0/1}_INTR); read back and if corresponding bit remains set, increment test_err.\n   - Re-enable group interrupt via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0xFFFFFFFF); clear GIC IRQ (87/88).\n8) finish(test_err).",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "For each pin i in [8..39], bounded wait must complete via ISR clearing int_pend; timeouts constitute failure. Group interrupt status read from MIZAR_GPIO_GP0_INTR1_INTR_STS1 must be nonzero during service and must be 0x0 after clearing all per-pin raw bits; otherwise failure. System-level status must clear after writing MIZAR_LSS_SYSREG_RAW_STCR1; residual bit indicates failure. Test passes if test_err==0."
  }
}'''

# -------------------- Helpers --------------------

def parse_json_array(json_text: str):
    obj = json.loads(json_text)
    # Enforce array order: [TC1, TC2, TC3] if present
    order = ["TC1", "TC2", "TC3"]
    rows = [obj[k] for k in order if k in obj]
    if not rows:
        raise ValueError("Empty JSON after extraction of TC1..TC3")
    return rows


def union_keys_preserve_order(rows):
    seen = set()
    keys = []
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                keys.append(k)
    return keys


def col_letter(idx: int):
    return get_column_letter(idx)


def guess_width(value: str, min_width: int = 10, max_width: int = 120) -> int:
    if value is None:
        return min_width
    length = len(str(value))
    return max(min_width, min(max_width, int(length * 1.2) + 2))


def normalize_numbered_block(text: str) -> str:
    if text is None:
        return ""
    # Split by lines and strip existing bullets/numbering
    lines = [l.strip() for l in str(text).splitlines()]
    cleaned = []
    pat = re.compile(r"^(?:[-•]\s*|\(?\d+\)?[\.:\)]\s*)")
    for l in lines:
        l2 = pat.sub("", l).strip()
        if l2:
            cleaned.append(l2)
    # Re-number
    out = []
    for i, l in enumerate(cleaned, start=1):
        out.append(f"{i}. {l}")
    return "\n".join(out)


def apply_borders(ws):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def autofit_columns(ws):
    widths = {}
    for r in range(1, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            v = ws.cell(row=r, column=c).value
            w = guess_width(v)
            widths[c] = max(widths.get(c, 10), w)
    for c, w in widths.items():
        ws.column_dimensions[col_letter(c)].width = w


def adjust_row_heights(ws):
    for r in range(1, ws.max_row + 1):
        # Estimate lines by counting newlines in wrapped columns
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            v = ws.cell(row=r, column=c).value
            if isinstance(v, str):
                max_lines = max(max_lines, v.count("\n") + 1)
        ws.row_dimensions[r].height = min(15 * max_lines, 300)


def validate_xlsx(path: Path) -> bool:
    if not zipfile.is_zipfile(path):
        return False
    with zipfile.ZipFile(path, 'r') as zf:
        names = set(zf.namelist())
        required = {"[Content_Types].xml", "xl/workbook.xml"}
        if not required.issubset(names):
            return False
    # Try reopen via openpyxl
    _ = load_workbook(filename=path, read_only=True)
    return True


# -------------------- Main generation --------------------

def main():
    rows = parse_json_array(JSON_INPUT)
    keys = union_keys_preserve_order(rows)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header row
    for ci, k in enumerate(keys, start=1):
        cell = ws.cell(row=1, column=ci, value=k)
        cell.font = Font(bold=True, color='FFFFFFFF')
    ws.freeze_panes = "A2"

    # Data rows
    for ri, row in enumerate(rows, start=2):
        for ci, k in enumerate(keys, start=1):
            ws.cell(row=ri, column=ci, value=row.get(k, ""))

    # Basic header fill (temporary; final styling applied later after reorg)
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    for ci in range(1, len(keys) + 1):
        ws.cell(row=1, column=ci).fill = header_fill
        ws.cell(row=1, column=ci).alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)

    autofit_columns(ws)

    # Create META sheet with specified hidden columns
    meta_cols = [
        "Hidden_Test_Case_Name",
        "Hidden_Test_Description",
        "Hidden_Remarks",
        "Hidden_Test_Steps_Procedure",
        "Hidden_Impacted_Registers",
        "Hidden_Validation_Acceptance_Criteria",
    ]
    ws_meta = wb.create_sheet(title="Meta_data_sheet")
    for ci, k in enumerate(meta_cols, start=1):
        ws_meta.cell(row=1, column=ci, value=k).font = Font(bold=True)
    for ri, row in enumerate(rows, start=2):
        for ci, k in enumerate(meta_cols, start=1):
            ws_meta.cell(row=ri, column=ci, value=row.get(k, ""))
    # Very hidden
    ws_meta.sheet_state = 'veryHidden'

    # Numbering transform on Data sheet
    hdrs = keys
    def idx_of(col_name):
        try:
            return hdrs.index(col_name) + 1
        except ValueError:
            return None

    for target in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        cidx = idx_of(target)
        if cidx:
            for r in range(2, ws.max_row + 1):
                val = ws.cell(row=r, column=cidx).value
                ws.cell(row=r, column=cidx, value=normalize_numbered_block(val))
                ws.cell(row=r, column=cidx).alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')

    # Rename Data -> TestPlan (must operate in place)
    ws.title = "TestPlan"

    # Column pruning and ordering on TestPlan
    main_order = [
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

    # Build a row-major cache from existing sheet
    existing_hdrs = hdrs
    data_matrix = []
    for r in range(2, ws.max_row + 1):
        row_dict = {}
        for c, h in enumerate(existing_hdrs, start=1):
            row_dict[h] = ws.cell(row=r, column=c).value
        data_matrix.append(row_dict)

    # Clear sheet and rewrite with main_order
    ws.delete_rows(1, ws.max_row)

    # Write headers
    for ci, h in enumerate(main_order, start=1):
        cell = ws.cell(row=1, column=ci, value=h)
        cell.font = Font(bold=True, color='FFFFFFFF')
        cell.fill = header_fill
        cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)

    # Write data rows in new order
    for ri, rdict in enumerate(data_matrix, start=2):
        for ci, h in enumerate(main_order, start=1):
            ws.cell(row=ri, column=ci, value=rdict.get(h, ""))

    # Strict formatting
    wrap_cols = {"Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"}
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            h = ws.cell(row=1, column=c).value
            cell = ws.cell(row=r, column=c)
            if h in wrap_cols:
                cell.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            elif h == "Index":
                cell.alignment = Alignment(vertical='top', horizontal='center')
            else:
                cell.alignment = Alignment(vertical='top', horizontal='left')

    # Header finalized style already set above

    # Borders for all populated cells
    apply_borders(ws)

    # Autofit and adjust heights
    autofit_columns(ws)
    adjust_row_heights(ws)

    # Data validation on Code Generation (Required / Not)
    try:
        code_col_idx = main_order.index("Code Generation (Required / Not)") + 1
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True)
        dv.error = "Select one of: Required, Blank, Not Required"
        dv.errorTitle = "Invalid Selection"
        start = 2
        end = ws.max_row if ws.max_row >= 2 else 2
        rng = f"{col_letter(code_col_idx)}{start}:{col_letter(code_col_idx)}{end}"
        dv.add(rng)
        ws.add_data_validation(dv)
    except Exception as e:
        # Continue even if DV application fails; generation must proceed
        pass

    # Safety: ensure only TestPlan (visible) and Meta_data_sheet (veryHidden) exist; no 'Data' sheet
    assert "Data" not in [s.title for s in wb.worksheets], "Data sheet must not exist after normalization"

    # Prepare output path and name
    ip_name = os.getenv("IP_NAME", "GPIO")
    out_dir = Path(os.getenv("OUTPUT_DIR", "Test_Output/GPIO/TestPlan"))

    # Timestamp in Asia/Kolkata (UTC+05:30)
    # Compute IST by adding +5:30 to UTC
    now_utc = datetime.utcnow()
    # Manual offset +5:30
    from datetime import timedelta
    ist = now_utc + timedelta(hours=5, minutes=30)
    ts = ist.strftime("%Y%m%d_%H%M%S")
    out_name = f"{ip_name}_TestPlan_{ts}.xlsx"

    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / out_name

    wb.save(out_path)

    # Validate OOXML
    assert validate_xlsx(out_path), "XLSX validation failed"

    print(f"Wrote: {out_path}")


if __name__ == "__main__":
    main()
