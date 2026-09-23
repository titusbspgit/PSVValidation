#!/usr/bin/env python3
"""USB TestPlan Excel Generator - Agent 7
Generates USB_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx with TestPlan and MetaData sheets.
Run: python3 generate_excel.py
"""
import datetime
import os
import sys

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

# IST timestamp
from datetime import timezone, timedelta
ist = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.datetime.now(ist)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"

# ---- DATA ----
row_data = {
    "Index": "1",
    "SS / Module": "USB",
    "Test Case Name": "USB_FS_Device_Isochronous_Transfer_test",
    "Feature": "FS Device Isochronous Transfer",
    "Speed": "Full-Speed",
    "Mode": "Device Mode",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
    "Meta Macros": "Buffer_PointerLO; event_trb_addr; DWORD; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DEPCMDPAR1; MIZAR_USB_DSTS; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; Default_Event_Ring_Array; Buffer_PointerLO_1",
    "Meta Arrays": "buf_data[16]",
    "Meta Test Description": "This testcase validates USB Full-Speed Device mode Isochronous Transfer functionality on the DWC USB3 controller. The test begins by calling nic_programming() and GIC_EnableAllIRQ() for system initialization. It clears 20 DWORDs at Buffer_PointerLO and event_trb_addr memory regions. A soft reset is performed by writing 0x40f00000 to MIZAR_USB_DCTL and polling until MIZAR_USB_DCTL reads 0xf00000. USB2 PHY is configured by writing 0x40002407 to MIZAR_USB_GUSB2PHYCFG. The event ring is set up by writing Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO, 0x0 to MIZAR_USB_GEVNTADRHI, 0x30 to MIZAR_USB_GEVNTSIZ, and 0x0 to MIZAR_USB_GEVNTCOUNT. MIZAR_USB_GCTL is read-modify-written with 0x30c12214 to set port direction to device mode. MIZAR_USB_DCFG is read then written with 0x480801. MIZAR_USB_DEVTEN is written with 0x1f to enable device events. MIZAR_USB_GUCTL is read-modify-written with 0xa400010. Endpoint configuration is performed via set_configuration() calls for 9 endpoints (offsets 0x00 through 0x70) using MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, and MIZAR_USB_DEPCMD with various parameters and commands (0x409 for start new config, 0x401 for set endpoint config). TX resource allocation is done in a loop for 8 endpoints by writing 0x1 to MIZAR_USB_DEPCMDPAR0+(i*0x10) and 0x402 to MIZAR_USB_DEPCMD+(i*0x10), polling MIZAR_USB_DEPCMD until command completes. MIZAR_USB_DALEPENA is written with 0x3 then later 0xff to enable physical endpoints. MIZAR_USB_DCTL is written with 0x80f00000 to run the controller. MIZAR_LSS_SYSREG_INTR_EN0 is written with 0x80000000 to enable system-level interrupts. The test waits for link state connect/reset events via interrupt pending flags. Enumeration is performed including GET_DESCRIPTOR (device descriptor with 0x12 bytes, configuration descriptor with 0x9 and 0x3c bytes), SET_ADDRESS (MIZAR_USB_DCFG written with 0x480809), and SET_CONFIGURATION stages. Each stage uses TRB (Transfer Request Block) setup by writing buffer pointer, size, and control fields to event_trb_addr, then issuing Start Transfer commands (0x506) via DEPCMD and polling for completion. Handshake polling is done at 0xa0243ff4 and 0xa0243ff8. After enumeration, MIZAR_USB_DSTS is polled for frame number (mask 0x00000FF8) to be non-zero. Isochronous OUT transfer is initiated by setting up TRBs with Buffer_PointerLO_1, size 0x3ff, control 0x869 (isochronous type), and issuing Update Transfer commands (0x20506 and 0x40506) on endpoints at offsets 0x60 and 0x70. MIZAR_USB_DSTS is polled for specific frame numbers (frame number bits [5:3] checked for values 0x2, 0x3, 0x4). Progress markers are written to 0xA0243ffc (0xdeadbee0 through 0xdeadbee9). The Default_IRQHandler reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, reads and acknowledges MIZAR_USB_GEVNTCOUNT, clears the raw status by writing 0x80000000 to MIZAR_LSS_SYSREG_RAW_STCR0, and clears GIC IRQ 84.",
    "Test Description": "This test validates USB Full-Speed Device mode Isochronous Transfer functionality. The test initializes the USB controller by performing a soft reset via the device control register and polling for reset completion. It configures the USB2 PHY, sets up the event ring buffer address and size registers, configures the global control register for device mode port direction, sets device configuration and device event enable registers, and configures the USB controller timing register. Multiple endpoints are configured using endpoint command parameter and command registers with Start New Configuration and Set Endpoint Configuration commands. TX resources are allocated for 8 endpoints by issuing Transfer Resource Allocation commands through the endpoint command registers and polling for completion. Physical endpoints are enabled via the active endpoint enable register. System-level interrupts are enabled. The test then performs USB enumeration including device descriptor request, configuration descriptor request, SET_ADDRESS, and SET_CONFIGURATION control transfers. Each control transfer involves setup, data, and status stages using Transfer Request Blocks and Start Transfer commands issued through endpoint command registers with polling for command completion. After enumeration, the device status register is polled for valid frame numbers. Two Isochronous OUT transfers are initiated on separate endpoints using Update Transfer commands with isochronous TRB descriptors. The device status register is polled to synchronize transfers to specific frame numbers (frames 2, 3, and 4). An interrupt handler reads system interrupt status, acknowledges USB events via the event count register, clears raw interrupt status, and clears the GIC interrupt.",
    "Meta Test Steps / Procedure": "1. Call nic_programming() and GIC_EnableAllIRQ() for system and interrupt initialization. 2. Clear 20 DWORDs at Buffer_PointerLO and event_trb_addr using write_reg in a loop (j=0 to 19, offset j*DWORD). 3. Perform soft reset: write 0x40f00000 to MIZAR_USB_DCTL, then poll read_reg(MIZAR_USB_DCTL) with wait_on(100) until value equals 0xf00000. 4. Configure USB2 PHY: write 0x40002407 to MIZAR_USB_GUSB2PHYCFG. 5. Set up event ring: write Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO, 0x0 to MIZAR_USB_GEVNTADRHI, 0x30 to MIZAR_USB_GEVNTSIZ, 0x0 to MIZAR_USB_GEVNTCOUNT. 6. Read MIZAR_USB_GCTL, then write 0x30c12214 to MIZAR_USB_GCTL (port direction = device). 7. Read MIZAR_USB_DCFG, then write 0x480801 to MIZAR_USB_DCFG. 8. Write 0x1f to MIZAR_USB_DEVTEN to enable device events (disconnect, USB reset, connection done, link state change, wakeup). 9. Read MIZAR_USB_GUCTL, then write 0xa400010 to MIZAR_USB_GUCTL. 10. Call set_configuration() 9 times with varying offsets (0x00-0x70) and parameters for Start New Configuration (cmd=0x409) and Set Endpoint Configuration (cmd=0x401) commands via MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, MIZAR_USB_DEPCMD. Each call polls MIZAR_USB_DEPCMD+offset until command completes. 11. Loop i=0 to 7: write 0x1 to MIZAR_USB_DEPCMDPAR0+(i*0x10), write 0x402 to MIZAR_USB_DEPCMD+(i*0x10), poll MIZAR_USB_DEPCMD+(i*0x10) with wait_on(10) until != 0x402 (TX resource allocation). 12. Write 0x3 to MIZAR_USB_DALEPENA (enable physical endpoints 0,1). 13. Write 0x80f00000 to MIZAR_USB_DCTL (run controller). 14. Write 0x80000000 to MIZAR_LSS_SYSREG_INTR_EN0 (enable sysreg interrupt). 15. Wait for link state connect/reset events: set int_pend=1, poll while(int_pend) with wait_on(100). Check event_counter <= 0x4 for additional interrupt waits. 16. Write 0x480801 to MIZAR_USB_DCFG. 17. Write 0xdeadbee0 to 0xA0243ffc (enumeration progress marker). 18. Read MIZAR_USB_DCFG and MIZAR_USB_DSTS. Write 0x480801 to MIZAR_USB_DCFG, 0x80f00a00 to MIZAR_USB_DCTL, 0xff to MIZAR_USB_DALEPENA. 19. Call setup_stage(): write TRB (Buffer_PointerLO, size=0x8, ctrl=0x823) to event_trb_addr, issue Start Transfer (0x506) via MIZAR_USB_DEPCMD, poll for completion, wait for interrupt. 20. Write 0x40002547 to MIZAR_USB_GUSB2PHYCFG. Wait for interrupts based on event_counter. 21. SET_ADDRESS: write 0x480809 to MIZAR_USB_DCFG. Set up TRB (Buffer_PointerLO, size=0x0, ctrl=0x853) at event_trb_addr. Issue Start Transfer (0x506) via MIZAR_USB_DEPCMD+0x10, poll for completion. Wait for interrupts. 22. Poll read_reg(0xa0243ff4) until non-zero (handshake). 23. Call enumeration(): performs GET_DESCRIPTOR device (0x12 bytes), GET_DESCRIPTOR configuration (0x9 bytes then 0x3c bytes), SET_CONFIGURATION via setup_stage/data/status_stage sequences. Each uses TRBs with ctrl=0x853 for data, 0x823 for setup, 0x843 for status. Descriptor data written to Buffer_PointerLO. Commands issued via MIZAR_USB_DEPCMD+0x10 with 0x506, polled for completion. Progress markers 0xdeadbee1-0xdeadbee5 written to 0xA0243ffc. 24. Poll read_reg(0xa0243ff8) until non-zero (handshake). 25. Read MIZAR_USB_DSTS, poll while (rd_data & 0x00000FF8) == 0x0 with wait_on(100) (wait for frame number 1). 26. Write 0xdeadbee6 to 0xA0243ffc. 27. ISOCHRONOUS OUT: write TRB (Buffer_PointerLO_1, size=0x3ff, ctrl=0x869) to event_trb_addr. Issue Update Transfer (0x20506) via MIZAR_USB_DEPCMD+0x60, poll for completion. Wait for interrupt. 28. Poll MIZAR_USB_DSTS: while ((rd_data & 0x00000038) >> 3) != 0x2 with wait_on(100) (wait for frame number 2). 29. Write 0xdeadbee7 to 0xA0243ffc. Wait for interrupt. 30. Poll MIZAR_USB_DSTS: while ((rd_data & 0x00000038) >> 3) != 0x3 with wait_on(100) (wait for frame number 3). 31. Write 0xdeadbee8 to 0xA0243ffc. Wait for interrupt. 32. Second ISOCHRONOUS OUT: write TRB (Buffer_PointerLO_1, size=0x3ff, ctrl=0x869) to event_trb_addr. Issue Update Transfer (0x40506) via MIZAR_USB_DEPCMD+0x70, poll for completion. Wait for interrupt. 33. Poll MIZAR_USB_DSTS: while ((rd_data & 0x00000038) >> 3) != 0x4 with wait_on(100) (wait for frame number 4). 34. Write 0xdeadbee9 to 0xA0243ffc. Wait for interrupt. 35. Call finish(0). 36. Default_IRQHandler: clear int_pend=0, read MIZAR_LSS_SYSREG_MSK_STS0, read MIZAR_LSS_SYSREG_RAW_STCR0, read MIZAR_USB_GEVNTCOUNT into event_count/event_counter, write event_count back to MIZAR_USB_GEVNTCOUNT to acknowledge, if rd_data && 0x80000000 write 0x80000000 to MIZAR_LSS_SYSREG_RAW_STCR0, call GIC_ClearIRQ(84).",
    "Test Steps / Procedure": "1. Initialize the system by calling NIC programming and enabling all GIC interrupts. 2. Clear the data buffer and event TRB memory regions by writing zeros to 20 DWORD-aligned locations. 3. Perform a soft reset of the USB controller by writing to the device control register and polling until the reset completes. 4. Configure the USB2 PHY settings register for Full-Speed operation. 5. Set up the event ring by writing the event buffer base address to the event address low and high registers, configuring the event size register, and clearing the event count register. 6. Read-modify-write the global control register to set port direction to device mode. 7. Read-modify-write the device configuration register with the desired configuration value. 8. Enable device events (disconnect, USB reset, connection done, link state change, wakeup) by writing to the device event enable register. 9. Read-modify-write the USB controller timing register. 10. Configure 9 endpoints using the endpoint command parameter and command registers with Start New Configuration and Set Endpoint Configuration commands, polling each command for completion. 11. Allocate TX resources for 8 endpoints by issuing Transfer Resource Allocation commands through the endpoint command registers and polling for completion. 12. Enable physical endpoints 0 and 1 via the active endpoint enable register. 13. Start the USB controller by writing to the device control register. 14. Enable system-level USB interrupts via the system register interrupt enable register. 15. Wait for link state connect and reset events through interrupt-driven handshaking. 16. Begin USB enumeration by writing progress markers and reading device configuration and status registers. 17. Enable all physical endpoints via the active endpoint enable register. 18. Execute the setup stage of control transfers by preparing TRBs and issuing Start Transfer commands, polling for completion. 19. Perform SET_ADDRESS by updating the device configuration register and issuing a data stage transfer on endpoint 1. 20. Poll the handshake register until the host-side acknowledges readiness. 21. Execute full enumeration sequence including GET_DESCRIPTOR (device and configuration descriptors) and SET_CONFIGURATION control transfers with setup, data, and status stages. 22. Poll the second handshake register until the host-side acknowledges enumeration completion. 23. Poll the device status register until a valid frame number is detected. 24. Initiate the first Isochronous OUT transfer by preparing an isochronous TRB and issuing an Update Transfer command on endpoint 6, then polling for command completion. 25. Poll the device status register to synchronize to frame number 2, then frame number 3. 26. Initiate the second Isochronous OUT transfer by preparing an isochronous TRB and issuing an Update Transfer command on endpoint 7, then polling for command completion. 27. Poll the device status register to synchronize to frame number 4. 28. Write final progress marker and wait for the last interrupt before completing the test.",
    "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; MIZAR_USB_DSTS; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
    "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DEPCMDPAR1; DSTS",
    "Meta Validation / Acceptance Criteria": "1. Soft reset validation: poll read_reg(MIZAR_USB_DCTL) until value equals 0xf00000, confirming reset completion. 2. Endpoint command completion: for each MIZAR_USB_DEPCMD+(offset) write, poll until the register value no longer equals the written command value (e.g., 0x402, 0x506, 0x20506, 0x40506), indicating the controller has processed the command. 3. Interrupt-driven event handling: int_pend flag is set to 1 before waiting and cleared to 0 inside Default_IRQHandler, confirming interrupt was received and serviced. event_counter is checked against 0x4 to determine if additional interrupt waits are needed. 4. Handshake polling: read_reg(0xa0243ff4) polled until non-zero, confirming host-side readiness. read_reg(0xa0243ff8) polled until non-zero, confirming enumeration completion acknowledgment. 5. Frame number validation: MIZAR_USB_DSTS polled with mask 0x00000FF8 until non-zero for frame number 1. MIZAR_USB_DSTS polled with mask 0x00000038 shifted right by 3, checked for values 0x2 (frame 2), 0x3 (frame 3), 0x4 (frame 4) to synchronize isochronous transfers. 6. IRQ handler validation: MIZAR_LSS_SYSREG_MSK_STS0 read to check masked interrupt status. MIZAR_LSS_SYSREG_RAW_STCR0 read and written with 0x80000000 to clear raw status if bit 31 is set. MIZAR_USB_GEVNTCOUNT read and written back to acknowledge events. GIC_ClearIRQ(84) called to clear the interrupt. 7. Progress markers: values 0xdeadbee0 through 0xdeadbee9 written to 0xA0243ffc at key stages to track test progress. 8. Test completes by calling finish(0) indicating successful completion.",
    "Validation / Acceptance Criteria": "1. The soft reset completes successfully as confirmed by polling the device control register until it returns the expected post-reset value. 2. All endpoint configuration commands (Start New Configuration, Set Endpoint Configuration, Transfer Resource Allocation, Start Transfer, Update Transfer) complete successfully as confirmed by polling the endpoint command register until the command active bit clears. 3. USB device events (link state changes, connect, reset) are received and processed correctly through the interrupt handler, which reads and acknowledges the event count register and clears system interrupt status. 4. Handshake synchronization with the host side succeeds for both pre-enumeration and post-enumeration phases. 5. USB enumeration completes successfully including device descriptor, configuration descriptor, SET_ADDRESS, and SET_CONFIGURATION control transfers. 6. The device status register reports valid frame numbers after enumeration, confirming the USB link is active. 7. Two Isochronous OUT transfers are initiated successfully on separate endpoints using Update Transfer commands, with frame-level synchronization confirmed by polling the device status register for specific frame numbers (2, 3, and 4). 8. The test completes by calling finish with a success code of 0.",
    "Remarks": "The test operates in USB Full-Speed Device mode with isochronous transfer capability. Interrupt-driven event handling is used with GIC IRQ 84 for USB events. The test uses polling loops with wait_on() delays for command completion and frame synchronization. Handshake registers at external addresses are used for host-device synchronization during enumeration. Progress marker values are written to an external register to track test execution stages. Three LSS system register macros (MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0) and three direct hex addresses (used for handshake and progress markers) could not be mapped to named registers in the specification. Buffer_PointerLO and event_trb_addr are RAM-based memory regions used for TRB and data buffer storage, not memory-mapped controller registers. The set_configuration and enumeration helper functions internally use the same endpoint command registers with varying offsets for different endpoints."
}

# ---- TESTPLAN SHEET COLUMNS ----
testplan_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# ---- METADATA SHEET COLUMNS ----
metadata_columns = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# ---- CREATE WORKBOOK ----
wb = Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = "TestPlan"

# MetaData sheet
ws_md = wb.create_sheet(title="MetaData")

# ---- FORMATTING ----
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# ---- POPULATE TESTPLAN SHEET ----
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for col_idx, col_name in enumerate(testplan_columns, 1):
    value = row_data.get(col_name, "")
    cell = ws_tp.cell(row=2, column=col_idx, value=value)
    cell.alignment = wrap_alignment

# ---- POPULATE METADATA SHEET ----
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for col_idx, col_name in enumerate(metadata_columns, 1):
    value = row_data.get(col_name, "")
    cell = ws_md.cell(row=2, column=col_idx, value=value)
    cell.alignment = wrap_alignment

# ---- AUTO-SIZE COLUMNS ----
def auto_size(ws, max_width=60):
    for col in ws.columns:
        max_len = 0
        col_letter = get_column_letter(col[0].column)
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
        adjusted = min(max_len + 2, max_width)
        ws.column_dimensions[col_letter].width = max(adjusted, 12)

auto_size(ws_tp)
auto_size(ws_md)

# ---- FREEZE FIRST ROW ----
ws_tp.freeze_panes = "A2"
ws_md.freeze_panes = "A2"

# ---- SET METADATA SHEET TO VERY HIDDEN ----
ws_md.sheet_state = "veryHidden"

# ---- SAVE ----
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
wb.save(output_path)
print(f"Workbook saved: {output_path}")
print(f"Filename: {filename}")

# ---- VALIDATE ----
assert os.path.exists(output_path), "File does not exist!"
assert os.path.getsize(output_path) > 0, "File is empty!"
wb2 = load_workbook(output_path)
assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing!"
assert "MetaData" in wb2.sheetnames, "MetaData sheet missing!"
print("Validation: PASSED")
print(f"TestPlan rows: {ws_tp.max_row - 1}")
print(f"MetaData rows: {ws_md.max_row - 1}")
