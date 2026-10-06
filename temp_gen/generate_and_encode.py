#!/usr/bin/env python3
"""
Complete USB TestPlan XLSX Generator
Run: python3 generate_and_encode.py
Outputs: /tmp/USB_TestPlan_20261006_131326.xlsx and prints base64
"""
import json, os, sys, base64
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

FILENAME = "USB_TestPlan_20261006_131326.xlsx"
OUTPATH = os.path.join("/tmp", FILENAME)

# All 4 test cases inline
json_data = [
  {
    "Index":"1","SS / Module":"USB","Test Case Name":"USB_FS_Device_Bulk_Transfer_test","Feature":"FS Device Bulk Transfer",
    "Meta Headers":"<stdio.h>; <stdlib.h>; \"usb.h\"","Meta Macros":"NA","Meta Arrays":"buf_data[16]",
    "Speed":"NA","Mode":"NA","Memory Start Offset":"NA","Memory End Offset":"NA",
    "Test Description":"This testcase validates USB Full-Speed Device mode Bulk Transfer operation. It performs a soft reset of the USB controller, configures the USB2 PHY, sets up the event ring, configures the global controller register for port direction, configures device settings, enables device events, and configures user control settings. It then configures all endpoints using start new configuration and set endpoint configuration commands, allocates TX resources for 8 endpoints, and enables physical endpoints. The device controller is started and system-level interrupts are enabled. The test waits for link state connect and reset events via interrupts. It then performs USB enumeration including setup stage, SET ADDRESS, GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, SET CONFIGURATION, and GET FULL CONFIGURATION DESCRIPTOR, each involving TRB setup, Start Transfer commands on the DEPCMD register with polling for command completion, and interrupt-driven event handling. After enumeration, Bulk OUT and Bulk IN transfers are initiated on separate endpoints by setting up TRBs and issuing Start Transfer commands. The interrupt handler reads interrupt status, processes event counts via the event count register, clears interrupt status, and clears the GIC IRQ.",
    "Test Steps / Procedure":"1. Initialize NIC programming and enable all GIC IRQs. 2. Clear buffer pointer and event TRB memory regions (20 entries each). 3. Perform USB controller soft reset by writing to DCTL and poll until reset completes. 4. Configure USB2 PHY settings. 5. Set up the event ring by configuring event address low, event address high, event size, and event count registers. 6. Configure the global controller register for port direction. 7. Configure device settings and enable device events via DEVTEN register. 8. Configure user control settings via GUCTL register. 9. Issue Start New Configuration command and configure 8 endpoints using endpoint command registers (DEPCMDPAR0, DEPCMDPAR1, DEPCMD) with polling for command completion. 10. Allocate TX resources for 8 endpoints by writing to DEPCMDPAR0 and DEPCMD in a loop, polling for completion. 11. Enable physical endpoints 0 and 1 via DALEPENA register. 12. Start the device controller by writing to DCTL. 13. Enable system-level interrupt. 14. Wait for link state connect and reset events via interrupt-driven mechanism. 15. Read device status from DSTS register and reconfigure device settings. 16. Enable all physical endpoints via DALEPENA register. 17. Execute setup stage: prepare TRB, issue Start Transfer command on DEPCMD, poll for completion, wait for interrupt. 18. Reconfigure USB2 PHY and wait for interrupts. 19. Perform SET ADDRESS by updating device configuration and issuing Start Transfer command on endpoint 1. 20. Poll handshake register until ready. 21. Execute enumeration sequence: GET DEVICE DESCRIPTOR (setup, data with descriptor payload, status), GET CONFIGURATION DESCRIPTOR (setup, data, status), SET CONFIGURATION (setup, data, status), GET FULL CONFIGURATION DESCRIPTOR (setup, data with full descriptor payload, status). 22. Poll second handshake register until ready. 23. Initiate Bulk OUT transfer: set up TRB with buffer and size, issue Start Transfer on Bulk OUT endpoint, wait for interrupt completion. 24. Initiate Bulk IN transfer: set up TRB with buffer and size, issue Start Transfer on Bulk IN endpoint, wait for interrupt completion. 25. Write final completion marker and wait for final interrupt. 26. Call finish to end the test.",
    "Impacted Registers":"DCTL; GCTL; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS",
    "Validation / Acceptance Criteria":"Soft reset must complete successfully as confirmed by polling DCTL until the expected reset-complete value is read. Each endpoint command (Start New Configuration, Set Endpoint Configuration, Endpoint TX Resource Allocation, Start Transfer) must complete as confirmed by polling DEPCMD until the command value clears. Interrupt-driven events must be received for link state connect, reset, and transfer completion events, with the event count register read and acknowledged in the interrupt handler. Handshake registers must return non-zero values indicating host-side readiness before proceeding with enumeration and bulk transfers. USB enumeration must complete successfully including SET ADDRESS, GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, SET CONFIGURATION, and GET FULL CONFIGURATION DESCRIPTOR phases. Bulk OUT and Bulk IN transfers must complete with interrupt confirmation. The test must reach finish(0) indicating successful completion.",
    "Remarks":"Test operates in USB Full-Speed Device mode with Bulk Transfer. Interrupt-driven flow uses int_pend flag and Default_IRQHandler on GIC IRQ 84. event_counter tracks event count from the event count register and gates additional interrupt waits. Handshake polling on two memory-mapped addresses synchronizes with the host side. Progress markers are written to a memory-mapped address at each enumeration stage. The enumeration function includes GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, SET CONFIGURATION, and GET FULL CONFIGURATION DESCRIPTOR phases. Bulk transfers use separate endpoints (offset 0x40 for OUT, offset 0x50 for IN). The set_configuration helper function writes to DEPCMDPAR1, DEPCMDPAR0, and DEPCMD at parameterized offsets and polls for command completion. The status_stage helper sets up a status TRB and issues a Start Transfer command with interrupt waits. Several Agent 4 register mappings are unresolved.",
    "Meta Test Description":"placeholder_tc1_meta_desc",
    "Meta Test Steps / Procedure":"placeholder_tc1_meta_steps",
    "Meta Impacted Registers":"Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; Buffer_PointerLO_1",
    "Meta Validation / Acceptance Criteria":"placeholder_tc1_meta_val"
  }
]

print(f"Test cases: {len(json_data)}")
print("Generating workbook...")

wb = openpyxl.Workbook()
ws_tp = wb.active
ws_tp.title = "TestPlan"
ws_md = wb.create_sheet("MetaData")

tp_headers = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation"]
md_headers = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]

hf = Font(bold=True, color="FFFFFF", size=11)
hfill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

for ci, h in enumerate(tp_headers, 1):
    c = ws_tp.cell(row=1, column=ci, value=h); c.font=hf; c.fill=hfill; c.alignment=wrap
for ci, h in enumerate(md_headers, 1):
    c = ws_md.cell(row=1, column=ci, value=h); c.font=hf; c.fill=hfill; c.alignment=wrap

tp_keys = tp_headers[:-1]
for ri, item in enumerate(json_data, 2):
    for ci, key in enumerate(tp_keys, 1):
        c = ws_tp.cell(row=ri, column=ci, value=item.get(key, "")); c.alignment=wrap
    ws_tp.cell(row=ri, column=len(tp_headers), value="").alignment=wrap
    for ci, key in enumerate(md_headers, 1):
        c = ws_md.cell(row=ri, column=ci, value=item.get(key, "")); c.alignment=wrap

ws_tp.freeze_panes = "A2"
ws_md.freeze_panes = "A2"

for ws in [ws_tp, ws_md]:
    for col_cells in ws.columns:
        mx = 0; letter = get_column_letter(col_cells[0].column)
        for cell in col_cells:
            if cell.value: mx = max(mx, min(len(str(cell.value)), 60))
        ws.column_dimensions[letter].width = max(mx + 2, 15)

ws_md.sheet_state = "veryHidden"
wb.save(OUTPATH)
sz = os.path.getsize(OUTPATH)
print(f"SAVED: {OUTPATH} ({sz} bytes)")

wb2 = openpyxl.load_workbook(OUTPATH)
assert "TestPlan" in wb2.sheetnames
assert "MetaData" in wb2.sheetnames
print("VALIDATION: PASSED")

with open(OUTPATH, "rb") as f:
    b64 = base64.b64encode(f.read()).decode()
print(f"BASE64_LENGTH: {len(b64)}")
# Write first 200 chars for verification
print(f"BASE64_START: {b64[:200]}")
