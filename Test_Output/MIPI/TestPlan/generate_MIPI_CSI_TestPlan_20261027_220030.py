#!/usr/bin/env python3
"""
Agent 7: Excel Generation and GitHub Publishing
Generates MIPI_CSI_TestPlan_20261027_220030.xlsx

Usage:
  pip install openpyxl
  export GITHUB_TOKEN=<your_token>
  python generate_MIPI_CSI_TestPlan_20261027_220030.py

Produces a genuine .xlsx workbook with:
  - TestPlan sheet (14 columns, 1 data row, frozen header, blue headers, wrap text, Code Generation dropdown)
  - MetaData sheet (9 columns, 1 data row, frozen header, blue headers, wrap text, veryHidden)
Then uploads the binary .xlsx to GitHub and prints the status JSON.
"""
import json, os, sys, base64, urllib.request, urllib.error

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
    from openpyxl.utils import get_column_letter
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "openpyxl"])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
    from openpyxl.utils import get_column_letter

# ============================================================
# CONFIGURATION
# ============================================================
IP_NAME = "MIPI_CSI"
FILENAME = "MIPI_CSI_TestPlan_20261027_220030.xlsx"
GITHUB_OWNER = "titusbspgit"
GITHUB_REPO = "PSVValidation"
GITHUB_BRANCH = "main"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"

# ============================================================
# TESTPLAN DATA (from Agent 6B)
# ============================================================
TESTPLAN_JSON = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Feature": "Register Read/Write Verification",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This test case verifies the register read/write functionality of the MIPI CSI-2 Receiver Block (RB). The test targets 60 registers spanning control/configuration, interrupt, DPHY, DMA address, and boot register categories. The test performs two primary validations: (1) Default/Reset Value Check - reads all 60 registers after reset and verifies that each register contains its expected default value using the applicable read mask; (2) Write/Read-back Check - writes a known data pattern to each writable register (skipping read-only registers and explicitly excluded registers), then reads back and verifies the written value using the applicable write and read masks. The test tracks default value mismatches and write/read-back mismatches separately to identify specific register failures.",
    "Test Steps / Procedure": "Step 1: Execute the default/reset value verification routine. For each of the 60 MIPI CSI-2 RB registers, read the current register value, apply the corresponding read mask, and compare the masked read value against the expected masked default value. Record any mismatches in the default value failure counter.\n\nStep 2: Execute the write and read-back verification routine. Iterate through all 60 registers. For each register, check if the register is in the skip list or if the register has a zero write mask (indicating it is not writable). If either condition is true, skip that register. Otherwise, write a known test data pattern to the register.\n\nStep 3: After writing to all applicable registers, iterate through all 60 registers again. For each register not in the skip list and not having a zero write mask, read back the register value, apply the appropriate mask, and compare the masked read-back value against the expected masked written value. Record any mismatches in the write failure counter.\n\nStep 4: Evaluate the default value failure counter and write failure counter. The test passes if both counters remain at zero, indicating all registers contain correct default values and all writable registers correctly accept and retain written data.",
    "Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0, boot_addr_VC1, boot_addr_VC2, boot_addr_VC3, boot_irq_ns_VC0, boot_irq_ns_VC1, boot_irq_ns_VC2, boot_irq_ns_VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m1_addr_ar_ch0_data, dma_m1_addr_ar_ch0_Instruction, dma_m2_addr_ar_ch0_data, dma_m2_addr_ar_ch0_Instruction, dma_m3_addr_ar_ch0_data, dma_m3_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, dma_m1_addr_aw_ch0_data, dma_m1_addr_aw_ch0_Instruction, dma_m2_addr_aw_ch0_data, dma_m2_addr_aw_ch0_Instruction, dma_m3_addr_aw_ch0_data, dma_m3_addr_aw_ch0_Instruction, dma_m0_addr_ar_ch1_data, dma_m0_addr_ar_ch1_Instruction, dma_m1_addr_ar_ch1_data, dma_m1_addr_ar_ch1_Instruction, dma_m2_addr_ar_ch1_data, dma_m2_addr_ar_ch1_Instruction, dma_m3_addr_ar_ch1_data, dma_m3_addr_ar_ch1_Instruction, dma_m0_addr_aw_ch1_data, dma_m0_addr_aw_ch1_Instruction, dma_m1_addr_aw_ch1_data, dma_m1_addr_aw_ch1_Instruction, dma_m2_addr_aw_ch1_data, dma_m2_addr_aw_ch1_Instruction, dma_m3_addr_aw_ch1_data, dma_m3_addr_aw_ch1_Instruction, dphy_rx",
    "Validation / Acceptance Criteria": "1. Default Value Verification: For each of the 60 registers, the masked read value after reset must exactly match the corresponding masked expected default value. Any mismatch causes the default value failure counter to increment, indicating a test failure for that register.\n\n2. Write/Read-back Verification: For each writable register (registers not in the skip list and with a non-zero write mask), the masked read-back value after a write operation must exactly match the expected masked written value. Any mismatch causes the write failure counter to increment, indicating a test failure for that register.\n\n3. Skip Conditions: Registers explicitly marked in the skip list are excluded from write/read-back verification. Registers with a zero write mask (read-only registers) are excluded from write/read-back verification.\n\n4. Overall Pass Criteria: The test passes only when both the default value failure counter and the write failure counter are zero at the end of execution.",
    "Remarks": "The test covers 60 registers in the MIPI CSI-2 Receiver Block including control/configuration registers (virtual_channel, lane_clk, control_data, etc.), interrupt registers (interrupt_raw, interrupt_mask, interrupt_enable, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask), DPHY registers (dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dphy_rx), DMA address registers for channels 0 and 1 across masters 0-3 for both read and write paths (data and instruction), boot registers (dmac_boot, boot_addr_VC0-VC3, boot_irq_ns_VC0-VC3), and miscellaneous registers (soft_rst, null_blank, flush, mem, fifo_threshold_val, line_info). A soft reset check routine is also defined in the test but is currently disabled in the test execution flow."
  }
]

# ============================================================
# METADATA (from Agent 6A)
# ============================================================
META_JSON = [
  {
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Meta Headers": "SOURCE_FILE: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"\nEND_SOURCE_FILE\n\nSOURCE_FILE: test_define.c\n#include<mipi_csi2_rb_reg.h>\nEND_SOURCE_FILE",
    "Meta Macros": "SOURCE_FILE: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA\nNote: SOFT_RST_REG_DATA appears in the supplied source with no visible replacement text. Its value may be defined in test_common.h which was not supplied.\nConditional compilation directive: #ifdef DEBUG_DISPLAY / #endif (used in test_case, chk_rd_wr write loop, and chk_rd_wr read loop)\nEND_SOURCE_FILE\n\nSOURCE_FILE: test_define.c\n#define CNT 60\nEND_SOURCE_FILE\n\nThe following macros are referenced in the supplied source arrays but defined in the external header mipi_csi2_rb_reg.h (not supplied):\n\nAddress macros (60, in addr_array[60]): MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, MIZAR_MIPI_CSI2_RB_REG_LINE_INFO, MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL, MIZAR_MIPI_CSI2_RB_REG_LANE_CLK, MIZAR_MIPI_CSI2_RB_REG_MEM, MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW, MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK, MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE, MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, MIZAR_MIPI_CSI2_RB_REG_FLUSH, MIZAR_MIPI_CSI2_RB_REG_FIFO_INTERRUPT_RAW, MIZAR_MIPI_CSI2_RB_REG_FIFO_INTERRUPT_ENABLE, MIZAR_MIPI_CSI2_RB_REG_FIFO_INTERRUPT_MASK, MIZAR_MIPI_CSI2_RB_REG_SOFT_RST, MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK, MIZAR_MIPI_CSI2_RB_REG_DMAC_BOOT, MIZAR_MIPI_CSI2_RB_REG_BOOT_ADDR_VC0, MIZAR_MIPI_CSI2_RB_REG_BOOT_ADDR_VC1, MIZAR_MIPI_CSI2_RB_REG_BOOT_ADDR_VC2, MIZAR_MIPI_CSI2_RB_REG_BOOT_ADDR_VC3, MIZAR_MIPI_CSI2_RB_REG_BOOT_IRQ_NS_VC0, MIZAR_MIPI_CSI2_RB_REG_BOOT_IRQ_NS_VC1, MIZAR_MIPI_CSI2_RB_REG_BOOT_IRQ_NS_VC2, MIZAR_MIPI_CSI2_RB_REG_BOOT_IRQ_NS_VC3, MIZAR_MIPI_CSI2_RB_REG_DPHY_RX_CFGCLK, MIZAR_MIPI_CSI2_RB_REG_DPHY_RX_HS, MIZAR_MIPI_CSI2_RB_REG_DPHY_IO_CONTINUITY, and 33 DMA address macros, MIZAR_MIPI_CSI2_RB_REG_DPHY_RX\n\nDefault value macros (60), Write mask macros (60), Read mask macros (60) - all corresponding to the 60 registers.",
    "Meta Arrays": "addr_array[60], default_val_array[60], write_mask_array[60], read_mask_array[60], skip_array[60]",
    "Meta Types": "unsigned int, int, void",
    "Meta Test Description": "This testcase (mipi_csi2_rb_reg_wr_rd_test) is a register read/write verification test for the MIPI CSI-2 Receiver Block (RB) on the Mizar platform. It operates on 60 registers (CNT=60) spanning control/configuration, interrupt, DPHY, DMA address, and boot registers. The test_case() function orchestrates two sub-steps: (1) chk_rst_val() reads each register and compares the read value (masked by read_mask_array) against the expected default value (default_val_array), tracking mismatches via def_fail_cnt; (2) chk_rd_wr() writes data_wr to each writable register (skipping entries in skip_array or with zero write_mask), then reads back and compares (masked) to verify write correctness, tracking mismatches via wr_fail_cnt.",
    "Meta Test Steps / Procedure": "Step 1: Call chk_rst_val() to verify default/reset values of all 60 registers. Step 2: Call chk_rd_wr() which writes data_wr to each writable register (skipping skip_array entries and zero write_mask registers), then reads back and compares masked values.",
    "Meta Impacted Registers": "60 unique registers: virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0-VC3, boot_irq_ns_VC0-VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0-m3_addr_ar/aw_ch0/ch1_data/Instruction, dphy_rx",
    "Meta Validation / Acceptance Criteria": "1. chk_rst_val(): Read each register, mask with read_mask_array, compare against default_val_array masked with read_mask_array. Mismatch increments def_fail_cnt. 2. chk_rd_wr(): Write data_wr to writable registers, read back, mask and compare. Mismatch increments wr_fail_cnt. Skip conditions: skip_array[i]==1 or write_mask_array[i]==0x00000000."
  }
]

# ============================================================
# SHEET COLUMNS
# ============================================================
TESTPLAN_COLUMNS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks",
    "Code Generation (Required / Not)"
]

META_COLUMNS = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

# ============================================================
# FORMATTING
# ============================================================
HEADER_FONT = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
HEADER_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
WRAP_ALIGNMENT = Alignment(wrap_text=True, vertical="top")

def apply_header_format(ws, columns):
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = WRAP_ALIGNMENT

def auto_size_columns(ws, max_width=60):
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            if cell.value:
                for line in str(cell.value).split('\n'):
                    max_len = max(max_len, len(line))
        ws.column_dimensions[col_letter].width = min(max(max_len + 2, 12), max_width)

# ============================================================
# MAIN
# ============================================================
def main():
    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    apply_header_format(ws_tp, TESTPLAN_COLUMNS)
    for row_idx, tc in enumerate(TESTPLAN_JSON, 2):
        for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
            val = "" if col_name == "Code Generation (Required / Not)" else tc.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
            cell.alignment = WRAP_ALIGNMENT

    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=False)
    dv.error = "Please select Required or Not Required"
    dv.errorTitle = "Invalid Entry"
    dv.prompt = "Select Code Generation status"
    dv.promptTitle = "Code Generation"
    col_letter = get_column_letter(len(TESTPLAN_COLUMNS))
    dv.sqref = f"{col_letter}2:{col_letter}{len(TESTPLAN_JSON) + 1}"
    ws_tp.add_data_validation(dv)
    ws_tp.freeze_panes = "A2"
    auto_size_columns(ws_tp)

    # ---- MetaData Sheet ----
    ws_meta = wb.create_sheet("MetaData")
    apply_header_format(ws_meta, META_COLUMNS)
    for row_idx, meta in enumerate(META_JSON, 2):
        for col_idx, col_name in enumerate(META_COLUMNS, 1):
            cell = ws_meta.cell(row=row_idx, column=col_idx, value=meta.get(col_name, ""))
            cell.alignment = WRAP_ALIGNMENT
    ws_meta.freeze_panes = "A2"
    auto_size_columns(ws_meta)
    ws_meta.sheet_state = "veryHidden"

    # ---- Save ----
    wb.save(FILENAME)
    file_size = os.path.getsize(FILENAME)
    print(f"Workbook saved: {FILENAME} ({file_size} bytes)")

    # ---- Validate ----
    wb2 = load_workbook(FILENAME)
    assert "TestPlan" in wb2.sheetnames
    assert "MetaData" in wb2.sheetnames
    tp_rows = wb2["TestPlan"].max_row - 1
    meta_rows = wb2["MetaData"].max_row - 1
    assert tp_rows == len(TESTPLAN_JSON)
    assert meta_rows == len(META_JSON)
    assert file_size > 0
    print(f"Validation PASSED: TestPlan={tp_rows}, MetaData={meta_rows}")

    # ---- Upload to GitHub ----
    token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    if not token:
        print("ERROR: GITHUB_TOKEN not set. Set it and re-run.")
        print(json.dumps({"status":"FAILURE","output_file_path":f"{OUTPUT_DIR}/{FILENAME}","github_url":"","filename":FILENAME,"rows_testplan":tp_rows,"rows_metadata":meta_rows,"commit_sha":"","validation":"PASSED"}, indent=2))
        sys.exit(1)

    with open(FILENAME, "rb") as f:
        content_b64 = base64.b64encode(f.read()).decode()

    api_url = f"https://api.github.com/repos/{GITHUB_OWNER}/{GITHUB_REPO}/contents/{OUTPUT_DIR}/{FILENAME}"
    headers = {"Authorization": f"token {token}", "Accept": "application/vnd.github.v3+json", "Content-Type": "application/json"}

    sha = None
    try:
        req = urllib.request.Request(api_url, headers=headers)
        sha = json.loads(urllib.request.urlopen(req).read().decode()).get("sha")
    except urllib.error.HTTPError:
        pass

    payload = {"message": "Generated TestPlan Excel Workbook", "content": content_b64, "branch": GITHUB_BRANCH}
    if sha:
        payload["sha"] = sha

    req = urllib.request.Request(api_url, data=json.dumps(payload).encode(), headers=headers, method="PUT")
    result = json.loads(urllib.request.urlopen(req).read().decode())

    print(json.dumps({
        "status": "SUCCESS",
        "output_file_path": f"{OUTPUT_DIR}/{FILENAME}",
        "github_url": result["content"]["html_url"],
        "filename": FILENAME,
        "rows_testplan": tp_rows,
        "rows_metadata": meta_rows,
        "commit_sha": result["commit"]["sha"],
        "validation": "PASSED"
    }, indent=2))

if __name__ == "__main__":
    main()
