#!/usr/bin/env python3
"""
Sandbox generator - writes xlsx to /tmp/USB_TestPlan.xlsx
Run: python3 sandbox_gen.py
Then the file at /tmp/USB_TestPlan.xlsx can be read and uploaded.
"""
import openpyxl, os
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation

TESTPLAN_DATA = {
    "Index": "1",
    "SS / Module": "USB",
    "Feature": "Full-Speed Device Bulk Transfer",
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Speed": "Full-Speed",
    "Mode": "Device",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": """This test validates USB Full-Speed Device mode Bulk Transfer functionality on the Mizar platform. The test initializes the USB controller by performing NIC programming and enabling GIC interrupts, clears buffer and event TRB memory regions, executes a USB controller soft reset and polls for its completion, configures the USB 2.0 PHY, sets up the event buffer (address low/high, size, and count), and programs global controller settings including port direction, device configuration, device event enable, and user control registers. The test then configures 8 device endpoints (START NEW CONFIGURATION followed by 8 endpoint configurations) and allocates TX resources for each endpoint. Physical endpoints 0 and 1 are enabled, the controller is placed into run state, and system-level interrupts are enabled. The test handles link state, connect, and reset events via interrupt-driven handshaking. USB enumeration is performed including SET ADDRESS, GET DESCRIPTOR (device descriptor and configuration descriptor), and USB SET CONFIGURATION. After enumeration, bulk data transfers are initiated on two bulk endpoints. An interrupt handler services USB events by reading the event count, acknowledging the event buffer, clearing system register interrupt status, and clearing the GIC IRQ. The test completes successfully upon finishing all bulk transfers.""",
    "Test Steps / Procedure": """1. Perform NIC programming and enable all GIC interrupts.
2. Clear buffer and event TRB memory regions by writing zeros in a loop (20 iterations).
3. Initiate USB controller soft reset by writing to the Device Control register (DCTL) and poll until the reset completes.
4. Configure the USB 2.0 PHY via the PHY Configuration register (GUSB2PHYCFG).
5. Set up the event buffer by programming the Event Address Low (GEVNTADRLO), Event Address High (GEVNTADRHI), Event Size (GEVNTSIZ), and Event Count (GEVNTCOUNT) registers.
6. Read the Global Control register (GCTL), then write the port direction configuration value.
7. Read and program the Device Configuration register (DCFG) for Full-Speed operation.
8. Enable device events (disconnect, reset, connection done, link state change, wakeup) via the Device Event Enable register (DEVTEN).
9. Read and program the User Control register (GUCTL).
10. Issue START NEW CONFIGURATION command via the endpoint command interface (DEPCMDPAR1, DEPCMDPAR0, DEPCMD).
11. Configure 8 endpoints by issuing endpoint configuration commands with appropriate parameters and polling for command completion.
12. Configure TX resources for all 8 endpoints in a loop, issuing resource allocation commands and polling for completion.
13. Enable physical endpoints 0 and 1 via the Active Endpoint Enable register (DALEPENA).
14. Set the controller to run state via the Device Control register (DCTL).
15. Enable system-level USB interrupt via the System Register Interrupt Enable register.
16. Wait for link state, connect, and reset events via interrupt-driven handshaking; conditionally wait for additional events based on event counter value.
17. Re-program the Device Configuration register (DCFG) after reset events. Read Device Status register (DSTS).
18. Perform USB enumeration Setup Stage.
19. Update PHY configuration (GUSB2PHYCFG) and Device Configuration (DCFG) for SET ADDRESS.
20. Prepare SET ADDRESS data TRB and issue Start Transfer command on the IN endpoint; poll for command completion.
21. Wait for handshake synchronization from the external agent.
22. Execute full enumeration sequence: GET DESCRIPTOR (device descriptor) with setup, data, and status stages.
23. Wait for second handshake synchronization.
24. Perform GET DESCRIPTOR (short configuration descriptor) with setup, data, and status stages.
25. Perform USB SET CONFIGURATION with setup and status data stages.
26. Execute second full enumeration sequence (device and configuration descriptors).
27. Perform GET DESCRIPTOR (full configuration descriptor with interface and endpoint descriptors).
28. Initiate Bulk OUT transfer on the first bulk endpoint.
29. Initiate Bulk OUT transfer on the second bulk endpoint.
30. Signal test completion and call finish with pass status.

Interrupt Handler Steps:
a. On interrupt, clear the interrupt pending flag.
b. Read the System Register Masked Status and Raw Status registers.
c. Read the Event Count register (GEVNTCOUNT) and store the event count.
d. Write back the event count to GEVNTCOUNT to acknowledge processed events.
e. If the raw status indicates a USB interrupt, clear the raw status bit.
f. Clear the GIC IRQ.""",
    "Impacted Registers": "GCTL, GUCTL, DALEPENA, DCFG, DCTL, DEVTEN, DSTS, GUSB2PHYCFG, GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, GEVNTCOUNT, DEPCMDPAR2, DEPCMDPAR1, DEPCMDPAR0, DEPCMD",
    "Validation / Acceptance Criteria": """1. USB controller soft reset must complete successfully.
2. All endpoint configuration commands must complete.
3. All endpoint TX resource allocation commands must complete.
4. All Start Transfer commands must complete.
5. Link state, connect, and reset events must be received via interrupts.
6. Event counter checks must pass.
7. Handshake synchronization must succeed.
8. All bulk transfer Start Transfer commands must complete.
9. The interrupt handler must correctly handle events.
10. The test must reach completion with finish(0).""",
    "Remarks": "The test includes a commented-out DEVICE QUALIFIER descriptor response section. Handshake synchronization with an external agent is performed via memory-mapped polling. The interrupt handler uses GIC IRQ 84. Three system-level registers (LSS_SYSREG_INTR_EN0, LSS_SYSREG_MSK_STS0, LSS_SYSREG_RAW_STCR0) are used for interrupt management but belong to a separate LSS SYSREG module."
}

META_DATA = {
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Meta Headers": 'File: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include "usb.h"',
    "Meta Macros": "Macros used but defined in usb.h: MIZAR_USB_DCTL, MIZAR_USB_GUSB2PHYCFG, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, MIZAR_USB_GEVNTSIZ, MIZAR_USB_GEVNTCOUNT, MIZAR_USB_GCTL, MIZAR_USB_DCFG, MIZAR_USB_DEVTEN, MIZAR_USB_GUCTL, MIZAR_USB_DALEPENA, MIZAR_USB_DSTS, MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, MIZAR_USB_DEPCMD, MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, Buffer_PointerLO, Buffer_PointerLO_1, Default_Event_Ring_Array, event_trb_addr, DWORD",
    "Meta Arrays": "File: program.c, Function: test_case()\nint buf_data[16];",
    "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: program.c, file scope:\nextern int int_pend;\nint event_counter;",
    "Meta Test Description": "USB Full-Speed Device Bulk Transfer test for the USB IP (Mizar platform).",
    "Meta Test Steps / Procedure": "SOURCE_FILE: program.c (full source code preserved)",
    "Meta Impacted Registers": "Register mapping from Agent 4 output (23 entries)",
    "Meta Validation / Acceptance Criteria": "23 validation criteria from program.c analysis"
}

wb = openpyxl.Workbook()
hf = Font(bold=True, color="FFFFFF")
hfill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

ws = wb.active
ws.title = "TestPlan"
tp_cols = ["Index","SS / Module","Feature","Test Case Name","Speed","Mode","Memory Start Offset","Memory End Offset","Test Description","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Remarks","Code Generation (Required / Not)"]
for c,h in enumerate(tp_cols,1):
    cell = ws.cell(row=1, column=c, value=h)
    cell.font = hf; cell.fill = hfill; cell.alignment = wrap
for c,col in enumerate(tp_cols[:13],1):
    ws.cell(row=2, column=c, value=TESTPLAN_DATA.get(col,"")).alignment = wrap
dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=False)
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
for c,col in enumerate(md_cols,1):
    md.cell(row=2, column=c, value=META_DATA.get(col,"")).alignment = wrap
md.freeze_panes = "A2"

outpath = "/tmp/USB_TestPlan_20261026_140418.xlsx"
wb.save(outpath)
sz = os.path.getsize(outpath)
print(f"Generated: {outpath} ({sz} bytes)")
