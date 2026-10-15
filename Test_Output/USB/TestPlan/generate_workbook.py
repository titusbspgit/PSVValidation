#!/usr/bin/env python3
"""
Generate USB TestPlan Excel Workbook.
Run: python3 generate_workbook.py
Output: USB_TestPlan_<timestamp>.xlsx in same directory
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import sys, os

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"
script_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(script_dir, filename)

META_JSON = [{"Test Case Name":"USB_FS_Device_Bulk_Transfer_test","IP Name":"NA","Feature Information":"NA","Meta Headers":"#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"","Meta Macros":"Buffer_PointerLO\nDWORD\nevent_trb_addr\nMIZAR_USB_DCFG\nMIZAR_USB_DSTS\nMIZAR_USB_DCTL\nMIZAR_USB_DALEPENA\nMIZAR_USB_DEPCMDPAR1\nMIZAR_USB_DEPCMDPAR0\nMIZAR_USB_DEPCMD","Meta Arrays":"int buf_data[16];","Meta Types":"int rd_data;\nint wr_data;\nint port_count;\nint db_offset;\nint buf_data[16];\nint i;\nint bulk;\nint intr;\nint event_comletion;\nint j;\nint hand_shake;\nextern int int_pend;\nint event_counter;","Meta Test Description":"int test_case() declared with return type int. File includes stdio.h, stdlib.h, usb.h. Declares extern int int_pend. Declares int event_counter. Declares void Default_IRQHandler(). Declares void setup_stage(). Declares int test_case().","Meta Test Steps / Procedure":"1. nic_programming();\n2. GIC_EnableAllIRQ();\n3. for(j=0; j<20; j++) { write_reg(Buffer_PointerLO + j*DWORD, 0x0); write_reg(event_trb_addr + j*DWORD, 0x0); }\n4. write_reg(0xA0243ffc, 0xdeadbee0);\n5. rd_data = read_reg(MIZAR_USB_DCFG);\n6. rd_data = read_reg(MIZAR_USB_DSTS);\n7. write_reg(MIZAR_USB_DCFG, 0x480801);\n8. write_reg(MIZAR_USB_DCTL, 0x80f00a00);\n9. write_reg(MIZAR_USB_DALEPENA, 0xff);\n10. write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);\n11. write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);\n12. write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);\n13. rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); while(rd_data == 0x506) { wait_on(10); rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); }\n14. int_pend = 1; while(int_pend) { wait_on(5); }\n15. status_stage();\n16. setup_stage();\n17. write_reg(0xA0243ffc, 0xdeadbee3);\n18. write_reg(event_trb_addr, Buffer_PointerLO);\n19. write_reg(event_trb_addr + 0x8, 0x9);\n20. write_reg(event_trb_addr + 0xc, 0x853);\n21. write_reg(Buffer_PointerLO, 0x003c0209);\n22. write_reg(Buffer_PointerLO + 0x4, 0xe0000101);\n23. write_reg(Buffer_PointerLO + 0x8, 0x00000032);\n24. write_reg(Buffer_PointerLO + 0xc, 0x00000000);\n25. int_pend = 1; while(int_pend) { wait_on(5); }\n26. write_reg(0xA0243ffc, 0xdeadbee4);\n27. setup_stage();\n28. write_reg(0xA0243ffc, 0xdeadbee5);","Meta Impacted Registers":"HCSPARAMS1\nGCTL\nGUCTL\nDALEPENA","Meta Validation / Acceptance Criteria":"1. rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); while(rd_data == 0x506) { wait_on(10); rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); } \u2014 polling loop validates DEPCMD+0x10 value changes from 0x506 to confirm endpoint command completion.\n2. int_pend = 1; while(int_pend) { wait_on(5); } \u2014 polling loop validates int_pend is cleared by Default_IRQHandler to confirm interrupt event received. This pattern is repeated multiple times in the test.\n3. write_reg(0xA0243ffc, 0xdeadbee0) \u2014 debug marker written to validate test progress at enumeration start.\n4. write_reg(0xA0243ffc, 0xdeadbee3) \u2014 debug marker written to validate test progress at GET DESCRIPTOR USB CONFIGURATION.\n5. write_reg(0xA0243ffc, 0xdeadbee4) \u2014 debug marker written to validate test progress at SET CONFIGURATION or RESET TT.\n6. write_reg(0xA0243ffc, 0xdeadbee5) \u2014 debug marker written to validate test progress at subsequent GET DESCRIPTOR USB CONFIGURATION."}]

TESTPLAN_JSON = [{"Index":"1","SS / Module":"NA","Test Case Name":"USB_FS_Device_Bulk_Transfer_test","Feature":"NA","Test Description":"This testcase verifies USB Full-Speed device-mode bulk transfer functionality. The test configures the USB device controller, performs device enumeration including setup and status stages, configures endpoint parameters, initiates bulk data transfers using Transfer Request Blocks (TRBs), and validates successful completion through interrupt-driven event handling. The expected behavior is that the device controller completes endpoint commands, processes interrupt events correctly, and successfully handles USB descriptor requests and configuration stages during enumeration and bulk transfer operations.","Test Steps / Procedure":"1. Perform NIC programming initialization.\n2. Enable all IRQs via the GIC.\n3. Clear the data buffer and event TRB memory regions by writing zeros to all entries.\n4. Write a debug marker to indicate enumeration start.\n5. Read the Device Configuration register to capture initial state.\n6. Read the Device Status register to capture current device status.\n7. Configure the Device Configuration register for Full-Speed operation.\n8. Configure the Device Control register to initiate device operation.\n9. Enable all relevant endpoints via the Device Active Endpoint Enable register.\n10. Configure endpoint command parameters for event TRB handling.\n11. Issue an endpoint command and poll for command completion.\n12. Wait for an interrupt event to confirm the endpoint command was processed.\n13. Execute the status stage of the control transfer.\n14. Execute the setup stage for the next control transfer.\n15. Write a debug marker to indicate GET DESCRIPTOR USB CONFIGURATION phase.\n16. Prepare the event TRB with buffer pointer and transfer parameters.\n17. Populate the data buffer with USB configuration descriptor data.\n18. Wait for an interrupt event to confirm descriptor transfer completion.\n19. Write a debug marker to indicate SET CONFIGURATION or RESET TT phase.\n20. Execute the setup stage for the subsequent control transfer.\n21. Write a debug marker to indicate completion of the subsequent GET DESCRIPTOR USB CONFIGURATION phase.","Impacted Registers":"HCSPARAMS1, GCTL, GUCTL, DALEPENA","Validation / Acceptance Criteria":"1. Endpoint command completion is confirmed by polling the endpoint command register until the command active bit clears, indicating the controller has successfully processed the command.\n2. Interrupt events are correctly received and handled by the IRQ handler, confirmed by the interrupt pending flag being cleared after each expected event.\n3. Debug markers are written at key test milestones (enumeration start, GET DESCRIPTOR USB CONFIGURATION, SET CONFIGURATION or RESET TT, and subsequent GET DESCRIPTOR USB CONFIGURATION) to validate sequential test progress.\n4. The USB device controller successfully transitions through Full-Speed enumeration stages including setup, status, and data phases.\n5. Bulk transfer TRBs are correctly configured and processed, with successful completion indicated by interrupt event generation.","Remarks":"NA"}]

# Validation
tp_names = {tc["Test Case Name"] for tc in TESTPLAN_JSON}
md_names = {m["Test Case Name"] for m in META_JSON}
assert tp_names == md_names, f"Mismatch: {tp_names ^ md_names}"

wb = openpyxl.Workbook()
header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

# TestPlan
ws = wb.active
ws.title = "TestPlan"
tp_cols = ["Index","SS / Module","Feature","Test Case Name","Speed","Mode","Memory Start Offset","Memory End Offset","Test Description","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Remarks"]
for ci, h in enumerate(tp_cols, 1):
    c = ws.cell(row=1, column=ci, value=h); c.font=header_font; c.fill=header_fill; c.alignment=wrap
for ri, tc in enumerate(TESTPLAN_JSON, 2):
    for ci, h in enumerate(tp_cols, 1):
        c = ws.cell(row=ri, column=ci, value=tc.get(h, "")); c.alignment=wrap
ws.freeze_panes = "A2"

# MetaData
md = wb.create_sheet("MetaData")
md.sheet_state = "veryHidden"
md_cols = ["Test Case Name","Meta Headers","Meta Macros","Meta Arrays","Meta Types","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria"]
for ci, h in enumerate(md_cols, 1):
    c = md.cell(row=1, column=ci, value=h); c.font=header_font; c.fill=header_fill; c.alignment=wrap
for ri, m in enumerate(META_JSON, 2):
    for ci, h in enumerate(md_cols, 1):
        c = md.cell(row=ri, column=ci, value=m.get(h, "")); c.alignment=wrap
md.freeze_panes = "A2"

for s in [ws, md]:
    for col in s.columns:
        mx = 0; cl = col[0].column_letter
        for cell in col:
            if cell.value:
                mx = max(mx, max(len(l) for l in str(cell.value).split('\n')))
        s.column_dimensions[cl].width = min(mx + 2, 60)

wb.save(output_path)
print(f"Generated: {output_path}")
print(f"TestPlan rows: {len(TESTPLAN_JSON)}")
print(f"MetaData rows: {len(META_JSON)}")
