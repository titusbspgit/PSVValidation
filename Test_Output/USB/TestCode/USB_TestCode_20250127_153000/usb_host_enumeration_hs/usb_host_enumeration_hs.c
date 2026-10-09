// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_host_enumeration_hs.h"
#include "test_define.inc"

// Forward declaration for nic_programming
void nic_programming(void);

// External interrupt pending flag
extern volatile int int_pend;

static int g_errors = 0;

/*
 * Function: set_address
 * Description: Issues Enable Slot command TRB via Default_Command_Ring,
 *              reads USBSTS for command completion, clears status.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void set_address(void)
{
    int input_context_address;
    int event_completion, usb_status;

    // slot command TRB
    write_reg(Default_Command_Ring + 0x0, 0x0);
    write_reg(Default_Command_Ring + 0x4, 0x0);
    write_reg(Default_Command_Ring + 0x8, 0x0);
    write_reg(Default_Command_Ring + 0xc, 0x00002401);

    usb_status = read_reg(MIZAR_USB_USBSTS);
    LOGT("set_address: USBSTS read = 0x%x", usb_status);
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
}

/*
 * Function: enumeration
 * Description: Issues GET_DESCRIPTOR (Device) TRBs via EP0 Transfer Ring
 *              including setup stage and data stage.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void enumeration(void)
{
    int event_completion;

    // GET configuration get device - Setup Stage TRB
    write_reg(EP0_TR_Dequeue_Pointer, 0x01000680);
    write_reg(EP0_TR_Dequeue_Pointer + 0x4, 0x00120000);
    write_reg(EP0_TR_Dequeue_Pointer + 0x8, 0x08);
    write_reg(EP0_TR_Dequeue_Pointer + 0xc, 0x00030861);

    // DATA Stage TRB
    write_reg(EP0_TR_Dequeue_Pointer + 0x10, EP0_TR_Dequeue_Pointer + 0x200);
    write_reg(EP0_TR_Dequeue_Pointer + 0x18, 0x12);
    write_reg(EP0_TR_Dequeue_Pointer + 0x1c, 0x00010c27);

    LOGT("enumeration: GET_DESCRIPTOR (Device) TRBs issued via EP0_TR_Dequeue_Pointer");
}

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler that clears int_pend, reads system register
 *              status, acknowledges USB interrupt via IMAN, clears raw
 *              interrupt status, and clears GIC IRQ 84.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rd_data, sysreg_rd_data;
    int_pend = 0;
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("Default_IRQHandler: MSK_STS0 = 0x%x", rd_data);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("Default_IRQHandler: RAW_STCR0 = 0x%x", rd_data);
    // NOTE: Source uses logical AND (&&) not bitwise AND (&). Preserved as-is per No Silent Correction Rule.
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_USB_IMAN, 0x1);
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }
    GIC_ClearIRQ(84);
    LOGT("Default_IRQHandler: GIC IRQ 84 cleared");
}

/*
 * Function: usb_host_enumeration_hs_init
 * Description: Initializes the xHCI host controller: configures PHY,
 *              reads structural parameters, sets up Event Ring Segment Table,
 *              Device Context Base Address Array, Command Ring, Event Ring,
 *              Interrupter registers, CONFIG, enables system interrupts,
 *              and starts the host controller.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_host_enumeration_hs_init(const TestsItem cfg)
{
    int rd_data, port_count;

    (void)cfg;

    g_errors = 0;

    LOGT("usb_host_enumeration_hs_init: starting xHCI host controller initialization");

    // Step 1: NIC programming
    nic_programming();
    LOGT("usb_host_enumeration_hs_init: nic_programming() called");

    // Step 2: Enable all IRQs
    GIC_EnableAllIRQ();
    LOGT("usb_host_enumeration_hs_init: GIC_EnableAllIRQ() called");

    // Step 3: PHY CONTROL REG - read then write
    rd_data = read_reg(MIZAR_USB_BASE + 0xc200);
    LOGT("usb_host_enumeration_hs_init: PHY CTRL REG read = 0x%x", rd_data);
    write_reg(MIZAR_USB_BASE + 0xc200, 0x102407);
    LOGT("usb_host_enumeration_hs_init: PHY CTRL REG written with 0x102407");

    // Step 4: Read HCSPARAMS1 for port count
    port_count = read_reg(MIZAR_USB_HCSPARAMS1);
    LOGT("usb_host_enumeration_hs_init: HCSPARAMS1 = 0x%x", port_count);

    // Step 5: Read SUPTPRT2_DW2 and SUPTPRT3_DW2
    rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2);
    LOGT("usb_host_enumeration_hs_init: SUPTPRT2_DW2 = 0x%x", rd_data);
    rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2);
    LOGT("usb_host_enumeration_hs_init: SUPTPRT3_DW2 = 0x%x", rd_data);

    // Step 6: Read PORTSC_20 for initial port status
    rd_data = read_reg(MIZAR_USB_PORTSC_20);
    LOGT("usb_host_enumeration_hs_init: PORTSC_20 = 0x%x", rd_data);

    // Step 7: Read HCSPARAMS2 and PAGESIZE
    rd_data = read_reg(MIZAR_USB_HCSPARAMS2);
    LOGT("usb_host_enumeration_hs_init: HCSPARAMS2 = 0x%x", rd_data);
    rd_data = read_reg(MIZAR_USB_PAGESIZE);
    LOGT("usb_host_enumeration_hs_init: PAGESIZE = 0x%x", rd_data);

    // Step 8: Event Ring Segment Table setup
    write_reg(Event_Ring_Segment_Table, Default_Event_Ring_Array);
    write_reg(Event_Ring_Segment_Table + DWORD, 0x0);
    write_reg(Event_Ring_Segment_Table + 2 * (DWORD), 0x20);
    LOGT("usb_host_enumeration_hs_init: Event Ring Segment Table configured");

    // Step 9: Device Context Base Address Array setup
    write_reg(Device_Context_Base_Address_Array + 2 * DWORD, Device_Context_Array + 0x100);
    write_reg(Device_Context_Base_Address_Array + 3 * DWORD, 0x0);
    write_reg(Device_Context_Base_Address_Array + 4 * DWORD, Device_Context_Array + 0x0d00);
    write_reg(Device_Context_Base_Address_Array + 5 * DWORD, 0x0);
    LOGT("usb_host_enumeration_hs_init: DCBAA populated");

    // Step 10: Read and write CONFIG with 0x110
    rd_data = read_reg(MIZAR_USB_CONFIG);
    LOGT("usb_host_enumeration_hs_init: CONFIG read = 0x%x", rd_data);
    write_reg(MIZAR_USB_CONFIG, 0x110);

    // Step 11: Write DCBAAP_LO and DCBAAP_HI
    write_reg(MIZAR_USB_DCBAAP_LO, Device_Context_Base_Address_Array);
    write_reg(MIZAR_USB_DCBAAP_HI, 0x0);

    // Step 12: Write ERSTSZ
    write_reg(MIZAR_USB_ERSTSZ, 0x1);

    // Step 13: Write CRCR_LO and CRCR_HI
    write_reg(MIZAR_USB_CRCR_LO, Default_Command_Ring + 0x1);
    write_reg(MIZAR_USB_CRCR_HI, 0x0);

    // Step 14: Write CONFIG with 0x10 (MaxSlotsEn = 16)
    write_reg(MIZAR_USB_CONFIG, 0x10);

    // Step 15: Write ERDP_LO and ERDP_HI
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);

    // Step 16: Write ERSTBA_LO and ERSTBA_HI
    write_reg(MIZAR_USB_ERSTBA_LO, Event_Ring_Segment_Table);
    write_reg(MIZAR_USB_ERSTBA_HI, 0x0);

    // Step 17: Write IMOD, IMAN, USBCMD
    write_reg(MIZAR_USB_IMOD, 0x0);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_USBCMD, 0x4);
    LOGT("usb_host_enumeration_hs_init: xHCI registers configured, USBCMD interrupt enable set");

    // Step 18: Enable system interrupt
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
    LOGT("usb_host_enumeration_hs_init: system interrupt enabled (INTR_EN0 = 0x80000000)");

    // Step 19: Write USBCMD with 0x5 (Run/Stop = Run)
    write_reg(MIZAR_USB_USBCMD, 0x5);
    LOGT("usb_host_enumeration_hs_init: USBCMD Run set (0x5)");

    return 0;
}

/*
 * Function: usb_host_enumeration_hs_run
 * Description: Executes the USB host enumeration flow at High Speed:
 *              waits for device connection interrupt, performs port reset,
 *              issues Set Address command, EP command, doorbell ring,
 *              and calls enumeration for GET_DESCRIPTOR.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_host_enumeration_hs_run(const TestsItem *cfg, TestOutput out)
{
    int rd_data;
    int usb_status, port_status;
    int input_context_address;
    int count;

    (void)cfg;

    if (out == 0) {
        LOGE("usb_host_enumeration_hs_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("usb_host_enumeration_hs_run: starting USB host enumeration");

    // Step 20: Set int_pend = 1, poll until interrupt clears int_pend
    int_pend = 1;
    LOGT("usb_host_enumeration_hs_run: waiting for device connect interrupt (int_pend polling)");
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 10000; d++);
        count++;
        if (count > 100000) {
            LOGE("usb_host_enumeration_hs_run: timeout waiting for connect interrupt");
            g_errors++;
            out->status = -1;
            break;
        }
    }

    // Step 21: Read USBSTS, clear status by writing 0x8
    usb_status = read_reg(MIZAR_USB_USBSTS);
    LOGT("usb_host_enumeration_hs_run: USBSTS = 0x%x", usb_status);
    write_reg(MIZAR_USB_USBSTS, 0x8);

    // Step 22: Write IMAN, ERDP_HI, ERDP_LO
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x18);

    // Step 23: Write PORTSC_20 with 0xe0006f1 (port reset)
    write_reg(MIZAR_USB_PORTSC_20, 0xe0006f1);
    LOGT("usb_host_enumeration_hs_run: PORTSC_20 port reset written (0xe0006f1)");

    // Step 24: Clear USBSTS (0x8), IMAN (0x2)
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);

    // Step 25: Write PORTSC_20 with 0xe220200, read back PORTSC_20
    write_reg(MIZAR_USB_PORTSC_20, 0xe220200);
    port_status = read_reg(MIZAR_USB_PORTSC_20);
    LOGT("usb_host_enumeration_hs_run: PORTSC_20 readback = 0x%x", port_status);

    // Step 26: Set int_pend = 1, poll until interrupt clears int_pend
    int_pend = 1;
    LOGT("usb_host_enumeration_hs_run: waiting for port reset interrupt (int_pend polling)");
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 10000; d++);
        count++;
        if (count > 100000) {
            LOGE("usb_host_enumeration_hs_run: timeout waiting for port reset interrupt");
            g_errors++;
            out->status = -1;
            break;
        }
    }

    // Step 27: Call set_address()
    set_address();
    LOGT("usb_host_enumeration_hs_run: set_address() completed");

    // Step 28: Compute input_context_address from Default_Input_Context
    input_context_address = (Default_Input_Context) + 0x0;

    // Step 29: Issue EP command TRB via Default_Command_Ring at offset 0x30
    write_reg(Default_Command_Ring + 0x30, input_context_address);
    write_reg(Default_Command_Ring + 0x34, 0x0);
    write_reg(Default_Command_Ring + 0x38, 0x0);
    write_reg(Default_Command_Ring + 0x3c, 0x1003001);
    LOGT("usb_host_enumeration_hs_run: EP command TRB written to Command Ring + 0x30");

    // Step 30: Clear USBSTS, IMAN, update ERDP, write DB (doorbell)
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x58);
    write_reg(MIZAR_USB_DB, 0x0);
    LOGT("usb_host_enumeration_hs_run: doorbell rung (DB = 0x0)");

    // Step 31: Call enumeration()
    enumeration();
    LOGT("usb_host_enumeration_hs_run: enumeration() completed");

    if (g_errors > 0) {
        out->status = -1;
    }

    LOGT("usb_host_enumeration_hs_run: %s errors=%d",
         (out->status == 0) ? "PASS" : "FAIL", g_errors);

    return out->status;
}

/*
 * Function: usb_host_enumeration_hs_teardown
 * Description: Performs final cleanup and reports testcase result.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_host_enumeration_hs_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("usb_host_enumeration_hs_teardown: teardown complete, errors=%d", g_errors);
    return (g_errors == 0) ? 0 : -1;
}
