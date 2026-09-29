#!/usr/bin/env python3
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl'])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

json_data = [
    {
        "Index": "1",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
        "Feature": "Device Mode Bulk Transfer",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"usb.h\"",
        "Meta Macros": "Buffer_PointerLO; Buffer_PointerLO_1; event_trb_addr; DWORD; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; Default_Event_Ring_Array",
        "Meta Arrays": "buf_data[16]",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Memory Start Offset": "0x1100",
        "Memory End Offset": "0x3000",
        "Meta Test Description": "This testcase validates USB Full-Speed Device mode Bulk Transfer operation.",
        "Test Description": "This test validates USB Full-Speed Device mode Bulk Transfer functionality. The test initializes the USB controller by performing a soft reset via the device control register and polling for completion. It configures the USB2 PHY, sets up the event buffer address and size registers, configures the global control register for device mode port direction, sets device configuration and enables device events. Endpoint configuration is performed for multiple endpoints using the endpoint command interface with Start New Configuration and Set Endpoint Configuration commands, followed by Transfer Resource Configuration for 8 endpoints. Physical endpoints are enabled via the active endpoint enable register. The controller is started and system-level interrupts are enabled. The test then handles link state connect and reset events through interrupt-driven flow. Device enumeration is performed including GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short and full), and SET CONFIGURATION control transfers, each involving setup, data, and status stages using TRB-based transfers through the endpoint command interface. After enumeration, Bulk OUT transfers are initiated on bulk endpoints with 64-byte data payloads. Each endpoint command is polled for completion. An interrupt handler services USB events by reading the event count register, acknowledging events, and clearing system-level interrupt status. The test uses handshake polling on external registers for synchronization between enumeration phases and completes with a final status marker write.",
        "Meta Test Steps / Procedure": "1. Call nic_programming() to perform NIC initialization...",
        "Test Steps / Procedure": "1. Initialize NIC programming and enable all GIC IRQs. 2. Clear buffer and event TRB memory regions by writing zeros to 20 DWORD locations. 3. Perform USB controller soft reset by writing to the device control register and polling until reset completes. 4. Configure USB2 PHY settings via the PHY configuration register. 5. Set up event buffer by writing event address low, address high, event size, and clearing event count registers. 6. Read and configure the global control register for device mode port direction. 7. Read and configure the device configuration register for Full-Speed operation. 8. Enable device events via the device event enable register. 9. Read and configure the USB controller utility register. 10. Issue Start New Configuration command on endpoint 0 and poll for completion. 11. Configure 8 endpoints using Set Endpoint Configuration commands. 12. Configure transfer resources for 8 endpoints. 13. Enable physical endpoints 0 and 1. 14. Start the device controller. 15. Enable system-level USB interrupt. 16. Wait for link state connect and reset event interrupts. 17. Re-write device configuration register. 18. Read device configuration and device status registers. 19. Enable all physical endpoints. 20. Execute setup stage for control transfer. 21. Reconfigure USB2 PHY settings. 22. Set device address. 23. Prepare a status TRB and issue Start Transfer on endpoint 1. 24. Poll external handshake register. 25. Execute full device enumeration sequence. 26. Poll second external handshake register. 27. Initiate Bulk OUT transfer on first bulk endpoint. 28. Initiate Bulk OUT transfer on second bulk endpoint. 29. Write final completion marker. 30. Verify test completes successfully.",
        "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xA0243ffc; 0xa0243ff4; 0xa0243ff8",
        "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
        "Meta Validation / Acceptance Criteria": "Soft reset validation: MIZAR_USB_DCTL is polled until read value equals 0xf00000...",
        "Validation / Acceptance Criteria": "1. Soft reset completes successfully. 2. All endpoint configuration commands complete. 3. All Start Transfer commands complete. 4. USB interrupts are received. 5. Event count is properly acknowledged. 6. External handshake synchronization succeeds. 7. System-level interrupt status is properly cleared. 8. Full device enumeration completes. 9. Two Bulk OUT transfers complete successfully. 10. Test completes successfully.",
        "Remarks": "The test operates in USB Full-Speed Device mode with interrupt-driven event handling using IRQ 84."
    },
    {
        "Index": "2",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Interrupt_Transfer_test",
        "Feature": "Device Mode Interrupt Transfer",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Test Description": "This test validates USB Full-Speed Device mode Interrupt Transfer functionality.",
        "Test Steps / Procedure": "1. Initialize NIC programming and enable all GIC IRQs. 2-26. Same init and enumeration as Bulk test. 27. Initiate Interrupt IN transfer. 28. Initiate Interrupt OUT transfer. 29. Poll device status register for valid frame number. 30-31. Complete test.",
        "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
        "Validation / Acceptance Criteria": "1. Soft reset completes. 2. Endpoint commands complete. 3. Enumeration completes. 4. Interrupt IN/OUT transfers complete. 5. Frame number validation passes. 6. Test completes.",
        "Remarks": "The test operates in USB Full-Speed Device mode. Interrupt transfers use TRB control 0x815 (IN) and 0x813 (OUT)."
    },
    {
        "Index": "3",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Isochronous_Transfer_test",
        "Feature": "Device Mode Isochronous Transfer",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Test Description": "This test validates USB Full-Speed Device mode Isochronous Transfer functionality.",
        "Test Steps / Procedure": "1. Initialize NIC programming and enable all GIC IRQs. 2-26. Same init and enumeration as Bulk test. 27. Poll device status register for valid frame number. 28. Initiate first Isochronous OUT transfer (1023 bytes). 29-30. Poll for frame numbers 2 and 3. 31. Initiate second Isochronous OUT transfer. 32. Poll for frame number 4. 33-34. Complete test.",
        "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
        "Validation / Acceptance Criteria": "1. Soft reset completes. 2. Endpoint commands complete. 3. Enumeration completes. 4. Frame number validation passes. 5. Two Isochronous OUT transfers complete. 6. Test completes.",
        "Remarks": "Isochronous transfers use 1023-byte payloads with TRB control 0x869 and frame-number-aware Start Transfer commands."
    },
    {
        "Index": "4",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_fs",
        "Feature": "Host Mode Enumeration",
        "Speed": "Full-Speed",
        "Mode": "Host Mode",
        "Test Description": "This test validates USB Host mode Full-Speed device enumeration using the xHCI host controller interface.",
        "Test Steps / Procedure": "1. Initialize NIC and GIC. 2-5. Configure global, PHY, and port registers. 6-14. Initialize xHCI data structures. 15. Start host controller. 16-18. Handle port connection and reset. 19-22. Enable Slot, Address Device (BSR=1, BSR=0), Configure Endpoint. 23-28. Execute enumeration transfers. 29. Verify completion.",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Validation / Acceptance Criteria": "1. Port connection detected. 2. Port reset completes. 3. Enable Slot completes. 4. Address Device completes. 5. Configure Endpoint completes. 6. Enumeration transfers complete. 7. Test completes.",
        "Remarks": "Host mode xHCI enumeration targeting Full-Speed device. Slot context 0x08100000."
    },
    {
        "Index": "5",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_hs",
        "Feature": "Host Mode Enumeration",
        "Speed": "High-Speed",
        "Mode": "Host Mode",
        "Test Description": "This test validates USB Host mode High-Speed device enumeration using the xHCI host controller interface.",
        "Test Steps / Procedure": "1. Initialize NIC and GIC. 2-5. Configure global, PHY, and port registers. 6-14. Initialize xHCI data structures. 15. Start host controller. 16-18. Handle port connection and reset. 19-22. Enable Slot, Address Device (BSR=1, BSR=0), Configure Endpoint. 23-28. Execute enumeration transfers including GET DEVICE QUALIFIER. 29-31. Verify completion.",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Validation / Acceptance Criteria": "1. Port connection detected. 2. Port reset completes. 3. Enable Slot completes. 4. Address Device completes. 5. Configure Endpoint completes. 6. Enumeration including GET QUALIFIER completes. 7. Test completes.",
        "Remarks": "Host mode xHCI enumeration targeting High-Speed device. Slot context 0x08300000. Includes GET DEVICE QUALIFIER."
    },
    {
        "Index": "6",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_ls",
        "Feature": "Host Mode Enumeration",
        "Speed": "Low-Speed",
        "Mode": "Host Mode",
        "Test Description": "This test validates USB Host mode Low-Speed device enumeration using the xHCI host controller interface.",
        "Test Steps / Procedure": "1. Initialize NIC and GIC. 2-5. Configure global, PHY, and port registers. 6-14. Initialize xHCI data structures with 48-entry event ring. 15. Start host controller. 16-18. Handle port connection and reset. 19-21. Enable Slot, Address Device (BSR=1, BSR=0). 22-28. Execute enumeration transfers with 24-byte full config descriptor. 29. Verify completion.",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Validation / Acceptance Criteria": "1. Port connection detected. 2. Port reset completes. 3. Enable Slot completes. 4. Address Device completes. 5. Enumeration with 24-byte config descriptor completes. 6. Test completes.",
        "Remarks": "Host mode xHCI enumeration targeting Low-Speed device. Slot context 0x08200000. Max packet size 8 bytes. Configure Endpoint is commented out."
    }
]

# TestPlan columns
tp_cols = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# MetaData columns
md_cols = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

def create_workbook():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    ts = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"USB_TestPlan_{ts}.xlsx"

    wb = Workbook()

    # --- TestPlan sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    header_font = Font(bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_align = Alignment(wrap_text=True, vertical="top")

    # Write headers
    for ci, col_name in enumerate(tp_cols, 1):
        cell = ws_tp.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    # Write data
    for ri, row in enumerate(json_data, 2):
        for ci, col_name in enumerate(tp_cols, 1):
            val = row.get(col_name, "")
            if val is None:
                val = ""
            cell = ws_tp.cell(row=ri, column=ci, value=str(val))
            cell.alignment = wrap_align

    ws_tp.freeze_panes = "A2"

    # Auto-size columns
    for ci, col_name in enumerate(tp_cols, 1):
        max_len = len(col_name)
        for ri in range(2, len(json_data) + 2):
            val = str(ws_tp.cell(row=ri, column=ci).value or "")
            lines = val.split("\n")
            for line in lines:
                if len(line) > max_len:
                    max_len = len(line)
        width = min(max_len + 4, 80)
        if width < 12:
            width = 12
        ws_tp.column_dimensions[get_column_letter(ci)].width = width

    # --- MetaData sheet ---
    ws_md = wb.create_sheet("MetaData")
    for ci, col_name in enumerate(md_cols, 1):
        cell = ws_md.cell(row=1, column=ci, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    for ri, row in enumerate(json_data, 2):
        for ci, col_name in enumerate(md_cols, 1):
            val = row.get(col_name, "")
            if val is None:
                val = ""
            cell = ws_md.cell(row=ri, column=ci, value=str(val))
            cell.alignment = wrap_align

    ws_md.freeze_panes = "A2"

    for ci, col_name in enumerate(md_cols, 1):
        max_len = len(col_name)
        for ri in range(2, len(json_data) + 2):
            val = str(ws_md.cell(row=ri, column=ci).value or "")
            lines = val.split("\n")
            for line in lines:
                if len(line) > max_len:
                    max_len = len(line)
        width = min(max_len + 4, 80)
        if width < 12:
            width = 12
        ws_md.column_dimensions[get_column_letter(ci)].width = width

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save
    output_dir = os.path.dirname(os.path.abspath(__file__))
    filepath = os.path.join(output_dir, filename)
    wb.save(filepath)

    # Validate
    assert os.path.exists(filepath), "File does not exist"
    assert os.path.getsize(filepath) > 0, "File is empty"
    vwb = load_workbook(filepath)
    assert "TestPlan" in vwb.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in vwb.sheetnames, "MetaData sheet missing"
    vwb.close()

    print(f"SUCCESS: {filename}")
    print(f"PATH: {filepath}")
    print(f"SIZE: {os.path.getsize(filepath)} bytes")
    print(f"ROWS_TESTPLAN: {len(json_data)}")
    print(f"ROWS_METADATA: {len(json_data)}")
    return filename, filepath

if __name__ == "__main__":
    create_workbook()
