#!/usr/bin/env python3
"""
USB TestPlan Excel Workbook Generator
=====================================
Generates USB_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx with:
- TestPlan sheet (visible) with 14 columns
- MetaData sheet (veryHidden) with 9 columns

Usage:
  pip install openpyxl
  python gen_xlsx.py

Output will be saved in the same directory as this script.
The generated .xlsx file should be committed to:
  Test_Output/USB/TestCode/USB_TestPlan_<timestamp>.xlsx
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os, sys

IST = timezone(timedelta(hours=5, minutes=30))
now = datetime.now(IST)
timestamp = now.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"

json_data = [
    {
        "Index": "1",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
        "Feature": "USB Full-Speed Device Bulk Transfer",
        "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "usb.h"',
        "Meta Macros": "MIZAR_USB_DCFG, MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, MIZAR_USB_DEPCMD, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, MIZAR_USB_GEVNTSIZ, MIZAR_USB_GEVNTCOUNT, MIZAR_USB_GCTL, Buffer_PointerLO, Default_Event_Ring_Array, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, DWORD, event_trb_addr, Buffer_PointerLO_1",
        "Meta Arrays": "int buf_data[16];",
        "Speed": "Full Speed",
        "Mode": "Device",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": """USB Full-Speed Device Bulk Transfer test. The testcase initializes the USB device controller via nic_programming() (which configures global registers such as MIZAR_USB_GCTL with 0x30c11234 via set_data), enables all IRQs via GIC_EnableAllIRQ(), clears 20 entries of Buffer_PointerLO and event_trb_addr memory regions in a loop. The test proceeds through USB device enumeration including setup_stage(), status_stage(), and enumeration() functions which configure event TRBs and data buffers for device descriptor (0x02000012, length 0x12) and configuration descriptor (0x003c0209, length 0x3c) responses. The set_configuration() function is used to issue endpoint commands by writing MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, and MIZAR_USB_DEPCMD registers with polling for command completion. Bulk data transfer is performed using TRBs configured at Buffer_PointerLO with control field 0x853 and DEPCMD+0x10 with command 0x506, followed by interrupt-driven synchronization via int_pend polling with wait_on(5). The Default_IRQHandler reads MIZAR_USB_GEVNTCOUNT, stores it in event_counter, writes it back to acknowledge events, reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, and calls GIC_ClearIRQ(84).""",
        "Test Description": "USB Full-Speed Device Bulk Transfer test. Initializes the USB device controller global registers, enables interrupts, clears buffer and event TRB memory. Performs USB device enumeration including device descriptor and configuration descriptor responses. Configures bulk endpoints using set_configuration with DEPCMD parameter registers. Executes bulk data transfers using TRBs with interrupt-driven synchronization. The interrupt handler reads GEVNTCOUNT for event acknowledgment and clears system-level interrupt status.",
        "Meta Test Steps / Procedure": """extern int int_pend;
int event_counter;

int test_case() {
 int rd_data,wr_data,port_count,db_offset;
 int buf_data[16],i,bulk,intr,event_comletion,j;
 int hand_shake;

 nic_programming();
 GIC_EnableAllIRQ();
 for(j=0;j< 20;j++){
 write_reg(Buffer_PointerLO + jDWORD, 0x0);
 write_reg(event_trb_addr + jDWORD, 0x0);
 }

 // ... controller initialization via nic_programming() which includes:
 // write_reg(MIZAR_USB_GCTL,set_data(read_reg(MIZAR_USB_GCTL),0xFFFFFFFF,0x30c11234));

 // DEPCMD+0x10 Start Transfer command
 write_reg(MIZAR_USB_DEPCMD+0x10,0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
 while(rd_data == 0x506){
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
 }

 int_pend = 1;
 while(int_pend) { wait_on(5); }

 wait_on(5);

 int_pend = 1;
 while(int_pend) { wait_on(5); }

 // ADDED to check the xfernotready event
 int_pend = 1;
 while(int_pend) { wait_on(5); }

 status_stage();

 // USB_SET_CONFIGURATION_OR_RESET_TT
 setup_stage();

 write_reg(0xA0243ffc,0xdeadbee4);

 // GET DESCRIPTOR USB CONFIGURATION
 int_pend = 1;
 while(int_pend) { wait_on(5); }

 setup_stage();
 write_reg(0xA0243ffc,0xdeadbee5);

 int_pend = 1;
 while(int_pend) { wait_on(5); }

 // data stage - configuration descriptor
 //09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301

 // enumeration();

 // Bulk data transfer TRB setup
 write_reg(event_trb_addr,Buffer_PointerLO);
 write_reg(event_trb_addr+0x8,0x0);
 write_reg(event_trb_addr+0xc,0x853);
 write_reg(Buffer_PointerLO,0x00);
 write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);
 write_reg(MIZAR_USB_DEPCMD+0x10,0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
 while(rd_data == 0x506){
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);
 }

 // Bulk endpoint TRB with Buffer_PointerLO_1
 // write_reg(event_trb_addr,Buffer_PointerLO_1);
 // write_reg(event_trb_addr+0x8,0x40);
 // write_reg(event_trb_addr+0xc,0x813);
 // write_reg(MIZAR_USB_DEPCMDPAR1+0x40,event_trb_addr);
 // write_reg(MIZAR_USB_DEPCMDPAR0+0x40,0x0);
 // write_reg(MIZAR_USB_DEPCMD+0x40,0x506);

 int_pend = 1;
 while(int_pend) { wait_on(5); }
}

void Default_IRQHandler() {
 int rd_data,sysreg_rd_data,event_count;
 int_pend = 0;
 rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
 event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
 event_counter = event_count;
 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
 if( rd_data && 0x80000000){
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,0x80000000);
 }
 GIC_ClearIRQ(84);
}

void setup_stage() {
 int rd_data;
 write_reg(event_trb_addr,Buffer_PointerLO);
 write_reg(event_trb_addr+0x8,0x8);
 write_reg(event_trb_addr+0xc,0x823);
 write_reg(MIZAR_USB_DEPCMDPAR1,event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0,0x0);
 write_reg(MIZAR_USB_DEPCMD,0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 while(rd_data == 0x506){
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 }
 int_pend = 1;
 while(int_pend) {
 wait_on(5);
 }
}

void status_stage() {
 int rd_data;
 write_reg(event_trb_addr,Buffer_PointerLO);
 write_reg(event_trb_addr+0x8,0x0);
 write_reg(event_trb_addr+0xc,0x843);
 write_reg(MIZAR_USB_DEPCMDPAR1,event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0,0x0);
 write_reg(MIZAR_USB_DEPCMD,0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 while(rd_data == 0x506){
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 }
 int_pend = 1;
 while(int_pend) {
 wait_on(5);
 }
 wait_on(5);
}

void enumeration() {
 // Device descriptor response
 write_reg(event_trb_addr,Buffer_PointerLO);
 write_reg(event_trb_addr+0x8,0x12);
 write_reg(event_trb_addr+0xc,0x853);
 //data
 write_reg(Buffer_PointerLO,0x02000012);
 write_reg(Buffer_PointerLO+0x4,0x40000000);
 write_reg(Buffer_PointerLO+0x8,0x00000000);
 write_reg(Buffer_PointerLO+0xc,0x00000000);
 write_reg(Buffer_PointerLO+0x10,0x00000100);
 write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);

 // Configuration descriptor response
 write_reg(event_trb_addr,Buffer_PointerLO);
 write_reg(event_trb_addr+0x8,0x3c);
 write_reg(event_trb_addr+0xc,0x853);
 write_reg(Buffer_PointerLO,0x003c0209);
 write_reg(Buffer_PointerLO+0x4,0xe0000101);
 write_reg(Buffer_PointerLO+0x8,0x00000032);
 write_reg(Buffer_PointerLO+0xc,0x00000000);
 write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);
}

void set_configuration(int trb_address,int parameter0,int parameter1,int cmd) {
 int read_data;
 write_reg(MIZAR_USB_DEPCMDPAR1+trb_address,parameter1);
 write_reg(MIZAR_USB_DEPCMDPAR0+trb_address,parameter0);
 write_reg(MIZAR_USB_DEPCMD+trb_address,cmd);
 read_data = read_reg(MIZAR_USB_DEPCMD+trb_address);
 while(read_data == cmd){
 wait_on(30);
 read_data = read_reg(MIZAR_USB_DEPCMD+trb_address);
 }
}""",
        "Test Steps / Procedure": """1. Call nic_programming() for USB device controller initialization (configures GCTL with 0x30c11234 and other global registers).
2. Enable all IRQs via GIC_EnableAllIRQ().
3. Clear 20 entries of Buffer_PointerLO and event_trb_addr memory regions by writing 0x0 in a loop.
4. Issue Start Transfer command by writing 0x506 to DEPCMD+0x10, poll until command completes.
5. Set int_pend = 1, poll with wait_on(5) until interrupt clears int_pend (repeated three times for xfernotready event handling).
6. Call status_stage() to complete status phase of control transfer.
7. Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT.
8. Write debug marker 0xdeadbee4 to address 0xA0243ffc.
9. Wait for interrupt, then call setup_stage() for GET DESCRIPTOR USB CONFIGURATION.
10. Write debug marker 0xdeadbee5 to address 0xA0243ffc.
11. Wait for interrupt, proceed with configuration descriptor data stage.
12. Configure event TRB with Buffer_PointerLO, transfer length, and control field 0x853.
13. Write device descriptor data (0x02000012, 0x40000000, etc.) to Buffer_PointerLO.
14. Write configuration descriptor data (0x003c0209, 0xe0000101, 0x00000032) to Buffer_PointerLO.
15. Set DEPCMDPAR1+0x10 to event_trb_addr, DEPCMDPAR0+0x10 to 0x0.
16. Issue DEPCMD+0x10 with 0x506 for bulk endpoint Start Transfer, poll until complete.
17. Wait for interrupt completion via int_pend polling with wait_on(5).
18. Default_IRQHandler: clears int_pend, reads GEVNTCOUNT, stores in event_counter, writes back to acknowledge, reads system register status, clears raw interrupt status with 0x80000000, calls GIC_ClearIRQ(84).""",
        "Meta Impacted Registers": "GCTL",
        "Impacted Registers": "GCTL",
        "Meta Validation / Acceptance Criteria": """1. After writing 0x506 to MIZAR_USB_DEPCMD+0x10, poll read_reg(MIZAR_USB_DEPCMD+0x10) until value changes from 0x506, indicating Start Transfer command completion.
2. In setup_stage(): after writing 0x506 to MIZAR_USB_DEPCMD, poll read_reg(MIZAR_USB_DEPCMD) until value changes from 0x506, indicating endpoint command completion.
3. In status_stage(): after writing 0x506 to MIZAR_USB_DEPCMD, poll read_reg(MIZAR_USB_DEPCMD) until value changes from 0x506, indicating endpoint command completion.
4. In set_configuration(): after writing cmd to MIZAR_USB_DEPCMD+trb_address, poll read_reg(MIZAR_USB_DEPCMD+trb_address) until value changes from cmd, indicating command completion.
5. In Default_IRQHandler(): read MIZAR_USB_GEVNTCOUNT to obtain event_count, store in event_counter global, write event_count back to MIZAR_USB_GEVNTCOUNT to acknowledge events; read MIZAR_LSS_SYSREG_RAW_STCR0 and check condition (rd_data && 0x80000000) to verify interrupt source; write MIZAR_LSS_SYSREG_RAW_STCR0 with 0x80000000 to clear raw interrupt status; call GIC_ClearIRQ(84).
6. Interrupt-driven synchronization: int_pend set to 1 before each wait loop, cleared to 0 by Default_IRQHandler, polled via wait_on(5) in while loop.""",
        "Validation / Acceptance Criteria": """1. DEPCMD Start Transfer command (0x506) completes successfully - verified by polling DEPCMD register until value changes.
2. Endpoint commands in setup_stage and status_stage complete successfully via DEPCMD polling.
3. set_configuration endpoint commands complete successfully via DEPCMD+trb_address polling.
4. Interrupt handler correctly reads GEVNTCOUNT, acknowledges events by writing count back, and clears system-level interrupt status.
5. All interrupt-driven synchronization points complete (int_pend cleared by IRQ handler).
6. Device descriptor response (18 bytes) and configuration descriptor response (60 bytes) are correctly populated in Buffer_PointerLO memory.""",
        "Remarks": "The testcase uses interrupt-driven polling with wait_on(5) which is shorter than other USB transfer tests (Isochronous/Interrupt use wait_on(100)), indicating tighter timing for bulk transfers. The Default_IRQHandler uses logical AND (&&) instead of bitwise AND (&) for checking bit 31 of rd_data, which may be a potential bug. The nic_programming() function body is not available in the retrieved source but is known to configure MIZAR_USB_GCTL with 0x30c11234 via set_data(). Some portions of the test_case() function body (particularly the complete bulk data transfer phase and set_configuration call parameters) are not fully available in the retrieved RAG source context. The MIZAR_USB_DCFG, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, and MIZAR_USB_GEVNTSIZ registers are referenced in Agent 4 mappings but their initialization is likely within nic_programming() which is not visible in the retrieved source. Only GCTL has a matched register_name from Agent 4; all other macros are unresolved."
    }
]

wb = openpyxl.Workbook()
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_headers = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation"]
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

for ci, h in enumerate(tp_headers, 1):
    c = ws_tp.cell(row=1, column=ci, value=h)
    c.font = header_font; c.fill = header_fill; c.alignment = wrap_alignment

tp_map = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria"]
for ri, rd in enumerate(json_data, 2):
    for ci, key in enumerate(tp_map, 1):
        ws_tp.cell(row=ri, column=ci, value=rd.get(key, "")).alignment = wrap_alignment
    ws_tp.cell(row=ri, column=14, value="").alignment = wrap_alignment

ws_tp.freeze_panes = "A2"
for col in ws_tp.columns:
    ml = 0; cl = col[0].column_letter
    for cell in col:
        if cell.value: ml = max(ml, min(len(str(cell.value)), 80))
    ws_tp.column_dimensions[cl].width = min(max(ml + 2, 12), 80)

ws_md = wb.create_sheet("MetaData")
md_headers = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]
for ci, h in enumerate(md_headers, 1):
    c = ws_md.cell(row=1, column=ci, value=h)
    c.font = header_font; c.fill = header_fill; c.alignment = wrap_alignment

md_map = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]
for ri, rd in enumerate(json_data, 2):
    for ci, key in enumerate(md_map, 1):
        ws_md.cell(row=ri, column=ci, value=rd.get(key, "")).alignment = wrap_alignment

ws_md.freeze_panes = "A2"
for col in ws_md.columns:
    ml = 0; cl = col[0].column_letter
    for cell in col:
        if cell.value: ml = max(ml, min(len(str(cell.value)), 80))
    ws_md.column_dimensions[cl].width = min(max(ml + 2, 12), 80)

ws_md.sheet_state = "veryHidden"

output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
wb.save(output_path)

# Validation
assert os.path.exists(output_path), "File does not exist"
assert os.path.getsize(output_path) > 0, "File size is 0"
wb2 = openpyxl.load_workbook(output_path)
assert "TestPlan" in wb2.sheetnames
assert "MetaData" in wb2.sheetnames
ws2 = wb2["MetaData"]
for ci, key in enumerate(md_map, 1):
    assert ws2.cell(row=2, column=ci).value == json_data[0].get(key, ""), f"Mismatch: {key}"
wb2.close()

print(f"SUCCESS|{filename}|{os.path.getsize(output_path)}|{len(json_data)}|{output_path}")
