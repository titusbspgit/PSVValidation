#!/usr/bin/env python3
"""
Agent 7 - MIPI_CSI TestPlan XLSX Generator
Run: python3 generate_mipi_csi_testplan_xlsx_agent7_final.py
Requires: pip install openpyxl requests
After running, the script generates the XLSX and pushes it to GitHub.
"""
import os, sys, json, base64, datetime, requests
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side

# GitHub config
GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")
OWNER = "titusbspgit"
REPO = "PSVValidation"
BRANCH = "main"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"

def generate_and_push():
    ist = datetime.timezone(datetime.timedelta(hours=5, minutes=30))
    now = datetime.datetime.now(ist)
    ts = now.strftime("%Y%m%d_%H%M%S")
    filename = f"MIPI_CSI_TestPlan_{ts}.xlsx"

    json_data = [
        {
            "Index": "1",
            "SS / Module": "MIPI_CSI",
            "Test Case Name": "mipi_csi2_dphy_lanes_test",
            "Feature": "D-PHY Lane Configuration",
            "Test Description": "Verify MIPI CSI-2 D-PHY lane configuration by iterating through all supported lane counts (4 down to 1). For each lane configuration, enable CSI-2 host interrupts, initialize the D-PHY, poll for PHY stop state, configure the active lane count, and perform DMA-based control and data packet transfers with interrupt-driven completion. Validate that the PHY enters stop state correctly for each lane configuration and that DMA transfers complete successfully.",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Remarks": "Two unresolved hex register addresses exist: 0xa0243ffc and 0xE6001000. External functions snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() are not defined within this testcase folder. DMA offset-based registers excluded per offset-exclusion rules. Registers written twice in sequence. Conditional compilation selects GDMA path.",
            "Test Steps / Procedure": "1. Enable CSI-2 host controller interrupts by reading INT_ST_MAIN to clear pending interrupts, then writing enable values to all interrupt mask registers.\n2. Configure the virtual channel ID in the CSI-2 wrapper virtual_channel register.\n3. Set gdma_reg_base address and write virtual_channel and control_data registers (twice).\n4. Initialize the D-PHY.\n5. Poll PHY_STOPSTATE until all lanes enter stop state (0x1000f).\n6. Set DMA program counter addresses for channel 0 and channel 1.\n7. Begin outer loop iterating lane_num from 3 down to 0.\n8. Write N_LANES register with current lane_num.\n9. Calculate control packet count based on vertical resolution.\n10. Signal lane count to external test pattern generator.\n11. Begin inner loop iterating through each expected packet.\n12. Enable DMA interrupts for both channels.\n13. Program DMA channel 0 for 8-byte control data transfer and start DMA.\n14. Poll DMA interrupt status for channel 0 completion (bit 0).\n15. Clear DMA channel 0 interrupt.\n16. Read transferred control data from destination address.\n17. If data type indicates image data, program DMA channel 1 and start it.\n18. Poll DMA interrupt status for channel 1 completion (bit 1).\n19. Clear DMA channel 1 interrupt.\n20. End inner loop.\n21. End outer loop.\n22. Signal test completion with pass status.",
            "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
            "Validation / Acceptance Criteria": "1. PHY_STOPSTATE must read back expected value for all lane configurations.\n2. DMA channel 0 interrupt status must indicate transfer completion.\n3. Control data must contain valid data type and word count.\n4. DMA channel 1 interrupt status must indicate transfer completion.\n5. Test must complete successfully for all four lane configurations.",
            "Meta Headers": '#include <stdio.h>; #include <stdlib.h>; #include "test_common.h"; #include "mipi_csi2.h"',
            "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 1080 (if GDMA0_FULL_MEM defined), else 3\n#define HRES 1920 (if GDMA0_FULL_MEM defined), else 64\n#define DATA_TYPE CSI2_RGB888",
            "Meta Arrays": "NA",
            "Meta Test Description": "This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through all lane counts (4 lanes down to 1 lane). For each lane configuration, the test enables CSI-2 host controller interrupts by reading INT_ST_MAIN to clear pending interrupts and then writing interrupt mask registers with specific enable values. It then initializes the D-PHY via snps_phy_init() and polls PHY_STOPSTATE until the value equals 0x1000f indicating all lanes have entered stop state. For each lane count (3 down to 0), the test writes N_LANES to configure the number of active lanes, signals the lane count to an external entity via address 0xa0243ffc, and then enters a packet processing loop with DMA-based control and data packet transfers.",
            "Meta Test Steps / Procedure": "1. Call csi2_enable_interrupt().\n2. Inside csi2_enable_interrupt(): rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) to clear pending interrupts.\n3. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f).\n4. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003).\n5. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f).\n6. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f).\n7. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff).\n8. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff).\n9. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff).\n10. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff).\n11. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff).\n12. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff).\n13. Return from csi2_enable_interrupt().\n14-57. [Remaining steps as documented in full meta procedure]",
            "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
            "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE poll: exit condition rd_data == 0x1000f.\n2. DMA channel 0 completion poll: exit condition (rd_data & 0x1) != 0x0.\n3. Control data type check: (csi_ctrl_data & 0x3f) > 0xf.\n4. DMA channel 1 completion poll: exit condition (rd_data & 0x2) != 0x0.\n5. Test completion: finish(0) called with argument 0."
        },
        {
            "Index": "2",
            "SS / Module": "MIPI_CSI",
            "Test Case Name": "mipi_csi2_test_pattern_generator",
            "Feature": "Test Pattern Generator",
            "Test Description": "Verify the MIPI CSI-2 host controller's internal test pattern generator by configuring the virtual channel, enabling interrupts, initializing the D-PHY, polling for PHY stop state, computing the DMA transfer size for a 320x16 image at 24 bits per pixel, programming DMA channel 0 address registers and transfer instructions, enabling the fractional divider output, starting the DMA transfer, enabling the pattern generator with specific vertical resolution, horizontal resolution, and configuration values, disabling the pattern generator after a delay, polling for DMA completion, and verifying successful test completion.",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Remarks": "The local function csi2_enable_interrupt() is defined in program.c but is NOT called from test_case(). Instead, test_case() calls csi2_subsys_enable_interrupt() which is an external function. DMA offset-based register excluded per offset-exclusion rules. PPI_PG_ENABLE is written twice: once with value 1 (enable) and once with value 0 (disable). Conditional compilation selects GDMA path and FPS60 mode.",
            "Test Steps / Procedure": "1. Initialize the interrupt pending flag and set the virtual channel ID to 3.\n2. Configure the virtual channel ID mapping in the CSI-2 wrapper.\n3. Write virtual_channel register and disable control data transfer.\n4. Enable CSI-2 and DMA interrupts via subsystem interrupt enable function.\n5. Initialize the D-PHY.\n6. Poll PHY_STOPSTATE until all lanes enter stop state (0x1000f).\n7. Compute DMA transfer size for 320x16 image at 24 bpp, 8-byte aligned.\n8. Program DMA M0 higher-order AXI address registers for channel 0.\n9. Enable fractional divider output to CSI-2 subsystem.\n10. Program DMA channel 0 transfer instructions.\n11. Start DMA channel 0.\n12. Enable test pattern generator with VRES=0x10, HRES=0x70140, CONFIG=0xe401, ENABLE=1.\n13. Wait for 100 time units.\n14. Disable test pattern generator by writing 0 to PPI_PG_ENABLE.\n15. Poll DMA interrupt status for channel 0 completion (bit 0).\n16. Wait for 10000 time units.\n17. Signal test completion with pass status.",
            "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; enableclkgating_csictrl; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
            "Validation / Acceptance Criteria": "1. PHY_STOPSTATE must read back expected value for all lanes.\n2. Test pattern generator must be successfully enabled and disabled.\n3. DMA channel 0 interrupt status must indicate transfer completion.\n4. DMA transfer size must correctly represent 320x16 image at 24 bpp.\n5. Test must complete successfully by calling finish routine.",
            "Meta Headers": '#include <stdio.h>; #include <stdlib.h>; #include "test_common.h"; #include "mipi_csi2.h"',
            "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
            "Meta Arrays": "NA",
            "Meta Test Description": "This testcase validates the MIPI CSI-2 host controller's internal test pattern generator (PPI PG) functionality. The test configures the CSI-2 subsystem virtual channel register and disables control data transfer. It calls csi2_subsys_enable_interrupt() to enable CSI-2 and DMA interrupts. It then initializes the D-PHY and polls PHY_STOPSTATE until 0x1000f. The test computes the DMA transfer size based on hres=320, vres=16, valid_bits_per_pixel=24, aligning to 8-byte boundaries. It programs DMA M0 address registers, enables fractional divider output, programs DMA channel 0, starts DMA, enables pattern generator, waits, disables pattern generator, polls DMA INTMIS for completion, and calls finish(0).",
            "Meta Test Steps / Procedure": "1. Set int_pend = 1.\n2. printf(\"start line\\n\").\n3. vcid = 3.\n4. vcid_unselected_path = ((vcid + 1) & 0xf) = 4.\n5-7. Conditional compilation for GDMA path selection.\n8. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\n9. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0).\n10. Call csi2_subsys_enable_interrupt().\n11. Call snps_phy_init().\n12-13. Poll PHY_STOPSTATE until 0x1000f.\n14-17. Compute csi2_data_trnsfr_size.\n18-21. Write DMA M0 address registers.\n22. write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1).\n23-27. Program and start DMA channel 0.\n28-33. Call csi2_ctrlr_pg_enable() - write PG registers.\n34. wait_on(100).\n35. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0).\n36-37. Poll DMA INTMIS for channel 0 completion.\n38. wait_on(10000).\n39. Call finish(0).\n40-51. [csi2_enable_interrupt() defined but NOT called from test_case()]",
            "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
            "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE poll: exit condition rd_data == 0x1000f.\n2. DMA channel 0 completion poll: exit condition (rd_data & 0x1) != 0.\n3. Test completion: finish(0) called.\n4. Pattern generator enable/disable: PPI_PG_ENABLE written 1 then 0.\n5. DMA transfer size: 15360 bytes (960 * 16)."
        }
    ]

    tp_cols = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
               "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
               "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
               "Code Generation"]
    md_cols = ["Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
               "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
               "Meta Headers", "Meta Macros", "Meta Arrays"]

    wb = Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    hf = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    hfill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    ha = Alignment(horizontal='center', vertical='center', wrap_text=True)
    ca = Alignment(vertical='top', wrap_text=True)
    tb = Border(left=Side(style='thin'), right=Side(style='thin'),
                top=Side(style='thin'), bottom=Side(style='thin'))

    for ci, cn in enumerate(tp_cols, 1):
        c = ws_tp.cell(row=1, column=ci, value=cn)
        c.font, c.fill, c.alignment, c.border = hf, hfill, ha, tb
    for ri, rd in enumerate(json_data, 2):
        for ci, cn in enumerate(tp_cols, 1):
            c = ws_tp.cell(row=ri, column=ci, value=rd.get(cn, ""))
            c.alignment, c.border = ca, tb
    ws_tp.freeze_panes = 'A2'
    for ci, cn in enumerate(tp_cols, 1):
        ws_tp.column_dimensions[ws_tp.cell(row=1, column=ci).column_letter].width = min(max(len(cn)+4, 20), 80)

    ws_md = wb.create_sheet("MetaData")
    for ci, cn in enumerate(md_cols, 1):
        c = ws_md.cell(row=1, column=ci, value=cn)
        c.font, c.fill, c.alignment, c.border = hf, hfill, ha, tb
    for ri, rd in enumerate(json_data, 2):
        for ci, cn in enumerate(md_cols, 1):
            c = ws_md.cell(row=ri, column=ci, value=rd.get(cn, ""))
            c.alignment, c.border = ca, tb
    ws_md.freeze_panes = 'A2'
    for ci, cn in enumerate(md_cols, 1):
        ws_md.column_dimensions[ws_md.cell(row=1, column=ci).column_letter].width = min(max(len(cn)+4, 20), 80)
    ws_md.sheet_state = 'veryHidden'

    wb.save(filename)
    file_size = os.path.getsize(filename)
    print(f"Generated: {filename} ({file_size} bytes)")

    # Push to GitHub
    if GITHUB_TOKEN:
        with open(filename, 'rb') as f:
            content_b64 = base64.b64encode(f.read()).decode('utf-8')
        path = f"{OUTPUT_DIR}/{filename}"
        url = f"https://api.github.com/repos/{OWNER}/{REPO}/contents/{path}"
        headers = {"Authorization": f"token {GITHUB_TOKEN}", "Accept": "application/vnd.github.v3+json"}
        # Check if file exists
        r = requests.get(url, headers=headers, params={"ref": BRANCH})
        data = {"message": "Added generated TestPlan Excel", "content": content_b64, "branch": BRANCH}
        if r.status_code == 200:
            data["sha"] = r.json()["sha"]
        r = requests.put(url, headers=headers, json=data)
        if r.status_code in (200, 201):
            result = r.json()
            print(f"Pushed to GitHub: {result['content']['html_url']}")
            print(f"Commit SHA: {result['commit']['sha']}")
        else:
            print(f"GitHub push failed: {r.status_code} {r.text}")
    else:
        print("No GITHUB_TOKEN set. File saved locally only.")

if __name__ == '__main__':
    generate_and_push()
