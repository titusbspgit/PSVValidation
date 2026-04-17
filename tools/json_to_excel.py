#!/usr/bin/env python3
# Deterministic JSON -> Excel generator for PSVValidation
# - Preserves key order based on first appearance
# - Creates one sheet named "Data"
# - Bold header, freeze top row, auto-fit columns

import json
import os
import sys
from collections import OrderedDict
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

# Embedded JSON input (exactly as provided)
JSON_INPUT = r'''{ "TestCases": [ { "Index": "1", "SS / Module": "GPIO", "Feature": "interrupts can be generated based on positive edge or negative edge or level high or level low detection at GPIO input", "Test Case Name": "test_gpio_negedge_intr_en", "Test Description": "Configures the per-pin control registers for a 32-pin group as inputs with negative-edge interrupt enabled, enables the top-level interrupt mask, then for each pin generates a falling edge via the pad control at 0xA0243ffc, waits for the interrupt handler, and validates that the pin input reads low, the raw interrupt is latched in the group status, and that status clears correctly when the per-pin raw and group raw clear registers are written.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "Top-level interrupt enable is performed via INTR_EN1 prior to pin-level enable. The wait is armed before generating the falling edge to avoid race conditions. A bounded wait uses a timeout counter of 5000 iterations with delays. The group raw clear is assumed to be write-one-to-clear. The same global loop index is used by the interrupt handler to determine which pin to service. The pad output is driven high after handling to restore a known state.", "Test Steps / Procedure": "1) Initialize error counter to zero. 2) Conditionally enable the platform interrupt controller line for the selected GPIO instance. 3) Enable the top-level GPIO interrupt in INTR_EN1. 4) Drive the pad output control at address 0xA0243ffc to all ones to establish a high level. 5) For each of the 32 pins in the group: a) Compute the per-pin control register address starting from GPIO_8 with an index-based offset. b) Write the control to set input mode, enable negative-edge interrupt, and clear any existing raw status. c) Delay for a short period. 6) For each of the 32 pins: a) Compute a one-hot mask for the current pin. b) Clear the group raw status for this pin via GPIO_GPIO_INTR_RAW_STCLR1. c) Enable the pin-level interrupt for this pin via GPIO_GP0_INTR1_INTR_EN1. d) Delay briefly. e) Arm the interrupt wait flag. f) Write all ones to 0xA0243ffc, delay, then write a value with the current pin driven low to create a falling edge. g) Enter a bounded wait loop with a timeout budget; repeatedly delay while waiting for the handler to clear the pending flag. h) On timeout, record an error. 7) Upon interrupt: a) Clear the pending flag. b) Drive 0xA0243ffc to all ones to restore pad state. c) Read the current per-pin control register and verify the input bit is low. d) If the raw interrupt indicator is set, read GPIO_GP0_INTR1_INTR_STS1 and verify the group bit for the current pin is set. e) Clear per-pin raw status through the per-pin control register and clear the group raw status via GPIO_GPIO_INTR_RAW_STCLR1. f) Read GPIO_GP0_INTR1_INTR_STS1 and verify it is zero. g) Clear the top-level raw status via RAW_STCR1 and clear the platform interrupt controller for the selected line. h) If the raw indicator was not set, record an error. 8) End the test and report the total error count.", "Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1", "Validation / Acceptance Criteria": "- Each generated falling edge must result in an interrupt before the timeout expires. - After each interrupt, a read of the corresponding per-pin control must show the input bit is low. - The group interrupt status must show the bit for the pin set after the edge and must read back zero after clearing per-pin and group raw status. - The top-level raw status and the platform interrupt line must be cleared as part of the handling. - The test passes if no timeouts or validation errors occur for any of the 32 pins.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en", "Hidden_Test_Description": "This test configures GPIO[8..39] as input with negedge interrupt enabled, enables GIC (IRQ 87 for GPIO0 or 88 for GPIO1) and LSS SYSREG interrupt routing, then for each bit i (0..31) generates a falling edge using writes to 0xA0243ffc and waits for Default_IRQHandler to execute. The handler verifies DIN=0, raw status set in MIZAR_GPIO_GP0_INTR1_INTR_STS1, clears per-pin raw via MIZAR_GPIO_GP0_GPIO_8+(i4) (iclr=1) and group raw via MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, re-checks group status=0, and clears LSS raw/GIC.", "Hidden_Remarks": "- Uses write_reg(0xA0243ffc, ...) to force pad values (all ones then bit cleared). - Bounded wait uses 'unsigned int timeout = 5000' and wait_on(10) in the loop. - The wait flag 'int_pend' is set before edge generation to avoid race. - Assumes MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 is write-one-to-clear (W1C). - Default_IRQHandler depends on global 'i' to compute the current pin register address and bit mask. - Conditional compilation via GPIO0/GPIO1 selects GIC IRQ number and SYSREG bits.", "Hidden_Test_Steps_Procedure": "Entry: int test_case()\n1. test_err = 0;\n2. Ifdef GPIO0: GIC_EnableIRQ(87);\n3. Ifdef GPIO1: GIC_EnableIRQ(88);\n4. Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);\n5. Ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);\n6. write_reg(0xA0243ffc, 0xffffffff);\n7. For (i = 0; i < 32; i++):\n 7.1 addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);\n 7.2 write_reg(addr1, (1u << 20) | (1u << 18) | (1u << 16)); // doe=1, neie=1, iclr=1\n 7.3 wait_on(10);\n8. For (i = 0; i < 32; i++):\n 8.1 wr_val = 1u << i;\n 8.2 write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);\n 8.3 write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);\n 8.4 wait_on(10);\n 8.5 int_pend = 1;\n 8.6 write_reg(0xA0243ffc, 0xffffffff);\n 8.7 wait_on(30);\n 8.8 write_reg(0xA0243ffc, ~wr_val);\n 8.9 unsigned int timeout = 5000;\n 8.10 while (int_pend && timeout--) { wait_on(10); }\n 8.11 if (timeout == 0) { printf(\"ERROR: Timeout waiting for GPIO%u negedge interrupt\\n\", (unsigned)(i + 8)); test_err++; }\n9. finish(test_err);\n\nInterrupt handler: void Default_IRQHandler()\nA. unsigned int local_wr = 1u << i; int_pend = 0;\nB. write_reg(0xA0243ffc, 0xffffffff);\nC. raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);\nD. rdata = read_reg(raddr);\nE. if ((rdata & 0x1) != 0) { test_err++; }\nF. if ((rdata & 0x2) != 0x0) {\n F.1 rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);\n F.2 if ((rdata_grp & local_wr) == 0) { test_err++; }\n F.3 raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);\n F.4 write_reg(raddr2, (1u << 20) | (1u << 16)); // doe=1, iclr=1\n F.5 write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);\n F.6 rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);\n F.7 if (rdata_grp != 0x0) { test_err++; }\n F.8 Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87);\n F.9 Ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88);\n} else {\n G. test_err++;\n}", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1", "Hidden_Validation_Acceptance_Criteria": "- No timeout in the wait loop for any i in 0..31 (int_pend must be cleared by Default_IRQHandler before timeout reaches zero). - In Default_IRQHandler, (rdata & 0x1) must be 0 (DIN low) after the falling edge. - (rdata & 0x2) must be non-zero (raw interrupt set). - read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) must have the bit (1u<<i) set initially, and must read back 0x0 after write_reg(MIZAR_GPIO_GP0_GPIO_8+(i4), (1u<<20)|(1u<<16)) and write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u<<i)). - LSS SYSREG raw clear and GIC_ClearIRQ must be invoked for the enabled instance. - Test passes if test_err remains 0 on finish(test_err)." } ] }'''

# Output path inside the repo
OUTPUT_XLSX = os.path.join('TestRepo', 'gpio', 'test_gpio_negedge_intr_en.xlsx')
SHEET_NAME = 'Data'


def parse_input(json_input: str):
    obj = json.loads(json_input)
    if isinstance(obj, dict) and 'TestCases' in obj and isinstance(obj['TestCases'], list):
        rows = obj['TestCases']
    elif isinstance(obj, list):
        rows = obj
    elif isinstance(obj, dict):
        rows = [obj]
    else:
        raise ValueError('Unsupported JSON structure')
    if len(rows) == 0:
        raise ValueError('Empty JSON array provided')
    return rows


def compute_headers(rows):
    seen = set()
    headers = []
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError('Each row must be a JSON object')
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                headers.append(k)
    return headers


def auto_fit_columns(ws):
    widths = {}
    for r in ws.iter_rows(values_only=True):
        for idx, value in enumerate(r, start=1):
            text = '' if value is None else str(value)
            # consider multi-line cells: take the longest line
            max_seg = max((len(seg) for seg in text.splitlines()), default=0)
            widths[idx] = max(widths.get(idx, 0), max_seg)
    for idx, w in widths.items():
        # Add padding
        adj = min(max(w + 2, 10), 120)
        ws.column_dimensions[get_column_letter(idx)].width = adj


def write_excel(rows, headers, out_path):
    wb = Workbook()
    ws = wb.active
    ws.title = SHEET_NAME

    # Header row
    bold_font = Font(bold=True)
    for c, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = bold_font

    # Data rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, h in enumerate(headers, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(h, ''))

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Auto-fit
    auto_fit_columns(ws)

    # Ensure directory exists
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)


def main():
    try:
        rows = parse_input(JSON_INPUT)
        headers = compute_headers(rows)
        write_excel(rows, headers, OUTPUT_XLSX)
        print(f'Wrote {len(rows)} rows, {len(headers)} columns to {OUTPUT_XLSX}')
    except Exception as e:
        print(f'ERROR: {e}', file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
