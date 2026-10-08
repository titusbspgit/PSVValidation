// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_host_enumeration_hs.h"
#include "test_define.inc"

/* Global interrupt pending flag (Meta step 1) */
extern int int_pend;

/* Testcase error tracking */
static unsigned int g_errors = 0U;

#define USB_HOST_ENUM_HS_TIMEOUT 100000U

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler (Meta steps 55-63)
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rd_data;

    /* Meta step 56 */
    int_pend = 0;

    /* Meta step 57 */
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);

    /* Meta step 58 */
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

    /* Meta step 59: preserved as logical AND (&&) per No Silent Correction Rule */
    if (rd_data && 0x80000000) {
        /* Meta step 60 */
        write_reg(MIZAR_USB_IMAN, 0x1);
        /* Meta step 61 */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    /* Meta step 63 */
    GIC_ClearIRQ(84);
}

/*
 * Function: set_address
 * Description: Issue Enable Slot and Address Device commands (Meta steps 64-85)
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void set_address(void)
{
    int input_context_address;
    int event_completion, usb_status;

    /* Meta step 67-68: Enable Slot command TRB */
    write_reg(Default_Command_Ring + 0x0, 0x0);
    /* Meta step 69 */
    write_reg(Default_Command_Ring + 0x4, 0x0);
    /* Meta step 70 */
    write_reg(Default_Command_Ring + 0x8, 0x0);
    /* Meta step 71 */
    write_reg(Default_Command_Ring + 0xc, 0x00002401);

    /* Meta step 72 */
    usb_status = read_reg(MIZAR_USB_USBSTS);
    /* Meta step 73 */
    write_reg(MIZAR_USB_USBSTS, 0x8);
    /* Meta step 74 */
    write_reg(MIZAR_USB_IMAN, 0x2);
    /* Meta step 75 */
    write_reg(MIZAR_USB_ERDP_HI, 0x0);

    /* Meta step 76-77: Address Device command TRB */
    input_context_address = (Default_Input_Context) + 0x0;
    /* Meta step 78 */
    write_reg(Default_Command_Ring + 0x10, input_context_address);
    /* Meta step 79 */
    write_reg(Default_Command_Ring + 0x1c, 0x01002e01);

    /* Meta step 80 */
    write_reg(MIZAR_USB_USBSTS, 0x8);
    /* Meta step 81 */
    write_reg(MIZAR_USB_IMAN, 0x2);
    /* Meta step 82 */
    write_reg(MIZAR_USB_ERDP_HI, 0x0);
    /* Meta step 83 */
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x68);

    /* Meta step 84: Ring doorbell */
    write_reg(MIZAR_USB_DB, 0x0);

    LOGT("set_address: Enable Slot and Address Device commands issued");
}

/*
 * Function: enumeration
 * Description: Issue GET_DESCRIPTOR control transfer TRBs (Meta steps 86-101)
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void enumeration(void)
{
    int event_completion;

    /* Meta step 87-88: GET Device Descriptor - Setup Stage */
    write_reg(EP0_TR_Dequeue_Pointer, 0x01000680);
    /* Meta step 89 */
    write_reg(EP0_TR_Dequeue_Pointer + 0x4, 0x00120000);
    /* Meta step 90 */
    write_reg(EP0_TR_Dequeue_Pointer + 0x8, 0x08);
    /* Meta step 91 */
    write_reg(EP0_TR_Dequeue_Pointer + 0xc, 0x00030861);

    /* Meta step 92-93: Data Stage */
    write_reg(EP0_TR_Dequeue_Pointer + 0x10, EP0_TR_Dequeue_Pointer + 0x200);
    /* Meta step 94 */
    write_reg(EP0_TR_Dequeue_Pointer + 0x18, 0x12);
    /* Meta step 95 */
    write_reg(EP0_TR_Dequeue_Pointer + 0x1c, 0x00010c27);

    /* Meta step 96-97: Status Stage */
    write_reg(EP0_TR_Dequeue_Pointer + 0xc8, 0x18);
    /* Meta step 98 */
    write_reg(EP0_TR_Dequeue_Pointer + 0xcc, 0x00010c25);
    /* Meta step 99 */
    write_reg(EP0_TR_Dequeue_Pointer + 0xdc, 0x00001023);

    /* Meta step 100: Ring doorbell */
    write_reg(MIZAR_USB_BASE + 0x484, 0x1);

    LOGT("enumeration: GET_DESCRIPTOR TRBs programmed and doorbell rung");
}

/*
 * Function: usb_host_enumeration_hs_init
 * Description: Initialize USB host enumeration HS testcase
 * Parameters:
 *   cfg - test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int usb_host_enumeration_hs_init(const TestsItem *cfg)
{
    (void)cfg;

    g_errors = 0U;

    LOGT("USB host enumeration HS test init");

    /* Meta step 4: nic_programming */
    /* MANUAL_REVIEW: nic_programming() is a DV/platform initialization call. */
    /* Verify if an FV/PSV equivalent is required and available. */
    nic_programming();

    /* Meta step 5: Enable all IRQs via GIC */
    GIC_EnableAllIRQ();

    LOGT("GIC IRQs enabled");

    return 0;
}

/*
 * Function: usb_host_enumeration_hs_run
 * Description: Execute USB host enumeration HS testcase
 * Parameters:
 *   cfg - test configuration item
 *   out - test output structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_host_enumeration_hs_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    int port_count;
    int db_offset;
    int event_completion;
    int usb_status;
    int port_status;
    int input_context_address;
    int j;
    int i;
    int count = 0;
    unsigned int timeout_cnt;

    (void)cfg;

    if (out == 0) {
        LOGE("USB host enumeration HS: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB host enumeration HS: run started");

    /* Meta step 6: Read PHY CONTROL REG */
    rd_data = read_reg(MIZAR_USB_BASE + 0xc200);
    LOGT("PHY CONTROL REG read: 0x%x", rd_data);

    /* Meta step 7: Write PHY CONTROL REG */
    write_reg(MIZAR_USB_BASE + 0xc200, 0x102407);
    LOGT("PHY CONTROL REG configured with 0x102407");

    /* Meta step 8: Read HCSPARAMS1 */
    port_count = read_reg(MIZAR_USB_HCSPARAMS1);
    LOGT("HCSPARAMS1 read: 0x%x", port_count);

    /* Meta step 9: Read SUPTPRT2_DW2 */
    rd_data = read_reg(MIZAR_USB_SUPTPRT2_DW2);

    /* Meta step 10: Read SUPTPRT3_DW2 */
    rd_data = read_reg(MIZAR_USB_SUPTPRT3_DW2);

    /* Meta step 11: Read PORTSC_20 initial */
    rd_data = read_reg(MIZAR_USB_PORTSC_20);

    /* Meta step 12: Read CONFIG */
    rd_data = read_reg(MIZAR_USB_CONFIG);

    /* Meta step 13: Write CONFIG with CIE set */
    write_reg(MIZAR_USB_CONFIG, 0x110);
    LOGT("CONFIG register set to 0x110");

    /* Meta step 14: Write DCBAAP_LO */
    write_reg(MIZAR_USB_DCBAAP_LO, Device_Context_Base_Address_Array);

    /* Meta step 15: Write DCBAAP_HI */
    write_reg(MIZAR_USB_DCBAAP_HI, 0x0);

    /* Meta step 16: Write ERSTSZ */
    write_reg(MIZAR_USB_ERSTSZ, 0x1);

    /* Meta step 17: Write ERDP_LO */
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array);

    /* Meta step 18: Write ERDP_HI */
    write_reg(MIZAR_USB_ERDP_HI, 0x0);

    /* Meta step 19: Write ERSTBA_LO */
    write_reg(MIZAR_USB_ERSTBA_LO, Event_Ring_Segment_Table);

    /* Meta step 20: Write ERSTBA_HI */
    write_reg(MIZAR_USB_ERSTBA_HI, 0x0);

    /* Meta step 21: Write IMOD */
    write_reg(MIZAR_USB_IMOD, 0x0);

    /* Meta step 22: Write IMAN */
    write_reg(MIZAR_USB_IMAN, 0x2);

    /* Meta step 23: Write USBCMD interrupt enable */
    write_reg(MIZAR_USB_USBCMD, 0x4);
    LOGT("USBCMD interrupt enable set (0x4)");

    /* Meta step 24: PORTSC_20 Wake on Connect Enable */
    write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WCE, 1));

    /* Meta step 25: PORTSC_20 Wake on Disconnect Enable */
    write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WDE, 1));

    /* Meta step 26: PORTSC_20 Wake on Over-current Enable */
    write_reg(MIZAR_USB_PORTSC_20, set_data(read_reg(MIZAR_USB_PORTSC_20), USB_PORTSC_20_WOE, 1));

    /* Meta step 27: Write PORTSC_20 */
    write_reg(MIZAR_USB_PORTSC_20, 0xe0002a0);

    /* Meta step 28: Read DBOFF */
    db_offset = read_reg(MIZAR_USB_DBOFF);
    LOGT("DBOFF read: 0x%x", db_offset);

    /* Meta step 29: Enable system-level interrupt */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
    LOGT("SYSREG_INTR_EN0 set to 0x80000000");

    /* Meta step 30: USBCMD run/stop bit = 1, run */
    write_reg(MIZAR_USB_USBCMD, 0x5);
    LOGT("USBCMD set to 0x5 - host controller running");

    /* Meta step 31: int_pend = 1 */
    int_pend = 1;

    /* Meta step 32: Wait for port status change interrupt */
    timeout_cnt = 0U;
    while (int_pend) {
        wait_on(100);
        timeout_cnt++;
        if (timeout_cnt >= USB_HOST_ENUM_HS_TIMEOUT) {
            LOGE("Timeout waiting for port status change interrupt");
            g_errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("Port status change interrupt received");

    /* Meta step 33: Read USBSTS */
    usb_status = read_reg(MIZAR_USB_USBSTS);
    LOGT("USBSTS after interrupt: 0x%x", usb_status);

    /* Meta step 34: Clear USBSTS */
    write_reg(MIZAR_USB_USBSTS, 0x8);

    /* Meta step 35: Write IMAN */
    write_reg(MIZAR_USB_IMAN, 0x2);

    /* Meta step 36: Clear ERDP_HI */
    write_reg(MIZAR_USB_ERDP_HI, 0x0);

    /* Meta step 37: Read PORTSC_20 - port connect status should be high */
    port_status = read_reg(MIZAR_USB_PORTSC_20);
    LOGT("PORTSC_20 after connect interrupt: 0x%x", port_status);

    /* Meta step 38: Update ERDP_LO */
    write_reg(MIZAR_USB_ERDP_LO, Default_Event_Ring_Array + 0x18);

    /* Meta step 39: Port reset */
    write_reg(MIZAR_USB_PORTSC_20, 0xe0006f1);
    LOGT("Port reset issued (0xe0006f1)");

    /* Meta step 40: Clear USBSTS */
    write_reg(MIZAR_USB_USBSTS, 0x8);

    /* Meta step 41: Write IMAN */
    write_reg(MIZAR_USB_IMAN, 0x2);

    /* Meta step 42: Post-reset port configuration */
    write_reg(MIZAR_USB_PORTSC_20, 0xe220200);

    /* Meta step 43: Read PORTSC_20 */
    port_status = read_reg(MIZAR_USB_PORTSC_20);
    LOGT("PORTSC_20 after post-reset config: 0x%x", port_status);

    /* Meta step 44: int_pend = 1 */
    int_pend = 1;

    /* Meta step 45: Wait for port reset completion interrupt */
    timeout_cnt = 0U;
    while (int_pend) {
        wait_on(100);
        timeout_cnt++;
        if (timeout_cnt >= USB_HOST_ENUM_HS_TIMEOUT) {
            LOGE("Timeout waiting for port reset completion interrupt");
            g_errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("Port reset completion interrupt received");

    /* Meta step 46: Call set_address() */
    set_address();

    /* Meta step 47: input_context_address */
    input_context_address = (Default_Input_Context) + 0x0;

    /* Meta step 48: Read event completion */
    event_completion = read_reg(Default_Event_Ring_Array + 0x50);
    LOGT("Event completion before interrupt: 0x%x", event_completion);

    /* Meta step 49: int_pend = 1 */
    int_pend = 1;

    /* Meta step 50: Wait for interrupt */
    timeout_cnt = 0U;
    while (int_pend) {
        wait_on(100);
        timeout_cnt++;
        if (timeout_cnt >= USB_HOST_ENUM_HS_TIMEOUT) {
            LOGE("Timeout waiting for command completion interrupt");
            g_errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("Command completion interrupt received");

    /* Meta step 51: Read event completion after interrupt */
    event_completion = read_reg(Default_Event_Ring_Array + 0x50);
    LOGT("Event completion after interrupt: 0x%x", event_completion);

    /* Meta step 52: Call enumeration() */
    enumeration();

    /* Meta step 53: finish(0) converted to FV/PSV status */
    /* finish(0) is prohibited - use out->status for pass/fail */
    if (g_errors == 0U) {
        out->status = 0;
        LOGT("USB host enumeration HS: PASS");
    } else {
        out->status = -1;
        LOGE("USB host enumeration HS: FAIL errors=%u", g_errors);
    }

    return out->status;
}

/*
 * Function: usb_host_enumeration_hs_teardown
 * Description: Teardown USB host enumeration HS testcase
 * Parameters:
 *   cfg - test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int usb_host_enumeration_hs_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB host enumeration HS teardown: no additional cleanup required");
    return g_errors == 0U ? 0 : -1;
}
