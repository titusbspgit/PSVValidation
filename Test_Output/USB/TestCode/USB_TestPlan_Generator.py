#!/usr/bin/env python3
"""
USB TestPlan Excel Generator
Run this script to generate: USB_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx
Requirements: pip install openpyxl
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os, sys, json

# IST Timezone
IST = timezone(timedelta(hours=5, minutes=30))
now = datetime.now(IST)
timestamp = now.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"

# ============================================================
# JSON DATA - Exact values from aggregated pipeline
# ============================================================
json_data = [
    {
        "Index": "1",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
        "Feature": "USB Full-Speed Device Bulk Transfer",
        "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"",
        "Meta Macros": "MIZAR_USB_DCFG, MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, MIZAR_USB_DEPCMD, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, MIZAR_USB_GEVNTSIZ, MIZAR_USB_GEVNTCOUNT, MIZAR_USB_GCTL, Buffer_PointerLO, Default_Event_Ring_Array, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, DWORD, event_trb_addr, Buffer_PointerLO_1",
        "Meta Arrays": "int buf_data[16];",
        "Speed": "Full Speed",
        "Mode": "Device",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "USB Full-Speed Device Bulk Transfer test. The testcase initializes the USB device controller via nic_programming() (which configures global registers such as MIZAR_USB_GCTL with 0x30c11234 via set_data), enables all IRQs via GIC_EnableAllIRQ(), clears 20 entries of Buffer_PointerLO and event_trb_addr memory regions in a loop. The test proceeds through USB device enumeration including setup_stage(), status_stage(), and enumeration() functions which configure event TRBs and data buffers for device descriptor (0x02000012, length 0x12) and configuration descriptor (0x003c0209, length 0x3c) responses. The set_configuration() function is used to issue endpoint commands by writing MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, and MIZAR_USB_DEPCMD registers with polling for command completion. Bulk data transfer is performed using TRBs configured at Buffer_PointerLO with control field 0x853 and DEPCMD+0x10 with command 0x506, followed by interrupt-driven synchronization via int_pend polling with wait_on(5). The Default_IRQHandler reads MIZAR_USB_GEVNTCOUNT, stores it in event_counter, writes it back to acknowledge events, reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, and calls GIC_ClearIRQ(84).",
        "Test Description": "USB Full-Speed Device Bulk Transfer test. Initializes the USB device controller global registers, enables interrupts, clears buffer and event TRB memory. Performs USB device enumeration including device descriptor and configuration descriptor responses. Configures bulk endpoints using set_configuration with DEPCMD parameter registers. Executes bulk data transfers using TRBs with interrupt-driven synchronization. The interrupt handler reads GEVNTCOUNT for event acknowledgment and clears system-level interrupt status.",
        "Meta Test Steps / Procedure": "extern int int_pend;\nint event_counter;\n\nint test_case() {\n int rd_data,wr_data,port_count,db_offset;\n int buf_data[16],i,bulk,intr,event_comletion,j;\n int hand_shake;\n\n nic_programming();\n GIC_EnableAllIRQ();\n for(j=0;j< 20;j++){\n write_reg(Buffer_PointerLO + jDWORD, 0x0);\n write_reg(event_trb_addr + jDWORD, 0x0);\n }\n\n // ... controller initialization via nic_programming() which includes:\n // write_reg(MIZAR_USB_GCTL,set_data(read_reg(MIZAR_USB_GCTL),0xFFFFFFFF,0x30c11234));\n\n // DEPCMD+0x10 Start Transfer command\n write_reg(MIZAR_USB_DEPCMD+0x10,0x506);\n rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);\n while(rd_data == 0x506){\n wait_on(10);\n rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);\n }\n\n int_pend = 1;\n while(int_pend) { wait_on(5); }\n\n wait_on(5);\n\n int_pend = 1;\n while(int_pend) { wait_on(5); }\n\n // ADDED to check the xfernotready event\n int_pend = 1;\n while(int_pend) { wait_on(5); }\n\n status_stage();\n\n // USB_SET_CONFIGURATION_OR_RESET_TT\n setup_stage();\n\n write_reg(0xA0243ffc,0xdeadbee4);\n\n // GET DESCRIPTOR USB CONFIGURATION\n int_pend = 1;\n while(int_pend) { wait_on(5); }\n\n setup_stage();\n write_reg(0xA0243ffc,0xdeadbee5);\n\n int_pend = 1;\n while(int_pend) { wait_on(5); }\n\n // data stage - configuration descriptor\n //09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301\n\n // enumeration();\n\n // Bulk data transfer TRB setup\n write_reg(event_trb_addr,Buffer_PointerLO);\n write_reg(event_trb_addr+0x8,0x0);\n write_reg(event_trb_addr+0xc,0x853);\n write_reg(Buffer_PointerLO,0x00);\n write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);\n write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);\n write_reg(MIZAR_USB_DEPCMD+0x10,0x506);\n rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);\n while(rd_data == 0x506){\n wait_on(10);\n rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);\n }\n\n // Bulk endpoint TRB with Buffer_PointerLO_1\n // write_reg(event_trb_addr,Buffer_PointerLO_1);\n // write_reg(event_trb_addr+0x8,0x40);\n // write_reg(event_trb_addr+0xc,0x813);\n // write_reg(MIZAR_USB_DEPCMDPAR1+0x40,event_trb_addr);\n // write_reg(MIZAR_USB_DEPCMDPAR0+0x40,0x0);\n // write_reg(MIZAR_USB_DEPCMD+0x40,0x506);\n\n int_pend = 1;\n while(int_pend) { wait_on(5); }\n}\n\nvoid Default_IRQHandler() {\n int rd_data,sysreg_rd_data,event_count;\n int_pend = 0;\n rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);\n rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);\n event_count = read_reg(MIZAR_USB_GEVNTCOUNT);\n event_counter = event_count;\n write_reg(MIZAR_USB_GEVNTCOUNT, event_count);\n if( rd_data && 0x80000000){\n write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,0x80000000);\n }\n GIC_ClearIRQ(84);\n}\n\nvoid setup_stage() {\n int rd_data;\n write_reg(event_trb_addr,Buffer_PointerLO);\n write_reg(event_trb_addr+0x8,0x8);\n write_reg(event_trb_addr+0xc,0x823);\n write_reg(MIZAR_USB_DEPCMDPAR1,event_trb_addr);\n write_reg(MIZAR_USB_DEPCMDPAR0,0x0);\n write_reg(MIZAR_USB_DEPCMD,0x506);\n rd_data = read_reg(MIZAR_USB_DEPCMD);\n while(rd_data == 0x506){\n wait_on(10);\n rd_data = read_reg(MIZAR_USB_DEPCMD);\n }\n int_pend = 1;\n while(int_pend) {\n wait_on(5);\n }\n}\n\nvoid status_stage() {\n int rd_data;\n write_reg(event_trb_addr,Buffer_PointerLO);\n write_reg(event_trb_addr+0x8,0x0);\n write_reg(event_trb_addr+0xc,0x843);\n write_reg(MIZAR_USB_DEPCMDPAR1,event_trb_addr);\n write_reg(MIZAR_USB_DEPCMDPAR0,0x0);\n write_reg(MIZAR_USB_DEPCMD,0x506);\n rd_data = read_reg(MIZAR_USB_DEPCMD);\n while(rd_data == 0x506){\n wait_on(10);\n rd_data = read_reg(MIZAR_USB_DEPCMD);\n }\n int_pend = 1;\n while(int_pend) {\n wait_on(5);\n }\n wait_on(5);\n}\n\nvoid enumeration() {\n // Device descriptor response\n write_reg(event_trb_addr,Buffer_PointerLO);\n write_reg(event_trb_addr+0x8,0x12);\n write_reg(event_trb_addr+0xc,0x853);\n //data\n write_reg(Buffer_PointerLO,0x02000012);\n write_reg(Buffer_PointerLO+0x4,0x40000000);\n write_reg(Buffer_PointerLO+0x8,0x00000000);\n write_reg(Buffer_PointerLO+0xc,0x00000000);\n write_reg(Buffer_PointerLO+0x10,0x00000100);\n write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);\n write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);\n\n // Configuration descriptor response\n write_reg(event_trb_addr,Buffer_PointerLO);\n write_reg(event_trb_addr+0x8,0x3c);\n write_reg(event_trb_addr+0xc,0x853);\n write_reg(Buffer_PointerLO,0x003c0209);\n write_reg(Buffer_PointerLO+0x4,0xe0000101);\n write_reg(Buffer_PointerLO+0x8,0x00000032);\n write_reg(Buffer_PointerLO+0xc,0x00000000);\n write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);\n write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);\n}\n\nvoid set_configuration(int trb_address,int parameter0,int parameter1,int cmd) {\n int read_data;\n write_reg(MIZAR_USB_DEPCMDPAR1+trb_address,parameter1);\n write_reg(MIZAR_USB_DEPCMDPAR0+trb_address,parameter0);\n write_reg(MIZAR_USB_DEPCMD+trb_address,cmd);\n read_data = read_reg(MIZAR_USB_DEPCMD+trb_address);\n while(read_data == cmd){\n wait_on(30);\n read_data = read_reg(MIZAR_USB_DEPCMD+trb_address);\n }\n}",
        "Test Steps / Procedure": "1. Call nic_programming() for USB device controller initialization (configures GCTL with 0x30c11234 and other global registers).\n2. Enable all IRQs via GIC_EnableAllIRQ().\n3. Clear 20 entries of Buffer_PointerLO and event_trb_addr memory regions by writing 0x0 in a loop.\n4. Issue Start Transfer command by writing 0x506 to DEPCMD+0x10, poll until command completes.\n5. Set int_pend = 1, poll with wait_on(5) until interrupt clears int_pend (repeated three times for xfernotready event handling).\n6. Call status_stage() to complete status phase of control transfer.\n7. Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT.\n8. Write debug marker 0xdeadbee4 to address 0xA0243ffc.\n9. Wait for interrupt, then call setup_stage() for GET DESCRIPTOR USB CONFIGURATION.\n10. Write debug marker 0xdeadbee5 to address 0xA0243ffc.\n11. Wait for interrupt, proceed with configuration descriptor data stage.\n12. Configure event TRB with Buffer_PointerLO, transfer length, and control field 0x853.\n13. Write device descriptor data (0x02000012, 0x40000000, etc.) to Buffer_PointerLO.\n14. Write configuration descriptor data (0x003c0209, 0xe0000101, 0x00000032) to Buffer_PointerLO.\n15. Set DEPCMDPAR1+0x10 to event_trb_addr, DEPCMDPAR0+0x10 to 0x0.\n16. Issue DEPCMD+0x10 with 0x506 for bulk endpoint Start Transfer, poll until complete.\n17. Wait for interrupt completion via int_pend polling with wait_on(5).\n18. Default_IRQHandler: clears int_pend, reads GEVNTCOUNT, stores in event_counter, writes back to acknowledge, reads system register status, clears raw interrupt status with 0x80000000, calls GIC_ClearIRQ(84).",
        "Meta Impacted Registers": "GCTL",
        "Impacted Registers": "GCTL",
        "Meta Validation / Acceptance Criteria": "1. After writing 0x506 to MIZAR_USB_DEPCMD+0x10, poll read_reg(MIZAR_USB_DEPCMD+0x10) until value changes from 0x506, indicating Start Transfer command completion.\n2. In setup_stage(): after writing 0x506 to MIZAR_USB_DEPCMD, poll read_reg(MIZAR_USB_DEPCMD) until value changes from 0x506, indicating endpoint command completion.\n3. In status_stage(): after writing 0x506 to MIZAR_USB_DEPCMD, poll read_reg(MIZAR_USB_DEPCMD) until value changes from 0x506, indicating endpoint command completion.\n4. In set_configuration(): after writing cmd to MIZAR_USB_DEPCMD+trb_address, poll read_reg(MIZAR_USB_DEPCMD+trb_address) until value changes from cmd, indicating command completion.\n5. In Default_IRQHandler(): read MIZAR_USB_GEVNTCOUNT to obtain event_count, store in event_counter global, write event_count back to MIZAR_USB_GEVNTCOUNT to acknowledge events; read MIZAR_LSS_SYSREG_RAW_STCR0 and check condition (rd_data && 0x80000000) to verify interrupt source; write MIZAR_LSS_SYSREG_RAW_STCR0 with 0x80000000 to clear raw interrupt status; call GIC_ClearIRQ(84).\n6. Interrupt-driven synchronization: int_pend set to 1 before each wait loop, cleared to 0 by Default_IRQHandler, polled via wait_on(5) in while loop.",
        "Validation / Acceptance Criteria": "1. DEPCMD Start Transfer command (0x506) completes successfully - verified by polling DEPCMD register until value changes.\n2. Endpoint commands in setup_stage and status_stage complete successfully via DEPCMD polling.\n3. set_configuration endpoint commands complete successfully via DEPCMD+trb_address polling.\n4. Interrupt handler correctly reads GEVNTCOUNT, acknowledges events by writing count back, and clears system-level interrupt status.\n5. All interrupt-driven synchronization points complete (int_pend cleared by IRQ handler).\n6. Device descriptor response (18 bytes) and configuration descriptor response (60 bytes) are correctly populated in Buffer_PointerLO memory.",
        "Remarks": "The testcase uses interrupt-driven polling with wait_on(5) which is shorter than other USB transfer tests (Isochronous/Interrupt use wait_on(100)), indicating tighter timing for bulk transfers. The Default_IRQHandler uses logical AND (&&) instead of bitwise AND (&) for checking bit 31 of rd_data, which may be a potential bug. The nic_programming() function body is not available in the retrieved source but is known to configure MIZAR_USB_GCTL with 0x30c11234 via set_data(). Some portions of the test_case() function body (particularly the complete bulk data transfer phase and set_configuration call parameters) are not fully available in the retrieved RAG source context. The MIZAR_USB_DCFG, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, and MIZAR_USB_GEVNTSIZ registers are referenced in Agent 4 mappings but their initialization is likely within nic_programming() which is not visible in the retrieved source. Only GCTL has a matched register_name from Agent 4; all other macros are unresolved."
    }
]

# ============================================================
# WORKBOOK GENERATION
# ============================================================

wb = openpyxl.Workbook()

# ---- TestPlan Sheet ----
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_headers = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# Header formatting
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# Write TestPlan headers
for col_idx, header in enumerate(tp_headers, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    ws_tp.cell(row=row_idx, column=1, value=row_data.get("Index", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=2, value=row_data.get("SS / Module", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=3, value=row_data.get("Feature", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=4, value=row_data.get("Test Case Name", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=5, value=row_data.get("Test Description", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=6, value=row_data.get("Speed", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=7, value=row_data.get("Mode", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=8, value=row_data.get("Memory Start Offset", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=9, value=row_data.get("Memory End Offset", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=10, value=row_data.get("Remarks", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=11, value=row_data.get("Test Steps / Procedure", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=12, value=row_data.get("Impacted Registers", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=13, value=row_data.get("Validation / Acceptance Criteria", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=14, value="").alignment = wrap_alignment  # Code Generation - empty

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns with max width
for col in ws_tp.columns:
    max_length = 0
    col_letter = col[0].column_letter
    for cell in col:
        if cell.value:
            max_length = max(max_length, min(len(str(cell.value)), 80))
    adjusted_width = min(max(max_length + 2, 12), 80)
    ws_tp.column_dimensions[col_letter].width = adjusted_width

# ---- MetaData Sheet ----
ws_md = wb.create_sheet("MetaData")

md_headers = [
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria", "Meta Headers",
    "Meta Macros", "Meta Arrays"
]

# Write MetaData headers
for col_idx, header in enumerate(md_headers, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    ws_md.cell(row=row_idx, column=1, value=row_data.get("Index", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=2, value=row_data.get("Test Case Name", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=3, value=row_data.get("Meta Test Description", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=4, value=row_data.get("Meta Test Steps / Procedure", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=5, value=row_data.get("Meta Impacted Registers", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=6, value=row_data.get("Meta Validation / Acceptance Criteria", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=7, value=row_data.get("Meta Headers", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=8, value=row_data.get("Meta Macros", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=9, value=row_data.get("Meta Arrays", "")).alignment = wrap_alignment

# Freeze first row
ws_md.freeze_panes = "A2"

# Auto-size columns with max width
for col in ws_md.columns:
    max_length = 0
    col_letter = col[0].column_letter
    for cell in col:
        if cell.value:
            max_length = max(max_length, min(len(str(cell.value)), 80))
    adjusted_width = min(max(max_length + 2, 12), 80)
    ws_md.column_dimensions[col_letter].width = adjusted_width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# ============================================================
# SAVE WORKBOOK
# ============================================================
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
wb.save(output_path)
print(f"SAVED: {output_path}")

# ============================================================
# POST-SAVE VALIDATION
# ============================================================
if not os.path.exists(output_path):
    print("VALIDATION: FAILED - File does not exist")
    sys.exit(1)

file_size = os.path.getsize(output_path)
if file_size <= 0:
    print("VALIDATION: FAILED - File size is 0")
    sys.exit(1)

# Reopen and validate
wb2 = openpyxl.load_workbook(output_path)
if "TestPlan" not in wb2.sheetnames:
    print("VALIDATION: FAILED - TestPlan sheet missing")
    sys.exit(1)
if "MetaData" not in wb2.sheetnames:
    print("VALIDATION: FAILED - MetaData sheet missing")
    sys.exit(1)

# Validate MetaData content
ws_md2 = wb2["MetaData"]
meta_fields = {
    3: "Meta Test Description",
    4: "Meta Test Steps / Procedure",
    5: "Meta Impacted Registers",
    6: "Meta Validation / Acceptance Criteria",
    7: "Meta Headers",
    8: "Meta Macros",
    9: "Meta Arrays"
}

validation_passed = True
for col_idx, field_name in meta_fields.items():
    cell_value = ws_md2.cell(row=2, column=col_idx).value
    expected_value = json_data[0].get(field_name, "")
    if cell_value != expected_value:
        print(f"VALIDATION: FAILED - {field_name} mismatch")
        print(f"  Expected: {expected_value[:100]}...")
        print(f"  Got: {str(cell_value)[:100]}...")
        validation_passed = False

wb2.close()

if validation_passed:
    print(f"VALIDATION: PASSED")
    print(f"FILE: {filename}")
    print(f"SIZE: {file_size} bytes")
    print(f"ROWS_TESTPLAN: {len(json_data)}")
    print(f"ROWS_METADATA: {len(json_data)}")
else:
    print("VALIDATION: FAILED")
    sys.exit(1)
