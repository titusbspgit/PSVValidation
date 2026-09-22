// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_isochronous_transfer_test.h"
#include "test_define.inc"

/*
 * USB_FS_Device_Isochronous_Transfer_test
 * Validates USB Full-Speed Device mode Isochronous Transfer on the DWC USB3
 * controller. Covers soft reset, PHY config, event buffer setup, endpoint
 * configuration, USB enumeration, SET_ADDRESS, isochronous OUT transfers with
 * frame number polling, and interrupt-driven event handling.
 */

/* Testcase context structure */
typedef struct {
    unsigned int errors;
    volatile unsigned int int_pend;
    unsigned int event_counter;
    unsigned int rd_data;
} usb_iso_test_ctx_t;

static usb_iso_test_ctx_t g_ctx;

/* ------------------------------------------------------------------ */
/* Helper: set_configuration                                          */
/* Writes DEPCMDPAR1, DEPCMDPAR0, DEPCMD at the given endpoint offset */
/* and polls DEPCMD until the command completes.                      */
/* ------------------------------------------------------------------ */
static void set_configuration(unsigned int par1, unsigned int par0,
                              unsigned int ep_offset, unsigned int cmd)
{
 unsigned int timeout;

    writel_reg(MIZAR_USB_DEPCMDPAR1 + ep_offset, par1);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, par0);
    writel_reg(MIZAR_USB_DEPCMD + ep_offset, cmd);

    timeout = 10000U;
    while ((readl_reg(MIZAR_USB_DEPCMD + ep_offset) == cmd) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("set_configuration timeout: ep_offset=0x%x cmd=0x%x",
             ep_offset, cmd);
        g_ctx.errors++;
    }
}

/* ------------------------------------------------------------------ */
/* Helper: setup_stage                                                */
/* Writes TRB at event_trb_addr with Buffer_PointerLO, size, control, */
/* issues DEPCMD 0x506 on EP0 OUT, polls until complete, waits for    */
/* interrupt.                                                         */
/* ------------------------------------------------------------------ */
static void setup_stage(void)
{
 unsigned int timeout;

    /* Write TRB: buffer pointer low, size 0x8, control 0x823 */
    writel_reg(event_trb_addr, Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x04, 0x0);
    writel_reg(event_trb_addr + 0x08, 0x8);
    writel_reg(event_trb_addr + 0x0C, 0x823);

    /* Issue start transfer on EP0 OUT */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x0);
    writel_reg(MIZAR_USB_DEPCMDPAR0, event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD, 0x506);

    timeout = 10000U;
    while ((readl_reg(MIZAR_USB_DEPCMD) == 0x506) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("setup_stage DEPCMD poll timeout");
        g_ctx.errors++;
    }

    /* Wait for interrupt */
    g_ctx.int_pend = 1U;
    timeout = 10000U;
    while ((g_ctx.int_pend != 0U) && (timeout > 0U)) {
        wait_on(100);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("setup_stage interrupt wait timeout");
        g_ctx.errors++;
    }
}

/* ------------------------------------------------------------------ */
/* Helper: status_stage                                               */
/* Writes TRB at event_trb_addr with size 0x0, control 0x853, issues  */
/* DEPCMD 0x506 on EP1 IN (offset 0x10), polls, waits for interrupt.  */
/* ------------------------------------------------------------------ */
static void status_stage(void)
{
 unsigned int timeout;

    writel_reg(event_trb_addr, Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x04, 0x0);
    writel_reg(event_trb_addr + 0x08, 0x0);
    writel_reg(event_trb_addr + 0x0C, 0x853);

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, 0x0);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

    timeout = 10000U;
    while ((readl_reg(MIZAR_USB_DEPCMD + 0x10) == 0x506) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("status_stage DEPCMD poll timeout");
        g_ctx.errors++;
    }

    g_ctx.int_pend = 1U;
    timeout = 10000U;
    while ((g_ctx.int_pend != 0U) && (timeout > 0U)) {
        wait_on(100);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("status_stage interrupt wait timeout");
        g_ctx.errors++;
    }
}

/* ------------------------------------------------------------------ */
/* Helper: data_stage_ep1in                                           */
/* Writes TRB at event_trb_addr, issues DEPCMD 0x506 on EP1 IN       */
/* (offset 0x10), polls, waits for interrupt.                         */
/* ------------------------------------------------------------------ */
static void data_stage_ep1in(unsigned int buf_lo, unsigned int size,
                             unsigned int control)
{
 unsigned int timeout;

    writel_reg(event_trb_addr, buf_lo);
    writel_reg(event_trb_addr + 0x04, 0x0);
    writel_reg(event_trb_addr + 0x08, size);
    writel_reg(event_trb_addr + 0x0C, control);

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, 0x0);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

    timeout = 10000U;
    while ((readl_reg(MIZAR_USB_DEPCMD + 0x10) == 0x506) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("data_stage_ep1in DEPCMD poll timeout");
        g_ctx.errors++;
    }

    g_ctx.int_pend = 1U;
    timeout = 10000U;
    while ((g_ctx.int_pend != 0U) && (timeout > 0U)) {
        wait_on(100);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("data_stage_ep1in interrupt wait timeout");
        g_ctx.errors++;
    }
}

/* ------------------------------------------------------------------ */
/* Helper: enumeration                                                */
/* Performs GET_DEVICE_DESCRIPTOR, GET_CONFIGURATION_DESCRIPTOR,       */
/* SET_CONFIGURATION, GET_FULL_CONFIGURATION_DESCRIPTOR with progress  */
/* markers 0xdeadbee1 through 0xdeadbee5.                             */
/* ------------------------------------------------------------------ */
static void enumeration(void)
{
 unsigned int timeout;

    /* GET_DEVICE_DESCRIPTOR */
    LOGT("enumeration: GET_DEVICE_DESCRIPTOR");
    setup_stage();
    writel_reg(Buffer_PointerLO, 0x02000012);
    data_stage_ep1in(Buffer_PointerLO, 0x12, 0x853);
    status_stage();
    writel_reg(0xA0243ffc, 0xdeadbee1);
    LOGT("enumeration: progress marker 0xdeadbee1");

    /* GET_CONFIGURATION_DESCRIPTOR */
    LOGT("enumeration: GET_CONFIGURATION_DESCRIPTOR");
    setup_stage();
    writel_reg(Buffer_PointerLO, 0x003c0209);
    data_stage_ep1in(Buffer_PointerLO, 0x09, 0x853);
    status_stage();
    writel_reg(0xA0243ffc, 0xdeadbee2);
    LOGT("enumeration: progress marker 0xdeadbee2");

    /* SET_CONFIGURATION */
    LOGT("enumeration: SET_CONFIGURATION");
    setup_stage();
    writel_reg(Buffer_PointerLO, 0x00);
    data_stage_ep1in(Buffer_PointerLO, 0x0, 0x853);
    status_stage();
    writel_reg(0xA0243ffc, 0xdeadbee3);
    LOGT("enumeration: progress marker 0xdeadbee3");

    /* GET_FULL_CONFIGURATION_DESCRIPTOR (60-byte config descriptor) */
    LOGT("enumeration: GET_FULL_CONFIGURATION_DESCRIPTOR");
    setup_stage();
    // MANUAL_REVIEW: Full 60-byte configuration descriptor data written to
    // Buffer_PointerLO offsets 0x00-0x38 is described in the test procedure
    // but exact per-offset values are not fully enumerated in the Meta TestPlan
    // JSON. The first DWORD 0x003c0209 is known. Remaining offsets require
    // manual population from the USB descriptor specification.
    writel_reg(Buffer_PointerLO, 0x003c0209);
    data_stage_ep1in(Buffer_PointerLO, 0x3c, 0x853);
    status_stage();
    writel_reg(0xA0243ffc, 0xdeadbee4);
    LOGT("enumeration: progress marker 0xdeadbee4");

    writel_reg(0xA0243ffc, 0xdeadbee5);
    LOGT("enumeration: progress marker 0xdeadbee5");
}

/* ------------------------------------------------------------------ */
/* Helper: wait_for_interrupt                                         */
/* Sets int_pend=1 and polls with timeout until cleared by IRQ handler*/
/* ------------------------------------------------------------------ */
static void wait_for_interrupt(void)
{
 unsigned int timeout;

    g_ctx.int_pend = 1U;
    timeout = 10000U;
    while ((g_ctx.int_pend != 0U) && (timeout > 0U)) {
        wait_on(100);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("wait_for_interrupt timeout");
        g_ctx.errors++;
    }
}

/* ------------------------------------------------------------------ */
/* Default_IRQHandler (Step 48)                                       */
/* Clears int_pend, reads sysreg status, acknowledges USB events,     */
/* clears sysreg interrupt, clears GIC IRQ 84.                        */
/* ------------------------------------------------------------------ */
void Default_IRQHandler(void)
{
 unsigned int rd_data;
    unsigned int event_count;

    /* Clear int_pend */
    g_ctx.int_pend = 0U;

    /* Read system register masked status */
    rd_data = readl_reg(MIZAR_LSS_SYSREG_MSK_STS0);

    /* Read system register raw status */
    rd_data = readl_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

    /* Read and acknowledge USB event count */
    event_count = readl_reg(MIZAR_USB_GEVNTCOUNT);
    writel_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    g_ctx.event_counter += event_count;

    /* Clear sysreg interrupt if bit 31 is set */
    if (rd_data && 0x80000000) {
        writel_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    /* Clear GIC IRQ 84 */
    GIC_ClearIRQ(84);
}

/*
 * Function: usb_fs_device_isochronous_transfer_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              usb_fs_device_isochronous_transfer_test. Calls platform init
 *              and enables GIC interrupts.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_isochronous_transfer_test_init(const TestsItem *cfg)
{
 (void)cfg;

    /* Zero the testcase context */
    g_ctx = (usb_iso_test_ctx_t){0};

    LOGT("USB FS Device Isochronous Transfer test init");

    /* Step 1: Platform initialization */
    nic_programming();
    LOGT("nic_programming() complete");

    /* Step 2: Enable all GIC interrupts */
    GIC_EnableAllIRQ();
    LOGT("GIC_EnableAllIRQ() complete");

    return 0;
}

/*
 * Function: usb_fs_device_isochronous_transfer_test_run
 * Description: Executes the main testcase flow for
 *              usb_fs_device_isochronous_transfer_test. Performs soft reset,
 *              PHY configuration, event buffer setup, endpoint configuration,
 *              USB enumeration, SET_ADDRESS, and two isochronous OUT transfers
 *              with frame number polling.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_isochronous_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
 unsigned int j;
    unsigned int i;
    unsigned int rd_data;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("USB ISO test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting USB FS Device Isochronous Transfer test run");

    /* Step 3: Clear 20 DWORDs at Buffer_PointerLO and event_trb_addr */
    LOGT("Step 3: Clearing Buffer_PointerLO and event_trb_addr memory regions");
    for (j = 0U; j < 20U; j++) {
        writel_reg(Buffer_PointerLO + (j * DWORD), 0x0);
        writel_reg(event_trb_addr + (j * DWORD), 0x0);
    }

    /* Step 4: Perform soft reset - write 0x40f00000 to MIZAR_USB_DCTL */
    LOGT("Step 4: Soft reset - writing 0x40f00000 to MIZAR_USB_DCTL");
    writel_reg(MIZAR_USB_DCTL, 0x40f00000);

    /* Step 5: Poll MIZAR_USB_DCTL until read value equals 0xf00000 */
    LOGT("Step 5: Polling MIZAR_USB_DCTL for soft reset completion");
    timeout = 10000U;
    do {
        wait_on(100);
        rd_data = readl_reg(MIZAR_USB_DCTL);
        timeout--;
    } while ((rd_data != 0xf00000) && (timeout > 0U));
    if (timeout == 0U) {
        LOGE("Soft reset poll timeout: MIZAR_USB_DCTL=0x%x", rd_data);
        g_ctx.errors++;
    } else {
        LOGT("Soft reset complete: MIZAR_USB_DCTL=0x%x", rd_data);
    }

    /* Step 6: Write 0x40002407 to MIZAR_USB_GUSB2PHYCFG */
    LOGT("Step 6: USB2 PHY configuration");
    writel_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002407);

    /* Step 7: Write Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO */
    LOGT("Step 7: Event buffer address low");
    writel_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);

    /* Step 8: Write 0x0 to MIZAR_USB_GEVNTADRHI */
    LOGT("Step 8: Event buffer address high");
    writel_reg(MIZAR_USB_GEVNTADRHI, 0x0);

    /* Step 9: Write 0x30 to MIZAR_USB_GEVNTSIZ */
    LOGT("Step 9: Event buffer size = 0x30");
    writel_reg(MIZAR_USB_GEVNTSIZ, 0x30);

    /* Step 10: Write 0x0 to MIZAR_USB_GEVNTCOUNT */
    LOGT("Step 10: Clear event count");
    writel_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    /* Step 11: Read MIZAR_USB_GCTL, then write 0x30c12214 */
    LOGT("Step 11: Configure GCTL for device mode");
    rd_data = readl_reg(MIZAR_USB_GCTL);
    writel_reg(MIZAR_USB_GCTL, 0x30c12214);

    /* Step 12: Read MIZAR_USB_DCFG, then write 0x480801 */
    LOGT("Step 12: Device configuration");
    rd_data = readl_reg(MIZAR_USB_DCFG);
    writel_reg(MIZAR_USB_DCFG, 0x480801);

    /* Step 13: Write 0x1f to MIZAR_USB_DEVTEN */
    LOGT("Step 13: Enable device events");
    writel_reg(MIZAR_USB_DEVTEN, 0x1f);

    /* Step 14: Read MIZAR_USB_GUCTL, then write 0xa400010 */
    LOGT("Step 14: Configure GUCTL");
    rd_data = readl_reg(MIZAR_USB_GUCTL);
    writel_reg(MIZAR_USB_GUCTL, 0xa400010);

    /* Step 15: START_NEW_CONFIGURATION command */
    LOGT("Step 15: set_configuration START_NEW_CONFIGURATION");
    set_configuration(0, 0, 0, 0x409);

    /* Step 16: Configure 8 physical endpoints */
    // MANUAL_REVIEW: The Meta TestPlan JSON states set_configuration() is called
    // 8 times with varying parameter0/parameter1 values for physical endpoint
    // configuration (offsets 0x00-0x70, command 0x401). The exact per-endpoint
    // parameter0 and parameter1 values are not individually enumerated in the
    // Meta TestPlan JSON. Placeholder calls are generated below. Populate the
    // exact par1/par0 values from the DV source or USB endpoint configuration.
    LOGT("Step 16: Configure 8 physical endpoints");
    set_configuration(0, 0, 0x00, 0x401);
    set_configuration(0, 0, 0x10, 0x401);
    set_configuration(0, 0, 0x20, 0x401);
    set_configuration(0, 0, 0x30, 0x401);
    set_configuration(0, 0, 0x40, 0x401);
    set_configuration(0, 0, 0x50, 0x401);
    set_configuration(0, 0, 0x60, 0x401);
    set_configuration(0, 0, 0x70, 0x401);

    /* Step 17: TX resource allocation for 8 endpoints */
    LOGT("Step 17: TX resource allocation loop");
    for (i = 0U; i < 8U; i++) {
        writel_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
        writel_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);

        timeout = 10000U;
        while ((readl_reg(MIZAR_USB_DEPCMD + (i * 0x10)) == 0x402) && (timeout > 0U)) {
            timeout--;
        }
        if (timeout == 0U) {
            LOGE("TX resource alloc timeout: endpoint %u", i);
            g_ctx.errors++;
        }
    }

    /* Step 18: Write 0x3 to MIZAR_USB_DALEPENA */
    LOGT("Step 18: Enable EP0 IN/OUT");
    writel_reg(MIZAR_USB_DALEPENA, 0x3);

    /* Step 19: Write 0x80f00000 to MIZAR_USB_DCTL (run/stop) */
    LOGT("Step 19: DCTL run/stop");
    writel_reg(MIZAR_USB_DCTL, 0x80f00000);

    /* Step 20: Write 0x80000000 to MIZAR_LSS_SYSREG_INTR_EN0 */
    LOGT("Step 20: Enable sysreg interrupt");
    writel_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);

    /* Step 21: Wait for link state connect/reset events */
    LOGT("Step 21: Waiting for link state connect/reset events");
    wait_for_interrupt();
    wait_for_interrupt();

    /* Step 22: Write 0x480801 to MIZAR_USB_DCFG */
    LOGT("Step 22: Re-write DCFG");
    writel_reg(MIZAR_USB_DCFG, 0x480801);

    /* Step 23: Wait for additional events if event_counter <= 0x4 */
    LOGT("Step 23: Wait for additional events if needed");
    if (g_ctx.event_counter <= 0x4) {
        wait_for_interrupt();
    }

    /* Step 24: Write 0xdeadbee0 to 0xA0243ffc */
    LOGT("Step 24: Progress marker 0xdeadbee0");
    writel_reg(0xA0243ffc, 0xdeadbee0);

    /* Step 25: Read MIZAR_USB_DCFG, read MIZAR_USB_DSTS */
    LOGT("Step 25: Read DCFG and DSTS");
    rd_data = readl_reg(MIZAR_USB_DCFG);
    rd_data = readl_reg(MIZAR_USB_DSTS);

    /* Step 26: Write 0x480801 to DCFG, write 0x80f00a00 to DCTL */
    LOGT("Step 26: Update DCFG and DCTL");
    writel_reg(MIZAR_USB_DCFG, 0x480801);
    writel_reg(MIZAR_USB_DCTL, 0x80f00a00);

    /* Step 27: Write 0xff to MIZAR_USB_DALEPENA */
    LOGT("Step 27: Enable all 8 endpoints");
    writel_reg(MIZAR_USB_DALEPENA, 0xff);

    /* Step 28: Wait 5000 cycles */
    LOGT("Step 28: Wait 5000 cycles");
    wait_on(5000);

    /* Step 29: setup_stage() */
    LOGT("Step 29: Setup stage");
    setup_stage();

    /* Step 30: Write 0x40002547 to MIZAR_USB_GUSB2PHYCFG */
    LOGT("Step 30: Update PHY config");
    writel_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

    /* Step 31: Wait for events via int_pend loops */
    LOGT("Step 31: Wait for events");
    wait_for_interrupt();

    /* Step 32: SET_ADDRESS - write 0x480809 to MIZAR_USB_DCFG */
    LOGT("Step 32: SET_ADDRESS");
    writel_reg(MIZAR_USB_DCFG, 0x480809);

    /* Step 33: Write TRB at event_trb_addr, issue DEPCMD+0x10 with 0x506 */
    LOGT("Step 33: Status stage TRB on EP1 IN");
    writel_reg(event_trb_addr, Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x04, 0x0);
    writel_reg(event_trb_addr + 0x08, 0x0);
    writel_reg(event_trb_addr + 0x0C, 0x853);
    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, 0x0);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    timeout = 10000U;
    while ((readl_reg(MIZAR_USB_DEPCMD + 0x10) == 0x506) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Step 33 DEPCMD poll timeout");
        g_ctx.errors++;
    }

    /* Step 34: Wait for events, poll 0xa0243ff4 until non-zero */
    LOGT("Step 34: Wait for events and handshake polling");
    wait_for_interrupt();
    timeout = 10000U;
    do {
        rd_data = readl_reg(0xa0243ff4);
        wait_on(100);
        timeout--;
    } while ((rd_data == 0U) && (timeout > 0U));
    if (timeout == 0U) {
        LOGE("Handshake 1 poll timeout at 0xa0243ff4");
        g_ctx.errors++;
    } else {
        LOGT("Handshake 1 passed: 0xa0243ff4=0x%x", rd_data);
    }

    /* Step 35: enumeration() */
    LOGT("Step 35: USB enumeration");
    enumeration();

    /* Step 36: Poll 0xa0243ff8 until non-zero */
    LOGT("Step 36: Handshake 2 polling");
    timeout = 10000U;
    do {
        rd_data = readl_reg(0xa0243ff8);
        wait_on(100);
        timeout--;
    } while ((rd_data == 0U) && (timeout > 0U));
    if (timeout == 0U) {
        LOGE("Handshake 2 poll timeout at 0xa0243ff8");
        g_ctx.errors++;
    } else {
        LOGT("Handshake 2 passed: 0xa0243ff8=0x%x", rd_data);
    }

    /* Step 37: Poll MIZAR_USB_DSTS until bits[11:3] != 0 */
    LOGT("Step 37: Frame number validation (bits[11:3] != 0)");
    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DSTS);
        wait_on(100);
        timeout--;
    } while (((rd_data & 0xFF8) == 0U) && (timeout > 0U));
    if (timeout == 0U) {
        LOGE("Frame number poll timeout: DSTS=0x%x", rd_data);
        g_ctx.errors++;
    } else {
        LOGT("Frame number valid: DSTS=0x%x bits[11:3]=0x%x", rd_data, (rd_data >> 3) & 0x1FF);
    }

    /* Step 38: Write 0xdeadbee6 to 0xA0243ffc */
    LOGT("Step 38: Progress marker 0xdeadbee6");
    writel_reg(0xA0243ffc, 0xdeadbee6);

    /* Step 39: ISOCHRONOUS OUT transfer 1 */
    LOGT("Step 39: Isochronous OUT transfer 1 on EP offset 0x60");
    writel_reg(event_trb_addr, Buffer_PointerLO_1);
    writel_reg(event_trb_addr + 0x04, 0x0);
    writel_reg(event_trb_addr + 0x08, 0x3ff);
    writel_reg(event_trb_addr + 0x0C, 0x869);
    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x60, 0x0);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x60, event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x60, 0x20506);
    timeout = 10000U;
    while ((readl_reg(MIZAR_USB_DEPCMD + 0x60) == 0x20506) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("ISO transfer 1 DEPCMD poll timeout");
        g_ctx.errors++;
    }
    wait_for_interrupt();

    /* Step 40: Poll MIZAR_USB_DSTS until bits[5:3] == 0x2 */
    LOGT("Step 40: Poll DSTS for frame number 2");
    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DSTS);
        wait_on(100);
        timeout--;
    } while ((((rd_data >> 3) & 0x7) != 0x2) && (timeout > 0U));
    if (timeout == 0U) {
        LOGE("Frame number 2 poll timeout: DSTS=0x%x", rd_data);
        g_ctx.errors++;
    } else {
        LOGT("Frame number 2 reached: DSTS=0x%x", rd_data);
    }

    /* Step 41: Write 0xdeadbee7 to 0xA0243ffc, wait for interrupt */
    LOGT("Step 41: Progress marker 0xdeadbee7");
    writel_reg(0xA0243ffc, 0xdeadbee7);
    wait_for_interrupt();

    /* Step 42: Poll MIZAR_USB_DSTS until bits[5:3] == 0x3 */
    LOGT("Step 42: Poll DSTS for frame number 3");
    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DSTS);
        wait_on(100);
        timeout--;
    } while ((((rd_data >> 3) & 0x7) != 0x3) && (timeout > 0U));
    if (timeout == 0U) {
        LOGE("Frame number 3 poll timeout: DSTS=0x%x", rd_data);
        g_ctx.errors++;
    } else {
        LOGT("Frame number 3 reached: DSTS=0x%x", rd_data);
    }

    /* Step 43: Write 0xdeadbee8 to 0xA0243ffc, wait for interrupt */
    LOGT("Step 43: Progress marker 0xdeadbee8");
    writel_reg(0xA0243ffc, 0xdeadbee8);
    wait_for_interrupt();

    /* Step 44: ISOCHRONOUS OUT transfer 2 on EP offset 0x70 */
    LOGT("Step 44: Isochronous OUT transfer 2 on EP offset 0x70");
    writel_reg(event_trb_addr, Buffer_PointerLO_1);
    writel_reg(event_trb_addr + 0x04, 0x0);
    writel_reg(event_trb_addr + 0x08, 0x3ff);
    writel_reg(event_trb_addr + 0x0C, 0x869);
    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x70, 0x0);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x70, event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x70, 0x40506);
    timeout = 10000U;
    while ((readl_reg(MIZAR_USB_DEPCMD + 0x70) == 0x40506) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("ISO transfer 2 DEPCMD poll timeout");
        g_ctx.errors++;
    }
    wait_for_interrupt();

    /* Step 45: Poll MIZAR_USB_DSTS until bits[5:3] == 0x4 */
    LOGT("Step 45: Poll DSTS for frame number 4");
    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DSTS);
        wait_on(100);
        timeout--;
    } while ((((rd_data >> 3) & 0x7) != 0x4) && (timeout > 0U));
    if (timeout == 0U) {
        LOGE("Frame number 4 poll timeout: DSTS=0x%x", rd_data);
        g_ctx.errors++;
    } else {
        LOGT("Frame number 4 reached: DSTS=0x%x", rd_data);
    }

    /* Step 46: Write 0xdeadbee9 to 0xA0243ffc, wait for interrupt */
    LOGT("Step 46: Progress marker 0xdeadbee9");
    writel_reg(0xA0243ffc, 0xdeadbee9);
    wait_for_interrupt();

    /* Step 47: DV finish(0) converted to PSV/FV status reporting */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to
    // PSV/FV-native out->status based PASS/FAIL reporting per FV Template.

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: usb_fs_device_isochronous_transfer_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 *              usb_fs_device_isochronous_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_isochronous_transfer_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

    LOGT("USB FS Device Isochronous Transfer test teardown: errors=%u",
         g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
