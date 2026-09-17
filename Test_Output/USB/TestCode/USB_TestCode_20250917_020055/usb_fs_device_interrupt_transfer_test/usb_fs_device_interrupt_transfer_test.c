// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_interrupt_transfer_test.h"
#include "test_define.inc"

/*
 * USB_FS_Device_Interrupt_Transfer_test
 *
 * This test validates USB Full-Speed Device mode Interrupt Transfer functionality.
 * The test performs a soft reset of the USB controller, configures the USB2 PHY,
 * sets up the event buffer, configures the global controller for device mode,
 * configures device settings and enables device events, configures all endpoints
 * including interrupt endpoint types, allocates TX resources, enables physical
 * endpoints, starts the controller, enables system-level interrupts, waits for
 * link state connect and reset events, performs full USB enumeration (SET ADDRESS,
 * GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR short and full,
 * SET CONFIGURATION), performs handshake synchronization, initiates interrupt data
 * transfers on two interrupt endpoints, polls DSTS for a valid frame number,
 * and verifies completion.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
    volatile unsigned int int_pend;
} usb_intr_test_ctx_t;

static usb_intr_test_ctx_t g_ctx;

/*
 * Function: poll_depcmd_completion
 * Description: Polls the DEPCMD register until the command active bit clears.
 * Parameters:
 *   ep_offset - Endpoint offset for the DEPCMD register.
 *   timeout - Maximum poll iterations.
 * Returns:
 *   0 on success, -1 on timeout.
 */
static int poll_depcmd_completion(unsigned int ep_offset, unsigned int timeout)
{
    unsigned int val;
    unsigned int count = timeout;

    do {
        val = readl_reg(MIZAR_USB_DEPCMD + ep_offset);
        if ((val & (1U << 10)) == 0U) {
            return 0;
        }
        count--;
    } while (count > 0U);

    LOGE("USB Intr: DEPCMD poll timeout at ep_offset=0x%x", ep_offset);
    return -1;
}

/*
 * Function: wait_for_interrupt
 * Description: Polls the int_pend flag until an interrupt is received or timeout.
 * Parameters:
 *   timeout - Maximum poll iterations.
 * Returns:
 *   0 on success, -1 on timeout.
 */
static int wait_for_interrupt(unsigned int timeout)
{
    unsigned int count = timeout;

    while (g_ctx.int_pend == 0U) {
        if (count == 0U) {
            LOGE("USB Intr: interrupt wait timeout");
            return -1;
        }
        count--;
    }
    g_ctx.int_pend = 0U;
    return 0;
}

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for USB events. Reads GEVNTCOUNT, writes back
 *              to acknowledge, clears sysreg interrupt, and clears GIC IRQ.
 * Parameters:
 *   None.
 * Returns:
 *   None.
 */
void Default_IRQHandler(void)
{
    unsigned int evt_count;
    unsigned int sysreg_sts;

    /* Read system register interrupt status */
    sysreg_sts = readl_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("USB Intr IRQ: sysreg_sts=0x%x", sysreg_sts);

    /* Read and write back event count to acknowledge USB events */
    evt_count = readl_reg(MIZAR_USB_GEVNTCOUNT);
    if (evt_count > 0U) {
        writel_reg(MIZAR_USB_GEVNTCOUNT, evt_count);
        LOGT("USB Intr IRQ: GEVNTCOUNT acknowledged, count=%u", evt_count);
    }

    /* Clear system register interrupt */
    writel_reg(MIZAR_LSS_SYSREG_RAW_STCR0, sysreg_sts);

    /* Signal interrupt pending */
    g_ctx.int_pend = 1U;

    /* Clear GIC IRQ */
    // MANUAL_REVIEW: GIC IRQ clear mechanism is platform-specific.
}

/*
 * Function: set_configuration
 * Description: Issues endpoint configuration commands using DEPCMDPAR1, DEPCMDPAR0,
 *              and DEPCMD registers for a given endpoint offset.
 * Parameters:
 *   ep_offset - Endpoint register offset.
 *   par1_val - Value for DEPCMDPAR1.
 *   par0_val - Value for DEPCMDPAR0.
 *   cmd_val - Value for DEPCMD.
 * Returns:
 *   0 on success, -1 on failure.
 */
static int set_configuration(unsigned int ep_offset, unsigned int par1_val,
                             unsigned int par0_val, unsigned int cmd_val)
{
    writel_reg(MIZAR_USB_DEPCMDPAR1 + ep_offset, par1_val);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, par0_val);
    writel_reg(MIZAR_USB_DEPCMD + ep_offset, cmd_val);

    if (poll_depcmd_completion(ep_offset, 10000U) != 0) {
        g_ctx.errors++;
        return -1;
    }
    g_ctx.checks_passed++;
    g_ctx.checks_total++;
    return 0;
}

/*
 * Function: issue_start_transfer
 * Description: Prepares a TRB and issues a Start Transfer command on an endpoint.
 * Parameters:
 *   ep_offset - Endpoint register offset.
 *   trb_buf_addr - TRB buffer low address.
 *   trb_size - Transfer size.
 *   trb_ctrl - TRB control value.
 * Returns:
 *   0 on success, -1 on failure.
 */
static int issue_start_transfer(unsigned int ep_offset, unsigned int trb_buf_addr,
                                unsigned int trb_size, unsigned int trb_ctrl)
{
    /* Write TRB buffer pointer low */
    writel_reg(Buffer_PointerLO + ep_offset, trb_buf_addr);

    /* Write TRB size */
    writel_reg(Buffer_PointerLO + ep_offset + 0x08U, trb_size);

    /* Write TRB control */
    writel_reg(Buffer_PointerLO + ep_offset + 0x0CU, trb_ctrl);

    /* Issue Start Transfer command via DEPCMD */
    writel_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, 0U);
    writel_reg(MIZAR_USB_DEPCMD + ep_offset, 0x00000406U);

    if (poll_depcmd_completion(ep_offset, 10000U) != 0) {
        g_ctx.errors++;
        return -1;
    }
    return 0;
}

/*
 * Function: usb_fs_device_interrupt_transfer_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              usb_fs_device_interrupt_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_interrupt_transfer_test_init(const TestsItem *cfg)
{
    unsigned int i;
    unsigned int val;

    (void)cfg;

    /* Initialize test context */
    g_ctx = (usb_intr_test_ctx_t){0};

    LOGT("USB Intr: Init - starting USB FS Device Interrupt Transfer test");

    /* Step 1: Initialize the system by calling NIC programming and enabling all GIC IRQs */
    // MANUAL_REVIEW: NIC programming and GIC IRQ enable are platform-specific.
    LOGT("USB Intr: Step 1 - System initialization (NIC programming, GIC IRQ enable)");

    /* Step 2: Clear the data buffer and event TRB buffer regions by writing zeros to 20 DWORD entries */
    LOGT("USB Intr: Step 2 - Clearing data buffer and event TRB buffer regions");
    for (i = 0U; i < 20U; i++) {
        writel_reg(Buffer_PointerLO + (i * DWORD), 0x00000000U);
    }
    for (i = 0U; i < 20U; i++) {
        writel_reg(event_trb_addr + (i * DWORD), 0x00000000U);
    }

    /* Step 3: Initiate a soft reset of the USB controller by writing to the DCTL register */
    LOGT("USB Intr: Step 3 - Initiating USB controller soft reset via DCTL");
    writel_reg(MIZAR_USB_DCTL, (1U << 30));
    {
        unsigned int timeout = 10000U;
        do {
            val = readl_reg(MIZAR_USB_DCTL);
            if ((val & (1U << 30)) == 0U) {
                break;
            }
            timeout--;
        } while (timeout > 0U);
        if (timeout == 0U) {
            LOGE("USB Intr: Step 3 - DCTL soft reset timeout");
            g_ctx.errors++;
            return -1;
        }
    }
    g_ctx.checks_passed++;
    g_ctx.checks_total++;
    LOGT("USB Intr: Step 3 - Soft reset complete, DCTL=0x%x", val);

    /* Step 4: Configure the USB2 PHY by writing to the GUSB2PHYCFG register */
    LOGT("USB Intr: Step 4 - Configuring USB2 PHY via GUSB2PHYCFG");
    val = readl_reg(MIZAR_USB_GUSB2PHYCFG);
    // MANUAL_REVIEW: Exact PHY configuration value is platform-specific.
    writel_reg(MIZAR_USB_GUSB2PHYCFG, val);

    /* Step 5: Set up the event buffer */
    LOGT("USB Intr: Step 5 - Setting up event buffer");
    writel_reg(MIZAR_USB_GEVNTADRLO, (unsigned int)(uintptr_t)Default_Event_Ring_Array);
    writel_reg(MIZAR_USB_GEVNTADRHI, 0x00000000U);
    writel_reg(MIZAR_USB_GEVNTSIZ, 0x00000050U);
    writel_reg(MIZAR_USB_GEVNTCOUNT, 0x00000000U);

    /* Step 6: Configure the global controller for device mode via GCTL */
    LOGT("USB Intr: Step 6 - Configuring GCTL for device mode");
    val = readl_reg(MIZAR_USB_GCTL);
    val = (val & ~(0x3U << 12)) | (0x2U << 12);
    writel_reg(MIZAR_USB_GCTL, val);

    /* Step 7: Configure device settings via DCFG, DEVTEN, GUCTL */
    LOGT("USB Intr: Step 7 - Configuring DCFG, DEVTEN, GUCTL");
    val = readl_reg(MIZAR_USB_DCFG);
    // MANUAL_REVIEW: Exact DCFG configuration value depends on device speed and settings.
    writel_reg(MIZAR_USB_DCFG, val);
    writel_reg(MIZAR_USB_DEVTEN, 0x00000007U);
    val = readl_reg(MIZAR_USB_GUCTL);
    writel_reg(MIZAR_USB_GUCTL, val);

    LOGT("USB Intr: Init complete");
    return 0;
}

/*
 * Function: usb_fs_device_interrupt_transfer_test_run
 * Description: Executes the main testcase flow for usb_fs_device_interrupt_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_interrupt_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int val;
    unsigned int i;
    unsigned int ep_offset;

    (void)cfg;

    if (out == 0) {
        LOGE("USB Intr: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB Intr: Run - starting main testcase flow");

    /* Step 8: Issue Start New Configuration and Set Endpoint Configuration commands */
    /*         for 8 endpoints including interrupt endpoint types */
    LOGT("USB Intr: Step 8 - Configuring 8 endpoints (including interrupt types) via DEPCMDPAR1, DEPCMDPAR0, DEPCMD");
    for (i = 0U; i < 8U; i++) {
        ep_offset = i * 0x10U;
        // MANUAL_REVIEW: Exact DEPCMDPAR1, DEPCMDPAR0, DEPCMD values for each endpoint
        // depend on endpoint type (control, interrupt, bulk) and configuration.
        if (set_configuration(ep_offset, 0U, 0U, 0x00000409U) != 0) {
            LOGE("USB Intr: Step 8 - Endpoint %u configuration failed", i);
            out->status = -1;
        }
    }

    /* Step 9: Allocate TX resources for 8 endpoints */
    LOGT("USB Intr: Step 9 - Allocating TX resources for 8 endpoints");
    for (i = 0U; i < 8U; i++) {
        ep_offset = i * 0x10U;
        writel_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, 0x00000001U);
        writel_reg(MIZAR_USB_DEPCMD + ep_offset, 0x00000802U);
        if (poll_depcmd_completion(ep_offset, 10000U) != 0) {
            LOGE("USB Intr: Step 9 - TX resource allocation failed for endpoint %u", i);
            g_ctx.errors++;
            out->status = -1;
        } else {
            g_ctx.checks_passed++;
        }
        g_ctx.checks_total++;
    }

    /* Step 10: Enable physical endpoints 0 and 1 via DALEPENA */
    LOGT("USB Intr: Step 10 - Enabling physical endpoints 0 and 1");
    writel_reg(MIZAR_USB_DALEPENA, 0x00000003U);

    /* Step 11: Start the USB controller by writing the run bit to DCTL */
    LOGT("USB Intr: Step 11 - Starting USB controller via DCTL run bit");
    val = readl_reg(MIZAR_USB_DCTL);
    val |= (1U << 31);
    writel_reg(MIZAR_USB_DCTL, val);

    /* Step 12: Enable system-level interrupts at the sysreg level */
    LOGT("USB Intr: Step 12 - Enabling system-level interrupts via SYSREG");
    writel_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0xFFFFFFFFU);

    /* Step 13: Wait for link state connect and reset events via interrupt-driven polling */
    LOGT("USB Intr: Step 13 - Waiting for link state connect event");
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 13 - Link state connect event timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 13 - Link state connect event received");
    }
    g_ctx.checks_total++;

    LOGT("USB Intr: Step 13 - Waiting for reset event");
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 13 - Reset event timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 13 - Reset event received");
    }
    g_ctx.checks_total++;

    /* Step 14: Read device status from DSTS, reconfigure DCFG, update DCTL, enable all endpoints */
    LOGT("USB Intr: Step 14 - Reading DSTS and reconfiguring device");
    val = readl_reg(MIZAR_USB_DSTS);
    LOGT("USB Intr: Step 14 - DSTS=0x%x", val);

    val = readl_reg(MIZAR_USB_DCFG);
    // MANUAL_REVIEW: Update DCFG based on DSTS speed information.
    writel_reg(MIZAR_USB_DCFG, val);

    val = readl_reg(MIZAR_USB_DCTL);
    // MANUAL_REVIEW: Set accept connection bits in DCTL.
    writel_reg(MIZAR_USB_DCTL, val);

    writel_reg(MIZAR_USB_DALEPENA, 0x000000FFU);

    /* Step 15: Execute setup stage - prepare TRB and issue Start Transfer on control endpoint */
    LOGT("USB Intr: Step 15 - Executing setup stage TRB on control endpoint");
    if (issue_start_transfer(0U, 0U, 8U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 15 - Setup stage Start Transfer failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 15 - Setup stage transfer complete interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 15 - Setup stage transfer complete");
    }
    g_ctx.checks_total++;

    /* Step 16: Update the USB2 PHY configuration and wait for related interrupts */
    LOGT("USB Intr: Step 16 - Updating USB2 PHY configuration");
    val = readl_reg(MIZAR_USB_GUSB2PHYCFG);
    // MANUAL_REVIEW: Exact PHY update value is platform-specific.
    writel_reg(MIZAR_USB_GUSB2PHYCFG, val);
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 16 - PHY update interrupt timeout");
        g_ctx.errors++;
    }

    /* Step 17: Handle SET ADDRESS - update DCFG with assigned device address */
    LOGT("USB Intr: Step 17 - Handling SET ADDRESS");
    val = readl_reg(MIZAR_USB_DCFG);
    // MANUAL_REVIEW: Set device address bits in DCFG from setup packet data.
    writel_reg(MIZAR_USB_DCFG, val);

    /* Prepare status TRB and issue Start Transfer */
    if (issue_start_transfer(0U, 0U, 0U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 17 - SET ADDRESS status stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 17 - SET ADDRESS transfer complete timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 17 - SET ADDRESS complete");
    }
    g_ctx.checks_total++;

    /* Step 18: Perform handshake synchronization by polling dedicated memory location */
    LOGT("USB Intr: Step 18 - Handshake synchronization (polling 0xa0243ff4)");
    {
        unsigned int hs_timeout = 100000U;
        do {
            val = readl_reg(0xa0243ff4U);
            if (val != 0U) {
                break;
            }
            hs_timeout--;
        } while (hs_timeout > 0U);
        if (hs_timeout == 0U) {
            LOGE("USB Intr: Step 18 - Handshake sync timeout at 0xa0243ff4");
            g_ctx.errors++;
            out->status = -1;
        } else {
            g_ctx.checks_passed++;
            LOGT("USB Intr: Step 18 - Handshake sync complete, val=0x%x", val);
        }
        g_ctx.checks_total++;
    }

    /* Step 19: GET DEVICE DESCRIPTOR - populate buffer with device descriptor data */
    LOGT("USB Intr: Step 19 - Responding to GET DEVICE DESCRIPTOR");
    /* Prepare setup stage TRB */
    if (issue_start_transfer(0U, 0U, 8U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 19 - Setup stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 19 - Setup stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    }
    g_ctx.checks_total++;

    /* Populate buffer with device descriptor data */
    // MANUAL_REVIEW: Device descriptor data values are device-specific.
    // Populate buf_data[] with the 18-byte USB device descriptor.

    /* Issue data stage transfer */
    if (issue_start_transfer(0U, (unsigned int)(uintptr_t)buf_data, 18U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 19 - Data stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 19 - Data stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
    }
    g_ctx.checks_total++;

    /* Issue status stage transfer */
    if (issue_start_transfer(0U, 0U, 0U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 19 - Status stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 19 - Status stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 19 - GET DEVICE DESCRIPTOR complete");
    }
    g_ctx.checks_total++;

    /* Step 20: GET CONFIGURATION DESCRIPTOR (short) */
    LOGT("USB Intr: Step 20 - Responding to GET CONFIGURATION DESCRIPTOR (short)");
    if (issue_start_transfer(0U, 0U, 8U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 20 - Setup stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 20 - Setup stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    }
    g_ctx.checks_total++;

    /* Populate buffer with configuration descriptor header */
    // MANUAL_REVIEW: Configuration descriptor header values are device-specific.

    if (issue_start_transfer(0U, (unsigned int)(uintptr_t)buf_data, 9U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 20 - Data stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 20 - Data stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
    }
    g_ctx.checks_total++;

    if (issue_start_transfer(0U, 0U, 0U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 20 - Status stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 20 - Status stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 20 - GET CONFIGURATION DESCRIPTOR (short) complete");
    }
    g_ctx.checks_total++;

    /* Step 21: SET CONFIGURATION - issue zero-length status data stage transfer */
    LOGT("USB Intr: Step 21 - Responding to SET CONFIGURATION");
    if (issue_start_transfer(0U, 0U, 8U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 21 - Setup stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 21 - Setup stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    }
    g_ctx.checks_total++;

    if (issue_start_transfer(0U, 0U, 0U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 21 - Status stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 21 - Status stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 21 - SET CONFIGURATION complete");
    }
    g_ctx.checks_total++;

    /* Step 22: GET CONFIGURATION DESCRIPTOR (full) with interface and endpoint descriptors */
    /*          for interrupt and bulk endpoints */
    LOGT("USB Intr: Step 22 - Responding to GET CONFIGURATION DESCRIPTOR (full)");
    if (issue_start_transfer(0U, 0U, 8U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 22 - Setup stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 22 - Setup stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    }
    g_ctx.checks_total++;

    /* Populate buffer with complete configuration descriptor including */
    /* interface and endpoint descriptors for interrupt and bulk endpoints */
    // MANUAL_REVIEW: Full configuration descriptor data values are device-specific.

    if (issue_start_transfer(0U, (unsigned int)(uintptr_t)buf_data, 32U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 22 - Data stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 22 - Data stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
    }
    g_ctx.checks_total++;

    if (issue_start_transfer(0U, 0U, 0U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 22 - Status stage failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 22 - Status stage interrupt timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 22 - GET CONFIGURATION DESCRIPTOR (full) complete");
    }
    g_ctx.checks_total++;

    /* Step 23: Perform second handshake synchronization by polling another dedicated memory location */
    LOGT("USB Intr: Step 23 - Second handshake synchronization (polling 0xa0243ff8)");
    {
        unsigned int hs_timeout2 = 100000U;
        do {
            val = readl_reg(0xa0243ff8U);
            if (val != 0U) {
                break;
            }
            hs_timeout2--;
        } while (hs_timeout2 > 0U);
        if (hs_timeout2 == 0U) {
            LOGE("USB Intr: Step 23 - Second handshake sync timeout at 0xa0243ff8");
            g_ctx.errors++;
            out->status = -1;
        } else {
            g_ctx.checks_passed++;
            LOGT("USB Intr: Step 23 - Second handshake sync complete, val=0x%x", val);
        }
        g_ctx.checks_total++;
    }

    /* Step 24: Initiate interrupt data transfers on two interrupt endpoints */
    /*          using 64-byte TRBs with interrupt TRB type (0x815) */
    LOGT("USB Intr: Step 24 - Initiating interrupt data transfers on two interrupt endpoints");

    /* Interrupt endpoint 1 - 64-byte transfer with interrupt TRB type */
    LOGT("USB Intr: Step 24 - Interrupt transfer on endpoint 1");
    if (issue_start_transfer(0x10U, (unsigned int)(uintptr_t)buf_data, 64U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 24 - Interrupt EP1 Start Transfer failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 24 - Interrupt EP1 transfer complete timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 24 - Interrupt EP1 transfer complete");
    }
    g_ctx.checks_total++;

    /* Interrupt endpoint 2 - 64-byte transfer with interrupt TRB type */
    LOGT("USB Intr: Step 24 - Interrupt transfer on endpoint 2");
    if (issue_start_transfer(0x20U, (unsigned int)(uintptr_t)buf_data, 64U, 0x00000815U) != 0) {
        LOGE("USB Intr: Step 24 - Interrupt EP2 Start Transfer failed");
        out->status = -1;
    }
    if (wait_for_interrupt(100000U) != 0) {
        LOGE("USB Intr: Step 24 - Interrupt EP2 transfer complete timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        g_ctx.checks_passed++;
        LOGT("USB Intr: Step 24 - Interrupt EP2 transfer complete");
    }
    g_ctx.checks_total++;

    /* Step 25: Poll the DSTS register to wait for a valid frame number */
    LOGT("USB Intr: Step 25 - Polling DSTS for valid frame number");
    {
        unsigned int dsts_timeout = 100000U;
        unsigned int frame_num;
        do {
            val = readl_reg(MIZAR_USB_DSTS);
            frame_num = (val >> 3) & 0x3FFFU;
            if (frame_num != 0U) {
                break;
            }
            dsts_timeout--;
        } while (dsts_timeout > 0U);
        if (dsts_timeout == 0U) {
            LOGE("USB Intr: Step 25 - DSTS frame number poll timeout, DSTS=0x%x", val);
            g_ctx.errors++;
            out->status = -1;
        } else {
            g_ctx.checks_passed++;
            LOGT("USB Intr: Step 25 - Valid frame number detected: %u, DSTS=0x%x", frame_num, val);
        }
        g_ctx.checks_total++;
    }

    /* Step 26: Verify test completion */
    // MANUAL_REVIEW: DV finish() was present in the source flow, but PSV/FV-native
    // completion uses out->status based PASS/FAIL reporting.
    g_ctx.checks_failed = g_ctx.errors;

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("USB Intr: Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/*
 * Function: usb_fs_device_interrupt_transfer_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 *              usb_fs_device_interrupt_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_interrupt_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB Intr: Teardown - Final validation");

    /* Validation 1: Soft reset must have completed (checked in init) */
    /* Validation 2: All endpoint commands must have completed (checked via poll_depcmd_completion) */
    /* Validation 3: All interrupt-driven events must have been serviced (checked via wait_for_interrupt) */
    /* Validation 4: GEVNTCOUNT read/writeback handled in Default_IRQHandler */
    /* Validation 5: System register interrupts cleared in Default_IRQHandler */
    /* Validation 6: Handshake synchronization verified in run (Steps 18, 23) */
    /* Validation 7: Full USB enumeration sequence completed in run (Steps 17, 19, 20, 21, 22) */
    /* Validation 8: Interrupt data transfers completed in run (Step 24) with interrupt TRB type */
    /* Validation 9: DSTS frame number validated in run (Step 25) */
    /* Validation 10: Test completion verified via out->status in run */

    LOGT("USB Intr: Teardown complete - errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return g_ctx.errors == 0U ? 0 : -1;
}
