import json, datetime, sys
try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
except ImportError:
    print('openpyxl not installed, writing CSV fallback')
    HAS_OPENPYXL = False
else:
    HAS_OPENPYXL = True

# ── IST timestamp ──
from datetime import timezone, timedelta
IST = timezone(timedelta(hours=5, minutes=30))
now = datetime.datetime.now(IST)
timestamp = now.strftime('%Y%m%d_%H%M%S')
filename = f'USB_TestPlan_{timestamp}.xlsx'

# ── All 6 testcases ──
testcases = [
  {
    "Index": "1",
    "SS / Module": "USB",
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Feature": "Full-Speed Device Bulk Transfer",
    "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
    "Meta Macros": "Buffer_PointerLO; Buffer_PointerLO_1; event_trb_addr; DWORD; Default_Event_Ring_Array; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
    "Meta Arrays": "buf_data[16]",
    "Speed": "Full-Speed (FS)",
    "Mode": "Device Mode",
    "Memory Start Offset": "0x1100",
    "Memory End Offset": "0x3000",
    "Meta Test Description": "This testcase validates USB Full-Speed (FS) Device mode Bulk Transfer operation on the DWC USB3 controller. The test performs a soft reset via DCTL, configures USB2 PHY via GUSB2PHYCFG, sets up the event buffer using GEVNTADRLO/HI/SIZ/COUNT, configures GCTL for device mode, configures DCFG/DEVTEN/GUCTL, performs endpoint configuration via DEPCMDPAR0/1 and DEPCMD with START NEW CONFIGURATION (0x409) and SET ENDPOINT CONFIGURATION (0x401) commands, allocates TX resources for 8 endpoints, enables endpoints via DALEPENA, starts the controller via DCTL, enables interrupts via MIZAR_LSS_SYSREG_INTR_EN0, performs USB enumeration (GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, SET CONFIGURATION), and initiates bulk OUT transfers on two bulk endpoints using TRB control 0x813.",
    "Test Description": "This test validates USB Full-Speed Device mode Bulk Transfer functionality. The test initializes the USB controller by performing a soft reset via DCTL and polling for completion. It configures the USB2 PHY via GUSB2PHYCFG, sets up the event buffer using GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, and GEVNTCOUNT, and configures the global controller register GCTL for device mode operation. Device configuration is applied through DCFG, device events are enabled via DEVTEN, and the user control register GUCTL is configured. Endpoints are configured using DEPCMDPAR0, DEPCMDPAR1, and DEPCMD with start new configuration and set endpoint configuration commands, followed by TX resource allocation for 8 endpoints. Physical endpoints are enabled via DALEPENA. The controller is started and system-level interrupts are enabled. The test then handles link state events, performs USB enumeration including GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, and SET CONFIGURATION control transfers through setup, data, and status stages using Transfer Request Blocks (TRBs). After enumeration, bulk OUT transfers are initiated on bulk endpoints. An interrupt handler services USB events by reading interrupt status, clearing event counts in GEVNTCOUNT, clearing raw interrupt status, and clearing the GIC IRQ. The test completes successfully after all bulk transfers finish.",
    "Meta Test Steps / Procedure": "1. Call nic_programming() and GIC_EnableAllIRQ(). 2. Clear 20 DWORD entries at Buffer_PointerLO and event_trb_addr. 3. Write 0x40f00000 to MIZAR_USB_DCTL for soft reset, poll until 0xf00000. 4. Write 0x40002407 to MIZAR_USB_GUSB2PHYCFG. 5. Write Default_Event_Ring_Array to GEVNTADRLO, 0x0 to GEVNTADRHI, 0x30 to GEVNTSIZ, 0x0 to GEVNTCOUNT. 6. Read-modify-write GCTL with 0x30c12214. 7. Write 0x480801 to DCFG, 0x1f to DEVTEN, read-modify-write GUCTL with 0xa400010. 8. Issue START NEW CONFIGURATION (0x409) and 8x SET ENDPOINT CONFIGURATION (0x401) via DEPCMD. 9. Allocate TX resources for 8 endpoints via DEPCMDPAR0/DEPCMD (0x402). 10. Write 0x3 to DALEPENA, 0x80f00000 to DCTL. 11. Write 0x80000000 to MIZAR_LSS_SYSREG_INTR_EN0. 12. Wait for link state events. 13. Perform enumeration: GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, SET CONFIGURATION. 14. Initiate bulk OUT transfers on endpoints +0x40 and +0x50 with TRB control 0x813. 15. finish(0).",
    "Test Steps / Procedure": "1. Initialize NIC and enable all GIC IRQs. 2. Clear data buffer and event TRB memory regions. 3. Perform soft reset via DCTL and poll for completion. 4. Configure USB2 PHY via GUSB2PHYCFG. 5. Set up event buffer via GEVNTADRLO/HI/SIZ/COUNT. 6. Configure GCTL for device mode. 7. Configure DCFG, DEVTEN, GUCTL. 8. Configure endpoints via DEPCMD commands. 9. Allocate TX resources. 10. Enable endpoints via DALEPENA and start controller. 11. Enable system interrupts. 12. Handle link state events and perform USB enumeration. 13. Initiate bulk OUT transfers. 14. Finish test.",
    "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; 0xa0243ff4; 0xa0243ff8; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
    "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
    "Meta Validation / Acceptance Criteria": "1. Soft reset completes: poll DCTL until 0xf00000. 2. All DEPCMD commands complete (poll until command active bit clears). 3. Interrupts received via int_pend flag. 4. GEVNTCOUNT read and cleared in IRQ handler. 5. Handshake registers polled until non-zero. 6. Bulk transfers complete on DEPCMD+0x40 and DEPCMD+0x50. 7. finish(0) reached.",
    "Validation / Acceptance Criteria": "1. Soft reset completes successfully. 2. All endpoint configuration commands complete. 3. USB enumeration completes (GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, SET CONFIGURATION). 4. Bulk OUT transfers complete on both endpoints. 5. Interrupt handler correctly clears events. 6. Test finishes successfully.",
    "Remarks": "Interrupt-driven flow with int_pend flag. GIC IRQ 84. Debug markers at 0xA0243ffc. RAM-based Buffer_PointerLO/event_trb_addr unresolved. Three MIZAR_LSS_SYSREG macros and three hex addresses unmapped to spec. IRQ handler uses && instead of & for bitmask check."
  },
  {
    "Index": "2",
    "SS / Module": "USB",
    "Test Case Name": "USB_FS_Device_Interrupt_Transfer_test",
    "Feature": "Full-Speed Device Interrupt Transfer",
    "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
    "Meta Macros": "Buffer_PointerLO; Buffer_PointerLO_1; event_trb_addr; DWORD; Default_Event_Ring_Array; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
    "Meta Arrays": "buf_data[16]",
    "Speed": "Full-Speed (FS)",
    "Mode": "Device Mode",
    "Memory Start Offset": "0x1100",
    "Memory End Offset": "0x3000",
    "Meta Test Description": "This testcase validates USB Full-Speed (FS) Device mode Interrupt Transfer operation. Similar initialization to bulk transfer test. After enumeration, interrupt IN transfers are initiated on endpoint +0x20 with TRB control 0x815, and interrupt OUT transfers on endpoint +0x30 with TRB control 0x813. DSTS is polled with mask 0x00000FF8 to wait for a non-zero frame number.",
    "Test Description": "This test validates USB Full-Speed Device mode Interrupt Transfer functionality. After standard USB controller initialization and enumeration, an interrupt IN transfer is initiated on an interrupt IN endpoint and an interrupt OUT transfer on an interrupt OUT endpoint. DSTS is polled to wait for a valid frame number. The test completes after all interrupt transfers finish and a valid frame number is detected.",
    "Meta Test Steps / Procedure": "1-12. Same as Bulk Transfer test initialization and enumeration. 13. Interrupt IN Transfer on endpoint +0x20: TRB control 0x815, Start Transfer (0x506). 14. Interrupt OUT Transfer on endpoint +0x30: TRB control 0x813, Start Transfer (0x506). 15. Poll MIZAR_USB_DSTS with mask 0x00000FF8 until frame number non-zero. 16. finish(0).",
    "Test Steps / Procedure": "1. Initialize USB controller (soft reset, PHY config, event buffer, GCTL, DCFG, DEVTEN, GUCTL). 2. Configure endpoints including interrupt types. 3. Start controller and enable interrupts. 4. Perform USB enumeration. 5. Initiate Interrupt IN and OUT transfers. 6. Poll DSTS for valid frame number. 7. Finish test.",
    "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; 0xa0243ff4; 0xa0243ff8; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
    "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
    "Meta Validation / Acceptance Criteria": "1. Soft reset completes. 2. All DEPCMD commands complete. 3. Interrupt IN transfer completes (DEPCMD+0x20, TRB 0x815). 4. Interrupt OUT transfer completes (DEPCMD+0x30, TRB 0x813). 5. DSTS frame number field non-zero (mask 0x00000FF8). 6. finish(0) reached.",
    "Validation / Acceptance Criteria": "1. Soft reset completes. 2. Endpoint configuration commands complete. 3. USB enumeration completes. 4. Interrupt IN and OUT transfers complete. 5. Valid frame number detected in DSTS. 6. Test finishes successfully.",
    "Remarks": "Interrupt IN uses TRB control 0x815, OUT uses 0x813. DSTS polled for frame number validation (mask 0x00000FF8). Same IRQ handler pattern as bulk test."
  },
  {
    "Index": "3",
    "SS / Module": "USB",
    "Test Case Name": "USB_FS_Device_Isochronous_Transfer_test",
    "Feature": "Full-Speed Device Isochronous Transfer",
    "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
    "Meta Macros": "Buffer_PointerLO; Buffer_PointerLO_1; event_trb_addr; DWORD; Default_Event_Ring_Array; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
    "Meta Arrays": "buf_data[16]",
    "Speed": "Full-Speed (FS)",
    "Mode": "Device Mode",
    "Memory Start Offset": "0x1100",
    "Memory End Offset": "0x3000",
    "Meta Test Description": "This testcase validates USB Full-Speed (FS) Device mode Isochronous Transfer operation. After enumeration, DSTS is polled for initial frame number. Two isochronous OUT transfers are initiated on endpoints +0x60 and +0x70 with TRB control 0x869 and size 0x3ff. Start Transfer commands embed frame numbers (0x20506 and 0x40506). DSTS is polled with mask 0x00000038 shifted right by 3 for specific micro-frame values (0x2, 0x3, 0x4).",
    "Test Description": "This test validates USB Full-Speed Device mode Isochronous Transfer functionality. After initialization and enumeration, DSTS is polled for a valid frame number. Two isochronous OUT transfers are initiated with specific frame numbers embedded in the Start Transfer commands. DSTS is polled for expected micro-frame number values between transfers. The test completes after both isochronous transfers finish and expected frame numbers are validated.",
    "Meta Test Steps / Procedure": "1-12. Same initialization and enumeration. 13. Poll DSTS with mask 0x00000FF8 for non-zero frame number. 14. Isochronous OUT Transfer 1 on endpoint +0x60: TRB control 0x869, size 0x3ff, Start Transfer 0x20506. 15. Poll DSTS for frame number field = 0x2. 16. Isochronous OUT Transfer 2 on endpoint +0x70: TRB control 0x869, Start Transfer 0x40506. 17. Poll DSTS for frame number field = 0x4. 18. finish(0).",
    "Test Steps / Procedure": "1. Initialize USB controller. 2. Configure endpoints including isochronous types. 3. Start controller and enable interrupts. 4. Perform USB enumeration. 5. Poll DSTS for valid frame number. 6. Initiate first isochronous OUT transfer with specific frame number. 7. Poll DSTS for expected micro-frame. 8. Initiate second isochronous OUT transfer. 9. Poll DSTS for expected micro-frame. 10. Finish test.",
    "Meta Impacted Registers": "Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; 0xa0243ff4; 0xa0243ff8; MIZAR_USB_DEPCMDPAR1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0",
    "Impacted Registers": "DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1",
    "Meta Validation / Acceptance Criteria": "1. Soft reset completes. 2. All DEPCMD commands complete. 3. DSTS frame number non-zero (mask 0x00000FF8). 4. Isochronous transfer 1 completes (DEPCMD+0x60, cmd 0x20506, TRB 0x869). 5. DSTS micro-frame = 0x2 (mask 0x00000038 >> 3). 6. Isochronous transfer 2 completes (DEPCMD+0x70, cmd 0x40506). 7. DSTS micro-frame = 0x4. 8. finish(0) reached.",
    "Validation / Acceptance Criteria": "1. Soft reset completes. 2. Endpoint configuration commands complete. 3. USB enumeration completes. 4. Valid frame number detected in DSTS. 5. Both isochronous OUT transfers complete with correct frame number scheduling. 6. Expected micro-frame numbers reached. 7. Test finishes successfully.",
    "Remarks": "Isochronous transfers use TRB control 0x869, size 0x3ff. Start Transfer commands embed frame numbers (0x20506, 0x40506). DSTS polled with multiple masks for frame number validation. Endpoint types include control, interrupt, bulk, and isochronous."
  },
  {
    "Index": "4",
    "SS / Module": "USB",
    "Test Case Name": "usb_host_enumeration_fs",
    "Feature": "Host Mode Full-Speed Enumeration",
    "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
    "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Event_Ring_Array; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; EP0_TR_Dequeue_Pointer",
    "Meta Arrays": "data_in[512]; data_out[512]",
    "Speed": "Full-Speed (FS)",
    "Mode": "Host Mode",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates USB Host mode Full-Speed enumeration on the DWC USB3 xHCI controller. The test configures GCTL (0x30c11234), GFLADJ (0xa87f000), GUCTL (0x2000010), pipe control (MIZAR_USB_BASE+0xc2c0), and PHY control (MIZAR_USB_BASE+0xc200). It reads HCSPARAMS1, SUPTPRT2_DW2, SUPTPRT3_DW2, PORTSC_20, DBOFF, HCSPARAMS2, PAGESIZE. It sets up Event Ring Segment Table, Scratchpad Buffer Array, Device Context Base Address Array, Command Ring (CRCR_LO/HI), CONFIG, DCBAAP_LO/HI, ERSTSZ, ERDP_LO/HI, ERSTBA_LO/HI, IMOD, IMAN, USBCMD. Enables sysreg interrupt. Performs port reset via PORTSC_20, slot enable, set address (BSR=1 then BSR=0), configure endpoint, and enumeration (GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR short/full, SET CONFIGURATION) via EP0 Transfer Ring and Doorbell register.",
    "Test Description": "This test validates USB Host mode Full-Speed enumeration. The xHCI controller is initialized by configuring global registers (GCTL, GFLADJ, GUCTL), pipe and PHY control registers, and reading capability registers (HCSPARAMS1/2, PAGESIZE). The xHCI data structures (Event Ring, Scratchpad, DCBAA, Command Ring) are set up in memory. The controller is started via USBCMD. After port connect detection, port reset is performed via PORTSC_20. The Enable Slot, Address Device (BSR=1 then BSR=0), and Configure Endpoint commands are issued via the Command Ring. USB enumeration is performed via EP0 Transfer Ring including GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, and SET CONFIGURATION. The Doorbell register triggers command/transfer execution.",
    "Meta Test Steps / Procedure": "1. Call nic_programming() and GIC_EnableAllIRQ(). 2. Read-modify-write GCTL with 0x30c11234, GFLADJ with 0xa87f000, GUCTL with 0x2000010. 3. Read/write MIZAR_USB_BASE+0xc2c0 (pipe control) and MIZAR_USB_BASE+0xc200 (PHY control). 4. Read HCSPARAMS1, SUPTPRT2_DW2, SUPTPRT3_DW2, PORTSC_20, DBOFF, HCSPARAMS2, PAGESIZE. 5. Set up Event_Ring_Segment_Table, Scratchpad_Buffer_Array, Device_Context_Base_Address_Array. 6. Write CRCR_LO/HI, CONFIG, DCBAAP_LO/HI, ERSTSZ, ERDP_LO/HI, ERSTBA_LO/HI, IMOD, IMAN. 7. Write USBCMD=0x4 then 0x5 (interrupt enable + run). 8. Write MIZAR_LSS_SYSREG_INTR_EN0=0x80000000. 9. Wait for port connect interrupt. 10. Read USBSTS, clear with 0x8, write IMAN=0x2. 11. Port reset: write PORTSC_20=0xe0006f1 then 0xe220200. 12. Enable Slot command via Default_Command_Ring, ring doorbell (MIZAR_USB_DB=0x0). 13. Address Device (BSR=1): write Default_Input_Context, Default_Command_Ring, ring doorbell. 14. Address Device (BSR=0): repeat with different command. 15. Configure Endpoint: write Default_Input_Context with EP config, Default_Command_Ring, ring doorbell. 16. Enumeration via EP0_TR_Dequeue_Pointer: GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short 0x9, full 0x3c), SET CONFIGURATION. 17. Ring doorbell MIZAR_USB_BASE+0x484=0x1. 18. Wait for completion events. 19. finish(0).",
    "Test Steps / Procedure": "1. Initialize NIC and GIC. 2. Configure global registers (GCTL, GFLADJ, GUCTL). 3. Configure pipe and PHY control registers. 4. Read capability registers. 5. Set up xHCI data structures (Event Ring, Scratchpad, DCBAA, Command Ring). 6. Configure operational registers (CONFIG, DCBAAP, ERSTSZ, ERDP, ERSTBA, IMOD, IMAN). 7. Start controller via USBCMD. 8. Enable system interrupts. 9. Wait for port connect. 10. Perform port reset. 11. Issue Enable Slot command. 12. Issue Address Device commands (BSR=1, BSR=0). 13. Issue Configure Endpoint command. 14. Perform USB enumeration (GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, SET CONFIGURATION). 15. Wait for all events to complete. 16. Finish test.",
    "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Event_Ring_Array; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; EP0_TR_Dequeue_Pointer",
    "Impacted Registers": "GCTL; GFLADJ; GUCTL; GUSB3PIPECTL; GUSB2PHYCFG; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
    "Meta Validation / Acceptance Criteria": "1. Port connect detected via interrupt. 2. Port reset completes (PORTSC_20 updated). 3. Enable Slot command completes (event ring read). 4. Address Device (BSR=1) completes. 5. Address Device (BSR=0) completes. 6. Configure Endpoint completes. 7. GET DEVICE DESCRIPTOR completes. 8. GET CONFIGURATION DESCRIPTOR (short and full) completes. 9. SET CONFIGURATION completes. 10. All event completions read from Default_Event_Ring_Array. 11. finish(0) reached.",
    "Validation / Acceptance Criteria": "1. Port connect detected. 2. Port reset completes. 3. Enable Slot, Address Device, Configure Endpoint commands complete. 4. USB enumeration completes (GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR, SET CONFIGURATION). 5. All transfer events complete. 6. Test finishes successfully.",
    "Remarks": "Host mode xHCI test. GFLADJ set to 0xa87f000 for FS timing. Slot context speed field set to 0x08100000 (FS). Max packet size 0x40 for FS. Uses Command Ring and EP0 Transfer Ring. Doorbell at MIZAR_USB_DB and MIZAR_USB_BASE+0x484. Three MIZAR_LSS_SYSREG macros unmapped. IRQ handler uses && instead of & for bitmask check."
  },
  {
    "Index": "5",
    "SS / Module": "USB",
    "Test Case Name": "usb_host_enumeration_hs",
    "Feature": "Host Mode High-Speed Enumeration",
    "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
    "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Event_Ring_Array; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; EP0_TR_Dequeue_Pointer",
    "Meta Arrays": "data_in[512]; data_out[512]",
    "Speed": "High-Speed (HS)",
    "Mode": "Host Mode",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates USB Host mode High-Speed enumeration on the DWC USB3 xHCI controller. Same structure as FS host enumeration but with GFLADJ=0xa07f000 for HS timing and slot context speed field 0x08300000/0x38300000 (HS). Enumeration includes GET DEVICE DESCRIPTOR, GET DEVICE QUALIFIER, GET CONFIGURATION DESCRIPTOR (short/full x2), and SET CONFIGURATION. Event completion polled at Default_Event_Ring_Array+0x150.",
    "Test Description": "This test validates USB Host mode High-Speed enumeration. The xHCI controller is initialized with HS-specific timing (GFLADJ). After port connect and reset, Enable Slot, Address Device, and Configure Endpoint commands are issued. USB enumeration includes GET DEVICE DESCRIPTOR, GET DEVICE QUALIFIER, GET CONFIGURATION DESCRIPTOR (short, full, and extended), and SET CONFIGURATION via EP0 Transfer Ring.",
    "Meta Test Steps / Procedure": "1-11. Same as FS host enumeration but GFLADJ=0xa07f000, slot context=0x08300000/0x38300000. 12-15. Same Enable Slot, Address Device, Configure Endpoint. 16. Enumeration: GET DEVICE DESCRIPTOR, GET DEVICE QUALIFIER, GET CONFIGURATION DESCRIPTOR (short 0x9), SET CONFIGURATION, GET CONFIGURATION DESCRIPTOR (full 0x9), GET CONFIGURATION DESCRIPTOR (full 0x3c). 17. Ring doorbell MIZAR_USB_BASE+0x484=0x1. 18. Poll Default_Event_Ring_Array+0x150 for completion. 19. finish(0).",
    "Test Steps / Procedure": "1. Initialize NIC and GIC. 2. Configure global registers with HS timing. 3. Configure pipe and PHY control. 4. Read capability registers. 5. Set up xHCI data structures. 6. Start controller. 7. Enable interrupts. 8. Wait for port connect and perform reset. 9. Issue Enable Slot, Address Device, Configure Endpoint. 10. Perform extended USB enumeration (including Device Qualifier). 11. Wait for all events. 12. Finish test.",
    "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Event_Ring_Array; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; EP0_TR_Dequeue_Pointer",
    "Impacted Registers": "GCTL; GFLADJ; GUCTL; GUSB3PIPECTL; GUSB2PHYCFG; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
    "Meta Validation / Acceptance Criteria": "1. Port connect detected. 2. Port reset completes. 3. Enable Slot completes. 4. Address Device (BSR=1 and BSR=0) completes. 5. Configure Endpoint completes. 6. GET DEVICE DESCRIPTOR completes. 7. GET DEVICE QUALIFIER completes. 8. GET CONFIGURATION DESCRIPTOR (short, full, extended) completes. 9. SET CONFIGURATION completes. 10. Event completion at Default_Event_Ring_Array+0x150 non-zero. 11. finish(0) reached.",
    "Validation / Acceptance Criteria": "1. Port connect and reset complete. 2. All xHCI commands complete. 3. Extended USB enumeration completes (including Device Qualifier). 4. All transfer events complete. 5. Test finishes successfully.",
    "Remarks": "HS mode test. GFLADJ=0xa07f000 for HS timing. Slot context speed=0x08300000 (HS). Includes GET DEVICE QUALIFIER not present in FS test. Extended enumeration with multiple GET CONFIGURATION DESCRIPTOR requests. Event completion polled at offset 0x150."
  },
  {
    "Index": "6",
    "SS / Module": "USB",
    "Test Case Name": "usb_host_enumeration_ls",
    "Feature": "Host Mode Low-Speed Enumeration",
    "Meta Headers": '<stdio.h>; <stdlib.h>; "usb.h"',
    "Meta Macros": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Event_Ring_Array; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; EP0_TR_Dequeue_Pointer",
    "Meta Arrays": "data_in[512]; data_out[512]",
    "Speed": "Low-Speed (LS)",
    "Mode": "Host Mode",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates USB Host mode Low-Speed enumeration on the DWC USB3 xHCI controller. Same structure as FS host enumeration but with GFLADJ=0xa87f000 for LS timing and slot context speed field 0x08200000 (LS). Max packet size 0x08 for LS (EP context 0x00080020). Event Ring Segment size=0x30. Enumeration includes GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short/full x2), and SET CONFIGURATION. No set_address() helper function; address commands are inline. Event completion polled at Default_Event_Ring_Array+0x110.",
    "Test Description": "This test validates USB Host mode Low-Speed enumeration. The xHCI controller is initialized with LS-specific parameters. After port connect and reset, Enable Slot, Address Device, and Configure Endpoint commands are issued inline (no set_address helper). USB enumeration includes GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short, full, and extended with size 0x18), and SET CONFIGURATION. Max packet size is 8 bytes for LS.",
    "Meta Test Steps / Procedure": "1-11. Same as FS host enumeration but GFLADJ=0xa87f000, Event Ring size=0x30, slot context=0x08200000, EP max packet size=0x00080020. 12. Enable Slot command inline (no set_address helper). 13. Address Device (BSR=1): Default_Input_Context with 0x08200000, EP context 0x00080020. 14. Address Device (BSR=0). 15. Enumeration: GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR (short 0x9), SET CONFIGURATION, GET CONFIGURATION DESCRIPTOR (full 0x8), GET CONFIGURATION DESCRIPTOR (full 0x18). 16. Ring doorbell MIZAR_USB_BASE+0x484=0x1. 17. Poll Default_Event_Ring_Array+0x110 for completion. 18. finish(0).",
    "Test Steps / Procedure": "1. Initialize NIC and GIC. 2. Configure global registers with LS parameters. 3. Configure pipe and PHY control. 4. Read capability registers. 5. Set up xHCI data structures (Event Ring size 0x30). 6. Start controller. 7. Enable interrupts. 8. Wait for port connect and perform reset. 9. Issue Enable Slot, Address Device (inline). 10. Perform USB enumeration with LS max packet size. 11. Wait for all events. 12. Finish test.",
    "Meta Impacted Registers": "MIZAR_USB_GCTL; MIZAR_USB_GFLADJ; MIZAR_USB_GUCTL; MIZAR_USB_BASE; MIZAR_USB_HCSPARAMS1; MIZAR_USB_SUPTPRT2_DW2; MIZAR_USB_SUPTPRT3_DW2; MIZAR_USB_PORTSC_20; MIZAR_USB_DBOFF; Event_Ring_Segment_Table; MIZAR_USB_HCSPARAMS2; MIZAR_USB_PAGESIZE; Scratchpad_Buffer_Array; Device_Context_Base_Address_Array; MIZAR_USB_CRCR_LO; MIZAR_USB_CRCR_HI; MIZAR_USB_CONFIG; MIZAR_USB_DCBAAP_LO; MIZAR_USB_DCBAAP_HI; MIZAR_USB_ERSTSZ; MIZAR_USB_ERDP_LO; MIZAR_USB_ERDP_HI; MIZAR_USB_ERSTBA_LO; MIZAR_USB_ERSTBA_HI; MIZAR_USB_IMOD; MIZAR_USB_IMAN; MIZAR_USB_USBCMD; MIZAR_LSS_SYSREG_INTR_EN0; MIZAR_USB_USBSTS; Default_Event_Ring_Array; Default_Input_Context; Default_Command_Ring; MIZAR_USB_DB; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0; EP0_TR_Dequeue_Pointer",
    "Impacted Registers": "GCTL; GFLADJ; GUCTL; GUSB3PIPECTL; GUSB2PHYCFG; HCSPARAMS1; SUPTPRT2_DW2; SUPTPRT3_DW2; PORTSC_20; DBOFF; HCSPARAMS2; PAGESIZE; CRCR_LO; CRCR_HI; CONFIG; DCBAAP_LO; DCBAAP_HI; ERSTSZ; ERDP_LO; ERDP_HI; ERSTBA_LO; ERSTBA_HI; IMOD; IMAN; USBCMD; USBSTS; DB",
    "Meta Validation / Acceptance Criteria": "1. Port connect detected. 2. Port reset completes. 3. Enable Slot completes. 4. Address Device (BSR=1 and BSR=0) completes. 5. GET DEVICE DESCRIPTOR completes. 6. GET CONFIGURATION DESCRIPTOR (short 0x9, full 0x8, full 0x18) completes. 7. SET CONFIGURATION completes. 8. Event completion at Default_Event_Ring_Array+0x110 non-zero. 9. finish(0) reached.",
    "Validation / Acceptance Criteria": "1. Port connect and reset complete. 2. All xHCI commands complete. 3. USB enumeration completes with LS max packet size. 4. All transfer events complete. 5. Test finishes successfully.",
    "Remarks": "LS mode test. GFLADJ=0xa87f000. Slot context speed=0x08200000 (LS). Max packet size 8 bytes (0x00080020). No set_address() helper - address commands inline. Event Ring Segment size=0x30. No Configure Endpoint command in active code (commented out). Extended GET CONFIGURATION DESCRIPTOR with size 0x18. Event completion polled at offset 0x110."
  }
]

columns = [
    'Index', 'SS / Module', 'Test Case Name', 'Feature',
    'Meta Headers', 'Meta Macros', 'Meta Arrays',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset',
    'Meta Test Description', 'Test Description',
    'Meta Test Steps / Procedure', 'Test Steps / Procedure',
    'Meta Impacted Registers', 'Impacted Registers',
    'Meta Validation / Acceptance Criteria', 'Validation / Acceptance Criteria',
    'Remarks'
]

if HAS_OPENPYXL:
    wb = Workbook()
    ws = wb.active
    ws.title = 'USB_TestPlan'

    header_font = Font(name='Calibri', bold=True, size=11, color='FFFFFF')
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
    cell_align = Alignment(vertical='top', wrap_text=True)
    thin_border = Border(
        left=Side(style='thin'),
        right=Side(style='thin'),
        top=Side(style='thin'),
        bottom=Side(style='thin')
    )

    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align
        cell.border = thin_border

    for row_idx, tc in enumerate(testcases, 2):
        for col_idx, col_name in enumerate(columns, 1):
            val = tc.get(col_name, '')
            cell = ws.cell(row=row_idx, column=col_idx, value=val)
            cell.alignment = cell_align
            cell.border = thin_border

    col_widths = {
        1: 8, 2: 14, 3: 40, 4: 35,
        5: 30, 6: 50, 7: 20,
        8: 20, 9: 15, 10: 20, 11: 20,
        12: 80, 13: 80,
        14: 80, 15: 80,
        16: 60, 17: 60,
        18: 60, 19: 60,
        20: 60
    }
    for col_idx, width in col_widths.items():
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = width

    ws.auto_filter.ref = ws.dimensions
    ws.freeze_panes = 'A2'

    wb.save(filename)
    print(f'Excel file saved: {filename}')
else:
    import csv
    csv_filename = filename.replace('.xlsx', '.csv')
    with open(csv_filename, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=columns)
        writer.writeheader()
        for tc in testcases:
            writer.writerow({col: tc.get(col, '') for col in columns})
    print(f'CSV file saved: {csv_filename}')
