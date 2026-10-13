import json
import os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IP_NAME = "USB"

json_data = [
  {
    "Index": "1",
    "SS / Module": "USB",
    "Test Case Name": "usb_host_enumeration_hs",
    "Feature": "USB Host High-Speed Enumeration via xHCI Controller",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"usb.h\"",
    "Meta Macros": "#define MIZAR_USB_BASE 0xa1800000\n#define MIZAR_USB_GCTL MIZAR_USB_BASE + USB_GCTL_OFFSET\n#define MIZAR_USB_GFLADJ MIZAR_USB_BASE + USB_GFLADJ_OFFSET\n#define MIZAR_USB_GUCTL MIZAR_USB_BASE + USB_GUCTL_OFFSET\n#define MIZAR_USB_HCSPARAMS1 MIZAR_USB_BASE + USB_HCSPARAMS1_OFFSET\n#define MIZAR_USB_HCSPARAMS2 MIZAR_USB_BASE + USB_HCSPARAMS2_OFFSET\n#define MIZAR_USB_PAGESIZE MIZAR_USB_BASE + USB_PAGESIZE_OFFSET\n#define MIZAR_USB_SUPTPRT2_DW2 MIZAR_USB_BASE + USB_SUPTPRT2_DW2_OFFSET\n#define MIZAR_USB_SUPTPRT3_DW2 MIZAR_USB_BASE + USB_SUPTPRT3_DW2_OFFSET\n#define MIZAR_USB_CONFIG MIZAR_USB_BASE + USB_CONFIG_OFFSET\n#define MIZAR_USB_DCBAAP_LO MIZAR_USB_BASE + USB_DCBAAP_LO_OFFSET\n#define MIZAR_USB_DCBAAP_HI MIZAR_USB_BASE + USB_DCBAAP_HI_OFFSET\n#define MIZAR_USB_CRCR_LO MIZAR_USB_BASE + USB_CRCR_LO_OFFSET\n#define MIZAR_USB_CRCR_HI MIZAR_USB_BASE + USB_CRCR_HI_OFFSET\n#define MIZAR_USB_ERSTSZ MIZAR_USB_BASE + USB_ERSTSZ_OFFSET\n#define MIZAR_USB_ERSTBA_LO MIZAR_USB_BASE + USB_ERSTBA_LO_OFFSET\n#define MIZAR_USB_ERSTBA_HI MIZAR_USB_BASE + USB_ERSTBA_HI_OFFSET\n#define MIZAR_USB_ERDP_LO MIZAR_USB_BASE + USB_ERDP_LO_OFFSET\n#define MIZAR_USB_ERDP_HI MIZAR_USB_BASE + USB_ERDP_HI_OFFSET\n#define MIZAR_USB_IMOD MIZAR_USB_BASE + USB_IMOD_OFFSET\n#define MIZAR_USB_IMAN MIZAR_USB_BASE + USB_IMAN_OFFSET\n#define MIZAR_USB_USBCMD MIZAR_USB_BASE + USB_USBCMD_OFFSET\n#define MIZAR_USB_USBSTS MIZAR_USB_BASE + USB_USBSTS_OFFSET\n#define MIZAR_USB_PORTSC_20 MIZAR_USB_BASE + USB_PORTSC_20_OFFSET\n#define MIZAR_USB_DBOFF MIZAR_USB_BASE + USB_DBOFF_OFFSET\n#define MIZAR_USB_DB MIZAR_USB_BASE + USB_DB_OFFSET\n#define USB_GCTL_OFFSET 0xC110\n#define USB_GFLADJ_OFFSET 0xC630\n#define USB_GUCTL_OFFSET 0xC12C\n#define USB_HCSPARAMS1_OFFSET 0x4\n#define USB_HCSPARAMS2_OFFSET 0x8\n#define USB_PAGESIZE_OFFSET 0x28\n#define USB_SUPTPRT2_DW2_OFFSET 0x898\n#define USB_SUPTPRT3_DW2_OFFSET 0x8A8\n#define USB_CONFIG_OFFSET 0x58\n#define USB_DCBAAP_LO_OFFSET 0x50\n#define USB_DCBAAP_HI_OFFSET 0x54\n#define USB_CRCR_LO_OFFSET 0x38\n#define USB_CRCR_HI_OFFSET 0x3C\n#define USB_ERSTSZ_OFFSET 0x468\n#define USB_ERSTBA_LO_OFFSET 0x470\n#define USB_ERSTBA_HI_OFFSET 0x474\n#define USB_ERDP_LO_OFFSET 0x478\n#define USB_ERDP_HI_OFFSET 0x47C\n#define USB_IMOD_OFFSET 0x464\n#define USB_IMAN_OFFSET 0x460\n#define USB_USBCMD_OFFSET 0x20\n#define USB_USBSTS_OFFSET 0x24\n#define USB_PORTSC_20_OFFSET 0x420\n#define USB_DBOFF_OFFSET 0x14\n#define USB_DB_OFFSET 0x480\n#define USB_PORTSC_20_WCE 0x02000000\n#define USB_PORTSC_20_WDE 0x04000000\n#define USB_PORTSC_20_WOE 0x08000000\n#define Default_Command_Ring RAM+0x0\n#define Default_Event_Ring_Array RAM+0xd00\n#define Default_Input_Context RAM+0xe00\n#define EP0_TR_Dequeue_Pointer RAM+0x1870\n#define Device_Context_Base_Address_Array RAM+0xf00\n#define Device_Context_Array RAM+0x3940\n#define Event_Ring_Segment_Table RAM+0x1000\n#define DWORD 0x4",
    "Meta Arrays": "int data_in[512];\nint data_out[512];",
    "Speed": "High-Speed (HS)",
    "Mode": "Host",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase performs USB Host High-Speed enumeration using an xHCI-compliant controller (Mizar USB). The test_case() function is the main entry point. It calls nic_programming() for NIC/controller initialization, then configures global registers (GCTL, GFLADJ, GUCTL), PHY control register (MIZAR_USB_BASE+0xc200), and pipe control register (MIZAR_USB_BASE+0xc2c0). It reads capability registers (HCSPARAMS1, HCSPARAMS2, PAGESIZE, SUPTPRT2_DW2, SUPTPRT3_DW2, PORTSC_20, DBOFF). It configures PORTSC_20 wake enables (WCE, WDE, WOE), sets CONFIG (MaxSlotsEn and CIE), programs DCBAAP, CRCR, Event Ring Segment Table, ERSTBA, ERSTSZ, ERDP, IMOD, IMAN, and USBCMD. It enables system-level interrupts via MIZAR_LSS_SYSREG_INTR_EN0, starts the host controller (USBCMD=0x5), waits for port connect interrupt, performs port reset (PORTSC_20=0xe0006f1), calls set_address() to issue Enable Slot and Address Device commands via the Command Ring, populates Default_Input_Context with Slot Context and EP0 Context, rings the doorbell (DB=0x0), calls enumeration() to issue GET_DESCRIPTOR (Device, Device Qualifier, Configuration) and SET_CONFIGURATION commands via EP0 Transfer Ring TRBs, and uses Configure Endpoint Command. The Default_IRQHandler() clears interrupts by reading MIZAR_LSS_SYSREG_MSK_STS0 and MIZAR_LSS_SYSREG_RAW_STCR0, writing IMAN and clearing GIC IRQ 84.",
    "Test Description": "USB Host High-Speed Enumeration test that initializes the xHCI host controller, configures global and PHY registers, sets up the Event Ring, Command Ring, and Device Context data structures, enables interrupts, starts the controller, detects device connection, performs port reset, assigns a USB device address, retrieves Device Descriptor, Device Qualifier, and Configuration Descriptors, and issues SET_CONFIGURATION and Configure Endpoint commands to complete enumeration.",
    "Meta Test Steps / Procedure": "1. int test_case() { int rd_data,port_count,db_offset; int event_completion,usb_status,port_status; int input_context_address; int data_in[512],j,data_out[512],i; int count=0;\n2. nic_programming();\n3. GIC_EnableAllIRQ();\n4. rd_data = read_reg(MIZAR_USB_BASE+0xc200); //PHY CONTROL REG\n5. write_reg(MIZAR_USB_BASE+0xc200, 0x102407);\n6. port_count = read_reg(MIZAR_USB_HCSPARAMS1);\n7. rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2);\n8. rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2);\n9. rd_data = read_reg(MIZAR_USB_PORTSC_20); //PORTSC REG\n10. write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WCE, 1));\n11. write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WDE, 1));\n12. write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WOE, 1));\n13. write_reg(MIZAR_USB_PORTSC_20, 0xe0002a0);\n14. write_reg(MIZAR_USB_GCTL, set_data(read_reg(MIZAR_USB_GCTL), 0xFFFFFFFF, 0x30c11234));\n15. write_reg(MIZAR_USB_GFLADJ, set_data(read_reg(MIZAR_USB_GFLADJ), 0xFFFFFFFF, 0xa07f000));\n16. write_reg(MIZAR_USB_GUCTL, set_data(read_reg(MIZAR_USB_GUCTL), 0xFFFFFFFF, 0x2000010));\n17. rd_data = read_reg(MIZAR_USB_BASE+0xc2c0); //PIPE CONTROL REG\n18. write_reg(MIZAR_USB_BASE+0xc2c0, 0x10c0002);\n19. rd_data = read_reg(MIZAR_USB_HCSPARAMS2);\n20. rd_data = read_reg(MIZAR_USB_PAGESIZE);\n21. db_offset = read_reg(MIZAR_USB_DBOFF);\n22. rd_data = read_reg(MIZAR_USB_CONFIG);\n23. write_reg(MIZAR_USB_CONFIG, 0x10); //CONFIG MaxSlotsEn\n24. write_reg(MIZAR_USB_DCBAAP_LO, Device_Context_Base_Address_Array);\n25. write_reg(MIZAR_USB_DCBAAP_HI, 0x0);\n26. write_reg(Device_Context_Base_Address_Array+2*DWORD, Device_Context_Array+0x100);\n27. write_reg(Device_Context_Base_Address_Array+3*DWORD, 0x0);\n28. write_reg(Device_Context_Base_Address_Array+4*DWORD, Device_Context_Array+0x0D00);\n29. write_reg(Device_Context_Base_Address_Array+5*DWORD, 0x0);\n30. write_reg(MIZAR_USB_CRCR_LO, Default_Command_Ring+0x1);\n31. write_reg(MIZAR_USB_CRCR_HI, 0x0);\n32. write_reg(Event_Ring_Segment_Table, Default_Event_Ring_Array); //EVENT RING SEGMENT table\n33. write_reg(Event_Ring_Segment_Table+DWORD, 0x0);\n34. write_reg(Event_Ring_Segment_Table+2*DWORD, 0x20); //EVENT RING SEGMENT SIZE = 32\n35. write_reg(MIZAR_USB_ERSTSZ, 0x1);\n36. write_reg(MIZAR_USB_ERSTBA_LO, Event_Ring_Segment_Table);\n37. write_reg(MIZAR_USB_ERSTBA_HI, 0x0);\n38. write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array);\n39. write_reg(MIZAR_USB_ERDP_HI, 0x0);\n40. write_reg(MIZAR_USB_IMOD, 0x0);\n41. write_reg(MIZAR_USB_IMAN, 0x2);\n42. write_reg(MIZAR_USB_USBCMD, 0x4); //INTE interrupter enable\n43. rd_data = read_reg(MIZAR_USB_CONFIG);\n44. write_reg(MIZAR_USB_CONFIG, 0x110); //CONFIG CIE\n45. write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000); //Interrupt enable at sysreg\n46. write_reg(MIZAR_USB_USBCMD, 0x5); //usbcmd run/stop bit is 1, run\n47. int_pend = 1; while(int_pend) { wait_on(100); }\n48. usb_status = read_reg(MIZAR_USB_USBSTS);\n49. write_reg(MIZAR_USB_USBSTS, 0x8);\n50. write_reg(MIZAR_USB_IMAN, 0x2);\n51. write_reg(MIZAR_USB_ERDP_HI, 0x0);\n52. port_status = read_reg(MIZAR_USB_PORTSC_20); //port connect status should be high\n53. write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array+0x18);\n54. write_reg(MIZAR_USB_PORTSC_20, 0xe0006f1); //port reset\n55. write_reg(MIZAR_USB_USBSTS, 0x8);\n56. write_reg(MIZAR_USB_IMAN, 0x2);\n57. write_reg(MIZAR_USB_PORTSC_20, 0xe220200);\n58. port_status = read_reg(MIZAR_USB_PORTSC_20);\n59. int_pend = 1; while(int_pend) { wait_on(100); }\n60. set_address();\n61-176. [continued test steps including set_address(), enumeration(), Default_IRQHandler()]",
    "Test Steps / Procedure": "1. Call nic_programming() to initialize the NIC/controller.\n2. Enable all IRQs via GIC_EnableAllIRQ().\n3. Read and configure PHY Control Register with value 0x102407.\n4. Read HCSPARAMS1 to obtain port count.\n5. Read SUPTPRT2_DW2 and SUPTPRT3_DW2 for supported protocol capabilities.\n6. Read PORTSC_20 for initial port status.\n7. Enable Wake-on-Connect (WCE), Wake-on-Disconnect (WDE), and Wake-on-Over-current (WOE) on PORTSC_20.\n8. Write PORTSC_20 with 0xe0002a0.\n9. Configure GCTL with 0x30c11234 via read-modify-write.\n10. Configure GFLADJ with 0xa07f000 via read-modify-write.\n11. Configure GUCTL with 0x2000010 via read-modify-write.\n12. Read and configure Pipe Control Register with 0x10c0002.\n13. Read HCSPARAMS2, PAGESIZE, and DBOFF.\n14. Read CONFIG, then write CONFIG with 0x10 (MaxSlotsEn=16).\n15. Program DCBAAP_LO/HI with Device Context Base Address Array pointer.\n16. Populate Device Context Base Address Array entries.\n17. Program CRCR_LO/HI with Command Ring pointer (cycle bit set).\n18. Initialize Event Ring Segment Table with Default_Event_Ring_Array base, size=32.\n19. Program ERSTSZ=1, ERSTBA_LO/HI, ERDP_LO/HI, IMOD=0, IMAN=0x2.\n20. Write USBCMD=0x4 (INTE enable).\n21. Write CONFIG=0x110 (CIE enable).\n22. Enable system-level interrupt via MIZAR_LSS_SYSREG_INTR_EN0=0x80000000.\n23. Write USBCMD=0x5 (Run + INTE).\n24. Wait for port connect interrupt (int_pend polling loop).\n25. Read USBSTS, clear USBSTS=0x8, clear IMAN=0x2.\n26. Read PORTSC_20 to verify port connect status is high.\n27. Advance ERDP_LO, issue port reset via PORTSC_20=0xe0006f1.\n28. Clear USBSTS and IMAN, write PORTSC_20=0xe220200.\n29. Wait for port reset completion interrupt.\n30. Call set_address() to issue Enable Slot Command TRB (0x00002401) and Address Device Command TRB.\n31. Populate Default_Input_Context with Slot Context (0x08300000) and EP0 Context (max packet size 64, control EP type).\n32. Write Address Device Command TRB (0x01002e01) to Command Ring.\n33. Ring doorbell (DB=0x0), wait for completion, read event completion.\n34. Issue additional doorbell rings and wait for completions.\n35. Update PORTSC_20=0xe200e01.\n36. Prepare SET_CONFIGURATION: update Input Context (0x38300000, config value=0x01).\n37. Write Configure Endpoint Command TRB (0x1003001) to Command Ring.\n38. Ring doorbell, wait for completion.\n39. Ring doorbell for slot 1 (MIZAR_USB_BASE+0x484=0x1).\n40. Call enumeration() to issue GET_DESCRIPTOR (Device, 18 bytes), GET_DESCRIPTOR (Device Qualifier, 10 bytes), GET_DESCRIPTOR (Configuration, 9 bytes), GET_DESCRIPTOR (Configuration, 9 bytes), GET_DESCRIPTOR (Configuration, 60 bytes full) via EP0 Transfer Ring TRBs.",
    "Meta Impacted Registers": "GCTL, GFLADJ, GUCTL, HCSPARAMS1, HCSPARAMS2, PAGESIZE, SUPTPRT2_DW2, SUPTPRT3_DW2, CONFIG, DCBAAP_LO, DCBAAP_HI, CRCR_LO, CRCR_HI, ERSTSZ, ERSTBA_LO, ERSTBA_HI, ERDP_LO, ERDP_HI, IMOD, IMAN, USBCMD, USBSTS, PORTSC_20, DBOFF, DB",
    "Impacted Registers": "GCTL, GFLADJ, GUCTL, HCSPARAMS1, HCSPARAMS2, PAGESIZE, SUPTPRT2_DW2, SUPTPRT3_DW2, CONFIG, DCBAAP_LO, DCBAAP_HI, CRCR_LO, CRCR_HI, ERSTSZ, ERSTBA_LO, ERSTBA_HI, ERDP_LO, ERDP_HI, IMOD, IMAN, USBCMD, USBSTS, PORTSC_20, DBOFF, DB",
    "Meta Validation / Acceptance Criteria": "1. After USBCMD=0x5 and interrupt wait, read MIZAR_USB_USBSTS to confirm event interrupt (EINT bit 3) is set.\n2. After port connect interrupt, read MIZAR_USB_PORTSC_20 - port connect status (CCS, bit 0) should be high.\n3. After port reset (PORTSC_20=0xe0006f1), read MIZAR_USB_PORTSC_20 to verify port reset completion.\n4. After Address Device Command, read event_completion from Default_Event_Ring_Array+0x40 to verify successful command completion.\n5. After each doorbell ring and interrupt wait, USBSTS is read and cleared with 0x8, IMAN is cleared with 0x2, confirming interrupt acknowledgment.\n6. In Default_IRQHandler(): read MIZAR_LSS_SYSREG_MSK_STS0, read MIZAR_LSS_SYSREG_RAW_STCR0, check bit 31 (0x80000000) - if set, write IMAN=0x1 and clear MIZAR_LSS_SYSREG_RAW_STCR0=0x80000000, then GIC_ClearIRQ(84).\n7. Note: The condition in Default_IRQHandler uses logical AND (&&) instead of bitwise AND (&): if(rd_data && 0x80000000) - this evaluates true whenever rd_data is non-zero.",
    "Validation / Acceptance Criteria": "1. Port connect status (CCS bit) in PORTSC_20 must be high after device connection interrupt.\n2. USBSTS Event Interrupt (EINT) bit must be set after each command completion.\n3. Address Device Command completion event must indicate success.\n4. Configure Endpoint Command completion event must indicate success.\n5. All GET_DESCRIPTOR transfers (Device, Device Qualifier, Configuration) must complete via interrupt-driven event ring processing.\n6. Interrupt handler must correctly acknowledge USB interrupt via IMAN and clear system-level interrupt via MIZAR_LSS_SYSREG_RAW_STCR0 and GIC IRQ 84.",
    "Remarks": "Three system register macros (MIZAR_LSS_SYSREG_INTR_EN0, MIZAR_LSS_SYSREG_MSK_STS0, MIZAR_LSS_SYSREG_RAW_STCR0) are unresolved - their definitions reside in lss_sysreg_offset.h and lss_sysreg_def.h which are not in the available corpus. Seven RAM-based data structure macros (Default_Command_Ring, Default_Event_Ring_Array, Default_Input_Context, EP0_TR_Dequeue_Pointer, Device_Context_Base_Address_Array, Device_Context_Array, Event_Ring_Segment_Table) are ambiguous due to conditional RAM base address definition. The Default_IRQHandler uses logical AND (&&) instead of bitwise AND (&) in the condition check for bit 31, which may be a source code bug. The variable sysreg_rd_data is declared but never used in Default_IRQHandler."
  }
]

# Define column order for TestPlan worksheet (non-Meta fields)
testplan_columns = [
    "Index",
    "SS / Module",
    "Test Case Name",
    "Feature",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Test Description",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Remarks"
]

# Define column order for MetaData worksheet (all fields)
metadata_columns = [
    "Index",
    "SS / Module",
    "Test Case Name",
    "Feature",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Meta Test Description",
    "Test Description",
    "Meta Test Steps / Procedure",
    "Test Steps / Procedure",
    "Meta Impacted Registers",
    "Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Validation / Acceptance Criteria",
    "Remarks"
]

# Large-text columns that need a reasonable max width cap
large_text_columns = {
    "Meta Macros", "Meta Test Description", "Test Description",
    "Meta Test Steps / Procedure", "Test Steps / Procedure",
    "Meta Validation / Acceptance Criteria", "Validation / Acceptance Criteria",
    "Remarks", "Meta Headers", "Meta Arrays", "Meta Impacted Registers",
    "Impacted Registers", "Feature"
}

MAX_COL_WIDTH = 60
MIN_COL_WIDTH = 10


def auto_size_columns(ws, columns):
    """Auto-size columns with reasonable limits for large-text columns."""
    for col_idx, col_name in enumerate(columns, 1):
        col_letter = get_column_letter(col_idx)
        max_length = len(str(col_name))
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    cell_lines = str(cell.value).split("\n")
                    for line in cell_lines:
                        if len(line) > max_length:
                            max_length = len(line)
        adjusted_width = max_length + 2
        if col_name in large_text_columns:
            adjusted_width = min(adjusted_width, MAX_COL_WIDTH)
        adjusted_width = max(adjusted_width, MIN_COL_WIDTH)
        ws.column_dimensions[col_letter].width = adjusted_width


def create_worksheet(wb, sheet_name, columns, data):
    """Create and populate a worksheet with headers and data."""
    ws = wb.create_sheet(title=sheet_name)

    # Header styling
    header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    header_alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)

    # Write headers
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment

    # Data styling
    data_alignment = Alignment(vertical="top", wrap_text=True)
    data_font = Font(name="Calibri", size=11)

    # Write data rows
    for row_idx, record in enumerate(data, 2):
        for col_idx, col_name in enumerate(columns, 1):
            value = record.get(col_name, "")
            cell = ws.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = data_alignment
            cell.font = data_font

    # Freeze first row
    ws.freeze_panes = "A2"

    # Auto-size columns
    auto_size_columns(ws, columns)

    return ws


def main():
    # Generate IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")

    # Build filename
    filename = f"{IP_NAME}_TestPlan_{timestamp}.xlsx"

    # Remove existing file if present
    if os.path.exists(filename):
        os.remove(filename)

    # Create workbook
    wb = Workbook()

    # Remove default sheet
    default_sheet = wb.active
    wb.remove(default_sheet)

    # Create TestPlan worksheet
    create_worksheet(wb, "TestPlan", testplan_columns, json_data)

    # Create MetaData worksheet
    ws_meta = create_worksheet(wb, "MetaData", metadata_columns, json_data)

    # Set MetaData worksheet to veryHidden
    ws_meta.sheet_state = "veryHidden"

    # Save workbook (overwrites if exists)
    wb.save(filename)

    # Verify workbook exists
    if os.path.exists(filename):
        file_size = os.path.getsize(filename)
        print(f"SUCCESS: Workbook generated successfully.")
        print(f"Filename: {filename}")
        print(f"File size: {file_size} bytes")
        print(f"Timestamp (IST): {now_ist.strftime('%Y-%m-%d %H:%M:%S %Z')}")
    else:
        print(f"ERROR: Failed to generate workbook {filename}")

    return filename


if __name__ == "__main__":
    generated_file = main()
    print(f"Generated workbook: {generated_file}")
