#!/usr/bin/env python3
"""PCIE TestPlan XLSX Generator - Agent 7 Automated Output
Generates a genuine Office Open XML workbook (.xlsx) using openpyxl.
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter

# ============================================================
# TESTCASE DATA - 4 PCIE Test Cases
# ============================================================
TESTCASES = [
    {
        "S.No": 1,
        "Test Case Name": "pcie_device_enumerate_test",
        "Test Category": "Functional",
        "Functional Area": "PCIe Device Enumeration and Configuration",
        "Test Objective": "Verify PCIe device enumeration by performing link training, cache coherency programming, link status polling, vendor ID read, command register configuration, memory base programming, BAR enumeration on both DM0 and DM1 instances, and synchronization handshake.",
        "Test Description": "This test validates the complete PCIe device enumeration flow. It begins by initializing the synchronization register and performing link training on the configured PCIe instance (DM0 or DM1 in RC or EP mode). Cache coherency control is then programmed on both PCIE0 and PCIE1 instances by setting the master AXI read/write cache mode and value fields via read-modify-write operations on the COHERENCY_CONTROL_3_OFF register. The test polls the SII link status registers on both instances until the link is confirmed up (status bits 0xD1). In RC mode, the Vendor ID is read from the TYPE1_DEV_ID_VEND_ID_REG, the TYPE1_STATUS_COMMAND_REG is configured to enable IO, Memory, and Bus Master access, and memory base programming is performed for both DM0 and DM1. System-level configuration registers are then written. Cache coherency is subsequently disabled by clearing the cache value fields. BAR enumeration is performed on both slave interfaces by writing 0xFFFFFFFF to BAR0_REG through PREF_MEM_LIMIT_PREF_MEM_BASE_REG, reading back to determine BAR sizes, then programming final BAR values. The test concludes by polling the synchronization register for a completion handshake value of 0x12345678.",
        "Pre-conditions": "1. PCIe subsystem is powered on and out of reset.\n2. PCIe link partner (RC or EP) is present and ready.\n3. The compile-time mode flag (DM0_RC, DM1_RC, DM0_EP, or DM1_EP) is defined.\n4. System integration interface (SII) registers are accessible.\n5. PCIe DBI and slave configuration spaces are accessible.\n6. Header file pcie.h with macro definitions and helper functions is available.",
        "Test Steps": "1. Write 0x0 to the synchronization register to initialize the handshake.\n2. Perform PCIe link training on the configured instance (DM0 or DM1) with x4 lane width.\n3. Program cache coherency on PCIE0 instance: read the Coherency Control 3 register, set AXI write cache mode bits [14:11] to 0xF and read cache mode bits [6:3] to 0xF via read-modify-write, then set AXI write cache value bits [30:27] to 0xF and read cache value bits [22:19] to 0xF.\n4. Repeat cache coherency programming for PCIE1 instance with the same bit field settings.\n5. Wait for 20 cycles, then perform a consolidated cache coherency write on both PCIE0 and PCIE1 setting all four cache fields simultaneously.\n6. Read the SII0 link status register and poll until link-up status bits (0xD1) are confirmed.\n7. Configure non-secure protection via NIC programming.\n8. Read the SII1 link status register and poll until link-up status bits (0xD1) are confirmed.\n9. In RC mode: Read the TYPE1_DEV_ID_VEND_ID_REG to retrieve the Vendor ID.\n10. In RC mode: Write 0x7 to the TYPE1_STATUS_COMMAND_REG to enable IO Space, Memory Space, and Bus Master.\n11. In RC mode: Execute memory base programming for DM0 and DM1 instances.\n12. Write 0x1 to six system-level configuration registers for subsystem initialization.\n13. Disable cache coherency on PCIE0 and PCIE1.\n14. Perform BAR enumeration on slave interface 1 and slave interface 0.\n15. Poll the synchronization register until the completion handshake value 0x12345678 is received.\n16. Call finish to end the test.",
        "Expected Results": "1. Link training completes successfully on the configured PCIe instance.\n2. Cache coherency control fields are correctly programmed and verified on both PCIE0 and PCIE1.\n3. SII0 and SII1 link status registers report link-up status (bits matching 0xD1).\n4. Vendor ID read returns the expected value (0x16C3 per spec reset value).\n5. Command register is successfully written with IO, Memory, and Bus Master enable bits.\n6. Memory base programming completes without errors.\n7. System-level configuration registers accept writes.\n8. Cache coherency is successfully disabled on both instances.\n9. BAR enumeration on both slave interfaces returns valid BAR size information when written with 0xFFFFFFFF.\n10. Final BAR address values are programmed and read back correctly.\n11. Synchronization register reaches the expected handshake value 0x12345678.\n12. Test completes with finish(0) indicating PASS.",
        "Registers Accessed": "COHERENCY_CONTROL_3_OFF; TYPE1_DEV_ID_VEND_ID_REG; TYPE1_STATUS_COMMAND_REG; BAR0_REG; BAR1_REG; SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG; SEC_STAT_IO_LIMIT_IO_BASE_REG; MEM_LIMIT_MEM_BASE_REG; PREF_MEM_LIMIT_PREF_MEM_BASE_REG",
        "Register Fields": "COHERENCY_CONTROL_3_OFF: CFG_MSTR_ARCACHE_MODE[6:3], CFG_MSTR_AWCACHE_MODE[14:11], CFG_MSTR_ARCACHE_VALUE[22:19], CFG_MSTR_AWCACHE_VALUE[30:27]; TYPE1_DEV_ID_VEND_ID_REG: VENDOR_ID[15:0], DEVICE_ID[31:16]; TYPE1_STATUS_COMMAND_REG: IO_EN[0], MSE[1], BME[2]; BAR0_REG: BAR0_MEM_IO[0], BAR0_TYPE[2:1], BAR0_PREFETCH[3], BAR0_START[31:4]; BAR1_REG: BAR1_MEM_IO[0], BAR1_TYPE[2:1], BAR1_PREFETCH[3], BAR1_START[31:4]; SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG: PRIM_BUS[7:0], SEC_BUS[15:8], SUB_BUS[23:16], SEC_LAT_TIMER[31:24]; SEC_STAT_IO_LIMIT_IO_BASE_REG: IO_DECODE[0], IO_BASE[7:4], IO_LIMIT[15:12]; MEM_LIMIT_MEM_BASE_REG: MEM_BASE[15:4], MEM_LIMIT[31:20]; PREF_MEM_LIMIT_PREF_MEM_BASE_REG: PREF_MEM_DECODE[0], PREF_MEM_BASE[15:4], PREF_MEM_LIMIT[31:20]",
        "Register Address/Offset": "COHERENCY_CONTROL_3_OFF: 0x8E8; TYPE1_DEV_ID_VEND_ID_REG: 0x0; TYPE1_STATUS_COMMAND_REG: 0x4; BAR0_REG: 0x10; BAR1_REG: 0x14; SEC_LAT_TIMER: 0x18; SEC_STAT_IO: 0x1c; MEM_LIMIT: 0x20; PREF_MEM: 0x24; SII: 0xC0; Sync: 0xE6004100; System Config: 0xE690000C-0xE6900034",
        "Access Type": "COHERENCY_CONTROL_3_OFF: RW; TYPE1_DEV_ID_VEND_ID_REG: RO; TYPE1_STATUS_COMMAND_REG: RW; BAR0-PREF_MEM: RW; SII: RO; Sync: RW; System Config: WO",
        "Reset Value": "COHERENCY_CONTROL_3_OFF: 0x00000000; TYPE1_DEV_ID_VEND_ID_REG: 0xABCD16C3; TYPE1_STATUS_COMMAND_REG: 0x00100000; BAR0_REG: 0x00000004; BAR1_REG: 0x00000000; Others: 0x00000000; PREF_MEM: 0x00010001",
        "Pass/Fail Criteria": "PASS: All operations complete successfully. FAIL: Any timeout or unexpected value.",
        "Priority": "High",
        "Test Type": "Automated",
        "Applicable Modes": "DM0_RC; DM1_RC; DM0_EP; DM1_EP",
        "Macros Used": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF; mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF; DM0_RC; DM1_RC; DM0_EP; DM1_EP; DEBUG_DISPLAY",
        "Source File Path": "TestRepo/PCIE/pcie_device_enumerate_test/program.c",
        "SS_Module": "PCIE",
        "Feature": "Device Enumeration"
    },
    {
        "S.No": 2,
        "Test Case Name": "pcie_dma_write_test",
        "Test Category": "Functional",
        "Functional Area": "PCIe DMA Write and Read Data Transfer",
        "Test Objective": "Verify PCIe DMA write and read data transfer across all four DMA channels (CH0-CH3) on the configured PCIe instance, including interrupt-driven completion handling, data preloading, BAR programming, and memory base configuration.",
        "Test Description": "This test validates PCIe DMA write and read operations across all four channels on the configured PCIe instance (DM0 or DM1 in RC mode). The test begins by initializing the synchronization register and performing link training. After confirming link-up by polling the SII link status register, the Vendor ID is read and the command register is configured. BAR and memory base programming are performed. The test waits for synchronization, preloads source memory with patterns (0xC0DEBEED and 0xF00DDEAF), initializes GIC, clears DMA interrupt masks, then executes DMA write and read transfers sequentially on CH0-CH3 with interrupt-driven completion. The interrupt handler reads status, masks lower 4 bits, and clears interrupts.",
        "Pre-conditions": "1. PCIe subsystem is powered on and out of reset.\n2. PCIe link partner (RC or EP) is present and ready.\n3. DM0_RC or DM1_RC is defined.\n4. GIC interrupt controller is available.\n5. Source memory at 0xE6000000 is accessible.",
        "Test Steps": "1. Initialize the synchronization register by writing 0x0.\n2. Perform PCIe link training on the configured instance with x4 lane width.\n3. Poll the SII link status register until link-up status (0xD1 mask) is confirmed.\n4. Read the TYPE1_DEV_ID_VEND_ID_REG to verify the Vendor ID.\n5. Write 0x7 to the TYPE1_STATUS_COMMAND_REG to enable IO Space, Memory Space, and Bus Master.\n6. Execute BAR programming and memory base programming.\n7. Wait for synchronization register to reach 0x12345678.\n8. Preload source memory with known data patterns.\n9. Initialize GIC and enable all IRQs.\n10. Clear DMA write and read interrupt masks.\n11. Execute DMA write transfers on CH0-CH3 via doorbell register.\n12. Execute DMA read transfers on CH0-CH3 via doorbell register.\n13. Verify interrupt handler reads status and clears interrupts.\n14. Call finish to end the test.",
        "Expected Results": "1. Link training completes successfully.\n2. SII link status reports link-up.\n3. Vendor ID read returns expected value.\n4. DMA write and read interrupt masks are cleared.\n5. All four DMA write channels complete with interrupt-driven completion.\n6. All four DMA read channels complete with interrupt-driven completion.\n7. Interrupt handler correctly reads status and clears interrupts.\n8. Test completes with finish(0) indicating PASS.",
        "Registers Accessed": "TYPE1_DEV_ID_VEND_ID_REG; TYPE1_STATUS_COMMAND_REG; DMA_WRITE_INT_MASK_OFF; DMA_READ_INT_MASK_OFF; DMA_WRITE_DOORBELL_OFF; DMA_READ_DOORBELL_OFF; DMA_WRITE_INT_STATUS_OFF; DMA_READ_INT_STATUS_OFF; DMA_WRITE_INT_CLEAR_OFF; DMA_READ_INT_CLEAR_OFF",
        "Register Fields": "TYPE1_DEV_ID_VEND_ID_REG: VENDOR_ID[15:0], DEVICE_ID[31:16]; TYPE1_STATUS_COMMAND_REG: IO_EN[0], MSE[1], BME[2]; DMA_WRITE_INT_MASK_OFF: WRITE_DONE_INT_MASK[3:0], WRITE_ABORT_INT_MASK[19:16]; DMA_READ_INT_MASK_OFF: READ_DONE_INT_MASK[3:0], READ_ABORT_INT_MASK[19:16]; DMA_WRITE_DOORBELL_OFF: WR_DOORBELL_NUM[2:0]; DMA_READ_DOORBELL_OFF: RD_DOORBELL_NUM[2:0]; DMA_WRITE_INT_STATUS_OFF: WR_DONE_INT_STATUS[3:0]; DMA_READ_INT_STATUS_OFF: RD_DONE_INT_STATUS[3:0]; DMA_WRITE_INT_CLEAR_OFF: WR_DONE_INT_CLEAR[3:0]; DMA_READ_INT_CLEAR_OFF: RD_DONE_INT_CLEAR[3:0]",
        "Register Address/Offset": "TYPE1_DEV_ID_VEND_ID_REG: 0x0; TYPE1_STATUS_COMMAND_REG: 0x4; DMA_WRITE_INT_MASK_OFF: 0x380054; DMA_READ_INT_MASK_OFF: 0x3800A8; DMA_WRITE_DOORBELL_OFF: 0x380010; DMA_READ_DOORBELL_OFF: 0x380030; DMA_WRITE_INT_STATUS_OFF: 0x38004C; DMA_READ_INT_STATUS_OFF: 0x3800A0; DMA_WRITE_INT_CLEAR_OFF: 0x380058; DMA_READ_INT_CLEAR_OFF: 0x3800AC",
        "Access Type": "TYPE1_DEV_ID_VEND_ID_REG: RO; TYPE1_STATUS_COMMAND_REG: RW; DMA masks: RW; DMA doorbells: WO; DMA status: RO; DMA clear: WO",
        "Reset Value": "TYPE1_DEV_ID_VEND_ID_REG: 0xABCD16C3; TYPE1_STATUS_COMMAND_REG: 0x00100000; DMA masks: 0x000F000F; Others: 0x00000000",
        "Pass/Fail Criteria": "PASS: All DMA transfers complete with interrupts. FAIL: Any timeout or missing interrupt.",
        "Priority": "High",
        "Test Type": "Automated",
        "Applicable Modes": "DM0_RC; DM1_RC",
        "Macros Used": "mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF; mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF; mizar_PCIE1 DMA macros; DM0_RC; DM1_RC",
        "Source File Path": "TestRepo/PCIE/pcie_dma_write_test/program.c",
        "SS_Module": "PCIE",
        "Feature": "DMA Data Transfer"
    },
    {
        "S.No": 3,
        "Test Case Name": "pcie_mem_wr_rd_test",
        "Test Category": "Functional",
        "Functional Area": "PCIe Memory Write and Read Verification",
        "Test Objective": "Verify PCIe memory write and read operations through the slave interfaces on the configured PCIe instance, including cache coherency programming, link training, BAR programming, memory base configuration, and data integrity verification across multiple memory addresses.",
        "Test Description": "This test validates PCIe memory write and read operations on the configured PCIe instance (DM0 or DM1 in RC or EP mode). The test programs cache coherency, polls SII link status, configures command register, performs BAR and memory base programming, then executes memory write/read operations at multiple addresses with known data patterns. In RC mode, 3 addresses are tested; in EP mode, 5 BAR1 addresses are tested.",
        "Pre-conditions": "1. PCIe subsystem is powered on and out of reset.\n2. PCIe link partner (RC or EP) is present and ready.\n3. The compile-time mode flag (DM0_RC, DM1_RC, DM0_EP, or DM1_EP) is defined.\n4. System integration interface (SII) registers are accessible.\n5. PCIe DBI and slave configuration spaces are accessible.\n6. Header file pcie.h with macro definitions and helper functions is available.\n7. Memory regions targeted by write/read operations are accessible.",
        "Test Steps": "1. Initialize the synchronization register by writing 0x0.\n2. Perform PCIe link training on the configured instance with x4 lane width.\n3. Program cache coherency on both PCIE0 and PCIE1 instances.\n4. Poll the SII link status register until link-up status bits (0xD1 mask) are confirmed.\n5. In RC mode: Read Vendor ID and write command register.\n6. Execute BAR programming and memory base programming.\n7. Write synchronization handshake value 0x11111111.\n8. Disable cache coherency on both instances.\n9. Perform memory write/read operations at multiple addresses with data patterns.\n10. Poll synchronization register until 0x12345678 is received.\n11. Call finish to end the test.",
        "Expected Results": "1. Link training completes successfully.\n2. Cache coherency fields are correctly programmed and disabled.\n3. SII link status reports link-up.\n4. Vendor ID read returns expected value.\n5. All memory write/read operations complete with data integrity verified.\n6. Synchronization register reaches 0x12345678.\n7. Test completes with finish(0) indicating PASS.",
        "Registers Accessed": "COHERENCY_CONTROL_3_OFF; TYPE1_DEV_ID_VEND_ID_REG; TYPE1_STATUS_COMMAND_REG",
        "Register Fields": "COHERENCY_CONTROL_3_OFF: CFG_MSTR_ARCACHE_MODE[6:3], CFG_MSTR_AWCACHE_MODE[14:11], CFG_MSTR_ARCACHE_VALUE[22:19], CFG_MSTR_AWCACHE_VALUE[30:27]; TYPE1_DEV_ID_VEND_ID_REG: VENDOR_ID[15:0], DEVICE_ID[31:16]; TYPE1_STATUS_COMMAND_REG: IO_EN[0], MSE[1], BME[2]",
        "Register Address/Offset": "COHERENCY_CONTROL_3_OFF: 0x8E8; TYPE1_DEV_ID_VEND_ID_REG: 0x0; TYPE1_STATUS_COMMAND_REG: 0x4; SII: 0xC0; Sync: 0xE6004100",
        "Access Type": "COHERENCY_CONTROL_3_OFF: RW; TYPE1_DEV_ID_VEND_ID_REG: RO; TYPE1_STATUS_COMMAND_REG: RW",
        "Reset Value": "COHERENCY_CONTROL_3_OFF: 0x00000000; TYPE1_DEV_ID_VEND_ID_REG: 0xABCD16C3; TYPE1_STATUS_COMMAND_REG: 0x00100000",
        "Pass/Fail Criteria": "PASS: All memory write/read operations match. FAIL: Any data mismatch or timeout.",
        "Priority": "High",
        "Test Type": "Automated",
        "Applicable Modes": "DM0_RC; DM1_RC; DM0_EP; DM1_EP",
        "Macros Used": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF; mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF; DM0_RC; DM1_RC; DM0_EP; DM1_EP; DM0; DM1; DEBUG_DISPLAY",
        "Source File Path": "TestRepo/PCIE/pcie_mem_wr_rd_test/program.c",
        "SS_Module": "PCIE",
        "Feature": "Memory Write Read"
    },
    {
        "S.No": 4,
        "Test Case Name": "pcie_reg_wr_rd_test",
        "Test Category": "Functional",
        "Functional Area": "PCIe Register Write/Read and Reset Value Verification",
        "Test Objective": "Verify PCIe register reset default values and write/read data integrity across RC DBI controller registers, SII registers, and PHY registers on both PCIE0 and PCIE1 instances using multiple data patterns.",
        "Test Description": "This test validates PCIe register accessibility and data integrity across three register domains: RC DBI controller registers (MSI_CAP_OFF_08H_REG, MSI_CAP_OFF_10H_REG, FILTER_MASK_2_OFF, AXI_MSTR_MSG_ADDR_HIGH_OFF, UTILITY_OFF), SII registers (Transmit Header 2/3, PHY Control 23), and PHY registers. Phase 1 checks reset values. Phase 2 writes three patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555) and verifies readback.",
        "Pre-conditions": "1. PCIe subsystem is powered on and out of reset.\n2. PCIE0 and PCIE1 DBI controller register spaces are accessible.\n3. PCIE0 and PCIE1 SII register spaces are accessible.\n4. PCIE0 and PCIE1 PHY register spaces are accessible.\n5. Header file pcie.h with macro definitions is available.\n6. No prior register writes have modified default values before Phase 1.",
        "Test Steps": "1. Read all five RC DBI controller registers on PCIE0 and PCIE1 and verify reset defaults of 0x0.\n2. Read all three SII registers on both instances and verify reset defaults.\n3. Write PHY reset control register on both instances with 0x01203000.\n4. Read all three PHY registers on both instances and verify reset defaults.\n5. For each of three data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555): write to all registers, read back and verify.\n6. Verify both error counters are zero.\n7. Call finish to end the test.",
        "Expected Results": "1. All RC DBI registers return reset default 0x0.\n2. All SII registers return reset default 0x0.\n3. All PHY registers return reset default 0x0.\n4. All write/read patterns match for all three data patterns.\n5. Both error counters remain at zero.\n6. Test completes with finish(0) indicating PASS.",
        "Registers Accessed": "MSI_CAP_OFF_08H_REG; MSI_CAP_OFF_10H_REG; FILTER_MASK_2_OFF; AXI_MSTR_MSG_ADDR_HIGH_OFF; UTILITY_OFF",
        "Register Fields": "MSI_CAP_OFF_08H_REG: MSI_DATA[31:0]; MSI_CAP_OFF_10H_REG: MSI_DATA[31:0]; FILTER_MASK_2_OFF: FILTER_MASK_2[31:0]; AXI_MSTR_MSG_ADDR_HIGH_OFF: CFG_AXIMSTR_MSG_ADDR_HIGH[31:0]; UTILITY_OFF: UTILITY[31:0]; SII and PHY fields with write masks",
        "Register Address/Offset": "MSI_CAP_OFF_08H_REG: 0x58; MSI_CAP_OFF_10H_REG: 0x60; FILTER_MASK_2_OFF: 0x720; AXI_MSTR_MSG_ADDR_HIGH_OFF: 0x8F4; UTILITY_OFF: 0xC80; PHY0: 0xE68860B8, 0xE68862B8, 0xE68864B8; PHY1: 0xE68A60B8, 0xE68A62B8, 0xE68A64B8",
        "Access Type": "All RC registers: RW; SII: RW with masks; PHY: RW with 16-bit extraction and 13-bit mask 0x1FFF",
        "Reset Value": "All: 0x00000000",
        "Pass/Fail Criteria": "PASS: All reset values and write/read patterns match. FAIL: Any mismatch.",
        "Priority": "High",
        "Test Type": "Automated",
        "Applicable Modes": "NA",
        "Macros Used": "mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE0_DBI_DSP_UTILITY_OFF; PCIE1 counterparts; SII macros; PHY_RST_CONTROL macros",
        "Source File Path": "TestRepo/PCIE/pcie_reg_wr_rd_test/program.c",
        "SS_Module": "PCIE",
        "Feature": "Register Write Read"
    }
]

# ============================================================
# COLUMN DEFINITIONS
# ============================================================
TESTPLAN_HEADERS = [
    "S.No", "Test Case Name", "Test Category", "Functional Area",
    "Test Objective", "Test Description", "Pre-conditions", "Test Steps",
    "Expected Results", "Registers Accessed", "Register Fields",
    "Register Address/Offset", "Access Type", "Reset Value",
    "Pass/Fail Criteria", "Priority", "Test Type", "Applicable Modes",
    "Macros Used", "Source File Path"
]

METADATA_HEADERS = [
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria", "Meta Headers",
    "Meta Macros", "Meta Arrays"
]


def generate_workbook(output_dir):
    """Generate the XLSX workbook."""
    # IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"PCIE_TestPlan_{timestamp}.xlsx"
    filepath = os.path.join(output_dir, filename)

    # Create workbook
    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    # Header formatting
    header_font = Font(name="Calibri", size=11, bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=True)
    cell_align = Alignment(vertical="top", wrap_text=True)
    thin_border = Border(
        left=Side(style="thin"),
        right=Side(style="thin"),
        top=Side(style="thin"),
        bottom=Side(style="thin")
    )

    # Write TestPlan headers
    for col_idx, header in enumerate(TESTPLAN_HEADERS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=header)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align
        cell.border = thin_border

    # Write TestPlan data
    for row_idx, tc in enumerate(TESTCASES, 2):
        for col_idx, header in enumerate(TESTPLAN_HEADERS, 1):
            value = tc.get(header, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_align
            cell.border = thin_border

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # Auto-size columns
    for col_idx, header in enumerate(TESTPLAN_HEADERS, 1):
        max_len = len(str(header))
        for row_idx in range(2, len(TESTCASES) + 2):
            cell_val = str(ws_tp.cell(row=row_idx, column=col_idx).value or "")
            lines = cell_val.split("\n")
            for line in lines:
                max_len = max(max_len, len(line))
        col_letter = get_column_letter(col_idx)
        adjusted_width = min(max_len + 4, 60)
        ws_tp.column_dimensions[col_letter].width = max(adjusted_width, 12)

    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet("MetaData")

    # Write MetaData headers
    for col_idx, header in enumerate(METADATA_HEADERS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=header)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align
        cell.border = thin_border

    # Write MetaData data
    for row_idx, tc in enumerate(TESTCASES, 2):
        ws_md.cell(row=row_idx, column=1, value=tc.get("S.No", "")).alignment = cell_align
        ws_md.cell(row=row_idx, column=1).border = thin_border
        ws_md.cell(row=row_idx, column=2, value=tc.get("Test Case Name", "")).alignment = cell_align
        ws_md.cell(row=row_idx, column=2).border = thin_border
        ws_md.cell(row=row_idx, column=3, value=tc.get("Test Description", "")).alignment = cell_align
        ws_md.cell(row=row_idx, column=3).border = thin_border
        ws_md.cell(row=row_idx, column=4, value=tc.get("Test Steps", "")).alignment = cell_align
        ws_md.cell(row=row_idx, column=4).border = thin_border
        ws_md.cell(row=row_idx, column=5, value=tc.get("Registers Accessed", "")).alignment = cell_align
        ws_md.cell(row=row_idx, column=5).border = thin_border
        ws_md.cell(row=row_idx, column=6, value=tc.get("Pass/Fail Criteria", "")).alignment = cell_align
        ws_md.cell(row=row_idx, column=6).border = thin_border
        ws_md.cell(row=row_idx, column=7, value=tc.get("Register Fields", "")).alignment = cell_align
        ws_md.cell(row=row_idx, column=7).border = thin_border
        ws_md.cell(row=row_idx, column=8, value=tc.get("Macros Used", "")).alignment = cell_align
        ws_md.cell(row=row_idx, column=8).border = thin_border
        ws_md.cell(row=row_idx, column=9, value="").alignment = cell_align
        ws_md.cell(row=row_idx, column=9).border = thin_border

    # Freeze first row
    ws_md.freeze_panes = "A2"

    # Auto-size MetaData columns
    for col_idx, header in enumerate(METADATA_HEADERS, 1):
        col_letter = get_column_letter(col_idx)
        ws_md.column_dimensions[col_letter].width = 40

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save workbook
    os.makedirs(output_dir, exist_ok=True)
    wb.save(filepath)
    print(f"Workbook saved: {filepath}")

    # Validate
    assert os.path.exists(filepath), "File does not exist!"
    assert os.path.getsize(filepath) > 0, "File is empty!"
    wb_check = load_workbook(filepath)
    assert "TestPlan" in wb_check.sheetnames, "TestPlan sheet missing!"
    assert "MetaData" in wb_check.sheetnames, "MetaData sheet missing!"
    tp_sheet = wb_check["TestPlan"]
    assert tp_sheet.max_row == 5, f"Expected 5 rows (1 header + 4 data), got {tp_sheet.max_row}"
    print(f"Validation PASSED: {filename} ({os.path.getsize(filepath)} bytes, {tp_sheet.max_row} rows)")
    wb_check.close()

    return filepath, filename


if __name__ == "__main__":
    out_dir = os.environ.get("OUTPUT_DIR", ".")
    fpath, fname = generate_workbook(out_dir)
    print(f"OUTPUT_FILE={fpath}")
    print(f"OUTPUT_FILENAME={fname}")
