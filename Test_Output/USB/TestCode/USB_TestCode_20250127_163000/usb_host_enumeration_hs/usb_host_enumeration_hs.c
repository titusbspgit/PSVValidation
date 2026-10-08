// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_host_enumeration_hs.h"
#include "test_define.inc"

/* Extern interrupt pending flag used by IRQ handler */
extern int int_pend;

/*
 * Function: set_address
 * Description: Issues Enable Slot command TRB on the Default Command Ring,
 *              reads USBSTS for command completion, clears status.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void set_address(void)
{
    int input_context_address;
    int event_completion, usb_status;

    LOGT("set_address: issuing slot command TRB");

    /* slot command TRB */
    write_reg(Default_Command_Ring + 0x0, 0x0);
    write_reg(Default_Command_Ring + 0x4, 0x0);
    write_reg(Default_Command_Ring + 0x8, 0x0);
    write_reg(Default_Command_Ring + 0xc, 0x00002401);

    usb_status = read_reg(MIZAR_USB_USBSTS);
    LOGT("set_address: USBSTS = 0x%x", usb_status);
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);

    LOGT("set_address: complete");
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
static void enumeration(void)
{
    int event_completion;

    LOGT("enumeration: issuing GET_DESCRIPTOR (Device) TRBs");

    /* GET configuration get device - setup stage TRB */
    write_reg(EP0_TR_Dequeue_Pointer, 0x01000680);
    write_reg(EP0_TR_Dequeue_Pointer + 0x4, 0x00120000);
    write_reg(EP0_TR_Dequeue_Pointer + 0x8, 0x08);
    write_reg(EP0_TR_Dequeue_Pointer + 0xc, 0x00030861);

    /* DATA stage TRB */
    write_reg(EP0_TR_Dequeue_Pointer + 0x10, EP0_TR_Dequeue_Pointer + 0x200);
    write_reg(EP0_TR_Dequeue_Pointer + 0x18, 0x12);
    write_reg(EP0_TR_Dequeue_Pointer + 0x1c, 0x00010c27);

    LOGT("enumeration: GET_DESCRIPTOR TRBs issued");
}

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler. Clears int_pend, reads system register
 *              masked status and raw status, acknowledges USB interrupt via
 *              IMAN, clears raw interrupt status, and clears GIC IRQ 84.
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
    LOGT("IRQHandler: MSK_STS0 = 0x%x", rd_data);

    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("IRQHandler: RAW_STCR0 = 0x%x", rd_data);

    /* NOTE: Source uses logical AND (&&) not bitwise AND (&). Preserved as-is per No Silent Correction Rule. */
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_USB_IMAN, 0x1);
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    GIC_ClearIRQ(84);
    LOGT("IRQHandler: GIC IRQ 84 cleared");
}

/*
 * Function: usb_host_enumeration_hs_init
 * Description: Initializes the xHCI host controller for USB High Speed
 *              enumeration. Configures PHY, reads structural parameters,
 *              sets up Event Ring Segment Table, DCBAA, Command Ring,
 *              Event Ring, Interrupter registers, and CONFIG.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_host_enumeration_hs_init(const TestsItem cfg)
{
    int rd_data, port_count;

    (void)cfg;

    LOGT("usb_host_enumeration_hs_init: starting xHCI initialization");

    /* Step 1: NIC initialization */
    nic_programming();
    LOGT("nic_programming() complete");

    /* Step 2: Enable all IRQs */
    GIC_EnableAllIRQ();
    LOGT("GIC_EnableAllIRQ() complete");

    /* Step 3: PHY CONTROL REG */
    rd_data = read_reg(MIZAR_USB_BASE + 0xc200);
    LOGT("PHY Control Reg read: 0x%x", rd_data);
    write_reg(MIZAR_USB_BASE + 0xc200, 0x102407);
    LOGT("PHY Control Reg written: 0x102407");

    /* Step 4: Read HCSPARAMS1 for port count */
    port_count = read_reg(MIZAR_USB_HCSPARAMS1);
    LOGT("HCSPARAMS1 (port_count) = 0x%x", port_count);

    /* Step 5: Read supported protocol information */
    rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2);
    LOGT("SUPTPRT2_DW2 = 0x%x", rd_data);
    rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2);
    LOGT("SUPTPRT3_DW2 = 0x%x", rd_data);

    /* Step 6: Read initial port status */
    rd_data = read_reg(MIZAR_USB_PORTSC_20);
    LOGT("PORTSC_20 initial = 0x%x", rd_data);

    /* Step 7: Read HCSPARAMS2 and PAGESIZE */
    rd_data = read_reg(MIZAR_USB_HCSPARAMS2);
    LOGT("HCSPARAMS2 = 0x%x", rd_data);
    rd_data = read_reg(MIZAR_USB_PAGESIZE);
    LOGT("PAGESIZE = 0x%x", rd_data);

    /* Step 8: Event Ring Segment Table setup */
    write_reg(Event_Ring_Segment_Table, Default_Event_Ring_Array);
    write_reg(Event_Ring_Segment_Table + DWORD, 0x0);
    write_reg(Event_Ring_Segment_Table + 2 * (DWORD), 0x20);
    LOGT("Event Ring Segment Table configured");

    /* Step 9: Device Context Base Address Array setup */
    write_reg(Device_Context_Base_Address_Array + 2 * DWORD, Device_Context_Array + 0x100);
    write_reg(Device_Context_Base_Address_Array + 3 * DWORD, 0x0);
    write_reg(Device_Context_Base_Address_Array + 4 * DWORD, Device_Context_Array + 0x0d00);
    write_reg(Device_Context_Base_Address_Array + 5 * DWORD, 0x0);
    LOGT("DCBAA configured");

    /* Step 10: Read and write CONFIG with 0x110 */
    rd_data = read_reg(MIZAR_USB_CONFIG);
    LOGT("CONFIG read = 0x%x", rd_data);
    write_reg(MIZAR_USB_CONFIG, 0x110);
    LOGT("CONFIG written: 0x110");

    /* Step 11: Write DCBAAP */
    write_reg(MIZAR_USB_DCBAAP_LO, Device_Context_Base_Address_Array);
    write_reg(MIZAR_USB_DCBAAP_HI, 0x0);
    LOGT("DCBAAP configured");

    /* Step 12: Write ERSTSZ */
    write_reg(MIZAR_USB_ERSTSZ, 0x1);
    LOGT("ERSTSZ written: 0x1");

    /* Step 13: Write CRCR */
    write_reg(MIZAR_USB_CRCR_LO, Default_Command_Ring + 0x1);
    write_reg(MIZAR_USB_CRCR_HI, 0x0);
    LOGT("CRCR configured");

    /* Step 14: Write CONFIG with 0x10 (MaxSlotsEn = 16) */
    write_reg(MIZAR_USB_CONFIG, 0x10);
    LOGT("CONFIG written: 0x10");

    /* Step 15: Write ERDP */
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
    LOGT("ERDP configured");

    /* Step 16: Write ERSTBA */
    write_reg(MIZAR_USB_ERSTBA_LO, Event_Ring_Segment_Table);
    write_reg(MIZAR_USB_ERSTBA_HI, 0x0);
    LOGT("ERSTBA configured");

    /* Step 17: Write IMOD, IMAN, USBCMD */
    write_reg(MIZAR_USB_IMOD, 0x0);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_USBCMD, 0x4);
    LOGT("IMOD=0x0, IMAN=0x2, USBCMD=0x4 written");

    /* Step 18: Enable system interrupt */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
    LOGT("System interrupt enabled: INTR_EN0 = 0x80000000");

    LOGT("usb_host_enumeration_hs_init: complete");
    return 0;
}

/*
 * Function: usb_host_enumeration_hs_run
 * Description: Runs the USB host enumeration sequence at High Speed.
 *              Starts the host controller, waits for device connection,
 *              performs port reset, issues Set Address command, EP command,
 *              and enumerates the device via GET_DESCRIPTOR.
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

    LOGT("usb_host_enumeration_hs_run: starting");

    /* Step 19: Write USBCMD with 0x5 (Run/Stop = Run) */
    write_reg(MIZAR_USB_USBCMD, 0x5);
    LOGT("USBCMD written: 0x5 (Run)");

    /* Step 20: Set int_pend = 1, poll until interrupt clears int_pend */
    /* wait_on(100) converted to deterministic busy-wait loop per PSV WAIT API ADAPTATION RULE */
    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 10000; d++);
        count++;
        if (count > 100000) {
            LOGE("Timeout waiting for interrupt (first int_pend poll)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("First interrupt received, int_pend cleared");

    /* Step 21: Read USBSTS, clear status */
    usb_status = read_reg(MIZAR_USB_USBSTS);
    LOGT("USBSTS after first interrupt = 0x%x", usb_status);
    write_reg(MIZAR_USB_USBSTS, 0x8);

    /* Step 22: Write IMAN, ERDP_HI, ERDP_LO */
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x18);
    LOGT("IMAN, ERDP updated after first interrupt");

    /* Step 23: Write PORTSC_20 for port reset */
    write_reg(MIZAR_USB_PORTSC_20, 0xe0006f1);
    LOGT("PORTSC_20 written: 0xe0006f1 (port reset)");

    /* Step 24: Clear USBSTS, IMAN */
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);

    /* Step 25: Write PORTSC_20, read back */
    write_reg(MIZAR_USB_PORTSC_20, 0xe220200);
    port_status = read_reg(MIZAR_USB_PORTSC_20);
    LOGT("PORTSC_20 after port status update = 0x%x", port_status);

    /* Step 26: Set int_pend = 1, poll until interrupt clears int_pend */
    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 10000; d++);
        count++;
        if (count > 100000) {
            LOGE("Timeout waiting for interrupt (second int_pend poll)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("Second interrupt received, int_pend cleared");

    /* Step 27: Call set_address() */
    set_address();
    LOGT("set_address() complete");

    /* Step 28: Compute input_context_address */
    input_context_address = (Default_Input_Context) + 0x0;

    /* Step 29: EP command TRB via Default_Command_Ring at offset 0x30 */
    write_reg(Default_Command_Ring + 0x30, input_context_address);
    write_reg(Default_Command_Ring + 0x34, 0x0);
    write_reg(Default_Command_Ring + 0x38, 0x0);
    write_reg(Default_Command_Ring + 0x3c, 0x1003001);
    LOGT("EP command TRB written to Command Ring + 0x30");

    /* Step 30: Clear USBSTS, IMAN, update ERDP, write DB */
    write_reg(MIZAR_USB_USBSTS, 0x8);
    write_reg(MIZAR_USB_IMAN, 0x2);
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x58);
    write_reg(MIZAR_USB_DB, 0x0);
    LOGT("Doorbell rung (DB = 0x0)");

    /* Step 31: Call enumeration() */
    enumeration();
    LOGT("enumeration() complete");

    LOGT("usb_host_enumeration_hs_run: complete, status=%d", out->status);
    return out->status;
}

/*
 * Function: usb_host_enumeration_hs_teardown
 * Description: Teardown for USB host enumeration testcase.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success
 */
int usb_host_enumeration_hs_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("usb_host_enumeration_hs_teardown: no additional cleanup required");
    return 0;
}
