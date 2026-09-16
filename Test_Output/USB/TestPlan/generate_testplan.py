#!/usr/bin/env python3
"""Generate USB TestPlan Excel workbook using openpyxl."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
import json, os, sys
from datetime import datetime, timezone, timedelta

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"

json_data = [
    {
        "Index": "1",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
        "Feature": "Full-Speed Device Bulk Transfer",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"usb.h\"",
        "Meta Macros": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; DWORD; Default_Event_Ring_Array",
        "Meta Arrays": "buf_data[16]",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "",
        "Meta Test Steps / Procedure": "",
        "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; Buffer_PointerLO_1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Validation / Acceptance Criteria": "",
        "Test Description": "This test validates USB Full-Speed Device mode Bulk Transfer functionality. The test performs a soft reset of the USB controller, configures the USB2 PHY, sets up the event buffer, configures the global controller for device mode port direction, configures device settings and enables device events. It then configures all endpoints using endpoint command parameters and start new configuration and set endpoint configuration commands, allocates TX resources for 8 endpoints, and enables physical endpoints. The controller is started and system-level interrupts are enabled. The test waits for link state connect and reset events via interrupts. During enumeration, the device status is read, device configuration is updated, and the controller is set to accept connections. The setup stage prepares Transfer Request Blocks (TRBs) and issues Start Transfer commands, polling the endpoint command register for completion. The test handles SET ADDRESS, GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short and full), and SET CONFIGURATION standard USB requests by populating descriptor data into buffers and issuing data-stage and status-stage transfers. Handshake synchronization is performed by polling dedicated memory locations. After enumeration completes, bulk data transfers are initiated on bulk endpoints by preparing TRBs with bulk data buffer pointers and issuing Start Transfer commands, polling for completion and waiting for transfer complete interrupts. The interrupt handler reads system register interrupt status, acknowledges USB events by reading and writing back the event count register, clears the system register interrupt, and clears the GIC IRQ. The test concludes successfully after all bulk transfers complete.",
        "Test Steps / Procedure": "1. Initialize the system by calling NIC programming and enabling all GIC IRQs.\n2. Clear the data buffer and event TRB buffer regions by writing zeros to 20 DWORD entries.\n3. Initiate a soft reset of the USB controller by writing to the DCTL register and poll until the reset completes.\n4. Configure the USB2 PHY by writing the desired PHY configuration to the GUSB2PHYCFG register.\n5. Set up the event buffer by writing the event ring base address to GEVNTADRLO, clearing GEVNTADRHI, setting event buffer size in GEVNTSIZ, and clearing GEVNTCOUNT.\n6. Configure the global controller for device mode by reading and updating the GCTL register with port direction settings.\n7. Configure device settings by reading and updating the DCFG register, enable device events by writing to the DEVTEN register, and update the GUCTL register.\n8. Issue Start New Configuration command and Set Endpoint Configuration commands for 8 endpoints using DEPCMDPAR1, DEPCMDPAR0, and DEPCMD registers, polling each command for completion.\n9. Allocate TX resources for 8 endpoints by writing resource allocation commands to DEPCMDPAR0 and DEPCMD, polling each for completion.\n10. Enable physical endpoints 0 and 1 by writing to the DALEPENA register.\n11. Start the USB controller by writing the run bit to the DCTL register.\n12. Enable system-level interrupts at the sysreg level.\n13. Wait for link state connect and reset events via interrupt-driven polling.\n14. Read device status from the DSTS register, reconfigure DCFG, update DCTL for accepting connections, and enable all endpoints via DALEPENA.\n15. Execute the setup stage by preparing a TRB and issuing a Start Transfer command on the control endpoint, polling DEPCMD for completion and waiting for the transfer complete interrupt.\n16. Update the USB2 PHY configuration and wait for related interrupts.\n17. Handle SET ADDRESS by updating DCFG with the assigned device address, preparing a status TRB, and issuing a Start Transfer command.\n18. Perform handshake synchronization by polling a dedicated memory location until a non-zero response is received.\n19. Execute the full enumeration sequence: respond to GET DEVICE DESCRIPTOR by populating the buffer with device descriptor data and issuing data and status stage transfers.\n20. Respond to GET CONFIGURATION DESCRIPTOR (short) by populating the buffer with the configuration descriptor header and issuing transfers.\n21. Respond to SET CONFIGURATION by issuing a zero-length status data stage transfer.\n22. Respond to GET CONFIGURATION DESCRIPTOR (full) by populating the buffer with the complete configuration descriptor including interface and endpoint descriptors, and issuing transfers.\n23. Perform a second handshake synchronization by polling another dedicated memory location.\n24. Initiate bulk data transfers on two bulk endpoints by preparing TRBs pointing to the bulk data buffer with 64-byte transfer size and issuing Start Transfer commands, polling for completion and waiting for transfer complete interrupts.\n25. Verify test completion and call finish to end the test.",
        "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
        "Validation / Acceptance Criteria": "1. The USB controller soft reset must complete successfully, confirmed by the DCTL register returning the expected post-reset value.\n2. All endpoint configuration commands (Start New Configuration, Set Endpoint Configuration, Transfer Resource Allocation, Start Transfer) must complete successfully, confirmed by polling the DEPCMD register until the command active bit clears.\n3. All interrupt-driven events (link state, connect, reset, transfer complete) must be received and serviced by the interrupt handler, confirmed by the interrupt pending flag being cleared.\n4. The event count register GEVNTCOUNT must be read and written back in the interrupt handler to properly acknowledge processed events.\n5. System register interrupts must be properly cleared by writing to the raw status/clear register.\n6. Handshake synchronization with the external test environment must succeed, confirmed by the polled memory locations returning non-zero values.\n7. The full USB enumeration sequence must complete: SET ADDRESS, GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short and full), and SET CONFIGURATION must all be handled with proper setup, data, and status stage transfers.\n8. Bulk data transfers on two bulk endpoints must be initiated and completed successfully using 64-byte TRBs.\n9. The test must reach the finish call with a pass status of 0.",
        "Remarks": "The test operates in USB Full-Speed Device mode and performs complete enumeration followed by bulk transfers. Interrupt-driven event handling is used throughout with int_pend flag polling and Default_IRQHandler servicing IRQ 84. Handshake synchronization with an external host or test environment is performed via dedicated memory-mapped locations. Several Agent 3 macro resolutions are ambiguous due to multiple offset definitions in headers (GUSB2PHYCFG, GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, GEVNTCOUNT, DEPCMDPAR0, DEPCMD, DEPCMDPAR1). LSS sysreg macros (MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0) and memory-mapped handshake addresses could not be mapped to spec register names. Buffer_PointerLO and Buffer_PointerLO_1 are RAM buffer addresses, not USB IP registers. The set_configuration() helper function is called with varying offsets to configure multiple endpoints. The enumeration() function handles multiple standard USB descriptor requests. The test uses GIC IRQ 84 for USB interrupt handling.",
        "Code Generation": ""
    },
    {
        "Index": "2",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Interrupt_Transfer_test",
        "Feature": "Full-Speed Device Interrupt Transfer",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"usb.h\"",
        "Meta Macros": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; DWORD; Default_Event_Ring_Array; Buffer_PointerLO_1",
        "Meta Arrays": "buf_data[16]",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "",
        "Meta Test Steps / Procedure": "",
        "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Validation / Acceptance Criteria": "",
        "Test Description": "This test validates USB Full-Speed Device mode Interrupt Transfer functionality. The test performs a soft reset of the USB controller, configures the USB2 PHY, sets up the event buffer, configures the global controller for device mode port direction, configures device settings and enables device events. It then configures all endpoints using endpoint command parameters with Start New Configuration and Set Endpoint Configuration commands for 8 endpoints including interrupt endpoint types, allocates TX resources for 8 endpoints, and enables physical endpoints. The controller is started and system-level interrupts are enabled. The test waits for link state connect and reset events via interrupts. During enumeration, the device status is read, device configuration is updated, and the controller is set to accept connections. The setup stage prepares Transfer Request Blocks (TRBs) and issues Start Transfer commands, polling the endpoint command register for completion. The test handles SET ADDRESS, GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short and full with interface and endpoint descriptors), and SET CONFIGURATION standard USB requests by populating descriptor data into buffers and issuing data-stage and status-stage transfers. Handshake synchronization is performed by polling dedicated memory locations. After enumeration completes, interrupt data transfers are initiated on two interrupt endpoints by preparing TRBs pointing to the interrupt data buffer with 64-byte transfer size and issuing Start Transfer commands, polling for completion and waiting for transfer complete interrupts. The device status register is then polled to wait for a valid frame number. The interrupt handler reads system register interrupt status, acknowledges USB events by reading and writing back the event count register, clears the system register interrupt, and clears the GIC IRQ. The test concludes successfully after all interrupt transfers complete and a valid frame number is detected.",
        "Test Steps / Procedure": "1. Initialize the system by calling NIC programming and enabling all GIC IRQs.\n2. Clear the data buffer and event TRB buffer regions by writing zeros to 20 DWORD entries.\n3. Initiate a soft reset of the USB controller by writing to the DCTL register and poll until the reset completes.\n4. Configure the USB2 PHY by writing the desired PHY configuration to the GUSB2PHYCFG register.\n5. Set up the event buffer by writing the event ring base address to GEVNTADRLO, clearing GEVNTADRHI, setting event buffer size in GEVNTSIZ, and clearing GEVNTCOUNT.\n6. Configure the global controller for device mode by reading and updating the GCTL register with port direction settings.\n7. Configure device settings by reading and updating the DCFG register, enable device events by writing to the DEVTEN register, and update the GUCTL register.\n8. Issue Start New Configuration command and Set Endpoint Configuration commands for 8 endpoints including interrupt endpoint types using DEPCMDPAR1, DEPCMDPAR0, and DEPCMD registers, polling each command for completion.\n9. Allocate TX resources for 8 endpoints by writing resource allocation commands to DEPCMDPAR0 and DEPCMD, polling each for completion.\n10. Enable physical endpoints 0 and 1 by writing to the DALEPENA register.\n11. Start the USB controller by writing the run bit to the DCTL register.\n12. Enable system-level interrupts at the sysreg level.\n13. Wait for link state connect and reset events via interrupt-driven polling.\n14. Read device status from the DSTS register, reconfigure DCFG, update DCTL for accepting connections, and enable all endpoints via DALEPENA.\n15. Execute the setup stage by preparing a TRB and issuing a Start Transfer command on the control endpoint, polling DEPCMD for completion and waiting for the transfer complete interrupt.\n16. Update the USB2 PHY configuration and wait for related interrupts.\n17. Handle SET ADDRESS by updating DCFG with the assigned device address, preparing a status TRB, and issuing a Start Transfer command.\n18. Perform handshake synchronization by polling a dedicated memory location until a non-zero response is received.\n19. Execute the full enumeration sequence: respond to GET DEVICE DESCRIPTOR by populating the buffer with device descriptor data and issuing data and status stage transfers.\n20. Respond to GET CONFIGURATION DESCRIPTOR (short) by populating the buffer with the configuration descriptor header and issuing transfers.\n21. Respond to SET CONFIGURATION by issuing a zero-length status data stage transfer.\n22. Respond to GET CONFIGURATION DESCRIPTOR (full) by populating the buffer with the complete configuration descriptor including interface and endpoint descriptors for interrupt and bulk endpoints, and issuing transfers.\n23. Perform a second handshake synchronization by polling another dedicated memory location.\n24. Initiate interrupt data transfers on two interrupt endpoints by preparing TRBs pointing to the interrupt data buffer with 64-byte transfer size and interrupt TRB type, issuing Start Transfer commands, polling for completion and waiting for transfer complete interrupts.\n25. Poll the DSTS register to wait for a valid frame number to confirm the device is operating on the bus.\n26. Verify test completion and call finish to end the test.",
        "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
        "Validation / Acceptance Criteria": "1. The USB controller soft reset must complete successfully, confirmed by the DCTL register returning the expected post-reset value.\n2. All endpoint configuration commands must complete successfully, confirmed by polling the DEPCMD register until the command active bit clears.\n3. All interrupt-driven events must be received and serviced by the interrupt handler.\n4. The event count register GEVNTCOUNT must be read and written back in the interrupt handler to properly acknowledge processed events.\n5. System register interrupts must be properly cleared.\n6. Handshake synchronization must succeed.\n7. The full USB enumeration sequence must complete.\n8. Interrupt data transfers on two interrupt endpoints must be initiated and completed successfully using 64-byte TRBs with interrupt transfer type.\n9. The DSTS register must report a valid non-zero frame number.\n10. The test must reach the finish call with a pass status of 0.",
        "Remarks": "The test operates in USB Full-Speed Device mode and performs complete enumeration followed by interrupt transfers. TRB control value 0x815 indicates an interrupt transfer type.",
        "Code Generation": ""
    },
    {
        "Index": "3",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Isochronous_Transfer_test",
        "Feature": "Full-Speed Device Isochronous Transfer",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"usb.h\"",
        "Meta Macros": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; DWORD; Default_Event_Ring_Array; Buffer_PointerLO_1",
        "Meta Arrays": "buf_data[16]",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "",
        "Meta Test Steps / Procedure": "",
        "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Validation / Acceptance Criteria": "",
        "Test Description": "This test validates USB Full-Speed Device mode Isochronous Transfer functionality. The test performs a soft reset of the USB controller, configures the USB2 PHY, sets up the event buffer, configures the global controller for device mode port direction, configures device settings and enables device events. It then configures all endpoints using endpoint command parameters with Start New Configuration and Set Endpoint Configuration commands for 8 endpoints including isochronous endpoint types, allocates TX resources for 8 endpoints, and enables physical endpoints. The controller is started and system-level interrupts are enabled. After enumeration, the device status register is polled to wait for a valid frame number. Two isochronous OUT transfers are initiated on separate isochronous endpoints by preparing TRBs with 1023-byte transfer size and isochronous TRB type, issuing Start Transfer commands with specific frame numbers. Between and after each isochronous transfer, the device status register is polled to synchronize with specific USB frame numbers (frame 2, frame 3, and frame 4), ensuring proper isochronous timing. The test concludes successfully after all isochronous transfers complete and frame number synchronization is verified.",
        "Test Steps / Procedure": "1. Initialize the system.\n2. Clear buffers.\n3. Soft reset USB controller via DCTL.\n4. Configure USB2 PHY via GUSB2PHYCFG.\n5. Set up event buffer.\n6. Configure GCTL for device mode.\n7. Configure DCFG, DEVTEN, GUCTL.\n8. Configure 8 endpoints including isochronous types.\n9. Allocate TX resources.\n10. Enable endpoints via DALEPENA.\n11. Start controller.\n12. Enable interrupts.\n13. Wait for connect/reset events.\n14. Perform enumeration.\n15. Poll DSTS for valid frame number.\n16. First isochronous OUT transfer with 1023-byte TRB.\n17. Poll DSTS for frame number 2.\n18. Second isochronous OUT transfer.\n19. Poll DSTS for frame numbers 3 and 4.\n20. Verify and finish.",
        "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
        "Validation / Acceptance Criteria": "1. Soft reset must complete.\n2. All endpoint commands must complete.\n3. All interrupts must be serviced.\n4. DSTS must report valid frame number before isochronous transfers.\n5. Two isochronous OUT transfers must complete with 1023-byte TRBs.\n6. Frame number synchronization (frames 2, 3, 4) must succeed.\n7. Test must reach finish(0).",
        "Remarks": "Isochronous transfer test. TRB control 0x869 indicates isochronous type. DEPCMD values 0x20506 and 0x40506 include frame number parameters.",
        "Code Generation": ""
    },
    {
        "Index": "4",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_fs",
        "Feature": "Host Mode Full-Speed Enumeration",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"usb.h\"",
        "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; USB_PORTSC_20_WCE; USB_PORTSC_20_WDE; USB_PORTSC_20_WOE; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; Default_Event_Ring_Array; DWORD; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; SCRATCHPAD0; SCRATCHPAD1; Device_Context_Base_Address_Array; Device_Context_Array; MIZAR_USB_CRCR_LO; Default_Command_Ring; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Input_Context; MIZAR_USB_DB; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Arrays": "data_in[512]; data_out[512]",
        "Speed": "Full-Speed",
        "Mode": "Host Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "",
        "Meta Test Steps / Procedure": "",
        "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; Default_Event_Ring_Array; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Validation / Acceptance Criteria": "",
        "Test Description": "This test validates USB Host mode Full-Speed device enumeration using the xHCI host controller interface. The test configures the global controller, frame length adjustment, and user control registers. It sets up the USB3 PIPE control and USB2 PHY control registers. Host capability parameters are read including port count, supported protocol capabilities for USB2 and USB3, and page size. The port status and control register is configured with wake-on-connect, wake-on-disconnect, and wake-on-overcurrent enable bits. The xHCI data structures are initialized: Event Ring Segment Table, Scratchpad Buffer Array, Device Context Base Address Array, Command Ring. The host controller is started. After device connection and port reset, Enable Slot, Address Device (BSR=1 and BSR=0), and Configure Endpoint commands are issued. Enumeration issues GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short and full), and SET CONFIGURATION via TRBs. Event completion is verified by polling the Event Ring.",
        "Test Steps / Procedure": "1. Initialize system.\n2. Configure GCTL, GFLADJ, GUCTL.\n3. Configure PIPE and PHY registers.\n4. Read HCSPARAMS1, SUPTPRT2_DW2, SUPTPRT3_DW2.\n5. Configure PORTSC_20 with wake enables.\n6. Set up xHCI data structures.\n7. Configure Command Ring, CONFIG, DCBAAP, Event Ring.\n8. Start host controller.\n9. Wait for port status change.\n10. Port reset.\n11. Enable Slot Command.\n12. Address Device BSR=1.\n13. Address Device BSR=0.\n14. Configure Endpoint.\n15. GET DEVICE DESCRIPTOR.\n16. GET CONFIGURATION DESCRIPTOR (short).\n17. SET CONFIGURATION.\n18. GET CONFIGURATION DESCRIPTOR (full).\n19. Poll Event Ring for completion.\n20. Finish.",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Validation / Acceptance Criteria": "1. Port status change event received.\n2. Port connect status confirmed.\n3. Port reset complete.\n4. Enable Slot completed.\n5. Address Device commands completed.\n6. Configure Endpoint completed.\n7. All enumeration transfers completed.\n8. Interrupts properly acknowledged.\n9. Test finishes with pass.",
        "Remarks": "Host mode xHCI Full-Speed enumeration. Slot context 0x08100000 indicates Full-Speed.",
        "Code Generation": ""
    },
    {
        "Index": "5",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_hs",
        "Feature": "Host Mode High-Speed Enumeration",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"usb.h\"",
        "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; USB_PORTSC_20_WCE; USB_PORTSC_20_WDE; USB_PORTSC_20_WOE; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; Default_Event_Ring_Array; DWORD; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; SCRATCHPAD0; SCRATCHPAD1; Device_Context_Base_Address_Array; Device_Context_Array; MIZAR_USB_CRCR_LO; Default_Command_Ring; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Input_Context; MIZAR_USB_DB; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Arrays": "data_in[512]; data_out[512]",
        "Speed": "High-Speed",
        "Mode": "Host Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "",
        "Meta Test Steps / Procedure": "",
        "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; Default_Event_Ring_Array; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Validation / Acceptance Criteria": "",
        "Test Description": "This test validates USB Host mode High-Speed device enumeration using the xHCI host controller interface. Similar to Full-Speed enumeration but additionally issues GET DEVICE QUALIFIER and a second GET CONFIGURATION DESCRIPTOR request. Slot context uses 0x08300000 for High-Speed. GFLADJ value is 0xa07f000 for High-Speed.",
        "Test Steps / Procedure": "1. Initialize system.\n2. Configure GCTL, GFLADJ, GUCTL.\n3. Configure PIPE and PHY registers.\n4. Read capabilities.\n5. Configure port.\n6. Set up xHCI data structures.\n7. Start host controller.\n8. Wait for connection and port reset.\n9. Enable Slot, Address Device (BSR=1, BSR=0).\n10. Configure Endpoint.\n11. GET DEVICE DESCRIPTOR.\n12. GET DEVICE QUALIFIER.\n13. GET CONFIGURATION DESCRIPTOR (short).\n14. SET CONFIGURATION.\n15. GET CONFIGURATION DESCRIPTOR 2 (short).\n16. GET CONFIGURATION DESCRIPTOR (full).\n17. Poll Event Ring.\n18. Finish.",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Validation / Acceptance Criteria": "1. Port status change and reset complete.\n2. All xHCI commands completed.\n3. All enumeration transfers including GET DEVICE QUALIFIER completed.\n4. Interrupts properly acknowledged.\n5. Test finishes with pass.",
        "Remarks": "Host mode xHCI High-Speed enumeration. Slot context 0x08300000 indicates High-Speed. Includes GET DEVICE QUALIFIER request.",
        "Code Generation": ""
    },
    {
        "Index": "6",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_ls",
        "Feature": "Host Mode Low-Speed Enumeration",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"usb.h\"",
        "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; USB_PORTSC_20_WCE; USB_PORTSC_20_WDE; USB_PORTSC_20_WOE; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; Default_Event_Ring_Array; DWORD; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; SCRATCHPAD0; SCRATCHPAD1; Device_Context_Base_Address_Array; Device_Context_Array; MIZAR_USB_CRCR_LO; Default_Command_Ring; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Input_Context; MIZAR_USB_DB; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Arrays": "data_in[512]; data_out[512]",
        "Speed": "Low-Speed",
        "Mode": "Host Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "",
        "Meta Test Steps / Procedure": "",
        "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Command_Ring; MIZAR_USB_DB; Default_Input_Context; Default_Event_Ring_Array; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Validation / Acceptance Criteria": "",
        "Test Description": "This test validates USB Host mode Low-Speed device enumeration using the xHCI host controller interface. EP0 max packet size is 8 bytes for Low-Speed. GET DEVICE QUALIFIER is not issued. Configure Endpoint is skipped. Full config descriptor is 24 bytes.",
        "Test Steps / Procedure": "1. Initialize system.\n2. Configure GCTL, GFLADJ, GUCTL.\n3. Configure PIPE and PHY registers.\n4. Read capabilities.\n5. Configure port.\n6. Set up xHCI data structures.\n7. Start host controller.\n8. Wait for connection and port reset.\n9. Enable Slot, Address Device (BSR=1, BSR=0).\n10. GET DEVICE DESCRIPTOR.\n11. GET CONFIGURATION DESCRIPTOR (short).\n12. SET CONFIGURATION.\n13. GET CONFIGURATION DESCRIPTOR 2 (short).\n14. GET CONFIGURATION DESCRIPTOR (full, 24 bytes).\n15. Poll Event Ring.\n16. Finish.",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Validation / Acceptance Criteria": "1. Port status change and reset complete.\n2. All xHCI commands completed.\n3. All enumeration transfers completed.\n4. Interrupts properly acknowledged.\n5. Test finishes with pass.",
        "Remarks": "Host mode xHCI Low-Speed enumeration. Slot context 0x08200000 indicates Low-Speed. EP0 max packet size is 8 bytes. Configure Endpoint is skipped.",
        "Code Generation": ""
    }
]

# TestPlan columns
tp_cols = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
           "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
           "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
           "Code Generation"]

# MetaData columns
md_cols = ["Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
           "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
           "Meta Headers", "Meta Macros", "Meta Arrays"]

wb = openpyxl.Workbook()
ws_tp = wb.active
ws_tp.title = "TestPlan"
ws_md = wb.create_sheet("MetaData")

header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

for ci, col_name in enumerate(tp_cols, 1):
    c = ws_tp.cell(row=1, column=ci, value=col_name)
    c.font = header_font
    c.fill = header_fill
    c.alignment = wrap

for ci, col_name in enumerate(md_cols, 1):
    c = ws_md.cell(row=1, column=ci, value=col_name)
    c.font = header_font
    c.fill = header_fill
    c.alignment = wrap

for ri, row in enumerate(json_data, 2):
    for ci, col in enumerate(tp_cols, 1):
        val = row.get(col, "")
        c = ws_tp.cell(row=ri, column=ci, value=val)
        c.alignment = wrap
    for ci, col in enumerate(md_cols, 1):
        val = row.get(col, "")
        c = ws_md.cell(row=ri, column=ci, value=val)
        c.alignment = wrap

ws_tp.freeze_panes = "A2"
ws_md.freeze_panes = "A2"

for ws in [ws_tp, ws_md]:
    for col_cells in ws.columns:
        max_len = 0
        col_letter = get_column_letter(col_cells[0].column)
        for cell in col_cells:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
        width = min(max_len + 2, 60)
        ws.column_dimensions[col_letter].width = max(width, 12)

ws_md.sheet_state = "veryHidden"

script_dir = os.path.dirname(os.path.abspath(__file__)) if '__file__' in dir() else '.'
out_path = os.path.join(script_dir, filename)
wb.save(out_path)
print(f"GENERATED:{filename}")
print(f"PATH:{out_path}")
print(f"SIZE:{os.path.getsize(out_path)}")

# Validate
wb2 = openpyxl.load_workbook(out_path)
sheets = wb2.sheetnames
print(f"SHEETS:{sheets}")
print(f"ROWS_TP:{ws_tp.max_row - 1}")
print(f"ROWS_MD:{ws_md.max_row - 1}")
print("VALIDATION:PASSED")
