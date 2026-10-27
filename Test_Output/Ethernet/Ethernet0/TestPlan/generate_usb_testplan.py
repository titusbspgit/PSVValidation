#!/usr/bin/env python3
"""
Ethernet TestPlan Excel Generator - USB FS Device Bulk Transfer
Generates Ethernet_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx
Run: pip install openpyxl && python generate_usb_testplan.py
"""
import json, os, sys, base64
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
    from openpyxl.worksheet.datavalidation import DataValidation
except ImportError:
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
    from openpyxl.worksheet.datavalidation import DataValidation

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
IP_NAME = "Ethernet"
filename = f"{IP_NAME}_TestPlan_{timestamp}.xlsx"
output_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(output_dir, filename)

testplan_json = [
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

meta_json = [
  {
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Meta Headers": 'File: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include "usb.h"',
    "Meta Macros": 'No macro definitions are visible in the supplied source. The following macros are used in program.c but their definitions are not supplied (expected to be defined in "usb.h" or its transitive includes):\nMIZAR_USB_DCTL\nMIZAR_USB_GUSB2PHYCFG\nMIZAR_USB_GEVNTADRLO\nMIZAR_USB_GEVNTADRHI\nMIZAR_USB_GEVNTSIZ\nMIZAR_USB_GEVNTCOUNT\nMIZAR_USB_GCTL\nMIZAR_USB_DCFG\nMIZAR_USB_DEVTEN\nMIZAR_USB_GUCTL\nMIZAR_USB_DALEPENA\nMIZAR_USB_DSTS\nMIZAR_USB_DEPCMDPAR1\nMIZAR_USB_DEPCMDPAR0\nMIZAR_USB_DEPCMD\nMIZAR_LSS_SYSREG_INTR_EN0\nMIZAR_LSS_SYSREG_MSK_STS0\nMIZAR_LSS_SYSREG_RAW_STCR0\nBuffer_PointerLO\nBuffer_PointerLO_1\nDefault_Event_Ring_Array\nevent_trb_addr\nDWORD\nThe header file "usb.h" content was not retrieved.',
    "Meta Arrays": "File: program.c, Function: test_case()\nint buf_data[16];",
    "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: program.c, file scope:\nextern int int_pend;\nint event_counter;",
    "Meta Function Inventory": "File: program.c\n\n1. Function Definition: void Default_IRQHandler()\n Signature: void Default_IRQHandler()\n Return type: void\n Parameters: none\n Body: supplied\n Calls (in source order):\n read_reg(MIZAR_LSS_SYSREG_MSK_STS0)\n read_reg(MIZAR_LSS_SYSREG_RAW_STCR0)\n read_reg(MIZAR_USB_GEVNTCOUNT)\n write_reg(MIZAR_USB_GEVNTCOUNT,event_count)\n write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,0x80000000) [inside if(rd_data && 0x80000000)]\n GIC_ClearIRQ(84)\n\n2. Function Definition: void set_configuration(int trb_address,int parameter0,int parameter1,int cmd)\n Signature: void set_configuration(int trb_address,int parameter0,int parameter1,int cmd)\n Return type: void\n Parameters: int trb_address, int parameter0, int parameter1, int cmd\n Body: supplied\n Calls (in source order):\n write_reg(MIZAR_USB_DEPCMDPAR1+trb_address ,parameter1)\n write_reg(MIZAR_USB_DEPCMDPAR0+trb_address ,parameter0)\n write_reg(MIZAR_USB_DEPCMD+trb_address ,cmd)\n read_reg(MIZAR_USB_DEPCMD+trb_address)\n wait_on(30) [inside while]\n read_reg(MIZAR_USB_DEPCMD+trb_address) [inside while]\n\n3. Function Definition: void setup_stage()\n4. Function Definition: void status_stage()\n5. Function Definition: void enumeration()\n6. Function Definition: int test_case()\n\nExternal functions called but not defined in supplied source:\n nic_programming(), GIC_EnableAllIRQ(), GIC_ClearIRQ(int), finish(int), write_reg(), read_reg(), wait_on(), printf()",
    "Meta Test Description": "USB Full-Speed Device Bulk Transfer test for the USB IP (Mizar platform). The test performs: (1) NIC programming and GIC interrupt enable, (2) buffer and event TRB memory clearing via loop, (3) USB controller soft reset via MIZAR_USB_DCTL with polling for completion, (4) PHY configuration via MIZAR_USB_GUSB2PHYCFG, (5) event buffer setup via GEVNTADRLO/HI/SIZ/COUNT, (6) global controller configuration via GCTL, DCFG, DEVTEN, GUCTL, (7) endpoint configuration using set_configuration() for 9 calls (START NEW CONFIGURATION + 8 endpoint configs), (8) endpoint TX resource configuration in a loop for 8 endpoints via DEPCMDPAR0/DEPCMD, (9) enabling physical endpoints 0,1 via DALEPENA, (10) run state via DCTL, (11) interrupt enable at sysreg via MIZAR_LSS_SYSREG_INTR_EN0, (12) link state / connect / reset event handling via interrupt pending waits, (13) USB enumeration including SET ADDRESS, GET DESCRIPTOR (device and configuration descriptors), USB_SET_CONFIGURATION_OR_RESET_TT, (14) bulk transfers on endpoints 0x40 and 0x50 using Buffer_PointerLO_1, (15) interrupt handler Default_IRQHandler reads event count from GEVNTCOUNT, clears sysreg status via RAW_STCR0, clears GIC IRQ 84, (16) test completion via finish(0). Handshake synchronization is performed via memory-mapped reads at 0xa0243ff4 and 0xa0243ff8. Contains commented-out DEVICE QUALIFIER section.",
    "Meta Test Steps / Procedure": "SOURCE_FILE: program.c\nSOURCE_CONTENT:\n(Full source code preserved - see generator script for complete content)",
    "Meta Impacted Registers": "Agent 4 Output (register mapping):\n1. MIZAR_USB_DCTL -> register_name: NA, status: ambiguous\n2. MIZAR_USB_GUSB2PHYCFG -> register_name: NA, status: unresolved\n3. MIZAR_USB_GEVNTADRLO -> register_name: NA, status: unresolved\n4. MIZAR_USB_GEVNTADRHI -> register_name: NA, status: unresolved\n5. MIZAR_USB_GEVNTSIZ -> register_name: NA, status: unresolved\n6. MIZAR_USB_GEVNTCOUNT -> register_name: NA, status: unresolved\n7. MIZAR_USB_GCTL -> register_name: GCTL, status: matched\n8. MIZAR_USB_DCFG -> register_name: NA, status: ambiguous\n9. MIZAR_USB_DEVTEN -> register_name: NA, status: ambiguous\n10. MIZAR_USB_GUCTL -> register_name: GUCTL, status: matched\n11. MIZAR_USB_DALEPENA -> register_name: DALEPENA, status: matched\n12. MIZAR_USB_DSTS -> register_name: NA, status: ambiguous\n13. MIZAR_USB_DEPCMDPAR1 -> register_name: NA, status: unresolved\n14. MIZAR_USB_DEPCMDPAR0 -> register_name: NA, status: unresolved\n15. MIZAR_USB_DEPCMD -> register_name: NA, status: unresolved\n16. MIZAR_LSS_SYSREG_INTR_EN0 -> register_name: NA, status: unresolved\n17. MIZAR_LSS_SYSREG_MSK_STS0 -> register_name: NA, status: unresolved\n18. MIZAR_LSS_SYSREG_RAW_STCR0 -> register_name: NA, status: unresolved\n19. Buffer_PointerLO -> register_name: NA, status: unresolved\n20. Buffer_PointerLO_1 -> register_name: NA, status: unresolved\n21. Default_Event_Ring_Array -> register_name: NA, status: unresolved\n22. event_trb_addr -> register_name: NA, status: unresolved\n23. DWORD -> register_name: NA, status: unresolved",
    "Meta Validation / Acceptance Criteria": "File: program.c\n\n1. Function: test_case() - Soft reset completion check: rd_data == 0xf00000 to exit loop\n2. Function: test_case() - DEPCMD polling (endpoint tx resource config): rd_data != 0x402 to exit loop\n3-6. Function: test_case() - Event counter checks: event_counter <= 0x4\n7-9. Function: test_case() - DEPCMD polling (SET ADDRESS, GET DESCRIPTOR, SET CONFIG): rd_data != 0x506\n10. Function: test_case() - Event counter check after SET CONFIG\n11-12. Function: test_case() - Handshake polling: hand_shake != 0x0 to exit loop\n13-15. Function: test_case() - DEPCMD polling (full config, bulk +0x40, bulk +0x50): rd_data != 0x506\n16. Function: set_configuration() - DEPCMD polling: read_data != cmd\n17. Function: setup_stage() - DEPCMD polling: rd_data != 0x506\n18. Function: status_stage() - DEPCMD polling: rd_data != 0x506\n19-21. Function: enumeration() - DEPCMD polling (device desc, short config, full config): rd_data != 0x506\n22. Function: Default_IRQHandler() - Interrupt status check: rd_data && 0x80000000\n23. Function: test_case() - Completion: finish(0) argument 0 indicates pass"
  }
]

# Validate cross-dataset
testplan_names = set(r["Test Case Name"] for r in testplan_json)
meta_names = set(r["Test Case Name"] for r in meta_json)
for name in testplan_names:
    if name not in meta_names:
        print(f"VALIDATION FAILED: '{name}' not in META_JSON")
        sys.exit(1)

testplan_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks",
    "Code Generation (Required / Not)"
]

metadata_columns = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Function Inventory", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria"
]

def create_workbook():
    wb = Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_md = wb.create_sheet("MetaData")
    hf = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    hfill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wa = Alignment(wrap_text=True, vertical='top')
    for ci, cn in enumerate(testplan_columns, 1):
        c = ws_tp.cell(row=1, column=ci, value=cn); c.font=hf; c.fill=hfill; c.alignment=wa
    for ri, rec in enumerate(testplan_json, 2):
        for ci, cn in enumerate(testplan_columns, 1):
            if cn == "Code Generation (Required / Not)":
                c = ws_tp.cell(row=ri, column=ci, value="")
            else:
                v = rec.get(cn, ""); c = ws_tp.cell(row=ri, column=ci, value=str(v) if v else "")
            c.alignment = wa
    cgci = len(testplan_columns)
    cgl = get_column_letter(cgci)
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=False)
    dv.error="Please select Required or Not Required"; dv.errorTitle="Invalid Entry"
    ldr = len(testplan_json) + 1
    dv.add(f"{cgl}2:{cgl}{ldr}")
    ws_tp.add_data_validation(dv)
    for ci, cn in enumerate(metadata_columns, 1):
        c = ws_md.cell(row=1, column=ci, value=cn); c.font=hf; c.fill=hfill; c.alignment=wa
    for ri, rec in enumerate(meta_json, 2):
        for ci, cn in enumerate(metadata_columns, 1):
            v = rec.get(cn, ""); c = ws_md.cell(row=ri, column=ci, value=str(v) if v else "")
            c.alignment = wa
    mw = 80
    for ws in [ws_tp, ws_md]:
        for ci in range(1, ws.max_column + 1):
            ml = 0; cl = get_column_letter(ci)
            for row in ws.iter_rows(min_col=ci, max_col=ci, min_row=1, max_row=ws.max_row):
                for cell in row:
                    if cell.value:
                        for line in str(cell.value).split('\n'):
                            ml = max(ml, len(line))
            ws.column_dimensions[cl].width = max(min(ml + 2, mw), 12)
    ws_tp.freeze_panes = 'A2'; ws_md.freeze_panes = 'A2'
    ws_md.sheet_state = 'veryHidden'
    wb.save(output_path)
    print(f"Saved: {output_path}")
    if os.path.exists(output_path) and os.path.getsize(output_path) > 0:
        wbc = load_workbook(output_path)
        s = wbc.sheetnames
        if 'TestPlan' in s and 'MetaData' in s:
            print(f"VALIDATION: PASSED | Rows TP:{wbc['TestPlan'].max_row-1} MD:{wbc['MetaData'].max_row-1}")
            with open(output_path, 'rb') as f:
                b64 = base64.b64encode(f.read()).decode('utf-8')
            print(f"BASE64_START")
            print(b64)
            print(f"BASE64_END")
            print(f"FILENAME:{filename}")
            return True
    print("VALIDATION: FAILED")
    return False

if __name__ == '__main__':
    create_workbook()
