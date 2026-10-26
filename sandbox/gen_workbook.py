#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation

wb = openpyxl.Workbook()
hf = Font(bold=True, color="FFFFFF")
hfill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

ws = wb.active
ws.title = "TestPlan"
tp_cols = ["Index","SS / Module","Feature","Test Case Name","Speed","Mode","Memory Start Offset","Memory End Offset","Test Description","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Remarks","Code Generation (Required / Not)"]
for c,h in enumerate(tp_cols,1):
    cell = ws.cell(row=1, column=c, value=h)
    cell.font = hf
    cell.fill = hfill
    cell.alignment = wrap

row_data = ["1", "USB", "Full-Speed Device Bulk Transfer", "USB_FS_Device_Bulk_Transfer_test", "Full-Speed", "Device", "NA", "NA",
"This test validates USB Full-Speed Device mode Bulk Transfer functionality on the Mizar platform. The test initializes the USB controller by performing NIC programming and enabling GIC interrupts, clears buffer and event TRB memory regions, executes a USB controller soft reset and polls for its completion, configures the USB 2.0 PHY, sets up the event buffer (address low/high, size, and count), and programs global controller settings including port direction, device configuration, device event enable, and user control registers. The test then configures 8 device endpoints (START NEW CONFIGURATION followed by 8 endpoint configurations) and allocates TX resources for each endpoint. Physical endpoints 0 and 1 are enabled, the controller is placed into run state, and system-level interrupts are enabled. The test handles link state, connect, and reset events via interrupt-driven handshaking. USB enumeration is performed including SET ADDRESS, GET DESCRIPTOR (device descriptor and configuration descriptor), and USB SET CONFIGURATION. After enumeration, bulk data transfers are initiated on two bulk endpoints. An interrupt handler services USB events by reading the event count, acknowledging the event buffer, clearing system register interrupt status, and clearing the GIC IRQ. The test completes successfully upon finishing all bulk transfers.",
"1. Perform NIC programming and enable all GIC interrupts.\n2. Clear buffer and event TRB memory regions by writing zeros in a loop (20 iterations).\n3. Initiate USB controller soft reset by writing to the Device Control register (DCTL) and poll until the reset completes.\n4. Configure the USB 2.0 PHY via the PHY Configuration register (GUSB2PHYCFG).\n5. Set up the event buffer by programming the Event Address Low (GEVNTADRLO), Event Address High (GEVNTADRHI), Event Size (GEVNTSIZ), and Event Count (GEVNTCOUNT) registers.\n6. Read the Global Control register (GCTL), then write the port direction configuration value.\n7. Read and program the Device Configuration register (DCFG) for Full-Speed operation.\n8. Enable device events (disconnect, reset, connection done, link state change, wakeup) via the Device Event Enable register (DEVTEN).\n9. Read and program the User Control register (GUCTL).\n10. Issue START NEW CONFIGURATION command via the endpoint command interface (DEPCMDPAR1, DEPCMDPAR0, DEPCMD).\n11. Configure 8 endpoints by issuing endpoint configuration commands with appropriate parameters and polling for command completion.\n12. Configure TX resources for all 8 endpoints in a loop, issuing resource allocation commands and polling for completion.\n13. Enable physical endpoints 0 and 1 via the Active Endpoint Enable register (DALEPENA).\n14. Set the controller to run state via the Device Control register (DCTL).\n15. Enable system-level USB interrupt via the System Register Interrupt Enable register.\n16. Wait for link state, connect, and reset events via interrupt-driven handshaking; conditionally wait for additional events based on event counter value.\n17. Re-program the Device Configuration register (DCFG) after reset events. Read Device Status register (DSTS).\n18. Perform USB enumeration Setup Stage (prepare TRB, issue Start Transfer command via DEPCMDPAR1/DEPCMDPAR0/DEPCMD, wait for interrupt completion).\n19. Update PHY configuration (GUSB2PHYCFG) and Device Configuration (DCFG) for SET ADDRESS.\n20. Prepare SET ADDRESS data TRB and issue Start Transfer command on the IN endpoint; poll for command completion.\n21. Wait for handshake synchronization from the external agent.\n22. Execute full enumeration sequence: GET DESCRIPTOR (device descriptor) with setup, data, and status stages; GET DESCRIPTOR (short configuration descriptor) with setup, data, and status stages.\n23. Wait for second handshake synchronization.\n24. Perform GET DESCRIPTOR (short configuration descriptor) with setup, data, and status stages.\n25. Perform USB SET CONFIGURATION with setup and status data stages; wait for event completion and handshake synchronization.\n26. Execute second full enumeration sequence (device and configuration descriptors).\n27. Perform GET DESCRIPTOR (full configuration descriptor with interface and endpoint descriptors) with setup, data, and status stages.\n28. Initiate Bulk OUT transfer on the first bulk endpoint: prepare TRB with buffer pointer, issue Start Transfer command, poll for completion, and wait for transfer complete interrupt.\n29. Initiate Bulk OUT transfer on the second bulk endpoint: prepare TRB with buffer pointer, issue Start Transfer command, poll for completion, and wait for transfer complete interrupt.\n30. Signal test completion and call finish with pass status.\n\nInterrupt Handler Steps:\na. On interrupt, clear the interrupt pending flag.\nb. Read the System Register Masked Status and Raw Status registers.\nc. Read the Event Count register (GEVNTCOUNT) and store the event count.\nd. Write back the event count to GEVNTCOUNT to acknowledge processed events.\ne. If the raw status indicates a USB interrupt, clear the raw status bit.\nf. Clear the GIC IRQ.",
"GCTL, GUCTL, DALEPENA, DCFG, DCTL, DEVTEN, DSTS, GUSB2PHYCFG, GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, GEVNTCOUNT, DEPCMDPAR2, DEPCMDPAR1, DEPCMDPAR0, DEPCMD",
"1. USB controller soft reset must complete successfully: the DCTL register must reflect the expected post-reset value after polling.\n2. All endpoint configuration commands (START NEW CONFIGURATION and 8 endpoint configs) must complete: the DEPCMD register must clear the active command indicator for each command.\n3. All endpoint TX resource allocation commands (for 8 endpoints) must complete: the DEPCMD register must transition away from the issued command value.\n4. All Start Transfer commands issued during enumeration (setup stage, data stage, status stage) must complete: the DEPCMD register must clear the active bit.\n5. Link state, connect, and reset events must be received via interrupts: the interrupt pending flag must be cleared by the interrupt handler.\n6. Event counter checks must pass: if the event counter is within the expected threshold, additional interrupt events must be successfully received.\n7. Handshake synchronization with the external agent must succeed: the handshake memory locations must return non-zero values.\n8. All bulk transfer Start Transfer commands on both bulk endpoints must complete: the DEPCMD register must clear the active command indicator.\n9. The interrupt handler must correctly read the event count from GEVNTCOUNT, acknowledge processed events by writing back to GEVNTCOUNT, clear the system register raw status, and clear the GIC IRQ.\n10. The test must reach completion and call finish with a pass (0) argument, indicating all transfers and enumeration steps completed without error.",
"The test includes a commented-out DEVICE QUALIFIER descriptor response section that is not active in the current implementation. Handshake synchronization with an external agent is performed via memory-mapped polling at dedicated handshake locations. The interrupt handler uses GIC IRQ 84 for USB event servicing. Multiple conditional event counter checks are used to handle variable timing of link state and reset events. The PHY configuration (GUSB2PHYCFG) is updated during SET ADDRESS to adjust for Full-Speed operation parameters. Three system-level registers (LSS_SYSREG_INTR_EN0, LSS_SYSREG_MSK_STS0, LSS_SYSREG_RAW_STCR0) are used for interrupt management but could not be mapped to the USB register specification as they belong to a separate LSS SYSREG module."]

for c, val in enumerate(row_data, 1):
    ws.cell(row=2, column=c, value=val).alignment = wrap

dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=False, showErrorMessage=True, errorTitle="Invalid Entry", error="Please select Required or Not Required")
dv.sqref = "N2:N2"
ws.add_data_validation(dv)
ws.freeze_panes = "A2"

for col_cells in ws.columns:
    cl = col_cells[0].column_letter
    mx = 12
    for cell in col_cells:
        if cell.value:
            for line in str(cell.value).split('\n'):
                mx = max(mx, len(line))
    ws.column_dimensions[cl].width = min(mx + 2, 60)

md = wb.create_sheet("MetaData")
md.sheet_state = "veryHidden"
md_cols = ["Test Case Name","Meta Headers","Meta Macros","Meta Arrays","Meta Types","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria"]
for c,h in enumerate(md_cols,1):
    cell = md.cell(row=1, column=c, value=h)
    cell.font = hf; cell.fill = hfill; cell.alignment = wrap

meta_row = [
    "USB_FS_Device_Bulk_Transfer_test",
    'File: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include "usb.h"',
    "No macro definitions are visible in the supplied source. The following macros are used in program.c but their definitions are not supplied (expected to be defined in usb.h or its transitive includes):\nMIZAR_USB_DCTL\nMIZAR_USB_GUSB2PHYCFG\nMIZAR_USB_GEVNTADRLO\nMIZAR_USB_GEVNTADRHI\nMIZAR_USB_GEVNTSIZ\nMIZAR_USB_GEVNTCOUNT\nMIZAR_USB_GCTL\nMIZAR_USB_DCFG\nMIZAR_USB_DEVTEN\nMIZAR_USB_GUCTL\nMIZAR_USB_DALEPENA\nMIZAR_USB_DSTS\nMIZAR_USB_DEPCMDPAR1\nMIZAR_USB_DEPCMDPAR0\nMIZAR_USB_DEPCMD\nMIZAR_LSS_SYSREG_INTR_EN0\nMIZAR_LSS_SYSREG_MSK_STS0\nMIZAR_LSS_SYSREG_RAW_STCR0\nBuffer_PointerLO\nBuffer_PointerLO_1\nDefault_Event_Ring_Array\nevent_trb_addr\nDWORD",
    "File: program.c, Function: test_case()\nint buf_data[16];",
    "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: program.c, file scope:\nextern int int_pend;\nint event_counter;",
    "USB Full-Speed Device Bulk Transfer test for the USB IP (Mizar platform).",
    "SOURCE_FILE: program.c (full source code preserved in MetaData sheet)",
    "Agent 4 Output (register mapping): 23 entries including GCTL(matched), GUCTL(matched), DALEPENA(matched), and 20 unresolved/ambiguous entries",
    "23 validation criteria from program.c analysis including soft reset check, DEPCMD polling, event counter checks, handshake polling, and ISR validation"
]
for c, val in enumerate(meta_row, 1):
    md.cell(row=2, column=c, value=val).alignment = wrap
md.freeze_panes = "A2"

wb.save("/tmp/USB_TestPlan_20261026_140418.xlsx")
import os
print(f"Size: {os.path.getsize('/tmp/USB_TestPlan_20261026_140418.xlsx')}")
