#!/usr/bin/env python3
import os
import datetime as dt
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# Source JSON data embedded exactly as provided
json_data = [
    {
        "Index": "1",
        "SS / Module": "GPIO",
        "Feature": "GPIO Register Read/Write and Reset Default Verification",
        "Test Case Name": "gpio_reg_wr_rd_test"
    },
    {
        "Index": "2",
        "SS / Module": "GPIO",
        "Feature": "GPIO Interrupt - Negative Edge Trigger",
        "Test Case Name": "test_gpio_negedge_intr_en"
    },
    {
        "Index": "3",
        "SS / Module": "GPIO",
        "Feature": "GPIO Interrupt - Positive Edge Trigger (All Pads)",
        "Test Case Name": "test_gpio_pedge_all_pads_en",
        "Test Description": "Configure all per-pad GPIO control registers for positive-edge interrupt...",
        "Meta Test Description": "test_case(): Optionally enable GIC interrupt line 87 (GPIO0) or 88 (GPIO1)...",
        "Speed": "NA",
        "Mode": "ISR",
        "Memory Start Offset": "0xA0243ffc",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "- Ensure the platform interrupt controller line...",
        "Test Steps / Procedure": "1. Enable the platform interrupt line...",
        "Meta Test Steps / Procedure": "1) Initialization: Optionally GIC_EnableIRQ(87 or 88); ...",
        "Impacted Registers": "GPIO_8 (per-pad control, pads 8–39 via + i*4); gp0_intr2_intr_en1 (group interrupt enable); INTR1_INTR_STS1 (group interrupt status); GPIO_IO_CTRL_GROUP1; GPIO_IO_CTRL_GROUP2; GPIO_IO_CTRL_GROUP3; GPIO_IO_CTRL_GROUP4; INTR_EN1 (system interrupt enable); RAW_STCR1 (system raw status clear)",
        "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8 (+ i*4 for i=0..31); MIZAR_GPIO_GP0_INTR1_INTR_EN1; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_GPIO_GPIO_IO_CTRL_GROUP1; MIZAR_GPIO_GPIO_IO_CTRL_GROUP2; MIZAR_GPIO_GPIO_IO_CTRL_GROUP3; MIZAR_GPIO_GPIO_IO_CTRL_GROUP4; MIZAR_LSS_SYSREG_INTR_EN1; MIZAR_LSS_SYSREG_RAW_STCR1",
        "Validation / Acceptance Criteria": "Pass if, for each pad, a rising edge generates an interrupt within the timeout...",
        "Meta Validation / Acceptance Criteria": "- Timeout: After writing 0xFFFFFFFF to 0xA0243ffc, int_pend must be cleared...",
        "Code Generation (Required / Not)": "Not",
        "Meta Headers": "#include <lss_sysreg.h>...",
        "Meta Macros": "#define CNT 49",
        "Meta Arrays": "addr_array[20] = { ... }"
    }
]

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

def build_workbook(data):
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    # Headers bold
    header_font = Font(bold=True)
    ws_plan.append(TESTPLAN_COLUMNS)
    ws_meta.append(METADATA_COLUMNS)
    for cell in ws_plan[1]:
        cell.font = header_font
    for cell in ws_meta[1]:
        cell.font = header_font

    # Data rows preserving order
    for row in data:
        plan_row = [str(row.get(col, "")) for col in TESTPLAN_COLUMNS]
        meta_row = [str(row.get(col, "")) for col in METADATA_COLUMNS]
        ws_plan.append(plan_row)
        ws_meta.append(meta_row)

    # Freeze header rows
    ws_plan.freeze_panes = "A2"
    ws_meta.freeze_panes = "A2"

    # Very hide MetaData sheet
    ws_meta.sheet_state = 'veryHidden'

    return wb


def main():
    # Determine output directory (default per task)
    out_dir = os.environ.get("OUTPUT_DIR", "Test_Output/GPIO/TestPlan")
    os.makedirs(out_dir, exist_ok=True)

    # IST timestamp
    now_ist = dt.datetime.now(ZoneInfo("Asia/Kolkata"))
    ts = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"
    out_path = os.path.join(out_dir, filename)

    wb = build_workbook(json_data)
    wb.save(out_path)

    print(f"Saved: {out_path}")

if __name__ == "__main__":
    main()
