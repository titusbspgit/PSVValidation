#!/usr/bin/env python3
"""
GPIO TestPlan XLSX Generator & GitHub Pusher
=============================================
Self-contained script that generates a real .xlsx workbook with:
  - TestPlan sheet (visible, 21 columns, 2 data rows)
  - MetaData sheet (very hidden, full JSON)
Then pushes the file to GitHub.

Requirements:
  pip install openpyxl PyGithub

Usage:
  python generate_gpio_testplan.py

Environment variable required:
  GITHUB_TOKEN - GitHub Personal Access Token with repo write access
  (Or edit the TOKEN variable below directly)
"""

import json
import os
import sys
import io
import base64
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

try:
    from github import Github
except ImportError:
    print("ERROR: PyGithub not installed. Run: pip install PyGithub")
    sys.exit(1)

# ============================================================
# CONFIGURATION
# ============================================================
GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")
OWNER = "titusbspgit"
REPO = "PSVValidation"
BRANCH = "main"
OUTPUT_DIR = "Test_Output/GPIO/TestPlan/"
IP_NAME = "GPIO"

# IST timezone (GMT+05:30)
IST = timezone(timedelta(hours=5, minutes=30))
NOW_IST = datetime.now(IST)
TIMESTAMP = NOW_IST.strftime("%Y%m%d_%H%M%S")
FILENAME = f"{IP_NAME}_TestPlan_{TIMESTAMP}.xlsx"
FILE_PATH = f"{OUTPUT_DIR}{FILENAME}"

# ============================================================
# 21-COLUMN HEADERS
# ============================================================
HEADERS = [
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
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]

# ============================================================
# FINAL AGGREGATED JSON DATA (2 entries)
# ============================================================
DATA = [
    {
        "Index": 1,
        "SS / Module": "GPIO",
        "Feature": "Register Write Read Verification",
        "Test Case Name": "gpio_reg_wr_rd_test",
        "Test Description": "Verify register write and read operations for GPIO per-pin registers gp0_gpio_8 through gp0_gpio_30. The test reads default reset values, writes multiple patterns (0x00, 0xFF, 0x55, 0xAA, 0xA5, 0x5A), reads back and verifies using field-specific read/write masks considering RO, WO, RW, and RW2 field types.",
        "Speed": "NA",
        "Mode": "Polling",
        "Memory Start Offset": "0x0",
        "Memory End Offset": "0x58",
        "Remarks": "",
        "Test Steps / Procedure": "1. Read default reset value from each GPIO register (gp0_gpio_8 to gp0_gpio_30) and verify against expected reset value 0x00100000 (io_ctrl=1 at bit 20).\n2. Write pattern 0x00000000 to each register.\n3. Read back and verify: RW/RW2 fields updated, RO fields unchanged, WO fields read as 0.\n4. Repeat steps 2-3 for patterns 0x000E0000, 0x000A0000, 0x00140000, 0x00060000, 0x00180000 (masked for writable fields).\n5. Trigger soft reset via SOFT_RST_REG_ADDRESS.\n6. Re-read all registers and verify reset values restored.",
        "Impacted Registers": "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10; gp0_gpio_11; gp0_gpio_12; gp0_gpio_13; gp0_gpio_14; gp0_gpio_15; gp0_gpio_16; gp0_gpio_17; gp0_gpio_18; gp0_gpio_19; gp0_gpio_20; gp0_gpio_21; gp0_gpio_22; gp0_gpio_23; gp0_gpio_24; gp0_gpio_25; gp0_gpio_26; gp0_gpio_27; gp0_gpio_28; gp0_gpio_29; gp0_gpio_30",
        "Validation / Acceptance Criteria": "PASS: All registers return expected reset value 0x00100000 on initial read. All write patterns are correctly reflected in RW/RW2 fields on read-back. RO fields (data_in, intr_raw_sts) remain unchanged after write. WO field (intr_clr) reads as 0. After soft reset, all registers return to reset value. FAIL: Any mismatch between expected and actual values.",
        "Code Generation (Required / Not)": "Not Required",
        "Meta Test Description": "Verify register write and read operations for GPIO per-pin registers gp0_gpio_8 through gp0_gpio_30. The test reads default reset values, writes multiple patterns (0x00, 0xFF, 0x55, 0xAA, 0xA5, 0x5A), reads back and verifies using field-specific read/write masks considering RO, WO, RW, and RW2 field types.",
        "Meta Test Steps / Procedure": "1. Read default reset value from each GPIO register (gp0_gpio_8 to gp0_gpio_30) and verify against expected reset value 0x00100000 (io_ctrl=1 at bit 20).\n2. Write pattern 0x00000000 to each register.\n3. Read back and verify: RW/RW2 fields updated, RO fields unchanged, WO fields read as 0.\n4. Repeat steps 2-3 for patterns 0x000E0000, 0x000A0000, 0x00140000, 0x00060000, 0x00180000 (masked for writable fields).\n5. Trigger soft reset via SOFT_RST_REG_ADDRESS.\n6. Re-read all registers and verify reset values restored.",
        "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; MIZAR_GPIO_GP0_GPIO_11; MIZAR_GPIO_GP0_GPIO_12; MIZAR_GPIO_GP0_GPIO_13; MIZAR_GPIO_GP0_GPIO_14; MIZAR_GPIO_GP0_GPIO_15; MIZAR_GPIO_GP0_GPIO_16; MIZAR_GPIO_GP0_GPIO_17; MIZAR_GPIO_GP0_GPIO_18; MIZAR_GPIO_GP0_GPIO_19; MIZAR_GPIO_GP0_GPIO_20; MIZAR_GPIO_GP0_GPIO_21; MIZAR_GPIO_GP0_GPIO_22; MIZAR_GPIO_GP0_GPIO_23; MIZAR_GPIO_GP0_GPIO_24; MIZAR_GPIO_GP0_GPIO_25; MIZAR_GPIO_GP0_GPIO_26; MIZAR_GPIO_GP0_GPIO_27; MIZAR_GPIO_GP0_GPIO_28; MIZAR_GPIO_GP0_GPIO_29; MIZAR_GPIO_GP0_GPIO_30; SOFT_RST_REG_ADDRESS",
        "Meta Validation / Acceptance Criteria": "PASS: All registers return expected reset value 0x00100000 on initial read. All write patterns are correctly reflected in RW/RW2 fields on read-back. RO fields (data_in, intr_raw_sts) remain unchanged after write. WO field (intr_clr) reads as 0. After soft reset, all registers return to reset value. FAIL: Any mismatch between expected and actual values.",
        "Meta Headers": "",
        "Meta Macros": "",
        "Meta Arrays": "",
    },
    {
        "Index": 2,
        "SS / Module": "GPIO",
        "Feature": "Level Select Interrupt Enable",
        "Test Case Name": "test_gpio_level_sel_intr_en",
        "Test Description": "Verify GPIO level-select interrupt functionality for all 32 GPIO pins (pin 8 through pin 39). The test configures each per-pin GPIO register in input mode with level-triggered interrupt enabled, enables the group interrupt via the group interrupt enable register, enables LSS sysreg interrupt routing via the sysreg interrupt enable register, drives an external stimulus, and verifies interrupt assertion through the per-pin interrupt raw status bit, the group interrupt status register, and the LSS sysreg raw status register. The test covers both active HIGH level (level_sel=1) and active LOW level (level_sel=0) interrupt generation. After each interrupt, the test clears the per-pin interrupt, verifies the clear succeeded, disables the group interrupt, verifies the group status is cleared, clears the sysreg raw status, and verifies the sysreg clear succeeded. The test uses GIC IRQ 87 for GPIO0 interrupt routing.",
        "Speed": "NA",
        "Mode": "ISR",
        "Memory Start Offset": "0x0",
        "Memory End Offset": "0x88",
        "Remarks": "Requires GIC interrupt controller to be initialized and IRQ 87 routed for GPIO0. External stimulus hardware must be connected at the designated SRAM location to drive GPIO pin levels. The test depends on conditional compilation with GPIO0 defined. LSS sysreg interrupt enable must be configured before GPIO interrupts can propagate to the GIC.",
        "Test Steps / Procedure": "1. Enable GIC IRQ for GPIO0 interrupt routing.\n2. Write to the LSS sysreg interrupt enable register (intr_en1) to enable GPIO0 interrupt at bit 1.\n3. For each GPIO pin (pin 8 to pin 39), configure the per-pin register (gp0_gpio_8 through gp0_gpio_39) in input mode with level-select interrupt set to active HIGH (io_ctrl=1, level_sel=1).\n4. Enable the group interrupt for the target pin by writing the corresponding bit to the group interrupt enable register (gp0_intr1_intr_en1).\n5. Drive external stimulus HIGH by writing to the external stimulus address to trigger the level interrupt.\n6. Wait for the interrupt service routine to execute.\n7. In the ISR, read the per-pin register and verify the interrupt raw status bit (bit 1) is asserted.\n8. Read the group interrupt status register (gp0_intr1_intr_sts1) and verify the corresponding pin bit is set.\n9. Clear the per-pin interrupt by writing to the interrupt clear field (bit 16) in the per-pin register.\n10. Read back the per-pin register and verify the interrupt has been cleared successfully (expected value 0x100001).\n11. Disable the group interrupt by writing 0x00000000 to the group interrupt enable register.\n12. Read the group interrupt status register and verify it reads 0x0.\n13. Write to the LSS sysreg raw status clear register (raw_stcr1) to clear the GPIO0 interrupt status.\n14. Read back the sysreg raw status register and verify the GPIO0 interrupt bit is cleared.\n15. Clear the GIC IRQ.\n16. Repeat steps 3-15 for all 32 GPIO pins with active HIGH level.\n17. Repeat the entire sequence (steps 3-16) with level-select set to active LOW (io_ctrl=1, level_sel=0) and drive external stimulus LOW for each pin.\n18. Verify the test completes with zero errors.",
        "Impacted Registers": "intr_en1; gp0_gpio_8; gp0_intr1_intr_en1; gp0_intr1_intr_sts1; raw_stcr1",
        "Validation / Acceptance Criteria": "PASS: For each of the 32 GPIO pins in active HIGH mode (level_sel=1): the per-pin register interrupt raw status bit (bit 1) is asserted when the external stimulus is HIGH. The group interrupt status register (gp0_intr1_intr_sts1) shows the corresponding pin bit set. After clearing the interrupt via the per-pin register interrupt clear field, the per-pin register reads 0x100001. After disabling the group interrupt enable, the group interrupt status register reads 0x0. After clearing the sysreg raw status register (raw_stcr1), the GPIO0 interrupt bit is cleared. For each of the 32 GPIO pins in active LOW mode (level_sel=0): the same validation sequence passes when the external stimulus is driven LOW for the target pin. The test completes with zero accumulated errors. FAIL: Any per-pin interrupt raw status not asserted, any group interrupt status mismatch, any interrupt clear failure (per-pin register not equal to 0x100001), any group interrupt status not clearing to 0x0, any sysreg raw status not clearing, or any interrupt not occurring.",
        "Code Generation (Required / Not)": "Not Required",
        "Meta Test Description": "Verify GPIO level-select interrupt functionality for all 32 GPIO pins (GPIO_8 to GPIO_39). The test enables GIC IRQ 87 (GPIO0) via GIC_EnableIRQ(87). It writes LSS_SYSREG_INTR_EN1_GPIO0_INTR to MIZAR_LSS_SYSREG_INTR_EN1 to enable sysreg interrupt routing. In the first loop (active HIGH, i=0..31): writes 0x00180000 to MIZAR_GPIO_GP0_GPIO_8+(i*4) setting io_ctrl=1 (bit 20) and level_sel=1 (bit 19), writes (1<<i) to MIZAR_GPIO_GP0_INTR1_INTR_EN1 (offset 0x84) to enable group interrupt for pin i, writes 0xffffffff to 0xA0243ffc to drive external stimulus HIGH, sets int_pend=1 and polls while(int_pend==1) with wait_on(10). In Default_IRQHandler: sets int_pend=0, writes 0xffffffff to 0xA0243ffc, reads MIZAR_GPIO_GP0_GPIO_8+(i*4) into rdata, checks (rdata & 0x2) != 0x0 for intr_raw_sts assertion, reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 (offset 0x88) into rdata_grp, checks (rdata_grp & (1<<i)) != 0 for group interrupt, writes 0x00110000 to MIZAR_GPIO_GP0_GPIO_8+(i*4) to clear interrupt, waits wait_on(20), reads back and checks rdata==0x100001, writes 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1, reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 and checks ==0x0, writes LSS_SYSREG_RAW_STCR1_GPIO0_INTR to MIZAR_LSS_SYSREG_RAW_STCR1, reads back and checks cleared, GIC_ClearIRQ(87). Second loop (active LOW): writes 0x00100000, drives ~(wr_val). finish(test_err).",
        "Meta Test Steps / Procedure": "1. GIC_EnableIRQ(87).\n2. write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR).\n3. Active HIGH loop (i=0 to 31): write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00180000); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 1<<i); write_reg(0xA0243ffc, 0xffffffff).\n4. ISR: rdata=read_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4)); check (rdata & 0x2) != 0.\n5. rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); check (rdata_grp & (1<<i)) != 0.\n6. write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00110000); check read_reg==0x100001.\n7. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0); check read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)==0.\n8. write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); check cleared.\n9. GIC_ClearIRQ(87).\n10. Active LOW loop: write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00100000); write_reg(0xA0243ffc, ~(1<<i)).\n11. finish(test_err).",
        "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_INTR1_INTR_EN1; 0xA0243ffc; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_LSS_SYSREG_RAW_STCR1",
        "Meta Validation / Acceptance Criteria": "PASS conditions: (1) (rdata & 0x2) != 0x0. (2) (rdata_grp & (1<<i)) != 0. (3) read_reg == 0x100001 after clear. (4) group status == 0x0 after disable. (5) sysreg raw_stcr1 gpio0_intr bit cleared. FAIL if test_err > 0.",
        "Meta Headers": "#include<lss_sysreg.h>; #include<stdio.h>; #include<test_define.c>; #include<test_common.h>; #include<gpio/gpio_def.h>; #include<gpio/gpio_offset.h>",
        "Meta Macros": "#define CNT 49",
        "Meta Arrays": "const unsigned long int addr_array[49]={MIZAR_GPIO_GP0_GPIO_8,...}; const unsigned int default_value_array[49]={...}; const unsigned int read_mask_array[49]={...}; const unsigned int write_mask_array[49]={...}; const int skip_array[49]={0,...,0};",
    },
]

# ============================================================
# COLUMN WIDTHS (approximate, in characters)
# ============================================================
COL_WIDTHS = {
    1: 8,    # Index
    2: 14,   # SS / Module
    3: 30,   # Feature
    4: 35,   # Test Case Name
    5: 60,   # Test Description
    6: 8,    # Speed
    7: 10,   # Mode
    8: 20,   # Memory Start Offset
    9: 20,   # Memory End Offset
    10: 40,  # Remarks
    11: 70,  # Test Steps / Procedure
    12: 50,  # Impacted Registers
    13: 70,  # Validation / Acceptance Criteria
    14: 25,  # Code Generation
    15: 60,  # Meta Test Description
    16: 70,  # Meta Test Steps / Procedure
    17: 50,  # Meta Impacted Registers
    18: 70,  # Meta Validation / Acceptance Criteria
    19: 40,  # Meta Headers
    20: 25,  # Meta Macros
    21: 50,  # Meta Arrays
}


def create_workbook():
    """Create the .xlsx workbook with TestPlan and MetaData sheets."""
    wb = Workbook()

    # ---- TestPlan Sheet (visible) ----
    ws = wb.active
    ws.title = "TestPlan"

    # Header styles
    header_font = Font(name="Calibri", size=11, bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    header_alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
    thin_border = Border(
        left=Side(style="thin"),
        right=Side(style="thin"),
        top=Side(style="thin"),
        bottom=Side(style="thin"),
    )

    # Write header row
    for col_idx, header in enumerate(HEADERS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=header)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment
        cell.border = thin_border

    # Data styles
    data_font = Font(name="Calibri", size=10)
    data_alignment = Alignment(vertical="top", wrap_text=True)

    # Write data rows
    for row_idx, entry in enumerate(DATA, start=2):
        for col_idx, header in enumerate(HEADERS, start=1):
            value = entry.get(header, "")
            cell = ws.cell(row=row_idx, column=col_idx, value=value)
            cell.font = data_font
            cell.alignment = data_alignment
            cell.border = thin_border

    # Set column widths
    for col_idx, width in COL_WIDTHS.items():
        ws.column_dimensions[get_column_letter(col_idx)].width = width

    # Freeze first row
    ws.freeze_panes = "A2"

    # Set row heights for data rows
    for row_idx in range(2, len(DATA) + 2):
        ws.row_dimensions[row_idx].height = 200

    # ---- MetaData Sheet (very hidden) ----
    ws_meta = wb.create_sheet(title="MetaData")
    ws_meta.sheet_state = "veryHidden"

    # Write full JSON as a single cell
    full_json = json.dumps(DATA, indent=2, ensure_ascii=False)
    ws_meta.cell(row=1, column=1, value="Full JSON Data")
    ws_meta.cell(row=1, column=1).font = Font(bold=True)

    # Split JSON into chunks of 32000 chars (Excel cell limit is ~32767)
    chunk_size = 32000
    chunks = [full_json[i : i + chunk_size] for i in range(0, len(full_json), chunk_size)]
    for i, chunk in enumerate(chunks):
        ws_meta.cell(row=2 + i, column=1, value=chunk)

    # Also write individual field data for each entry
    meta_row = 2 + len(chunks) + 1
    ws_meta.cell(row=meta_row, column=1, value="--- Individual Entry Data ---")
    ws_meta.cell(row=meta_row, column=1).font = Font(bold=True)
    meta_row += 1

    for entry in DATA:
        for key, val in entry.items():
            ws_meta.cell(row=meta_row, column=1, value=str(key))
            ws_meta.cell(row=meta_row, column=2, value=str(val))
            meta_row += 1
        meta_row += 1  # blank row between entries

    return wb


def save_and_push(wb):
    """Save workbook to bytes and push to GitHub."""
    # Save to bytes buffer
    buffer = io.BytesIO()
    wb.save(buffer)
    xlsx_bytes = buffer.getvalue()
    buffer.close()

    print(f"Generated XLSX: {len(xlsx_bytes)} bytes")
    print(f"Target path: {FILE_PATH}")

    # Also save locally
    local_path = FILENAME
    with open(local_path, "wb") as f:
        f.write(xlsx_bytes)
    print(f"Saved locally: {local_path}")

    # Push to GitHub
    if not GITHUB_TOKEN:
        print("WARNING: GITHUB_TOKEN not set. File saved locally only.")
        print(f"To push manually, set GITHUB_TOKEN environment variable and re-run.")
        return None, local_path

    g = Github(GITHUB_TOKEN)
    repo = g.get_repo(f"{OWNER}/{REPO}")

    # Check if file already exists
    try:
        existing = repo.get_contents(FILE_PATH, ref=BRANCH)
        result = repo.update_file(
            path=FILE_PATH,
            message=f"Update {FILENAME} - GPIO TestPlan XLSX (IST {NOW_IST.strftime('%Y-%m-%d %H:%M:%S')})",
            content=xlsx_bytes,
            sha=existing.sha,
            branch=BRANCH,
        )
    except Exception:
        result = repo.create_file(
            path=FILE_PATH,
            message=f"Add {FILENAME} - GPIO TestPlan XLSX (IST {NOW_IST.strftime('%Y-%m-%d %H:%M:%S')})",
            content=xlsx_bytes,
            branch=BRANCH,
        )

    commit_sha = result["commit"].sha
    github_url = f"https://github.com/{OWNER}/{REPO}/blob/{BRANCH}/{FILE_PATH}"

    print(f"\nSUCCESS!")
    print(f"Commit SHA: {commit_sha}")
    print(f"GitHub URL: {github_url}")

    return commit_sha, github_url


def main():
    print("=" * 60)
    print("GPIO TestPlan XLSX Generator")
    print("=" * 60)
    print(f"IST Timestamp: {NOW_IST.strftime('%Y-%m-%d %H:%M:%S')} IST")
    print(f"Filename: {FILENAME}")
    print(f"Entries: {len(DATA)}")
    print(f"Columns: {len(HEADERS)}")
    print()

    # Step 1: Create workbook
    print("Step 1: Creating XLSX workbook...")
    wb = create_workbook()
    print("  - TestPlan sheet: 1 header + 2 data rows, 21 columns")
    print("  - MetaData sheet: veryHidden, full JSON")

    # Step 2: Save and push
    print("\nStep 2: Saving and pushing to GitHub...")
    commit_sha, url = save_and_push(wb)

    # Step 3: Output status
    print("\n" + "=" * 60)
    status = {
        "status": "SUCCESS" if commit_sha else "SUCCESS_LOCAL_ONLY",
        "execution_mode": "Fallback Automation",
        "output_file_path": FILE_PATH,
        "github_url": url or "NA",
        "commit_sha": commit_sha or "NA",
        "filename": FILENAME,
        "ist_timestamp": NOW_IST.strftime("%Y-%m-%d %H:%M:%S IST"),
        "total_rows": len(DATA),
        "total_columns": len(HEADERS),
    }
    print(json.dumps(status, indent=2))
    print("=" * 60)


if __name__ == "__main__":
    main()
