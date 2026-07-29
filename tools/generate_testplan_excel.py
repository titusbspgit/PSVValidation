#!/usr/bin/env python3
import json
import os
from pathlib import Path
from datetime import datetime
try:
    from zoneinfo import ZoneInfo  # Python 3.9+
except Exception:
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# ---- Input bindings ----
IP_NAME = "USB"
OUTPUT_DIR = Path("Test_Output/USB/TestPlan/")
SOURCE_SUBDIR = "TestRepo/usb"

# JSON data embedded (preserve order)
json_text = r'''[
  {
    "Index": "1",
    "SS / Module": "USB",
    "Feature": "USB Register Read/Write Sanity",
    "Test Case Name": "basic_test",
    "Test Description": "Perform basic USB controller MMIO access and verify write-readback of a USB register (usb_reg); log read values from the memory map.",
    "Meta Test Description": "C program prints a banner, performs read_reg(0xA0000000) and logs the value, performs read_reg(0xA001706C) and logs the value, writes 0xDEADBEEF to 0xA0240000 using write_reg, reads back from 0xA0240000 via read_reg and logs the value, then calls finish(0). No conditional checks or explicit assertions present.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Requires read_reg, write_reg, and finish APIs to be available; execution environment must permit MMIO access to the USB controller space.",
    "Test Steps / Procedure": "1) Launch the test application to access the USB controller MMIO space. 2) Read a location in the USB memory map to confirm bus access. 3) Read another USB controller address to capture current state. 4) Write 0xDEADBEEF to the USB register (usb_reg) and read it back. 5) Review logs and complete the test.",
    "Meta Test Steps / Procedure": "1) printf(\"[C-Programme] Hello world\\n\"). 2) int rd_data = 0. 3) rd_data = read_reg(0xA0000000); printf(\"THE READ DATA is %X\\n\", rd_data). 4) rd_data = read_reg(0xA001706C); printf(\"THE READ DATA1 is %X\\n\", rd_data). 5) write_reg(0xA0240000, 0xDEADBEEF). 6) rd_data = read_reg(0xA0240000); printf(\"THE READ DATA2 is %X\\n\", rd_data). 7) finish(0).",
    "Impacted Registers": "DWC USB3 Memory Map; usb_reg",
    "Meta Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "The USB register (usb_reg) returns 0xDEADBEEF on readback after it is written with 0xDEADBEEF; all MMIO reads complete without access faults.",
    "Meta Validation / Acceptance Criteria": "No explicit assert/if conditions in code. Manual check required: verify the printed value from read_reg(0xA0240000) equals 0xDEADBEEF. Successful completion reaches finish(0).",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>; #include <stdlib.h>; #include <usb/usb_def.h>; #include <usb/usb_offsets.h>",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  },
  {
    "Index": "2",
    "SS / Module": "USB",
    "Feature": "USB Host Enumeration - Low Speed",
    "Test Case Name": "usb_host_enumeration_ls",
    "Test Description": "Configure the USB xHCI host controller, initialize event/command rings and device contexts, enable interrupts, reset and enable the host port, then enumerate a low-speed device via EP0 control transfers (Get Descriptor/Configuration and Set Configuration) and verify completion events before exiting.",
    "Meta Test Description": "The test initializes NIC and enables all IRQs, then programs xHCI global/operational registers: writes MIZAR_USB_GCTL, MIZAR_USB_GFLADJ, and MIZAR_USB_GUCTL using set_data. It configures PIPE and PHY control via MIZAR_USB_BASE+0xc2c0 and +0xc200. It reads MIZAR_USB_HCSPARAMS1, MIZAR_USB_SUPTPRT2_DW2, MIZAR_USB_SUPTPRT3_DW2, and MIZAR_USB_PORTSC_20, then enables wake bits (USB_PORTSC_20_WCE/WDE/WOE) and writes 0xe0002a0 to MIZAR_USB_PORTSC_20. It reads MIZAR_USB_DBOFF and sets up the Event Ring Segment Table and Default_Event_Ring_Array (size via ERSTSZ). It reads MIZAR_USB_HCSPARAMS2 and MIZAR_USB_PAGESIZE, sets up Scratchpad_Buffer_Array (SCRATCHPAD0/1), and loads Device_Context_Base_Address_Array with Device_Context_Array entries. It points MIZAR_USB_CRCR_LO/HI to Default_Command_Ring, sets MIZAR_USB_CONFIG to 0x10 then 0x110, programs MIZAR_USB_DCBAAP_LO/HI, MIZAR_USB_ERSTSZ, MIZAR_USB_ERDP_LO/HI, MIZAR_USB_ERSTBA_LO/HI, MIZAR_USB_IMOD=0x0, MIZAR_USB_IMAN=0x2, enables interrupts and run via MIZAR_USB_USBCMD (0x4 then 0x5), enables system interrupts via MIZAR_LSS_SYSREG_INTR_EN0=0x80000000, and waits while int_pend is set. On interrupt, it reads MIZAR_USB_USBSTS, clears with 0x8, re-enables MIZAR_USB_IMAN, and advances ERDP. It reads port status from MIZAR_USB_PORTSC_20, increments ERDP by 0x18, asserts port reset via PORTSC writes (0xe0006f1, then 0xe220200), and waits for another interrupt. It issues an Enable Slot command by writing TRBs at Default_Command_Ring+0x0..0xc, processes events (USBSTS/IMAN/ERDP/PORTSC updates), rings the doorbell (MIZAR_USB_DB), and waits. It builds Default_Input_Context (slot and EP0 context fields) including EP0_TR_Dequeue_Pointer, issues Address Device commands (writes at Default_Command_Ring+0x10/0x1c and +0x20/+0x2c), updates status/ERDP, waits, and reads completions from Default_Event_Ring_Array (+0x30, +0x40). The enumeration() routine programs EP0 TRBs for Get Device Descriptor (setup/data/status), Get Configuration (two sizes), Set Configuration (setup/status), another Get Configuration (larger size), then rings doorbell at MIZAR_USB_BASE+0x484 and polls Default_Event_Ring_Array+0x110 until non-zero. Default_IRQHandler clears system RAW status and acknowledges GIC, and writes MIZAR_USB_IMAN to re-arm. On successful flow, finish(0) is called.",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "System interrupt enable must be configured (INTR_EN0) and the xHCI controller must allow MMIO access. Ensure the event ring, command ring, and device context memory regions are accessible and properly aligned. A low-speed USB device should be connected to observe port connect and enumeration events.",
    "Test Steps / Procedure": "1) Initialize the platform and enable all interrupts. 2) Program USB global control and frame-length adjustment, and configure PIPE/PHY control. 3) Enable port wake on connect, disconnect, and over-current, then apply a port reset and set the link state using the host port status/control register. 4) Initialize xHCI host data structures: Event Ring Segment Table, Event Ring, Scratchpad buffers, Device Context Base Address Array, and the Default Command Ring. 5) Configure xHCI operational registers (CRCR, CONFIG, DCBAAP, ERSTSZ, ERDP, ERSTBA, IMOD, IMAN), enable interrupts, and start the controller (USBCMD); enable system-level interrupt routing. 6) Wait for and service the initial event, advance the dequeue pointer, and verify the port connect status is asserted. 7) Issue an Enable Slot command and process the corresponding completion event. 8) Build the Input Context for EP0 and issue Address Device commands; process completion events and advance the event ring. 9) Perform the enumeration sequence on EP0: Get Device Descriptor, Get Configuration Descriptor(s), and Set Configuration; ring the doorbell and wait for completion events. 10) Declare pass if expected events complete and the test exits successfully.",
    "Meta Test Steps / Procedure": "1) nic_programming(); GIC_EnableAllIRQ(). 2) write_reg(MIZAR_USB_GCTL, set_data(read_reg(MIZAR_USB_GCTL), 0xFFFFFFFF, 0x30c11234)); write_reg(MIZAR_USB_GFLADJ, set_data(read_reg(MIZAR_USB_GFLADJ), 0xFFFFFFFF, 0x0a87f000)); write_reg(MIZAR_USB_GUCTL, set_data(read_reg(MIZAR_USB_GUCTL), 0xFFFFFFFF, 0x02000010)). 3) rd_data = read_reg(MIZAR_USB_BASE+0xc2c0); write_reg(MIZAR_USB_BASE+0xc2c0, 0x010c0002); rd_data = read_reg(MIZAR_USB_BASE+0xc200); write_reg(MIZAR_USB_BASE+0xc200, 0x00102407). 4) port_count = read_reg(MIZAR_USB_HCSPARAMS1); rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2); rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2); rd_data = read_reg(MIZAR_USB_PORTSC_20); write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WCE, 1)); write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WDE, 1)); write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WOE, 1)); write_reg(MIZAR_USB_PORTSC_20, 0x0e0002a0). 5) db_offset = read_reg(MIZAR_USB_DBOFF); write_reg(Event_Ring_Segment_Table, Default_Event_Ring_Array); write_reg(Event_Ring_Segment_Table + DWORD, 0x0); write_reg(Event_Ring_Segment_Table + 2*DWORD, 0x30); rd_data = read_reg(MIZAR_USB_HCSPARAMS2); rd_data = read_reg(MIZAR_USB_PAGESIZE). 6) write_reg(Scratchpad_Buffer_Array, SCRATCHPAD0); write_reg(Scratchpad_Buffer_Array + DWORD, 0x0); write_reg(Scratchpad_Buffer_Array + 2*DWORD, SCRATCHPAD1); write_reg(Scratchpad_Buffer_Array + 3*DWORD, 0x0). 7) write_reg(Device_Context_Base_Address_Array, Scratchpad_Buffer_Array); write_reg(Device_Context_Base_Address_Array + DWORD, 0x0); write_reg(Device_Context_Base_Address_Array + 2*DWORD, Device_Context_Array + 0x100); write_reg(Device_Context_Base_Address_Array + 3*DWORD, 0x0); write_reg(Device_Context_Base_Address_Array + 4*DWORD, Device_Context_Array + 0x0d00); write_reg(Device_Context_Base_Address_Array + 5*DWORD, 0x0). 8) write_reg(MIZAR_USB_CRCR_LO, Default_Command_Ring + 0x1); write_reg(MIZAR_USB_CRCR_HI, 0x0); write_reg(MIZAR_USB_CONFIG, 0x10); rd_data = read_reg(MIZAR_USB_CONFIG); write_reg(MIZAR_USB_CONFIG, 0x110). 9) write_reg(MIZAR_USB_DCBAAP_LO, Device_Context_Base_Address_Array); write_reg(MIZAR_USB_DCBAAP_HI, 0x0); write_reg(MIZAR_USB_ERSTSZ, 0x1); write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array); write_reg(MIZAR_USB_ERDP_HI, 0x0); write_reg(MIZAR_USB_ERSTBA_LO, Event_Ring_Segment_Table); write_reg(MIZAR_USB_ERSTBA_HI, 0x0); write_reg(MIZAR_USB_IMOD, 0x0); write_reg(MIZAR_USB_IMAN, 0x2); write_reg(MIZAR_USB_USBCMD, 0x4); write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000); write_reg(MIZAR_USB_USBCMD, 0x5); int_pend=1; while(int_pend){ wait_on(100); }. 10) usb_status=read_reg(MIZAR_USB_USBSTS); write_reg(MIZAR_USB_USBSTS, 0x8); write_reg(MIZAR_USB_IMAN, 0x2); write_reg(MIZAR_USB_ERDP_HI, 0x0); port_status = read_reg(MIZAR_USB_PORTSC_20); write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x18); write_reg(MIZAR_USB_PORTSC_20, 0x0e0006f1); write_reg(MIZAR_USB_USBSTS, 0x8); write_reg(MIZAR_USB_IMAN, 0x2); write_reg(MIZAR_USB_PORTSC_20, 0x0e220200); port_status=read_reg(MIZAR_USB_PORTSC_20); int_pend=1; while(int_pend){ wait_on(100); }. 11) Slot Enable TRB: write_reg(Default_Command_Ring+0x0,0x0); +0x4=0x0; +0x8=0x0; +0xc=0x00002401; service USBSTS/IMAN; write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x28); write_reg(MIZAR_USB_USBSTS, 0x8); write_reg(MIZAR_USB_IMAN, 0x2); write_reg(MIZAR_USB_PORTSC_20, 0x0e200e01); write_reg(MIZAR_USB_DB, 0x0); int_pend=1; while(int_pend){ wait_on(100); }. 12) Program Default_Input_Context: at +DWORD=0x3; +0x40=0x08200000; +0x44=0x00010000; +0x80=0x00; +0x84=0x00080020; +0x88=(EP0_TR_Dequeue_Pointer|0x1); +0x90=0x08; input_context_address = Default_Input_Context; Address Device: write_reg(Default_Command_Ring+0x10, input_context_address); +0x1c=0x01002e01; service status/IMAN/ERDP_LO=Default_Event_Ring_Array+0x38; write_reg(MIZAR_USB_DB, 0x0); int_pend=1; while(int_pend){ wait_on(100); }; event_completion=read_reg(Default_Event_Ring_Array+0x30). 13) Re-write input context similarly; Address Device 2: write_reg(Default_Command_Ring+0x20, input_context_address); +0x2c=0x01002c01; service and wait; event_completion=read_reg(Default_Event_Ring_Array+0x40); call enumeration(). 14) enumeration(): Program EP0 TRBs for Get Device Descriptor (setup/data/status at EP0_TR_Dequeue_Pointer..+0x2c), Get Configuration (two sizes), Set Configuration (setup/status at +0x60..+0x7c), subsequent Get Configuration sequences at +0x80..+0xac and +0xb0..+0xdc; ring doorbell write_reg(MIZAR_USB_BASE+0x484,0x1); poll event_completion = read_reg(Default_Event_Ring_Array+0x110) until non-zero. 15) Default_IRQHandler(): read MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0; if (rd_data && 0x80000000) then write_reg(MIZAR_USB_IMAN,0x1) and write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,0x80000000); GIC_ClearIRQ(84). 16) On success path, finish(0).",
    "Impacted Registers": "usb_reg; fladj_30mhz_reg; MIZAR_USB_GUCTL; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; usb_host_inputs; MIZAR_USB_DBOFF; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_USB_USBSTS; MIZAR_USB_DB; INTR_EN0; MSK_STS0; RAW_STCR0",
    "Meta Impacted Registers": "MIZAR_USB_GCTL, MIZAR_USB_GFLADJ, MIZAR_USB_GUCTL, MIZAR_USB_BASE, MIZAR_USB_HCSPARAMS1, MIZAR_USB_SUPTPRT2_DW2, MIZAR_USB_SUPTPRT3_DW2, MIZAR_USB_PORTSC_20, USB_PORTSC_20_WCE, USB_PORTSC_20_WDE, USB_PORTSC_20_WOE, MIZAR_USB_DBOFF, Event_Ring_Segment_Table, Default_Event_Ring_Array, DWORD, MIZAR_USB_HCSPARAMS2, MIZAR_USB_PAGESIZE, Scratchpad_Buffer_Array, SCRATCHPAD0, SCRATCHPAD1, Device_Context_Base_Address_Array, Device_Context_Array, MIZAR_USB_CRCR_LO, MIZAR_USB_CRCR_HI, MIZAR_USB_CONFIG, MIZAR_USB_DCBAAP_LO, MIZAR_USB_DCBAAP_HI, MIZAR_USB_ERSTSZ, MIZAR_USB_ERDP_LO, MIZAR_USB_ERDP_HI, MIZAR_USB_ERSTBA_LO, MIZAR_USB_ERSTBA_HI, MIZAR_USB_IMOD, MIZAR_USB_IMAN, MIZAR_USB_USBCMD, MIZAR_USB_USBSTS, MIZAR_USB_DB, MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, Default_Command_Ring, Default_Input_Context, EP0_TR_Dequeue_Pointer, EP2_Out_TR_Dequeue_Pointer, EP2_In_TR_Dequeue_Pointer",
    "Validation / Acceptance Criteria": "Pass if: 1) The host port connect status is asserted in the port status/control register after reset, 2) Enable Slot and Address Device commands complete with corresponding events observed, 3) EP0 control transfers for Get Descriptor/Configuration and Set Configuration complete with events posted (non-zero event entries), and 4) The test terminates with a successful exit.",
    "Meta Validation / Acceptance Criteria": "- While(int_pend) loops exit only after Default_IRQHandler clears the interrupt condition (writes MIZAR_USB_IMAN and clears MIZAR_LSS_SYSREG_RAW_STCR0); usb_status is read and MIZAR_USB_USBSTS is cleared with 0x8. - A comment notes: \"port connect status should be high\" after reading MIZAR_USB_PORTSC_20. - ERDP is advanced (e.g., +0x18, +0x28, +0x38, +0x48) between events. - Doorbell writes (MIZAR_USB_DB and MIZAR_USB_BASE+0x484) trigger processing. - In enumeration(), event_completion = read_reg(Default_Event_Ring_Array+0x110) is polled until non-zero. - Successful flow reaches finish(0).",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>; #include <stdlib.h>; #include \"usb.h\"",
    "Meta Macros": "NA",
    "Meta Arrays": "int data_in[512]; int data_out[512];"
  }
]'''

# ---- Step 1: Validate JSON ----
try:
    data = json.loads(json_text)
except Exception as e:
    raise SystemExit(f"Invalid JSON input: {e}")

if not isinstance(data, list) or len(data) == 0:
    raise SystemExit("json_data must be a non-empty array of objects")

# ---- Compute IST timestamp ----
if ZoneInfo is not None:
    ist = ZoneInfo("Asia/Kolkata")
    now_ist = datetime.now(ist)
else:
    # Fallback: offset of +05:30 without TZ database
    from datetime import timezone, timedelta
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)

ts_str = now_ist.strftime("%Y%m%d_%H%M%S")

# ---- Step 2/3: Build workbook with formatting ----
wb = Workbook()

# Sheet 1: TestPlan
ws1 = wb.active
ws1.title = "TestPlan"

plan_headers = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation (Required / Not)",
]

# Styling
header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(fill_type="solid", start_color="4472C4", end_color="4472C4")
wrap = Alignment(wrap_text=True, vertical="top")

# Write headers
ws1.append(plan_headers)
for c in range(1, len(plan_headers) + 1):
    cell = ws1.cell(row=1, column=c)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap

# Write rows preserving order, mapping missing keys to ""
for row in data:
    values = [
        row.get("Index", ""),
        row.get("SS / Module", ""),
        row.get("Feature", ""),
        row.get("Test Case Name", ""),
        row.get("Test Description", ""),
        row.get("Speed", ""),
        row.get("Mode", ""),
        row.get("Memory Start Offset", ""),
        row.get("Memory End Offset", ""),
        row.get("Remarks", ""),
        row.get("Test Steps / Procedure", ""),
        row.get("Impacted Registers", ""),
        row.get("Validation / Acceptance Criteria", ""),
        row.get("Code Generation (Required / Not)", ""),
    ]
    ws1.append(values)

# Apply wrap alignment to all cells and auto width
for col_idx, col in enumerate(ws1.columns, start=1):
    max_len = 0
    col_letter = get_column_letter(col_idx)
    for cell in col:
        cell.alignment = wrap
        val = "" if cell.value is None else str(cell.value)
        ln = min(len(val), 120)
        if ln > max_len:
            max_len = ln
    ws1.column_dimensions[col_letter].width = min(max_len + 2, 80)

ws1.freeze_panes = "A2"

# Sheet 2: MetaData (Very Hidden)
ws2 = wb.create_sheet("MetaData")

# Top metadata block
ws2.append(["Key", "Value"])
ws2.append(["IP_NAME", IP_NAME])
ws2.append(["source_subdirectory", SOURCE_SUBDIR])
ws2.append(["generated_timezone", "IST (GMT+05:30)"])
ws2.append(["generation_timestamp_IST", ts_str])
ws2.append([""])

# Meta table headers
meta_headers = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]
ws2.append(meta_headers)

hdr_row = ws2.max_row
for c in range(1, len(meta_headers) + 1):
    cell = ws2.cell(row=hdr_row, column=c)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap

# Meta rows
for row in data:
    values = [
        row.get("Index", ""),
        row.get("Test Case Name", ""),
        row.get("Meta Test Description", ""),
        row.get("Meta Test Steps / Procedure", ""),
        row.get("Meta Impacted Registers", ""),
        row.get("Meta Validation / Acceptance Criteria", ""),
        row.get("Meta Headers", ""),
        row.get("Meta Macros", ""),
        row.get("Meta Arrays", ""),
    ]
    ws2.append(values)

# Wrap and width for MetaData
for col_idx, col in enumerate(ws2.columns, start=1):
    max_len = 0
    col_letter = get_column_letter(col_idx)
    for cell in col:
        cell.alignment = wrap
        val = "" if cell.value is None else str(cell.value)
        ln = min(len(val), 120)
        if ln > max_len:
            max_len = ln
    ws2.column_dimensions[col_letter].width = min(max_len + 2, 80)

ws2.freeze_panes = "A2"
ws2.sheet_state = 'veryHidden'

# ---- Step 4: Save file ----
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
filename = f"{IP_NAME}_TestPlan_{ts_str}.xlsx"
file_path = OUTPUT_DIR / filename
wb.save(file_path.as_posix())

print(f"Generated: {file_path}")
