#!/usr/bin/env python3
import os
import json
import zipfile
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# ------------------ Embedded JSON (Fallback_JSON) ------------------
FALLBACK_JSON = {
    "metadata": {
        "ip_name": "GPIO",
        "source_repo": "https://github.com/titusbspgit/PSVValidation",
        "subdirectory": "TestRepo/gpio",
        "generation_timestamp_ist": "<populate>"
    },
    "test_cases": [
        {
            "Index": 1,
            "SS / Module": "GPIO",
            "Feature": "AHB 32-bit register interface.",
            "Test Case Name": "gpio_reg_wr_rd_test",
            "Test Description": "Checks default values and masked write/read behavior of multiple GPIO-related registers, comparing against expected defaults and computed expected values.",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Remarks": "Default value comparison ignores bit 0. Some registers are intentionally skipped in write/read and reset checks as noted in the source comments.",
            "Test Steps / Procedure": "1) Iterate the listed GPIO registers and skip any that are marked to be ignored for reset checks.\n2) For readable registers, read the value and compare against the documented default while ignoring bit 0.\n3) For each of the six data patterns, write only the writable bits to each non-skipped register.\n4) For each register, read back only the readable bits and compute the expected value using the write and read masks with the documented default.\n5) Compare the read value against the expected value for each pattern and track mismatches.\n6) Report failure if any default-value or write/read comparisons fail; otherwise report pass.",
            "Impacted Registers": [
                "GPIO_8","GPIO_9","GPIO_10","GPIO_11","GPIO_12","GPIO_13","GPIO_14","GPIO_15","GPIO_16","GPIO_17","GPIO_18","GPIO_19","GPIO_20","GPIO_21","GPIO_22","GPIO_23","GPIO_24","GPIO_25","GPIO_26","GPIO_27","GPIO_28","GPIO_29","GPIO_30","GPIO_31","GPIO_32","GPIO_33","GPIO_34","GPIO_35","GPIO_36","GPIO_37","GPIO_38","GPIO_39","GPIO_INTR_RAW_STCLR1","INTR1_INTR_EN1","INTR1_INTR_STS1","INTR2_INTR_EN1","INTR2_INTR_STS1","GPIO_IO_CTRL_GROUP1","GPIO_IO_CTRL_GROUP2","GPIO_IO_CTRL_GROUP3","GPIO_IO_CTRL_GROUP4","GPIO_DOUT_GROUP1","GPIO_DOUT_GROUP2","GPIO_DOUT_GROUP3","GPIO_DOUT_GROUP4","GPIO_DIN_GROUP1","GPIO_DIN_GROUP2","GPIO_DIN_GROUP3","GPIO_DIN_GROUP4"
            ],
            "Validation / Acceptance Criteria": "1) Each readable register’s value matches its documented default when bit 0 is ignored.\n2) For each data pattern, every non-skipped register’s read value equals the expected value derived from the write mask, read mask, and default.\n3) The test passes if there are zero default mismatches and zero write/read mismatches; otherwise it fails.",
            "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
            "Hidden_Test_Description": "Verifies default values and masked R/W functionality for a set of GPIO IP registers using arrays of addresses, default values, read/write masks, and skip controls.",
            "Hidden_Remarks": "SKIPPING certain VRRW or special-behavior registers per skip arrays in source.",
            "Hidden_Impacted_Registers": [
                "MIZAR_GPIO_GP0_GPIO_8","MIZAR_GPIO_GP0_GPIO_9","MIZAR_GPIO_GP0_GPIO_10","MIZAR_GPIO_GP0_GPIO_11","MIZAR_GPIO_GP0_GPIO_12","MIZAR_GPIO_GP0_GPIO_13","MIZAR_GPIO_GP0_GPIO_14","MIZAR_GPIO_GP0_GPIO_15","MIZAR_GPIO_GP0_GPIO_16","MIZAR_GPIO_GP0_GPIO_17","MIZAR_GPIO_GP0_GPIO_18","MIZAR_GPIO_GP0_GPIO_19","MIZAR_GPIO_GP0_GPIO_20","MIZAR_GPIO_GP0_GPIO_21","MIZAR_GPIO_GP0_GPIO_22","MIZAR_GPIO_GP0_GPIO_23","MIZAR_GPIO_GP0_GPIO_24","MIZAR_GPIO_GP0_GPIO_25","MIZAR_GPIO_GP0_GPIO_26","MIZAR_GPIO_GP0_GPIO_27","MIZAR_GPIO_GP0_GPIO_28","MIZAR_GPIO_GP0_GPIO_29","MIZAR_GPIO_GP0_GPIO_30","MIZAR_GPIO_GP0_GPIO_31","MIZAR_GPIO_GP0_GPIO_32","MIZAR_GPIO_GP0_GPIO_33","MIZAR_GPIO_GP0_GPIO_34","MIZAR_GPIO_GP0_GPIO_35","MIZAR_GPIO_GP0_GPIO_36","MIZAR_GPIO_GP0_GPIO_37","MIZAR_GPIO_GP0_GPIO_38","MIZAR_GPIO_GP0_GPIO_39","MIZAR_GPIO_GPIO_INTR_RAW_STCLR1","MIZAR_GPIO_GP0_INTR1_INTR_EN1","MIZAR_GPIO_GP0_INTR1_INTR_STS1","MIZAR_GPIO_GP0_INTR2_INTR_EN1","MIZAR_GPIO_GP0_INTR2_INTR_STS1","MIZAR_GPIO_GPIO_IO_CTRL_GROUP1","MIZAR_GPIO_GPIO_IO_CTRL_GROUP2","MIZAR_GPIO_GPIO_IO_CTRL_GROUP3","MIZAR_GPIO_GPIO_IO_CTRL_GROUP4","MIZAR_GPIO_GPIO_DOUT_GROUP1","MIZAR_GPIO_GPIO_DOUT_GROUP2","MIZAR_GPIO_GPIO_DOUT_GROUP3","MIZAR_GPIO_GPIO_DOUT_GROUP4","MIZAR_GPIO_GPIO_DIN_GROUP1","MIZAR_GPIO_GPIO_DIN_GROUP2","MIZAR_GPIO_GPIO_DIN_GROUP3","MIZAR_GPIO_GPIO_DIN_GROUP4"
            ]
        },
        {
            "Index": 2,
            "SS / Module": "GPIO",
            "Feature": "interrupts can be generated based on positive edge or negative edge or level high or level low detection at GPIO input.",
            "Test Case Name": "test_gpio_negedge_intr_en",
            "Test Description": "Configures GPIOs for negative-edge interrupts, generates falling edges per pin, and validates per-pin and group interrupt status and clearing.",
            "Speed": "NA",
            "Mode": "Interrupt",
            "Memory Start Offset": "0xA0243ffc",
            "Memory End Offset": "0xA0243ffc",
            "Remarks": "Execution depends on GPIO0 or GPIO1 being defined to select the interrupt line. Uses GIC IRQ 87/88 and a pad driver register at 0xA0243ffc to generate edges. Bounded timeouts avoid infinite waits.",
            "Test Steps / Procedure": "1) Enable the appropriate system interrupt output for the selected GPIO instance via INTR_EN1.\n2) Drive all pads high using the external pad driver register to establish a known state.\n3) For each of GPIO_8 through GPIO_39, program the per-pin control to enable input, enable negative-edge detection, and clear any pending raw status.\n4) For each bit position, clear the group raw status, enable that bit in INTR1_INTR_EN1, arm the wait, and generate a single falling edge using the pad driver.\n5) Wait with a timeout for the interrupt to be handled; on timeout, record an error and proceed.\n6) In the interrupt handler, verify the per-pin DIN is low, confirm the group status bit is set in INTR1_INTR_STS1, clear the per-pin raw and the group raw status, and confirm the group status becomes zero.\n7) Clear the system raw status via RAW_STCR1 and clear the GIC pending interrupt.\n8) Report pass if no errors were recorded, otherwise report fail.",
            "Impacted Registers": [
                "INTR_EN1","GPIO_8","GPIO_9","GPIO_10","GPIO_11","GPIO_12","GPIO_13","GPIO_14","GPIO_15","GPIO_16","GPIO_17","GPIO_18","GPIO_19","GPIO_20","GPIO_21","GPIO_22","GPIO_23","GPIO_24","GPIO_25","GPIO_26","GPIO_27","GPIO_28","GPIO_29","GPIO_30","GPIO_31","GPIO_32","GPIO_33","GPIO_34","GPIO_35","GPIO_36","GPIO_37","GPIO_38","GPIO_39","GPIO_INTR_RAW_STCLR1","INTR1_INTR_EN1","INTR1_INTR_STS1","RAW_STCR1"
            ],
            "Validation / Acceptance Criteria": "1) For each pin, the interrupt is observed before the timeout expires.\n2) In the handler, the per-pin input is low after the falling edge; the group status includes the pin’s bit; both the per-pin raw and group raw status clear successfully.\n3) The system raw interrupt status clears; the test passes only if no errors are recorded.",
            "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
            "Hidden_Test_Description": "Negative-edge interrupt enable test per pin: configure GPIOs as inputs with negedge detection, generate falling edges via external pad driver, and validate status/clear and system routing.",
            "Hidden_Remarks": "Conditional selection of GPIO0 or GPIO1 and use of GIC IRQ 87/88.",
            "Hidden_Impacted_Registers": [
                "MIZAR_LSS_SYSREG_INTR_EN1","MIZAR_GPIO_GP0_GPIO_8","MIZAR_GPIO_GP0_GPIO_9","MIZAR_GPIO_GP0_GPIO_10","MIZAR_GPIO_GP0_GPIO_11","MIZAR_GPIO_GP0_GPIO_12","MIZAR_GPIO_GP0_GPIO_13","MIZAR_GPIO_GP0_GPIO_14","MIZAR_GPIO_GP0_GPIO_15","MIZAR_GPIO_GP0_GPIO_16","MIZAR_GPIO_GP0_GPIO_17","MIZAR_GPIO_GP0_GPIO_18","MIZAR_GPIO_GP0_GPIO_19","MIZAR_GPIO_GP0_GPIO_20","MIZAR_GPIO_GP0_GPIO_21","MIZAR_GPIO_GP0_GPIO_22","MIZAR_GPIO_GP0_GPIO_23","MIZAR_GPIO_GP0_GPIO_24","MIZAR_GPIO_GP0_GPIO_25","MIZAR_GPIO_GP0_GPIO_26","MIZAR_GPIO_GP0_GPIO_27","MIZAR_GPIO_GP0_GPIO_28","MIZAR_GPIO_GP0_GPIO_29","MIZAR_GPIO_GP0_GPIO_30","MIZAR_GPIO_GP0_GPIO_31","MIZAR_GPIO_GP0_GPIO_32","MIZAR_GPIO_GP0_GPIO_33","MIZAR_GPIO_GP0_GPIO_34","MIZAR_GPIO_GP0_GPIO_35","MIZAR_GPIO_GP0_GPIO_36","MIZAR_GPIO_GP0_GPIO_37","MIZAR_GPIO_GP0_GPIO_38","MIZAR_GPIO_GP0_GPIO_39","MIZAR_GPIO_GPIO_INTR_RAW_STCLR1","MIZAR_GPIO_GP0_INTR1_INTR_EN1","MIZAR_GPIO_GP0_INTR1_INTR_STS1","MIZAR_LSS_SYSREG_RAW_STCR1"
            ]
        },
        {
            "Index": 3,
            "SS / Module": "GPIO",
            "Feature": "interrupts can be generated based on positive edge or negative edge or level high or level low detection at GPIO input.",
            "Test Case Name": "test_gpio_pedge_all_pads_en",
            "Test Description": "Enables positive-edge interrupts for all GPIOs 8–39, drives rising edges, and validates group interrupt status, clear, and system raw status handling.",
            "Speed": "NA",
            "Mode": "Interrupt",
            "Memory Start Offset": "0xA0243ffc",
            "Memory End Offset": "0xA0243ffc",
            "Remarks": "Requires GPIO0 or GPIO1 selection for routing. Uses GIC IRQ 87/88. Uses group IO control to set inputs and a pad driver at 0xA0243ffc to create rising edges. Timeouts prevent infinite waits.",
            "Test Steps / Procedure": "1) Enable the system interrupt output for the selected GPIO instance using INTR_EN1.\n2) Enable positive-edge detection in each of GPIO_8 through GPIO_39.\n3) Set input mode for groups 1 through 4 using GPIO_IO_CTRL_GROUP1..4.\n4) Enable all bits in INTR1_INTR_EN1.\n5) For each pin, drive low, arm the wait, then drive high to create a single rising edge and wait (with timeout) for the interrupt.\n6) In the interrupt handler, verify group interrupt status is set in INTR1_INTR_STS1, mask group enable, clear per-pin raw for all pins, and confirm group status clears to zero.\n7) Clear the system raw status via RAW_STCR1 and re-enable group interrupt for the next iteration.",
            "Impacted Registers": [
                "INTR_EN1","GPIO_8","GPIO_9","GPIO_10","GPIO_11","GPIO_12","GPIO_13","GPIO_14","GPIO_15","GPIO_16","GPIO_17","GPIO_18","GPIO_19","GPIO_20","GPIO_21","GPIO_22","GPIO_23","GPIO_24","GPIO_25","GPIO_26","GPIO_27","GPIO_28","GPIO_29","GPIO_30","GPIO_31","GPIO_32","GPIO_33","GPIO_34","GPIO_35","GPIO_36","GPIO_37","GPIO_38","GPIO_39","GPIO_IO_CTRL_GROUP1","GPIO_IO_CTRL_GROUP2","GPIO_IO_CTRL_GROUP3","GPIO_IO_CTRL_GROUP4","INTR1_INTR_EN1","INTR1_INTR_STS1","RAW_STCR1"
            ],
            "Validation / Acceptance Criteria": "1) Each rising edge is detected before the timeout expires.\n2) The group interrupt status is asserted and then clears to zero after per-pin raw clear operations.\n3) The system raw interrupt status is cleared successfully; the test passes only if no errors are recorded.",
            "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
            "Hidden_Test_Description": "Positive-edge interrupt enable for all GPIOs [8..39]; configures per-pin posedge, sets group IO control to input, enables all group bits, generates rising edges via external driver, validates group status and clears system raw.",
            "Hidden_Remarks": "Conditional routing via GPIO0 or GPIO1 and use of GIC IRQ 87/88.",
            "Hidden_Impacted_Registers": [
                "MIZAR_LSS_SYSREG_INTR_EN1","MIZAR_GPIO_GP0_GPIO_8","MIZAR_GPIO_GP0_GPIO_9","MIZAR_GPIO_GP0_GPIO_10","MIZAR_GPIO_GP0_GPIO_11","MIZAR_GPIO_GP0_GPIO_12","MIZAR_GPIO_GP0_GPIO_13","MIZAR_GPIO_GP0_GPIO_14","MIZAR_GPIO_GP0_GPIO_15","MIZAR_GPIO_GP0_GPIO_16","MIZAR_GPIO_GP0_GPIO_17","MIZAR_GPIO_GP0_GPIO_18","MIZAR_GPIO_GP0_GPIO_19","MIZAR_GPIO_GP0_GPIO_20","MIZAR_GPIO_GP0_GPIO_21","MIZAR_GPIO_GP0_GPIO_22","MIZAR_GPIO_GP0_GPIO_23","MIZAR_GPIO_GP0_GPIO_24","MIZAR_GPIO_GP0_GPIO_25","MIZAR_GPIO_GP0_GPIO_26","MIZAR_GPIO_GP0_GPIO_27","MIZAR_GPIO_GP0_GPIO_28","MIZAR_GPIO_GP0_GPIO_29","MIZAR_GPIO_GP0_GPIO_30","MIZAR_GPIO_GP0_GPIO_31","MIZAR_GPIO_GP0_GPIO_32","MIZAR_GPIO_GP0_GPIO_33","MIZAR_GPIO_GP0_GPIO_34","MIZAR_GPIO_GP0_GPIO_35","MIZAR_GPIO_GP0_GPIO_36","MIZAR_GPIO_GP0_GPIO_37","MIZAR_GPIO_GP0_GPIO_38","MIZAR_GPIO_GP0_GPIO_39","MIZAR_GPIO_GPIO_IO_CTRL_GROUP1","MIZAR_GPIO_GPIO_IO_CTRL_GROUP2","MIZAR_GPIO_GPIO_IO_CTRL_GROUP3","MIZAR_GPIO_GPIO_IO_CTRL_GROUP4","MIZAR_GPIO_GP0_INTR1_INTR_EN1","MIZAR_GPIO_GP0_INTR1_INTR_STS1","MIZAR_LSS_SYSREG_RAW_STCR1"
            ]
        }
    ]
}
# ------------------ End Embedded JSON ------------------

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

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

ALLOWED_DV = ["Required", "Blank", "Not Required"]


def now_ist():
    if ZoneInfo is not None:
        tz = ZoneInfo("Asia/Kolkata")
        dt = datetime.now(tz)
    else:
        # Fallback to manual offset +05:30 without DST
        dt = datetime.utcnow()
    return dt


def enforce_numbering(text: str) -> str:
    if text is None:
        return ""
    if isinstance(text, list):
        parts = [str(x).strip() for x in text if str(x).strip()]
    else:
        # split by newlines first
        parts = [p.strip() for p in str(text).replace('\r', '').split('\n') if p.strip()]
    out = []
    n = 1
    for p in parts:
        # remove common bullet prefixes
        q = p
        if q[:2] in ("- ", "• ", "* "):
            q = q[2:].strip()
        # remove leading numbers like '1) ' or '1. '
        if len(q) > 2 and (q[1] in ").") and q[0].isdigit():
            q = q[2:].strip()
        out.append(f"{n}. {q}")
        n += 1
    return "\n".join(out)


def set_styles(ws, max_row, max_col, wrap_cols):
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill("solid", fgColor="4F81BD")
    center = Alignment(horizontal="center", vertical="center", wrap_text=False)
    top_left = Alignment(horizontal="left", vertical="top", wrap_text=False)
    top_left_wrap = Alignment(horizontal="left", vertical="top", wrap_text=True)
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    # Header row styling
    for c in range(1, max_col + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = center
        cell.border = border

    # Data rows
    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws.cell(row=r, column=c)
            # Wrap for specific columns
            if ws.cell(row=1, column=c).value in wrap_cols:
                cell.alignment = top_left_wrap
            else:
                # Index numeric-like center; everything else left/top
                if ws.cell(row=1, column=c).value == "Index":
                    cell.alignment = Alignment(horizontal="center", vertical="top")
                else:
                    cell.alignment = top_left
            cell.border = border

    # Rough auto-fit columns by max string length
    for c in range(1, max_col + 1):
        header = str(ws.cell(row=1, column=c).value or "")
        maxlen = len(header) + 2
        for r in range(2, max_row + 1):
            v = ws.cell(row=r, column=c).value
            if v is None:
                l = 0
            else:
                l = len(str(v))
            if l + 2 > maxlen:
                maxlen = l + 2
        # Cap width
        ws.column_dimensions[ws.cell(row=1, column=c).column_letter].width = min(maxlen, 80)


def validate_xlsx(path: str) -> None:
    if not zipfile.is_zipfile(path):
        raise RuntimeError("Generated file is not a valid XLSX (not a ZIP archive)")
    # try to open with openpyxl
    _ = load_workbook(path)


def main():
    # Compute IST timestamp
    dt = now_ist()
    # When ZoneInfo not available, dt is naive UTC; convert to IST manually for strings
    if hasattr(dt, 'tzinfo') and dt.tzinfo is not None:
        ist_dt = dt
        ist_str = ist_dt.strftime('%Y-%m-%d %H:%M:%S %Z')
        ymd = ist_dt.strftime('%Y%m%d')
        hms = ist_dt.strftime('%H%M%S')
    else:
        # naive UTC -> add +05:30 approx
        from datetime import timedelta
        ist_dt = dt + timedelta(hours=5, minutes=30)
        ist_str = ist_dt.strftime('%Y-%m-%d %H:%M:%S IST')
        ymd = ist_dt.strftime('%Y%m%d')
        hms = ist_dt.strftime('%H%M%S')

    data = json.loads(json.dumps(FALLBACK_JSON))  # deep copy
    data['metadata']['generation_timestamp_ist'] = ist_str

    test_cases = data.get('test_cases', [])
    if not isinstance(test_cases, list) or not test_cases:
        raise SystemExit("Invalid or empty JSON test_cases input")

    # Phase 1 — Build union of keys preserving first-seen order
    union_keys = []
    seen = set()
    for rec in test_cases:
        if not isinstance(rec, dict):
            continue
        for k in rec.keys():
            if k not in seen:
                seen.add(k)
                union_keys.append(k)

    # Create workbook and Data sheet
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Header row
    for c, key in enumerate(union_keys, start=1):
        ws.cell(row=1, column=c, value=key)
    # Freeze header row
    ws.freeze_panes = 'A2'

    # Data rows (preserve raw values EXACTLY at this stage)
    for r, rec in enumerate(test_cases, start=2):
        for c, key in enumerate(union_keys, start=1):
            v = rec.get(key, "")
            ws.cell(row=r, column=c, value=v)

    # Create Meta_data_sheet with metadata and META columns
    meta = wb.create_sheet(title='Meta_data_sheet')
    # Top-level metadata key/value pairs
    meta.append(["ip_name", data['metadata'].get('ip_name', '')])
    meta.append(["source_repo", data['metadata'].get('source_repo', '')])
    meta.append(["subdirectory", data['metadata'].get('subdirectory', '')])
    meta.append(["generation_timestamp_ist", data['metadata'].get('generation_timestamp_ist', '')])
    # Blank row then META columns table
    meta.append([])
    meta.append(META_COLS)
    for rec in test_cases:
        row = [rec.get(col, "") for col in META_COLS]
        meta.append(row)
    # Very hidden
    meta.sheet_state = 'veryHidden'

    # Step 7 — Normalize MAIN sheet directly on 'Data' by renaming it to 'TestPlan'
    ws.title = 'TestPlan'

    # Drop META columns from visible sheet and reorder to MAIN_ORDER
    # Build mapping from header to column index for current sheet
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column + 1)]
    # Create a data matrix for all rows
    matrix = []
    for r in range(2, ws.max_row + 1):
        row_dict = {headers[c-1]: ws.cell(row=r, column=c).value for c in range(1, ws.max_column + 1)}
        matrix.append(row_dict)

    # Clear current sheet contents
    ws.delete_rows(1, ws.max_row)

    # Write MAIN_ORDER headers
    for c, key in enumerate(MAIN_ORDER, start=1):
        ws.cell(row=1, column=c, value=key)

    # Write rows with transformed values
    def join_impacted(v):
        if v is None or v == "":
            return ""
        if isinstance(v, list):
            return ", ".join([str(x) for x in v])
        return str(v)

    for r, rec in enumerate(matrix, start=2):
        row_vals = []
        for key in MAIN_ORDER:
            val = rec.get(key, "")
            if key == "Test Steps / Procedure":
                val = enforce_numbering(val)
            elif key == "Validation / Acceptance Criteria":
                val = enforce_numbering(val)
            elif key == "Impacted Registers":
                val = join_impacted(val)
                # Enforce that visible does not contain macro prefixes
                if isinstance(val, str) and 'MIZAR_' in val:
                    # Strip occurrences best-effort
                    val = val.replace('MIZAR_', '')
            row_vals.append(val)
        for c, v in enumerate(row_vals, start=1):
            ws.cell(row=r, column=c, value=v)

    # Apply formatting to TestPlan
    wrap_cols = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }
    set_styles(ws, ws.max_row, ws.max_column, wrap_cols)

    # Data validation for Code Generation (Required / Not)
    dv = DataValidation(type="list", formula1=f'"{",".join(ALLOWED_DV)}"', allow_blank=True, showErrorMessage=True)
    ws.add_data_validation(dv)
    # Apply to data rows only in the proper column
    header_idx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    cg_col = header_idx.get("Code Generation (Required / Not)")
    if cg_col:
        rng = f"{ws.cell(row=2, column=cg_col).coordinate}:{ws.cell(row=max(2, ws.max_row), column=cg_col).coordinate}"
        dv.add(rng)

    # Final visibility check: Only TestPlan (visible) and Meta_data_sheet (veryHidden)
    names = [s.title for s in wb.worksheets]
    if set(names) != {"TestPlan", "Meta_data_sheet"}:
        # attempt to fix if default leftover sheet exists
        for s in list(wb.worksheets):
            if s.title not in ("TestPlan", "Meta_data_sheet"):
                wb.remove(s)
        names = [s.title for s in wb.worksheets]
    if set(names) != {"TestPlan", "Meta_data_sheet"}:
        raise RuntimeError(f"Unexpected sheets present: {names}")

    # Save final Excel
    out_dir = os.path.join("Test_Output", "GPIO", "TestPlan")
    os.makedirs(out_dir, exist_ok=True)
    out_name = f"GPIO_TestPlan_{ymd}_{hms}.xlsx"
    out_path = os.path.join(out_dir, out_name)
    wb.save(out_path)

    # Validate XLSX
    validate_xlsx(out_path)

    # Expose outputs to GitHub Actions
    gha_out = os.environ.get('GITHUB_OUTPUT')
    if gha_out:
        with open(gha_out, 'a', encoding='utf-8') as f:
            f.write(f"output_file={out_path}\n")
            f.write(f"ist_timestamp={ist_str}\n")

    print(f"Generated: {out_path}")
    print(f"IST Timestamp: {ist_str}")

if __name__ == '__main__':
    main()
