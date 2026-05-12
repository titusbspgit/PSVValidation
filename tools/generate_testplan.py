#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Deterministic TestPlan Excel generator.
- Reads embedded JSON data
- Produces a true binary .xlsx using openpyxl
- Applies required formatting, hidden META sheet, data validation
- Ensures final workbook contains only: TestPlan (visible) and Meta_data_sheet (veryHidden)
- Validates output as proper OOXML zip
"""

from __future__ import annotations
import json
import os
import zipfile
from typing import List, Dict, Any
from copy import deepcopy

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# -------------------- CONFIG --------------------
OUTPUT_DIR = os.path.join("Test_Output", "GPIO", "TestPlan")
OUTPUT_FILE = "GPIO_TestPlan_20260512_000000.xlsx"

# Embedded JSON data (exact, as provided)
JSON_DATA: List[Dict[str, Any]] = [
  {
    "Index": 1,
    "SS / Module": "GPIO",
    "Feature": "negative edge detection at GPIO input",
    "Test Case Name": "test_gpio_nedge_random_pads_en",
    "Test Description": "Tests negative-edge interrupt handling on randomly chosen GPIO pads by configuring inputs, generating falling edges, and verifying that pad, group, and system statuses set and clear correctly.",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "One GPIO instance must be enabled. Pads are selected randomly without repetition. A fixed memory-mapped location is used to generate edges.",
    "Test Steps / Procedure": "1) Enable the GPIO interrupt source in LSS SYSREG Interrupt Enable 1 for the selected instance.\n2) Initialize the stimulus memory location to all ones.\n3) For each selected pad, program the per-pin control register for input with negative-edge detection enabled.\n4) Enable the corresponding bit in GPIO Group Interrupt Enable 1 for that pad.\n5) Drive a falling-edge pattern using the stimulus memory location and then restore it to all ones.\n6) In the interrupt handler, read the per-pin control register and verify input and raw status, read GPIO Group Interrupt Status 1, clear the per-pin interrupt, verify the readback, verify the group status clears, and clear LSS SYSREG Raw Status Clear 1.",
    "Impacted Registers": "LSS SYSREG Interrupt Enable 1, GPIO per-pin control (GPIO_8..GPIO_39), GPIO Group Interrupt Enable 1, GPIO Group Interrupt Status 1, LSS SYSREG Raw Status Clear 1",
    "Validation / Acceptance Criteria": "1) Input status after the falling edge → Expected input level is observed.\n2) Per-pin raw interrupt status after the falling edge → Raw status indicates a negative-edge event.\n3) Group interrupt status for the active pad → Corresponding group bit is set.\n4) After per-pin clear write → Per-pin readback matches the expected cleared value.\n5) After clearing → Group interrupt status reads zero.\n6) After system raw status clear → The corresponding system status bit is cleared.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_nedge_random_pads_en",
    "Hidden_Test_Description": "...",
    "Hidden_Remarks": "GPIO0 or GPIO1 must be defined at compile time to route the interrupt. Pads in the range 8–39 are used. Pads are chosen randomly without duplication. The stimulus uses memory-mapped writes to 0xA0243ffc to create falling edges.",
    "Hidden_Test_Steps_Procedure": "...",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, ...",
    "Hidden_Validation_Acceptance_Criteria": "..."
  },
  {
    "Index": 2,
    "SS / Module": "GPIO",
    "Feature": "negative edge detection at GPIO input",
    "Test Case Name": "test_gpio_nedge_walking_zeros_pattern",
    "Test Description": "Verify negative-edge interrupt behavior across GPIO pads using a walking-zero input pattern and confirm correct status set and clear through the interrupt handler.",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "One GPIO instance must be active. The stimulus source must reflect pad inputs. The handler must clear status and continue.",
    "Test Steps / Procedure": "1) Enable system interrupt for the selected GPIO instance.\n2) For each pad, program the per-pin control to input with negative-edge detection.\n3) Set I/O control groups 1 to 4 to input mode.\n4) Enable the group interrupt mask for all pads.\n5) Apply a walking-zero input stimulus to generate falling edges on each pad.\n6) In the handler, read the per-pin control and group status, then clear per-pin status and system raw status and verify both are cleared.",
    "Impacted Registers": "System Interrupt Enable 1, GPIO per-pin control (GPIO_8..GPIO_39), GPIO I/O Control Group1, GPIO I/O Control Group2, GPIO I/O Control Group3, GPIO I/O Control Group4, GPIO Group Interrupt Enable 1, GPIO Group Interrupt Status 1, System Raw Status Clear 1",
    "Validation / Acceptance Criteria": "1) Per-pin input after a falling edge → Input bit shows the expected level.\n2) Per-pin raw status after a falling edge → Raw status indicates an interrupt event.\n3) Group interrupt status for the active pad → Corresponding group bit is set.\n4) After writing the per-pin clear value → Per-pin readback equals the expected cleared value.\n5) After clearing → Group interrupt status reads zero.\n6) After system raw clear → System raw status bit reads cleared.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_nedge_walking_zeros_pattern",
    "Hidden_Test_Description": "...",
    "Hidden_Remarks": "#ifdef GPIO0 uses IRQ 87; #ifdef GPIO1 uses IRQ 88. Stimulus generated by writes to 0xA0243ffc. Test iterates i=0..31 for GPIO_8..GPIO_39. Uses int_pend to wait for ISR.",
    "Hidden_Test_Steps_Procedure": "...",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, ...",
    "Hidden_Validation_Acceptance_Criteria": "..."
  },
  {
    "Index": 3,
    "SS / Module": "GPIO",
    "Feature": "negative edge detection at GPIO input",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Configure GPIO inputs with negative-edge interrupts, generate falling edges via a memory-mapped stimulus, and verify pad, group, and system status handling and clear through an interrupt handler.",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "GPIO0 uses interrupt 87 and GPIO1 uses interrupt 88. Pads 8 to 39 are exercised. The test relies on a memory-mapped stimulus location.",
    "Test Steps / Procedure": "...",
    "Impacted Registers": "LSS System Register Interrupt Enable 1, GPIO per-pin control (GPIO_8..GPIO_39), GPIO Group Interrupt Enable 1, GPIO Group Interrupt Status 1, LSS System Raw Status Clear 1",
    "Validation / Acceptance Criteria": "...",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "...",
    "Hidden_Remarks": "#ifdef GPIO0 path uses IRQ 87; #ifdef GPIO1 path uses IRQ 88. Pads 8–39 are targeted. Stimulus is generated by writes to 0xA0243ffc. int_pend is used to synchronize ISR completion.",
    "Hidden_Test_Steps_Procedure": "...",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, ...",
    "Hidden_Validation_Acceptance_Criteria": "..."
  },
  {
    "Index": 4,
    "SS / Module": "GPIO",
    "Feature": "dout",
    "Test Case Name": "test_gpio_op_mode_all_pad_en",
    "Test Description": "Configure pads for output, drive each pad high and low, and verify the pad status matches the driven value. Any unexpected interrupt is treated as a failure.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "One GPIO instance is enabled. Pads eight to thirty nine are used as outputs. Interrupts are not expected.",
    "Test Steps / Procedure": "1) Set all group I/O control registers to configure pads as outputs.\n2) Enable the group interrupt mask while ensuring no interrupt occurs.\n3) For each per-pin control register, drive the pad high, read the pad status register, and compare the bit for that pad.\n4) For each per-pin control register, drive the pad low, read the pad status register, and compare the bit for that pad.\n5) Treat any interrupt event as a test failure and clear the interrupt source.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1) After driving a pad high → The corresponding pad status bit reads one.\n2) After driving a pad low → The corresponding pad status bit reads zero.\n3) During execution → No interrupt occurs; any interrupt indicates failure.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_op_mode_all_pad_en",
    "Hidden_Test_Description": "Enable IRQ 87 when GPIO0 is defined or IRQ 88 when GPIO1 is defined. Configure output mode for GPIOs 8–39 by writing 0x00FF00FF to each of GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, and GPIO_IO_CTRL_GROUP4, then wait_on(10). Enable all group interrupts by writing 0xFFFFFFFF to INTR1_INTR_EN1. For i from 0 to 31: write 0x00200000 to the per-pin control register at (GPIO_8 base + i*4) to drive output high; set gp0_flag_dout_one = 1 and call check_for_pad_value(i). Wait_on(20). Write 0x00000000 to the same per-pin control register to drive output low; set gp0_flag_dout_zero = 1 and call check_for_pad_value(i). Finish with finish(test_err). In check_for_pad_value(gpio_pad_num): read rdata = read_reg(0xA0243ffc). If gp0_flag_dout_one == 1, check (rdata & (1 << gpio_pad_num)) != 0; on pass clear gp0_flag_dout_one; on fail print error, clear gp0_flag_dout_one, and increment test_err. If gp0_flag_dout_zero == 1, check (rdata & (1 << gpio_pad_num)) == 0; on pass clear gp0_flag_dout_zero; on fail print error, clear gp0_flag_dout_zero, and increment test_err. Clear IRQ 87 or 88 depending on instance. Default_IRQHandler prints an error message and increments test_err.",
    "Hidden_Remarks": "#ifdef GPIO0 uses GIC IRQ 87; #ifdef GPIO1 uses GIC IRQ 88. Pads 8–39 are forced to output using GPIO_IO_CTRL_GROUP1–4 with value 0x00FF00FF. INTR1_INTR_EN1 is set to 0xFFFFFFFF but interrupts are not expected; Default_IRQHandler increments test_err if triggered. Pad status is read from 0xA0243ffc.",
    "Hidden_Test_Steps_Procedure": "test_case():\n- test_err = 0.\n- If GPIO0: GIC_EnableIRQ(87); If GPIO1: GIC_EnableIRQ(88).\n- write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x00FF00FF);\n- write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x00FF00FF);\n- write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x00FF00FF);\n- write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x00FF00FF);\n- wait_on(10);\n- write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);\n- for (i = 0; i < 32; i++):\n  - write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00200000);\n  - gp0_flag_dout_one = 1; check_for_pad_value(i);\n  - wait_on(20);\n  - write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00000000);\n  - gp0_flag_dout_zero = 1; check_for_pad_value(i);\n- finish(test_err).\n\ncheck_for_pad_value(gpio_pad_num):\n- rdata = read_reg(0xA0243ffc).\n- If (gp0_flag_dout_one == 1):\n  - If ((rdata & (1 << gpio_pad_num)) != 0): gp0_flag_dout_one = 0; else: print error; gp0_flag_dout_one = 0; test_err++.\n- If (gp0_flag_dout_zero == 1):\n  - If ((rdata & (1 << gpio_pad_num)) == 0): gp0_flag_dout_zero = 0; else: print error; gp0_flag_dout_zero = 0; test_err++.\n- If GPIO0: GIC_ClearIRQ(87); If GPIO1: GIC_ClearIRQ(88).\n\nDefault_IRQHandler(): print error; test_err++.",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8",
    "Hidden_Validation_Acceptance_Criteria": "- If gp0_flag_dout_one == 1: (read_reg(0xA0243ffc) & (1 << gpio_pad_num)) != 0 → pass; else increment test_err.\n- If gp0_flag_dout_zero == 1: (read_reg(0xA0243ffc) & (1 << gpio_pad_num)) == 0 → pass; else increment test_err.\n- Entering Default_IRQHandler at any time → failure (test_err++)."
  }
]

# Column definitions
META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_ORDER = [
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

# -------------------- HELPERS --------------------

def union_keys_preserve_order(rows: List[Dict[str, Any]]) -> List[str]:
    seen = []
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.append(k)
    return seen


def estimate_col_width(value: Any) -> int:
    text = "" if value is None else str(value)
    # consider wrapped content by lines
    lines = text.splitlines() if text else [""]
    max_len = max((len(line) for line in lines), default=0)
    # padding factor for readability
    return min(max(10, max_len + 2), 120)


def apply_borders(ws, min_row: int, max_row: int, min_col: int, max_col: int):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(min_row, max_row + 1):
        for c in range(min_col, max_col + 1):
            cell = ws.cell(row=r, column=c)
            cell.border = border


def number_multiline(cell_text: str) -> str:
    if not cell_text:
        return ""
    # split on lines and semicolons to derive logical items
    parts: List[str] = []
    for raw_line in cell_text.splitlines():
        for seg in raw_line.split(";"):
            seg = seg.strip()
            if seg:
                parts.append(seg)
    if not parts:
        return cell_text
    return "\n".join(f"{i+1}. {p}" for i, p in enumerate(parts))


def very_hidden(ws):
    ws.sheet_state = 'veryHidden'


def validate_xlsx_zip(path: str) -> bool:
    required = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
    try:
        with zipfile.ZipFile(path, 'r') as zf:
            names = set(zf.namelist())
            if not required.issubset(names):
                return False
            # at least one worksheet
            if not any(n.startswith("xl/worksheets/") and n.endswith(".xml") for n in names):
                return False
        return True
    except Exception:
        return False

# -------------------- MAIN --------------------

def main():
    rows = deepcopy(JSON_DATA)
    if not isinstance(rows, list) or not rows:
        raise SystemExit("JSON input invalid or empty")
    for r in rows:
        if not isinstance(r, dict):
            raise SystemExit("JSON records must be objects")

    # Build unified column set preserving encounter order
    all_cols = union_keys_preserve_order(rows)

    # Create workbook and stage on 'Data'
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write headers
    for col_idx, key in enumerate(all_cols, start=1):
        cell = ws.cell(row=1, column=col_idx, value=key)
        cell.font = Font(bold=True, color="FFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = PatternFill("solid", fgColor="4F81BD")

    # Write data rows preserving values exactly
    for row_idx, rec in enumerate(rows, start=2):
        for col_idx, key in enumerate(all_cols, start=1):
            val = rec.get(key, "")
            ws.cell(row=row_idx, column=col_idx, value=val)

    # Freeze top row
    ws.freeze_panes = "A2"

    # Initial column width sizing (approx.)
    for col_idx, key in enumerate(all_cols, start=1):
        maxw = estimate_col_width(key)
        for row_idx in range(2, len(rows) + 2):
            v = ws.cell(row=row_idx, column=col_idx).value
            maxw = max(maxw, estimate_col_width(v))
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = maxw

    # Create META sheet and copy META columns AS-IS
    meta = wb.create_sheet("Meta_data_sheet")
    # headers
    for c_idx, k in enumerate(META_COLUMNS, start=1):
        meta.cell(row=1, column=c_idx, value=k)
    # rows
    for r_idx, rec in enumerate(rows, start=2):
        for c_idx, k in enumerate(META_COLUMNS, start=1):
            meta.cell(row=r_idx, column=c_idx, value=rec.get(k, ""))
    very_hidden(meta)

    # Normalize main sheet: rename Data -> TestPlan and reorganize columns
    ws.title = "TestPlan"

    # Build reordered data for MAIN_ORDER and drop META columns from visible sheet
    final_cols = MAIN_ORDER

    # Clear and rewrite current sheet with final_cols
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column)

    # Write headers again for TestPlan
    for col_idx, key in enumerate(final_cols, start=1):
        cell = ws.cell(row=1, column=col_idx, value=key)
        cell.font = Font(bold=True, color="FFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = PatternFill("solid", fgColor="4F81BD")

    # Write data
    for row_idx, rec in enumerate(rows, start=2):
        for col_idx, key in enumerate(final_cols, start=1):
            ws.cell(row=row_idx, column=col_idx, value=rec.get(key, ""))

    # Formatting: wrap text for specific columns
    wrap_cols = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }

    # Apply numbering inside cells for Steps and Acceptance columns
    numbered_cols = {
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }

    header_alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
    data_text_align = Alignment(horizontal="left", vertical="top", wrap_text=True)
    data_num_align = Alignment(horizontal="center", vertical="top", wrap_text=True)

    # Re-style header row (already styled) and size columns again
    for col_idx, key in enumerate(final_cols, start=1):
        # Column width estimation
        values_for_width = [key]
        for r in rows:
            values_for_width.append(r.get(key, ""))
        maxw = max(estimate_col_width(v) for v in values_for_width)
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = maxw

    # Data rows formatting and numbering
    for r_idx in range(2, len(rows) + 2):
        for c_idx, key in enumerate(final_cols, start=1):
            cell = ws.cell(row=r_idx, column=c_idx)
            # Numbering if applicable
            if key in numbered_cols:
                cell.value = number_multiline(cell.value or "")
            # Wrapping where applicable
            if key in wrap_cols:
                cell.alignment = data_text_align
            else:
                # Index numeric centered; others left
                if key == "Index":
                    cell.alignment = data_num_align
                else:
                    cell.alignment = Alignment(horizontal="left", vertical="top")

        # Approximate row height based on wrapped content lines
        text_cells = [ws.cell(row=r_idx, column=final_cols.index(k)+1).value or "" for k in wrap_cols if k in final_cols]
        line_count = 1
        for t in text_cells:
            lc = max(1, len(str(t).splitlines()))
            line_count = max(line_count, lc)
        ws.row_dimensions[r_idx].height = min(15 * line_count, 600)  # approx 15pt per line, capped

    # Borders for all populated cells
    apply_borders(ws, 1, ws.max_row, 1, ws.max_column)

    # Data validation for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in final_cols:
        cg_col_idx = final_cols.index("Code Generation (Required / Not)") + 1
        col_letter = ws.cell(row=1, column=cg_col_idx).column_letter
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True)
        dv.error = "Select a value from the list"
        dv.errorTitle = "Invalid value"
        ws.add_data_validation(dv)
        dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")

    # Safety check: only TestPlan (visible) and Meta_data_sheet (veryHidden)
    names = [s.title for s in wb.worksheets]
    if "Data" in names:
        # Remove any stray 'Data' sheet
        for s in wb.worksheets:
            if s.title == "Data":
                wb.remove(s)
                break

    # Final check
    final_names = [s.title for s in wb.worksheets]
    assert set(final_names) == {"TestPlan", "Meta_data_sheet"}, f"Unexpected sheets: {final_names}"

    # Ensure output dir exists
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    out_path = os.path.join(OUTPUT_DIR, OUTPUT_FILE)
    wb.save(out_path)

    # Validate xlsx as proper OOXML zip
    if not validate_xlsx_zip(out_path):
        raise SystemExit("Generated XLSX failed OOXML ZIP validation")

    print(f"OK: generated {out_path}")


if __name__ == "__main__":
    main()
