#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import json

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'USB_TestPlan_{timestamp}.xlsx'

script_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(script_dir, filename)

json_data = [
    {
        "Index": "1",
        "SS / Module": "USB",
        "Test Case Name": "usb_host_enumeration_hs",
        "Feature": "USB Host Enumeration (High Speed)",
        "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"",
        "Meta Macros": "MIZAR_USB_BASE, MIZAR_USB_HCSPARAMS1, MIZAR_USB_HCSPARAMS2, MIZAR_USB_SUPTPRT2_DW2, MIZAR_USB_SUPTPRT3_DW2, MIZAR_USB_PAGESIZE, MIZAR_USB_CONFIG, MIZAR_USB_USBCMD, MIZAR_USB_USBSTS, MIZAR_USB_DNCTRL, MIZAR_USB_CRCR_LO, MIZAR_USB_CRCR_HI, MIZAR_USB_DCBAAP_LO, MIZAR_USB_DCBAAP_HI, MIZAR_USB_PORTSC_20, MIZAR_USB_IMAN, MIZAR_USB_IMOD, MIZAR_USB_ERSTSZ, MIZAR_USB_ERSTBA_LO, MIZAR_USB_ERSTBA_HI, MIZAR_USB_ERDP_LO, MIZAR_USB_ERDP_HI, MIZAR_USB_DB, MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, DWORD, Device_Context_Base_Address_Array, Device_Context_Array, Default_Command_Ring, Default_Event_Ring_Array, Default_Input_Context, Event_Ring_Segment_Table, EP0_TR_Dequeue_Pointer",
        "Meta Arrays": "int data_in[512];\nint data_out[512];",
        "Speed": "High Speed",
        "Mode": "Host",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "USB host enumeration testcase at High Speed. The testcase initializes the xHCI host controller by configuring PHY control registers, reading structural parameters (HCSPARAMS1, HCSPARAMS2, SUPTPRT2_DW2, SUPTPRT3_DW2, PAGESIZE), setting up the Device Context Base Address Array (DCBAAP), Command Ring (CRCR), Event Ring Segment Table (ERSTBA, ERSTSZ, ERDP), Interrupter registers (IMAN, IMOD), and CONFIG register. It enables interrupts at the system register level (MIZAR_LSS_SYSREG_INTR_EN0), starts the host controller (USBCMD run), waits for port connect via interrupt-driven polling (int_pend/wait_on), performs port reset via PORTSC_20, issues Set Address command via Command Ring TRBs, and performs USB enumeration including GET_DESCRIPTOR (Device) via EP0 Transfer Ring TRBs (setup stage, data stage, status stage). The Default_IRQHandler clears interrupts by reading MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, writing MIZAR_USB_IMAN, clearing raw status, and calling GIC_ClearIRQ(84).",
        "Test Description": "USB Host Enumeration at High Speed. Initializes the xHCI host controller registers including structural parameters, device context base address array, command ring, event ring, interrupter management, and configuration. Enables system-level interrupts, starts the host controller, waits for device connection, performs port reset, issues Set Address command, and enumerates the USB device by sending GET_DESCRIPTOR requests through EP0 transfer ring TRBs.",
        "Meta Test Steps / Procedure": "extern int int_pend;\n\nint test_case() {\n    int rd_data, port_count, db_offset;\n    int event_completion, usb_status, port_status;\n    int input_context_address;\n    int data_in[512], j, data_out[512], i;\n    int count = 0;\n\n    nic_programming();\n    GIC_EnableAllIRQ();\n\n    // PHY CONTROL REG\n    rd_data = read_reg(MIZAR_USB_BASE + 0xc200);\n    write_reg(MIZAR_USB_BASE + 0xc200, 0x102407);\n\n    port_count = read_reg(MIZAR_USB_HCSPARAMS1);\n    rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2);\n    rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2);\n    rd_data = read_reg(MIZAR_USB_PORTSC_20);\n    rd_data = read_reg(MIZAR_USB_HCSPARAMS2);\n    rd_data = read_reg(MIZAR_USB_PAGESIZE);\n\n    // Event Ring Segment Table setup\n    write_reg(Event_Ring_Segment_Table, Default_Event_Ring_Array);\n    write_reg(Event_Ring_Segment_Table + DWORD, 0x0);\n    write_reg(Event_Ring_Segment_Table + 2*(DWORD), 0x20);\n\n    // Device Context Base Address Array setup\n    write_reg(Device_Context_Base_Address_Array + 2DWORD, Device_Context_Array + 0x100);\n    write_reg(Device_Context_Base_Address_Array + 3DWORD, 0x0);\n    write_reg(Device_Context_Base_Address_Array + 4DWORD, Device_Context_Array + 0x0d00);\n    write_reg(Device_Context_Base_Address_Array + 5DWORD, 0x0);\n\n    rd_data = read_reg(MIZAR_USB_CONFIG);\n    write_reg(MIZAR_USB_CONFIG, 0x110);\n\n    write_reg(MIZAR_USB_DCBAAP_LO, Device_Context_Base_Address_Array);\n    write_reg(MIZAR_USB_DCBAAP_HI, 0x0);\n    write_reg(MIZAR_USB_ERSTSZ, 0x1);\n\n    write_reg(MIZAR_USB_CRCR_LO, Default_Command_Ring + 0x1);\n    write_reg(MIZAR_USB_CRCR_HI, 0x0);\n    write_reg(MIZAR_USB_CONFIG, 0x10);\n\n    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array);\n    write_reg(MIZAR_USB_ERDP_HI, 0x0);\n    write_reg(MIZAR_USB_ERSTBA_LO, Event_Ring_Segment_Table);\n    write_reg(MIZAR_USB_ERSTBA_HI, 0x0);\n    write_reg(MIZAR_USB_IMOD, 0x0);\n    write_reg(MIZAR_USB_IMAN, 0x2);\n    write_reg(MIZAR_USB_USBCMD, 0x4);\n\n    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);\n    write_reg(MIZAR_USB_USBCMD, 0x5);\n\n    int_pend = 1;\n    while(int_pend) { wait_on(100); }\n\n    usb_status = read_reg(MIZAR_USB_USBSTS);\n    write_reg(MIZAR_USB_USBSTS, 0x8);\n    write_reg(MIZAR_USB_IMAN, 0x2);\n    write_reg(MIZAR_USB_ERDP_HI, 0x0);\n    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x18);\n\n    write_reg(MIZAR_USB_PORTSC_20, 0xe0006f1);\n    write_reg(MIZAR_USB_USBSTS, 0x8);\n    write_reg(MIZAR_USB_IMAN, 0x2);\n    write_reg(MIZAR_USB_PORTSC_20, 0xe220200);\n    port_status = read_reg(MIZAR_USB_PORTSC_20);\n\n    int_pend = 1;\n    while(int_pend) { wait_on(100); }\n\n    set_address();\n\n    input_context_address = (Default_Input_Context) + 0x0;\n\n    // EP command\n    write_reg(Default_Command_Ring + 0x30, input_context_address);\n    write_reg(Default_Command_Ring + 0x34, 0x0);\n    write_reg(Default_Command_Ring + 0x38, 0x0);\n    write_reg(Default_Command_Ring + 0x3c, 0x1003001);\n\n    write_reg(MIZAR_USB_USBSTS, 0x8);\n    write_reg(MIZAR_USB_IMAN, 0x2);\n    write_reg(MIZAR_USB_ERDP_HI, 0x0);\n    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x58);\n    write_reg(MIZAR_USB_DB, 0x0);\n\n    enumeration();\n}\n\nvoid Default_IRQHandler() {\n    int rd_data, sysreg_rd_data;\n    int_pend = 0;\n    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);\n    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);\n    if (rd_data && 0x80000000) {\n        write_reg(MIZAR_USB_IMAN, 0x1);\n        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);\n    }\n    GIC_ClearIRQ(84);\n}\n\nvoid set_address() {\n    int input_context_address;\n    int event_completion, usb_status;\n\n    // slot command TRB\n    write_reg(Default_Command_Ring + 0x0, 0x0);\n    write_reg(Default_Command_Ring + 0x4, 0x0);\n    write_reg(Default_Command_Ring + 0x8, 0x0);\n    write_reg(Default_Command_Ring + 0xc, 0x00002401);\n\n    usb_status = read_reg(MIZAR_USB_USBSTS);\n    write_reg(MIZAR_USB_USBSTS, 0x8);\n    write_reg(MIZAR_USB_IMAN, 0x2);\n    write_reg(MIZAR_USB_ERDP_HI, 0x0);\n}\n\nvoid enumeration() {\n    int event_completion;\n\n    // GET configuration get device\n    write_reg(EP0_TR_Dequeue_Pointer, 0x01000680);\n    write_reg(EP0_TR_Dequeue_Pointer + 0x4, 0x00120000);\n    write_reg(EP0_TR_Dequeue_Pointer + 0x8, 0x08);\n    write_reg(EP0_TR_Dequeue_Pointer + 0xc, 0x00030861);\n\n    // DATA\n    write_reg(EP0_TR_Dequeue_Pointer + 0x10, EP0_TR_Dequeue_Pointer + 0x200);\n    write_reg(EP0_TR_Dequeue_Pointer + 0x18, 0x12);\n    write_reg(EP0_TR_Dequeue_Pointer + 0x1c, 0x00010c27);\n}",
        "Test Steps / Procedure": "1. Call nic_programming() for NIC initialization.\n2. Enable all IRQs via GIC_EnableAllIRQ().\n3. Read and configure PHY Control Register at MIZAR_USB_BASE + 0xc200 with value 0x102407.\n4. Read HCSPARAMS1 to obtain port count.\n5. Read SUPTPRT2_DW2 and SUPTPRT3_DW2 for supported protocol information.\n6. Read PORTSC_20 for initial port status.\n7. Read HCSPARAMS2 and PAGESIZE for host controller parameters.\n8. Set up Event Ring Segment Table with Default_Event_Ring_Array base, size 0x20.\n9. Populate Device Context Base Address Array with Device_Context_Array offsets (0x100, 0x0d00).\n10. Read and write CONFIG register with 0x110 (CIE enable).\n11. Write DCBAAP_LO with Device_Context_Base_Address_Array, DCBAAP_HI with 0x0.\n12. Write ERSTSZ with 0x1.\n13. Write CRCR_LO with Default_Command_Ring + 0x1 (Ring Cycle State), CRCR_HI with 0x0.\n14. Write CONFIG with 0x10 (MaxSlotsEn = 16).\n15. Write ERDP_LO with Default_Event_Ring_Array, ERDP_HI with 0x0.\n16. Write ERSTBA_LO with Event_Ring_Segment_Table, ERSTBA_HI with 0x0.\n17. Write IMOD with 0x0, IMAN with 0x2, USBCMD with 0x4 (interrupt enable).\n18. Enable system interrupt via MIZAR_LSS_SYSREG_INTR_EN0 with 0x80000000.\n19. Write USBCMD with 0x5 (Run/Stop = Run).\n20. Set int_pend = 1, poll with wait_on(100) until interrupt clears int_pend.\n21. Read USBSTS, clear status by writing 0x8 to USBSTS.\n22. Write IMAN with 0x2, ERDP_HI with 0x0, ERDP_LO with Default_Event_Ring_Array + 0x18.\n23. Write PORTSC_20 with 0xe0006f1 (port reset).\n24. Clear USBSTS (0x8), IMAN (0x2).\n25. Write PORTSC_20 with 0xe220200, read back PORTSC_20.\n26. Set int_pend = 1, poll with wait_on(100) until interrupt clears int_pend.\n27. Call set_address() which issues slot command TRB via Default_Command_Ring.\n28. Compute input_context_address from Default_Input_Context.\n29. Issue EP command TRB via Default_Command_Ring at offset 0x30.\n30. Clear USBSTS, IMAN, update ERDP, write DB (doorbell) with 0x0.\n31. Call enumeration() which issues GET_DESCRIPTOR (Device) TRBs via EP0_TR_Dequeue_Pointer (setup stage, data stage).\n32. Default_IRQHandler: clears int_pend, reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, writes IMAN with 0x1, clears raw status with 0x80000000, calls GIC_ClearIRQ(84).",
        "Meta Impacted Registers": "HCSPARAMS1, HCSPARAMS2, SUPTPRT2_DW2, SUPTPRT3_DW2, PAGESIZE, CONFIG, USBCMD, USBSTS, CRCR_LO, CRCR_HI, DCBAAP_LO, DCBAAP_HI, PORTSC_20, IMAN, IMOD, ERSTSZ, ERSTBA_LO, ERSTBA_HI, ERDP_LO, ERDP_HI, DB",
        "Impacted Registers": "HCSPARAMS1, HCSPARAMS2, SUPTPRT2_DW2, SUPTPRT3_DW2, PAGESIZE, CONFIG, USBCMD, USBSTS, CRCR_LO, CRCR_HI, DCBAAP_LO, DCBAAP_HI, PORTSC_20, IMAN, IMOD, ERSTSZ, ERSTBA_LO, ERSTBA_HI, ERDP_LO, ERDP_HI, DB",
        "Meta Validation / Acceptance Criteria": "1. After enabling USBCMD run (0x5) and waiting for interrupt (int_pend polling with wait_on(100)), read MIZAR_USB_USBSTS to verify USB status; clear status by writing 0x8.\n2. Read MIZAR_USB_PORTSC_20 after port reset (0xe0006f1) and port status update (0xe220200) to verify port connect status is high.\n3. In Default_IRQHandler: read MIZAR_LSS_SYSREG_RAW_STCR0 and check condition (rd_data && 0x80000000) to verify interrupt source; write MIZAR_USB_IMAN with 0x1 to acknowledge USB interrupt; write MIZAR_LSS_SYSREG_RAW_STCR0 with 0x80000000 to clear raw interrupt status; call GIC_ClearIRQ(84).\n4. In set_address(): after issuing slot command TRB (0x00002401) to Default_Command_Ring, read MIZAR_USB_USBSTS to verify command completion; clear status by writing 0x8 to USBSTS.\n5. After EP command TRB (0x1003001) written to Default_Command_Ring + 0x30, doorbell register (MIZAR_USB_DB) is written with 0x0 to trigger command processing.\n6. In enumeration(): GET_DESCRIPTOR (Device) setup stage TRB value 0x01000680 with length 0x12 (18 bytes) is issued via EP0_TR_Dequeue_Pointer; data stage TRB points to EP0_TR_Dequeue_Pointer + 0x200 with transfer length 0x12.",
        "Validation / Acceptance Criteria": "1. After starting the host controller (USBCMD = Run), verify USB status via USBSTS read and clear event interrupt.\n2. After port reset via PORTSC_20, verify port connect status is high by reading PORTSC_20.\n3. Interrupt handler verifies interrupt source via system register reads, acknowledges USB interrupt via IMAN, clears raw interrupt status, and clears GIC IRQ 84.\n4. After Set Address command TRB is issued on the Command Ring, verify command completion via USBSTS read.\n5. After EP command TRB is issued, doorbell register (DB) is rung to trigger command processing.\n6. GET_DESCRIPTOR (Device) request is issued via EP0 Transfer Ring with correct setup and data stage TRBs for 18-byte device descriptor.",
        "Remarks": "The testcase uses interrupt-driven polling (int_pend flag set to 1, cleared by Default_IRQHandler, polled via wait_on). The Default_IRQHandler uses logical AND (&&) instead of bitwise AND (&) for checking bit 31 of rd_data, which may be a potential bug. The CONFIG register is written twice: first with 0x110 (CIE enable) and then with 0x10 (MaxSlotsEn = 16). The Event Ring Segment Table size is set to 0x20 (32 entries) in the segment table memory, while ERSTSZ register is set to 0x1 (1 segment). The source context is partially retrieved; some function bodies (enumeration status stage, complete set_address flow) are truncated in the available RAG source."
    },
    {
        "Index": "2",
        "SS / Module": "USB",
        "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
        "Feature": "USB Full-Speed Device Bulk Transfer",
        "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"",
        "Meta Macros": "MIZAR_USB_DCFG, MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, MIZAR_USB_DEPCMD, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, MIZAR_USB_GEVNTSIZ, MIZAR_USB_GEVNTCOUNT, MIZAR_USB_GCTL, Buffer_PointerLO, Default_Event_Ring_Array, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, DWORD, event_trb_addr, Buffer_PointerLO_1",
        "Meta Arrays": "int buf_data[16];",
        "Speed": "Full Speed",
        "Mode": "Device",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "USB Full-Speed Device Bulk Transfer test. The testcase initializes the USB device controller via nic_programming() (which configures global registers such as MIZAR_USB_GCTL with 0x30c11234 via set_data), enables all IRQs via GIC_EnableAllIRQ(), clears 20 entries of Buffer_PointerLO and event_trb_addr memory regions in a loop. The test proceeds through USB device enumeration including setup_stage(), status_stage(), and enumeration() functions which configure event TRBs and data buffers for device descriptor (0x02000012, length 0x12) and configuration descriptor (0x003c0209, length 0x3c) responses. The set_configuration() function is used to issue endpoint commands by writing MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, and MIZAR_USB_DEPCMD registers with polling for command completion. Bulk data transfer is performed using TRBs configured at Buffer_PointerLO with control field 0x853 and DEPCMD+0x10 with command 0x506, followed by interrupt-driven synchronization via int_pend polling with wait_on(5). The Default_IRQHandler reads MIZAR_USB_GEVNTCOUNT, stores it in event_counter, writes it back to acknowledge events, reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, and calls GIC_ClearIRQ(84).",
        "Test Description": "USB Full-Speed Device Bulk Transfer test. Initializes the USB device controller global registers, enables interrupts, clears buffer and event TRB memory. Performs USB device enumeration including device descriptor and configuration descriptor responses. Configures bulk endpoints using set_configuration with DEPCMD parameter registers. Executes bulk data transfers using TRBs with interrupt-driven synchronization. The interrupt handler reads GEVNTCOUNT for event acknowledgment and clears system-level interrupt status.",
        "Meta Test Steps / Procedure": "extern int int_pend;\nint event_counter;\n\nint test_case() {\n    int rd_data,wr_data,port_count,db_offset;\n    int buf_data[16],i,bulk,intr,event_comletion,j;\n    int hand_shake;\n\n    nic_programming();\n    GIC_EnableAllIRQ();\n    for(j=0;j< 20;j++){\n        write_reg(Buffer_PointerLO + jDWORD, 0x0);\n        write_reg(event_trb_addr + jDWORD, 0x0);\n    }\n\n    // ... controller initialization via nic_programming() which includes:\n    // write_reg(MIZAR_USB_GCTL,set_data(read_reg(MIZAR_USB_GCTL),0xFFFFFFFF,0x30c11234));\n\n    // DEPCMD+0x10 Start Transfer command\n    write_reg(MIZAR_USB_DEPCMD+0x10,0x506);\n    rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);\n    while(rd_data == 0x506){\n        wait_on(10);\n        rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);\n    }\n\n    int_pend = 1;\n    while(int_pend) { wait_on(5); }\n\n    wait_on(5);\n\n    int_pend = 1;\n    while(int_pend) { wait_on(5); }\n\n    // ADDED to check the xfernotready event\n    int_pend = 1;\n    while(int_pend) { wait_on(5); }\n\n    status_stage();\n\n    // USB_SET_CONFIGURATION_OR_RESET_TT\n    setup_stage();\n\n    write_reg(0xA0243ffc,0xdeadbee4);\n\n    // GET DESCRIPTOR USB CONFIGURATION\n    int_pend = 1;\n    while(int_pend) { wait_on(5); }\n\n    setup_stage();\n    write_reg(0xA0243ffc,0xdeadbee5);\n\n    int_pend = 1;\n    while(int_pend) { wait_on(5); }\n\n    // data stage - configuration descriptor\n    //09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301\n\n    // enumeration();\n\n    // Bulk data transfer TRB setup\n    write_reg(event_trb_addr,Buffer_PointerLO);\n    write_reg(event_trb_addr+0x8,0x0);\n    write_reg(event_trb_addr+0xc,0x853);\n    write_reg(Buffer_PointerLO,0x00);\n    write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);\n    write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);\n    write_reg(MIZAR_USB_DEPCMD+0x10,0x506);\n    rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);\n    while(rd_data == 0x506){\n        wait_on(10);\n        rd_data = read_reg(MIZAR_USB_DEPCMD+0x10);\n    }\n\n    // Bulk endpoint TRB with Buffer_PointerLO_1\n    // write_reg(event_trb_addr,Buffer_PointerLO_1);\n    // write_reg(event_trb_addr+0x8,0x40);\n    // write_reg(event_trb_addr+0xc,0x813);\n    // write_reg(MIZAR_USB_DEPCMDPAR1+0x40,event_trb_addr);\n    // write_reg(MIZAR_USB_DEPCMDPAR0+0x40,0x0);\n    // write_reg(MIZAR_USB_DEPCMD+0x40,0x506);\n\n    int_pend = 1;\n    while(int_pend) { wait_on(5); }\n}\n\nvoid Default_IRQHandler() {\n    int rd_data,sysreg_rd_data,event_count;\n    int_pend = 0;\n    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);\n    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);\n    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);\n    event_counter = event_count;\n    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);\n    if( rd_data && 0x80000000){\n        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,0x80000000);\n    }\n    GIC_ClearIRQ(84);\n}\n\nvoid setup_stage() {\n    int rd_data;\n    write_reg(event_trb_addr,Buffer_PointerLO);\n    write_reg(event_trb_addr+0x8,0x8);\n    write_reg(event_trb_addr+0xc,0x823);\n    write_reg(MIZAR_USB_DEPCMDPAR1,event_trb_addr);\n    write_reg(MIZAR_USB_DEPCMDPAR0,0x0);\n    write_reg(MIZAR_USB_DEPCMD,0x506);\n    rd_data = read_reg(MIZAR_USB_DEPCMD);\n    while(rd_data == 0x506){\n        wait_on(10);\n        rd_data = read_reg(MIZAR_USB_DEPCMD);\n    }\n    int_pend = 1;\n    while(int_pend) {\n        wait_on(5);\n    }\n}\n\nvoid status_stage() {\n    int rd_data;\n    write_reg(event_trb_addr,Buffer_PointerLO);\n    write_reg(event_trb_addr+0x8,0x0);\n    write_reg(event_trb_addr+0xc,0x843);\n    write_reg(MIZAR_USB_DEPCMDPAR1,event_trb_addr);\n    write_reg(MIZAR_USB_DEPCMDPAR0,0x0);\n    write_reg(MIZAR_USB_DEPCMD,0x506);\n    rd_data = read_reg(MIZAR_USB_DEPCMD);\n    while(rd_data == 0x506){\n        wait_on(10);\n        rd_data = read_reg(MIZAR_USB_DEPCMD);\n    }\n    int_pend = 1;\n    while(int_pend) {\n        wait_on(5);\n    }\n    wait_on(5);\n}\n\nvoid enumeration() {\n    // Device descriptor response\n    write_reg(event_trb_addr,Buffer_PointerLO);\n    write_reg(event_trb_addr+0x8,0x12);\n    write_reg(event_trb_addr+0xc,0x853);\n    //data\n    write_reg(Buffer_PointerLO,0x02000012);\n    write_reg(Buffer_PointerLO+0x4,0x40000000);\n    write_reg(Buffer_PointerLO+0x8,0x00000000);\n    write_reg(Buffer_PointerLO+0xc,0x00000000);\n    write_reg(Buffer_PointerLO+0x10,0x00000100);\n    write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);\n    write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);\n\n    // Configuration descriptor response\n    write_reg(event_trb_addr,Buffer_PointerLO);\n    write_reg(event_trb_addr+0x8,0x3c);\n    write_reg(event_trb_addr+0xc,0x853);\n    write_reg(Buffer_PointerLO,0x003c0209);\n    write_reg(Buffer_PointerLO+0x4,0xe0000101);\n    write_reg(Buffer_PointerLO+0x8,0x00000032);\n    write_reg(Buffer_PointerLO+0xc,0x00000000);\n    write_reg(MIZAR_USB_DEPCMDPAR1+0x10,event_trb_addr);\n    write_reg(MIZAR_USB_DEPCMDPAR0+0x10,0x0);\n}\n\nvoid set_configuration(int trb_address,int parameter0,int parameter1,int cmd) {\n    int read_data;\n    write_reg(MIZAR_USB_DEPCMDPAR1+trb_address,parameter1);\n    write_reg(MIZAR_USB_DEPCMDPAR0+trb_address,parameter0);\n    write_reg(MIZAR_USB_DEPCMD+trb_address,cmd);\n    read_data = read_reg(MIZAR_USB_DEPCMD+trb_address);\n    while(read_data == cmd){\n        wait_on(30);\n        read_data = read_reg(MIZAR_USB_DEPCMD+trb_address);\n    }\n}",
        "Test Steps / Procedure": "1. Call nic_programming() for USB device controller initialization (configures GCTL with 0x30c11234 and other global registers).\n2. Enable all IRQs via GIC_EnableAllIRQ().\n3. Clear 20 entries of Buffer_PointerLO and event_trb_addr memory regions by writing 0x0 in a loop.\n4. Issue Start Transfer command by writing 0x506 to DEPCMD+0x10, poll until command completes.\n5. Set int_pend = 1, poll with wait_on(5) until interrupt clears int_pend (repeated three times for xfernotready event handling).\n6. Call status_stage() to complete status phase of control transfer.\n7. Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT.\n8. Write debug marker 0xdeadbee4 to address 0xA0243ffc.\n9. Wait for interrupt, then call setup_stage() for GET DESCRIPTOR USB CONFIGURATION.\n10. Write debug marker 0xdeadbee5 to address 0xA0243ffc.\n11. Wait for interrupt, proceed with configuration descriptor data stage.\n12. Configure event TRB with Buffer_PointerLO, transfer length, and control field 0x853.\n13. Write device descriptor data (0x02000012, 0x40000000, etc.) to Buffer_PointerLO.\n14. Write configuration descriptor data (0x003c0209, 0xe0000101, 0x00000032) to Buffer_PointerLO.\n15. Set DEPCMDPAR1+0x10 to event_trb_addr, DEPCMDPAR0+0x10 to 0x0.\n16. Issue DEPCMD+0x10 with 0x506 for bulk endpoint Start Transfer, poll until complete.\n17. Wait for interrupt completion via int_pend polling with wait_on(5).\n18. Default_IRQHandler: clears int_pend, reads GEVNTCOUNT, stores in event_counter, writes back to acknowledge, reads system register status, clears raw interrupt status with 0x80000000, calls GIC_ClearIRQ(84).",
        "Meta Impacted Registers": "GCTL",
        "Impacted Registers": "GCTL",
        "Meta Validation / Acceptance Criteria": "1. After writing 0x506 to MIZAR_USB_DEPCMD+0x10, poll read_reg(MIZAR_USB_DEPCMD+0x10) until value changes from 0x506, indicating Start Transfer command completion.\n2. In setup_stage(): after writing 0x506 to MIZAR_USB_DEPCMD, poll read_reg(MIZAR_USB_DEPCMD) until value changes from 0x506, indicating endpoint command completion.\n3. In status_stage(): after writing 0x506 to MIZAR_USB_DEPCMD, poll read_reg(MIZAR_USB_DEPCMD) until value changes from 0x506, indicating endpoint command completion.\n4. In set_configuration(): after writing cmd to MIZAR_USB_DEPCMD+trb_address, poll read_reg(MIZAR_USB_DEPCMD+trb_address) until value changes from cmd, indicating command completion.\n5. In Default_IRQHandler(): read MIZAR_USB_GEVNTCOUNT to obtain event_count, store in event_counter global, write event_count back to MIZAR_USB_GEVNTCOUNT to acknowledge events; read MIZAR_LSS_SYSREG_RAW_STCR0 and check condition (rd_data && 0x80000000) to verify interrupt source; write MIZAR_LSS_SYSREG_RAW_STCR0 with 0x80000000 to clear raw interrupt status; call GIC_ClearIRQ(84).\n6. Interrupt-driven synchronization: int_pend set to 1 before each wait loop, cleared to 0 by Default_IRQHandler, polled via wait_on(5) in while loop.",
        "Validation / Acceptance Criteria": "1. DEPCMD Start Transfer command (0x506) completes successfully - verified by polling DEPCMD register until value changes.\n2. Endpoint commands in setup_stage and status_stage complete successfully via DEPCMD polling.\n3. set_configuration endpoint commands complete successfully via DEPCMD+trb_address polling.\n4. Interrupt handler correctly reads GEVNTCOUNT, acknowledges events by writing count back, and clears system-level interrupt status.\n5. All interrupt-driven synchronization points complete (int_pend cleared by IRQ handler).\n6. Device descriptor response (18 bytes) and configuration descriptor response (60 bytes) are correctly populated in Buffer_PointerLO memory.",
        "Remarks": "The testcase uses interrupt-driven polling with wait_on(5) which is shorter than other USB transfer tests (Isochronous/Interrupt use wait_on(100)), indicating tighter timing for bulk transfers. The Default_IRQHandler uses logical AND (&&) instead of bitwise AND (&) for checking bit 31 of rd_data, which may be a potential bug. The nic_programming() function body is not available in the retrieved source but is known to configure MIZAR_USB_GCTL with 0x30c11234 via set_data(). Some portions of the test_case() function body (particularly the complete bulk data transfer phase and set_configuration call parameters) are not fully available in the retrieved RAG source context. The MIZAR_USB_DCFG, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, and MIZAR_USB_GEVNTSIZ registers are referenced in Agent 4 mappings but their initialization is likely within nic_programming() which is not visible in the retrieved source. Only GCTL has a matched register_name from Agent 4; all other macros are unresolved."
    }
]

# TestPlan sheet columns
testplan_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# MetaData sheet columns
metadata_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

wb = openpyxl.Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = 'TestPlan'

header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

# Write headers
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, '')
        if value is None:
            value = ''
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')

for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, '')
        if value is None:
            value = ''
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row on both sheets
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# Auto-size columns with max width cap
max_col_width = 60
for ws in [ws_tp, ws_md]:
    for col in ws.columns:
        max_length = 0
        col_letter = col[0].column_letter
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    if len(line) > max_length:
                        max_length = len(line)
        adjusted_width = min(max_length + 4, max_col_width)
        if adjusted_width < 12:
            adjusted_width = 12
        ws.column_dimensions[col_letter].width = adjusted_width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save workbook
wb.save(output_path)
wb.close()

# Validate
import sys
try:
    vwb = openpyxl.load_workbook(output_path)
    sheets = vwb.sheetnames
    assert 'TestPlan' in sheets
    assert 'MetaData' in sheets
    tp_rows = vwb['TestPlan'].max_row - 1
    md_rows = vwb['MetaData'].max_row - 1
    fsize = os.path.getsize(output_path)
    vwb.close()
    print(f'VALIDATION=PASSED')
    print(f'FILE={output_path}')
    print(f'FILENAME={filename}')
    print(f'SIZE={fsize}')
    print(f'TP_ROWS={tp_rows}')
    print(f'MD_ROWS={md_rows}')
except Exception as e:
    print(f'VALIDATION=FAILED: {e}')
    sys.exit(1)
