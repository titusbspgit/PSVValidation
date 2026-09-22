import openpyxl
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from datetime import datetime, timezone, timedelta
import os

# Indian Standard Time
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'USB_TestPlan_{timestamp}.xlsx'

wb = openpyxl.Workbook()

# ============ TestPlan Sheet ============
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# Headers
tp_headers = [
    'Index', 'SS / Module', 'Test Case Name', 'Feature',
    'Test Description', 'Test Steps / Procedure',
    'Impacted Registers', 'Validation / Acceptance Criteria', 'Remarks'
]

# Header styling
header_font = Font(name='Calibri', bold=True, size=11, color='FFFFFF')
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
thin_border = Border(
    left=Side(style='thin'), right=Side(style='thin'),
    top=Side(style='thin'), bottom=Side(style='thin')
)
cell_alignment = Alignment(vertical='top', wrap_text=True)

for col_idx, header in enumerate(tp_headers, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Row 1 - USB_FS_Device_Bulk_Transfer_test
tp_row1 = [
    1,
    'USB',
    'USB_FS_Device_Bulk_Transfer_test',
    'FS Device Bulk Transfer',
    'This test validates USB Full-Speed Device mode Bulk Transfer on the DWC USB3 controller. It performs a controller soft reset via DCTL and polls for completion. The USB2 PHY is configured via GUSB2PHYCFG. Event buffers are set up using GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, and GEVNTCOUNT. The controller is placed in device mode by configuring GCTL, and device parameters are set via DCFG and DEVTEN. The user control register GUCTL is configured. Multiple endpoints are configured and started using DEPCMDPAR0, DEPCMDPAR1, and DEPCMD with polling for command completion. Active endpoints are enabled via DALEPENA. System-level interrupts are enabled. The test then performs USB enumeration including GET_DESCRIPTOR (device and configuration), SET_ADDRESS, and SET_CONFIGURATION control transfers through setup, data, and status stages. After enumeration, bulk OUT transfers are initiated on two endpoints by preparing Transfer Request Blocks and issuing start transfer commands. An interrupt handler services USB events by reading event count from GEVNTCOUNT, acknowledging events, and clearing system-level interrupt status. The test verifies successful completion of the full enumeration and bulk transfer sequence.',
    '1. Initialize the NIC subsystem and enable all GIC interrupts.\n2. Clear the data buffer and event TRB memory regions by writing zeros to 20 entries.\n3. Perform a soft reset of the USB controller by writing to DCTL and polling until the reset completes.\n4. Configure the USB2 PHY via GUSB2PHYCFG.\n5. Set up the event buffer by writing the event ring base address to GEVNTADRLO and GEVNTADRHI, the event buffer size to GEVNTSIZ, and clearing GEVNTCOUNT.\n6. Read and configure GCTL to set the port direction to device mode.\n7. Configure the device configuration register DCFG and enable device events via DEVTEN.\n8. Read and configure the user control register GUCTL.\n9. Issue endpoint configuration commands for 9 endpoints using DEPCMDPAR0, DEPCMDPAR1, and DEPCMD, polling each command for completion.\n10. Allocate TX resources for 8 endpoints by writing to DEPCMDPAR0 and DEPCMD in a loop, polling each for completion.\n11. Enable physical endpoints 0 and 1 via DALEPENA, then start the controller via DCTL.\n12. Enable system-level interrupts for USB.\n13. Wait for link state connect and reset events via interrupt-driven polling.\n14. Read the device status register DSTS and reconfigure the device. Enable all endpoints via DALEPENA.\n15. Execute the setup stage of the control transfer by preparing a TRB and issuing a start transfer command via DEPCMD, polling for completion.\n16. Perform SET_ADDRESS by updating DCFG and issuing a control transfer.\n17. Poll a handshake register until the host acknowledges.\n18. Execute full USB enumeration including GET_DESCRIPTOR (device and configuration descriptors) and SET_CONFIGURATION, each involving setup, data, and status stages with TRB preparation and DEPCMD polling.\n19. Poll a second handshake register until the host acknowledges.\n20. Initiate bulk OUT transfers on two endpoints by preparing TRBs pointing to a data buffer and issuing start transfer commands via DEPCMD, polling for completion and waiting for transfer complete interrupts.\n21. Verify the interrupt handler correctly reads event count from GEVNTCOUNT, acknowledges events, clears system interrupt status, and clears the GIC IRQ.\n22. Confirm the test completes successfully via finish.',
    'DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1',
    '1. The controller soft reset completes successfully, confirmed by polling DCTL until the expected reset-done value is read.\n2. All endpoint configuration and start transfer commands complete successfully, confirmed by polling DEPCMD until the command active bit clears.\n3. USB device events (connect, reset, enumeration) are properly received and acknowledged through the interrupt handler, which reads GEVNTCOUNT and writes it back.\n4. System-level interrupt status is correctly cleared in the interrupt handler.\n5. Handshake registers are polled until the host acknowledges, confirming host-device synchronization.\n6. Full USB enumeration completes including GET_DESCRIPTOR (device and configuration) and SET_CONFIGURATION control transfers.\n7. Bulk OUT transfers on two endpoints complete successfully with transfer complete interrupts received.\n8. The test calls finish with a pass indicator, confirming end-to-end success of the FS device bulk transfer sequence.',
    'The test uses interrupt-driven polling (int_pend flag set/cleared by IRQ handler) for synchronization between the main test flow and USB events. Multiple conditional waits based on event_counter <= 0x4 handle variable event timing. Handshake polling on two external addresses (0xa0243ff4, 0xa0243ff8) provides host-device synchronization. Progress markers (0xdeadbee0-0xdeadbee7) are written to 0xA0243ffc for external observability. Several LSS SYSREG registers (MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0) could not be mapped to the USB register specification as they belong to a separate system register block. Buffer_PointerLO and Buffer_PointerLO_1 reference RAM-based buffer regions not part of the USB register map.'
]

for col_idx, value in enumerate(tp_row1, 1):
    cell = ws_tp.cell(row=2, column=col_idx, value=value)
    cell.alignment = cell_alignment
    cell.border = thin_border

# Row 2 - USB_FS_Device_Isochronous_Transfer_test
tp_row2 = [
    2,
    'USB',
    'USB_FS_Device_Isochronous_Transfer_test',
    'FS Device Isochronous Transfer',
    'This test validates USB Full-Speed Device mode Isochronous Transfer on the DWC USB3 controller. It performs a controller soft reset via DCTL and polls for completion. The USB2 PHY is configured via GUSB2PHYCFG. Event buffers are set up using GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, and GEVNTCOUNT. The controller is placed in device mode by configuring GCTL, and device parameters are set via DCFG and DEVTEN. The user control register GUCTL is configured. Multiple endpoints are configured and started using DEPCMDPAR0, DEPCMDPAR1, and DEPCMD with polling for command completion. Active endpoints are enabled via DALEPENA. System-level interrupts are enabled. The test then performs USB enumeration including GET_DESCRIPTOR (device and configuration), SET_ADDRESS, and SET_CONFIGURATION control transfers through setup, data, and status stages. After enumeration, DSTS is polled to wait for a valid frame number. Isochronous OUT transfers are initiated on two endpoints by preparing Transfer Request Blocks with isochronous-specific control values and issuing start transfer commands with frame-number-based scheduling. Between transfers, DSTS is polled for specific frame number values to synchronize isochronous timing. An interrupt handler services USB events by reading event count from GEVNTCOUNT, acknowledging events, and clearing system-level interrupt status. The test verifies successful completion of the full enumeration and isochronous transfer sequence.',
    '1. Initialize the NIC subsystem and enable all GIC interrupts.\n2. Clear the data buffer and event TRB memory regions by writing zeros to 20 entries.\n3. Perform a soft reset of the USB controller by writing to DCTL and polling until the reset completes.\n4. Configure the USB2 PHY via GUSB2PHYCFG.\n5. Set up the event buffer by writing the event ring base address to GEVNTADRLO and GEVNTADRHI, the event buffer size to GEVNTSIZ, and clearing GEVNTCOUNT.\n6. Read and configure GCTL to set the port direction to device mode.\n7. Configure DCFG for device parameters and enable device events via DEVTEN.\n8. Read and configure the user control register GUCTL.\n9. Issue endpoint configuration commands for 9 endpoints using DEPCMDPAR0, DEPCMDPAR1, and DEPCMD, polling each command for completion.\n10. Allocate TX resources for 8 endpoints by writing to DEPCMDPAR0 and DEPCMD in a loop, polling each for completion.\n11. Enable physical endpoints 0 and 1 via DALEPENA, then start the controller via DCTL.\n12. Enable system-level interrupts for USB.\n13. Wait for link state connect and reset events via interrupt-driven polling.\n14. Read the device status register DSTS and reconfigure the device. Enable all endpoints via DALEPENA.\n15. Execute the setup stage of the control transfer by preparing a TRB and issuing a start transfer command via DEPCMD, polling for completion.\n16. Perform SET_ADDRESS by updating DCFG and issuing a control transfer.\n17. Poll a handshake register until the host acknowledges.\n18. Execute full USB enumeration including GET_DESCRIPTOR (device and configuration descriptors) and SET_CONFIGURATION, each involving setup, data, and status stages with TRB preparation and DEPCMD polling.\n19. Poll a second handshake register until the host acknowledges.\n20. Poll DSTS until a valid frame number is detected.\n21. Initiate the first isochronous OUT transfer by preparing a TRB with isochronous control flags and issuing a start transfer command via DEPCMD with frame-number scheduling, polling for completion and waiting for a transfer complete interrupt.\n22. Poll DSTS for specific frame number values to synchronize isochronous timing between transfers.\n23. Initiate the second isochronous OUT transfer on another endpoint with updated frame-number scheduling via DEPCMD, polling for completion and waiting for a transfer complete interrupt.\n24. Continue polling DSTS for the next expected frame number.\n25. Verify the interrupt handler correctly reads event count from GEVNTCOUNT, acknowledges events, clears system interrupt status, and clears the GIC IRQ.\n26. Confirm the test completes successfully via finish.',
    'DCTL; GUSB2PHYCFG; GEVNTADRLO; GEVNTADRHI; GEVNTSIZ; GEVNTCOUNT; GCTL; DCFG; DEVTEN; GUCTL; DEPCMDPAR0; DEPCMD; DALEPENA; DSTS; DEPCMDPAR1',
    '1. The controller soft reset completes successfully, confirmed by polling DCTL until the expected reset-done value is read.\n2. All endpoint configuration and start transfer commands complete successfully, confirmed by polling DEPCMD until the command active bit clears.\n3. USB device events (connect, reset, enumeration) are properly received and acknowledged through the interrupt handler, which reads GEVNTCOUNT and writes it back.\n4. System-level interrupt status is correctly cleared in the interrupt handler.\n5. Handshake registers are polled until the host acknowledges, confirming host-device synchronization.\n6. Full USB enumeration completes including GET_DESCRIPTOR (device and configuration) and SET_CONFIGURATION control transfers.\n7. DSTS is polled for valid frame numbers before and between isochronous transfers, confirming correct frame-level timing synchronization.\n8. Isochronous OUT transfers on two endpoints complete successfully with transfer complete interrupts received and correct frame-number scheduling.\n9. The test calls finish with a pass indicator, confirming end-to-end success of the FS device isochronous transfer sequence.',
    'The test uses interrupt-driven polling (int_pend flag set/cleared by IRQ handler) for synchronization between the main test flow and USB events. Multiple conditional waits based on event_counter <= 0x4 handle variable event timing. Handshake polling on two external addresses (0xa0243ff4, 0xa0243ff8) provides host-device synchronization. Frame number polling via DSTS with specific bit masks and shift operations is critical for isochronous transfer timing; the test waits for frame numbers 2, 3, and 4 sequentially. The isochronous TRB control value 0x869 and commands 0x20506 and 0x40506 include frame-number-based scheduling fields. Progress markers (0xdeadbee0-0xdeadbee9) are written to 0xA0243ffc for external observability. Several LSS SYSREG registers (MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0) could not be mapped to the USB register specification as they belong to a separate system register block. Buffer_PointerLO and Buffer_PointerLO_1 reference RAM-based buffer regions not part of the USB register map.'
]

for col_idx, value in enumerate(tp_row2, 1):
    cell = ws_tp.cell(row=3, column=col_idx, value=value)
    cell.alignment = cell_alignment
    cell.border = thin_border

# Column widths for TestPlan
tp_col_widths = [8, 15, 45, 35, 80, 80, 50, 80, 60]
for col_idx, width in enumerate(tp_col_widths, 1):
    ws_tp.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = width

# Freeze top row
ws_tp.freeze_panes = 'A2'

# ============ MetaData Sheet (veryHidden) ============
ws_meta = wb.create_sheet('MetaData')
ws_meta.sheet_state = 'veryHidden'

meta_headers = [
    'Index', 'SS / Module', 'Test Case Name', 'Feature',
    'Meta Headers', 'Meta Macros', 'Meta Arrays',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset',
    'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Test Description', 'Test Steps / Procedure',
    'Impacted Registers', 'Validation / Acceptance Criteria', 'Remarks'
]

for col_idx, header in enumerate(meta_headers, 1):
    cell = ws_meta.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# MetaData Row 1
meta_row1 = [
    1,
    'USB',
    'USB_FS_Device_Bulk_Transfer_test',
    'FS Device Bulk Transfer',
    '<stdio.h>; <stdlib.h>; "usb.h"',
    'NA',
    'buf_data[16]',
    'NA',
    'NA',
    'NA',
    'NA',
    'This testcase validates USB Full-Speed Device mode Bulk Transfer operation on the DWC USB3 controller. The test_case() function begins by calling nic_programming() and GIC_EnableAllIRQ() for system initialization. It then clears 20 DWORD entries at Buffer_PointerLO and event_trb_addr memory regions. A soft reset is performed by writing 0x40f00000 to MIZAR_USB_DCTL and polling until MIZAR_USB_DCTL reads 0xf00000. The USB2 PHY is configured by writing 0x40002407 to MIZAR_USB_GUSB2PHYCFG. Event buffer setup is done by writing Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO, 0x0 to MIZAR_USB_GEVNTADRHI, 0x30 to MIZAR_USB_GEVNTSIZ, and 0x0 to MIZAR_USB_GEVNTCOUNT. MIZAR_USB_GCTL is read-modify-written with 0x30c12214 to set port direction to device mode. MIZAR_USB_DCFG is configured with 0x480801, MIZAR_USB_DEVTEN with 0x1f to enable device events, and MIZAR_USB_GUCTL is read-modify-written with 0xa400010. Endpoint configuration is performed via set_configuration() calls for 9 endpoints using MIZAR_USB_DEPCMDPAR1, MIZAR_USB_DEPCMDPAR0, and MIZAR_USB_DEPCMD. TX resource allocation is done in a loop for 8 endpoints. Physical endpoints 0 and 1 are enabled via MIZAR_USB_DALEPENA with 0x3. MIZAR_USB_DCTL is written with 0x80f00000 to run the controller. MIZAR_LSS_SYSREG_INTR_EN0 is written with 0x80000000. After enumeration, bulk transfer TRBs are prepared at event_trb_addr pointing to Buffer_PointerLO_1 with size 0x40 and control 0x813, issued via MIZAR_USB_DEPCMD+0x40 and MIZAR_USB_DEPCMD+0x50. The Default_IRQHandler() reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, reads MIZAR_USB_GEVNTCOUNT, writes it back to acknowledge, and clears GIC IRQ 84.',
    '1. Call nic_programming() for NIC initialization. 2. Call GIC_EnableAllIRQ() to enable all IRQs. 3. Clear 20 DWORD entries at Buffer_PointerLO and event_trb_addr by writing 0x0 in a loop (j=0 to 19). 4. Write 0x40f00000 to MIZAR_USB_DCTL for soft reset. 5. Poll MIZAR_USB_DCTL with wait_on(5) until read value equals 0xf00000. 6. Write 0x40002407 to MIZAR_USB_GUSB2PHYCFG. 7. Write Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO. 8. Write 0x0 to MIZAR_USB_GEVNTADRHI. 9. Write 0x30 to MIZAR_USB_GEVNTSIZ. 10. Write 0x0 to MIZAR_USB_GEVNTCOUNT. 11. Read MIZAR_USB_GCTL, then write 0x30c12214. 12. Read MIZAR_USB_DCFG, then write 0x480801. 13. Write 0x1f to MIZAR_USB_DEVTEN. 14. Read MIZAR_USB_GUCTL, then write 0xa400010. 15. Call set_configuration() 9 times with various endpoint parameters. 16. Loop i=0 to 7: write 0x1 to MIZAR_USB_DEPCMDPAR0+(i*0x10), write 0x402 to MIZAR_USB_DEPCMD+(i*0x10), poll until completion. 17. Write 0x3 to MIZAR_USB_DALEPENA. 18. Write 0x80f00000 to MIZAR_USB_DCTL. 19. Write 0x80000000 to MIZAR_LSS_SYSREG_INTR_EN0. 20-31. Enumeration sequence with setup_stage/data/status_stage, SET_ADDRESS, GET_DESCRIPTOR, SET_CONFIGURATION. 32-38. Bulk transfer TRBs prepared and issued via DEPCMD+0x40 and DEPCMD+0x50. 39. Call finish(0). 40. Default_IRQHandler: read MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0; read/write MIZAR_USB_GEVNTCOUNT; clear GIC IRQ 84.',
    'Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; Buffer_PointerLO_1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0',
    '1. Soft reset validation: MIZAR_USB_DCTL polled after writing 0x40f00000 until read value equals 0xf00000. 2. Endpoint command completion: Each DEPCMD write followed by polling until command clears. 3. Interrupt-driven event handling: int_pend cleared by IRQ handler, GEVNTCOUNT read and written back. 4. Handshake polling: 0xa0243ff4 and 0xa0243ff8 polled until non-zero. 5. Progress markers: 0xdeadbee0-0xdeadbee7 written to 0xA0243ffc. 6. Test completion: finish(0) called.',
    tp_row1[4],  # Test Description
    tp_row1[5],  # Test Steps
    tp_row1[6],  # Impacted Registers
    tp_row1[7],  # Validation
    tp_row1[8],  # Remarks
]

for col_idx, value in enumerate(meta_row1, 1):
    cell = ws_meta.cell(row=2, column=col_idx, value=value)
    cell.alignment = cell_alignment
    cell.border = thin_border

# MetaData Row 2
meta_row2 = [
    2,
    'USB',
    'USB_FS_Device_Isochronous_Transfer_test',
    'FS Device Isochronous Transfer',
    '<stdio.h>; <stdlib.h>; "usb.h"',
    'NA',
    'buf_data[16]',
    'NA',
    'NA',
    'NA',
    'NA',
    'This testcase validates USB Full-Speed Device mode Isochronous Transfer operation on the DWC USB3 controller. The test_case() function begins by calling nic_programming() and GIC_EnableAllIRQ() for system initialization. It then clears 20 DWORD entries at Buffer_PointerLO and event_trb_addr memory regions. A soft reset is performed by writing 0x40f00000 to MIZAR_USB_DCTL and polling until MIZAR_USB_DCTL reads 0xf00000. The USB2 PHY is configured by writing 0x40002407 to MIZAR_USB_GUSB2PHYCFG. Event buffer setup is done similarly. After enumeration, MIZAR_USB_DSTS is polled for frame number (rd_data & 0x00000FF8) != 0x0. Isochronous OUT transfer TRBs are prepared at event_trb_addr pointing to Buffer_PointerLO_1 with size 0x3ff and control 0x869, issued via MIZAR_USB_DEPCMD+0x60 with command 0x20506. MIZAR_USB_DSTS is polled for specific frame number values: ((rd_data & 0x00000038) >> 3) == 0x2, 0x3, 0x4. A second isochronous transfer is issued via MIZAR_USB_DEPCMD+0x70 with command 0x40506. The Default_IRQHandler() reads MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, reads MIZAR_USB_GEVNTCOUNT, writes it back, and clears GIC IRQ 84.',
    '1. Call nic_programming() for NIC initialization. 2. Call GIC_EnableAllIRQ() to enable all IRQs. 3. Clear 20 DWORD entries at Buffer_PointerLO and event_trb_addr by writing 0x0 in a loop (j=0 to 19). 4. Write 0x40f00000 to MIZAR_USB_DCTL for soft reset. 5. Poll MIZAR_USB_DCTL with wait_on(100) until read value equals 0xf00000. 6. Write 0x40002407 to MIZAR_USB_GUSB2PHYCFG. 7-10. Event buffer setup. 11-14. GCTL, DCFG, DEVTEN, GUCTL configuration. 15-16. Endpoint configuration and TX resource allocation. 17-19. DALEPENA, DCTL run, sysreg interrupt enable. 20-31. Enumeration sequence. 32. Poll 0xa0243ff8 until non-zero. 33. Read MIZAR_USB_DSTS; poll until (rd_data & 0x00000FF8) != 0x0. 34-35. First isochronous OUT TRB with control 0x869 via DEPCMD+0x60 with 0x20506. 36-38. Poll DSTS for frame numbers 2, 3. 39-40. Second isochronous OUT TRB via DEPCMD+0x70 with 0x40506. 41. Poll DSTS for frame number 4. 42. Call finish(0). 43. Default_IRQHandler: read MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0; read/write MIZAR_USB_GEVNTCOUNT; clear GIC IRQ 84.',
    'Buffer_PointerLO; event_trb_addr; MIZAR_USB_DCTL; MIZAR_USB_GUSB2PHYCFG; MIZAR_USB_GEVNTADRLO; MIZAR_USB_GEVNTADRHI; MIZAR_USB_GEVNTSIZ; MIZAR_USB_GEVNTCOUNT; MIZAR_USB_GCTL; MIZAR_USB_DCFG; MIZAR_USB_DEVTEN; MIZAR_USB_GUCTL; MIZAR_USB_DEPCMDPAR0; MIZAR_USB_DEPCMD; MIZAR_USB_DALEPENA; MIZAR_LSS_SYSREG_INTR_EN0; 0xA0243ffc; MIZAR_USB_DSTS; MIZAR_USB_DEPCMDPAR1; 0xa0243ff4; 0xa0243ff8; Buffer_PointerLO_1; MIZAR_LSS_SYSREG_MSK_STS0; MIZAR_LSS_SYSREG_RAW_STCR0',
    '1. Soft reset validation: MIZAR_USB_DCTL polled after writing 0x40f00000 until read value equals 0xf00000. 2. Endpoint command completion: Each DEPCMD write followed by polling until command clears. 3. Interrupt-driven event handling: int_pend cleared by IRQ handler, GEVNTCOUNT read and written back. 4. Handshake polling: 0xa0243ff4 and 0xa0243ff8 polled until non-zero. 5. Frame number polling: DSTS polled with mask 0x00000FF8 for initial valid frame, then 0x00000038 shifted right by 3 for values 0x2, 0x3, 0x4. 6. Progress markers: 0xdeadbee0-0xdeadbee9 written to 0xA0243ffc. 7. Test completion: finish(0) called.',
    tp_row2[4],  # Test Description
    tp_row2[5],  # Test Steps
    tp_row2[6],  # Impacted Registers
    tp_row2[7],  # Validation
    tp_row2[8],  # Remarks
]

for col_idx, value in enumerate(meta_row2, 1):
    cell = ws_meta.cell(row=3, column=col_idx, value=value)
    cell.alignment = cell_alignment
    cell.border = thin_border

# Save
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
wb.save(output_path)
print(f'GENERATED:{filename}')
print(f'PATH:{output_path}')
