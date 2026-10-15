import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import json
import os

META_JSON = [
    {
        "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
        "IP Name": "NA",
        "Feature Information": "NA",
        "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"",
        "Meta Macros": "Buffer_PointerLO\nDWORD\nevent_trb_addr\nMIZAR_USB_DCFG\nMIZAR_USB_DSTS\nMIZAR_USB_DCTL\nMIZAR_USB_DALEPENA\nMIZAR_USB_DEPCMDPAR1\nMIZAR_USB_DEPCMDPAR0\nMIZAR_USB_DEPCMD",
        "Meta Arrays": "int buf_data[16];",
        "Meta Types": "int rd_data;\nint wr_data;\nint port_count;\nint db_offset;\nint buf_data[16];\nint i;\nint bulk;\nint intr;\nint event_comletion;\nint j;\nint hand_shake;\nextern int int_pend;\nint event_counter;",
        "Meta Test Description": "int test_case() declared with return type int. File includes stdio.h, stdlib.h, usb.h. Declares extern int int_pend. Declares int event_counter. Declares void Default_IRQHandler(). Declares void setup_stage(). Declares int test_case().",
        "Meta Test Steps / Procedure": "1. nic_programming();\n2. GIC_EnableAllIRQ();\n3. for(j=0; j<20; j++) { write_reg(Buffer_PointerLO + j*DWORD, 0x0); write_reg(event_trb_addr + j*DWORD, 0x0); }\n4. write_reg(0xA0243ffc, 0xdeadbee0);\n5. rd_data = read_reg(MIZAR_USB_DCFG);\n6. rd_data = read_reg(MIZAR_USB_DSTS);\n7. write_reg(MIZAR_USB_DCFG, 0x480801);\n8. write_reg(MIZAR_USB_DCTL, 0x80f00a00);\n9. write_reg(MIZAR_USB_DALEPENA, 0xff);\n10. write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);\n11. write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);\n12. write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);\n13. rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); while(rd_data == 0x506) { wait_on(10); rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); }\n14. int_pend = 1; while(int_pend) { wait_on(5); }\n15. status_stage();\n16. setup_stage();\n17. write_reg(0xA0243ffc, 0xdeadbee3);\n18. write_reg(event_trb_addr, Buffer_PointerLO);\n19. write_reg(event_trb_addr + 0x8, 0x9);\n20. write_reg(event_trb_addr + 0xc, 0x853);\n21. write_reg(Buffer_PointerLO, 0x003c0209);\n22. write_reg(Buffer_PointerLO + 0x4, 0xe0000101);\n23. write_reg(Buffer_PointerLO + 0x8, 0x00000032);\n24. write_reg(Buffer_PointerLO + 0xc, 0x00000000);\n25. int_pend = 1; while(int_pend) { wait_on(5); }\n26. write_reg(0xA0243ffc, 0xdeadbee4);\n27. setup_stage();\n28. write_reg(0xA0243ffc, 0xdeadbee5);",
        "Meta Impacted Registers": "HCSPARAMS1\nGCTL\nGUCTL\nDALEPENA",
        "Meta Validation / Acceptance Criteria": "1. rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); while(rd_data == 0x506) { wait_on(10); rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10); } — polling loop validates DEPCMD+0x10 value changes from 0x506 to confirm endpoint command completion.\n2. int_pend = 1; while(int_pend) { wait_on(5); } — polling loop validates int_pend is cleared by Default_IRQHandler to confirm interrupt event received. This pattern is repeated multiple times in the test.\n3. write_reg(0xA0243ffc, 0xdeadbee0) — debug marker written to validate test progress at enumeration start.\n4. write_reg(0xA0243ffc, 0xdeadbee3) — debug marker written to validate test progress at GET DESCRIPTOR USB CONFIGURATION.\n5. write_reg(0xA0243ffc, 0xdeadbee4) — debug marker written to validate test progress at SET CONFIGURATION or RESET TT.\n6. write_reg(0xA0243ffc, 0xdeadbee5) — debug marker written to validate test progress at subsequent GET DESCRIPTOR USB CONFIGURATION."
    }
]

TESTPLAN_JSON = [
    {
        "Index": "1",
        "SS / Module": "NA",
        "Test Case Name": "USB_FS_Device_Bulk_Transfer_test",
        "Feature": "NA",
        "Test Description": "This testcase verifies USB Full-Speed device-mode bulk transfer functionality. The test configures the USB device controller, performs device enumeration including setup and status stages, configures endpoint parameters, initiates bulk data transfers using Transfer Request Blocks (TRBs), and validates successful completion through interrupt-driven event handling. The expected behavior is that the device controller completes endpoint commands, processes interrupt events correctly, and successfully handles USB descriptor requests and configuration stages during enumeration and bulk transfer operations.",
        "Test Steps / Procedure": "1. Perform NIC programming initialization.\n2. Enable all IRQs via the GIC.\n3. Clear the data buffer and event TRB memory regions by writing zeros to all entries.\n4. Write a debug marker to indicate enumeration start.\n5. Read the Device Configuration register to capture initial state.\n6. Read the Device Status register to capture current device status.\n7. Configure the Device Configuration register for Full-Speed operation.\n8. Configure the Device Control register to initiate device operation.\n9. Enable all relevant endpoints via the Device Active Endpoint Enable register.\n10. Configure endpoint command parameters for event TRB handling.\n11. Issue an endpoint command and poll for command completion.\n12. Wait for an interrupt event to confirm the endpoint command was processed.\n13. Execute the status stage of the control transfer.\n14. Execute the setup stage for the next control transfer.\n15. Write a debug marker to indicate GET DESCRIPTOR USB CONFIGURATION phase.\n16. Prepare the event TRB with buffer pointer and transfer parameters.\n17. Populate the data buffer with USB configuration descriptor data.\n18. Wait for an interrupt event to confirm descriptor transfer completion.\n19. Write a debug marker to indicate SET CONFIGURATION or RESET TT phase.\n20. Execute the setup stage for the subsequent control transfer.\n21. Write a debug marker to indicate completion of the subsequent GET DESCRIPTOR USB CONFIGURATION phase.",
        "Impacted Registers": "HCSPARAMS1, GCTL, GUCTL, DALEPENA",
        "Validation / Acceptance Criteria": "1. Endpoint command completion is confirmed by polling the endpoint command register until the command active bit clears, indicating the controller has successfully processed the command.\n2. Interrupt events are correctly received and handled by the IRQ handler, confirmed by the interrupt pending flag being cleared after each expected event.\n3. Debug markers are written at key test milestones (enumeration start, GET DESCRIPTOR USB CONFIGURATION, SET CONFIGURATION or RESET TT, and subsequent GET DESCRIPTOR USB CONFIGURATION) to validate sequential test progress.\n4. The USB device controller successfully transitions through Full-Speed enumeration stages including setup, status, and data phases.\n5. Bulk transfer TRBs are correctly configured and processed, with successful completion indicated by interrupt event generation.",
        "Remarks": "NA"
    }
]

print("Script loaded successfully")
print(f"META_JSON entries: {len(META_JSON)}")
print(f"TESTPLAN_JSON entries: {len(TESTPLAN_JSON)}")
