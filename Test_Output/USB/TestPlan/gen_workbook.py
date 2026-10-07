#!/usr/bin/env python3
"""
USB TestPlan Excel Workbook Generator
=====================================
Generates USB_TestPlan_YYYYMMDD_HHMMSS.xlsx using openpyxl.

Usage:
  pip install openpyxl
  python3 gen_workbook.py

Output:
  USB_TestPlan_<IST_timestamp>.xlsx in the same directory as this script.

This script is self-contained. It embeds all test case data and produces
a fully formatted, validated Excel workbook ready for use.
"""

import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import os
import sys

# ============================================================
# IST Timezone and Filename
# ============================================================
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"USB_TestPlan_{timestamp}.xlsx"
script_dir = os.path.dirname(os.path.abspath(__file__))
filepath = os.path.join(script_dir, filename)

# ============================================================
# TEST CASE DATA (2 rows)
# ============================================================
json_data = [
  {
    "Index": "1",
    "SS / Module": "USB",
    "Test Case Name": "usb_host_enumeration_hs",
    "Feature": "USB Host Enumeration - High Speed",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"",
    "Meta Macros": "MIZAR_USB_BASE, MIZAR_USB_CONFIG, MIZAR_USB_DCBAAP_LO, MIZAR_USB_DCBAAP_HI, MIZAR_USB_ERSTSZ, MIZAR_USB_USBSTS, MIZAR_USB_IMAN, MIZAR_USB_ERDP_LO, MIZAR_USB_ERDP_HI, MIZAR_USB_ERSTBA_LO, MIZAR_USB_ERSTBA_HI, MIZAR_USB_IMOD, MIZAR_USB_USBCMD, MIZAR_USB_HCSPARAMS1, MIZAR_USB_SUPTPRT2_DW2, MIZAR_USB_SUPTPRT3_DW2, MIZAR_USB_PORTSC_20, MIZAR_USB_DBOFF, MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, USB_PORTSC_20_WCE, USB_PORTSC_20_WDE, USB_PORTSC_20_WOE, Device_Context_Base_Address_Array, Default_Event_Ring_Array, Event_Ring_Segment_Table, EP0_TR_Dequeue_Pointer, Default_Command_Ring, Default_Input_Context, MIZAR_USB_DB",
    "Meta Arrays": "int data_in[512];\nint data_out[512];",
    "Speed": "High-Speed (HS)",
    "Mode": "Host",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "Verify USB host enumeration of a high-speed device. The test initializes the USB host controller, configures the event ring and device context base address array, enables interrupts, starts the controller, detects device connection, performs port reset, assigns a device address via Enable Slot and Address Device commands, and enumerates the device by issuing GET_DESCRIPTOR control transfers on Endpoint 0.",
    "Meta Test Description": "USB host enumeration test for high-speed (HS) device. The testcase initializes the xHCI host controller, configures event ring infrastructure (ERDP, ERSTBA, ERSTSZ), enables interrupts (IMAN, USBCMD, SYSREG_INTR_EN0), starts the host controller via USBCMD run/stop bit, detects device connection via port status change interrupt on the USB 2.0 port (PORTSC_20), performs port reset, issues Enable Slot and Address Device commands via the command ring in set_address(), and performs USB enumeration (GET_DESCRIPTOR) via control transfer TRBs on Endpoint 0 in enumeration(). Completion is signaled via finish(0).",
    "Test Steps / Procedure": "1. Call NIC programming initialization.\n2. Enable all IRQs via GIC.\n3. Read and configure PHY control register.\n4. Read host controller structural parameters (HCSPARAMS1) to get port count.\n5. Read supported protocol capability registers (SUPTPRT2_DW2, SUPTPRT3_DW2).\n6. Read PORTSC_20 for initial port status.\n7. Read and configure CONFIG register with value 0x110 (CIE enabled).\n8. Write Device Context Base Address Array pointer to DCBAAP_LO and DCBAAP_HI.\n9. Write Event Ring Segment Table Size to ERSTSZ (value 0x1).\n10. Configure Event Ring Dequeue Pointer via ERDP_LO and ERDP_HI.\n11. Configure Event Ring Segment Table Base Address via ERSTBA_LO and ERSTBA_HI.\n12. Set Interrupter Moderation (IMOD) to 0x0.\n13. Configure Interrupter Management (IMAN) with value 0x2.\n14. Write USBCMD with 0x4 to enable interrupt generation.\n15. Configure PORTSC_20 wake enable bits (WCE, WDE, WOE) via read-modify-write, then write 0xe0002a0.\n16. Read doorbell offset register (DBOFF).\n17. Enable system-level interrupt via SYSREG_INTR_EN0.\n18. Write USBCMD with 0x5 to start host controller (Run).\n19. Wait for port status change interrupt (int_pend polling loop).\n20. Read USBSTS, clear status, acknowledge interrupt via IMAN, clear ERDP_HI.\n21. Read PORTSC_20 to verify port connect status is high.\n22. Update ERDP_LO, issue port reset via PORTSC_20 (write 0xe0006f1).\n23. Clear USBSTS and IMAN, write post-reset configuration to PORTSC_20, read port status.\n24. Wait for port reset completion interrupt.\n25. Call set_address(): Issue Enable Slot command TRB on command ring, clear status/interrupts, issue Address Device command TRB with input context, update ERDP_LO, ring doorbell via DB register.\n26. Read event completion from Event Ring Array, wait for interrupt.\n27. Read event completion again after interrupt.\n28. Call enumeration(): Construct GET_DESCRIPTOR Setup Stage TRB, Data Stage TRB, and Status Stage TRB on EP0 transfer ring, ring doorbell.\n29. Call finish(0) to signal test pass.",
    "Meta Test Steps / Procedure": "1. extern int int_pend;\n2. int test_case() {\n3. Declare variables: rd_data, port_count, db_offset, event_completion, usb_status, port_status, input_context_address, data_in[512], j, data_out[512], i, count=0;\n4. Call nic_programming();\n5. Call GIC_EnableAllIRQ(); // GIC_EnableIRQ(84) is commented out\n6. rd_data = read_reg(MIZAR_USB_BASE + 0xc200); // PHY CONTROL REG\n7. write_reg(MIZAR_USB_BASE + 0xc200, 0x102407);\n8. port_count = read_reg(MIZAR_USB_HCSPARAMS1);\n9. rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2);\n10. rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2);\n11. rd_data = read_reg(MIZAR_USB_PORTSC_20);\n12. rd_data = read_reg(MIZAR_USB_CONFIG);\n13. write_reg(MIZAR_USB_CONFIG, 0x110); // CONFIG CIE set to 1\n14. write_reg(MIZAR_USB_DCBAAP_LO, Device_Context_Base_Address_Array);\n15. write_reg(MIZAR_USB_DCBAAP_HI, 0x0);\n16. write_reg(MIZAR_USB_ERSTSZ, 0x1);\n17. write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array); // ERDP DWORD0\n18. write_reg(MIZAR_USB_ERDP_HI, 0x0);\n19. write_reg(MIZAR_USB_ERSTBA_LO, Event_Ring_Segment_Table);\n20. write_reg(MIZAR_USB_ERSTBA_HI, 0x0);\n21. write_reg(MIZAR_USB_IMOD, 0x0);\n22. write_reg(MIZAR_USB_IMAN, 0x2);\n23. write_reg(MIZAR_USB_USBCMD, 0x4); // Interrupt enable\n24. write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WCE, 1)); // Wake on Connect Enable\n25. write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WDE, 1)); // Wake on Disconnect Enable\n26. write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WOE, 1)); // Wake on Over-current Enable\n27. write_reg(MIZAR_USB_PORTSC_20, 0xe0002a0);\n28. db_offset = read_reg(MIZAR_USB_DBOFF);\n29. write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000); // Interrupt enable at sysreg\n30. write_reg(MIZAR_USB_USBCMD, 0x5); // USBCMD run/stop bit is 1, run\n31. int_pend = 1;\n32. while(int_pend) { wait_on(100); } // Wait for port status change interrupt\n33. usb_status = read_reg(MIZAR_USB_USBSTS);\n34. write_reg(MIZAR_USB_USBSTS, 0x8);\n35. write_reg(MIZAR_USB_IMAN, 0x2);\n36. write_reg(MIZAR_USB_ERDP_HI, 0x0);\n37. port_status = read_reg(MIZAR_USB_PORTSC_20); // port connect status should be high\n38. write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x18); // 32 bytes increment\n39. write_reg(MIZAR_USB_PORTSC_20, 0xe0006f1); // port reset\n40. write_reg(MIZAR_USB_USBSTS, 0x8);\n41. write_reg(MIZAR_USB_IMAN, 0x2);\n42. write_reg(MIZAR_USB_PORTSC_20, 0xe220200); // Post-reset port configuration\n43. port_status = read_reg(MIZAR_USB_PORTSC_20);\n44. int_pend = 1;\n45. while(int_pend) { wait_on(100); } // Wait for port reset completion\n46. Call set_address();\n47. input_context_address = (Default_Input_Context) + 0x0;\n48. event_completion = read_reg(Default_Event_Ring_Array + 0x50);\n49. int_pend = 1;\n50. while(int_pend) { wait_on(100); }\n51. event_completion = read_reg(Default_Event_Ring_Array + 0x50);\n52. Call enumeration();\n53. finish(0);\n54. }\n\nvoid Default_IRQHandler() {\n55. int rd_data, sysreg_rd_data;\n56. int_pend = 0;\n57. rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);\n58. rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);\n59. if (rd_data && 0x80000000) {\n60. write_reg(MIZAR_USB_IMAN, 0x1); // Clear interrupt pending\n61. write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);\n62. }\n63. GIC_ClearIRQ(84);\n64. }\n\nvoid set_address() {\n65. int input_context_address;\n66. int event_completion, usb_status;\n67. // Enable Slot command TRB\n68. write_reg(Default_Command_Ring + 0x0, 0x0);\n69. write_reg(Default_Command_Ring + 0x4, 0x0);\n70. write_reg(Default_Command_Ring + 0x8, 0x0);\n71. write_reg(Default_Command_Ring + 0xc, 0x00002401);\n72. usb_status = read_reg(MIZAR_USB_USBSTS);\n73. write_reg(MIZAR_USB_USBSTS, 0x8);\n74. write_reg(MIZAR_USB_IMAN, 0x2);\n75. write_reg(MIZAR_USB_ERDP_HI, 0x0);\n76. // Address Device command TRB\n77. input_context_address = (Default_Input_Context) + 0x0;\n78. write_reg(Default_Command_Ring + 0x10, input_context_address);\n79. write_reg(Default_Command_Ring + 0x1c, 0x01002e01);\n80. write_reg(MIZAR_USB_USBSTS, 0x8);\n81. write_reg(MIZAR_USB_IMAN, 0x2);\n82. write_reg(MIZAR_USB_ERDP_HI, 0x0);\n83. write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x68); // 32 bytes increment\n84. write_reg(MIZAR_USB_DB, 0x0); // Ring doorbell\n85. }\n\nvoid enumeration() {\n86. int event_completion;\n87. // GET Device Descriptor - Setup Stage\n88. write_reg(EP0_TR_Dequeue_Pointer, 0x01000680);\n89. write_reg(EP0_TR_Dequeue_Pointer + 0x4, 0x00120000);\n90. write_reg(EP0_TR_Dequeue_Pointer + 0x8, 0x08);\n91. write_reg(EP0_TR_Dequeue_Pointer + 0xc, 0x00030861);\n92. // Data Stage\n93. write_reg(EP0_TR_Dequeue_Pointer + 0x10, EP0_TR_Dequeue_Pointer + 0x200);\n94. write_reg(EP0_TR_Dequeue_Pointer + 0x18, 0x12);\n95. write_reg(EP0_TR_Dequeue_Pointer + 0x1c, 0x00010c27);\n96. // Status Stage\n97. write_reg(EP0_TR_Dequeue_Pointer + 0xc8, 0x18);\n98. write_reg(EP0_TR_Dequeue_Pointer + 0xcc, 0x00010c25);\n99. write_reg(EP0_TR_Dequeue_Pointer + 0xdc, 0x00001023);\n100. write_reg(MIZAR_USB_BASE + 0x484, 0x1); // Ring doorbell\n101.}",
    "Meta Impacted Registers": "CONFIG, DCBAAP_LO, DCBAAP_HI, ERSTSZ, USBSTS, IMAN, ERDP_LO, ERDP_HI, ERSTBA_LO, ERSTBA_HI, IMOD, USBCMD, HCSPARAMS1, SUPTPRT2_DW2, SUPTPRT3_DW2, PORTSC_20, DB",
    "Impacted Registers": "CONFIG, DCBAAP_LO, DCBAAP_HI, ERSTSZ, USBSTS, IMAN, ERDP_LO, ERDP_HI, ERSTBA_LO, ERSTBA_HI, IMOD, USBCMD, HCSPARAMS1, SUPTPRT2_DW2, SUPTPRT3_DW2, PORTSC_20, DB",
    "Meta Validation / Acceptance Criteria": "1. After port status change interrupt: port_status = read_reg(MIZAR_USB_PORTSC_20) \u2014 port connect status should be high.\n2. After port reset and interrupt: port_status = read_reg(MIZAR_USB_PORTSC_20) \u2014 verify port reset completed.\n3. event_completion = read_reg(Default_Event_Ring_Array + 0x50) \u2014 read before and after interrupt to verify event ring completion.\n4. Default_IRQHandler checks rd_data && 0x80000000 to verify USB interrupt (bit 31 of MIZAR_LSS_SYSREG_RAW_STCR0).\n5. finish(0) \u2014 signals test pass (return value 0).",
    "Validation / Acceptance Criteria": "1. After starting the host controller, a port status change interrupt must be received indicating device connection.\n2. PORTSC_20 port connect status should be high after the first interrupt.\n3. After issuing port reset via PORTSC_20, a port reset completion interrupt must be received.\n4. Event ring completion entries must be valid after Enable Slot and Address Device commands.\n5. The enumeration (GET_DESCRIPTOR) control transfer must complete successfully.\n6. The test must complete with finish(0) indicating pass.",
    "Remarks": "The high-speed enumeration test includes a set_address() function (Enable Slot + Address Device commands). The Default_IRQHandler contains a potential bug: uses logical AND (&&) instead of bitwise AND (&) when checking bit 31 of the raw interrupt status register. GIC_EnableIRQ(84) is commented out in favor of GIC_EnableAllIRQ(). The MIZAR_USB_BASE macro is used with offset 0xc200 for PHY control and offset 0x484 for doorbell but is unresolved in Agent 4 mappings. MIZAR_LSS_SYSREG_INTR_EN0 is also unresolved in Agent 4 mappings. In set_address(), ERDP_LO is updated to Default_Event_Ring_Array + 0x68, differing from the FS variant which uses 0x38. Post-interrupt handling includes clearing ERDP_HI after reading USBSTS, which differs slightly from the FS variant sequence."
  },
  {
    "Index": "2",
    "SS / Module": "USB",
    "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
    "Feature": "USB Full-Speed Device Bulk Transfer",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"",
    "Meta Macros": "MIZAR_USB_DEPCMD, MIZAR_USB_DEPCMDPAR0, MIZAR_USB_DEPCMDPAR1, MIZAR_USB_GCTL, MIZAR_USB_GUCTL, MIZAR_USB_DEVTEN, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, MIZAR_USB_GEVNTSIZ, MIZAR_USB_GEVNTCOUNT, MIZAR_USB_DCFG, MIZAR_USB_DSTS, MIZAR_USB_DCTL, MIZAR_USB_DALEPENA, MIZAR_USB_GUSB2PHYCFG, MIZAR_USB_GFLADJ, MIZAR_USB_BASE, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, Buffer_PointerLO, Buffer_PointerLO_1, event_trb_addr, Default_Event_Ring_Array, DWORD",
    "Meta Arrays": "int buf_data[16];",
    "Speed": "Full-Speed (FS)",
    "Mode": "Device",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "Verify USB full-speed device bulk data transfer. The test initializes the USB device controller, configures global event buffer and control registers, performs device enumeration (reading device configuration and status, setting DCFG, DCTL, DALEPENA), handles GET_DESCRIPTOR requests for device and configuration descriptors via setup/status stage control transfers, processes SET_ADDRESS, issues endpoint commands for bulk data transfer on the bulk endpoint, and validates transfer completion through interrupt-driven event handling.",
    "Meta Test Description": "USB full-speed device bulk transfer test. The testcase initializes the USB device controller by calling nic_programming() and GIC_EnableAllIRQ(), clears 20 entries of Buffer_PointerLO and event_trb_addr arrays, configures global event buffer registers (GEVNTADRLO with Default_Event_Ring_Array, GEVNTADRHI with 0x0, GEVNTSIZ with 0x30, GEVNTCOUNT with 0x0), reads GCTL, configures global registers (GCTL with 0x30c11234, GFLADJ with 0xa87f000, GUCTL with 0x2000010 via set_data read-modify-write), reads and writes PIPE control register at MIZAR_USB_BASE+0xc2c0 with 0x10c0002, performs enumeration by writing debug marker 0xdeadbee0 to 0xA0243ffc, reads DCFG and DSTS, writes DCFG with 0x480801, DCTL with 0x80f00a00, DALEPENA with 0xff, waits 5000 cycles, calls setup_stage() and writes GUSB2PHYCFG with 0x40002547, performs event_counter checks with int_pend wait loops using wait_on(5), handles GET DESCRIPTOR USB CONFIGURATION via setup_stage() with debug marker 0xdeadbee5, processes configuration descriptor data stage (09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301), handles SET ADDRESS by writing DCFG with 0x480809 and configuring TRBs with 0x853 control word, issues DEPCMD+0x10 with 0x506 for endpoint command and polls for completion, performs three int_pend wait loops with wait_on(5), calls status_stage(), calls setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT with debug marker 0xdeadbee4, configures bulk data transfer TRB with Buffer_PointerLO_1, transfer length 0x40 (64 bytes), TRB control 0x813, issues DEPCMD+0x40 with 0x506 for bulk endpoint start transfer command. The Default_IRQHandler reads GEVNTCOUNT, stores to event_counter, writes back to acknowledge events, checks SYSREG_RAW_STCR0 bit 31, and clears GIC IRQ 84.",
    "Test Steps / Procedure": "1. Call NIC programming initialization.\n2. Enable all IRQs via GIC.\n3. Clear 20 entries of Buffer_PointerLO and event_trb_addr arrays to 0x0.\n4. Configure global event buffer: write Default_Event_Ring_Array to GEVNTADRLO, 0x0 to GEVNTADRHI, 0x30 to GEVNTSIZ, 0x0 to GEVNTCOUNT.\n5. Read GCTL, then configure GCTL with 0x30c11234, GFLADJ with 0xa87f000, GUCTL with 0x2000010 via read-modify-write.\n6. Read and configure PIPE control register at base+0xc2c0 with 0x10c0002.\n7. Begin enumeration: write debug marker 0xdeadbee0, read DCFG and DSTS, write DCFG with 0x480801, DCTL with 0x80f00a00, DALEPENA with 0xff.\n8. Wait 5000 cycles for enumeration to settle.\n9. Call setup_stage() for initial setup, write GUSB2PHYCFG with 0x40002547.\n10. Check event_counter <= 0x4 and wait for interrupts via int_pend polling with wait_on(5) (two conditional loops).\n11. Wait for interrupt via int_pend polling.\n12. Call setup_stage() for GET DESCRIPTOR USB CONFIGURATION, write debug marker 0xdeadbee5.\n13. Wait for interrupt, then configure configuration descriptor response TRB (length 0x9, control 0x853) and buffer data (0x003c0209, 0xe0000101, 0x00000032, 0x00000000).\n14. Write DEPCMDPAR1+0x10 with event_trb_addr, DEPCMDPAR0+0x10 with 0x0.\n15. Process SET ADDRESS: write DCFG with 0x480809, configure TRB with control 0x853.\n16. Configure device descriptor response TRB (length 0x12, control 0x853) with buffer data (0x02000012, 0x40000000, etc.), write DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10.\n17. Issue DEPCMD+0x10 with 0x506 (Start Transfer), poll until command completes.\n18. Wait for three interrupt events via int_pend polling with wait_on(5).\n19. Call status_stage() to complete control transfer status phase.\n20. Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT, write debug marker 0xdeadbee4.\n21. Configure bulk data transfer TRB with Buffer_PointerLO_1, transfer length 0x40 (64 bytes), TRB control 0x813.\n22. Write DEPCMDPAR1+0x40 with event_trb_addr, DEPCMDPAR0+0x40 with 0x0, issue DEPCMD+0x40 with 0x506 for bulk endpoint start transfer.\n23. Poll and wait for bulk transfer completion.\n24. Call finish(0) to signal test pass.",
    "Meta Test Steps / Procedure": "1. extern int int_pend;\n2. int event_counter;\n3. int test_case() {\n4. Declare variables: rd_data, wr_data, port_count, db_offset, buf_data[16], i, bulk, intr, event_comletion, j, hand_shake;\n5. Call nic_programming();\n6. Call GIC_EnableAllIRQ();\n7. for(j=0; j<20; j++) { write_reg(Buffer_PointerLO + jDWORD, 0x0); write_reg(event_trb_addr + jDWORD, 0x0); }\n8. write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);\n9. write_reg(MIZAR_USB_GEVNTADRHI, 0x0);\n10. write_reg(MIZAR_USB_GEVNTSIZ, 0x30);\n11. write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);\n12. rd_data = read_reg(MIZAR_USB_GCTL);\n13. write_reg(MIZAR_USB_GCTL, set_data(read_reg(MIZAR_USB_GCTL), 0xFFFFFFFF, 0x30c11234));\n14. write_reg(MIZAR_USB_GFLADJ, set_data(read_reg(MIZAR_USB_GFLADJ), 0xFFFFFFFF, 0xa87f000));\n15. write_reg(MIZAR_USB_GUCTL, set_data(read_reg(MIZAR_USB_GUCTL), 0xFFFFFFFF, 0x2000010));\n16. rd_data = read_reg(MIZAR_USB_BASE + 0xc2c0); // PIPE Control Register\n17. write_reg(MIZAR_USB_BASE + 0xc2c0, 0x10c0002);\n18. // Enumeration\n19. write_reg(0xA0243ffc, 0xdeadbee0); // Debug marker\n20. rd_data = read_reg(MIZAR_USB_DCFG);\n21. rd_data = read_reg(MIZAR_USB_DSTS);\n22. write_reg(MIZAR_USB_DCFG, 0x480801);\n23. write_reg(MIZAR_USB_DCTL, 0x80f00a00);\n24. write_reg(MIZAR_USB_DALEPENA, 0xff);\n25. printf(\"Buffer_PointerLO is %x\\n\", Buffer_PointerLO);\n26. wait_on(5000);\n27. Call setup_stage();\n28. write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);\n29. if(event_counter <= 0x4) { int_pend = 1; while(int_pend) { wait_on(5); } }\n30. if(event_counter <= 0x4) { int_pend = 1; while(int_pend) { wait_on(5); } }\n31. int_pend = 1; while(int_pend) { wait_on(5); }\n32. // GET DESCRIPTOR USB CONFIGURATION\n33. Call setup_stage();\n34. write_reg(0xA0243ffc, 0xdeadbee5); // Debug marker\n35. int_pend = 1; while(int_pend) { wait_on(5); }\n36. // Data stage - Configuration descriptor: 09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301\n37. write_reg(event_trb_addr, Buffer_PointerLO);\n38. write_reg(event_trb_addr + 0x8, 0x9);\n39. write_reg(event_trb_addr + 0xc, 0x853);\n40. write_reg(Buffer_PointerLO, 0x003c0209);\n41. write_reg(Buffer_PointerLO + 0x4, 0xe0000101);\n42. write_reg(Buffer_PointerLO + 0x8, 0x00000032);\n43. write_reg(Buffer_PointerLO + 0xc, 0x00000000);\n44. write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);\n45. write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);\n46. // SET ADDRESS\n47. write_reg(MIZAR_USB_DCFG, 0x480809);\n48. write_reg(event_trb_addr, Buffer_PointerLO);\n49. write_reg(event_trb_addr + 0x8, 0x0);\n50. write_reg(event_trb_addr + 0xc, 0x853);\n51. write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);\n52. write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);\n53. // GET DESCRIPTOR (Device Descriptor response)\n54. write_reg(event_trb_addr, Buffer_PointerLO);\n55. write_reg(event_trb_addr + 0x8, 0x12);\n56. write_reg(event_trb_addr + 0xc, 0x853);\n57. write_reg(Buffer_PointerLO, 0x02000012);\n58. write_reg(Buffer_PointerLO + 0x4, 0x40000000);\n59. write_reg(Buffer_PointerLO + 0x8, 0x00000000);\n60. write_reg(Buffer_PointerLO + 0xc, 0x00000000);\n61. write_reg(Buffer_PointerLO + 0x10, 0x00000100);\n62. write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);\n63. write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);\n64. write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506); // Start Transfer command\n65. rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);\n66. while(rd_data == 0x506) { wait_on(10); rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); }\n67. int_pend = 1; while(int_pend) { wait_on(5); }\n68. wait_on(5);\n69. int_pend = 1; while(int_pend) { wait_on(5); }\n70. int_pend = 1; while(int_pend) { wait_on(5); }\n71. Call status_stage();\n72. // USB_SET_CONFIGURATION_OR_RESET_TT\n73. Call setup_stage();\n74. write_reg(0xA0243ffc, 0xdeadbee4); // Debug marker\n75. // Bulk data transfer phase\n76. write_reg(event_trb_addr, Buffer_PointerLO_1);\n77. write_reg(event_trb_addr + 0x8, 0x40); // Transfer length 64 bytes\n78. write_reg(event_trb_addr + 0xc, 0x813); // TRB control for bulk\n79. write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);\n80. write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);\n81. write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506); // Start bulk transfer\n82. // Poll and wait for completion\n83. finish(0);\n84. }\n\nvoid Default_IRQHandler() {\n85. int rd_data, sysreg_rd_data, event_count;\n86. int_pend = 0;\n87. rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);\n88. rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);\n89. event_count = read_reg(MIZAR_USB_GEVNTCOUNT);\n90. event_counter = event_count;\n91. write_reg(MIZAR_USB_GEVNTCOUNT, event_count);\n92. if (rd_data && 0x80000000) {\n93. write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);\n94. }\n95. GIC_ClearIRQ(84);\n96. }\n\nvoid setup_stage() {\n97. int rd_data;\n98. write_reg(event_trb_addr, Buffer_PointerLO);\n99. write_reg(event_trb_addr + 0x8, 0x8);\n100. write_reg(event_trb_addr + 0xc, 0x823);\n101. write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);\n102. write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);\n103. write_reg(MIZAR_USB_DEPCMD, 0x506);\n104. rd_data = read_reg(MIZAR_USB_DEPCMD);\n105. while(rd_data == 0x506) { wait_on(10); rd_data = read_reg(MIZAR_USB_DEPCMD); }\n106. int_pend = 1;\n107. while(int_pend) { wait_on(5); }\n108.}\n\nvoid status_stage() {\n109. int rd_data;\n110. write_reg(event_trb_addr, Buffer_PointerLO);\n111. write_reg(event_trb_addr + 0x8, 0x0);\n112. write_reg(event_trb_addr + 0xc, 0x843);\n113. write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);\n114. write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);\n115. write_reg(MIZAR_USB_DEPCMD, 0x506);\n116. rd_data = read_reg(MIZAR_USB_DEPCMD);\n117. while(rd_data == 0x506) { wait_on(10); rd_data = read_reg(MIZAR_USB_DEPCMD); }\n118. int_pend = 1;\n119. while(int_pend) { wait_on(5); }\n120. wait_on(5);\n121.}\n\nvoid set_configuration(int trb_address, int parameter0, int parameter1, int cmd) {\n122. int read_data;\n123. write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);\n124. write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);\n125. write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);\n126. read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);\n127. while(read_data == cmd) { wait_on(30); read_data = read_reg(MIZAR_USB_DEPCMD + trb_address); }\n128.}",
    "Meta Impacted Registers": "GCTL, GUCTL, DEVTEN, DSTS, DCTL, DALEPENA",
    "Impacted Registers": "GCTL, GUCTL, DEVTEN, DSTS, DCTL, DALEPENA",
    "Meta Validation / Acceptance Criteria": "1. DEPCMD polling: After writing 0x506 to MIZAR_USB_DEPCMD (and MIZAR_USB_DEPCMD+0x10, MIZAR_USB_DEPCMD+0x40), poll until register value changes from 0x506, indicating command accepted/completed.\n2. event_counter check: if(event_counter <= 0x4) \u2014 guard condition to wait for pending interrupts before proceeding.\n3. Default_IRQHandler: event_count = read_reg(MIZAR_USB_GEVNTCOUNT); event_counter = event_count; write_reg(MIZAR_USB_GEVNTCOUNT, event_count) \u2014 reads and acknowledges event count.\n4. Default_IRQHandler: rd_data && 0x80000000 check on MIZAR_LSS_SYSREG_RAW_STCR0 to verify USB interrupt (bit 31). Note: uses logical AND (&&) instead of bitwise AND (&).\n5. int_pend polling loops with wait_on(5) \u2014 interrupt handler clears int_pend to 0 to signal event completion.\n6. set_configuration() polls MIZAR_USB_DEPCMD + trb_address until read_data != cmd, with wait_on(30) between reads.\n7. finish(0) \u2014 signals test pass (return value 0).",
    "Validation / Acceptance Criteria": "1. All DEPCMD endpoint commands (0x506) must complete successfully \u2014 the polled register value must change from the command value.\n2. Event counter must be properly updated by the interrupt handler after each USB event.\n3. All interrupt-pending wait loops must be resolved by the interrupt handler clearing int_pend.\n4. The setup_stage(), status_stage(), and set_configuration() functions must each complete their DEPCMD commands and interrupt waits successfully.\n5. The bulk data transfer on the bulk endpoint (DEPCMD+0x40) must complete successfully with 64-byte transfer length.\n6. The test must complete with finish(0) indicating pass.",
    "Remarks": "The USB FS Device Bulk Transfer test operates in device mode and follows a DWC3-style USB device controller programming model. Key differences from the Isochronous and Interrupt transfer test variants: (1) wait_on(5) is used instead of wait_on(100) for int_pend polling, indicating tighter polling for bulk transfers. (2) The bulk data transfer TRB uses control word 0x813 (vs 0x815 for interrupt, 0x869 for isochronous). (3) The bulk endpoint uses DEPCMDPAR1+0x40 / DEPCMD+0x40 offset (vs 0x20 for interrupt, 0x70 for isochronous). (4) Transfer length for bulk data is 0x40 (64 bytes), consistent with USB full-speed bulk max packet size. The Default_IRQHandler contains a potential bug: uses logical AND (&&) instead of bitwise AND (&) when checking bit 31 of MIZAR_LSS_SYSREG_RAW_STCR0. The variable event_comletion is misspelled in the source (missing 'p'). MIZAR_USB_DEPCMD, MIZAR_USB_DEPCMDPAR0, MIZAR_USB_DEPCMDPAR1, MIZAR_USB_GEVNTADRLO, MIZAR_USB_GEVNTADRHI, MIZAR_USB_GEVNTSIZ, MIZAR_USB_GEVNTCOUNT, MIZAR_USB_DCFG, MIZAR_USB_GUSB2PHYCFG, MIZAR_LSS_SYSREG_MSK_STS0, and MIZAR_LSS_SYSREG_RAW_STCR0 are unresolved in Agent 4 mappings. A commented-out status_stage() call and commented-out int_pend wait loops exist in the source, suggesting iterative development. Debug marker writes to 0xA0243ffc with values 0xdeadbee0, 0xdeadbee3, 0xdeadbee4, 0xdeadbee5 are used as checkpoint indicators throughout the test flow."
  }
]

# ============================================================
# CREATE WORKBOOK
# ============================================================
wb = openpyxl.Workbook()

# TestPlan Sheet
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# MetaData Sheet
ws_md = wb.create_sheet("MetaData")

md_columns = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Formatting
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='0070C0', end_color='0070C0', fill_type='solid')
header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_align = Alignment(vertical='top', wrap_text=True)

# Write TestPlan headers
for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_align

# Write MetaData headers
for col_idx, col_name in enumerate(md_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_align

# Populate TestPlan
tp_field_map = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria"
]

for row_idx, item in enumerate(json_data, 2):
    for col_idx, field in enumerate(tp_field_map, 1):
        val = item.get(field, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = cell_align
    ws_tp.cell(row=row_idx, column=len(tp_columns), value="").alignment = cell_align

# Populate MetaData
md_field_map = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

for row_idx, item in enumerate(json_data, 2):
    for col_idx, field in enumerate(md_field_map, 1):
        val = item.get(field, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = cell_align

# Freeze panes
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# Auto-size columns
def auto_size(ws, columns, max_width=60):
    for col_idx, col_name in enumerate(columns, 1):
        max_len = len(col_name)
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_len:
                            max_len = len(line)
        adjusted = min(max_len + 2, max_width)
        col_letter = get_column_letter(col_idx)
        ws.column_dimensions[col_letter].width = adjusted

auto_size(ws_tp, tp_columns, 55)
auto_size(ws_md, md_columns, 60)

# Set MetaData to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save
wb.save(filepath)
wb.close()

# Validate
wb2 = openpyxl.load_workbook(filepath)
assert "TestPlan" in wb2.sheetnames
assert "MetaData" in wb2.sheetnames
assert wb2["TestPlan"].max_row == 3
assert wb2["MetaData"].max_row == 3

for row_idx, item in enumerate(json_data, 2):
    for col_idx, field in enumerate(md_field_map, 1):
        cell_val = wb2["MetaData"].cell(row=row_idx, column=col_idx).value
        expected = item.get(field, "")
        assert cell_val == expected, f"Mismatch: {field}"

wb2.close()
size = os.path.getsize(filepath)
print(f"FILE: {filename}")
print(f"PATH: {filepath}")
print(f"SIZE: {size}")
print("VALIDATION: PASSED")
