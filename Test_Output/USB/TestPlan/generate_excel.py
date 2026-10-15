#!/usr/bin/env python3
"""
Agent 7 - Excel Workbook Generator
Generates USB_TestPlan_YYYYMMDD_HHMMSS.xlsx
Triggered by GitHub Actions workflow: generate_testplan.yml
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import sys
import os

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"

# ── TESTPLAN_JSON Data (2 records) ────────────────────────────────────
TESTPLAN = [
  {
    "Index": "1",
    "SS / Module": "USB",
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Feature": "USB Device Mode Full-Speed Bulk Transfer",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This testcase validates USB device-mode full-speed bulk transfer functionality. It initializes the USB controller in device mode, configures the event buffer and global control registers, programs the device configuration for full-speed operation, enables all active endpoints, and configures the USB 2.0 PHY. The test performs device-side enumeration by responding to setup requests with device descriptor and configuration descriptor data through Transfer Request Blocks (TRBs). It issues endpoint commands and polls for their completion, uses interrupt-driven synchronization for event handling, and executes setup, data, and status stages of control transfers followed by bulk transfer operations. The interrupt service routine handles event acknowledgment and system-level interrupt clearing.",
    "Test Steps / Procedure": "1. Perform NIC programming and enable all GIC IRQs.\n2. Initialize the data buffer and event TRB memory regions to zero.\n3. Configure the event buffer by programming the event address low, event address high, event size, and event count registers.\n4. Read the global control register to verify controller state.\n5. Read the device configuration and device status registers.\n6. Program the device configuration register for full-speed device mode operation.\n7. Program the device control register to set run/stop and other control bits.\n8. Enable all active endpoints via the active endpoint enable register.\n9. Wait for the controller to stabilize.\n10. Execute the setup stage by writing TRB data to the event TRB memory, programming endpoint command parameter registers, issuing a Start Transfer endpoint command, and polling for command completion.\n11. Configure the USB 2.0 PHY configuration register.\n12. Check the event counter and wait for additional interrupts if needed.\n13. Write device descriptor data to the data buffer memory region.\n14. Program TRB entries with descriptor data pointer, transfer length, and control fields.\n15. Issue endpoint commands for the IN endpoint and poll for completion.\n16. Wait for interrupt-driven synchronization after each transfer phase.\n17. Execute the status stage by writing a zero-length TRB and issuing a Start Transfer endpoint command.\n18. Execute additional setup stages for SET_CONFIGURATION and GET_CONFIGURATION_DESCRIPTOR requests.\n19. Write configuration descriptor data to the data buffer memory.\n20. Issue endpoint commands and poll for completion for each descriptor response.\n21. Verify debug markers are written at each major test phase.\n22. The ISR reads system-level interrupt status, reads and clears the raw status register, reads the event count, stores it globally, writes back the event count for acknowledgment, and clears the GIC IRQ.",
    "Impacted Registers": "GCTL, GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, GEVNTCOUNT, DCFG, DSTS, DCTL, DALEPENA, DEVTEN, GUCTL, GUSB2PHYCFG, DEPCMDPAR1, DEPCMDPAR0, DEPCMD",
    "Validation / Acceptance Criteria": "1. Endpoint command completion: After issuing a Start Transfer command to the endpoint command register, the command active bit must clear, indicating successful command completion.\n2. Endpoint command completion for additional endpoints: The command active bit on secondary endpoint command instances must also clear after command issuance.\n3. Generic endpoint command completion via helper function: All endpoint commands issued through the configuration helper must complete successfully with the command active bit transitioning from active to inactive.\n4. Interrupt-driven synchronization: The interrupt pending flag must be cleared by the ISR after each interrupt event, confirming that the Default IRQ Handler fires correctly.\n5. Event counter validation: The event counter value must be checked after interrupt handling; if below the expected threshold, additional interrupt waits are performed to ensure all events are processed.\n6. ISR interrupt status check: The ISR must successfully read the system-level masked status and raw status registers, and clear bit 31 of the raw status register when set.\n7. Event count acknowledgment: The ISR must read the event count register, store the value, and write it back to acknowledge processed events.\n8. GIC IRQ clearing: The ISR must clear IRQ 84 in the GIC after completing interrupt handling.\n9. Debug marker verification: Debug marker values must be written at each major test phase to confirm sequential test progression through enumeration, descriptor responses, and configuration stages.",
    "Remarks": "This testcase operates in USB device mode at full-speed. It uses interrupt-based synchronization with a global pending flag and event counter for multi-phase transfer coordination. The test covers control transfer setup, data, and status stages as well as bulk transfer TRB programming. The ISR handles both USB event interrupts and system-level interrupt clearing."
  },
  {
    "Index": "2",
    "SS / Module": "USB",
    "Test Case Name": "usb_host_enumeration_hs",
    "Feature": "USB Host Mode High-Speed Enumeration",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This testcase validates USB host-mode high-speed device enumeration. It initializes the USB host controller, configures the USB 2.0 PHY, reads host capability parameters including port count and supported protocol capabilities for USB 2.0 and USB 3.0, and verifies initial port status. Wake-on events (Wake on Connect, Wake on Disconnect, Wake on Over-current) are enabled on the USB 2.0 port. The test sets up the Device Context Base Address Array, Command Ring, Event Ring Segment Table, and associated interrupter registers. The controller is started with interrupt enable and run/stop bits set. The test waits for a port connect event via interrupt-driven polling, verifies USB status and port status, then performs a port reset sequence. After port reset completion, the test issues an Enable Slot command followed by an Address Device command through the Command Ring, configuring the Input Context with slot and EP0 context data. The Configuration register is updated to enable Configuration Information. The test then performs standard USB enumeration by issuing GET Device Descriptor, GET Device Qualifier Descriptor, and GET Configuration Descriptor requests via EP0 Transfer Request Blocks. Each command and transfer phase is synchronized through doorbell writes and interrupt-driven event completion polling. The Event Ring Dequeue Pointer is advanced after each event consumption. The ISR handles system-level interrupt status checking, interrupter management acknowledgment, and GIC IRQ clearing. The test completes successfully upon finishing all enumeration descriptor transfers.",
    "Test Steps / Procedure": "1. Perform NIC programming and enable all GIC IRQs.\n2. Configure the USB 2.0 PHY control register.\n3. Read the host controller structural parameters register to determine port count.\n4. Read the USB 2.0 and USB 3.0 supported protocol capability registers.\n5. Read the initial port status and control register.\n6. Enable Wake on Connect, Wake on Disconnect, and Wake on Over-current events on the USB 2.0 port status register.\n7. Write the port status register with the combined wake-on configuration.\n8. Read the doorbell offset register.\n9. Set up the Device Context Base Address Array with pointers to device context memory regions for multiple slots.\n10. Configure the Command Ring Control register (low and high) with the command ring base pointer.\n11. Write the Configure register with the maximum number of device slots enabled.\n12. Write the Device Context Base Address Array Pointer registers (low and high).\n13. Write the Event Ring Segment Table Size register.\n14. Initialize the Event Ring Segment Table in memory with the event ring base address and segment size.\n15. Read the host controller structural parameters 2 register and page size register.\n16. Write the Event Ring Dequeue Pointer registers (low and high) with the event ring base address.\n17. Write the Event Ring Segment Table Base Address registers (low and high).\n18. Write the Interrupt Moderation register and Interrupt Management register.\n19. Enable host controller interrupts via the USB Command register.\n20. Enable system-level interrupt at the system register level.\n21. Start the host controller by setting the run/stop bit in the USB Command register.\n22. Wait for port connect event via interrupt-driven polling.\n23. Read the USB Status register to verify the event interrupt.\n24. Clear the event interrupt bit in the USB Status register.\n25. Acknowledge the Interrupt Management register.\n26. Read the port status register to verify port connect status.\n27. Clear USB status and acknowledge interrupt management again.\n28. Advance the Event Ring Dequeue Pointer.\n29. Initiate port reset by writing the port status register with the reset bit.\n30. Clear USB status and acknowledge interrupt management after port reset.\n31. Write the port status register to complete the reset sequence.\n32. Read the port status register to verify reset completion.\n33. Wait for port reset complete event via interrupt-driven polling.\n34. Execute the set_address sequence: write an Enable Slot command TRB to the Command Ring, set up the Input Context with slot context and EP0 endpoint context data, write an Address Device command TRB to the Command Ring.\n35. Clear USB status, acknowledge interrupt management, and advance the Event Ring Dequeue Pointer.\n36. Ring the host controller doorbell to initiate command processing.\n37. Wait for command completion via interrupt-driven polling.\n38. Read the event ring to verify command completion status.\n39. Advance the Event Ring Dequeue Pointer, clear USB status, and acknowledge interrupt management.\n40. Write the port status register for port enable.\n41. Ring the doorbell and wait for completion via interrupt-driven polling.\n42. Update the Input Context control fields.\n43. Read and update the Configure register to enable Configuration Information.\n44. Write an Evaluate Context command TRB to the Command Ring.\n45. Clear USB status, acknowledge interrupt management, and advance the Event Ring Dequeue Pointer.\n46. Ring the doorbell and wait for command completion via interrupt-driven polling.\n47. Read the event ring to verify Evaluate Context command completion.\n48. Execute the enumeration sequence: write GET Device Descriptor Setup Stage TRB and Data Stage TRB to the EP0 transfer ring.\n49. Write GET Device Qualifier Descriptor Setup Stage TRB, Data Stage TRB, and Status Stage TRB.\n50. Write GET Configuration Descriptor Setup Stage TRB, Data Stage TRB, and Status Stage TRB.\n51. Ring the host controller doorbell to initiate EP0 transfers.\n52. Wait for transfer completion via interrupt-driven polling.\n53. Clear USB status, acknowledge interrupt management, and advance the Event Ring Dequeue Pointer.\n54. Ring the doorbell for the device slot to complete enumeration.\n55. Wait for final transfer completion via interrupt-driven polling.\n56. Read the event ring to verify final transfer completion.\n57. Complete the test successfully.\n58. ISR: Clear the interrupt pending flag, read system-level masked status and raw status registers, write the Interrupt Management register to acknowledge, clear the raw status register bit 31 if set, and clear GIC IRQ.",
    "Impacted Registers": "USBCMD, USBSTS, HCSPARAMS1, PORTSC_20, SUPTPRT2_DW2, SUPTPRT3_DW2, CRCR_LO, CRCR_HI, CONFIG, DCBAAP_LO, DCBAAP_HI, ERSTSZ, ERSTBA_LO, ERSTBA_HI, ERDP_LO, ERDP_HI, IMAN, IMOD, DB",
    "Validation / Acceptance Criteria": "1. Port connect detection: After starting the host controller with run/stop and interrupt enable bits set, the ISR must fire and clear the interrupt pending flag, confirming a port status change event was received.\n2. USB status verification: The USB Status register must reflect the event interrupt bit being set after a port connect event.\n3. USB status clearing: Writing the event interrupt bit to the USB Status register must successfully clear it.\n4. Interrupt management acknowledgment: Writing to the Interrupt Management register must successfully acknowledge the pending interrupt.\n5. Port status verification: The Port Status and Control register must indicate a connected device after the port connect event.\n6. Port reset completion: After initiating a port reset via the Port Status register, the interrupt must fire indicating port reset is complete, and the port status must reflect the reset-complete state.\n7. Enable Slot command completion: After writing the Enable Slot command TRB to the Command Ring and ringing the doorbell, the command must complete successfully as indicated by the event ring completion entry.\n8. Address Device command completion: After writing the Address Device command TRB with the Input Context pointer and ringing the doorbell, the command must complete successfully as verified by reading the event ring completion entry.\n9. Evaluate Context command completion: After writing the Evaluate Context command TRB and ringing the doorbell, the command must complete successfully as verified by reading the event ring completion entry.\n10. GET Device Descriptor transfer completion: The Setup Stage and Data Stage TRBs for GET Device Descriptor must complete successfully, confirmed by interrupt-driven event completion.\n11. GET Device Qualifier Descriptor transfer completion: The Setup Stage, Data Stage, and Status Stage TRBs for GET Qualifier Descriptor must complete successfully.\n12. GET Configuration Descriptor transfer completion: The Setup Stage, Data Stage, and Status Stage TRBs for GET Configuration Descriptor must complete successfully.\n13. Event Ring Dequeue Pointer advancement: The ERDP must be correctly advanced in increments after each event consumption cycle to track processed events.\n14. ISR validation: The ISR must successfully read system-level status registers, acknowledge the Interrupt Management register, clear the raw status register bit 31 when set, and clear GIC IRQ after each interrupt.\n15. Final test completion: The test must complete successfully with a pass status after all enumeration descriptor transfers are finished.",
    "Remarks": "This testcase operates in USB host mode at high-speed. It performs full device enumeration including Enable Slot, Address Device, and Evaluate Context commands followed by GET Device Descriptor, GET Device Qualifier Descriptor, and GET Configuration Descriptor control transfers. The test uses interrupt-based synchronization with a global pending flag for multi-phase command and transfer coordination. The Event Ring Dequeue Pointer is manually advanced after each event consumption. The ISR handles both USB interrupter-level and system-level interrupt acknowledgment."
  }
]

# ── META_JSON Data (2 records) ────────────────────────────────────────
META = [
  {
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "usb.h"',
    "Meta Macros": "#define MIZAR_USB_BASE 0xa1800000\n#define USB_GCTL_OFFSET 0xC110\n#define USB_GEVNTADRLO_OFFSET 0xC400\n#define USB_GEVNTADRHI_OFFSET 0xC404\n#define USB_GEVNTSIZ_OFFSET 0xC408\n#define USB_GEVNTCOUNT_OFFSET 0xC40C\n#define USB_DCFG_OFFSET 0xC700\n#define USB_DCTL_OFFSET 0xC704\n#define USB_DEVTEN_OFFSET 0xC708\n#define USB_DSTS_OFFSET 0xC70C\n#define USB_DALEPENA_OFFSET 0xC720\n#define USB_GUCTL_OFFSET 0xC12C\n#define USB_GUSB2PHYCFG_OFFSET 0xC200\n#define USB_DEPCMDPAR1_OFFSET 0xC804\n#define USB_DEPCMDPAR0_OFFSET 0xC808\n#define USB_DEPCMD_OFFSET 0xC80C\n#define MIZAR_USB_GCTL (MIZAR_USB_BASE + USB_GCTL_OFFSET)\n#define MIZAR_USB_GEVNTADRLO (MIZAR_USB_BASE + USB_GEVNTADRLO_OFFSET)\n#define MIZAR_USB_GEVNTADRHI (MIZAR_USB_BASE + USB_GEVNTADRHI_OFFSET)\n#define MIZAR_USB_GEVNTSIZ (MIZAR_USB_BASE + USB_GEVNTSIZ_OFFSET)\n#define MIZAR_USB_GEVNTCOUNT (MIZAR_USB_BASE + USB_GEVNTCOUNT_OFFSET)\n#define MIZAR_USB_DCFG (MIZAR_USB_BASE + USB_DCFG_OFFSET)\n#define MIZAR_USB_DSTS (MIZAR_USB_BASE + USB_DSTS_OFFSET)\n#define MIZAR_USB_DCTL (MIZAR_USB_BASE + USB_DCTL_OFFSET)\n#define MIZAR_USB_DALEPENA (MIZAR_USB_BASE + USB_DALEPENA_OFFSET)\n#define MIZAR_USB_DEVTEN (MIZAR_USB_BASE + USB_DEVTEN_OFFSET)\n#define MIZAR_USB_GUCTL (MIZAR_USB_BASE + USB_GUCTL_OFFSET)\n#define MIZAR_USB_GUSB2PHYCFG (MIZAR_USB_BASE + USB_GUSB2PHYCFG_OFFSET)\n#define MIZAR_USB_DEPCMDPAR1 (MIZAR_USB_BASE + USB_DEPCMDPAR1_OFFSET)\n#define MIZAR_USB_DEPCMDPAR0 (MIZAR_USB_BASE + USB_DEPCMDPAR0_OFFSET)\n#define MIZAR_USB_DEPCMD (MIZAR_USB_BASE + USB_DEPCMD_OFFSET)\n#define DWORD 0x4\n#define Buffer_PointerLO (RAM+0x1100)\n#define Buffer_PointerLO_1 (RAM+0x1220)\n#define event_trb_addr (RAM+0x3000)\n#define Default_Event_Ring_Array (RAM+0xd00)\n#if defined(DDR_TRBS)\n#define RAM 0x11A0000000\n#else\n#define RAM 0xa0240000\n#endif",
    "Meta Arrays": "int buf_data[16]; \u2014 local integer array declared in test_case()",
    "Meta Types": "extern int int_pend;\nint event_counter;\nint test_case() \u2014 local variables: int rd_data, int wr_data, int port_count, int db_offset, int buf_data[16], int i, int bulk, int intr, int event_comletion, int j, int hand_shake\nvoid Default_IRQHandler() \u2014 local variables: int rd_data, int sysreg_rd_data, int event_count\nvoid setup_stage() \u2014 local variables: int rd_data\nvoid status_stage() \u2014 local variables: int rd_data\nvoid enumeration()\nvoid set_configuration(int trb_address, int parameter0, int parameter1, int cmd) \u2014 local variables: int read_data",
    "Meta Test Description": "The testcase calls nic_programming() and GIC_EnableAllIRQ(), initializes Buffer_PointerLO and event_trb_addr memory regions to zero in a loop of 20 iterations, writes MIZAR_USB_GEVNTADRLO with Default_Event_Ring_Array, writes MIZAR_USB_GEVNTADRHI with 0x0, writes MIZAR_USB_GEVNTSIZ with 0x30, writes MIZAR_USB_GEVNTCOUNT with 0x0, reads MIZAR_USB_GCTL, writes debug marker 0xdeadbee0 to 0xA0243ffc, reads MIZAR_USB_DCFG and MIZAR_USB_DSTS, writes MIZAR_USB_DCFG with 0x480801, writes MIZAR_USB_DCTL with 0x80f00a00, writes MIZAR_USB_DALEPENA with 0xff, calls setup_stage(), writes MIZAR_USB_GUSB2PHYCFG with 0x40002547, checks event_counter against 0x4, writes device descriptor data and configuration descriptor data to Buffer_PointerLO, sets up TRBs at event_trb_addr, issues endpoint commands via MIZAR_USB_DEPCMD with value 0x506 and polls for completion, uses int_pend-based synchronization with Default_IRQHandler, calls status_stage(), and writes debug markers 0xdeadbee3, 0xdeadbee4, 0xdeadbee5 to 0xA0243ffc.",
    "Meta Test Steps / Procedure": "1. Call nic_programming().\n2. Call GIC_EnableAllIRQ().\n3. for(j=0; j<20; j++) { write_reg(Buffer_PointerLO + j*DWORD, 0x0); write_reg(event_trb_addr + j*DWORD, 0x0); }\n4. write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);\n5. write_reg(MIZAR_USB_GEVNTADRHI, 0x0);\n6. write_reg(MIZAR_USB_GEVNTSIZ, 0x30);\n7. write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);\n8. rd_data = read_reg(MIZAR_USB_GCTL);\n9. write_reg(0xA0243ffc, 0xdeadbee0);\n10. rd_data = read_reg(MIZAR_USB_DCFG);\n11. rd_data = read_reg(MIZAR_USB_DSTS).\n12. write_reg(MIZAR_USB_DCFG, 0x480801);\n13. write_reg(MIZAR_USB_DCTL, 0x80f00a00);\n14. write_reg(MIZAR_USB_DALEPENA, 0xff);\n15. printf(\"Buffer_PointerLO is %x\\n\", Buffer_PointerLO);\n16. wait_on(5000);\n17. Call setup_stage().\n18. write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);\n19-37. Continue with descriptor writes, TRB setup, DEPCMD commands, ISR handling, and debug markers.",
    "Meta Impacted Registers": "GCTL, GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, GEVNTCOUNT, DCFG, DSTS, DCTL, DALEPENA, DEVTEN, GUCTL, GUSB2PHYCFG, DEPCMDPAR1, DEPCMDPAR0, DEPCMD",
    "Meta Validation / Acceptance Criteria": "1. DEPCMD command completion polling (EP0): write_reg(MIZAR_USB_DEPCMD, 0x506); rd_data = read_reg(MIZAR_USB_DEPCMD); while(rd_data == 0x506) { wait_on(10); rd_data = read_reg(MIZAR_USB_DEPCMD); } \u2014 pass condition: rd_data changes from 0x506.\n2. DEPCMD+0x10 command completion polling.\n3. set_configuration() command completion polling.\n4. Interrupt-driven synchronization.\n5. Event counter check.\n6. Interrupt status check in Default_IRQHandler.\n7. Event count acknowledgment.\n8. GIC_ClearIRQ(84).\n9. Debug marker writes."
  },
  {
    "Test Case Name": "usb_host_enumeration_hs",
    "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "usb.h"',
    "Meta Macros": "#define MIZAR_USB_BASE 0xa1800000\n#define USB_USBCMD_OFFSET 0x20\n#define USB_USBSTS_OFFSET 0x24\n#define USB_HCSPARAMS1_OFFSET 0x4\n#define USB_HCSPARAMS2_OFFSET 0x8\n#define USB_PAGESIZE_OFFSET 0x28\n#define USB_PORTSC_20_OFFSET 0x420\n#define USB_SUPTPRT2_DW2_OFFSET 0x898\n#define USB_SUPTPRT3_DW2_OFFSET 0x8A8\n#define USB_CRCR_LO_OFFSET 0x38\n#define USB_CRCR_HI_OFFSET 0x3C\n#define USB_CONFIG_OFFSET 0x58\n#define USB_DCBAAP_LO_OFFSET 0x50\n#define USB_DCBAAP_HI_OFFSET 0x54\n#define USB_ERSTSZ_OFFSET 0x468\n#define USB_ERSTBA_LO_OFFSET 0x470\n#define USB_ERSTBA_HI_OFFSET 0x474\n#define USB_ERDP_LO_OFFSET 0x478\n#define USB_ERDP_HI_OFFSET 0x47C\n#define USB_IMAN_OFFSET 0x460\n#define USB_IMOD_OFFSET 0x464\n#define USB_DB_OFFSET 0x480\n#define USB_DBOFF_OFFSET 0x14\n#define MIZAR_USB_USBCMD (MIZAR_USB_BASE + USB_USBCMD_OFFSET)\n#define MIZAR_USB_USBSTS (MIZAR_USB_BASE + USB_USBSTS_OFFSET)\n#define MIZAR_USB_HCSPARAMS1 (MIZAR_USB_BASE + USB_HCSPARAMS1_OFFSET)\n#define MIZAR_USB_HCSPARAMS2 (MIZAR_USB_BASE + USB_HCSPARAMS2_OFFSET)\n#define MIZAR_USB_PAGESIZE (MIZAR_USB_BASE + USB_PAGESIZE_OFFSET)\n#define MIZAR_USB_PORTSC_20 (MIZAR_USB_BASE + USB_PORTSC_20_OFFSET)\n#define MIZAR_USB_SUPTPRT2_DW2 (MIZAR_USB_BASE + USB_SUPTPRT2_DW2_OFFSET)\n#define MIZAR_USB_SUPTPRT3_DW2 (MIZAR_USB_BASE + USB_SUPTPRT3_DW2_OFFSET)\n#define MIZAR_USB_CRCR_LO (MIZAR_USB_BASE + USB_CRCR_LO_OFFSET)\n#define MIZAR_USB_CRCR_HI (MIZAR_USB_BASE + USB_CRCR_HI_OFFSET)\n#define MIZAR_USB_CONFIG (MIZAR_USB_BASE + USB_CONFIG_OFFSET)\n#define MIZAR_USB_DCBAAP_LO (MIZAR_USB_BASE + USB_DCBAAP_LO_OFFSET)\n#define MIZAR_USB_DCBAAP_HI (MIZAR_USB_BASE + USB_DCBAAP_HI_OFFSET)\n#define MIZAR_USB_ERSTSZ (MIZAR_USB_BASE + USB_ERSTSZ_OFFSET)\n#define MIZAR_USB_ERSTBA_LO (MIZAR_USB_BASE + USB_ERSTBA_LO_OFFSET)\n#define MIZAR_USB_ERSTBA_HI (MIZAR_USB_BASE + USB_ERSTBA_HI_OFFSET)\n#define MIZAR_USB_ERDP_LO (MIZAR_USB_BASE + USB_ERDP_LO_OFFSET)\n#define MIZAR_USB_ERDP_HI (MIZAR_USB_BASE + USB_ERDP_HI_OFFSET)\n#define MIZAR_USB_IMAN (MIZAR_USB_BASE + USB_IMAN_OFFSET)\n#define MIZAR_USB_IMOD (MIZAR_USB_BASE + USB_IMOD_OFFSET)\n#define MIZAR_USB_DB (MIZAR_USB_BASE + USB_DB_OFFSET)\n#define MIZAR_USB_DBOFF (MIZAR_USB_BASE + USB_DBOFF_OFFSET)\n#define DWORD 0x4\n#define Default_Command_Ring (RAM+0x0)\n#define Default_Event_Ring_Array (RAM+0xd00)\n#define Device_Context_Base_Address_Array (RAM+0xf00)\n#define Event_Ring_Segment_Table (RAM+0x1800)\n#define EP0_TR_Dequeue_Pointer (RAM+0x1870)\n#define Default_Input_Context (RAM+0x3800)\n#define Device_Context_Array (RAM+0x3940)\n#if defined(DDR_TRBS)\n#define RAM 0x11A0000000\n#else\n#define RAM 0xa0240000\n#endif",
    "Meta Arrays": "int data_in[512]; \u2014 local integer array declared in test_case()\nint data_out[512]; \u2014 local integer array declared in test_case()",
    "Meta Types": "extern int int_pend;\nint test_case() \u2014 local variables: int rd_data, int port_count, int db_offset, int event_completion, int usb_status, int port_status, int input_context_address, int data_in[512], int j, int data_out[512], int i, int count\nvoid Default_IRQHandler() \u2014 local variables: int rd_data, int sysreg_rd_data\nvoid set_address() \u2014 local variables: int input_context_address, int event_completion, int usb_status\nvoid enumeration() \u2014 local variables: int event_completion",
    "Meta Test Description": "USB host-mode high-speed enumeration testcase. Calls nic_programming() and GIC_EnableAllIRQ(). Configures PHY control register at MIZAR_USB_BASE+0xc200 with 0x102407. Reads MIZAR_USB_HCSPARAMS1 for port count, reads MIZAR_USB_SUPTPRT2_DW2 and MIZAR_USB_SUPTPRT3_DW2 for supported protocol capabilities, reads MIZAR_USB_PORTSC_20 for initial port status. Enables wake-on events (WCE, WDE, WOE) on MIZAR_USB_PORTSC_20 and writes 0xe0002a0. Reads MIZAR_USB_DBOFF for doorbell offset. Sets up Device_Context_Base_Address_Array with pointers to Device_Context_Array. Configures MIZAR_USB_CRCR_LO with Default_Command_Ring+0x1 and MIZAR_USB_CRCR_HI with 0x0. Writes MIZAR_USB_CONFIG with 0x10 (MaxSlotsEn=16). Writes MIZAR_USB_DCBAAP_LO with Device_Context_Base_Address_Array and MIZAR_USB_DCBAAP_HI with 0x0. Writes MIZAR_USB_ERSTSZ with 0x1. Initializes Event_Ring_Segment_Table with Default_Event_Ring_Array base address and segment size 0x20. Reads MIZAR_USB_HCSPARAMS2 and MIZAR_USB_PAGESIZE. Sets up ERDP, ERSTBA, IMOD, IMAN registers. Enables interrupts via MIZAR_USB_USBCMD with 0x4. Enables system-level interrupt via MIZAR_LSS_SYSREG_INTR_EN0 with 0x80000000. Starts controller via MIZAR_USB_USBCMD with 0x5. Waits for port connect event via int_pend polling. Reads MIZAR_USB_USBSTS and MIZAR_USB_PORTSC_20. Performs port reset via MIZAR_USB_PORTSC_20 writes. Calls set_address() to issue Enable Slot and Address Device commands via Default_Command_Ring. Sets up Default_Input_Context with slot and EP0 context. Writes MIZAR_USB_CONFIG with 0x110 (CIE). Calls enumeration() to perform GET Device Descriptor, GET Qualifier Descriptor, and GET Configuration Descriptor via EP0 TRBs. Rings doorbell via MIZAR_USB_DB. Completes with finish(0).",
    "Meta Test Steps / Procedure": "1. Call nic_programming().\n2. Call GIC_EnableAllIRQ().\n3. rd_data = read_reg(MIZAR_USB_BASE+0xc200); write_reg(MIZAR_USB_BASE+0xc200, 0x102407); \u2014 PHY Control Register.\n4. port_count = read_reg(MIZAR_USB_HCSPARAMS1).\n5. rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2).\n6. rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2).\n7. rd_data = read_reg(MIZAR_USB_PORTSC_20).\n8-90. Full steps including wake-on events, DCBAA setup, command ring setup, event ring setup, controller start, port connect detection, port reset, set_address with Enable Slot and Address Device commands, input context setup, enumeration with GET Device/Qualifier/Configuration Descriptors via EP0 TRBs, doorbell ringing, ISR handling.",
    "Meta Impacted Registers": "USBCMD, USBSTS, HCSPARAMS1, PORTSC_20, SUPTPRT2_DW2, SUPTPRT3_DW2, CRCR_LO, CRCR_HI, CONFIG, DCBAAP_LO, DCBAAP_HI, ERSTSZ, ERSTBA_LO, ERSTBA_HI, ERDP_LO, ERDP_HI, IMAN, IMOD, DB",
    "Meta Validation / Acceptance Criteria": "1. Port connect detection.\n2. USB status check.\n3. USB status clear.\n4. Interrupt management acknowledgment.\n5. Port status verification.\n6. Port reset completion.\n7. Slot command completion.\n8. Address Device command completion.\n9. EP command completion.\n10. GET Device Descriptor completion.\n11. GET Qualifier Descriptor completion.\n12. GET Configuration Descriptor completion.\n13. ERDP advancement validation.\n14. ISR validation.\n15. GIC_ClearIRQ(84).\n16. finish(0)."
  }
]

# ── Cross-Dataset Validation ─────────────────────────────────────────
tp_names = {tc["Test Case Name"] for tc in TESTPLAN}
meta_names = {m["Test Case Name"] for m in META}
if tp_names != meta_names:
    print("VALIDATION FAILED: Test Case Name mismatch between TESTPLAN and META")
    sys.exit(1)
print(f"Cross-dataset validation PASSED: {len(tp_names)} test case(s) matched")

# ── Workbook Creation ─────────────────────────────────────────────────
wb = openpyxl.Workbook()

header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

# ── TestPlan Sheet ────────────────────────────────────────────────────
ws_tp = wb.active
ws_tp.title = "TestPlan"
tp_cols = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks"
]

for c, h in enumerate(tp_cols, 1):
    cell = ws_tp.cell(row=1, column=c, value=h)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap

for r, tc in enumerate(TESTPLAN, 2):
    ws_tp.cell(row=r, column=1, value=tc.get("Index", "")).alignment = wrap
    ws_tp.cell(row=r, column=2, value=tc.get("SS / Module", "")).alignment = wrap
    ws_tp.cell(row=r, column=3, value=tc.get("Feature", "")).alignment = wrap
    ws_tp.cell(row=r, column=4, value=tc.get("Test Case Name", "")).alignment = wrap
    ws_tp.cell(row=r, column=5, value=tc.get("Speed", "")).alignment = wrap
    ws_tp.cell(row=r, column=6, value=tc.get("Mode", "")).alignment = wrap
    ws_tp.cell(row=r, column=7, value=tc.get("Memory Start Offset", "")).alignment = wrap
    ws_tp.cell(row=r, column=8, value=tc.get("Memory End Offset", "")).alignment = wrap
    ws_tp.cell(row=r, column=9, value=tc.get("Test Description", "")).alignment = wrap
    ws_tp.cell(row=r, column=10, value=tc.get("Test Steps / Procedure", "")).alignment = wrap
    ws_tp.cell(row=r, column=11, value=tc.get("Impacted Registers", "")).alignment = wrap
    ws_tp.cell(row=r, column=12, value=tc.get("Validation / Acceptance Criteria", "")).alignment = wrap
    ws_tp.cell(row=r, column=13, value=tc.get("Remarks", "")).alignment = wrap

ws_tp.freeze_panes = "A2"

# ── MetaData Sheet (veryHidden) ───────────────────────────────────────
ws_md = wb.create_sheet("MetaData")
ws_md.sheet_state = "veryHidden"
md_cols = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

for c, h in enumerate(md_cols, 1):
    cell = ws_md.cell(row=1, column=c, value=h)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap

for r, m in enumerate(META, 2):
    for c, col in enumerate(md_cols, 1):
        ws_md.cell(row=r, column=c, value=m.get(col, "")).alignment = wrap

ws_md.freeze_panes = "A2"

# ── Auto-size Columns ────────────────────────────────────────────────
for ws in [ws_tp, ws_md]:
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
        adjusted = min(max_len + 2, 60)
        ws.column_dimensions[col_letter].width = max(adjusted, 12)

# ── Save Workbook ────────────────────────────────────────────────────
output_dir = os.path.dirname(os.path.abspath(__file__))
filepath = os.path.join(output_dir, filename)
wb.save(filepath)

# ── Post-Save Validation ─────────────────────────────────────────────
if not os.path.exists(filepath):
    print("POST-SAVE VALIDATION FAILED: File does not exist")
    sys.exit(1)
if os.path.getsize(filepath) == 0:
    print("POST-SAVE VALIDATION FAILED: File size is 0")
    sys.exit(1)

wb_check = openpyxl.load_workbook(filepath)
sheet_names = wb_check.sheetnames
if "TestPlan" not in sheet_names:
    print("POST-SAVE VALIDATION FAILED: TestPlan sheet missing")
    sys.exit(1)
if "MetaData" not in sheet_names:
    print("POST-SAVE VALIDATION FAILED: MetaData sheet missing")
    sys.exit(1)

tp_row_count = wb_check["TestPlan"].max_row - 1
md_row_count = wb_check["MetaData"].max_row - 1

if tp_row_count != len(TESTPLAN):
    print(f"POST-SAVE VALIDATION FAILED: TestPlan rows {tp_row_count} != {len(TESTPLAN)}")
    sys.exit(1)
if md_row_count != len(META):
    print(f"POST-SAVE VALIDATION FAILED: MetaData rows {md_row_count} != {len(META)}")
    sys.exit(1)

print("POST-SAVE VALIDATION PASSED")
print(f"GENERATED: {filepath}")
print(f"FILENAME: {filename}")
print(f"TESTPLAN_ROWS: {len(TESTPLAN)}")
print(f"METADATA_ROWS: {len(META)}")
print(f"FILE_SIZE: {os.path.getsize(filepath)}")
