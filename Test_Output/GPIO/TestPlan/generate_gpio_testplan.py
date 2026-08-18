#!/usr/bin/env python3
"""
Fallback Automation Script: GPIO TestPlan Excel Generator & GitHub Pusher
=========================================================================
This script generates a real binary .xlsx workbook with two sheets:
  - TestPlan (visible)
  - MetaData (veryHidden)
And pushes it to the GitHub repository.

Requirements:
  pip install openpyxl PyGithub

Usage:
  python generate_gpio_testplan.py
"""

import os
import sys
import base64
from datetime import datetime, timezone, timedelta
from io import BytesIO

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# ============================================================
# IST Timestamp
# ============================================================
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
TIMESTAMP = now_ist.strftime("%Y%m%d_%H%M%S")
IP_NAME = "GPIO"
FILENAME = f"{IP_NAME}_TestPlan_{TIMESTAMP}.xlsx"
OUTPUT_DIR = "Test_Output/GPIO/TestPlan"
OUTPUT_PATH = f"{OUTPUT_DIR}/{FILENAME}"

# ============================================================
# GitHub Config
# ============================================================
GITHUB_OWNER = "titusbspgit"
GITHUB_REPO = "PSVValidation"
GITHUB_BRANCH = "main"

# ============================================================
# Test Data - 2 Records
# ============================================================
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
    "Code Generation (Required / Not)"
]

METADATA_COLUMNS = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays"
]

RECORDS = [
    {
        "Index": "1",
        "SS / Module": "GPIO",
        "Feature": "Register Write-Read Verification",
        "Test Case Name": "gpio_reg_wr_rd_test",
        "Test Description": "This test verifies the default reset values and write-read functionality of GPIO registers (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10 and others). It first reads all GPIO registers and compares against expected default values. Then it writes multiple data patterns (all-ones, alternating bits, etc.) to each writable register and reads back to verify correctness using read and write masks. The test reports pass if all default value checks and write-read checks succeed, and fail if any mismatch is detected.",
        "Meta Test Description": "The testcase includes test_define.c which defines CNT=49 registers in addr_array[] using macros such as MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10 along with corresponding default_value_array[], read_mask_array[], write_mask_array[], skip_array[], and skip_rst_array[]. The main test_case() function calls chk_rst_val() to iterate over all CNT registers, reading each via read_reg(addr), masking with 0xfffffffe, and comparing against default_value_array[i]. Registers flagged in skip_rst_array or with read_mask_array==0x00000000 are skipped. Then chk_rd_wr() iterates over 6 check values {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}, writing (data_wr & write_mask_array[i]) to each register, then reading back with (read_reg(addr) & read_mask_array[i]) and comparing against expected value computed as ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Registers in skip_array or with write_mask_array==0 or read_mask_array==0 are skipped. def_fail_cnt and wr_fail_cnt track failures. finish(1) is called on failure, finish(0) on success. A soft_reset_chk() function exists but is disabled via #ifdef 0.",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "Some registers are skipped during default value check and write-read check based on skip_rst_array and skip_array flags. Registers with read mask or write mask of 0x00000000 are also skipped. The din bit may automatically become 1 when reading default values if no value is forced, affecting level select bit behavior.",
        "Test Steps / Procedure": "1. Read all GPIO registers and verify that each register contains its expected default reset value after masking.\n2. Skip registers that are flagged as non-readable or excluded from reset value checking.\n3. Write the first data pattern (all ones) to all writable GPIO registers using the appropriate write mask.\n4. Read back each register using the read mask and verify the read value matches the expected value.\n5. Repeat write and readback verification for the remaining five data patterns (alternating bit patterns and mixed patterns).\n6. Skip registers flagged as non-writable or excluded from write-read checking.\n7. Report overall pass if all default value checks and all write-read checks succeed; report fail if any mismatch is detected.",
        "Meta Test Steps / Procedure": "1. Include test_define.c which defines addr_array[49] with register macros (MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, ...), default_value_array[49], read_mask_array[49], write_mask_array[49], skip_array[49], skip_rst_array[49]. CNT=49.\n2. Call chk_rst_val(): for i=0 to CNT-1, get addr=addr_array[i]. If skip_rst_array[i]==1, skip. If read_mask_array[i]==0x00000000, skip. Else data_rd=read_reg(addr), data=(data_rd & 0xfffffffe). Compare data==default_value_array[i]. If mismatch, increment def_fail_cnt.\n3. Call chk_rd_wr(): define chk_val[6]={0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For j=0 to 5: data_wr=chk_val[j]. For i=0 to CNT-1: if skip_array[i]==1, skip. If write_mask_array[i]==0x00000000, skip. Else write_reg(addr, data_wr & write_mask_array[i]).\n4. For i=0 to CNT-1: if skip_array[i]==1, skip. If write_mask_array[i]==0, skip. If read_mask_array[i]==0, skip. Else data_rd=(read_reg(addr) & read_mask_array[i]). wr_n=(write_mask_array[i] ^ 0xffffffff). exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Compare data_rd==exp_val. If mismatch, increment wr_fail_cnt.\n5. If def_fail_cnt>0 || wr_fail_cnt>0, call finish(1). Else call finish(0).",
        "Impacted Registers": "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10",
        "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10",
        "Validation / Acceptance Criteria": "PASS: All GPIO registers return their expected default reset values after masking, and all write-read cycles for six data patterns produce matching readback values. FAIL: Any register default value mismatch or any write-read mismatch is detected.",
        "Meta Validation / Acceptance Criteria": "PASS: For all i in 0..CNT-1 (non-skipped), (read_reg(addr_array[i]) & 0xfffffffe) == default_value_array[i] (def_fail_cnt remains 0). For all 6 chk_val patterns and all non-skipped registers, (read_reg(addr) & read_mask_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])) (wr_fail_cnt remains 0). finish(0) is called. FAIL: def_fail_cnt > 0 || wr_fail_cnt > 0, finish(1) is called.",
        "Code Generation (Required / Not)": "Not",
        "Meta Headers": "#include <stdio.h>; #include <stdlib.h>; #include \"test_common.h\"; #include \"test_define.c\"; #include<gpio/gpio_def.h>; #include<gpio/gpio_offset.h>",
        "Meta Macros": "#define SOFT_RST_REG_ADDRESS 0x00000000; #define SOFT_RST_REG_DATA 0x00000000; #define CNT 49",
        "Meta Arrays": "const unsigned long int addr_array[49]={MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,}; const unsigned int default_value_array[49]={GPIO_GP0_GPIO_8_DEFAULT_VAL,GPIO_GP0_GPIO_9_DEFAULT_VAL,GPIO_GP0_GPIO_10_DEFAULT_VAL,}; const unsigned int read_mask_array[49]={GPIO_GP0_GPIO_8_READ_MASK,GPIO_GP0_GPIO_9_READ_MASK,GPIO_GP0_GPIO_10_READ_MASK,}; const unsigned int write_mask_array[49]={GPIO_GP0_GPIO_8_WRITE_MASK,GPIO_GP0_GPIO_9_WRITE_MASK,GPIO_GP0_GPIO_10_WRITE_MASK,}; const unsigned int skip_array[49]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,}; const unsigned int skip_rst_array[49]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,}; unsigned int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}"
    },
    {
        "Index": "2",
        "SS / Module": "GPIO",
        "Feature": "Level-Select Interrupt Enable",
        "Test Case Name": "test_gpio_level_sel_intr_en",
        "Test Description": "This test verifies GPIO level-triggered interrupt functionality for GPIOs 8 through 39. It configures each GPIO pad in input mode with level interrupt enabled (active high and active low), enables the group interrupt, triggers the interrupt, and validates that the interrupt is raised and can be cleared. The test uses an ISR-driven flow where the interrupt handler checks the raw interrupt status bit, verifies the group interrupt status in gp0_intr1_intr_sts1, clears the interrupt, and clears the system register status via raw_stcr1. Both active-high and active-low level selections are tested for all 32 GPIO pads.",
        "Meta Test Description": "The testcase configures GPIO level-triggered interrupts for GPIO pads 8-39 (32 pads). In test_case(), GIC_EnableIRQ(87) or GIC_EnableIRQ(88) is called based on GPIO0/GPIO1 ifdef. MIZAR_LSS_SYSREG_INTR_EN1 is written with LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR to enable sysreg interrupt. For active-high level: loop i=0..31, write 0x00180000 to MIZAR_GPIO_GP0_GPIO_8+(i*4) to set input mode with level interrupt (bit 20=1, bit 19=1), write (1<<i) to MIZAR_GPIO_GP0_INTR1_INTR_EN1 to enable group interrupt, write 0xffffffff to 0xA0243ffc (SRAM trigger), set int_pend=1 and poll while(int_pend==1) with wait_on(10). For active-low level: loop i=0..31, write 0x00100000 to MIZAR_GPIO_GP0_GPIO_8+(i*4) (bit 20=1, bit 19=0), write (1<<i) to MIZAR_GPIO_GP0_INTR1_INTR_EN1, write ~(wr_val) to 0xA0243ffc, set int_pend=1 and poll. Default_IRQHandler(): reads MIZAR_GPIO_GP0_GPIO_8+(i*4), checks (rdata & 0x2)!=0 for raw interrupt status. Reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 and checks (rdata_grp & (1<<i))!=0 for group interrupt. Clears interrupt by writing 0x00110000 to MIZAR_GPIO_GP0_GPIO_8+(i*4), waits, reads back and checks rdata==0x100001. Disables group interrupt by writing 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1. Reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 and checks ==0. Clears sysreg status via MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO0_INTR/GPIO1_INTR and verifies cleared. Calls GIC_ClearIRQ. test_err incremented on each failure. finish(test_err) called at end.",
        "Speed": "NA",
        "Mode": "ISR",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "Requires GIC interrupt controller to be available for IRQ 87 (GPIO0) or IRQ 88 (GPIO1). Test behavior depends on GPIO0 or GPIO1 compile-time ifdef selection. An SRAM location at 0xA0243ffc is used to trigger the interrupt stimulus. The int_pend flag is used for ISR synchronization between the main loop and the interrupt handler.",
        "Test Steps / Procedure": "1. Enable the GIC interrupt for the appropriate GPIO instance (GPIO0 or GPIO1).\n2. Enable the system register interrupt for the selected GPIO instance by writing to the intr_en1 register.\n3. Configure each GPIO pad (8 through 39) in input mode with active-high level interrupt enabled.\n4. Enable the group interrupt for the current GPIO pad via the gp0_intr1_intr_en1 register.\n5. Trigger the interrupt by writing to the SRAM trigger location and wait for the ISR to execute.\n6. In the ISR, verify that the raw interrupt status bit is set in the GPIO pad register.\n7. Verify that the group interrupt status is set in the gp0_intr1_intr_sts1 register.\n8. Clear the interrupt by writing the clear value to the GPIO pad register and verify the register reads back the expected cleared value.\n9. Disable the group interrupt and verify the group interrupt status is cleared.\n10. Clear the system register interrupt status via the raw_stcr1 register and verify it is cleared.\n11. Repeat steps 3 through 10 for active-low level interrupt selection for all 32 GPIO pads.\n12. Report pass if no errors occurred across all pads and both level selections; report fail otherwise.",
        "Meta Test Steps / Procedure": "1. GIC_EnableIRQ(87) for GPIO0 or GIC_EnableIRQ(88) for GPIO1.\n2. write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) or LSS_SYSREG_INTR_EN1_GPIO1_INTR.\n3. Active-high loop: for i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00180000). wait_on(50). write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 1<<i). wait_on(10). write_reg(0xA0243ffc, 0xffffffff). int_pend=1. while(int_pend==1) { wait_on(10); }.\n4. Active-low loop: for i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00100000). wait_on(50). write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 1<<i). wait_on(10). write_reg(0xA0243ffc, ~(1<<i)). int_pend=1. while(int_pend==1) { wait_on(10); }.\n5. Default_IRQHandler(): int_pend=0. write_reg(0xA0243ffc, 0xffffffff). rdata=read_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4)). Check (rdata & 0x2)!=0x0 for raw interrupt status.\n6. rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1). Check (rdata_grp & (1<<i))!=0 for group interrupt. If fail, test_err++.\n7. write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00110000). wait_on(20). rdata=read_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4)). Check rdata==0x100001. If fail, test_err++.\n8. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000). rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1). Check rdata_grp==0x0. If fail, test_err++.\n9. write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or GPIO1_INTR). rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1). Check (rdata & LSS_SYSREG_RAW_STCR1_GPIOx_INTR)==0. If fail, test_err++.\n10. GIC_ClearIRQ(87 or 88). If (rdata & 0x2)==0, print 'Interrupt Not occured', test_err++.\n11. finish(test_err).",
        "Impacted Registers": "intr_en1; gp0_gpio_8; gp0_intr1_intr_en1; gp0_intr1_intr_sts1; raw_stcr1",
        "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_INTR1_INTR_EN1; 0xA0243ffc; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_LSS_SYSREG_RAW_STCR1",
        "Validation / Acceptance Criteria": "PASS: For all 32 GPIO pads in both active-high and active-low level modes: the raw interrupt status bit is set in the GPIO pad register upon interrupt, the group interrupt status is set in gp0_intr1_intr_sts1, the interrupt clears successfully with the pad register reading back the expected cleared value (0x100001), the group interrupt status clears to zero after disabling, and the system register status in raw_stcr1 clears successfully. No errors are accumulated. FAIL: Any pad fails to raise the raw interrupt, group interrupt status is not set, interrupt clear readback does not match expected value, group interrupt status does not clear, or system register status does not clear.",
        "Meta Validation / Acceptance Criteria": "PASS: For each i=0..31 in both active-high and active-low loops: (rdata & 0x2) != 0x0 (raw interrupt status set). (rdata_grp & (1<<i)) != 0 (group interrupt set in MIZAR_GPIO_GP0_INTR1_INTR_STS1). After clearing: rdata == 0x100001 (interrupt cleared in MIZAR_GPIO_GP0_GPIO_8+(i*4)). After disabling group interrupt: rdata_grp == 0x0 (MIZAR_GPIO_GP0_INTR1_INTR_STS1 cleared). (rdata & LSS_SYSREG_RAW_STCR1_GPIOx_INTR) == 0 (MIZAR_LSS_SYSREG_RAW_STCR1 cleared). test_err remains 0. finish(0) called. FAIL: Any of the above checks fail, test_err is incremented, finish(test_err) called with non-zero value.",
        "Code Generation (Required / Not)": "Not",
        "Meta Headers": "#include<lss_sysreg.h>; #include<stdio.h>; #include<test_define.c>; #include<test_common.h>; #include<gpio/gpio_def.h>; #include<gpio/gpio_offset.h>",
        "Meta Macros": "#define CNT 49",
        "Meta Arrays": "const unsigned long int addr_array[49]={MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,}; const unsigned int default_value_array[49]={GPIO_GP0_GPIO_8_DEFAULT_VAL,GPIO_GP0_GPIO_9_DEFAULT_VAL,GPIO_GP0_GPIO_10_DEFAULT_VAL,}; const unsigned int read_mask_array[49]={GPIO_GP0_GPIO_8_READ_MASK,GPIO_GP0_GPIO_9_READ_MASK,GPIO_GP0_GPIO_10_READ_MASK,}; const unsigned int write_mask_array[49]={GPIO_GP0_GPIO_8_WRITE_MASK,GPIO_GP0_GPIO_9_WRITE_MASK,GPIO_GP0_GPIO_10_WRITE_MASK,}; const int skip_array[49]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}"
    }
]


def create_workbook():
    """Create the .xlsx workbook with TestPlan and MetaData sheets."""
    wb = Workbook()

    # ---- Styles ----
    header_font = Font(name="Calibri", bold=True, size=11, color="FFFFFF")
    header_fill_tp = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")  # Blue for TestPlan
    header_fill_md = PatternFill(start_color="548235", end_color="548235", fill_type="solid")  # Green for MetaData
    cell_alignment = Alignment(wrap_text=True, vertical="top")
    header_alignment = Alignment(wrap_text=True, vertical="center", horizontal="center")
    thin_border = Border(
        left=Side(style="thin"),
        right=Side(style="thin"),
        top=Side(style="thin"),
        bottom=Side(style="thin")
    )

    # Column widths
    TP_COL_WIDTHS = {
        "Index": 8,
        "SS / Module": 15,
        "Feature": 30,
        "Test Case Name": 30,
        "Test Description": 60,
        "Speed": 10,
        "Mode": 10,
        "Memory Start Offset": 20,
        "Memory End Offset": 20,
        "Remarks": 50,
        "Test Steps / Procedure": 60,
        "Impacted Registers": 35,
        "Validation / Acceptance Criteria": 60,
        "Code Generation (Required / Not)": 25
    }

    MD_COL_WIDTHS = {
        "Index": 8,
        "SS / Module": 15,
        "Feature": 30,
        "Test Case Name": 30,
        "Meta Test Description": 70,
        "Meta Test Steps / Procedure": 70,
        "Meta Impacted Registers": 45,
        "Meta Validation / Acceptance Criteria": 70,
        "Meta Headers": 50,
        "Meta Macros": 50,
        "Meta Arrays": 70
    }

    # ========== Sheet 1: TestPlan (visible) ==========
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    # Write headers
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill_tp
        cell.alignment = header_alignment
        cell.border = thin_border

    # Write data rows
    for row_idx, record in enumerate(RECORDS, 2):
        for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
            value = record.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_alignment
            cell.border = thin_border

    # Set column widths
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        col_letter = get_column_letter(col_idx)
        ws_tp.column_dimensions[col_letter].width = TP_COL_WIDTHS.get(col_name, 20)

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # ========== Sheet 2: MetaData (veryHidden) ==========
    ws_md = wb.create_sheet(title="MetaData")

    # Write headers
    for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill_md
        cell.alignment = header_alignment
        cell.border = thin_border

    # Write data rows
    for row_idx, record in enumerate(RECORDS, 2):
        for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
            value = record.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_alignment
            cell.border = thin_border

    # Set column widths
    for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
        col_letter = get_column_letter(col_idx)
        ws_md.column_dimensions[col_letter].width = MD_COL_WIDTHS.get(col_name, 20)

    # Freeze first row
    ws_md.freeze_panes = "A2"

    # Set MetaData sheet as veryHidden
    ws_md.sheet_state = "veryHidden"

    return wb


def save_and_get_bytes(wb):
    """Save workbook to BytesIO and return bytes."""
    buffer = BytesIO()
    wb.save(buffer)
    buffer.seek(0)
    return buffer.read()


def push_to_github(file_bytes, filepath):
    """Push the file to GitHub using PyGithub."""
    try:
        from github import Github
    except ImportError:
        print("ERROR: PyGithub not installed. Run: pip install PyGithub")
        print("Saving file locally instead...")
        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, "wb") as f:
            f.write(file_bytes)
        print(f"File saved locally: {filepath}")
        return False

    token = os.environ.get("GITHUB_TOKEN")
    if not token:
        print("ERROR: GITHUB_TOKEN environment variable not set.")
        print("Saving file locally instead...")
        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, "wb") as f:
            f.write(file_bytes)
        print(f"File saved locally: {filepath}")
        return False

    g = Github(token)
    repo = g.get_repo(f"{GITHUB_OWNER}/{GITHUB_REPO}")

    content_b64 = base64.b64encode(file_bytes).decode("utf-8")

    try:
        # Check if file exists
        existing = repo.get_contents(filepath, ref=GITHUB_BRANCH)
        repo.update_file(
            path=filepath,
            message=f"Update {FILENAME} - GPIO TestPlan",
            content=file_bytes,
            sha=existing.sha,
            branch=GITHUB_BRANCH
        )
        print(f"File updated on GitHub: {filepath}")
    except Exception:
        repo.create_file(
            path=filepath,
            message=f"Add {FILENAME} - GPIO TestPlan",
            content=file_bytes,
            branch=GITHUB_BRANCH
        )
        print(f"File created on GitHub: {filepath}")

    return True


def main():
    print("=" * 60)
    print("GPIO TestPlan Excel Generator - Fallback Automation")
    print("=" * 60)
    print(f"Timestamp (IST): {now_ist.strftime('%Y-%m-%d %H:%M:%S %Z')}")
    print(f"Output filename:  {FILENAME}")
    print(f"Output path:      {OUTPUT_PATH}")
    print()

    # Step 1: Create workbook
    print("[1/3] Creating workbook...")
    wb = create_workbook()
    print("      - TestPlan sheet: 2 data rows, 14 columns")
    print("      - MetaData sheet: 2 data rows, 11 columns (veryHidden)")

    # Step 2: Save to bytes
    print("[2/3] Saving workbook to binary .xlsx...")
    file_bytes = save_and_get_bytes(wb)
    print(f"      - File size: {len(file_bytes)} bytes")

    # Step 3: Push to GitHub
    print("[3/3] Pushing to GitHub...")
    success = push_to_github(file_bytes, OUTPUT_PATH)

    print()
    print("=" * 60)
    if success:
        print("STATUS: SUCCESS")
        print(f"GitHub URL: https://github.com/{GITHUB_OWNER}/{GITHUB_REPO}/blob/{GITHUB_BRANCH}/{OUTPUT_PATH}")
    else:
        print("STATUS: PARTIAL - File saved locally (GitHub push requires GITHUB_TOKEN)")
        print(f"Local path: {OUTPUT_PATH}")
    print("=" * 60)


if __name__ == "__main__":
    main()
