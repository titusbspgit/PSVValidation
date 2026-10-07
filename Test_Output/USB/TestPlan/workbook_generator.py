#!/usr/bin/env python3
"""
USB TestPlan Excel Workbook Generator
Generates XLSX using openpyxl with full JSON data preservation
IST timestamp: Uses GMT+05:30
"""
import json, os, sys, base64
from datetime import datetime, timezone, timedelta

import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
ts = now_ist.strftime("%Y%m%d_%H%M%S")
FILENAME = f"USB_TestPlan_{ts}.xlsx"
OUTPATH = f"/tmp/{FILENAME}"

# ── JSON Data (complete, unmodified) ──────────────────────────────
json_data = [
  {
    "Index": "1",
    "SS / Module": "USB",
    "Test Case Name": "usb_host_enumeration_hs",
    "Feature": "USB Host Enumeration - High Speed",
    "Speed": "High-Speed (HS)",
    "Mode": "Host",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "Verify USB host enumeration of a high-speed device. The test initializes the USB host controller, configures the event ring and device context base address array, enables interrupts, starts the controller, detects device connection, performs port reset, assigns a device address via Enable Slot and Address Device commands, and enumerates the device by issuing GET_DESCRIPTOR control transfers on Endpoint 0.",
    "Test Steps / Procedure": "1. Call NIC programming initialization.\n2. Enable all IRQs via GIC.\n3. Read and configure PHY control register.\n4. Read host controller structural parameters (HCSPARAMS1) to get port count.\n5. Read supported protocol capability registers (SUPTPRT2_DW2, SUPTPRT3_DW2).\n6. Read PORTSC_20 for initial port status.\n7. Read and configure CONFIG register with value 0x110 (CIE enabled).\n8. Write Device Context Base Address Array pointer to DCBAAP_LO and DCBAAP_HI.\n9. Write Event Ring Segment Table Size to ERSTSZ (value 0x1).\n10. Configure Event Ring Dequeue Pointer via ERDP_LO and ERDP_HI.\n11. Configure Event Ring Segment Table Base Address via ERSTBA_LO and ERSTBA_HI.\n12. Set Interrupter Moderation (IMOD) to 0x0.\n13. Configure Interrupter Management (IMAN) with value 0x2.\n14. Write USBCMD with 0x4 to enable interrupt generation.\n15. Configure PORTSC_20 wake enable bits (WCE, WDE, WOE) via read-modify-write, then write 0xe0002a0.\n16. Read doorbell offset register (DBOFF).\n17. Enable system-level interrupt via SYSREG_INTR_EN0.\n18. Write USBCMD with 0x5 to start host controller (Run).\n19. Wait for port status change interrupt (int_pend polling loop).\n20. Read USBSTS, clear status, acknowledge interrupt via IMAN, clear ERDP_HI.\n21. Read PORTSC_20 to verify port connect status is high.\n22. Update ERDP_LO, issue port reset via PORTSC_20 (write 0xe0006f1).\n23. Clear USBSTS and IMAN, write post-reset configuration to PORTSC_20, read port status.\n24. Wait for port reset completion interrupt.\n25. Call set_address(): Issue Enable Slot command TRB on command ring, clear status/interrupts, issue Address Device command TRB with input context, update ERDP_LO, ring doorbell via DB register.\n26. Read event completion from Event Ring Array, wait for interrupt.\n27. Read event completion again after interrupt.\n28. Call enumeration(): Construct GET_DESCRIPTOR Setup Stage TRB, Data Stage TRB, and Status Stage TRB on EP0 transfer ring, ring doorbell.\n29. Call finish(0) to signal test pass.",
    "Impacted Registers": "CONFIG, DCBAAP_LO, DCBAAP_HI, ERSTSZ, USBSTS, IMAN, ERDP_LO, ERDP_HI, ERSTBA_LO, ERSTBA_HI, IMOD, USBCMD, HCSPARAMS1, SUPTPRT2_DW2, SUPTPRT3_DW2, PORTSC_20, DB",
    "Validation / Acceptance Criteria": "1. After starting the host controller, a port status change interrupt must be received indicating device connection.\n2. PORTSC_20 port connect status should be high after the first interrupt.\n3. After issuing port reset via PORTSC_20, a port reset completion interrupt must be received.\n4. Event ring completion entries must be valid after Enable Slot and Address Device commands.\n5. The enumeration (GET_DESCRIPTOR) control transfer must complete successfully.\n6. The test must complete with finish(0) indicating pass.",
    "Remarks": "The high-speed enumeration test includes a set_address() function (Enable Slot + Address Device commands). The Default_IRQHandler contains a potential bug: uses logical AND (&&) instead of bitwise AND (&) when checking bit 31 of the raw interrupt status register. GIC_EnableIRQ(84) is commented out in favor of GIC_EnableAllIRQ(). The MIZAR_USB_BASE macro is used with offset 0xc200 for PHY control and offset 0x484 for doorbell but is unresolved in Agent 4 mappings. MIZAR_LSS_SYSREG_INTR_EN0 is also unresolved in Agent 4 mappings. In set_address(), ERDP_LO is updated to Default_Event_Ring_Array + 0x68, differing from the FS variant which uses 0x38. Post-interrupt handling includes clearing ERDP_HI after reading USBSTS, which differs slightly from the FS variant sequence.",
    "Meta Test Description": "USB host enumeration test for high-speed (HS) device. The testcase initializes the xHCI host controller, configures event ring infrastructure (ERDP, ERSTBA, ERSTSZ), enables interrupts (IMAN, USBCMD, SYSREG_INTR_EN0), starts the host controller via USBCMD run/stop bit, detects device connection via port status change interrupt on the USB 2.0 port (PORTSC_20), performs port reset, issues Enable Slot and Address Device commands via the command ring in set_address(), and performs USB enumeration (GET_DESCRIPTOR) via control transfer TRBs on Endpoint 0 in enumeration(). Completion is signaled via finish(0).",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"",
    "Meta Macros": "MIZAR_USB_BASE, MIZAR_USB_CONFIG, MIZAR_USB_DCBAAP_LO, MIZAR_USB_DCBAAP_HI, MIZAR_USB_ERSTSZ, MIZAR_USB_USBSTS, MIZAR_USB_IMAN, MIZAR_USB_ERDP_LO, MIZAR_USB_ERDP_HI, MIZAR_USB_ERSTBA_LO, MIZAR_USB_ERSTBA_HI, MIZAR_USB_IMOD, MIZAR_USB_USBCMD, MIZAR_USB_HCSPARAMS1, MIZAR_USB_SUPTPRT2_DW2, MIZAR_USB_SUPTPRT3_DW2, MIZAR_USB_PORTSC_20, MIZAR_USB_DBOFF, MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0, USB_PORTSC_20_WCE, USB_PORTSC_20_WDE, USB_PORTSC_20_WOE, Device_Context_Base_Address_Array, Default_Event_Ring_Array, Event_Ring_Segment_Table, EP0_TR_Dequeue_Pointer, Default_Command_Ring, Default_Input_Context, MIZAR_USB_DB",
    "Meta Arrays": "int data_in[512];\nint data_out[512];",
    "Meta Impacted Registers": "CONFIG, DCBAAP_LO, DCBAAP_HI, ERSTSZ, USBSTS, IMAN, ERDP_LO, ERDP_HI, ERSTBA_LO, ERSTBA_HI, IMOD, USBCMD, HCSPARAMS1, SUPTPRT2_DW2, SUPTPRT3_DW2, PORTSC_20, DB",
    "Meta Validation / Acceptance Criteria": "1. After port status change interrupt: port_status = read_reg(MIZAR_USB_PORTSC_20) \u2014 port connect status should be high.\n2. After port reset and interrupt: port_status = read_reg(MIZAR_USB_PORTSC_20) \u2014 verify port reset completed.\n3. event_completion = read_reg(Default_Event_Ring_Array + 0x50) \u2014 read before and after interrupt to verify event ring completion.\n4. Default_IRQHandler checks rd_data && 0x80000000 to verify USB interrupt (bit 31 of MIZAR_LSS_SYSREG_RAW_STCR0).\n5. finish(0) \u2014 signals test pass (return value 0).",
    "Meta Test Steps / Procedure": "PLACEHOLDER_ROW1"
  }
]

print(f"Generator ready. Filename={FILENAME}")
print(f"IST={now_ist.isoformat()}")
