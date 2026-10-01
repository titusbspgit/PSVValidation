#!/usr/bin/env python3
"""Generate Ethernet TestPlan Excel workbook using openpyxl."""
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    import subprocess
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl'])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

# ============================================================
# DATA
# ============================================================
json_data = [
    {
        "Index": "1",
        "SS / Module": "ETHERNET0",
        "Test Case Name": "ethernet0_reg_wr_rd_test",
        "Feature": "Register Write Read",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_define.c"; <test_common.h>; <ethernet0/ethernet0_def.h>; <ethernet0/ethernet0_offset.h>',
        "Meta Macros": "#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n#define CNT 434",
        "Meta Arrays": """const unsigned long int addr_array[434] = {
 mizar_ETHERNET0_MAC_CONFIGURATION,
 mizar_ETHERNET0_MAC_EXT_CONFIGURATION,
 mizar_ETHERNET0_MAC_PACKET_FILTER,
 mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,
 mizar_ETHERNET0_MAC_HASH_TABLE_REG0,
};

const int default_value_array[434] = {
 ETHERNET0_MAC_CONFIGURATION_DEFAULT_VAL,
 ETHERNET0_MAC_EXT_CONFIGURATION_DEFAULT_VAL,
 ETHERNET0_MAC_PACKET_FILTER_DEFAULT_VAL,
 ETHERNET0_MAC_WD_JB_TIMEOUT_DEFAULT_VAL,
 ETHERNET0_MAC_HASH_TABLE_REG0_DEFAULT_VAL,
};

const int read_mask_array[434] = {
 ETHERNET0_MAC_CONFIGURATION_READ_MASK,
 ETHERNET0_MAC_EXT_CONFIGURATION_READ_MASK,
 ETHERNET0_MAC_PACKET_FILTER_READ_MASK,
 ETHERNET0_MAC_WD_JB_TIMEOUT_READ_MASK,
 ETHERNET0_MAC_HASH_TABLE_REG0_READ_MASK,
};

const int write_mask_array[434] = {
 ETHERNET0_MAC_CONFIGURATION_WRITE_MASK,
 ETHERNET0_MAC_EXT_CONFIGURATION_WRITE_MASK,
 ETHERNET0_MAC_PACKET_FILTER_WRITE_MASK,
 ETHERNET0_MAC_WD_JB_TIMEOUT_WRITE_MASK,
 ETHERNET0_MAC_HASH_TABLE_REG0_WRITE_MASK,
};

const int skip_array[434] = {0, 0, 0, 0, 0,};

const int skip_rst_array[434] = {0, 0, 0, 0, 0,};

int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};""",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": """This testcase performs a register write/read verification for the ETHERNET0 IP block. It operates in two phases:

Phase 1 (chk_rst_val): Iterates over all CNT (434) register addresses defined in addr_array. For each register, it checks if the register is readable (read_mask_array[i] != 0x00000000) and not in the skip_rst_array (skip_rst_array[i] != 1). If readable and not skipped, it reads the register using read_reg(addr) and compares the read value against the corresponding entry in default_value_array[i]. If mismatch, def_fail_cnt is incremented and a failure message is printed.

Phase 2 (chk_rd_wr): Iterates over 6 test patterns defined in chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}. For each test pattern (outer loop j=0..5), it performs:
 - Write pass: For each register i=0..CNT-1, if skip_array[i] != 1 and write_mask_array[i] != 0x00000000, writes data_wr (current chk_val[j]) to addr_array[i] using write_reg(addr, data_wr).
 - Read-back pass: For each register i=0..CNT-1, if skip_array[i] != 1 and write_mask_array[i] != 0x00000000 and read_mask_array[i] != 0x00000000, reads back using read_reg(addr). Computes expected value as: wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Compares data_rd against exp_val. If mismatch, wr_fail_cnt is incremented.

After both phases, if def_fail_cnt > 0 or wr_fail_cnt > 0, calls finish(1) indicating failure. Otherwise calls finish(0) indicating pass.

Note: soft_reset_chk() is commented out in test_case() and is not executed. The function soft_reset_chk() reads SOFT_RST_REG_ADDRESS (0x00000000), writes SOFT_RST_REG_DATA (0x00000000) to it, waits 1000 cycles via wait_on(1000), writes back the original default_value, and waits another 1000 cycles.

Global variables: int data_rd, data_wr (declared in both program.c and test_define.c); int def_fail_cnt = 0, wr_fail_cnt = 0.

The arrays addr_array, default_value_array, read_mask_array, write_mask_array are declared with size 434 but only 5 explicit initializer values are provided in test_define.c. The remaining 429 entries are implicitly zero-initialized by C. skip_array and skip_rst_array are similarly declared with 434 entries, all explicitly initialized to 0 for the first 5 entries and implicitly zero for the rest.""",
        "Test Description": "Verify ETHERNET0 register default values and write/read accessibility. Phase 1 reads all registers and validates their reset default values. Phase 2 writes six distinct test patterns to all writable registers and reads them back, validating the read data against expected values computed using write masks, read masks, and default values. The test passes if all default value checks and all write/read checks succeed.",
        "Meta Test Steps / Procedure": """1. Global variable declarations: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0;
2. Arrays are initialized from test_define.c: addr_array[434], default_value_array[434], read_mask_array[434], write_mask_array[434], skip_array[434], skip_rst_array[434] with 5 explicit initializer values each (remaining 429 entries zero-initialized).
3. test_case() entry point is called.
4. Call chk_rst_val().
5. chk_rst_val() internal flow:
 5a. Loop i = 0 to CNT-1 (i.e., 0 to 433):
 5b. addr = addr_array[i];
 5c. If read_mask_array[i] == 0x00000000, skip this register (continue). Print debug message if DEBUG_DISPLAY defined.
 5d. If skip_rst_array[i] == 1, skip this register (continue). Print debug message if DEBUG_DISPLAY defined.
 5e. data_rd = read_reg(addr);
 5f. If data_rd == default_value_array[i], PASS. Print debug message if DEBUG_DISPLAY defined.
 5g. Else, def_fail_cnt++. Print failure message: \"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x Read_data : 0x%x\".
6. After chk_rst_val() returns, print debug message \"********* Default value check end ***\" if DEBUG_DISPLAY defined.
7. Call chk_rd_wr().
8. chk_rd_wr() internal flow:
 8a. Local variable declarations: int i, j, exp_val, wr_n; unsigned long int addr;
 8b. int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};
 8c. Outer loop j = 0 to 5 (6 iterations, one per test pattern):
 8d. data_wr = chk_val[j];
 8e. Write pass inner loop i = 0 to CNT-1 (0 to 433):
 8f. addr = addr_array[i];
 8g. If skip_array[i] == 1, skip (continue). Print debug message if DEBUG_DISPLAY defined.
 8h. If write_mask_array[i] == 0x00000000, skip (continue). Print debug message if DEBUG_DISPLAY defined.
 8i. Else, write_reg(addr, data_wr). Print debug message if DEBUG_DISPLAY defined.
 8j. Read-back pass inner loop i = 0 to CNT-1 (0 to 433):
 8k. addr = addr_array[i];
 8l. If skip_array[i] == 1, skip (continue). Print debug message if DEBUG_DISPLAY defined.
 8m. If write_mask_array[i] == 0x00000000, skip (continue). Print debug message if DEBUG_DISPLAY defined.
 8n. If read_mask_array[i] == 0x00000000, skip (continue). Print debug message if DEBUG_DISPLAY defined.
 8o. Else:
 8p. data_rd = read_reg(addr);
 8q. wr_n = (write_mask_array[i] ^ 0xffffffff);
 8r. exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]));
 8s. If data_rd == exp_val, PASS. Print debug message if DEBUG_DISPLAY defined.
 8t. Else, wr_fail_cnt++. Print failure message: \"Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x Read value=0x%x\".
9. After chk_rd_wr() returns, print debug message \" Write & Read from registers end ************\" if DEBUG_DISPLAY defined.
10. Note: soft_reset_chk() call is commented out in test_case(). The function exists but is not invoked. Its implementation: default_value = read_reg(SOFT_RST_REG_ADDRESS); write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA); wait_on(1000); write_reg(SOFT_RST_REG_ADDRESS, default_value); wait_on(1000).
11. Check final result: if (def_fail_cnt > 0 || wr_fail_cnt > 0), call finish(1) (test FAIL).
12. Else, call finish(0) (test PASS).""",
        "Test Steps / Procedure": """1. Initialize global failure counters (default-value fail count and write/read fail count) to zero.
2. Initialize register address, default value, read mask, write mask, skip, and skip-reset arrays for all 434 registers.
3. Execute default value check phase: For each register in the address array, skip if not readable or marked for skip. Read the register and compare against its expected default value. Increment failure counter on mismatch.
4. Execute write/read check phase: For each of six test patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xFFFF0000), write the pattern to all writable registers, then read back each register and compare against the expected value computed from the written data, read mask, write mask, and default value. Increment failure counter on mismatch.
5. Evaluate overall result: If any failure counter is non-zero, report test failure. Otherwise, report test pass.""",
        "Meta Impacted Registers": "mizar_ETHERNET0_MAC_CONFIGURATION; mizar_ETHERNET0_MAC_EXT_CONFIGURATION; mizar_ETHERNET0_MAC_PACKET_FILTER; mizar_ETHERNET0_MAC_WD_JB_TIMEOUT; mizar_ETHERNET0_MAC_HASH_TABLE_REG0; SOFT_RST_REG_ADDRESS",
        "Impacted Registers": "MAC_Configuration; MAC_Ext_Configuration; MAC_Packet_Filter; MAC_WD_JB_Timeout; MAC_Hash_Table_Reg0",
        "Meta Validation / Acceptance Criteria": """Phase 1 (Default Value Check):
For each register i (0 to CNT-1) where read_mask_array[i] != 0x00000000 and skip_rst_array[i] != 1:
 - Read data_rd = read_reg(addr_array[i]).
 - Compare: data_rd == default_value_array[i].
 - If mismatch: def_fail_cnt++ and print failure with address, expected value (default_value_array[i]), and actual read value (data_rd).

Phase 2 (Write/Read Check):
For each test pattern j (0 to 5) from chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}:
 For each register i (0 to CNT-1) where skip_array[i] != 1 and write_mask_array[i] != 0x00000000 and read_mask_array[i] != 0x00000000:
 - Write data_wr = chk_val[j] to addr_array[i] via write_reg(addr, data_wr).
 - Read data_rd = read_reg(addr_array[i]).
 - Compute wr_n = (write_mask_array[i] ^ 0xffffffff).
 - Compute exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])).
 - Compare: data_rd == exp_val.
 - If mismatch: wr_fail_cnt++ and print failure with address, expected value (exp_val), and actual read value (data_rd).

Final Pass/Fail:
 - If (def_fail_cnt > 0 || wr_fail_cnt > 0): finish(1) \u2014 test FAIL.
 - Else: finish(0) \u2014 test PASS.""",
        "Validation / Acceptance Criteria": """1. All registers must return their expected default values after reset. Any mismatch in the default value check causes a test failure.
2. For each of six test patterns (all-ones, 0xAAAAAAAA, 0x55555555, all-zeros, 0xA5A5A5A5, 0xFFFF0000), all writable and readable registers must return the expected value after write, computed using the register's read mask, write mask, and default value. Any mismatch causes a test failure.
3. The test passes only if both the default value check and the write/read check complete with zero failures.""",
        "Remarks": """The soft_reset_chk() function is defined in program.c but its call is commented out in test_case(), so it is not executed during this test. The arrays are declared with 434 entries but only 5 explicit initializer values are provided in test_define.c; the remaining 429 entries are implicitly zero-initialized by C. The actual register addresses, default values, read masks, and write masks are resolved via macros from external headers ethernet0_def.h and ethernet0_offset.h which are not local to this testcase folder. SOFT_RST_REG_ADDRESS maps to address 0x00000000 and has no matched register name in Agent 4 (status: unresolved)."""
    }
]

# ============================================================
# COLUMN DEFINITIONS
# ============================================================
testplan_columns = [
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
    "Code Generation"
]

metadata_columns = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays"
]

# ============================================================
# GENERATE WORKBOOK
# ============================================================
def generate_workbook():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"Ethernet_TestPlan_{timestamp}.xlsx"
    output_dir = "Test_Output/Ethernet/Ethernet0/TestPlan"
    filepath = os.path.join(output_dir, filename)

    os.makedirs(output_dir, exist_ok=True)

    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_alignment = Alignment(wrap_text=True, vertical="top")

    # Write headers
    for col_idx, col_name in enumerate(testplan_columns, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    # Write data rows
    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(testplan_columns, 1):
            value = row_data.get(col_name, "")
            if value is None:
                value = ""
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # Auto-size columns
    MAX_WIDTH = 80
    for col_idx, col_name in enumerate(testplan_columns, 1):
        max_len = len(col_name)
        for row in ws_tp.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
        adjusted_width = min(max_len + 4, MAX_WIDTH)
        ws_tp.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet(title="MetaData")

    for col_idx, col_name in enumerate(metadata_columns, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(metadata_columns, 1):
            value = row_data.get(col_name, "")
            if value is None:
                value = ""
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

    ws_md.freeze_panes = "A2"

    for col_idx, col_name in enumerate(metadata_columns, 1):
        max_len = len(col_name)
        for row in ws_md.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
        adjusted_width = min(max_len + 4, MAX_WIDTH)
        ws_md.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save workbook
    wb.save(filepath)
    print(f"Workbook saved: {filepath}")
    print(f"Filename: {filename}")

    # ---- Validation ----
    assert os.path.exists(filepath), "File does not exist!"
    file_size = os.path.getsize(filepath)
    assert file_size > 0, "File size is 0!"
    print(f"File size: {file_size} bytes")

    wb_check = load_workbook(filepath)
    assert "TestPlan" in wb_check.sheetnames, "TestPlan sheet missing!"
    assert "MetaData" in wb_check.sheetnames, "MetaData sheet missing!"
    tp_rows = wb_check["TestPlan"].max_row - 1  # exclude header
    md_rows = wb_check["MetaData"].max_row - 1
    print(f"TestPlan rows: {tp_rows}")
    print(f"MetaData rows: {md_rows}")
    print(f"MetaData sheet_state: {wb_check['MetaData'].sheet_state}")
    print("VALIDATION: PASSED")
    wb_check.close()

    return filepath, filename, tp_rows, md_rows

if __name__ == "__main__":
    filepath, filename, tp_rows, md_rows = generate_workbook()
    print(f"\nSUCCESS: {filepath}")
