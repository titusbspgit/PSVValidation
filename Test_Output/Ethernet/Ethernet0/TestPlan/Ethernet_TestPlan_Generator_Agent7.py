#!/usr/bin/env python3
"""
Agent 7: Excel Generation and GitHub Publishing
Generates Ethernet_TestPlan_<YYYYMMDD_HHMMSS>.xlsx workbook
with TestPlan and MetaData (veryHidden) sheets.
"""

import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# ============================================================
# DATA
# ============================================================

TESTPLAN_JSON = [
  {
    "Index": "1",
    "SS / Module": "USB",
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Feature": "Full-Speed Device Bulk Transfer",
    "Speed": "Full-Speed",
    "Mode": "Device",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This test validates USB Full-Speed Device mode Bulk Transfer functionality on the Mizar platform. The test initializes the USB controller by performing NIC programming and enabling GIC interrupts, clears buffer and event TRB memory regions, executes a USB controller soft reset and polls for its completion, configures the USB 2.0 PHY, sets up the event buffer (address low/high, size, and count), and programs global controller settings including port direction, device configuration, device event enable, and user control registers. The test then configures 8 device endpoints (START NEW CONFIGURATION followed by 8 endpoint configurations) and allocates TX resources for each endpoint. Physical endpoints 0 and 1 are enabled, the controller is placed into run state, and system-level interrupts are enabled. The test handles link state, connect, and reset events via interrupt-driven handshaking. USB enumeration is performed including SET ADDRESS, GET DESCRIPTOR (device descriptor and configuration descriptor), and USB SET CONFIGURATION. After enumeration, bulk data transfers are initiated on two bulk endpoints. An interrupt handler services USB events by reading the event count, acknowledging the event buffer, clearing system register interrupt status, and clearing the GIC IRQ. The test completes successfully upon finishing all bulk transfers.",
    "Test Steps / Procedure": "1. Perform NIC programming and enable all GIC interrupts.\n2. Clear buffer and event TRB memory regions by writing zeros in a loop (20 iterations).\n3. Initiate USB controller soft reset by writing to the Device Control register (DCTL) and poll until the reset completes.\n4. Configure the USB 2.0 PHY via the PHY Configuration register (GUSB2PHYCFG).\n5. Set up the event buffer by programming the Event Address Low (GEVNTADRLO), Event Address High (GEVNTADRHI), Event Size (GEVNTSIZ), and Event Count (GEVNTCOUNT) registers.\n6. Read the Global Control register (GCTL), then write the port direction configuration value.\n7. Read and program the Device Configuration register (DCFG) for Full-Speed operation.\n8. Enable device events (disconnect, reset, connection done, link state change, wakeup) via the Device Event Enable register (DEVTEN).\n9. Read and program the User Control register (GUCTL).\n10. Issue START NEW CONFIGURATION command via the endpoint command interface (DEPCMDPAR1, DEPCMDPAR0, DEPCMD).\n11. Configure 8 endpoints by issuing endpoint configuration commands with appropriate parameters and polling for command completion.\n12. Configure TX resources for all 8 endpoints in a loop, issuing resource allocation commands and polling for completion.\n13. Enable physical endpoints 0 and 1 via the Active Endpoint Enable register (DALEPENA).\n14. Set the controller to run state via the Device Control register (DCTL).\n15. Enable system-level USB interrupt via the System Register Interrupt Enable register.\n16. Wait for link state, connect, and reset events via interrupt-driven handshaking; conditionally wait for additional events based on event counter value.\n17. Re-program the Device Configuration register (DCFG) after reset events. Read Device Status register (DSTS).\n18. Perform USB enumeration Setup Stage (prepare TRB, issue Start Transfer command via DEPCMDPAR1/DEPCMDPAR0/DEPCMD, wait for interrupt completion).\n19. Update PHY configuration (GUSB2PHYCFG) and Device Configuration (DCFG) for SET ADDRESS.\n20. Prepare SET ADDRESS data TRB and issue Start Transfer command on the IN endpoint; poll for command completion.\n21. Wait for handshake synchronization from the external agent.\n22. Execute full enumeration sequence: GET DESCRIPTOR (device descriptor) with setup, data, and status stages; GET DESCRIPTOR (short configuration descriptor) with setup, data, and status stages.\n23. Wait for second handshake synchronization.\n24. Perform GET DESCRIPTOR (short configuration descriptor) with setup, data, and status stages.\n25. Perform USB SET CONFIGURATION with setup and status data stages; wait for event completion and handshake synchronization.\n26. Execute second full enumeration sequence (device and configuration descriptors).\n27. Perform GET DESCRIPTOR (full configuration descriptor with interface and endpoint descriptors) with setup, data, and status stages.\n28. Initiate Bulk OUT transfer on the first bulk endpoint: prepare TRB with buffer pointer, issue Start Transfer command, poll for completion, and wait for transfer complete interrupt.\n29. Initiate Bulk OUT transfer on the second bulk endpoint: prepare TRB with buffer pointer, issue Start Transfer command, poll for completion, and wait for transfer complete interrupt.\n30. Signal test completion and call finish with pass status.\n\nInterrupt Handler Steps:\na. On interrupt, clear the interrupt pending flag.\nb. Read the System Register Masked Status and Raw Status registers.\nc. Read the Event Count register (GEVNTCOUNT) and store the event count.\nd. Write back the event count to GEVNTCOUNT to acknowledge processed events.\ne. If the raw status indicates a USB interrupt, clear the raw status bit.\nf. Clear the GIC IRQ.",
    "Impacted Registers": "GCTL, GUCTL, DALEPENA, DCFG, DCTL, DEVTEN, DSTS, GUSB2PHYCFG, GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, GEVNTCOUNT, DEPCMDPAR2, DEPCMDPAR1, DEPCMDPAR0, DEPCMD",
    "Validation / Acceptance Criteria": "1. USB controller soft reset must complete successfully: the DCTL register must reflect the expected post-reset value after polling.\n2. All endpoint configuration commands (START NEW CONFIGURATION and 8 endpoint configs) must complete: the DEPCMD register must clear the active command indicator for each command.\n3. All endpoint TX resource allocation commands (for 8 endpoints) must complete: the DEPCMD register must transition away from the issued command value.\n4. All Start Transfer commands issued during enumeration (setup stage, data stage, status stage) must complete: the DEPCMD register must clear the active bit.\n5. Link state, connect, and reset events must be received via interrupts: the interrupt pending flag must be cleared by the interrupt handler.\n6. Event counter checks must pass: if the event counter is within the expected threshold, additional interrupt events must be successfully received.\n7. Handshake synchronization with the external agent must succeed: the handshake memory locations must return non-zero values.\n8. All bulk transfer Start Transfer commands on both bulk endpoints must complete: the DEPCMD register must clear the active command indicator.\n9. The interrupt handler must correctly read the event count from GEVNTCOUNT, acknowledge processed events by writing back to GEVNTCOUNT, clear the system register raw status, and clear the GIC IRQ.\n10. The test must reach completion and call finish with a pass (0) argument, indicating all transfers and enumeration steps completed without error.",
    "Remarks": "The test includes a commented-out DEVICE QUALIFIER descriptor response section that is not active in the current implementation. Handshake synchronization with an external agent is performed via memory-mapped polling at dedicated handshake locations. The interrupt handler uses GIC IRQ 84 for USB event servicing. Multiple conditional event counter checks are used to handle variable timing of link state and reset events. The PHY configuration (GUSB2PHYCFG) is updated during SET ADDRESS to adjust for Full-Speed operation parameters. Three system-level registers (LSS_SYSREG_INTR_EN0, LSS_SYSREG_MSK_STS0, LSS_SYSREG_RAW_STCR0) are used for interrupt management but could not be mapped to the USB register specification as they belong to a separate LSS SYSREG module."
  }
]

META_JSON = [
  {
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Meta Headers": 'File: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"',
    "Meta Macros": 'No macro definitions are visible in the supplied source. The following macros are used in program.c but their definitions are not supplied (expected to be defined in \"usb.h\" or its transitive includes):\nMIZAR_USB_DCTL\nMIZAR_USB_GUSB2PHYCFG\nMIZAR_USB_GEVNTADRLO\nMIZAR_USB_GEVNTADRHI\nMIZAR_USB_GEVNTSIZ\nMIZAR_USB_GEVNTCOUNT\nMIZAR_USB_GCTL\nMIZAR_USB_DCFG\nMIZAR_USB_DEVTEN\nMIZAR_USB_GUCTL\nMIZAR_USB_DALEPENA\nMIZAR_USB_DSTS\nMIZAR_USB_DEPCMDPAR1\nMIZAR_USB_DEPCMDPAR0\nMIZAR_USB_DEPCMD\nMIZAR_LSS_SYSREG_INTR_EN0\nMIZAR_LSS_SYSREG_MSK_STS0\nMIZAR_LSS_SYSREG_RAW_STCR0\nBuffer_PointerLO\nBuffer_PointerLO_1\nDefault_Event_Ring_Array\nevent_trb_addr\nDWORD\nThe header file \"usb.h\" content was not retrieved.',
    "Meta Arrays": 'File: program.c, Function: test_case()\nint buf_data[16];',
    "Meta Types": 'TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: program.c, file scope:\nextern int int_pend;\nint event_counter;',
    "Meta Test Description": 'USB Full-Speed Device Bulk Transfer test for the USB IP (Mizar platform). The test performs: (1) NIC programming and GIC interrupt enable, (2) buffer and event TRB memory clearing via loop, (3) USB controller soft reset via MIZAR_USB_DCTL with polling for completion, (4) PHY configuration via MIZAR_USB_GUSB2PHYCFG, (5) event buffer setup via GEVNTADRLO/HI/SIZ/COUNT, (6) global controller configuration via GCTL, DCFG, DEVTEN, GUCTL, (7) endpoint configuration using set_configuration() for 9 calls (START NEW CONFIGURATION + 8 endpoint configs), (8) endpoint TX resource configuration in a loop for 8 endpoints via DEPCMDPAR0/DEPCMD, (9) enabling physical endpoints 0,1 via DALEPENA, (10) run state via DCTL, (11) interrupt enable at sysreg via MIZAR_LSS_SYSREG_INTR_EN0, (12) link state / connect / reset event handling via interrupt pending waits, (13) USB enumeration including SET ADDRESS, GET DESCRIPTOR (device and configuration descriptors), USB_SET_CONFIGURATION_OR_RESET_TT, (14) bulk transfers on endpoints 0x40 and 0x50 using Buffer_PointerLO_1, (15) interrupt handler Default_IRQHandler reads event count from GEVNTCOUNT, clears sysreg status via RAW_STCR0, clears GIC IRQ 84, (16) test completion via finish(0). Handshake synchronization is performed via memory-mapped reads at 0xa0243ff4 and 0xa0243ff8. Contains commented-out DEVICE QUALIFIER section.',
    "Meta Test Steps / Procedure": open('meta_test_steps.txt', 'r').read() if os.path.exists('meta_test_steps.txt') else 'SEE meta_test_steps.txt',
    "Meta Impacted Registers": 'Agent 4 Output (register mapping):\n1. MIZAR_USB_DCTL -> register_name: NA, status: ambiguous\n2. MIZAR_USB_GUSB2PHYCFG -> register_name: NA, status: unresolved\n3. MIZAR_USB_GEVNTADRLO -> register_name: NA, status: unresolved\n4. MIZAR_USB_GEVNTADRHI -> register_name: NA, status: unresolved\n5. MIZAR_USB_GEVNTSIZ -> register_name: NA, status: unresolved\n6. MIZAR_USB_GEVNTCOUNT -> register_name: NA, status: unresolved\n7. MIZAR_USB_GCTL -> register_name: GCTL, status: matched\n8. MIZAR_USB_DCFG -> register_name: NA, status: ambiguous\n9. MIZAR_USB_DEVTEN -> register_name: NA, status: ambiguous\n10. MIZAR_USB_GUCTL -> register_name: GUCTL, status: matched\n11. MIZAR_USB_DALEPENA -> register_name: DALEPENA, status: matched\n12. MIZAR_USB_DSTS -> register_name: NA, status: ambiguous\n13. MIZAR_USB_DEPCMDPAR1 -> register_name: NA, status: unresolved\n14. MIZAR_USB_DEPCMDPAR0 -> register_name: NA, status: unresolved\n15. MIZAR_USB_DEPCMD -> register_name: NA, status: unresolved\n16. MIZAR_LSS_SYSREG_INTR_EN0 -> register_name: NA, status: unresolved\n17. MIZAR_LSS_SYSREG_MSK_STS0 -> register_name: NA, status: unresolved\n18. MIZAR_LSS_SYSREG_RAW_STCR0 -> register_name: NA, status: unresolved\n19. Buffer_PointerLO -> register_name: NA, status: unresolved\n20. Buffer_PointerLO_1 -> register_name: NA, status: unresolved\n21. Default_Event_Ring_Array -> register_name: NA, status: unresolved\n22. event_trb_addr -> register_name: NA, status: unresolved\n23. DWORD -> register_name: NA, status: unresolved',
    "Meta Validation / Acceptance Criteria": 'SEE meta_validation.txt'
  }
]

IP_NAME = "Ethernet"

# ============================================================
# WORKBOOK GENERATION
# ============================================================

def generate_workbook():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"{IP_NAME}_TestPlan_{timestamp}.xlsx"

    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    tp_headers = [
        "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
        "Memory Start Offset", "Memory End Offset", "Test Description",
        "Test Steps / Procedure", "Impacted Registers",
        "Validation / Acceptance Criteria", "Remarks",
        "Code Generation (Required / Not)"
    ]

    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_align = Alignment(wrap_text=True, vertical="top")

    for col_idx, header in enumerate(tp_headers, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=header)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    for row_idx, tc in enumerate(TESTPLAN_JSON, 2):
        ws_tp.cell(row=row_idx, column=1, value=tc.get("Index", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=2, value=tc.get("SS / Module", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=3, value=tc.get("Feature", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=4, value=tc.get("Test Case Name", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=5, value=tc.get("Speed", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=6, value=tc.get("Mode", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=7, value=tc.get("Memory Start Offset", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=8, value=tc.get("Memory End Offset", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=9, value=tc.get("Test Description", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=10, value=tc.get("Test Steps / Procedure", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=11, value=tc.get("Impacted Registers", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=12, value=tc.get("Validation / Acceptance Criteria", "")).alignment = wrap_align
        ws_tp.cell(row=row_idx, column=13, value=tc.get("Remarks", "")).alignment = wrap_align
        # Column 14 (Code Generation) left blank

    # Data validation dropdown for Code Generation column
    dv = DataValidation(
        type="list",
        formula1='"Required,Not Required"',
        allow_blank=True
    )
    dv.error = "Please select Required or Not Required"
    dv.errorTitle = "Invalid Entry"
    dv.prompt = "Select Code Generation option"
    dv.promptTitle = "Code Generation"
    last_data_row = 1 + len(TESTPLAN_JSON)
    dv.add(f"N2:N{last_data_row}")
    ws_tp.add_data_validation(dv)

    ws_tp.freeze_panes = "A2"

    # Auto-size columns
    for col_idx in range(1, len(tp_headers) + 1):
        max_len = len(str(tp_headers[col_idx - 1]))
        for row in ws_tp.iter_rows(min_row=2, max_row=last_data_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_len:
                            max_len = len(line)
        width = min(max_len + 2, 80)
        ws_tp.column_dimensions[get_column_letter(col_idx)].width = width

    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet(title="MetaData")

    md_headers = [
        "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
        "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
        "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
    ]

    for col_idx, header in enumerate(md_headers, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=header)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    for row_idx, meta in enumerate(META_JSON, 2):
        ws_md.cell(row=row_idx, column=1, value=meta.get("Test Case Name", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=2, value=meta.get("Meta Headers", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=3, value=meta.get("Meta Macros", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=4, value=meta.get("Meta Arrays", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=5, value=meta.get("Meta Types", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=6, value=meta.get("Meta Test Description", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=7, value=meta.get("Meta Test Steps / Procedure", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=8, value=meta.get("Meta Impacted Registers", "")).alignment = wrap_align
        ws_md.cell(row=row_idx, column=9, value=meta.get("Meta Validation / Acceptance Criteria", "")).alignment = wrap_align

    ws_md.freeze_panes = "A2"
    ws_md.sheet_state = "veryHidden"

    for col_idx in range(1, len(md_headers) + 1):
        max_len = len(str(md_headers[col_idx - 1]))
        md_last_row = 1 + len(META_JSON)
        for row in ws_md.iter_rows(min_row=2, max_row=md_last_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_len:
                            max_len = len(line)
        width = min(max_len + 2, 80)
        ws_md.column_dimensions[get_column_letter(col_idx)].width = width

    wb.save(filename)
    print(f"Workbook saved: {filename}")

    # Validate
    wb2 = load_workbook(filename)
    assert "TestPlan" in wb2.sheetnames
    assert "MetaData" in wb2.sheetnames
    tp_rows = wb2["TestPlan"].max_row - 1
    md_rows = wb2["MetaData"].max_row - 1
    assert tp_rows == len(TESTPLAN_JSON)
    assert md_rows == len(META_JSON)
    print(f"Validation PASSED: TestPlan={tp_rows} rows, MetaData={md_rows} rows")
    return filename

if __name__ == "__main__":
    generate_workbook()
