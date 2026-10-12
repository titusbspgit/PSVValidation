#!/usr/bin/env python3
"""MIPI CSI TestPlan XLSX Generator - Agent 7 Fallback Automation
Generates a real Office Open XML (.xlsx) workbook using openpyxl.
Run: pip install openpyxl && python generate_mipi_csi_testplan_20261010_v5.py
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

# ============ INPUT DATA ============
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI CSI2 Subsystem",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lanes Test - Multi-lane CSI-2 data transfer with varying lane configurations",
        "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"",
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#ifdef DPHY_LANES_TEST\n #ifndef LS_LE_EN\n #define LS_LE_EN\t\t\t1\n #endif\n #ifndef TOTAL_FRAME\n #define TOTAL_FRAME \t\t1\n #endif\n #ifndef VC_ID\n #define VC_ID \t\t\t3\n #endif\n #ifndef VRES\n #ifdef GDMA0_FULL_MEM\n #define VRES \t\t\t1080\n #else\n #define VRES \t\t\t3\n #endif\n #endif\n #ifndef HRES\n #ifdef GDMA0_FULL_MEM\n #define HRES \t\t\t1920\n #else\n #define HRES \t\t\t64\n #endif\n #endif\n #ifndef DATA_TYPE\n #define DATA_TYPE \t\tCSI2_RGB888\n #endif\n#endif //DPHY_LANES_TEST",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "The testcase verifies MIPI CSI-2 DPHY lane operation by iterating through lane configurations from 4 lanes (lane_num=3) down to 1 lane (lane_num=0). For each lane configuration: (1) CSI-2 interrupts are enabled via csi2_enable_interrupt() which reads INT_ST_MAIN to clear interrupts and writes mask registers for PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, and ECC_CORRECTED. (2) Virtual channel register and control data register are configured. (3) D-PHY initialization is performed via snps_phy_init(). (4) PHY_STOPSTATE is polled until value 0x1000f is reached. (5) N_LANES register is configured with the current lane_num. (6) A write to address 0xa0243ffc triggers the CSI-2 sequence with (lane_num+1). (7) For each control packet (cntrl_pkt_cnt = (VRES3)+2 iterations), DMA channel 0 transfers control data from src 0x8000 to dest 0xE6001000 (8 bytes), polling DMA interrupt status for completion. (8) Control data is read from 0xE6001000 and if data type > 0xf, word_count is extracted, csi_data_size is computed with 8-byte alignment, and DMA channel 1 transfers CSI data from src 0x0000 to dest 0xE6002000. (9) DMA interrupts are cleared after each transfer. (10) Test completes via finish(0).",
        "Test Description": "Verify MIPI CSI-2 DPHY lane operation by iterating through all lane configurations (4 lanes down to 1 lane). For each configuration, enable CSI-2 interrupts, configure virtual channel and control data registers, initialize D-PHY, wait for PHY stop state, configure N_LANES register, trigger CSI-2 sequence, and perform DMA-based control and data packet transfers with interrupt-driven completion polling. Validate successful transfer across all lane configurations.",
        "Meta Test Steps / Procedure": "1. Call csi2_enable_interrupt():\n 1a. int rd_data;\n 1b. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);\n 1c. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);\n 1d. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);\n 1e. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);\n 1f. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);\n 1g. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);\n 1h. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);\n 1i. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);\n 1j. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);\n 1k. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);\n 1l. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);\n2. Conditional GDMA path selection:\n #ifdef GDMA3_PATH: vcid_csi2_wrap_reg = VC_ID; gdma_path = 3;\n #elif GDMA2_PATH: vcid_csi2_wrap_reg = (VC_ID << 4); gdma_path = 2;\n #elif GDMA1_PATH: vcid_csi2_wrap_reg = (VC_ID << 8); gdma_path = 1;\n #elif GDMA0_PATH: vcid_csi2_wrap_reg = (VC_ID << 12); printf(\"VC_ID=%d\\n\",VC_ID); gdma_path = 0;\n3. gdma_reg_base = 0xE6A00000;\n4. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);\n5. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);\n6. printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg);\n7. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);\n8. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);\n9. Call snps_phy_init();\n10. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);\n11. while(!(rd_data == 0x1000f)) { rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE); }\n12. ch0_pc = 0xE6000000;\n13. ch1_pc = 0xE6000500;\n14. for(int lane_num=3; lane_num>=0; lane_num--):\n 14a. write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num);\n 14b. cntrl_pkt_cnt = ((VRES3) + 2);\n 14c. printf(\"DEBUG: cntrl_pkt_cnt=%d\\n\", cntrl_pkt_cnt);\n 14d. write_reg(0xa0243ffc, (lane_num+1));\n 14e. for(int i=0; i< cntrl_pkt_cnt; i++):\n 14e-i. ch0_preload_loc = ch0_pc;\n 14e-ii. ch1_preload_loc = ch1_pc;\n 14e-iii. write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET, 0x3);\n 14e-iv. dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0);\n 14e-v. DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0);\n 14e-vi. rd_data = 0;\n 14e-vii. while((rd_data & 0x1) == 0x0) { rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET); printf(\"polling irq; irq_status_reg rd_data =%0x\\n\",rd_data); }\n 14e-viii. write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);\n 14e-ix. rd_data = read_reg(gdma_reg_base + 0x28);\n 14e-x. printf(\"DEBUG : irq polling completed ch0; rd_data = %0x \\n\", rd_data);\n 14e-xi. csi_ctrl_data = read_reg(0xE6001000);\n 14e-xii. printf(\"DEBUG: csi_ctrl_data=%0x\\n\", csi_ctrl_data);\n 14e-xiii. if((csi_ctrl_data & 0x3f) > 0xf):\n 14e-xiii-a. word_count = ((csi_ctrl_data >> 6)&0xffff);\n 14e-xiii-b. csi_data_size = (word_count%8) ? (word_count/8 + 1 )* 8 : word_count;\n 14e-xiii-c. dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1);\n 14e-xiii-d. DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1);\n 14e-xiii-e. rd_data = 0;\n 14e-xiii-f. while((rd_data & 0x2) == 0x0) { rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET); printf(\"polling irq; irq_status_reg rd_data =%0x\\n\",rd_data); }\n 14e-xiii-g. rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);\n 14e-xiii-h. write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2);\n 14e-xiii-i. printf(\"DEBUG : irq polling completed ch1; rd_data=%0x\\n\", rd_data);\n15. finish(0);",
        "Test Steps / Procedure": "1. Enable CSI-2 interrupts by reading INT_ST_MAIN to clear pending interrupts, then writing interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED) with appropriate enable values.\n2. Configure virtual channel register and enable control data transfer.\n3. Initialize D-PHY via snps_phy_init().\n4. Poll PHY_STOPSTATE register until all lanes and clock lane reach stop state (value 0x1000f).\n5. For each lane configuration (4 lanes down to 1 lane):\n a. Write N_LANES register with lane count.\n b. Trigger CSI-2 sequence.\n c. For each control packet in the frame:\n i. Enable DMA interrupts, preload DMA channel 0 for control data transfer (8 bytes), issue DMAGO, poll DMA interrupt status for channel 0 completion, clear DMA interrupt.\n ii. Read control data, extract data type. If data type > 0xf, compute word count and aligned data size, preload DMA channel 1 for CSI data transfer, issue DMAGO, poll DMA interrupt status for channel 1 completion, clear DMA interrupt.\n6. Complete test via finish(0).",
        "Meta Impacted Registers": "virtual_channel, control_data, PHY_STOPSTATE, N_LANES, INT_ST_MAIN, INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "virtual_channel, control_data, PHY_STOPSTATE, N_LANES, INT_ST_MAIN, INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE polling: rd_data == 0x1000f (all 4 data lanes and clock lane in stop state: bits [3:0] = 0xf for data lanes, bit [16] = 0x1 for clock lane).\n2. DMA channel 0 completion: (rd_data & 0x1) != 0x0 from read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET).\n3. Control data type check: (csi_ctrl_data & 0x3f) > 0xf determines if CSI data transfer is needed.\n4. DMA channel 1 completion: (rd_data & 0x2) != 0x0 from read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET).\n5. CSI data size alignment: csi_data_size = (word_count%8) ? (word_count/8 + 1)*8 : word_count.\n6. Test completion: finish(0) called indicating pass.",
        "Validation / Acceptance Criteria": "1. PHY_STOPSTATE register must read 0x1000f indicating all data lanes and clock lane are in stop state before proceeding.\n2. DMA channel 0 interrupt status bit 0 must be set after each control data transfer completes.\n3. If control data type field exceeds 0xf, DMA channel 1 interrupt status bit 1 must be set after CSI data transfer completes.\n4. CSI data transfer size must be 8-byte aligned.\n5. Test must iterate through all 4 lane configurations (4, 3, 2, 1 lanes) successfully.\n6. Test completes with finish(0) indicating pass.",
        "Remarks": "The testcase writes MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL and MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA twice (steps 4-5 and 7-8). Address 0xa0243ffc and 0xE6001000 are unresolved in Agent 4 mappings. The GDMA path selection is compile-time conditional (#ifdef GDMA3_PATH / GDMA2_PATH / GDMA1_PATH / GDMA0_PATH). The global variable gdma_int_rsts is declared twice in source. The function snps_phy_init() is called but not defined in this source file. The functions dma_trnsfr_instn_preload() and DMAGO_CSI() are called but not defined in this source file. The variable word_count is declared as long int globally and used locally."
    }
]

# ============ CONFIGURATION ============
IP_NAME = "MIPI_CSI"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"{IP_NAME}_TestPlan_{timestamp}.xlsx"
output_path = os.path.join(OUTPUT_DIR, filename)

# ============ SHEET DEFINITIONS ============
TESTPLAN_COLUMNS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

METADATA_COLUMNS = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# ============ FORMATTING ============
HEADER_FONT = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
HEADER_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
HEADER_ALIGN = Alignment(horizontal="center", vertical="center", wrap_text=True)
CELL_ALIGN = Alignment(vertical="top", wrap_text=True)
MAX_COL_WIDTH = 60
MIN_COL_WIDTH = 12

def auto_size_columns(ws, max_width=MAX_COL_WIDTH, min_width=MIN_COL_WIDTH):
    for col_idx in range(1, ws.max_column + 1):
        max_len = min_width
        col_letter = get_column_letter(col_idx)
        for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split("\n")
                    for line in lines:
                        max_len = max(max_len, len(line) + 2)
        ws.column_dimensions[col_letter].width = min(max_len, max_width)

def apply_formatting(ws):
    # Header formatting
    for cell in ws[1]:
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGN
    # Data cell formatting
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.alignment = CELL_ALIGN
    # Freeze first row
    ws.freeze_panes = "A2"
    # Auto-size
    auto_size_columns(ws)

def create_workbook():
    wb = Workbook()
    
    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_tp.append(TESTPLAN_COLUMNS)
    
    for row_data in json_data:
        row = []
        for col in TESTPLAN_COLUMNS:
            if col == "Code Generation":
                row.append(row_data.get(col, ""))
            else:
                row.append(row_data.get(col, ""))
        ws_tp.append(row)
    
    apply_formatting(ws_tp)
    
    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet(title="MetaData")
    ws_md.append(METADATA_COLUMNS)
    
    for row_data in json_data:
        row = []
        for col in METADATA_COLUMNS:
            row.append(row_data.get(col, ""))
        ws_md.append(row)
    
    apply_formatting(ws_md)
    
    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"
    
    return wb

def validate_workbook(filepath):
    results = {}
    results["file_exists"] = os.path.exists(filepath)
    results["file_size"] = os.path.getsize(filepath) if results["file_exists"] else 0
    results["size_gt_zero"] = results["file_size"] > 0
    
    try:
        wb_check = load_workbook(filepath)
        results["opens_successfully"] = True
        results["has_testplan"] = "TestPlan" in wb_check.sheetnames
        results["has_metadata"] = "MetaData" in wb_check.sheetnames
        results["testplan_rows"] = wb_check["TestPlan"].max_row - 1 if results["has_testplan"] else 0
        results["metadata_rows"] = wb_check["MetaData"].max_row - 1 if results["has_metadata"] else 0
        wb_check.close()
    except Exception as e:
        results["opens_successfully"] = False
        results["error"] = str(e)
    
    all_pass = all([
        results.get("file_exists"),
        results.get("size_gt_zero"),
        results.get("opens_successfully"),
        results.get("has_testplan"),
        results.get("has_metadata")
    ])
    results["validation"] = "PASSED" if all_pass else "FAILED"
    return results

if __name__ == "__main__":
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    print(f"Creating workbook: {filename}")
    wb = create_workbook()
    
    # Save locally
    local_path = filename
    wb.save(local_path)
    wb.close()
    print(f"Saved: {local_path}")
    
    # Validate
    results = validate_workbook(local_path)
    print(f"Validation: {json.dumps(results, indent=2)}")
    
    # Output for CI/CD
    print(f"::set-output name=filename::{filename}")
    print(f"::set-output name=validation::{results['validation']}")
    print(f"::set-output name=rows_testplan::{results.get('testplan_rows', 0)}")
    print(f"::set-output name=rows_metadata::{results.get('metadata_rows', 0)}")
    
    # Also save to output dir if different
    if OUTPUT_DIR != ".":
        import shutil
        dest = os.path.join(OUTPUT_DIR, filename)
        shutil.copy2(local_path, dest)
        print(f"Copied to: {dest}")
    
    print("DONE")
