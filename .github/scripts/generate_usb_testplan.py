#!/usr/bin/env python3
import os
import json
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")

IP_NAME = "USB"
filename = f"{IP_NAME}_TestCode_{timestamp}.xlsx"
output_dir = f"Test_Output/USB/TestCode/{IP_NAME}_TestCode_{timestamp}"
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Isochronous_Transfer_test",
        "Feature": "Full-Speed Device Isochronous Transfer",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
        "Meta Macros": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; Buffer_PointerLO_1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; DWORD; Default_Event_Ring_Array",
        "Meta Arrays": "buf_data[16]",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates USB Full-Speed Device mode Isochronous Transfer on the DWC USB3 controller. The test begins by calling nic_programming() and GIC_EnableAllIRQ() to initialize the platform and enable global interrupts. It clears 20 DWORDs at Buffer_PointerLO and event_trb_addr memory regions. A soft reset is performed by writing 0x40f00000 to MIZAR_USB_DCTL and polling until the register reads 0xf00000. The USB2 PHY is configured via MIZAR_USB_GUSB2PHYCFG with value 0x40002407. The event buffer is set up by writing Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO, 0x0 to MIZAR_USB_GEVNTADRHI, 0x30 to MIZAR_USB_GEVNTSIZ, and 0x0 to MIZAR_USB_GEVNTCOUNT. MIZAR_USB_GCTL is read-modify-written with 0x30c12214 to set port direction to device mode. MIZAR_USB_DCFG is written with 0x480801 for device configuration. MIZAR_USB_DEVTEN is written with 0x1f to enable device events (disconnect, USB reset, connect done, link state change, wakeup). MIZAR_USB_GUCTL is read-modify-written with 0xa400010. Endpoint configuration is performed via set_configuration() for 9 endpoints (START_NEW_CONFIGURATION command 0x409 plus 8 physical endpoints with command 0x401), writing MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, and MIZAR_USB_DEPCMD with appropriate parameters and polling MIZAR_USB_DEPCMD until the command completes. Endpoint TX resources are configured in a loop for 8 endpoints by writing 0x1 to MIZAR_USB_DEPCMDPAR0+(i*0x10) and 0x402 to MIZAR_USB_DEPCMD+(i*0x10), polling until completion. MIZAR_USB_DALEPENA is written with 0x3 to enable EP0 IN/OUT, then later 0xff to enable all 8 endpoints. MIZAR_USB_DCTL is written with 0x80f00000 to start the run/stop. MIZAR_LSS_SYSREG_INTR_EN0 is written with 0x80000000 to enable the system register interrupt. The test waits for link state connect/reset events via int_pend polling loops. USB enumeration is performed through setup_stage(), data stage, and status_stage() sequences for GET_DEVICE_DESCRIPTOR (descriptor 0x02000012), GET_CONFIGURATION_DESCRIPTOR (descriptor 0x003c0209), SET_CONFIGURATION, and GET_FULL_CONFIGURATION_DESCRIPTOR (60-byte descriptor written to Buffer_PointerLO offsets 0x00-0x38), each using TRB writes to event_trb_addr and Buffer_PointerLO with DEPCMD start transfer commands (0x506). Handshake polling is done on addresses 0xa0243ff4 and 0xa0243ff8 waiting for non-zero values. SET_ADDRESS is performed by writing MIZAR_USB_DCFG with 0x480809. MIZAR_USB_DSTS is polled for frame number (bits[11:3]) to become non-zero. Isochronous OUT transfer 1 is initiated by writing TRB at event_trb_addr with Buffer_PointerLO_1 as data buffer, size 0x3ff, control 0x869 (IOC, ISP, Isochronous), and issuing DEPCMD start transfer (0x20506) on endpoint offset 0x60. MIZAR_USB_DSTS is polled for specific frame numbers (0x2, 0x3) in bits[5:3]. A second isochronous transfer is issued on endpoint offset 0x70 with DEPCMD value 0x40506, and MIZAR_USB_DSTS is polled for frame number 0x4 in bits[5:3]. Progress markers are written to 0xA0243ffc (0xdeadbee0 through 0xdeadbee9). The Default_IRQHandler reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, reads MIZAR_USB_GEVNTCOUNT into event_count, writes back the event count to acknowledge, clears the sysreg interrupt by writing 0x80000000 to MIZAR_LSS_SYSREG_RAW_STCR0 if bit 31 is set (note: source uses && logical AND instead of & bitwise AND), and clears GIC IRQ 84. The test ends with finish(0).",
        "Test Description": "This test validates USB Full-Speed Device mode Isochronous Transfer functionality on the DWC USB3 controller. The test initializes the USB controller by performing a soft reset via DCTL, configuring the USB2 PHY via GUSB2PHYCFG, and setting up the event buffer using GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, and GEVNTCOUNT. The global controller is configured in device mode via GCTL, and device configuration is set through DCFG. Device events are enabled via DEVTEN, and the user control register GUCTL is configured. Multiple physical endpoints are configured using DEPCMDPAR1, DEPCMDPAR0, and DEPCMD with start new configuration and set endpoint configuration commands. TX resources are allocated for 8 endpoints. Active endpoints are enabled via DALEPENA. The controller run/stop is initiated via DCTL, and system-level interrupts are enabled. The test performs USB enumeration including device descriptor, configuration descriptor, SET_ADDRESS, and SET_CONFIGURATION control transfers through setup, data, and status stages. Handshake synchronization is performed by polling external memory-mapped locations. After enumeration, DSTS is polled for valid frame numbers. Two isochronous OUT transfers are initiated on separate endpoints with specific frame number targeting, validating isochronous scheduling across multiple microframes. The interrupt handler services USB events by reading the event count from GEVNTCOUNT, acknowledging events, clearing system register interrupts, and clearing the GIC interrupt. The test completes successfully after all isochronous transfers and frame synchronization are verified.",
        "Meta Test Steps / Procedure": "1. Call nic_programming() for platform initialization. 2. Call GIC_EnableAllIRQ() to enable all IRQ interrupts. 3. Clear 20 DWORDs at Buffer_PointerLO and event_trb_addr memory regions using a loop (j=0 to 19), writing 0x0 to each DWORD offset. 4. Perform soft reset: write 0x40f00000 to MIZAR_USB_DCTL. 5. Poll MIZAR_USB_DCTL with wait_on(100) until read value equals 0xf00000 (soft reset complete). 6. Write 0x40002407 to MIZAR_USB_GUSB2PHYCFG for USB2 PHY configuration. 7. Write Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO. 8. Write 0x0 to MIZAR_USB_GEVNTADRHI. 9. Write 0x30 to MIZAR_USB_GEVNTSIZ (event buffer size 48 bytes). 10. Write 0x0 to MIZAR_USB_GEVNTCOUNT. 11. Read MIZAR_USB_GCTL, then write 0x30c12214 (PRTCAPDIR=device, SCALEDOWN, other config). 12. Read MIZAR_USB_DCFG, then write 0x480801 to MIZAR_USB_DCFG. 13. Write 0x1f to MIZAR_USB_DEVTEN (enable disconnect, USB reset, connect done, link state change, wakeup events). 14. Read MIZAR_USB_GUCTL, then write 0xa400010. 15. Call set_configuration(0,0,0,0x409) for START_NEW_CONFIGURATION command: writes MIZAR_USB_DEPCMDPAR1+0, MIZAR_USB_DEPCMDPAR0+0, MIZAR_USB_DEPCMD+0 with 0x409, polls DEPCMD until complete. 16. Call set_configuration() 8 times with varying parameters for physical endpoint configuration (offsets 0x00-0x70, various parameter0/parameter1 values, command 0x401). Each call writes MIZAR_USB_DEPCMDPAR1+offset, MIZAR_USB_DEPCMDPAR0+offset, MIZAR_USB_DEPCMD+offset and polls DEPCMD until command completes. 17. Loop i=0 to 7: write 0x1 to MIZAR_USB_DEPCMDPAR0+(i*0x10), write 0x402 to MIZAR_USB_DEPCMD+(i*0x10), poll MIZAR_USB_DEPCMD+(i*0x10) until != 0x402 (TX resource allocation). 18. Write 0x3 to MIZAR_USB_DALEPENA (enable EP0 IN/OUT). 19. Write 0x80f00000 to MIZAR_USB_DCTL (run/stop with keep connect). 20. Write 0x80000000 to MIZAR_LSS_SYSREG_INTR_EN0 (enable sysreg interrupt). 21. Set int_pend=1, wait for link state connect/reset events via int_pend polling loops with wait_on(100). 22. If event_counter <= 0x4, set int_pend=1 and wait again. 23. Write 0x480801 to MIZAR_USB_DCFG. 24. If event_counter <= 0x4, set int_pend=1 and wait again. 25. Write 0xdeadbee0 to 0xA0243ffc (enumeration progress marker). 26. Read MIZAR_USB_DCFG, read MIZAR_USB_DSTS. 27. Write 0x480801 to MIZAR_USB_DCFG, write 0x80f00a00 to MIZAR_USB_DCTL. 28. Write 0xff to MIZAR_USB_DALEPENA (enable all 8 endpoints). 29. Wait 5000 cycles via wait_on(5000). 30. Call setup_stage(): write TRB (event_trb_addr with Buffer_PointerLO, size 0x8, control 0x823), write MIZAR_USB_DEPCMDPAR1/PAR0, issue DEPCMD 0x506, poll until complete, wait for interrupt. 31. Write 0x40002547 to MIZAR_USB_GUSB2PHYCFG (update PHY config). 32. Wait for events via int_pend loops if event_counter <= 0x4. 33. SET_ADDRESS: write 0x480809 to MIZAR_USB_DCFG. 34. Write TRB at event_trb_addr with Buffer_PointerLO, size 0x0, control 0x853. Write DEPCMDPAR1+0x10, DEPCMDPAR0+0x10, issue DEPCMD+0x10 with 0x506, poll until complete. 35. Wait for events, then poll 0xa0243ff4 until non-zero (first handshake). 36. Call enumeration(): performs GET_DEVICE_DESCRIPTOR (setup_stage, data stage with descriptor 0x02000012 written to Buffer_PointerLO, DEPCMD 0x506 on EP1 IN, status_stage), GET_CONFIGURATION_DESCRIPTOR (setup_stage, data stage with 0x003c0209 config descriptor, DEPCMD 0x506, status_stage), SET_CONFIGURATION (setup_stage, data stage with 0x00, DEPCMD 0x506, status_stage), GET_FULL_CONFIGURATION_DESCRIPTOR (setup_stage, data stage with full 60-byte config descriptor written to Buffer_PointerLO offsets 0x00-0x38, DEPCMD 0x506, status_stage). Progress markers 0xdeadbee1 through 0xdeadbee5 written to 0xA0243ffc. 37. Poll 0xa0243ff8 until non-zero (second handshake). 38. Read MIZAR_USB_DSTS, poll until bits[11:3] (frame number) != 0. 39. Write 0xdeadbee6 to 0xA0243ffc. 40. ISOCHRONOUS OUT transfer 1: write TRB at event_trb_addr with Buffer_PointerLO_1, size 0x3ff, control 0x869 (IOC|ISP|Isochronous). Write DEPCMDPAR1+0x60, DEPCMDPAR0+0x60, issue DEPCMD+0x60 with 0x20506 (start transfer with frame number). Poll DEPCMD+0x60 until != 0x20506. Wait for interrupt. 41. Poll MIZAR_USB_DSTS until bits[5:3] == 0x2 (frame number 2). 42. Write 0xdeadbee7 to 0xA0243ffc. Wait for interrupt. 43. Poll MIZAR_USB_DSTS until bits[5:3] == 0x3 (frame number 3). 44. Write 0xdeadbee8 to 0xA0243ffc. Wait for interrupt. 45. ISOCHRONOUS OUT transfer 2: write TRB at event_trb_addr with Buffer_PointerLO_1, size 0x3ff, control 0x869. Write DEPCMDPAR1+0x70, DEPCMDPAR0+0x70, issue DEPCMD+0x70 with 0x40506. Poll DEPCMD+0x70 until != 0x40506. Wait for interrupt. 46. Poll MIZAR_USB_DSTS until bits[5:3] == 0x4 (frame number 4). 47. Write 0xdeadbee9 to 0xA0243ffc. Wait for interrupt. 48. Call finish(0). 49. Default_IRQHandler: clear int_pend=0, read MIZAR_LSS_SYSREG_MSK_STS0, read MIZAR_LSS_SYSREG_RAW_STCR0, read MIZAR_USB_GEVNTCOUNT into event_count, set event_counter=event_count, write event_count back to MIZAR_USB_GEVNTCOUNT to acknowledge, if rd_data && 0x80000000 (note: logical AND bug, should be bitwise &) write 0x80000000 to MIZAR_LSS_SYSREG_RAW_STCR0, call GIC_ClearIRQ(84).",
        "Test Steps / Procedure": "1. Initialize the platform and enable all GIC interrupts. 2. Clear the data buffer and event TRB memory regions (20 DWORDs each). 3. Perform a soft reset by writing to DCTL and polling until the reset completes. 4. Configure the USB2 PHY via GUSB2PHYCFG. 5. Set up the event buffer by writing the event ring base address to GEVNTADRLO, clearing GEVNTADRHI, setting the event buffer size in GEVNTSIZ, and clearing GEVNTCOUNT. 6. Configure the global controller register GCTL for device mode operation with port direction set to device. 7. Set device configuration in DCFG and enable device events (disconnect, USB reset, connect done, link state change, wakeup) in DEVTEN. 8. Configure the user control register GUCTL. 9. Issue Start New Configuration command and configure 8 physical endpoints using endpoint command registers DEPCMDPAR1, DEPCMDPAR0, and DEPCMD, polling each command to completion. 10. Allocate TX resources for all 8 endpoints by issuing transfer resource configuration commands and polling for completion. 11. Enable physical endpoints 0 and 1 via DALEPENA. 12. Start the USB controller by writing the run/stop bit in DCTL and enable the system-level interrupt. 13. Wait for link state connect and reset events via interrupt-driven handshake. 14. Re-configure DCFG and wait for additional device events if needed. 15. Read DSTS to check device status, update DCTL, and enable all 8 endpoints via DALEPENA. 16. Execute the control transfer setup stage by preparing a TRB and issuing a start transfer command on endpoint 0, polling for completion and waiting for the transfer complete interrupt. 17. Update the USB2 PHY configuration via GUSB2PHYCFG and wait for events. 18. Perform SET_ADDRESS by updating DCFG with the device address and issuing a data stage transfer on endpoint 1 IN. 19. Synchronize with external handshake by polling a memory-mapped status location until ready. 20. Execute full USB enumeration including GET_DEVICE_DESCRIPTOR, GET_CONFIGURATION_DESCRIPTOR, SET_CONFIGURATION, and GET_FULL_CONFIGURATION_DESCRIPTOR control transfers, each with setup, data, and status stages. 21. Synchronize with a second external handshake by polling another memory-mapped status location. 22. Poll DSTS for a valid frame number (non-zero in the frame number field). 23. Initiate the first isochronous OUT transfer by preparing a TRB with the isochronous data buffer, size, and isochronous control flags, then issuing a start transfer command on the isochronous endpoint with a target frame number. Poll the command to completion and wait for the transfer complete interrupt. 24. Poll DSTS for frame number 2, then wait for the next interrupt. 25. Poll DSTS for frame number 3, then wait for the next interrupt. 26. Initiate the second isochronous OUT transfer on another endpoint with a different target frame number, poll the command to completion, and wait for the transfer complete interrupt. 27. Poll DSTS for frame number 4 and wait for the final interrupt. 28. Verify test completion by calling finish. 29. In the interrupt handler: read system register masked status and raw status, read and acknowledge the event count via GEVNTCOUNT, clear the system register interrupt, and clear GIC IRQ 84.",
        "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; Buffer_PointerLO_1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
        "Meta Validation / Acceptance Criteria": "1. Soft reset validation: MIZAR_USB_DCTL must read 0xf00000 after soft reset (while loop polls until rd_data != 0xf00000 exits, confirming DCTL[30] CoreSoftReset bit cleared). 2. Endpoint command completion: each write to MIZAR_USB_DEPCMD+(offset) is polled until the read value no longer equals the written command value, indicating the CmdAct bit has cleared (command accepted by controller). This applies to set_configuration() calls (command 0x401, 0x409), TX resource allocation (command 0x402), setup_stage/status_stage/data_stage start transfers (command 0x506), and isochronous start transfers (commands 0x20506, 0x40506). 3. Interrupt handling: int_pend flag is set to 1 before waiting and cleared to 0 inside Default_IRQHandler, confirming interrupt was received. event_counter is checked against 0x4 to determine if additional event waits are needed. 4. Handshake polling: read_reg(0xa0243ff4) must return non-zero to proceed past first handshake. read_reg(0xa0243ff8) must return non-zero to proceed past second handshake. 5. Frame number validation: MIZAR_USB_DSTS bits[11:3] must become non-zero after enumeration (first frame number check). MIZAR_USB_DSTS bits[5:3] must equal 0x2 (frame 2), 0x3 (frame 3), and 0x4 (frame 4) at successive isochronous transfer points. 6. IRQ handler validation: MIZAR_USB_GEVNTCOUNT is read and written back to acknowledge events. MIZAR_LSS_SYSREG_RAW_STCR0 is written with 0x80000000 to clear the interrupt if bit 31 is set (note: source uses && logical AND instead of & bitwise AND). GIC_ClearIRQ(84) is called to clear the GIC interrupt. 7. Progress markers: 0xA0243ffc is written with values 0xdeadbee0 through 0xdeadbee9 at various stages to track test progress. 8. Test passes if finish(0) is reached, indicating all polling loops exited successfully, all endpoint commands completed, all interrupts were serviced, all handshakes passed, and all frame number targets were met.",
        "Validation / Acceptance Criteria": "1. The soft reset must complete successfully, confirmed by DCTL reading the expected post-reset value. 2. All endpoint configuration commands issued via DEPCMD must complete, verified by polling until the command active indication clears. 3. All interrupt-driven event waits must be satisfied, confirming the interrupt handler correctly services USB events by reading and acknowledging the event count in GEVNTCOUNT and clearing the system register interrupt. 4. External handshake synchronization points must return non-zero values before the test proceeds. 5. After enumeration, DSTS must report a valid non-zero frame number. 6. The first isochronous OUT transfer must complete on the target endpoint, and DSTS must report frame numbers 2 and 3 at the expected synchronization points. 7. The second isochronous OUT transfer must complete on a different endpoint, and DSTS must report frame number 4. 8. The test passes if all transfers complete, all frame number targets are met, and finish is called with a success indication.",
        "Remarks": "The test uses interrupt-driven flow with a global int_pend flag toggled between the main test loop and the Default_IRQHandler. GIC IRQ 84 is used for USB interrupt. The event_counter variable tracks the cumulative event count and gates additional interrupt waits when event_counter is less than or equal to 4. Three RAM-relative buffer addresses (Buffer_PointerLO, Buffer_PointerLO_1, event_trb_addr) have unresolved base addresses since the RAM macro is not available in the provided headers. Three system register macros (MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0) and three direct hex addresses could not be mapped to registers in the USB specification. The isochronous TRB control value indicates IOC, ISP, and Isochronous transfer type. Endpoint commands use per-endpoint register offsets calculated as base register plus endpoint index times a fixed stride. The IRQ handler contains a logical error using logical AND instead of bitwise AND when checking bit 31 of the raw status register.",
        "Code Generation": "Required"
    }
]

# TestPlan columns
testplan_cols = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# MetaData columns
metadata_cols = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_align = Alignment(wrap_text=True, vertical="top")

# Write headers
for col_idx, col_name in enumerate(testplan_cols, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Write data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_cols, 1):
        val = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns
for col_idx, col_name in enumerate(testplan_cols, 1):
    max_len = len(col_name)
    for row in ws_tp.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
        for cell in row:
            if cell.value:
                max_len = max(max_len, min(len(str(cell.value)), 80))
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 80)

# --- MetaData Sheet ---
ws_md = wb.create_sheet("MetaData")

# Write headers
for col_idx, col_name in enumerate(metadata_cols, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Write data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_cols, 1):
        val = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

# Freeze first row
ws_md.freeze_panes = "A2"

# Auto-size columns
for col_idx, col_name in enumerate(metadata_cols, 1):
    max_len = len(col_name)
    for row in ws_md.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
        for cell in row:
            if cell.value:
                max_len = max(max_len, min(len(str(cell.value)), 80))
    ws_md.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 80)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save workbook
wb.save(output_path)

# Validation
from openpyxl import load_workbook
wb_check = load_workbook(output_path)
assert "TestPlan" in wb_check.sheetnames
assert "MetaData" in wb_check.sheetnames
assert os.path.getsize(output_path) > 0
print(f"SUCCESS: {output_path}")
print(f"File size: {os.path.getsize(output_path)} bytes")
print(f"TestPlan rows: {ws_tp.max_row - 1}")
print(f"MetaData rows: {ws_md.max_row - 1}")
