#!/usr/bin/env python3
"""MIPI_CSI TestPlan XLSX Generator - Agent 7 Direct Execution
Generates a genuine Office Open XML (.xlsx) workbook using openpyxl.
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
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

# ============================================================
# JSON DATA
# ============================================================
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "D-PHY Lane Configuration",
        "Meta Headers": '#include <stdio.h>; #include <stdlib.h>; #include "test_common.h"; #include "mipi_csi2.h"',
        "Meta Macros": '#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 1080 (if GDMA0_FULL_MEM defined), else 3\n#define HRES 1920 (if GDMA0_FULL_MEM defined), else 64\n#define DATA_TYPE CSI2_RGB888',
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through all lane counts (4 lanes down to 1 lane). For each lane configuration, the test enables CSI-2 host controller interrupts by reading INT_ST_MAIN to clear pending interrupts and then writing interrupt mask registers (PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, ECC_CORRECTED) with specific enable values. It then initializes the D-PHY via snps_phy_init() and polls PHY_STOPSTATE until the value equals 0x1000f indicating all lanes have entered stop state. For each lane count (3 down to 0), the test writes N_LANES to configure the number of active lanes, signals the lane count to an external entity via address 0xa0243ffc, and then enters a packet processing loop. In each iteration of the inner loop, the test programs DMA channel 0 to transfer 8 bytes of control data from source address 0x8000 to destination 0xE6001000, starts the DMA via DMAGO_CSI, polls the DMA interrupt status (INTMIS) for channel 0 completion (bit 0), clears the DMA interrupt (INTCLR with 0x1), reads the control data from 0xE6001000, and if the data type field (bits [5:0]) is greater than 0xf, calculates the word_count from bits [21:6], computes csi_data_size aligned to 8 bytes, programs DMA channel 1 to transfer csi_data_size bytes from source 0x0000 to destination 0xE6002000, starts DMA channel 1 via DMAGO_CSI, polls DMA INTMIS for channel 1 completion (bit 1), reads INTMIS again, and clears the channel 1 interrupt (INTCLR with 0x2). The test completes by calling finish(0).",
        "Test Description": "Verify MIPI CSI-2 D-PHY lane configuration by iterating through all supported lane counts (4 down to 1). For each lane configuration, enable CSI-2 host interrupts, initialize the D-PHY, poll for PHY stop state, configure the active lane count, and perform DMA-based control and data packet transfers with interrupt-driven completion. Validate that the PHY enters stop state correctly for each lane configuration and that DMA transfers complete successfully.",
        "Meta Test Steps / Procedure": "1. Call csi2_enable_interrupt().\n2. Inside csi2_enable_interrupt(): rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) to clear pending interrupts.\n3. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f).\n4. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003).\n5. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f).\n6. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f).\n7. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff).\n8. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff).\n9. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff).\n10. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff).\n11. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff).\n12. Inside csi2_enable_interrupt(): write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff).\n13. Return from csi2_enable_interrupt().\n14. Conditional compilation for GDMA path selection.\n15. gdma_reg_base = 0xE6A00000.\n16. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\n17. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1).\n18-20. Repeat virtual_channel and control_data writes.\n21. Call snps_phy_init().\n22-23. Poll PHY_STOPSTATE until 0x1000f.\n24-25. Set DMA PC addresses.\n26. Begin outer loop: lane_num from 3 to 0.\n27. write_reg(N_LANES, lane_num).\n28-30. Calculate packet count and signal lane count.\n31-57. Inner loop: DMA transfers, polling, and data processing.",
        "Test Steps / Procedure": "1. Enable CSI-2 host controller interrupts by reading the main interrupt status register to clear pending interrupts, then writing enable values to all interrupt mask registers (PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, ECC_CORRECTED).\n2. Configure the virtual channel ID in the CSI-2 wrapper virtual_channel register based on the selected GDMA path.\n3. Set the gdma_reg_base address and write the virtual_channel and control_data registers (performed twice for both occurrences).\n4. Initialize the D-PHY by calling the PHY initialization sequence.\n5. Poll the PHY_STOPSTATE register until all lanes enter stop state (expected value 0x1000f).\n6. Set DMA program counter addresses for channel 0 and channel 1.\n7. Begin outer loop iterating lane_num from 3 down to 0 (4 lanes to 1 lane).\n8. Write the N_LANES register with the current lane_num to configure the number of active lanes.\n9. Calculate the control packet count based on vertical resolution.\n10. Signal the lane count to the external test pattern generator.\n11. Begin inner loop iterating through each expected packet.\n12. Enable DMA interrupts for both channels by writing the DMA interrupt enable register.\n13. Program DMA channel 0 for an 8-byte control data transfer and start the DMA.\n14. Poll the DMA interrupt status register for channel 0 completion (bit 0 set).\n15. Clear the DMA channel 0 interrupt.\n16. Read the transferred control data from the destination address.\n17. If the data type field indicates image data (bits [5:0] > 0xf), extract the word count, compute the 8-byte aligned transfer size, program DMA channel 1 for the image data transfer, and start DMA channel 1.\n18. Poll the DMA interrupt status register for channel 1 completion (bit 1 set).\n19. Clear the DMA channel 1 interrupt.\n20. End inner loop.\n21. End outer loop.\n22. Signal test completion with pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE poll: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) is polled in a while loop. The exit condition is rd_data == 0x1000f.\n2. DMA channel 0 completion poll: bit 0 of INTMIS set.\n3. Control data type check: (csi_ctrl_data & 0x3f) > 0xf.\n4. DMA channel 1 completion poll: bit 1 of INTMIS set.\n5. Test completion: finish(0) called with argument 0.",
        "Validation / Acceptance Criteria": "1. The PHY_STOPSTATE register must read back the expected value indicating all configured D-PHY lanes and the clock lane have entered stop state.\n2. DMA channel 0 interrupt status must indicate transfer completion for each control packet transfer.\n3. The control data read from the destination must contain a valid data type and word count for image data packets.\n4. DMA channel 1 interrupt status must indicate transfer completion for each image data transfer when applicable.\n5. The test must complete successfully for all four lane configurations (4 lanes down to 1 lane) by calling the finish routine with a pass indication.",
        "Remarks": "Two unresolved hex register addresses exist: 0xa0243ffc (used to signal lane count to external test pattern generator) and 0xE6001000 (DMA control data destination, also read back for packet parsing). External functions snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() are not defined within this testcase folder. DMA offset-based registers (MIPI_CSI2_DMA_INTEN_OFFSET, MIPI_CSI2_DMA_INTMIS_OFFSET, MIPI_CSI2_DMA_INTCLR_OFFSET, and hardcoded offset 0x28) are accessed via gdma_reg_base variable and were excluded from Agent 2 token extraction per offset-exclusion rules. The MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL and MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA registers are each written twice in sequence. Conditional compilation selects GDMA path and VC_ID shift; default macros assume DPHY_LANES_TEST is defined."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "Test Pattern Generator",
        "Meta Headers": '#include <stdio.h>; #include <stdlib.h>; #include "test_common.h"; #include "mipi_csi2.h"',
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates the MIPI CSI-2 host controller's internal test pattern generator (PPI PG) functionality. The test configures the CSI-2 subsystem virtual channel register and disables control data transfer. It calls csi2_subsys_enable_interrupt() to enable CSI-2 and DMA interrupts (external function). It then initializes the D-PHY via snps_phy_init() and polls PHY_STOPSTATE until the value equals 0x1000f indicating all lanes have entered stop state. The test computes the DMA transfer size based on hres=320, vres=16, valid_bits_per_pixel=24, aligning to 8-byte boundaries. It programs the DMA M0 address registers for channel 0 (AR data=0x100, AR instruction=0x0, AW data=0x0, AW instruction=0x0), enables the fractional divider output to the CSI-2 subsystem by writing 0x1 to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, programs DMA channel 0 transfer instructions via dma_trnsfr_instn_preload_incr_addr(), and starts DMA channel 0 via DMAGO_CSI(). The test then enables the pattern generator by calling csi2_ctrlr_pg_enable() which writes PPI_PG_PATTERN_VRES=0x10, PPI_PG_PATTERN_HRES=0x70140, PPI_PG_CONFIG=0xe401, PPI_PG_ENABLE=1. After a wait_on(100) delay, the pattern generator is disabled by writing PPI_PG_ENABLE=0. The test then polls the DMA INTMIS register for channel 0 completion (bit 0), waits wait_on(10000), and calls finish(0). A local function csi2_enable_interrupt() is defined which reads INT_ST_MAIN to clear pending interrupts and writes all 10 interrupt mask registers with enable values, but it is not directly called from test_case(); instead csi2_subsys_enable_interrupt() is called.",
        "Test Description": "Verify the MIPI CSI-2 host controller's internal test pattern generator by configuring the virtual channel, enabling interrupts, initializing the D-PHY, polling for PHY stop state, computing the DMA transfer size for a 320x16 image at 24 bits per pixel, programming DMA channel 0 address registers and transfer instructions, enabling the fractional divider output, starting the DMA transfer, enabling the pattern generator with specific vertical resolution, horizontal resolution, and configuration values, disabling the pattern generator after a delay, polling for DMA completion, and verifying successful test completion.",
        "Meta Test Steps / Procedure": "1. Set int_pend = 1.\n2-7. Configure GDMA path and virtual channel.\n8. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\n9. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0).\n10. Call csi2_subsys_enable_interrupt().\n11. Call snps_phy_init().\n12-13. Poll PHY_STOPSTATE until 0x1000f.\n14-17. Compute DMA transfer size.\n18-21. Program DMA M0 address registers.\n22. write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1).\n23-27. Start DMA channel 0.\n28-33. Enable pattern generator.\n34-35. Wait and disable pattern generator.\n36-37. Poll DMA INTMIS for completion.\n38-39. Wait and finish.\n40-51. csi2_enable_interrupt() definition (not called from test_case).",
        "Test Steps / Procedure": "1. Initialize the interrupt pending flag and set the virtual channel ID to 3.\n2. Configure the virtual channel ID mapping in the CSI-2 wrapper virtual_channel register based on the selected GDMA path, and compute the GDMA register base address.\n3. Write the virtual_channel register with the computed VC ID mapping and disable control data transfer by writing 0 to the control_data register.\n4. Enable CSI-2 and DMA interrupts via the subsystem interrupt enable function.\n5. Initialize the D-PHY by calling the PHY initialization sequence.\n6. Poll the PHY_STOPSTATE register until all lanes enter stop state (expected value 0x1000f).\n7. Compute the DMA transfer size for a 320x16 image at 24 bits per pixel, aligned to 8-byte boundaries.\n8. Program the DMA M0 higher-order AXI address registers for channel 0: read data address to 0x100, read instruction address to 0x0, write data address to 0x0, write instruction address to 0x0.\n9. Enable the fractional divider output to the CSI-2 subsystem by writing to the enableclkgating_csictrl register.\n10. Program DMA channel 0 transfer instructions with source address 0x00, destination address 0xE6001000, and the computed transfer size.\n11. Start DMA channel 0.\n12. Enable the test pattern generator by writing vertical resolution (0x10), horizontal resolution (0x70140), configuration (0xe401), and enable (1) to the PPI PG registers.\n13. Wait for 100 time units.\n14. Disable the test pattern generator by writing 0 to the PPI_PG_ENABLE register.\n15. Poll the DMA interrupt status register for channel 0 completion (bit 0 set).\n16. Wait for 10000 time units.\n17. Signal test completion with pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; enableclkgating_csictrl; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE poll: exit condition rd_data == 0x1000f.\n2. DMA channel 0 completion poll: bit 0 of INTMIS set.\n3. Test completion: finish(0) called.\n4. Pattern generator enable/disable sequence verified.\n5. DMA transfer size = 15360 bytes.",
        "Validation / Acceptance Criteria": "1. The PHY_STOPSTATE register must read back the expected value indicating all configured D-PHY lanes and the clock lane have entered stop state.\n2. The test pattern generator must be successfully enabled and then disabled after the configured delay.\n3. DMA channel 0 interrupt status must indicate transfer completion for the image data transfer from the pattern generator.\n4. The computed DMA transfer size must correctly represent a 320x16 image at 24 bits per pixel with 8-byte alignment.\n5. The test must complete successfully by calling the finish routine with a pass indication.",
        "Remarks": "The local function csi2_enable_interrupt() is defined in program.c but is NOT called from test_case(). Instead, test_case() calls csi2_subsys_enable_interrupt() which is an external function not defined in this testcase folder. The csi2_enable_interrupt() function definition is preserved in Meta Test Steps (steps 40-51) for completeness as it contains register access information captured by Agent 2. External functions snps_phy_init(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), csi2_subsys_enable_interrupt(), and finish() are not defined within this testcase folder. DMA offset-based register MIPI_CSI2_DMA_INTMIS_OFFSET is accessed via gdma_reg_base variable and was excluded from Agent 2 token extraction per offset-exclusion rules. PPI_PG_ENABLE is written twice: once with value 1 (enable) inside csi2_ctrlr_pg_enable() and once with value 0 (disable) directly in test_case(). Conditional compilation selects GDMA path and FPS60 mode for destination address increment flag."
    }
]

# ============================================================
# SHEET DEFINITIONS
# ============================================================
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

# ============================================================
# GENERATE WORKBOOK
# ============================================================
def generate_workbook():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"
    output_dir = "Test_Output/MIPI/TestPlan"
    filepath = os.path.join(output_dir, filename)

    wb = Workbook()

    # --- TestPlan Sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_align = Alignment(wrap_text=True, vertical="top")

    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    for row_idx, item in enumerate(json_data, 2):
        ws_tp.cell(row=row_idx, column=1, value=item.get("Index", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=2, value=item.get("SS / Module", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=3, value=item.get("Feature", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=4, value=item.get("Test Case Name", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=5, value=item.get("Test Description", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=6, value=item.get("Speed", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=7, value=item.get("Mode", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=8, value=item.get("Memory Start Offset", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=9, value=item.get("Memory End Offset", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=10, value=item.get("Remarks", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=11, value=item.get("Test Steps / Procedure", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=12, value=item.get("Impacted Registers", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=13, value=item.get("Validation / Acceptance Criteria", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=14, value="").alignment = wrap_align

    ws_tp.freeze_panes = "A2"

    # Auto-size columns
    for col_idx in range(1, len(TESTPLAN_COLUMNS) + 1):
        max_len = len(TESTPLAN_COLUMNS[col_idx - 1])
        for row in ws_tp.iter_rows(min_row=2, max_row=ws_tp.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split("\n")
                    for line in lines:
                        max_len = max(max_len, len(line))
        width = min(max_len + 2, 80)
        ws_tp.column_dimensions[get_column_letter(col_idx)].width = width

    # --- MetaData Sheet ---
    ws_md = wb.create_sheet(title="MetaData")

    for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    for row_idx, item in enumerate(json_data, 2):
        ws_md.cell(row=row_idx, column=1, value=item.get("Index", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=2, value=item.get("Test Case Name", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=3, value=item.get("Meta Test Description", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=4, value=item.get("Meta Test Steps / Procedure", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=5, value=item.get("Meta Impacted Registers", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=6, value=item.get("Meta Validation / Acceptance Criteria", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=7, value=item.get("Meta Headers", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=8, value=item.get("Meta Macros", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=9, value=item.get("Meta Arrays", "")).alignment = wrap_align

    ws_md.freeze_panes = "A2"
    ws_md.sheet_state = "veryHidden"

    for col_idx in range(1, len(METADATA_COLUMNS) + 1):
        max_len = len(METADATA_COLUMNS[col_idx - 1])
        for row in ws_md.iter_rows(min_row=2, max_row=ws_md.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split("\n")
                    for line in lines:
                        max_len = max(max_len, len(line))
        width = min(max_len + 2, 80)
        ws_md.column_dimensions[get_column_letter(col_idx)].width = width

    # --- Save ---
    os.makedirs(output_dir, exist_ok=True)
    wb.save(filepath)
    print(f"Workbook saved: {filepath}")

    # --- Validate ---
    assert os.path.exists(filepath), "File does not exist"
    fsize = os.path.getsize(filepath)
    assert fsize > 0, "File is empty"
    print(f"File size: {fsize} bytes")

    wb2 = load_workbook(filepath)
    assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in wb2.sheetnames, "MetaData sheet missing"
    tp_rows = wb2["TestPlan"].max_row - 1
    md_rows = wb2["MetaData"].max_row - 1
    print(f"TestPlan rows: {tp_rows}, MetaData rows: {md_rows}")
    print(f"Validation: PASSED")
    print(f"FILENAME={filename}")
    return filename, filepath

if __name__ == "__main__":
    generate_workbook()
