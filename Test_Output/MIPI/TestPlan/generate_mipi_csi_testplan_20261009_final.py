#!/usr/bin/env python3
"""
MIPI_CSI TestPlan XLSX Generator - Agent 7 Direct Execution
Generates MIPI_CSI_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx and pushes to GitHub.
Run: python3 generate_mipi_csi_testplan_20261009_final.py
Triggered: auto-run via GitHub Actions
"""
import os, sys, base64, json, datetime

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
except ImportError:
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter

# IST timestamp
ist = datetime.timezone(datetime.timedelta(hours=5, minutes=30))
now = datetime.datetime.now(ist)
ts = now.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_CSI_TestPlan_{ts}.xlsx"

# JSON Data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "D-PHY Lane Configuration and CSI-2 Data Reception",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 1080 (if GDMA0_FULL_MEM defined), else 3\n#define HRES 1920 (if GDMA0_FULL_MEM defined), else 64\n#define DATA_TYPE CSI2_RGB888",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "0xE6000000; 0xE6000500; 0xE6001000; 0xE6002000; 0xE6040000; 0xE6040080",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through all lane counts (4 lanes down to 1 lane). For each lane configuration, the test performs CSI-2 interrupt enabling across all interrupt mask registers (PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, ECC_CORRECTED), configures the virtual channel register and control data register, initializes the SNPS D-PHY, polls PHY_STOPSTATE until stop state is achieved (value 0x1000f), then for each lane count programs the N_LANES register and triggers the CSI-2 sequence. For each control packet in the frame (cntrl_pkt_cnt = VRES*3 + 2), the test performs a DMA control packet transfer on channel 0 (8 bytes from src 0x8000 to dest 0xE6001000), polls DMA interrupt status for channel 0 completion, clears the DMA interrupt, reads the CSI control data from 0xE6001000, and if the data type field indicates image data (data_type > 0xf), calculates the word_count and csi_data_size (8-byte aligned), then performs a DMA CSI data transfer on channel 1 (from src 0x0000 to dest 0xE6002000 with calculated csi_data_size), polls DMA interrupt status for channel 1 completion, and clears the DMA interrupt. The test completes by calling finish(0).",
        "Test Description": "Verify MIPI CSI-2 D-PHY lane configuration by iterating from 4 lanes down to 1 lane. For each lane count, enable all CSI-2 host interrupt masks, configure the virtual channel and control data registers, initialize the D-PHY, wait for PHY stop state, then for each packet in the frame perform DMA-based control packet transfer and conditional CSI image data transfer with interrupt-driven completion polling. Validate DMA completion and CSI control data reception for each lane configuration.",
        "Meta Test Steps / Procedure": "1. Enter test_case().\n2. Declare local variables: rx_desc (long long int), tx_desc (long long int), gdma_tx_trnsfr_size (long long int), gdma_trnsfr_size (long long int), cntrl_pkt_cnt (int).\n3. printf(\"start line\\n\").\n4. Call csi2_enable_interrupt().\n5. [Inside csi2_enable_interrupt()] Declare local int rd_data.\n6. [Inside csi2_enable_interrupt()] rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) - Read INT_ST_MAIN to clear interrupts.\n7-16. Write enable masks to all interrupt mask registers.\n17. Return from csi2_enable_interrupt().\n18. Conditional compilation block for GDMA path selection.\n19. gdma_reg_base = 0xE6A00000.\n20-24. Configure virtual channel and control data registers (twice).\n25. Call snps_phy_init().\n26-27. Poll PHY_STOPSTATE until 0x1000f.\n28-29. Set DMA PC addresses.\n30-60. Lane loop with DMA transfers and polling.",
        "Test Steps / Procedure": "1. Enable all CSI-2 host interrupt masks by reading the main interrupt status register to clear pending interrupts, then writing enable values to PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt mask registers.\n2. Configure the virtual channel register with the computed VC ID value based on the selected GDMA path, and enable control data transfer by writing to the control data register. Repeat these writes a second time.\n3. Initialize the SNPS D-PHY by calling the PHY initialization routine.\n4. Poll the PHY stop state register until the D-PHY enters stop state (expected value indicating all lanes and clock in stop state).\n5. Set DMA program counter addresses for channel 0 and channel 1.\n6. For each lane configuration (4 lanes down to 1 lane):\n   a. Write the lane count to the N_LANES register.\n   b. Calculate the control packet count based on vertical resolution.\n   c. Trigger the CSI-2 sequence by writing the lane count to the trigger register.\n   d. For each control packet in the frame:\n      i. Enable DMA interrupts for both channels.\n      ii. Program DMA channel 0 for control packet transfer (8 bytes to control data destination).\n      iii. Start DMA channel 0 and poll the DMA interrupt status register until channel 0 transfer completes.\n      iv. Clear the DMA channel 0 interrupt.\n      v. Read the CSI control data from the destination address.\n      vi. If the data type indicates image data, calculate the aligned data size from the word count, program DMA channel 1 for CSI data transfer, start DMA channel 1, poll the DMA interrupt status register until channel 1 transfer completes, and clear the DMA channel 1 interrupt.\n7. Call finish to complete the test with pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000",
        "Impacted Registers": "INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED; virtual_channel; control_data; PHY_STOPSTATE; N_LANES",
        "Meta Validation / Acceptance Criteria": "1. PHY Stop State Validation: After snps_phy_init(), poll read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) until rd_data == 0x1000f.\n2. DMA Channel 0 Completion Polling: For each control packet transfer, poll read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) and check (rd_data & 0x1) != 0x0.\n3. CSI Control Data Type Check: After reading csi_ctrl_data = read_reg(0xE6001000), the condition (csi_ctrl_data & 0x3f) > 0xf determines whether the received packet is image data.\n4. Word Count Extraction: word_count = ((csi_ctrl_data >> 6) & 0xffff).\n5. Data Size Alignment: csi_data_size = (word_count % 8) ? (word_count / 8 + 1) * 8 : word_count.\n6. DMA Channel 1 Completion Polling: poll (rd_data & 0x2) != 0x0.\n7. Test Completion: finish(0) is called indicating test pass.",
        "Validation / Acceptance Criteria": "1. The D-PHY must enter stop state on all lanes and clock lane after initialization, confirmed by polling the PHY_STOPSTATE register.\n2. DMA channel 0 must complete each control packet transfer, confirmed by the DMA interrupt status register indicating channel 0 done.\n3. The CSI control data must be successfully read from the DMA destination, and the data type field must be correctly parsed to distinguish image data from short packets.\n4. For image data packets, DMA channel 1 must complete the CSI data transfer with the correctly aligned transfer size, confirmed by the DMA interrupt status register indicating channel 1 done.\n5. The test must iterate through all 4 lane configurations (4, 3, 2, 1 lanes) and process all control packets for each configuration.\n6. The test must complete successfully by calling finish with pass status (0).",
        "Remarks": "The test uses conditional compilation (GDMA0_PATH/GDMA1_PATH/GDMA2_PATH/GDMA3_PATH) to select the GDMA path and compute vcid_csi2_wrap_reg. VRES and HRES are also conditionally compiled based on GDMA0_FULL_MEM. The functions snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() are external and not defined within the testcase folder. The virtual channel and control data register writes occur twice in sequence (steps 20-21 and 23-24). Two hex addresses (0xa0243ffc for CSI-2 sequence trigger, 0xE6001000 for CSI control data read) could not be mapped to named registers in the specification documents."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "CSI-2 Internal Test Pattern Generator and DMA Data Capture",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "0xE6000000; 0xE6001000; 0xE6040000; 0xE6040080",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates the MIPI CSI-2 host controller's internal test pattern generator (PG) functionality and DMA-based data capture. The test configures the virtual channel register with a computed VC ID (vcid=3) based on the selected GDMA path, disables control data transfer, enables CSI-2 and DMA interrupts via csi2_subsys_enable_interrupt(), initializes the SNPS D-PHY via snps_phy_init(), and polls PHY_STOPSTATE until all lanes and clock are in stop state (value 0x1000f). The test then calculates the CSI-2 data transfer size based on hres=320, vres=16, valid_bits_per_pixel=24 with 8-byte alignment, programs the DMA higher-order address registers for channel 0, enables the fractional divider output, programs the DMA transfer, starts DMA channel 0, enables the pattern generator by writing PG_PATTERN_VRES=0x10, PG_PATTERN_HRES=0x70140, PG_CONFIG=0xe401, PG_ENABLE=1, waits 100 cycles, disables the pattern generator by writing PG_ENABLE=0, polls the DMA interrupt status register for channel 0 completion, waits 10000 cycles, and completes the test by calling finish(0).",
        "Test Description": "Verify the MIPI CSI-2 host controller's internal test pattern generator by configuring the virtual channel, enabling CSI-2 and DMA interrupts, initializing the D-PHY, waiting for PHY stop state, programming DMA channel 0 for CSI-2 data capture with computed transfer size based on resolution (320x16, 24bpp), enabling the pattern generator with specific vertical resolution, horizontal resolution, and configuration values, capturing the generated pattern data via DMA, then disabling the pattern generator and polling for DMA completion. Validate that the DMA transfer completes successfully.",
        "Meta Test Steps / Procedure": "1. Enter test_case().\n2. Declare local variables.\n3. int_pend = 1.\n4. printf(\"start line\\n\").\n5. vcid = 3.\n6. vcid_unselected_path = ((vcid + 1) & 0xf).\n7. Conditional compilation block for GDMA path selection.\n8. gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000).\n9-11. Configure virtual channel and control data registers.\n12-13. Enable interrupts and init PHY.\n14-15. Poll PHY_STOPSTATE until 0x1000f.\n16-19. Set resolution and calculate transfer size.\n20-24. Program DMA address registers and enable fracdiv.\n25-29. Start DMA transfer.\n30-35. Enable pattern generator via csi2_ctrlr_pg_enable().\n36-37. Wait and disable pattern generator.\n38-39. Poll DMA completion.\n40-41. Wait and finish.",
        "Test Steps / Procedure": "1. Set interrupt pending flag and initialize virtual channel ID (vcid=3) and compute the unselected path VC ID.\n2. Select the GDMA path based on conditional compilation and compute the virtual channel wrapper register value accordingly.\n3. Calculate the GDMA register base address from the selected path.\n4. Write the virtual channel register with the computed VC ID mapping.\n5. Disable control data transfer by writing 0 to the control data register.\n6. Enable CSI-2 subsystem and DMA interrupts.\n7. Initialize the SNPS D-PHY.\n8. Poll the PHY stop state register until all data lanes and clock lane enter stop state.\n9. Set resolution parameters: hres=320, vres=16, valid_bits_per_pixel=24.\n10. Calculate the total CSI-2 data transfer size with 8-byte alignment.\n11. Program the DMA higher-order address registers for channel 0 (AR data, AR instruction, AW data, AW instruction).\n12. Enable the fractional divider output to the CSI-2 subsystem.\n13. Set the DMA channel 0 program counter address and program the DMA transfer instructions with source, destination, and transfer size.\n14. Start DMA channel 0.\n15. Enable the internal test pattern generator by configuring vertical resolution, horizontal resolution, pattern configuration, and enabling the PG.\n16. Wait for the pattern generator to produce data.\n17. Disable the pattern generator.\n18. Poll the DMA interrupt status register until channel 0 transfer completes.\n19. Wait for post-transfer settling time.\n20. Complete the test with pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY Stop State Validation: After snps_phy_init(), poll read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) until rd_data == 0x1000f.\n2. DMA Channel 0 Completion Polling: poll (rd_data & 0x1) != 0.\n3. Transfer Size Calculation Validation: csi2_data_trnsfr_size computed with 8-byte alignment.\n4. Pattern Generator Configuration Validation: PG_PATTERN_VRES=0x10, PG_PATTERN_HRES=0x70140, PG_CONFIG=0xe401, PG_ENABLE=1 then PG_ENABLE=0.\n5. Test Completion: finish(0) is called indicating test pass.",
        "Validation / Acceptance Criteria": "1. The D-PHY must enter stop state on all lanes and clock lane after initialization.\n2. The internal test pattern generator must be successfully configured and enabled, then disabled after a wait period.\n3. DMA channel 0 must complete the CSI-2 data transfer from the pattern generator.\n4. The CSI-2 data transfer size must be correctly calculated based on resolution and bits per pixel with proper 8-byte alignment.\n5. The test must complete successfully by calling finish with pass status (0).",
        "Remarks": "The test uses conditional compilation (GDMA0_PATH/GDMA1_PATH/GDMA2_PATH/GDMA3_PATH) to select the GDMA path. FPS60 conditional compilation controls the dma_dest_addr_incr_flag. The locally defined csi2_enable_interrupt() is present but not called by test_case(); test_case() calls csi2_subsys_enable_interrupt() instead. The write to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 enables the fractional divider output. PPI_PG_ENABLE is written twice: once with value 1 (enable) and once with value 0 (disable)."
    }
]

# TestPlan columns
tp_cols = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
           "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
           "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
           "Code Generation"]

# MetaData columns
md_cols = ["Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
           "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
           "Meta Headers", "Meta Macros", "Meta Arrays"]

# Create workbook
wb = Workbook()
ws_tp = wb.active
ws_tp.title = "TestPlan"
ws_md = wb.create_sheet("MetaData")

# Formatting
header_font = Font(name="Calibri", size=11, bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_align = Alignment(wrap_text=True, vertical="top")
thin_border = Border(
    left=Side(style="thin"), right=Side(style="thin"),
    top=Side(style="thin"), bottom=Side(style="thin")
)

def populate_sheet(ws, columns, data):
    # Header row
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align
        cell.border = thin_border
    # Data rows
    for row_idx, row_data in enumerate(data, 2):
        for col_idx, col_name in enumerate(columns, 1):
            val = row_data.get(col_name, "")
            cell = ws.cell(row=row_idx, column=col_idx, value=val)
            cell.alignment = wrap_align
            cell.border = thin_border
    # Auto-size columns
    for col_idx, col_name in enumerate(columns, 1):
        max_len = len(str(col_name))
        for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split("\n")
                    for line in lines:
                        max_len = max(max_len, len(line))
        width = min(max_len + 2, 60)
        ws.column_dimensions[get_column_letter(col_idx)].width = width
    # Freeze first row
    ws.freeze_panes = "A2"

populate_sheet(ws_tp, tp_cols, json_data)
populate_sheet(ws_md, md_cols, json_data)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save
filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
wb.save(filepath)
print(f"Workbook saved: {filepath}")

# Validate
vwb = load_workbook(filepath)
assert "TestPlan" in vwb.sheetnames, "TestPlan sheet missing"
assert "MetaData" in vwb.sheetnames, "MetaData sheet missing"
assert vwb["TestPlan"].max_row == 3, f"Expected 3 rows in TestPlan, got {vwb['TestPlan'].max_row}"
assert vwb["MetaData"].max_row == 3, f"Expected 3 rows in MetaData, got {vwb['MetaData'].max_row}"
fsize = os.path.getsize(filepath)
assert fsize > 0, "File size is 0"
print(f"Validation PASSED. File size: {fsize} bytes")
print(f"Filename: {filename}")
print(f"Rows TestPlan: 2, Rows MetaData: 2")

# Push to GitHub if token available
token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
if token:
    import urllib.request
    with open(filepath, "rb") as f:
        content_b64 = base64.b64encode(f.read()).decode("utf-8")
    api_url = f"https://api.github.com/repos/titusbspgit/PSVValidation/contents/Test_Output/MIPI/TestPlan/{filename}"
    payload = json.dumps({
        "message": "Added generated TestPlan Excel",
        "content": content_b64,
        "branch": "main"
    }).encode("utf-8")
    req = urllib.request.Request(api_url, data=payload, method="PUT")
    req.add_header("Authorization", f"token {token}")
    req.add_header("Content-Type", "application/json")
    req.add_header("Accept", "application/vnd.github.v3+json")
    try:
        resp = urllib.request.urlopen(req)
        resp_data = json.loads(resp.read().decode("utf-8"))
        sha = resp_data.get("commit", {}).get("sha", "N/A")
        print(f"Pushed to GitHub. Commit SHA: {sha}")
    except Exception as e:
        print(f"GitHub push failed: {e}")
        print("File generated locally. Push manually.")
else:
    print("No GITHUB_TOKEN found. File generated locally only.")
    print(f"To push manually, upload {filepath} to: Test_Output/MIPI/TestPlan/{filename}")

print("\nDone.")
