#!/usr/bin/env python3
"""Generate PCIE TestPlan Excel workbook and commit it to the repository."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import os
import json
import base64

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"PCIE_TestPlan_{timestamp_str}.xlsx"

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "PCIE",
        "Test Case Name": "pcie_device_enumerate_test",
        "Feature": "Device Enumeration",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "DM0_RC; DM1_RC; DM0_EP; DM1_EP; DEBUG_DISPLAY",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs PCIe device enumeration...",
        "Test Description": "This testcase validates PCIe device enumeration by performing link training, programming coherency control registers for cache enable and disable, polling the GIC register for link status readiness, reading the device Vendor ID, configuring the command register, programming memory base addresses, writing system-level configuration registers, and performing BAR sizing and assignment on both PCIe slave interfaces. The test concludes by polling a synchronization register until a completion handshake value is received.",
        "Meta Test Steps / Procedure": "1. write_reg(0xE6004100, 0x0)...",
        "Test Steps / Procedure": "1. Initialize the synchronization register by writing zero to clear it. 2. Perform PCIe link training for the configured dual-mode controller (x4 width). 3. Program the COHERENCY_CONTROL_3_OFF register for PCIE0 by reading, setting cache-enable bit fields (bits 11-14, 3-6, 27-30, 19-22 to 0xF), and writing back. 4. Repeat coherency control cache-enable programming for PCIE1. 5. Wait for stabilization, then re-apply combined cache-enable programming for both PCIE0 and PCIE1 COHERENCY_CONTROL_3_OFF registers. 6. Repeat the link training and cache programming sequence. 7. Read the gic register on SII0 interface and poll until link status bits (mask 0xD1) indicate readiness. 8. Configure non-secure protection via NIC. 9. Poll the gic register on SII1 interface until link status bits indicate readiness. 10. Read the TYPE1_DEV_ID_VEND_ID_REG on PCIe slave 0 to retrieve the Vendor ID. 11. Write to TYPE1_STATUS_COMMAND_REG on PCIe slave 0 to enable bus master, memory space, and I/O space access. 12. Invoke memory base programming for both dual-mode controllers. 13. Write enable values to six system-level configuration registers. 14. Perform cache-disable programming on COHERENCY_CONTROL_3_OFF for both PCIE0 and PCIE1 by clearing bits 19-22 and 27-30. 15. Wait, then finalize cache-disable by clearing all cache fields in COHERENCY_CONTROL_3_OFF for both controllers. 16. Perform BAR sizing on PCIe slave 1 by writing all-ones to BAR0_REG through PREF_MEM_LIMIT_PREF_MEM_BASE_REG and reading back. 17. Assign BAR address values on PCIe slave 1 and verify by reading back. 18. Repeat BAR sizing and address assignment on PCIe slave 0. 19. Poll the synchronization register until the expected completion handshake value is received. 20. Complete the test.",
        "Meta Impacted Registers": "0xE6004100; mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF; ...",
        "Impacted Registers": "COHERENCY_CONTROL_3_OFF; gic; TYPE1_DEV_ID_VEND_ID_REG; TYPE1_STATUS_COMMAND_REG; BAR0_REG; BAR1_REG; SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG; SEC_STAT_IO_LIMIT_IO_BASE_REG; MEM_LIMIT_MEM_BASE_REG; PREF_MEM_LIMIT_PREF_MEM_BASE_REG",
        "Meta Validation / Acceptance Criteria": "1. Poll read_sii0_reg(0xC0)...",
        "Validation / Acceptance Criteria": "1. The gic register on SII0 must be polled until link status bits (mask 0xD1) are all set, confirming PCIE0 link readiness. 2. The gic register on SII1 must similarly indicate link readiness. 3. The TYPE1_DEV_ID_VEND_ID_REG must return a valid Vendor ID when read. 4. BAR sizing on both PCIe slave interfaces must return valid size masks when all-ones are written to BAR0_REG, BAR1_REG, SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, SEC_STAT_IO_LIMIT_IO_BASE_REG, MEM_LIMIT_MEM_BASE_REG, and PREF_MEM_LIMIT_PREF_MEM_BASE_REG. 5. BAR address assignment must be verified by reading back the programmed values from both slave interfaces. 6. The synchronization register must eventually return the expected completion handshake value to indicate successful enumeration. 7. The test must complete with a pass status via finish(0).",
        "Remarks": "The source contains a duplicated block of link training and cache programming code. Conditional compilation (DM0_RC, DM1_RC, DM0_EP, DM1_EP) controls which link training path is taken. The test uses two PCIe slave interfaces (slv0 and slv1) and two SII interfaces (sii0 and sii1). Six system-level registers at absolute addresses could not be mapped to named registers. The macro mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF could not be resolved. The polling on the synchronization register uses wait_on(5) delays between iterations. The non_secure_prot_nic() call configures NIC security settings before enumeration proceeds."
    }
]

print(f"Generated filename: {filename}")
print(f"Timestamp (IST): {now_ist.isoformat()}")
print("Script ready for execution")
