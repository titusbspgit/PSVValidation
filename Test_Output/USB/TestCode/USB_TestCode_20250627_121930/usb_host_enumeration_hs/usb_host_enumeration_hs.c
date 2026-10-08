// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_host_enumeration_hs.h"
#include "test_define.inc"

/* Global variables from DV testcase */
extern int int_pend;

/*
 * Function: psv_delay
 * Description: Deterministic busy-wait delay to replace DV wait_on().
 * Parameters:
 *   count - relative delay iteration count
 * Returns:
 *   void
 */
static void psv_delay(int count)
{
    volatile int d;
    for (d = 0; d < count * 1000; d++);
}

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler that clears int_pend, reads system register
 *              status, acknowledges USB interrupt via IMAN, clears raw interrupt
 *              status, and calls GIC_ClearIRQ(84).
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rd_data, sysreg_rd_data;

    int_pend = 0;
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    /* NOTE: Source uses logical AND (&&) not bitwise AND (&). Preserved as-is per No Silent Correction Rule. */
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_USB_IMAN, 0x1);
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }
    GIC_ClearIRQ(84);
}

/*
 * Function: set_address
 * Description: Issues Enable Slot command TRB via Default_Command_Ring,
 *              reads USBSTS for command completion, clears status and IMAN,
 *              updates ERDP_HI.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void set_address(void)
{
    int input_context_address;
    int event_completion, usb_status;

    /* slot command TRB */
    write_reg(Default_Command_Ring + 0x0, 0x0);
    write_reg(Default_Command_Ring + 0x4, 0x0);
    write_reg(Default_Command_Ring + 0x8, 0x0);
    write_reg(Default_Command_Ring + 0xc, 0x00002401);

    usb_status = read_reg(MIZAR_USB_USBSTS);
    LOGT("set_address: USBSTS after slot command TRB = 0x%x", usb_status);
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
}

/*
 * Function: enumeration
 * Description: Issues GET_DESCRIPTOR (Device) TRBs via EP0_TR_Dequeue_Pointer
 *              including setup stage and data stage.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void enumeration(void)
{
    int event_completion;

    /* GET configuration get device - setup stage */
    write_reg(EP0_TR_Dequeue_Pointer, 0x01000680);
    write_reg(EP0_TR_Dequeue_Pointer + 0x4, 0x00120000);
    write_reg(EP0_TR_Dequeue_Pointer + 0x8, 0x08);
    write_reg(EP0_TR_Dequeue_Pointer + 0xc, 0x00030861);

    /* DATA stage */
    write_reg(EP0_TR_Dequeue_Pointer + 0x10, EP0_TR_Dequeue_Pointer + 0x200);
    write_reg(EP0_TR_Dequeue_Pointer + 0x18, 0x12);
    write_reg(EP0_TR_Dequeue_Pointer + 0x1c, 0x00010c27);
}

/*
 * Function: usb_host_enumeration_hs_init
 * Description: Initializes the xHCI host controller by configuring PHY control
 *              registers, reading structural parameters, setting up DCBAA,
 *              Command Ring, Event Ring, Interrupter registers, CONFIG, and
 *              enabling system-level interrupts. Starts the host controller.
 * Parameters:
 *   cfg - pointer to test configuration structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_host_enumeration_hs_init(const TestsItem *cfg)
{
    int rd_data, port_count;

    (void)cfg;

    LOGT("USB Host Enumeration HS test init start");

    /* Step 1: Call nic_programming() for NIC initialization */
    nic_programming();

    /* Step 2: Enable all IRQs via GIC_EnableAllIRQ() */
    GIC_EnableAllIRQ();

    /* Step 3: PHY CONTROL REG - read and configure */
    rd_data = read_reg(MIZAR_USB_BASE + 0xc200);
    LOGT("PHY Control Reg read: 0x%x", rd_data);
    write_reg(MIZAR_USB_BASE + 0xc200, 0x102407);

    /* Step 4: Read HCSPARAMS1 to obtain port count */
    port_count = read_reg(MIZAR_USB_HCSPARAMS1);
    LOGT("HCSPARAMS1 (port_count): 0x%x", port_count);

    /* Step 5: Read SUPTPRT2_DW2 and SUPTPRT3_DW2 */
    rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2);
    LOGT("SUPTPRT2_DW2: 0x%x", rd_data);
    rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2);
    LOGT("SUPTPRT3_DW2: 0x%x", rd_data);

    /* Step 6: Read PORTSC_20 for initial port status */
    rd_data = read_reg(MIZAR_USB_PORTSC_20);
    LOGT("PORTSC_20 initial: 0x%x", rd_data);

    /* Step 7: Read HCSPARAMS2 and PAGESIZE */
    rd_data = read_reg(MIZAR_USB_HCSPARAMS2);
    LOGT("HCSPARAMS2: 0x%x", rd_data);
    rd_data = read_reg(MIZAR_USB_PAGESIZE);
    LOGT("PAGESIZE: 0x%x", rd_data);

    /* Step 8: Event Ring Segment Table setup */
    write_reg(Event_Ring_Segment_Table, Default_Event_Ring_Array);
    write_reg(Event_Ring_Segment_Table + DWORD, 0x0);
    write_reg(Event_Ring_Segment_Table + 2 * (DWORD), 0x20);

    /* Step 9: Device Context Base Address Array setup */
    write_reg(Device_Context_Base_Address_Array + 2 * DWORD, Device_Context_Array + 0x100);
    write_reg(Device_Context_Base_Address_Array + 3 * DWORD, 0x0);
    write_reg(Device_Context_Base_Address_Array + 4 * DWORD, Device_Context_Array + 0x0d00);
    write_reg(Device_Context_Base_Address_Array + 5 * DWORD, 0x0);

    /* Step 10: Read and write CONFIG register with 0x110 (CIE enable) */
    rd_data = read_reg(MIZAR_USB_CONFIG);
    LOGT("CONFIG read: 0x%x", rd_data);
    write_reg(MIZAR_USB_CONFIG, 0x110);

    /* Step 11: Write DCBAAP_LO and DCBAAP_HI */
    write_reg(MIZAR_USB_DCBAAP_LO, Device_Context_Base_Address_Array);
    write_reg(MIZAR_USB_DCBAAP_HI, 0x0);

    /* Step 12: Write ERSTSZ with 0x1 */
    write_reg(MIZAR_USB_ERSTSZ, 0x1);

    /* Step 13: Write CRCR_LO with Default_Command_Ring + 0x1, CRCR_HI with 0x0 */
    write_reg(MIZAR_USB_CRCR_LO, Default_Command_Ring + 0x1);
    write_reg(MIZAR_USB_CRCR_HI, 0x0);

    /* Step 14: Write CONFIG with 0x10 (MaxSlotsEn = 16) */
    write_reg(MIZAR_USB_CONFIG, 0x10);

    /* Step 15: Write ERDP_LO and ERDP_HI */
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);

    /* Step 16: Write ERSTBA_LO and ERSTBA_HI */
    write_reg(MIZAR_USB_ERSTBA_LO, Event_Ring_Segment_Table);
    write_reg(MIZAR_USB_ERSTBA_HI, 0x0);

    /* Step 17: Write IMOD, IMAN, USBCMD */
    write_reg(MIZAR_USB_IMOD, 0x0);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_USBCMD, 0x4);

    /* Step 18: Enable system interrupt */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);

    /* Step 19: Write USBCMD with 0x5 (Run/Stop = Run) */
    write_reg(MIZAR_USB_USBCMD, 0x5);

    LOGT("USB Host Enumeration HS test init complete");

    return 0;
}

/*
 * Function: usb_host_enumeration_hs_run
 * Description: Executes the USB Host Enumeration HS test flow including
 *              interrupt-driven wait for port connect, port reset, Set Address
 *              command, EP command TRB, doorbell ring, and GET_DESCRIPTOR
 *              enumeration.
 * Parameters:
 *   cfg - pointer to test configuration structure
 *   out - pointer to test output structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_host_enumeration_hs_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    int usb_status, port_status;
    int input_context_address;
    int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("USB output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB Host Enumeration HS test run start");

    /* Step 20: Set int_pend = 1, poll with wait_on(100) until interrupt clears int_pend */
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(100);
        timeout++;
        if (timeout > 10000) {
            LOGE("int_pend polling timeout (port connect wait)");
            out->status = -1;
            break;
        }
    }

    /* Step 21: Read USBSTS, clear status by writing 0x8 */
    usb_status = read_reg(MIZAR_USB_USBSTS);
    LOGT("USBSTS after port connect: 0x%x", usb_status);
    write_reg(MIZAR_USB_USBSTS, 0x8);

    /* Step 22: Write IMAN, ERDP_HI, ERDP_LO */
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x18);

    /* Step 23: Write PORTSC_20 with 0xe0006f1 (port reset) */
    write_reg(MIZAR_USB_PORTSC_20, 0xe0006f1);

    /* Step 24: Clear USBSTS (0x8), IMAN (0x2) */
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);

    /* Step 25: Write PORTSC_20 with 0xe220200, read back PORTSC_20 */
    write_reg(MIZAR_USB_PORTSC_20, 0xe220200);
    port_status = read_reg(MIZAR_USB_PORTSC_20);
    LOGT("PORTSC_20 after port reset: 0x%x", port_status);

    /* Step 26: Set int_pend = 1, poll with wait_on(100) until interrupt clears int_pend */
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(100);
        timeout++;
        if (timeout > 10000) {
            LOGE("int_pend polling timeout (post port reset)");
            out->status = -1;
            break;
        }
    }

    /* Step 27: Call set_address() which issues slot command TRB */
    set_address();

    /* Step 28: Compute input_context_address from Default_Input_Context */
    input_context_address = (Default_Input_Context) + 0x0;

    /* Step 29: Issue EP command TRB via Default_Command_Ring at offset 0x30 */
    write_reg(Default_Command_Ring + 0x30, input_context_address);
    write_reg(Default_Command_Ring + 0x34, 0x0);
    write_reg(Default_Command_Ring + 0x38, 0x0);
    write_reg(Default_Command_Ring + 0x3c, 0x1003001);

    /* Step 30: Clear USBSTS, IMAN, update ERDP, write DB (doorbell) with 0x0 */
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x58);
    write_reg(MIZAR_USB_DB, 0x0);

    /* Step 31: Call enumeration() for GET_DESCRIPTOR (Device) TRBs */
    enumeration();

    LOGT("USB Host Enumeration HS test run complete: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

/*
 * Function: usb_host_enumeration_hs_teardown
 * Description: Performs final cleanup for the USB Host Enumeration HS test.
 * Parameters:
 *   cfg - pointer to test configuration structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_host_enumeration_hs_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB Host Enumeration HS teardown: no additional cleanup required");

    return 0;
}
