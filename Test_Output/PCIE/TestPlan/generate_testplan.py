#!/usr/bin/env python3
import os, sys, json
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'PCIE_TestPlan_{timestamp_str}.xlsx'
output_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(output_dir, filename)

json_data = [
  {
    "Index": "1",
    "SS / Module": "PCIE",
    "Test Case Name": "pcie_device_enumerate_test",
    "Feature": "PCIe Device Enumeration",
    "Meta Test Description": "The testcase performs PCIe device enumeration across two controller instances (DM0 and DM1). It begins by writing 0x0 to 0xE6004100 to initialize the system. It then invokes link training (link_training_dm0_x4 or link_training_dm1_x4 depending on compile-time defines DM0_RC, DM1_RC, DM0_EP, DM1_EP) with a lane width of 4. Cache programming is performed by executing multiple read-modify-write sequences on mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF and mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF using set_data() to configure bit fields [3:6], [11:14], [19:22], and [27:30] to 0xF. A wait_on(20) delay is inserted between cache programming phases. The testcase then polls read_sii0_reg(0xC0) in a while loop until (data_rd & 0xD1) == 0xD1, and similarly polls read_sii1_reg(0xC0) for the same condition, confirming link status readiness on both SII interfaces. Under DM0_RC, the Vendor ID is read from read_pcie_slv0_reg(0x0), the command register is written via write_pcie_slv0_reg(0x4, 0x7) to enable IO, Memory, and Bus Master, and memory base programming functions mem_base_program_dm0_x4() and mem_base_program_dm1_x4() are called. System-level control registers at 0xE690000C, 0xE6900010, 0xE6900014, 0xE6900018, 0xE6900030, and 0xE6900034 are written with 0x1. Cache is then disabled by performing read-modify-write on both coherency control registers setting bit fields [19:22] and [27:30] to 0x0 with a wait_on(10) delay. After a wait_on(30), BAR sizing is performed on PCIe slave port 1 by writing 0xFFFFFFFF to registers at offsets 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, reading them back, then programming actual BAR values (0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000) and reading back again. The same BAR sizing and programming sequence is repeated on PCIe slave port 0. Finally, the testcase polls 0xE6004100 in a while loop waiting for the value 0x12345678 with wait_on(5) delays between reads, and calls finish(0) upon success.",
    "Test Description": "Verifies PCIe device enumeration by performing link training, configuring coherency control registers on both PCIe controller instances, polling link status on both SII interfaces until ready, reading the device Vendor ID from TYPE1_DEV_ID_VEND_ID_REG, enabling IO space, memory space, and bus master in TYPE1_STATUS_COMMAND_REG, programming memory base addresses, configuring system-level control registers, disabling cache coherency, performing BAR sizing and address assignment on BAR0_REG, BAR1_REG, SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, SEC_STAT_IO_LIMIT_IO_BASE_REG, MEM_LIMIT_MEM_BASE_REG, and PREF_MEM_LIMIT_PREF_MEM_BASE_REG for both PCIe slave ports, and polling a synchronization register until the expected completion value is observed.",
    "Meta Test Steps / Procedure": "1. Write 0x0 to 0xE6004100 to initialize the system. 2. Invoke link_training_dm0_x4(4) or link_training_dm1_x4(4) based on compile-time defines (DM0_RC, DM1_RC, DM0_EP, DM1_EP). 3. CACHE PROGRAMMING: Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, use set_data() to set bits [11:14] to 0xF and bits [3:6] to 0xF, write back. 4. Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF again, set bits [27:30] to 0xF and bits [19:22] to 0xF, write back. 5. Repeat steps 3-4 for mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF. 6. Call wait_on(20). 7. Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, set all four bit groups [3:6], [11:14], [19:22], [27:30] to 0xF, write back. 8. Repeat step 7 for mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF. 9. Read read_sii0_reg(0xC0) and poll in while loop until (data_rd & 0xD1) == 0xD1. 10. Call non_secure_prot_nic(). 11. Read read_sii1_reg(0xC0) and poll in while loop until (data_rd & 0xD1) == 0xD1. 12. Under DM0_RC: Read Vendor ID via read_pcie_slv0_reg(0x0). 13. Write 0x7 to write_pcie_slv0_reg(0x4) to enable IO, Memory, Bus Master. 14. Call mem_base_program_dm0_x4() and mem_base_program_dm1_x4(). 15. Call wait_on(10). 16. Write 0x1 to 0xE690000C, 0xE6900010, 0xE6900014, 0xE6900018, 0xE6900030, 0xE6900034. 17. DISABLE_CACHE PROGRAMMING: Read-modify-write mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF setting bits [19:22] and [27:30] to 0x0. 18. Repeat step 17 for mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF. 19. Call wait_on(10). 20. Final cache disable: Read-modify-write both coherency control registers setting bits [19:22] and [27:30] to 0x0. 21. Call wait_on(30). 22. BAR sizing on slave port 1: Write 0xFFFFFFFF to offsets 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24 via write_pcie_slv1_reg, then read back each. 23. Program BAR values on slave port 1: Write 0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000 to offsets 0x10-0x24, then read back each. 24. Repeat steps 22-23 for slave port 0 via write_pcie_slv0_reg and read_pcie_slv0_reg. 25. Call wait_on(10). 26. Poll read_reg(0xE6004100) in while loop until value equals 0x12345678, with wait_on(5) between iterations. 27. Call finish(0) to end the test.",
    "Test Steps / Procedure": "1. Initialize the system by writing to the system control register. 2. Perform PCIe link training on the applicable controller instance with x4 lane width. 3. Enable cache coherency by performing read-modify-write on COHERENCY_CONTROL_3_OFF for both PCIe controller instances, setting all four coherency bit groups. 4. Wait for the coherency configuration to take effect. 5. Poll the SII interface 0 link status register until the link-up condition is detected. 6. Configure non-secure protection settings. 7. Poll the SII interface 1 link status register until the link-up condition is detected. 8. Read the Vendor ID from TYPE1_DEV_ID_VEND_ID_REG on PCIe slave port 0 and verify the device is present. 9. Write to TYPE1_STATUS_COMMAND_REG on PCIe slave port 0 to enable IO space, memory space, and bus master. 10. Program memory base addresses for both controller instances. 11. Configure system-level control registers to enable required subsystem functions. 12. Disable cache coherency by performing read-modify-write on COHERENCY_CONTROL_3_OFF for both PCIe controller instances, clearing the coherency bit groups. 13. Wait for cache disable to take effect. 14. Perform BAR sizing on PCIe slave port 1 by writing all-ones to BAR0_REG, BAR1_REG, SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, SEC_STAT_IO_LIMIT_IO_BASE_REG, MEM_LIMIT_MEM_BASE_REG, and PREF_MEM_LIMIT_PREF_MEM_BASE_REG, then reading back to determine BAR sizes. 15. Program actual BAR address values on PCIe slave port 1 and read back to verify. 16. Repeat BAR sizing and address programming on PCIe slave port 0. 17. Poll the synchronization register until the expected completion value is received. 18. End the test successfully.",
    "Meta Impacted Registers": "0xE6004100; mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF; mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF; 0xC0; 0x0; 0x4; 0xE690000C; 0xE6900010; 0xE6900014; 0xE6900018; 0xE6900030; 0xE6900034; 0x10; 0x14; 0x18; 0x1c; 0x20; 0x24",
    "Impacted Registers": "COHERENCY_CONTROL_3_OFF; TYPE1_DEV_ID_VEND_ID_REG; TYPE1_STATUS_COMMAND_REG; BAR0_REG; BAR1_REG; SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG; SEC_STAT_IO_LIMIT_IO_BASE_REG; MEM_LIMIT_MEM_BASE_REG; PREF_MEM_LIMIT_PREF_MEM_BASE_REG",
    "Validation / Acceptance Criteria": "The test passes when: 1. PCIe link training completes successfully on the applicable controller instance. 2. SII interface 0 and SII interface 1 link status polling completes with the expected link-up bitmask condition satisfied. 3. TYPE1_DEV_ID_VEND_ID_REG returns a valid Vendor ID confirming device presence. 4. TYPE1_STATUS_COMMAND_REG is successfully written to enable IO space, memory space, and bus master. 5. BAR sizing on both slave ports returns valid size information when all-ones are written to BAR0_REG, BAR1_REG, SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, SEC_STAT_IO_LIMIT_IO_BASE_REG, MEM_LIMIT_MEM_BASE_REG, and PREF_MEM_LIMIT_PREF_MEM_BASE_REG. 6. BAR address programming is verified by reading back the programmed values. 7. The synchronization register polling completes with the expected completion value. 8. The test terminates via finish(0) indicating success.",
    "Remarks": "The testcase uses compile-time conditional compilation (DM0_RC, DM1_RC, DM0_EP, DM1_EP) to select the applicable PCIe controller mode and instance for link training. Cache coherency is enabled before enumeration and disabled afterward. Two SII interfaces are polled for link readiness with a bitmask-based condition. BAR sizing and programming is performed on two separate PCIe slave ports (slv0 and slv1). Several system-level control registers could not be mapped to the PCIe register specification and may belong to an external subsystem. One SII link status register used for polling could not be mapped to the PCIe register specification. One coherency control macro for the second PCIe instance could not be resolved from the available headers. The test includes multiple wait delays between configuration phases. A final synchronization polling loop waits for an external completion signal before finishing."
  }
]

# TestPlan sheet columns
tp_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# MetaData sheet columns
md_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = 'TestPlan'

header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_data in json_data:
    row_values = []
    for col_name in tp_columns:
        row_values.append(row_data.get(col_name, ''))
    ws_tp.append(row_values)

for row in ws_tp.iter_rows(min_row=2, max_row=ws_tp.max_row, min_col=1, max_col=len(tp_columns)):
    for cell in row:
        cell.alignment = wrap_alignment

ws_tp.freeze_panes = 'A2'

# Auto-size columns
for col_idx in range(1, len(tp_columns) + 1):
    max_len = len(str(ws_tp.cell(row=1, column=col_idx).value))
    for row_idx in range(2, ws_tp.max_row + 1):
        val = ws_tp.cell(row=row_idx, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 4, 60)

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')

for col_idx, col_name in enumerate(md_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_data in json_data:
    row_values = []
    for col_name in md_columns:
        if col_name == 'Meta Validation / Acceptance Criteria':
            row_values.append(row_data.get('Validation / Acceptance Criteria', ''))
        else:
            row_values.append(row_data.get(col_name, ''))
    ws_md.append(row_values)

for row in ws_md.iter_rows(min_row=2, max_row=ws_md.max_row, min_col=1, max_col=len(md_columns)):
    for cell in row:
        cell.alignment = wrap_alignment

ws_md.freeze_panes = 'A2'

for col_idx in range(1, len(md_columns) + 1):
    max_len = len(str(ws_md.cell(row=1, column=col_idx).value))
    for row_idx in range(2, ws_md.max_row + 1):
        val = ws_md.cell(row=row_idx, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_md.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 4, 60)

ws_md.sheet_state = 'veryHidden'

# Save
wb.save(output_path)
print(f'SAVED:{filename}')

# Validate
assert os.path.exists(output_path), 'File does not exist'
assert os.path.getsize(output_path) > 0, 'File is empty'
wb2 = load_workbook(output_path)
assert 'TestPlan' in wb2.sheetnames, 'TestPlan sheet missing'
assert 'MetaData' in wb2.sheetnames, 'MetaData sheet missing'
print(f'VALIDATION:PASSED')
print(f'FILENAME:{filename}')
print(f'SIZE:{os.path.getsize(output_path)}')
