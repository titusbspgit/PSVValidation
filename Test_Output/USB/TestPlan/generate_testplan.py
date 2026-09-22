#!/usr/bin/env python3
"""USB TestPlan Excel Generator - Auto-generates USB_TestPlan XLSX"""
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter

# IST Timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"

# Complete testcase data from Agent 5 outputs
json_data = [
    {
        "Index": "1",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
        "Feature": "Full-Speed Device Bulk Transfer",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
        "Meta Macros": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; Buffer_PointerLO_1; DWORD; Default_Event_Ring_Array",
        "Meta Arrays": "buf_data[16]",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Memory Start Offset": "0x1100",
        "Memory End Offset": "0x3000",
        "Meta Test Description": "This testcase validates USB Full-Speed (FS) Device mode Bulk Transfer operation on the DWC USB3 controller. The test begins by performing a soft reset via MIZAR_USB_DCTL and polling until reset completes (rd_data == 0xf00000). It then configures the USB2 PHY via MIZAR_USB_GUSB2PHYCFG, sets up the event buffer ring using MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, MIZAR_USB_GEVNTSIZ, and MIZAR_USB_GEVNTCOUNT. The global controller is configured via MIZAR_USB_GCTL with port direction set to device (0x30c12214). Device configuration is written to MIZAR_USB_DCFG (0x480801), device events enabled via MIZAR_USB_DEVTEN (0x1f), and MIZAR_USB_GUCTL configured. Endpoint configuration is performed using set_configuration() which writes MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, and MIZAR_USB_DEPCMD for 8 endpoints with transfer resource allocation (command 0x402). Endpoints are enabled via MIZAR_USB_DALEPENA. The device run/stop bit is set in MIZAR_USB_DCTL. Interrupt handling is enabled at the system register level via MIZAR_LSS_SYSREG_INTR_EN0. The test then handles link state connect/reset events via interrupt-driven polling (int_pend). Enumeration proceeds through setup_stage(), set address (MIZAR_USB_DCFG = 0x480809), get device descriptor, get configuration descriptor, set configuration, and get full configuration descriptor phases. Each phase uses TRB (Transfer Request Block) structures written to event_trb_addr and Buffer_PointerLO memory regions, with endpoint commands issued via MIZAR_USB_DEPCMD and polled for completion. After enumeration, bulk OUT transfers are initiated on physical endpoints 4 and 5 using Buffer_PointerLO_1 as the data buffer, with TRB type 0x813 (bulk). The Default_IRQHandler reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0 for interrupt status, reads and writes back MIZAR_USB_GEVNTCOUNT to acknowledge events, clears the sysreg interrupt, and clears IRQ 84 via GIC_ClearIRQ. Handshake synchronization is performed by polling addresses 0xa0243ff4 and 0xa0243ff8. The test writes marker values (0xdeadbee0 through 0xdeadbee7) to 0xA0243ffc for debug/progress tracking. The test concludes with finish(0).",
        "Test Description": "This test validates USB Full-Speed Device mode Bulk Transfer functionality. The test performs a controller soft reset and waits for completion, then configures the USB2 PHY, event buffer ring, global controller in device mode, device configuration, device event enables, and endpoint parameters. Eight endpoints are configured with transfer resource allocation commands, and physical endpoints are enabled via DALEPENA. The device is set to run mode via DCTL. System-level interrupts are enabled. The test handles link state connect and reset events through interrupt-driven polling. USB enumeration is performed including setup stage, set address via DCFG, get device descriptor, get configuration descriptor, set configuration, and get full configuration descriptor. Each enumeration phase uses Transfer Request Blocks and endpoint commands issued through the endpoint command registers with polling for command completion. After enumeration, bulk OUT transfers are initiated on bulk endpoints using dedicated data buffers. An interrupt handler reads system register interrupt status, acknowledges USB events through the event count register, clears system register interrupts, and clears the GIC IRQ. Handshake synchronization with an external agent is performed by polling dedicated memory-mapped locations. Debug progress markers are written at key stages. The test ends by calling finish.",
        "Meta Test Steps / Procedure": "1. Call nic_programming() and GIC_EnableAllIRQ() for NIC and GIC initialization. 2. Clear Buffer_PointerLO and event_trb_addr memory regions by writing 0x0 in a loop (j=0 to 19, each j*DWORD offset). 3. Write 0x40f00000 to MIZAR_USB_DCTL to initiate soft reset. 4. Poll MIZAR_USB_DCTL until read value equals 0xf00000 (soft reset complete), with wait_on(5) between polls. 5. Write 0x40002407 to MIZAR_USB_GUSB2PHYCFG to configure USB2 PHY. 6. Write Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO, 0x0 to MIZAR_USB_GEVNTADRHI, 0x30 to MIZAR_USB_GEVNTSIZ, 0x0 to MIZAR_USB_GEVNTCOUNT to set up event buffer. 7. Read MIZAR_USB_GCTL, then write 0x30c12214 to MIZAR_USB_GCTL to set port direction to device mode. 8. Read MIZAR_USB_DCFG, then write 0x480801 to MIZAR_USB_DCFG. 9. Write 0x1f to MIZAR_USB_DEVTEN to enable device events. 10. Read MIZAR_USB_GUCTL, then write 0xa400010 to MIZAR_USB_GUCTL. 11-31. Full endpoint configuration, enumeration, and bulk transfer steps.",
        "Test Steps / Procedure": "1. Initialize the NIC and enable all GIC IRQs. 2. Clear the data buffer and event TRB memory regions. 3. Perform a controller soft reset by writing to DCTL and poll until reset completes. 4. Configure the USB2 PHY register for Full-Speed operation. 5. Set up the event buffer ring by writing the event address low, event address high, event size, and event count registers. 6. Configure the global controller register GCTL for device mode port direction. 7. Write device configuration to DCFG and enable device events in DEVTEN. 8. Configure GUCTL with the desired timeout and control settings. 9. Issue Start New Configuration command and configure 8 endpoints using endpoint command parameter and command registers, polling each command for completion. 10. Allocate transfer resources for 8 endpoints by issuing transfer resource allocation commands and polling for completion. 11. Enable physical endpoints 0 and 1 via DALEPENA. 12. Set the device run/stop bit in DCTL to start the controller. 13. Enable system-level USB interrupt. 14. Wait for link state connect and reset event interrupts. 15. Re-apply device configuration to DCFG and wait for additional events if needed. 16. Write enumeration debug marker. Read DCFG and DSTS for status. Re-configure DCFG, set DCTL with updated control, and enable all endpoints via DALEPENA. 17. Execute setup stage: build a control TRB, issue Start Transfer command on endpoint 0, poll for completion, and wait for interrupt. 18. Update USB2 PHY configuration and wait for interrupts. 19. Set device address by updating DCFG, build a status TRB, issue Start Transfer on endpoint 1, poll for completion, and wait for interrupt. 20. Poll handshake location until external agent signals readiness. 21. Execute full enumeration sequence. 22. Poll second handshake location until external agent signals readiness. 23. Initiate bulk OUT transfer on endpoint 4. 24. Initiate bulk OUT transfer on endpoint 5. 25. Write final debug marker and wait for last interrupt. 26. Call finish to end the test. 27. Verify interrupt handler correctly reads system register interrupt status, acknowledges USB events, clears system register interrupt, and clears the GIC IRQ.",
        "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; Buffer_PointerLO_1",
        "Impacted Registers": "DCTL; GCTL; DCFG; DEVTEN; GUCTL; DALEPENA; DSTS; DEPCMDPAR1",
        "Meta Validation / Acceptance Criteria": "1. After writing 0x40f00000 to MIZAR_USB_DCTL, polling must eventually return 0xf00000 indicating soft reset completion. 2. Each endpoint command (0x402 for transfer resource allocation, 0x401 for endpoint config, 0x409 for start new config, 0x506 for start transfer) written to MIZAR_USB_DEPCMD+(offset) must complete: the polled read of MIZAR_USB_DEPCMD+(offset) must return a value != the command value. 3. int_pend must be cleared to 0 by Default_IRQHandler for each interrupt wait loop to proceed. 4. Default_IRQHandler must successfully read MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, read MIZAR_USB_GEVNTCOUNT, write back event_count to MIZAR_USB_GEVNTCOUNT, and conditionally clear the sysreg interrupt by writing 0x80000000 to MIZAR_LSS_SYSREG_RAW_STCR0. 5. Handshake polling at 0xa0243ff4 must return non-zero to proceed past enumeration setup. 6. Handshake polling at 0xa0243ff8 must return non-zero to proceed past enumeration. 7. Bulk transfer TRBs on EP4 (offset 0x40) and EP5 (offset 0x50) must complete. 8. The test must reach finish(0) without hanging in any polling loop.",
        "Validation / Acceptance Criteria": "1. Controller soft reset must complete: DCTL must return the expected reset-done value after the soft reset write. 2. All endpoint configuration commands must complete successfully: each endpoint command register must transition from the issued command value to a completion value when polled. 3. Each interrupt wait must be resolved by the interrupt handler clearing the pending flag. 4. The interrupt handler must correctly read system register interrupt status, acknowledge USB events by reading and writing back the event count register, clear the system register interrupt, and clear GIC IRQ 84. 5. The first handshake polling location must return a non-zero value to proceed past the set-address phase. 6. The second handshake polling location must return a non-zero value to proceed past the enumeration phase. 7. Bulk OUT transfer commands on endpoints 4 and 5 must complete. 8. The test must reach the finish call without hanging in any polling or interrupt wait loop.",
        "Remarks": "The test relies on an external USB host agent for handshake synchronization via dedicated memory-mapped polling locations. The interrupt handler uses a bitwise AND with a logical operator (&&) instead of bitwise (&) for the sysreg interrupt check, which may be a source code defect. Several Agent 4 register mappings are unresolved due to register-group-relative offsets in the specification not matching the absolute offsets used in the header file."
    },
    {
        "Index": "2",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Interrupt_Transfer_test",
        "Feature": "Full-Speed Device Interrupt Transfer",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
        "Meta Macros": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; Buffer_PointerLO_1; DWORD; Default_Event_Ring_Array",
        "Meta Arrays": "buf_data[16]",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Memory Start Offset": "0x1100",
        "Memory End Offset": "0x3000",
        "Meta Test Description": "This testcase validates USB Full-Speed (FS) Device mode Interrupt Transfer operation on the DWC USB3 controller. The test performs soft reset, PHY configuration, event buffer setup, global controller configuration in device mode, endpoint configuration including interrupt endpoint types, enumeration, and interrupt IN transfer on endpoint 2 with TRB type 0x815 (interrupt), followed by a second transfer on endpoint 3. The test polls DSTS for a non-zero frame number.",
        "Test Description": "This test validates USB Full-Speed Device mode Interrupt Transfer functionality. The test performs a controller soft reset and waits for completion, then configures the USB2 PHY, event buffer ring, global controller in device mode, device configuration, device event enables, and endpoint parameters. After enumeration, an interrupt IN transfer is initiated on endpoint 2 using a dedicated data buffer with an interrupt TRB type, followed by a second transfer on endpoint 3. The test then polls DSTS to wait for a non-zero frame number.",
        "Meta Test Steps / Procedure": "1. Call nic_programming() and GIC_EnableAllIRQ(). 2. Clear memory regions. 3. Soft reset via MIZAR_USB_DCTL. 4. Configure PHY, event buffer, GCTL, DCFG, DEVTEN, GUCTL. 5. Configure 8 endpoints and allocate transfer resources. 6. Enable endpoints, set run/stop, enable interrupts. 7. Handle link state events. 8. Perform enumeration. 9. Initiate interrupt IN transfer on EP2 (TRB control=0x815). 10. Initiate transfer on EP3 (TRB control=0x813). 11. Poll DSTS for non-zero frame number. 12. Call finish(0).",
        "Test Steps / Procedure": "1. Initialize the NIC and enable all GIC IRQs. 2. Clear the data buffer and event TRB memory regions. 3. Perform a controller soft reset by writing to DCTL and poll until reset completes. 4. Configure USB2 PHY, event buffer ring, GCTL for device mode, DCFG, DEVTEN, GUCTL. 5. Configure 8 endpoints and allocate transfer resources. 6. Enable endpoints via DALEPENA, set run/stop in DCTL, enable system interrupt. 7. Handle link state connect and reset events. 8. Perform full USB enumeration. 9. Initiate interrupt IN transfer on endpoint 2. 10. Initiate second transfer on endpoint 3. 11. Poll DSTS for non-zero frame number. 12. Call finish.",
        "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; Buffer_PointerLO_1",
        "Impacted Registers": "DCTL; GCTL; DCFG; DEVTEN; GUCTL; DALEPENA; DSTS",
        "Meta Validation / Acceptance Criteria": "1. Soft reset must complete. 2. All endpoint commands must complete. 3. Interrupt IN transfer on EP2 must complete. 4. Transfer on EP3 must complete. 5. DSTS must return non-zero frame number. 6. Test must reach finish(0).",
        "Validation / Acceptance Criteria": "1. Controller soft reset must complete. 2. All endpoint configuration commands must complete successfully. 3. Interrupt IN transfer command on endpoint 2 must complete. 4. Transfer command on endpoint 3 must complete. 5. DSTS must return a non-zero frame number when polled. 6. The test must reach the finish call without hanging.",
        "Remarks": "The test relies on an external USB host agent for handshake synchronization. The first post-enumeration transfer on EP2 uses TRB control 0x815 (interrupt transfer type), while the second transfer on EP3 uses TRB control 0x813. The interrupt handler uses && instead of & for the sysreg interrupt check."
    },
    {
        "Index": "3",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Isochronous_Transfer_test",
        "Feature": "Full-Speed Device Isochronous Transfer",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
        "Meta Macros": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; Buffer_PointerLO_1; DWORD; Default_Event_Ring_Array",
        "Meta Arrays": "buf_data[16]",
        "Speed": "Full-Speed",
        "Mode": "Device Mode",
        "Memory Start Offset": "0x1100",
        "Memory End Offset": "0x3000",
        "Meta Test Description": "This testcase validates USB Full-Speed (FS) Device mode Isochronous Transfer operation on the DWC USB3 controller. After enumeration, DSTS is polled for a valid frame number. Isochronous OUT transfers are initiated on endpoints 6 and 7 with TRB control 0x869 (isochronous type), size 0x3ff, and Start Transfer commands with frame numbers (0x20506 and 0x40506). DSTS is polled for specific frame number values (2, 3, 4) using mask 0x00000038.",
        "Test Description": "This test validates USB Full-Speed Device mode Isochronous Transfer functionality. The test performs a controller soft reset and waits for completion, then configures the USB2 PHY, event buffer ring, global controller in device mode, device configuration, device event enables, and endpoint parameters including isochronous endpoint types. After enumeration, DSTS is polled to wait for a valid frame number. An isochronous OUT transfer is initiated on endpoint 6 using a dedicated data buffer with an isochronous TRB type and a transfer size of 1023 bytes. The test then polls DSTS to wait for specific frame number values. A second isochronous OUT transfer is initiated on endpoint 7.",
        "Meta Test Steps / Procedure": "1. Initialize NIC and GIC. 2. Clear memory regions. 3. Soft reset. 4. Configure PHY, event buffer, GCTL, DCFG, DEVTEN, GUCTL. 5. Configure 8 endpoints including isochronous types for EP6 and EP7. 6. Allocate transfer resources. 7. Enable endpoints, set run/stop, enable interrupts. 8. Handle link state events. 9. Perform enumeration. 10. Poll DSTS for valid frame number. 11. Initiate isochronous OUT transfer on EP6 (TRB control=0x869, size=0x3ff, cmd=0x20506). 12. Poll DSTS for frame number 2. 13. Poll DSTS for frame number 3. 14. Initiate isochronous OUT transfer on EP7 (cmd=0x40506). 15. Poll DSTS for frame number 4. 16. Call finish(0).",
        "Test Steps / Procedure": "1. Initialize the NIC and enable all GIC IRQs. 2. Clear the data buffer and event TRB memory regions. 3. Perform a controller soft reset. 4. Configure USB2 PHY, event buffer ring, GCTL, DCFG, DEVTEN, GUCTL. 5. Configure 8 endpoints including isochronous types for endpoints 6 and 7. 6. Allocate transfer resources. 7. Enable endpoints, set run/stop, enable system interrupt. 8. Handle link state events. 9. Perform full USB enumeration. 10. Poll DSTS for valid non-zero frame number. 11. Initiate isochronous OUT transfer on endpoint 6 with 1023-byte size. 12. Poll DSTS for frame number value 2. 13. Poll DSTS for frame number value 3. 14. Initiate second isochronous OUT transfer on endpoint 7. 15. Poll DSTS for frame number value 4. 16. Call finish.",
        "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; Buffer_PointerLO_1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Impacted Registers": "DCTL; GCTL; DCFG; DEVTEN; GUCTL; DALEPENA; DSTS",
        "Meta Validation / Acceptance Criteria": "1. Soft reset must complete. 2. All endpoint commands must complete. 3. DSTS must return non-zero frame number after enumeration. 4. Isochronous OUT transfer on EP6 must complete. 5. DSTS must indicate frame number values 2, 3, and 4 when polled. 6. Isochronous OUT transfer on EP7 must complete. 7. Test must reach finish(0).",
        "Validation / Acceptance Criteria": "1. Controller soft reset must complete. 2. All endpoint configuration commands must complete successfully. 3. DSTS must return a non-zero frame number after enumeration. 4. Isochronous OUT transfer command on endpoint 6 must complete. 5. DSTS must indicate frame number value 2 after the first isochronous transfer. 6. DSTS must indicate frame number values 3 and 4 subsequently. 7. Isochronous OUT transfer command on endpoint 7 must complete. 8. The test must reach the finish call without hanging.",
        "Remarks": "The isochronous transfers use TRB control value 0x869 (isochronous type) and transfer size 0x3ff (1023 bytes). The Start Transfer commands include frame number fields (0x20506 and 0x40506) for isochronous scheduling. The test performs frame-level timing validation by polling DSTS for specific frame number values using mask 0x00000038 with a right shift of 3."
    },
    {
        "Index": "4",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_fs",
        "Feature": "Host Enumeration Full-Speed",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
        "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; USB_PORTSC_20_WCE; USB_PORTSC_20_WDE; USB_PORTSC_20_WOE; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; Default_Event_Ring_Array; DWORD; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; SCRATCHPAD0; SCRATCHPAD1; Device_Context_Base_Address_Array; Device_Context_Array; MIZAR_USB_CRCR_LO; Default_Command_Ring; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; MIZAR_USB_DB; Default_Input_Context; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Arrays": "data_in[512]; data_out[512]",
        "Speed": "Full-Speed",
        "Mode": "Host Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates USB Host mode Full-Speed device enumeration on the DWC USB3 xHCI controller. The test configures GCTL for host mode (0x30c11234), GFLADJ (0xa87f000), GUCTL (0x2000010), USB3 pipe control and USB2 PHY. xHCI data structures are initialized including event ring, scratchpad, device context, command ring. The controller is started via USBCMD. After port status change and port reset, set_address() issues Enable Slot, Address Device commands. enumeration() issues GET_DEVICE_DESCRIPTOR, GET_CONFIGURATION_DESCRIPTOR, SET_CONFIGURATION, and GET_FULL_CONFIGURATION_DESCRIPTOR.",
        "Test Description": "This test validates USB Host mode Full-Speed device enumeration using the xHCI controller. The test configures the global controller GCTL for host mode, adjusts frame length via GFLADJ, and sets user control via GUCTL. Host capability parameters are read. xHCI data structures are initialized. The controller is started via USBCMD. The test waits for port status change, issues port reset, performs set address sequence with Enable Slot and Address Device commands, Configure Endpoint, and enumeration with standard USB requests.",
        "Meta Test Steps / Procedure": "1. Initialize NIC and GIC. 2. Configure GCTL (0x30c11234), GFLADJ (0xa87f000), GUCTL (0x2000010). 3. Configure USB3 pipe control and USB2 PHY. 4. Read capability registers. 5. Configure PORTSC_20 with wake enables. 6. Initialize xHCI data structures. 7. Write CRCR_LO, CONFIG, DCBAAP, ERSTSZ, ERDP, ERSTBA, IMOD, IMAN. 8. Write USBCMD (0x5). 9. Enable system interrupt. 10. Wait for port status change. 11. Issue port reset. 12. Issue Enable Slot, Address Device commands. 13. Issue Configure Endpoint. 14. Issue enumeration requests. 15. Poll event ring for completion. 16. Call finish(0).",
        "Test Steps / Procedure": "1. Initialize the NIC and enable all GIC IRQs. 2. Configure GCTL for host mode. 3. Configure GFLADJ and GUCTL. 4. Configure USB3 pipe control and USB2 PHY. 5. Read host capability parameters. 6. Configure PORTSC_20 with wake enables. 7. Initialize xHCI data structures. 8. Start controller via USBCMD. 9. Enable system interrupt. 10. Wait for port status change. 11. Issue port reset. 12. Perform set address sequence. 13. Issue Configure Endpoint. 14. Execute enumeration requests. 15. Poll event ring for completion. 16. Call finish.",
        "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Event_Ring_Array; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Meta Validation / Acceptance Criteria": "1. Port status change interrupt must fire. 2. PORTSC_20 must indicate device connected. 3. Port reset must complete. 4. Enable Slot command must complete. 5. Address Device commands must complete. 6. Configure Endpoint must complete. 7. All enumeration requests must complete. 8. Test must reach finish(0).",
        "Validation / Acceptance Criteria": "1. The port status change interrupt must fire after the controller is started. 2. PORTSC_20 must indicate a device is connected. 3. Port reset must complete successfully. 4. The Enable Slot command must complete. 5. Address Device commands must complete. 6. Configure Endpoint command must complete. 7. All USB standard requests must complete. 8. The test must reach the finish call without hanging.",
        "Remarks": "The test operates in xHCI Host mode. The interrupt handler uses && instead of & for the sysreg interrupt check. USB3 pipe control, USB2 PHY, and slot 1 doorbell are accessed via inline base+offset expressions. Several register mappings are unresolved for RAM-relative buffer addresses."
    },
    {
        "Index": "5",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_hs",
        "Feature": "Host Enumeration High-Speed",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
        "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; USB_PORTSC_20_WCE; USB_PORTSC_20_WDE; USB_PORTSC_20_WOE; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; Default_Event_Ring_Array; DWORD; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; SCRATCHPAD0; SCRATCHPAD1; Device_Context_Base_Address_Array; Device_Context_Array; MIZAR_USB_CRCR_LO; Default_Command_Ring; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; MIZAR_USB_DB; Default_Input_Context; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Arrays": "data_in[512]; data_out[512]",
        "Speed": "High-Speed",
        "Mode": "Host Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates USB Host mode High-Speed device enumeration on the DWC USB3 xHCI controller. Uses GFLADJ value 0xa07f000 for High-Speed frame length adjustment. Includes GET_QUALIFIER request and second GET_CONFIGURATION_DESCRIPTOR. Device context base address array includes a third entry. Configure Endpoint slot context uses 0x38300000. Event ring completion polling at Default_Event_Ring_Array+0x150.",
        "Test Description": "This test validates USB Host mode High-Speed device enumeration using the xHCI controller. The test configures the global controller GCTL for host mode, adjusts frame length via GFLADJ, and sets user control via GUCTL. The enumeration sequence includes Get Device Descriptor, Get Device Qualifier, Get Configuration Descriptor, Set Configuration, a second Get Configuration Descriptor, and Get Full Configuration Descriptor.",
        "Meta Test Steps / Procedure": "1. Initialize NIC and GIC. 2. Configure GCTL (0x30c11234), GFLADJ (0xa07f000), GUCTL (0x2000010). 3. Configure USB3 pipe control and USB2 PHY. 4. Read capability registers. 5. Configure PORTSC_20. 6. Initialize xHCI data structures with 3 device context entries. 7. Start controller. 8. Wait for port status change and port reset. 9. Perform set address with Enable Slot and Address Device commands. 10. Issue Configure Endpoint (slot context 0x38300000). 11. Issue 6 enumeration requests including GET_QUALIFIER. 12. Poll Default_Event_Ring_Array+0x150 for completion. 13. Call finish(0).",
        "Test Steps / Procedure": "1. Initialize the NIC and enable all GIC IRQs. 2. Configure GCTL for host mode. 3. Configure GFLADJ and GUCTL. 4. Configure USB3 pipe control and USB2 PHY. 5. Read host capability parameters. 6. Configure PORTSC_20. 7. Initialize xHCI data structures with three device context entries. 8. Start controller. 9. Wait for port status change and port reset. 10. Perform set address sequence. 11. Issue Configure Endpoint. 12. Execute 6 enumeration requests including Get Device Qualifier. 13. Poll event ring for completion. 14. Call finish.",
        "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Event_Ring_Array; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Meta Validation / Acceptance Criteria": "1. Port status change interrupt must fire. 2. Port reset must complete. 3. Enable Slot and Address Device commands must complete. 4. Configure Endpoint must complete. 5. All 6 enumeration requests must complete. 6. Test must reach finish(0).",
        "Validation / Acceptance Criteria": "1. The port status change interrupt must fire. 2. Port reset must complete. 3. Enable Slot and Address Device commands must complete. 4. Configure Endpoint must complete. 5. All six USB standard requests must complete. 6. The test must reach the finish call without hanging.",
        "Remarks": "This test uses GFLADJ value 0xa07f000 for High-Speed frame length adjustment (vs 0xa87f000 in FS). Includes GET_QUALIFIER and second GET_CONFIGURATION_DESCRIPTOR. Device context base address array includes a third entry. Configure Endpoint slot context uses 0x38300000. Event ring completion polling at +0x150."
    },
    {
        "Index": "6",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_ls",
        "Feature": "Host Enumeration Low-Speed",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
        "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; USB_PORTSC_20_WCE; USB_PORTSC_20_WDE; USB_PORTSC_20_WOE; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; Default_Event_Ring_Array; DWORD; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; SCRATCHPAD0; SCRATCHPAD1; Device_Context_Base_Address_Array; Device_Context_Array; MIZAR_USB_CRCR_LO; Default_Command_Ring; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; MIZAR_USB_DB; Default_Input_Context; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Meta Arrays": "data_in[512]; data_out[512]",
        "Speed": "Low-Speed",
        "Mode": "Host Mode",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates USB Host mode Low-Speed device enumeration on the DWC USB3 xHCI controller. Uses GFLADJ value 0xa87f000 and slot context 0x08200000 for Low-Speed. GET_QUALIFIER is commented out. Configure Endpoint and interrupt endpoint data transfer sections are commented out. Get Full Configuration Descriptor uses size 0x18 (24 bytes). Event ring completion polling at Default_Event_Ring_Array+0x110.",
        "Test Description": "This test validates USB Host mode Low-Speed device enumeration using the xHCI controller. The test configures the global controller GCTL for host mode, adjusts frame length via GFLADJ, and sets user control via GUCTL. The enumeration sequence issues Get Device Descriptor, Get Configuration Descriptor, Set Configuration, a second Get Configuration Descriptor, and Get Full Configuration Descriptor.",
        "Meta Test Steps / Procedure": "1. Initialize NIC and GIC. 2. Configure GCTL (0x30c11234), GFLADJ (0xa87f000), GUCTL (0x2000010). 3. Configure USB3 pipe control and USB2 PHY. 4. Read capability registers. 5. Configure PORTSC_20. 6. Initialize xHCI data structures with event ring size 0x30. 7. Start controller. 8. Wait for port status change and port reset. 9. Issue Enable Slot command. 10. Perform Address Device commands (slot context 0x08200000, EP0 context 0x00080020). 11. Issue 5 enumeration requests (GET_FULL_CONFIGURATION_DESCRIPTOR size 0x18). 12. Poll Default_Event_Ring_Array+0x110 for completion. 13. Call finish(0).",
        "Test Steps / Procedure": "1. Initialize the NIC and enable all GIC IRQs. 2. Configure GCTL for host mode. 3. Configure GFLADJ and GUCTL. 4. Configure USB3 pipe control and USB2 PHY. 5. Read host capability parameters. 6. Configure PORTSC_20. 7. Initialize xHCI data structures. 8. Start controller. 9. Wait for port status change and port reset. 10. Issue Enable Slot command. 11. Perform Address Device commands. 12. Execute 5 enumeration requests. 13. Poll event ring for completion. 14. Call finish.",
        "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Command_Ring; MIZAR_USB_DB; Default_Input_Context; Default_Event_Ring_Array; EP0_TR_Dequeue_Pointer; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
        "Impacted Registers": "GCTL; GFLADJ; GUCTL; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
        "Meta Validation / Acceptance Criteria": "1. Port status change interrupt must fire. 2. Port reset must complete. 3. Enable Slot and Address Device commands must complete. 4. All 5 enumeration requests must complete. 5. Test must reach finish(0).",
        "Validation / Acceptance Criteria": "1. The port status change interrupt must fire. 2. Port reset must complete. 3. Enable Slot and Address Device commands must complete. 4. All five USB standard requests must complete. 5. The test must reach the finish call without hanging.",
        "Remarks": "The Configure Endpoint command and interrupt endpoint data transfer sections are entirely commented out. GET_QUALIFIER is commented out consistent with Low-Speed devices. Get Full Configuration Descriptor uses size 0x18 (24 bytes). Event ring completion polling at +0x110. The interrupt handler uses && instead of & for the sysreg interrupt check."
    }
]

# TestPlan sheet columns
tp_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# MetaData sheet columns
md_columns = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Header formatting
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_alignment = Alignment(vertical='top', wrap_text=True)
thin_border = Border(
    left=Side(style='thin'), right=Side(style='thin'),
    top=Side(style='thin'), bottom=Side(style='thin')
)

# Write TestPlan headers
for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(tp_columns, 1):
        value = row_data.get(col_name, "")
        if value is None:
            value = ""
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=str(value))
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_tp.freeze_panes = 'A2'

# Auto-size columns
for col_idx, col_name in enumerate(tp_columns, 1):
    max_length = len(col_name)
    for row in range(2, len(json_data) + 2):
        cell_value = ws_tp.cell(row=row, column=col_idx).value
        if cell_value:
            max_length = max(max_length, min(len(str(cell_value)), 80))
    adjusted_width = min(max(max_length + 2, 12), 60)
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

# --- MetaData Sheet ---
ws_md = wb.create_sheet(title="MetaData")

# Write MetaData headers
for col_idx, col_name in enumerate(md_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(md_columns, 1):
        value = row_data.get(col_name, "")
        if value is None:
            value = ""
        cell = ws_md.cell(row=row_idx, column=col_idx, value=str(value))
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_md.freeze_panes = 'A2'

# Auto-size MetaData columns
for col_idx, col_name in enumerate(md_columns, 1):
    max_length = len(col_name)
    for row in range(2, len(json_data) + 2):
        cell_value = ws_md.cell(row=row, column=col_idx).value
        if cell_value:
            max_length = max(max_length, min(len(str(cell_value)), 80))
    adjusted_width = min(max(max_length + 2, 12), 60)
    ws_md.column_dimensions[get_column_letter(col_idx)].width = adjusted_width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save workbook
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
wb.save(output_path)

# Validate
try:
    wb_check = load_workbook(output_path)
    assert 'TestPlan' in wb_check.sheetnames
    assert 'MetaData' in wb_check.sheetnames
    assert wb_check['MetaData'].sheet_state == 'veryHidden'
    tp_rows = wb_check['TestPlan'].max_row - 1
    md_rows = wb_check['MetaData'].max_row - 1
    file_size = os.path.getsize(output_path)
    print(f"SUCCESS: Generated {filename}")
    print(f"  Path: {output_path}")
    print(f"  Size: {file_size} bytes")
    print(f"  TestPlan rows: {tp_rows}")
    print(f"  MetaData rows: {md_rows}")
    print(f"  Validation: PASSED")
except Exception as e:
    print(f"VALIDATION FAILED: {e}")
