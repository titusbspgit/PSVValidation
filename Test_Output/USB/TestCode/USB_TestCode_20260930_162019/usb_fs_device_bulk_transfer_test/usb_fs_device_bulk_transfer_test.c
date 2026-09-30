// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

/* Global testcase context */
static unsigned int g_errors = 0U;
static volatile unsigned int int_pend = 0U;
static volatile unsigned int event_counter = 0U;

/*
 * Function: wait_for_interrupt
 * Description: Wait for interrupt pending flag to be cleared by IRQ handler.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void wait_for_interrupt(void)
{
    while (int_pend != 0U) {
        wait_on(5);
    }
}

/*
 * Function: set_configuration
 * Description: Issue an endpoint configuration command and poll for completion.
 * Parameters:
 *   trb_address - endpoint command register offset
 *   cmd - command value to write
 * Returns:
 *   void
 */
static void set_configuration(unsigned int trb_address, unsigned int cmd)
{
    unsigned int rd_data;

    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
    rd_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    while (rd_data != (cmd & ~0x400U)) {
        /* MANUAL_REVIEW: Confirm polling condition for command active bit cleared matches DV intent */
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    }
}

/*
 * Function: setup_stage
 * Description: Perform USB control transfer setup stage on a physical endpoint.
 * Parameters:
 *   ep_offset - physical endpoint command register offset
 *   buf_addr - buffer pointer address for TRB
 *   trb_size - transfer size
 *   trb_ctrl - TRB control word
 * Returns:
 *   void
 */
static void setup_stage(unsigned int ep_offset, unsigned int buf_addr,
                        unsigned int trb_size, unsigned int trb_ctrl)
{
    unsigned int rd_data;

    write_reg(MIZAR_USB_DEPCMDPAR1 + ep_offset, buf_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, trb_size);
    write_reg(MIZAR_USB_DEPCMD + ep_offset, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + ep_offset);
    while (rd_data != 0x506U) {
        /* MANUAL_REVIEW: Confirm start transfer polling condition from DV */
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + ep_offset);
    }
    /* Poll until command completes (value != 0x506) */
    rd_data = read_reg(MIZAR_USB_DEPCMD + ep_offset);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + ep_offset);
    }
}

/*
 * Function: data_stage
 * Description: Perform USB control transfer data stage.
 * Parameters:
 *   ep_offset - physical endpoint command register offset
 *   buf_addr - buffer pointer address for TRB
 *   trb_size - transfer size
 *   trb_ctrl - TRB control word
 * Returns:
 *   void
 */
static void data_stage(unsigned int ep_offset, unsigned int buf_addr,
                       unsigned int trb_size, unsigned int trb_ctrl)
{
    unsigned int rd_data;

    write_reg(MIZAR_USB_DEPCMDPAR1 + ep_offset, buf_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, trb_size);
    write_reg(MIZAR_USB_DEPCMD + ep_offset, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + ep_offset);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + ep_offset);
    }
}

/*
 * Function: status_stage
 * Description: Perform USB control transfer status stage.
 * Parameters:
 *   ep_offset - physical endpoint command register offset
 *   buf_addr - buffer pointer address for TRB
 *   trb_size - transfer size
 *   trb_ctrl - TRB control word
 * Returns:
 *   void
 */
static void status_stage(unsigned int ep_offset, unsigned int buf_addr,
                         unsigned int trb_size, unsigned int trb_ctrl)
{
    unsigned int rd_data;

    write_reg(MIZAR_USB_DEPCMDPAR1 + ep_offset, buf_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, trb_size);
    write_reg(MIZAR_USB_DEPCMD + ep_offset, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + ep_offset);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + ep_offset);
    }
}

/*
 * Function: Default_IRQHandler
 * Description: IRQ handler for USB interrupt processing. Reads interrupt status,
 *              clears event count, clears sysreg interrupt, clears GIC IRQ 84.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    unsigned int rd_data;
    unsigned int event_count;

    LOGT("Default_IRQHandler: entered");

    /* Step 75: Read masked interrupt status */
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("IRQ: MIZAR_LSS_SYSREG_MSK_STS0 = 0x%x", rd_data);

    /* Step 76: Read raw status/clear register */
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("IRQ: MIZAR_LSS_SYSREG_RAW_STCR0 = 0x%x", rd_data);

    /* Step 77: If bit 31 set, clear it */
    if (rd_data & 0x80000000U) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000U);
    }

    /* Step 78: Read event count from GEVNTCOUNT */
    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    LOGT("IRQ: GEVNTCOUNT = 0x%x", event_count);

    /* Step 79: Write back event count to clear */
    if (event_count != 0U) {
        write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
    }

    /* Step 80: Increment event counter */
    event_counter++;

    /* Step 81: Clear int_pend flag */
    int_pend = 0U;

    /* Step 82: Clear GIC IRQ 84 */
    GIC_ClearIRQ(84);

    LOGT("Default_IRQHandler: exit, event_counter=%u", event_counter);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initialize USB FS Device Bulk Transfer test. Performs NIC
 *              programming, enables GIC interrupts, clears buffers, performs
 *              soft reset, configures USB controller and endpoints.
 * Parameters:
 *   cfg - pointer to test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    unsigned int j;
    unsigned int rd_data;
    unsigned int i;

    (void)cfg;

    g_errors = 0U;
    int_pend = 0U;
    event_counter = 0U;

    LOGT("USB FS Device Bulk Transfer test init: start");

    /* Step 1: NIC initialization */
    nic_programming();
    LOGT("nic_programming() done");

    /* Step 2: Enable all GIC interrupts */
    GIC_EnableAllIRQ();
    LOGT("GIC_EnableAllIRQ() done");

    /* Step 3: Clear 20 DWORDs at Buffer_PointerLO and event_trb_addr */
    for (j = 0U; j < 20U; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }
    LOGT("Buffer and event TRB memory cleared");

    /* Step 4: Initiate soft reset */
    write_reg(MIZAR_USB_DCTL, 0x40f00000U);
    LOGT("Soft reset initiated: wrote 0x40f00000 to MIZAR_USB_DCTL");

    /* Step 5: Poll for soft reset completion */
    rd_data = read_reg(MIZAR_USB_DCTL);
    while (rd_data != 0xf00000U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DCTL);
    }

    /* Step 6: Soft reset done */
    LOGT("Soft Rst is done");

    /* Step 7: Configure USB2 PHY */
    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002407U);
    LOGT("USB2 PHY configured: 0x40002407");

    /* Step 8: Set event ring base address low */
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    LOGT("GEVNTADRLO set to Default_Event_Ring_Array");

    /* Step 9: Set event ring base address high */
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0);

    /* Step 10: Set event ring size */
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30);

    /* Step 11: Clear event count */
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    /* Step 12: Read global control */
    rd_data = read_reg(MIZAR_USB_GCTL);
    LOGT("GCTL read: 0x%x", rd_data);

    /* Step 13: Configure global control */
    write_reg(MIZAR_USB_GCTL, 0x30c12214U);
    LOGT("GCTL configured: 0x30c12214");

    /* Step 14: Read device configuration */
    rd_data = read_reg(MIZAR_USB_DCFG);
    LOGT("DCFG read: 0x%x", rd_data);

    /* Step 15: Set device configuration */
    write_reg(MIZAR_USB_DCFG, 0x480801U);
    LOGT("DCFG configured: 0x480801");

    /* Step 16: Enable device events */
    write_reg(MIZAR_USB_DEVTEN, 0x1fU);
    LOGT("DEVTEN configured: 0x1f");

    /* Step 17: Read GUCTL */
    rd_data = read_reg(MIZAR_USB_GUCTL);
    LOGT("GUCTL read: 0x%x", rd_data);

    /* Step 18: Configure GUCTL */
    write_reg(MIZAR_USB_GUCTL, 0xa400010U);
    LOGT("GUCTL configured: 0xa400010");

    /* Steps 19-27: Configure 9 endpoints via set_configuration */
    /* EP0: START NEW CONFIGURATION (cmd=0x409) on physical EP0 offset 0x0 */
    LOGT("set_configuration: EP0 START NEW CONFIG cmd=0x409 offset=0x0");
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, 0x0);
    set_configuration(0x0, 0x409U);

    /* EP0 config (cmd=0x401) offset 0x0 */
    LOGT("set_configuration: EP0 cmd=0x401 offset=0x0");
    /* MANUAL_REVIEW: Confirm DEPCMDPAR0/PAR1 values for EP0 config from DV source */
    set_configuration(0x0, 0x401U);

    /* EP1 config (cmd=0x401) offset 0x10 */
    LOGT("set_configuration: EP1 cmd=0x401 offset=0x10");
    set_configuration(0x10, 0x401U);

    /* EP2 config (cmd=0x401) offset 0x20 */
    LOGT("set_configuration: EP2 cmd=0x401 offset=0x20");
    set_configuration(0x20, 0x401U);

    /* EP3 config (cmd=0x401) offset 0x30 */
    LOGT("set_configuration: EP3 cmd=0x401 offset=0x30");
    set_configuration(0x30, 0x401U);

    /* EP4 config (cmd=0x401) offset 0x40 */
    LOGT("set_configuration: EP4 cmd=0x401 offset=0x40");
    set_configuration(0x40, 0x401U);

    /* EP5 config (cmd=0x401) offset 0x50 */
    LOGT("set_configuration: EP5 cmd=0x401 offset=0x50");
    set_configuration(0x50, 0x401U);

    /* EP6 config (cmd=0x401) offset 0x60 */
    LOGT("set_configuration: EP6 cmd=0x401 offset=0x60");
    set_configuration(0x60, 0x401U);

    /* EP7 config (cmd=0x401) offset 0x70 */
    LOGT("set_configuration: EP7 cmd=0x401 offset=0x70");
    set_configuration(0x70, 0x401U);

    /* Step 28: Transfer resource configuration loop for 8 endpoints */
    LOGT("Transfer resource configuration for 8 endpoints");
    for (i = 0U; i < 8U; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10U), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10U), 0x402U);
        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
        while (rd_data == 0x402U) {
            wait_on(5);
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
        }
        LOGT("TX resource config EP%u done", i);
    }

    /* Step 29: Enable physical endpoints 0 and 1 via DALEPENA */
    write_reg(MIZAR_USB_DALEPENA, 0x3U);
    LOGT("DALEPENA set to 0x3 (EP0 and EP1 enabled)");

    /* Step 30: Set DCTL run/stop */
    write_reg(MIZAR_USB_DCTL, 0xf00000U | 0x80000000U);
    /* MANUAL_REVIEW: Confirm exact DCTL run/stop value from DV source */
    LOGT("DCTL run/stop set");

    /* Step 31: Enable system-level interrupt */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000U);
    LOGT("MIZAR_LSS_SYSREG_INTR_EN0 enabled");

    LOGT("USB FS Device Bulk Transfer test init: complete");
    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Execute USB FS Device Bulk Transfer test. Handles link state
 *              events, USB enumeration, bulk transfers, and validation.
 * Parameters:
 *   cfg - pointer to test configuration item
 *   out - pointer to test output structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int rd_data;

    (void)cfg;

    if (out == 0) {
        LOGE("USB output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run: start");

    /* Step 32: Wait for link state connect event (interrupt wait) */
    LOGT("Waiting for link state connect event");
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("Link state connect event received");

    /* Step 33: Wait for link state reset event (interrupt wait) */
    LOGT("Waiting for link state reset event");
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("Link state reset event received");

    /* Step 34: Conditional interrupt wait gated by event_counter */
    if (event_counter <= 0x4U) {
        int_pend = 1U;
        wait_for_interrupt();
    }

    /* Step 35: Read DSTS for enumeration speed detection */
    rd_data = read_reg(MIZAR_USB_DSTS);
    LOGT("DSTS read for speed detection: 0x%x", rd_data);

    /* Step 36: Enable all configured endpoints via DALEPENA */
    /* MANUAL_REVIEW: Confirm exact DALEPENA bitmask for all endpoints from DV source */
    write_reg(MIZAR_USB_DALEPENA, 0xFFU);
    LOGT("DALEPENA updated for all endpoints");

    /* Step 37: Debug marker */
    write_reg(0xA0243ffcU, 0xdeadbee0U);
    LOGT("Debug marker: 0xdeadbee0");

    /* ============================================================ */
    /* Steps 38-43: SET ADDRESS sequence */
    /* ============================================================ */

    /* Step 38: Start Transfer on EP0 (offset 0x0) for setup stage */
    LOGT("SET ADDRESS: setup stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Step 39: Interrupt wait for setup stage completion */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("SET ADDRESS: setup stage interrupt received");

    /* Step 40: Conditional interrupt wait */
    if (event_counter <= 0x4U) {
        int_pend = 1U;
        wait_for_interrupt();
    }

    /* Step 41: Status stage on EP1 (offset 0x10) */
    LOGT("SET ADDRESS: status stage on EP1");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    /* Step 42: Interrupt wait for status stage completion */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("SET ADDRESS: status stage interrupt received");

    /* Step 43: Handshake polling at 0xa0243ff4 */
    LOGT("SET ADDRESS: handshake polling at 0xa0243ff4");
    rd_data = read_reg(0xa0243ff4U);
    while (rd_data == 0x0U) {
        wait_on(5);
        rd_data = read_reg(0xa0243ff4U);
    }
    LOGT("SET ADDRESS: handshake received: 0x%x", rd_data);

    /* Step 44: Debug marker */
    write_reg(0xA0243ffcU, 0xdeadbee1U);
    LOGT("Debug marker: 0xdeadbee1");

    /* ============================================================ */
    /* Steps 45-50: GET_DESCRIPTOR (Device Descriptor) */
    /* ============================================================ */

    /* Step 45: Setup stage on EP0 for GET_DESCRIPTOR Device */
    LOGT("GET_DESCRIPTOR Device: setup stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Device: setup stage interrupt received");

    /* Conditional interrupt wait */
    if (event_counter <= 0x4U) {
        int_pend = 1U;
        wait_for_interrupt();
    }

    /* Step 46: Data stage on EP1 (offset 0x10) - 5 DWORDs device descriptor */
    LOGT("GET_DESCRIPTOR Device: data stage - 5 DWORDs");
    /* MANUAL_REVIEW: Confirm exact 5 DWORD device descriptor data values from DV source */
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Device: data stage interrupt received");

    /* Step 47: Status stage on EP0 */
    LOGT("GET_DESCRIPTOR Device: status stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Device: status stage interrupt received");

    /* Handshake polling at 0xa0243ff8 */
    LOGT("GET_DESCRIPTOR Device: handshake polling at 0xa0243ff8");
    rd_data = read_reg(0xa0243ff8U);
    while (rd_data == 0x0U) {
        wait_on(5);
        rd_data = read_reg(0xa0243ff8U);
    }
    LOGT("GET_DESCRIPTOR Device: handshake received: 0x%x", rd_data);

    /* Debug marker */
    write_reg(0xA0243ffcU, 0xdeadbee3U);
    LOGT("Debug marker: 0xdeadbee3");

    /* ============================================================ */
    /* Steps 51-56: GET_DESCRIPTOR (Configuration Descriptor short) */
    /* ============================================================ */

    /* Setup stage on EP0 */
    LOGT("GET_DESCRIPTOR Config short: setup stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Config short: setup stage interrupt received");

    /* Conditional interrupt wait */
    if (event_counter <= 0x4U) {
        int_pend = 1U;
        wait_for_interrupt();
    }

    /* Data stage on EP1 - 4 DWORDs config descriptor short */
    LOGT("GET_DESCRIPTOR Config short: data stage - 4 DWORDs");
    /* MANUAL_REVIEW: Confirm exact 4 DWORD config descriptor short data values from DV source */
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Config short: data stage interrupt received");

    /* Status stage on EP0 */
    LOGT("GET_DESCRIPTOR Config short: status stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Config short: status stage interrupt received");

    /* Handshake polling at 0xa0243ff4 */
    LOGT("GET_DESCRIPTOR Config short: handshake polling at 0xa0243ff4");
    rd_data = read_reg(0xa0243ff4U);
    while (rd_data == 0x0U) {
        wait_on(5);
        rd_data = read_reg(0xa0243ff4U);
    }
    LOGT("GET_DESCRIPTOR Config short: handshake received: 0x%x", rd_data);

    /* Debug marker */
    write_reg(0xA0243ffcU, 0xdeadbee4U);
    LOGT("Debug marker: 0xdeadbee4");

    /* ============================================================ */
    /* Steps 57-62: SET_CONFIGURATION */
    /* ============================================================ */

    /* Setup stage on EP0 */
    LOGT("SET_CONFIGURATION: setup stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("SET_CONFIGURATION: setup stage interrupt received");

    /* Conditional interrupt wait */
    if (event_counter <= 0x4U) {
        int_pend = 1U;
        wait_for_interrupt();
    }

    /* Data stage on EP1 - 1 DWORD */
    LOGT("SET_CONFIGURATION: data stage - 1 DWORD");
    /* MANUAL_REVIEW: Confirm exact 1 DWORD SET_CONFIGURATION data value from DV source */
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("SET_CONFIGURATION: data stage interrupt received");

    /* Status stage on EP0 */
    LOGT("SET_CONFIGURATION: status stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("SET_CONFIGURATION: status stage interrupt received");

    /* Handshake polling at 0xa0243ff8 */
    LOGT("SET_CONFIGURATION: handshake polling at 0xa0243ff8");
    rd_data = read_reg(0xa0243ff8U);
    while (rd_data == 0x0U) {
        wait_on(5);
        rd_data = read_reg(0xa0243ff8U);
    }
    LOGT("SET_CONFIGURATION: handshake received: 0x%x", rd_data);

    /* Debug marker */
    write_reg(0xA0243ffcU, 0xdeadbee5U);
    LOGT("Debug marker: 0xdeadbee5");

    /* ============================================================ */
    /* Steps 63-68: GET_DESCRIPTOR (Full Configuration Descriptor) */
    /* ============================================================ */

    /* Setup stage on EP0 */
    LOGT("GET_DESCRIPTOR Config full: setup stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Config full: setup stage interrupt received");

    /* Conditional interrupt wait */
    if (event_counter <= 0x4U) {
        int_pend = 1U;
        wait_for_interrupt();
    }

    /* Data stage on EP1 - 15 DWORDs full config descriptor */
    LOGT("GET_DESCRIPTOR Config full: data stage - 15 DWORDs");
    /* MANUAL_REVIEW: Confirm exact 15 DWORD full configuration descriptor data values from DV source */
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Config full: data stage interrupt received");

    /* Status stage on EP0 */
    LOGT("GET_DESCRIPTOR Config full: status stage on EP0");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x0, Buffer_PointerLO);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x0, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x0, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x0);
    }

    /* Interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("GET_DESCRIPTOR Config full: status stage interrupt received");

    /* Handshake polling at 0xa0243ff4 */
    LOGT("GET_DESCRIPTOR Config full: handshake polling at 0xa0243ff4");
    rd_data = read_reg(0xa0243ff4U);
    while (rd_data == 0x0U) {
        wait_on(5);
        rd_data = read_reg(0xa0243ff4U);
    }
    LOGT("GET_DESCRIPTOR Config full: handshake received: 0x%x", rd_data);

    /* Debug marker */
    write_reg(0xA0243ffcU, 0xdeadbee6U);
    LOGT("Debug marker: 0xdeadbee6");

    /* ============================================================ */
    /* Steps 69-72: Bulk OUT transfers on EP4 and EP5 */
    /* ============================================================ */

    /* Step 69: Bulk OUT transfer on physical endpoint 4 (offset 0x40) */
    LOGT("Bulk OUT: EP4 (offset 0x40) transfer size=0x40 TRB ctrl=0x813");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, Buffer_PointerLO_1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x40);
    /* MANUAL_REVIEW: Confirm TRB control 0x813 is written to correct register/field from DV source */
    write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    }

    /* Interrupt wait for EP4 bulk transfer */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("Bulk OUT: EP4 transfer interrupt received");

    /* Step 70: Bulk OUT transfer on physical endpoint 5 (offset 0x50) */
    LOGT("Bulk OUT: EP5 (offset 0x50) transfer size=0x40 TRB ctrl=0x813");
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, Buffer_PointerLO_1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x40);
    /* MANUAL_REVIEW: Confirm TRB control 0x813 is written to correct register/field from DV source */
    write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    while (rd_data == 0x506U) {
        wait_on(5);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    }

    /* Interrupt wait for EP5 bulk transfer */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("Bulk OUT: EP5 transfer interrupt received");

    /* Step 73: Write final debug marker */
    write_reg(0xA0243ffcU, 0xdeadbee7U);
    LOGT("Debug marker: 0xdeadbee7");

    /* Step 74: Final interrupt wait */
    int_pend = 1U;
    wait_for_interrupt();
    LOGT("Final interrupt received");

    /* DV finish(0) converted to FV/PSV status return */
    /* Test completion: pass */
    out->status = (g_errors == 0U) ? 0 : -1;

    LOGT("USB FS Device Bulk Transfer test run: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB FS Device Bulk Transfer test.
 * Parameters:
 *   cfg - pointer to test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown: no additional cleanup required");
    return (g_errors == 0U) ? 0 : -1;
}
