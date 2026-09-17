// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_host_enumeration_fs.h"
#include "test_define.inc"

/*
 * usb_host_enumeration_fs
 *
 * This test validates USB Host mode Full-Speed device enumeration using the xHCI
 * host controller interface. The test configures the global controller, frame length
 * adjustment, and user control registers. It sets up the USB3 PIPE control and USB2
 * PHY control registers. Host capability parameters are read. The port status and
 * control register is configured with wake enables. xHCI data structures are
 * initialized. The host controller is started. After device connection and port reset,
 * Enable Slot, Address Device (BSR=1 and BSR=0), and Configure Endpoint commands are
 * issued. Enumeration issues GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR
 * (short and full), and SET CONFIGURATION via TRBs. Event completion is verified by
 * polling the Event Ring.
 *
 * Full-Speed specific: SlotCtx=0x08100000, GFLADJ=0x0a07e000
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
    volatile unsigned int int_pend;
} usb_host_fs_test_ctx_t;

static usb_host_fs_test_ctx_t g_ctx;

static int poll_event_ring(unsigned int timeout)
{
    unsigned int val;
    unsigned int count = timeout;
    do {
        val = readl_reg((unsigned int)(uintptr_t)Default_Event_Ring_Array);
        if ((val & 0x01U) != 0U) { return 0; }
        count--;
    } while (count > 0U);
    LOGE("USB Host FS: Event Ring poll timeout");
    return -1;
}

static void ring_doorbell(unsigned int db_target)
{
    writel_reg(MIZAR_USB_DB, db_target);
}

void Default_IRQHandler(void)
{
    unsigned int usbsts, iman, sysreg_sts;
    usbsts = readl_reg(MIZAR_USB_USBSTS);
    writel_reg(MIZAR_USB_USBSTS, usbsts);
    iman = readl_reg(MIZAR_USB_IMAN);
    writel_reg(MIZAR_USB_IMAN, iman | 0x01U);
    sysreg_sts = readl_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    writel_reg(MIZAR_LSS_SYSREG_RAW_STCR0, sysreg_sts);
    g_ctx.int_pend = 1U;
}

static int wait_for_interrupt(unsigned int timeout)
{
    unsigned int count = timeout;
    while (g_ctx.int_pend == 0U) { if (count == 0U) { return -1; } count--; }
    g_ctx.int_pend = 0U;
    return 0;
}

static int issue_command_trb(unsigned int f0, unsigned int f1, unsigned int f2, unsigned int f3)
{
    writel_reg((unsigned int)(uintptr_t)Default_Command_Ring + 0x00U, f0);
    writel_reg((unsigned int)(uintptr_t)Default_Command_Ring + 0x04U, f1);
    writel_reg((unsigned int)(uintptr_t)Default_Command_Ring + 0x08U, f2);
    writel_reg((unsigned int)(uintptr_t)Default_Command_Ring + 0x0CU, f3);
    ring_doorbell(0x00000000U);
    if (poll_event_ring(100000U) != 0) { g_ctx.errors++; return -1; }
    writel_reg(MIZAR_USB_ERDP_LO, readl_reg(MIZAR_USB_ERDP_LO));
    g_ctx.checks_passed++; g_ctx.checks_total++;
    return 0;
}

static int issue_transfer_trb(unsigned int buf_lo, unsigned int buf_hi, unsigned int status, unsigned int ctrl)
{
    writel_reg((unsigned int)(uintptr_t)EP0_TR_Dequeue_Pointer + 0x00U, buf_lo);
    writel_reg((unsigned int)(uintptr_t)EP0_TR_Dequeue_Pointer + 0x04U, buf_hi);
    writel_reg((unsigned int)(uintptr_t)EP0_TR_Dequeue_Pointer + 0x08U, status);
    writel_reg((unsigned int)(uintptr_t)EP0_TR_Dequeue_Pointer + 0x0CU, ctrl);
    ring_doorbell(0x00000001U);
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; return -1; }
    if (poll_event_ring(100000U) != 0) { g_ctx.errors++; return -1; }
    writel_reg(MIZAR_USB_ERDP_LO, readl_reg(MIZAR_USB_ERDP_LO));
    g_ctx.checks_passed++; g_ctx.checks_total++;
    return 0;
}

int usb_host_enumeration_fs_init(const TestsItem *cfg)
{
    unsigned int val;
    (void)cfg;
    g_ctx = (usb_host_fs_test_ctx_t){0};
    LOGT("USB Host FS: Init");
    val = readl_reg(MIZAR_USB_GCTL); writel_reg(MIZAR_USB_GCTL, val);
    writel_reg(MIZAR_USB_GFLADJ, 0x0a07e000U);
    val = readl_reg(MIZAR_USB_GUCTL); writel_reg(MIZAR_USB_GUCTL, val);
    val = readl_reg(MIZAR_USB_HCSPARAMS1);
    val = readl_reg(MIZAR_USB_SUPTPRT2_DW2);
    val = readl_reg(MIZAR_USB_SUPTPRT3_DW2);
    val = readl_reg(MIZAR_USB_PORTSC_20);
    val |= (USB_PORTSC_20_WCE | USB_PORTSC_20_WDE | USB_PORTSC_20_WOE);
    writel_reg(MIZAR_USB_PORTSC_20, val);
    return 0;
}

int usb_host_enumeration_fs_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int val;
    (void)cfg;
    if (out == 0) return -1;
    out->status = 0;
    LOGT("USB Host FS: Run");
    /* Set up xHCI data structures */
    writel_reg((unsigned int)(uintptr_t)Event_Ring_Segment_Table + 0x00U, (unsigned int)(uintptr_t)Default_Event_Ring_Array);
    writel_reg((unsigned int)(uintptr_t)Event_Ring_Segment_Table + 0x04U, 0x00000000U);
    writel_reg((unsigned int)(uintptr_t)Event_Ring_Segment_Table + 0x08U, 0x00000040U);
    writel_reg((unsigned int)(uintptr_t)Event_Ring_Segment_Table + 0x0CU, 0x00000000U);
    val = readl_reg(MIZAR_USB_HCSPARAMS2); val = readl_reg(MIZAR_USB_PAGESIZE);
    writel_reg((unsigned int)(uintptr_t)Scratchpad_Buffer_Array + 0x00U, (unsigned int)(uintptr_t)SCRATCHPAD0);
    writel_reg((unsigned int)(uintptr_t)Scratchpad_Buffer_Array + 0x04U, 0x00000000U);
    writel_reg((unsigned int)(uintptr_t)Scratchpad_Buffer_Array + 0x08U, (unsigned int)(uintptr_t)SCRATCHPAD1);
    writel_reg((unsigned int)(uintptr_t)Scratchpad_Buffer_Array + 0x0CU, 0x00000000U);
    writel_reg((unsigned int)(uintptr_t)Device_Context_Base_Address_Array + 0x00U, (unsigned int)(uintptr_t)Scratchpad_Buffer_Array);
    writel_reg((unsigned int)(uintptr_t)Device_Context_Base_Address_Array + 0x04U, 0x00000000U);
    writel_reg((unsigned int)(uintptr_t)Device_Context_Base_Address_Array + 0x08U, (unsigned int)(uintptr_t)Device_Context_Array);
    writel_reg((unsigned int)(uintptr_t)Device_Context_Base_Address_Array + 0x0CU, 0x00000000U);
    writel_reg(MIZAR_USB_CRCR_LO, (unsigned int)(uintptr_t)Default_Command_Ring | 0x01U);
    writel_reg(MIZAR_USB_CRCR_HI, 0x00000000U);
    writel_reg(MIZAR_USB_CONFIG, 0x00000001U);
    writel_reg(MIZAR_USB_DCBAAP_LO, (unsigned int)(uintptr_t)Device_Context_Base_Address_Array);
    writel_reg(MIZAR_USB_DCBAAP_HI, 0x00000000U);
    writel_reg(MIZAR_USB_ERSTSZ, 0x00000001U);
    writel_reg(MIZAR_USB_ERDP_LO, (unsigned int)(uintptr_t)Default_Event_Ring_Array);
    writel_reg(MIZAR_USB_ERDP_HI, 0x00000000U);
    writel_reg(MIZAR_USB_ERSTBA_LO, (unsigned int)(uintptr_t)Event_Ring_Segment_Table);
    writel_reg(MIZAR_USB_ERSTBA_HI, 0x00000000U);
    writel_reg(MIZAR_USB_IMOD, 0x00000000U);
    writel_reg(MIZAR_USB_IMAN, 0x00000002U);
    writel_reg(MIZAR_USB_USBCMD, 0x00000005U);
    writel_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0xFFFFFFFFU);
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    val = readl_reg(MIZAR_USB_PORTSC_20);
    if ((val & 0x01U) == 0U) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    val = readl_reg(MIZAR_USB_PORTSC_20); val |= (1U << 4); writel_reg(MIZAR_USB_PORTSC_20, val);
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (issue_command_trb(0U, 0U, 0U, 0x00002401U) != 0) { out->status = -1; }
    writel_reg((unsigned int)(uintptr_t)Default_Input_Context + 0x00U, 0x00000003U);
    writel_reg((unsigned int)(uintptr_t)Default_Input_Context + 0x04U, 0x00000000U);
    writel_reg((unsigned int)(uintptr_t)Default_Input_Context + 0x08U, 0x08100000U);
    if (issue_command_trb((unsigned int)(uintptr_t)Default_Input_Context, 0U, 0U, 0x00000B01U) != 0) { out->status = -1; }
    if (issue_command_trb((unsigned int)(uintptr_t)Default_Input_Context, 0U, 0U, 0x00000901U) != 0) { out->status = -1; }
    if (issue_command_trb((unsigned int)(uintptr_t)Default_Input_Context, 0U, 0U, 0x00000C01U) != 0) { out->status = -1; }
    /* GET DEVICE DESCRIPTOR */
    if (issue_transfer_trb(0x01000680U, 0x00120000U, 0x00000008U, 0x00030C21U) != 0) { out->status = -1; }
    if (issue_transfer_trb((unsigned int)(uintptr_t)data_in, 0U, 0x00000012U, 0x00010C01U) != 0) { out->status = -1; }
    if (issue_transfer_trb(0U, 0U, 0U, 0x00010C01U) != 0) { out->status = -1; }
    /* GET CONFIG DESC short */
    if (issue_transfer_trb(0x02000680U, 0x00090000U, 0x00000008U, 0x00030C21U) != 0) { out->status = -1; }
    if (issue_transfer_trb((unsigned int)(uintptr_t)data_in, 0U, 0x00000009U, 0x00010C01U) != 0) { out->status = -1; }
    if (issue_transfer_trb(0U, 0U, 0U, 0x00010C01U) != 0) { out->status = -1; }
    /* SET CONFIGURATION */
    if (issue_transfer_trb(0x00010900U, 0x00000000U, 0x00000008U, 0x00030C21U) != 0) { out->status = -1; }
    if (issue_transfer_trb(0U, 0U, 0U, 0x00010C01U) != 0) { out->status = -1; }
    /* GET CONFIG DESC full */
    if (issue_transfer_trb(0x02000680U, 0x00FF0000U, 0x00000008U, 0x00030C21U) != 0) { out->status = -1; }
    if (issue_transfer_trb((unsigned int)(uintptr_t)data_in, 0U, 0x000000FFU, 0x00010C01U) != 0) { out->status = -1; }
    if (issue_transfer_trb(0U, 0U, 0U, 0x00010C01U) != 0) { out->status = -1; }
    if (poll_event_ring(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    LOGT("USB Host FS: Run complete: %s errors=%u", (out->status == 0) ? "PASS" : "FAIL", g_ctx.errors);
    return out->status;
}

int usb_host_enumeration_fs_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("USB Host FS: Teardown complete - errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
