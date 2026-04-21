#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, Border, Side
from openpyxl.utils import get_column_letter

# JSON provided directly (as-is)
JSON_DATA = r'''{
  "ip_name": "GPIO",
  "source_repo": "titusbspgit/PSVValidation",
  "source_branch": "main",
  "source_subdir": "TestRepo/gpio",
  "generated_on_ist": "AUTO_COMPUTE_CURRENT_IST_TIMESTAMP",
  "tests": [
    {
      "Index": 1,
      "SS / Module": "GPIO",
      "Feature": "AHB 32-bit register interface.",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "Validates default register values and masked write/read behavior across GPIO per-pin and group registers using defined read/write masks and expected defaults.",
      "Speed": "NA",
      "Mode": "NA",
      "Memory Start Offset": "NA",
      "Memory End Offset": "NA",
      "Remarks": "During default-value reads, input data can float high unless forced; this affects DIN-related expectations as noted in source comments.",
      "Test Steps / Procedure": [
        "Entry: test_case() invokes reset-value checks followed by write/read checks, then reports pass/fail.",
        "Default-value phase: Iterate through the address list (GPIO_GP0_GPIO_8 through GPIO_GP0_GPIO_39 and group registers including GPIO_GPIO_INTR_RAW_STCLR1, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, GPIO_GP0_INTR2_INTR_EN1, GPIO_GP0_INTR2_INTR_STS1, GPIO_GPIO_IO_CTRL_GROUP1..4, GPIO_GPIO_DOUT_GROUP1..4, GPIO_GPIO_DIN_GROUP1..4). For each address: if the address is marked to skip reset checking, continue; if the read mask is zero, continue; otherwise read the register, mask off bit0 (LSB) before comparison, and compare against the provided default value.",
        "Write phase (6 patterns: FFFFFFFFh, AAAAAAAAh, 55555555h, F5F5F5F5h, A5A5A5A5h, FFFF0000h): For each pattern, iterate all addresses: if the address is marked to skip writes, continue; if the write mask is zero, continue; otherwise write (pattern AND per-register write mask) to the register.",
        "Read/verify phase (per pattern): Iterate all addresses used in the write phase: if skipped earlier, continue; if the write mask is zero or the read mask is zero, continue; otherwise read the register and mask the result with the per-register read mask; compute the expected value = (pattern AND read mask AND write mask) OR ((NOT write mask) AND read mask AND default value); compare read vs expected.",
        "Completion: If any default mismatches or any write/read mismatches occurred, mark failure; otherwise, mark pass."
      ],
      "Impacted Registers": "GPIO_GP0_GPIO_8, GPIO_GP0_GPIO_9, GPIO_GP0_GPIO_10, GPIO_GP0_GPIO_11, GPIO_GP0_GPIO_12, GPIO_GP0_GPIO_13, GPIO_GP0_GPIO_14, GPIO_GP0_GPIO_15, GPIO_GP0_GPIO_16, GPIO_GP0_GPIO_17, GPIO_GP0_GPIO_18, GPIO_GP0_GPIO_19, GPIO_GP0_GPIO_20, GPIO_GP0_GPIO_21, GPIO_GP0_GPIO_22, GPIO_GP0_GPIO_23, GPIO_GP0_GPIO_24, GPIO_GP0_GPIO_25, GPIO_GP0_GPIO_26, GPIO_GP0_GPIO_27, GPIO_GP0_GPIO_28, GPIO_GP0_GPIO_29, GPIO_GP0_GPIO_30, GPIO_GP0_GPIO_31, GPIO_GP0_GPIO_32, GPIO_GP0_GPIO_33, GPIO_GP0_GPIO_34, GPIO_GP0_GPIO_35, GPIO_GP0_GPIO_36, GPIO_GP0_GPIO_37, GPIO_GP0_GPIO_38, GPIO_GP0_GPIO_39, GPIO_GPIO_INTR_RAW_STCLR1, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, GPIO_GP0_INTR2_INTR_EN1, GPIO_GP0_INTR2_INTR_STS1, GPIO_GPIO_IO_CTRL_GROUP1, GPIO_GPIO_IO_CTRL_GROUP2, GPIO_GPIO_IO_CTRL_GROUP3, GPIO_GPIO_IO_CTRL_GROUP4, GPIO_GPIO_DOUT_GROUP1, GPIO_GPIO_DOUT_GROUP2, GPIO_GPIO_DOUT_GROUP3, GPIO_GPIO_DOUT_GROUP4, GPIO_GPIO_DIN_GROUP1, GPIO_GPIO_DIN_GROUP2, GPIO_GPIO_DIN_GROUP3, GPIO_GPIO_DIN_GROUP4",
      "Validation / Acceptance Criteria": [
        "Default-value phase: For each register where read_mask != 0 and not skipped for reset, (read_value & 0xFFFFFFFE) must equal expected_default for that address.",
        "Write/read phase: For each address where write/read are applicable, (read_value & read_mask) must equal ((pattern & read_mask & write_mask) | ((~write_mask) & read_mask & default_value)).",
        "Overall result: PASS if no default mismatches and no write/read mismatches; otherwise FAIL."
      ],
      "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
      "Hidden_Test_Description": "Check default values for each address (masking LSB) and verify masked write/read across six data patterns over all listed GPIO and group registers.",
      "Hidden_Remarks": "when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value",
      "Hidden_Test_Steps_Procedure": [
        "Entry: test_case() -> chk_rst_val(); chk_rd_wr(); finish().",
        "chk_rst_val(): for (i=0..CNT-1) { addr = addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0x00000000) continue; data_rd = read_reg(addr); data = (data_rd & 0xfffffffe); if (data == default_value_array[i]) PASS else { def_fail_cnt++; printf failure; } }",
        "chk_rd_wr(): patterns chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; for each j in 0..5 set data_wr=chk_val[j]; WRITE loop: for (i=0..CNT-1) { addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; write_reg(addr,(data_wr & write_mask_array[i])); }",
        "READ/VERIFY loop: for (i=0..CNT-1) { addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; if (read_mask_array[i]==0x00000000) continue; data_rd = (read_reg(addr) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) PASS else { wr_fail_cnt++; printf failure; } }",
        "Completion: if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0)."
      ],
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, ...",
      "Hidden_Validation_Acceptance_Criteria": [
        "chk_rst_val and chk_rd_wr checks as per algorithm"
      ],
      "test_id": "TestRepo/gpio/gpio_reg_wr_rd_test",
      "category": "register",
      "test_type": "validation",
      "priority": "P1",
      "preconditions/setup": "No explicit pinmux or clock setup in source. Operates via AHB register interface and helper APIs.",
      "steps": [
        "Run default-value verification over all listed registers.",
        "Write six patterns to writable registers.",
        "Read back and compare with expected values."
      ],
      "expected_results / pass_criteria": "Zero failures in default-value and masked write/read comparisons.",
      "key_registers_or_related_registers": [
        "GPIO_GP0_GPIO_8..GPIO_GP0_GPIO_39",
        "GPIO_GPIO_INTR_RAW_STCLR1",
        "GPIO_GP0_INTR1_INTR_EN1",
        "GPIO_GP0_INTR1_INTR_STS1",
        "GPIO_GP0_INTR2_INTR_EN1",
        "GPIO_GP0_INTR2_INTR_STS1",
        "GPIO_GPIO_IO_CTRL_GROUP1..GPIO_GPIO_IO_CTRL_GROUP4",
        "GPIO_GPIO_DOUT_GROUP1..GPIO_GPIO_DOUT_GROUP4",
        "GPIO_GPIO_DIN_GROUP1..GPIO_GPIO_DIN_GROUP4"
      ],
      "stimuli": "Register read/write sequences with six data patterns.",
      "dependencies": [
        "TestRepo/gpio/gpio_reg_wr_rd_test/program.c",
        "TestRepo/gpio/gpio_reg_wr_rd_test/test_define.c",
        "test_common.h (external include)"
      ],
      "artifacts/file_paths": [
        {"repo_relative": "TestRepo/gpio/gpio_reg_wr_rd_test/program.c", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/gpio_reg_wr_rd_test/program.c"},
        {"repo_relative": "TestRepo/gpio/gpio_reg_wr_rd_test/test_define.c", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/gpio_reg_wr_rd_test/test_define.c"},
        {"repo_relative": "TestRepo/gpio/gpio_reg_wr_rd_test/Makefile", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/gpio_reg_wr_rd_test/Makefile"}
      ],
      "negative_tests": "NA",
      "coverage_tags": ["regs_access", "masked_write", "default_values"],
      "assumptions/notes": "LSB masking applied before default comparison."
    },
    {
      "Index": 2,
      "SS / Module": "GPIO",
      "Feature": "neie: Negative edge interrupt enable;",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "Configures GPIO pins 8–39 for input and negative-edge interrupts, enables and tests each pin sequentially by generating a falling edge, and validates per-pin and group interrupt status/clearing via ISR.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "Interrupt wait uses a bounded timeout (5000 iterations).",
      "Test Steps / Procedure": [
        "Enable GIC IRQ and system register interrupt for selected GPIO instance.",
        "Configure pins 8..39 for input, neie=1, and clear raw.",
        "For each pin: pre-clear group raw, enable only that pin mask, create falling edge via 0xA0243ffc writes, and wait for ISR with timeout.",
        "ISR: verify DIN low, per-pin raw and group status set; clear per-pin raw and group raw; clear system raw and GIC."
      ],
      "Impacted Registers": "GPIO_GP0_GPIO_8..GPIO_GP0_GPIO_39, GPIO_GPIO_INTR_RAW_STCLR1, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, LSS_SYSREG_INTR_EN1, LSS_SYSREG_RAW_STCR1",
      "Validation / Acceptance Criteria": [
        "No timeout; DIN=0 observed; group status reflects tested pin; clears verified."
      ],
      "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
      "Hidden_Test_Description": "Enable neie and input mode; generate falling edges; validate ISR path and clearing.",
      "test_id": "TestRepo/gpio/test_gpio_negedge_intr_en",
      "category": "interrupt",
      "test_type": "validation",
      "priority": "P1",
      "preconditions/setup": "GIC and system register interrupts enabled; pad driver 0xA0243ffc available.",
      "steps": [
        "Configure pins and enable neie.",
        "Sequentially enable each pin and generate negedge.",
        "Validate ISR effects and clear statuses."
      ],
      "expected_results / pass_criteria": "Interrupt on each negedge without timeout; statuses set then cleared; finish(0).",
      "key_registers_or_related_registers": ["GPIO_GP0_GPIO_8..GPIO_GP0_GPIO_39", "GPIO_GP0_INTR1_INTR_EN1", "GPIO_GP0_INTR1_INTR_STS1", "GPIO_GPIO_INTR_RAW_STCLR1", "LSS_SYSREG_INTR_EN1", "LSS_SYSREG_RAW_STCR1"],
      "stimuli": "Programmatic falling edges via 0xA0243ffc.",
      "dependencies": [
        "TestRepo/gpio/test_gpio_negedge_intr_en/program.c",
        "TestRepo/gpio/test_gpio_negedge_intr_en/test_define.c"
      ],
      "artifacts/file_paths": [
        {"repo_relative": "TestRepo/gpio/test_gpio_negedge_intr_en/program.c", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/test_gpio_negedge_intr_en/program.c"},
        {"repo_relative": "TestRepo/gpio/test_gpio_negedge_intr_en/test_define.c", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/test_gpio_negedge_intr_en/test_define.c"},
        {"repo_relative": "TestRepo/gpio/test_gpio_negedge_intr_en/Makefile", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/test_gpio_negedge_intr_en/Makefile"}
      ],
      "negative_tests": "NA",
      "coverage_tags": ["intr_negedge", "per_pin", "group_status", "raw_clear"],
      "assumptions/notes": "Bounded waits prevent infinite loops."
    },
    {
      "Index": 3,
      "SS / Module": "GPIO",
      "Feature": "peie: Positive edge interrupt enable;",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "Enables positive-edge interrupts on GPIO pins 8–39, sets input mode via group IO control, drives a rising edge per pin, and verifies group interrupt behavior and clearing in the ISR.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "ISR masks group enable during service and clears all raw statuses.",
      "Test Steps / Procedure": [
        "Enable GIC IRQ and system register interrupt.",
        "Enable peie on pins 8–39 and configure input mode via group IO control.",
        "Enable all masked interrupts; generate rising edges; wait for ISR with timeout.",
        "ISR: check group status, clear all raw statuses, clear system raw, and re-enable group."
      ],
      "Impacted Registers": "GPIO_GP0_GPIO_8..GPIO_GP0_GPIO_39, GPIO_GPIO_IO_CTRL_GROUP1..4, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, LSS_SYSREG_INTR_EN1, LSS_SYSREG_RAW_STCR1",
      "Validation / Acceptance Criteria": ["ISR observed without timeout; group status set then cleared; system raw cleared; overall PASS."],
      "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
      "Hidden_Test_Description": "Enable peie, configure input mode, generate rising edges, and validate ISR/group behavior.",
      "test_id": "TestRepo/gpio/test_gpio_pedge_all_pads_en",
      "category": "interrupt",
      "test_type": "validation",
      "priority": "P1",
      "preconditions/setup": "GIC and system register interrupts enabled; pad control via 0xA0243ffc available.",
      "steps": [
        "Configure peie and inputs.",
        "Drive rising edges and wait for ISR.",
        "Clear statuses and continue."
      ],
      "expected_results / pass_criteria": "Each edge triggers ISR; statuses clear to zero; finish(0).",
      "key_registers_or_related_registers": ["GPIO_GP0_GPIO_8..GPIO_GP0_GPIO_39", "GPIO_GPIO_IO_CTRL_GROUP1..4", "GPIO_GP0_INTR1_INTR_EN1", "GPIO_GP0_INTR1_INTR_STS1", "LSS_SYSREG_INTR_EN1", "LSS_SYSREG_RAW_STCR1"],
      "stimuli": "Programmatic rising edges via 0xA0243ffc.",
      "dependencies": [
        "TestRepo/gpio/test_gpio_pedge_all_pads_en/program.c",
        "TestRepo/gpio/test_gpio_pedge_all_pads_en/test_define.c"
      ],
      "artifacts/file_paths": [
        {"repo_relative": "TestRepo/gpio/test_gpio_pedge_all_pads_en/program.c", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/test_gpio_pedge_all_pads_en/program.c"},
        {"repo_relative": "TestRepo/gpio/test_gpio_pedge_all_pads_en/test_define.c", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/test_gpio_pedge_all_pads_en/test_define.c"},
        {"repo_relative": "TestRepo/gpio/test_gpio_pedge_all_pads_en/Makefile", "github_link": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/test_gpio_pedge_all_pads_en/Makefile"}
      ],
      "negative_tests": "NA",
      "coverage_tags": ["intr_posedge", "all_pads", "group_status", "raw_clear"],
      "assumptions/notes": "ISR masks EN1 during service to avoid re-entry."
    }
  ]
}'''

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


def to_display(val):
    if isinstance(val, list):
        return "\n".join(str(x) for x in val)
    if isinstance(val, dict):
        return json.dumps(val, ensure_ascii=False)
    return val if val is not None else ""


data = json.loads(JSON_DATA)
records = data.get("tests", [])

# Prepare workbook and sheet (exactly one sheet: TestPlan)
wb = Workbook()
ws = wb.active
ws.title = "TestPlan"

# Header row
ws.append(MAIN_COLUMNS)

# Data rows
for rec in records:
    row = [to_display(rec.get(col, "")) for col in MAIN_COLUMNS]
    ws.append(row)

# Formatting (Stage1 strict for TestPlan)
header_font = Font(bold=True)
center_center = Alignment(horizontal="center", vertical="center", wrap_text=False)
left_top = Alignment(horizontal="left", vertical="top", wrap_text=False)
center_top = Alignment(horizontal="center", vertical="top", wrap_text=False)
wrap_left_top = Alignment(horizontal="left", vertical="top", wrap_text=True)

# Apply header formatting
for cell in ws[1]:
    cell.font = header_font
    cell.alignment = center_center

# Wrap text for specific columns
WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

col_index_by_name = {name: idx+1 for idx, name in enumerate(MAIN_COLUMNS)}
wrap_indexes = {col_index_by_name[name] for name in WRAP_COLS if name in col_index_by_name}

# Apply data alignment and wrapping
for r in range(2, ws.max_row + 1):
    for c in range(1, ws.max_column + 1):
        cell = ws.cell(row=r, column=c)
        if c in wrap_indexes:
            cell.alignment = wrap_left_top
        elif c == col_index_by_name.get("Index"):
            cell.alignment = center_top
        else:
            cell.alignment = left_top

# Borders for all populated cells
thin = Side(style="thin", color="000000")
border = Border(left=thin, right=thin, top=thin, bottom=thin)
for r in range(1, ws.max_row + 1):
    for c in range(1, ws.max_column + 1):
        ws.cell(row=r, column=c).border = border

# Freeze header row
ws.freeze_panes = "A2"

# Auto-filter on header
ws.auto_filter.ref = ws.dimensions

# Approximate auto-fit column widths using max line length per column
for c in range(1, ws.max_column + 1):
    max_len = 0
    for r in range(1, ws.max_row + 1):
        v = ws.cell(row=r, column=c).value
        if v is None:
            continue
        text = str(v)
        for line in text.split("\n"):
            if len(line) > max_len:
                max_len = len(line)
    width = min(max(10, max_len + 2), 120)  # clamp width for readability
    ws.column_dimensions[get_column_letter(c)].width = width

# Row heights: let Excel auto-adjust based on wrapping
for r in range(1, ws.max_row + 1):
    ws.row_dimensions[r].height = None

# Compute IST timestamp and output path
if ZoneInfo is not None:
    ist_now = datetime.now(ZoneInfo("Asia/Kolkata"))
else:
    from datetime import timedelta, timezone
    ist_now = datetime.now(timezone(timedelta(hours=5, minutes=30)))

filename = f"GPIO_TestPlan_{ist_now.strftime('%Y%m%d_%H%M%S')}.xlsx"
out_dir = os.path.join("Test_Output", "GPIO", "TestPlan")
os.makedirs(out_dir, exist_ok=True)
out_path = os.path.join(out_dir, filename)

# Save workbook (only one sheet exists)
wb.save(out_path)

print(f"Generated: {out_path}")
