// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

// INTEGRATION NOTE: This file contains MANUAL_REVIEW items that require
// platform-specific implementation. Search for "MANUAL_REVIEW" to locate them.

#include "usb_fs_device_isochronous_transfer_test.h"
#include "test_define.inc"

/*
 * USB Full-Speed Device Isochronous Transfer Test
 * Validates USB FS Device mode Isochronous Transfer on the DWC USB3 controller.
 * Performs soft reset, PHY config, event buffer setup, endpoint configuration,
 * USB enumeration, and two isochronous OUT transfers with frame number validation.
 */

/* Global variables for interrupt-driven flow */
static volatile unsigned int int_pend = 0U;
static volatile unsigned int event_counter = 0U;
static uint64_t rd_data = 0U;
static uint64_t event_count = 0U;

/* Test context */
typedef struct {
    unsigned int errors;
} usb_fs_iso_test_ctx_t;

static usb_fs_iso_test_ctx_t g_ctx;

/*
 * Helper: usb_fs_device_isochronous_transfer_test_set_configuration
 * Writes DEPCMDPAR1, DEPCMDPAR0, DEPCMD at the given endpoint offset
 * and polls DEPCMD until the command completes.
 */
static void usb_fs_device_isochronous_transfer_test_set_configuration(
    uint64_t ep_offset, uint64_t par1, uint64_t par0, uint64_t cmd)
{
    unsigned int timeout;

    writel_reg(MIZAR_USB_DEPCMDPAR1 + (uintptr_t)ep_offset, par1);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + (uintptr_t)ep_offset, par0);
    writel_reg(MIZAR_USB_DEPCMD + (uintptr_t)ep_offset, cmd);

    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DEPCMD + (uintptr_t)ep_offset);
        if (timeout == 0U) {
            LOGE("set_configuration timeout at ep_offset=0x%llx cmd=0x%llx",
                 (unsigned long long)ep_offset, (unsigned long long)cmd);
            g_ctx.errors++;
            return;
        }
        timeout--;
    } while (rd_data == cmd);

    LOGT("set_configuration complete: ep_offset=0x%llx cmd=0x%llx",
         (unsigned long long)ep_offset, (unsigned long long)cmd);
}

/*
 * Helper: usb_fs_device_isochronous_transfer_test_setup_stage
 * Writes a setup TRB at event_trb_addr with Buffer_PointerLO, size 0x8,
 * control 0x823, issues DEPCMD 0x506 on EP0 OUT, polls until complete,
 * and waits for interrupt.
 */
static void usb_fs_device_isochronous_transfer_test_setup_stage(void)
{
    unsigned int timeout;

    /* Write TRB: buffer pointer low, size, control */
    writel_reg((uintptr_t)event_trb_addr, (uint64_t)Buffer_PointerLO);
    writel_reg((uintptr_t)event_trb_addr + 0x04U, 0x0U);
    writel_reg((uintptr_t)event_trb_addr + 0x08U, 0x8U);
    writel_reg((uintptr_t)event_trb_addr + 0x0CU, 0x823U);

    /* Issue start transfer command on EP0 OUT */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x0U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, (uint64_t)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD, 0x506U);

    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DEPCMD);
        if (timeout == 0U) {
            LOGE("setup_stage DEPCMD poll timeout");
            g_ctx.errors++;
            return;
        }
        timeout--;
    } while (rd_data == 0x506U);

    /* Wait for interrupt */
    int_pend = 1U;
    timeout = 10000U;
    while (int_pend != 0U) {
        if (timeout == 0U) {
            LOGE("setup_stage interrupt wait timeout");
            g_ctx.errors++;
            break;
        }
        timeout--;
    }
    event_counter++;
    LOGT("setup_stage complete, event_counter=%u", event_counter);
}

/*
 * Helper: usb_fs_device_isochronous_transfer_test_status_stage
 * Writes a status TRB at event_trb_addr with Buffer_PointerLO, size 0x0,
 * control 0x853, issues DEPCMD 0x506 on EP1 IN (+0x10), polls until complete,
 * and waits for interrupt.
 */
static void usb_fs_device_isochronous_transfer_test_status_stage(void)
{
    unsigned int timeout;

    /* Write TRB: buffer pointer low, size 0, control status */
    writel_reg((uintptr_t)event_trb_addr, (uint64_t)Buffer_PointerLO);
    writel_reg((uintptr_t)event_trb_addr + 0x04U, 0x0U);
    writel_reg((uintptr_t)event_trb_addr + 0x08U, 0x0U);
    writel_reg((uintptr_t)event_trb_addr + 0x0CU, 0x853U);

    /* Issue start transfer command on EP1 IN */
    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, 0x0U);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, (uint64_t)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);

    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10U);
        if (timeout == 0U) {
            LOGE("status_stage DEPCMD poll timeout");
            g_ctx.errors++;
            return;
        }
        timeout--;
    } while (rd_data == 0x506U);

    /* Wait for interrupt */
    int_pend = 1U;
    timeout = 10000U;
    while (int_pend != 0U) {
        if (timeout == 0U) {
            LOGE("status_stage interrupt wait timeout");
            g_ctx.errors++;
            break;
        }
        timeout--;
    }
    event_counter++;
    LOGT("status_stage complete, event_counter=%u", event_counter);
}

/*
 * Helper: usb_fs_device_isochronous_transfer_test_data_stage_ep1in
 * Writes a data TRB at event_trb_addr with Buffer_PointerLO, given size,
 * given control, issues DEPCMD 0x506 on EP1 IN (+0x10), polls until complete,
 * and waits for interrupt.
 */
static void usb_fs_device_isochronous_transfer_test_data_stage_ep1in(
    uint64_t trb_size, uint64_t trb_ctrl)
{
    unsigned int timeout;

    writel_reg((uintptr_t)event_trb_addr, (uint64_t)Buffer_PointerLO);
    writel_reg((uintptr_t)event_trb_addr + 0x04U, 0x0U);
    writel_reg((uintptr_t)event_trb_addr + 0x08U, trb_size);
    writel_reg((uintptr_t)event_trb_addr + 0x0CU, trb_ctrl);

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, 0x0U);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, (uint64_t)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);

    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10U);
        if (timeout == 0U) {
            LOGE("data_stage_ep1in DEPCMD poll timeout");
            g_ctx.errors++;
            return;
        }
        timeout--;
    } while (rd_data == 0x506U);

    int_pend = 1U;
    timeout = 10000U;
    while (int_pend != 0U) {
        if (timeout == 0U) {
            LOGE("data_stage_ep1in interrupt wait timeout");
            g_ctx.errors++;
            break;
        }
        timeout--;
    }
    event_counter++;
    LOGT("data_stage_ep1in complete, event_counter=%u", event_counter);
}

/*
 * Helper: usb_fs_device_isochronous_transfer_test_enumeration
 * Performs full USB enumeration: GET_DEVICE_DESCRIPTOR, GET_CONFIGURATION_DESCRIPTOR,
 * SET_CONFIGURATION, GET_FULL_CONFIGURATION_DESCRIPTOR with progress markers.
 */
static void usb_fs_device_isochronous_transfer_test_enumeration(void)
{
    unsigned int timeout;

    /* --- GET_DEVICE_DESCRIPTOR --- */
    LOGT("Enumeration: GET_DEVICE_DESCRIPTOR");
    usb_fs_device_isochronous_transfer_test_setup_stage();

    /* Data stage: write device descriptor to Buffer_PointerLO */
    writel_reg((uintptr_t)Buffer_PointerLO, 0x02000012U);
    usb_fs_device_isochronous_transfer_test_data_stage_ep1in(0x12U, 0x853U);

    usb_fs_device_isochronous_transfer_test_status_stage();

    /* Progress marker */
    writel_reg(0xA0243ffcU, 0xdeadbee1U);
    LOGT("Enumeration: GET_DEVICE_DESCRIPTOR done, marker=0xdeadbee1");

    /* --- GET_CONFIGURATION_DESCRIPTOR --- */
    LOGT("Enumeration: GET_CONFIGURATION_DESCRIPTOR");
    usb_fs_device_isochronous_transfer_test_setup_stage();

    /* Data stage: write config descriptor to Buffer_PointerLO */
    writel_reg((uintptr_t)Buffer_PointerLO, 0x003c0209U);
    usb_fs_device_isochronous_transfer_test_data_stage_ep1in(0x09U, 0x853U);

    usb_fs_device_isochronous_transfer_test_status_stage();

    /* Progress marker */
    writel_reg(0xA0243ffcU, 0xdeadbee2U);
    LOGT("Enumeration: GET_CONFIGURATION_DESCRIPTOR done, marker=0xdeadbee2");

    /* --- SET_CONFIGURATION --- */
    LOGT("Enumeration: SET_CONFIGURATION");
    usb_fs_device_isochronous_transfer_test_setup_stage();

    /* Data stage: write 0x00 */
    writel_reg((uintptr_t)Buffer_PointerLO, 0x00U);
    usb_fs_device_isochronous_transfer_test_data_stage_ep1in(0x0U, 0x853U);

    usb_fs_device_isochronous_transfer_test_status_stage();

    /* Progress marker */
    writel_reg(0xA0243ffcU, 0xdeadbee3U);
    LOGT("Enumeration: SET_CONFIGURATION done, marker=0xdeadbee3");

    /* --- GET_FULL_CONFIGURATION_DESCRIPTOR (60 bytes) --- */
    LOGT("Enumeration: GET_FULL_CONFIGURATION_DESCRIPTOR");
    usb_fs_device_isochronous_transfer_test_setup_stage();

    /* Write full 60-byte config descriptor to Buffer_PointerLO offsets 0x00-0x38 */
    // MANUAL_REVIEW: Full 60-byte configuration descriptor data
    // Original intent: Write complete configuration descriptor (60 bytes) to Buffer_PointerLO
    // at offsets 0x00 through 0x38. The exact descriptor byte content is not fully
    // specified in the Meta TestPlan JSON. Only the first DWORD (0x003c0209) is known.
    // PSV action: Populate the full descriptor data per USB spec and device configuration.
    writel_reg((uintptr_t)Buffer_PointerLO, 0x003c0209U);
    /* Remaining descriptor DWORDs at offsets 0x04-0x38 need platform-specific data */

    usb_fs_device_isochronous_transfer_test_data_stage_ep1in(0x3CU, 0x853U);

    usb_fs_device_isochronous_transfer_test_status_stage();

    /* Progress markers */
    writel_reg(0xA0243ffcU, 0xdeadbee4U);
    LOGT("Enumeration: GET_FULL_CONFIGURATION_DESCRIPTOR done, marker=0xdeadbee4");

    writel_reg(0xA0243ffcU, 0xdeadbee5U);
    LOGT("Enumeration: complete, marker=0xdeadbee5");
}

/*
 * Helper: usb_fs_device_isochronous_transfer_test_wait_interrupt
 * Waits for int_pend to be cleared by the IRQ handler with timeout.
 */
static void usb_fs_device_isochronous_transfer_test_wait_interrupt(void)
{
    unsigned int timeout = 10000U;

    int_pend = 1U;
    while (int_pend != 0U) {
        if (timeout == 0U) {
            LOGE("Interrupt wait timeout");
            g_ctx.errors++;
            return;
        }
        timeout--;
    }
    event_counter++;
}

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler. Reads sysreg status, acknowledges USB
 *   events via GEVNTCOUNT, clears sysreg interrupt, and clears GIC IRQ 84.
 * Step 48 from test procedure.
 */
void Default_IRQHandler(void)
{
    uint64_t msk_sts;
    uint64_t raw_sts;

    /* Clear int_pend flag */
    int_pend = 0U;

    /* Read system register masked status */
    msk_sts = readl_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("IRQHandler: MSK_STS0=0x%llx", (unsigned long long)msk_sts);

    /* Read system register raw status */
    raw_sts = readl_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("IRQHandler: RAW_STCR0=0x%llx", (unsigned long long)raw_sts);

    /* Read event count and acknowledge */
    event_count = readl_reg(MIZAR_USB_GEVNTCOUNT);
    writel_reg(MIZAR_USB_GEVNTCOUNT, event_count);
    LOGT("IRQHandler: GEVNTCOUNT=0x%llx acknowledged", (unsigned long long)event_count);

    /* Clear sysreg interrupt if bit 31 is set */
    /* NOTE: Original DV code used && (logical AND) instead of & (bitwise AND). */
    /* Using bitwise AND as the correct hardware check. */
    if (raw_sts & 0x80000000U) {
        writel_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000U);
        LOGT("IRQHandler: sysreg interrupt cleared");
    }

    /* Clear GIC IRQ 84 */
    GIC_ClearIRQ(84);
    LOGT("IRQHandler: GIC IRQ 84 cleared");
}

/*
 * Function: usb_fs_device_isochronous_transfer_test_init
 * Description: Performs testcase initialization and pre-condition setup for usb_fs_device_isochronous_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_isochronous_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (usb_fs_iso_test_ctx_t){0};
    int_pend = 0U;
    event_counter = 0U;
    rd_data = 0U;
    event_count = 0U;

    LOGT("USB FS Device Isochronous Transfer test init");

    /* Step 1: Platform initialization */
    nic_programming();
    LOGT("nic_programming() called");

    /* Step 2: Enable all GIC IRQ interrupts */
    GIC_EnableAllIRQ();
    LOGT("GIC_EnableAllIRQ() called");

    return 0;
}

/*
 * Function: usb_fs_device_isochronous_transfer_test_run
 * Description: Executes the main testcase flow for usb_fs_device_isochronous_transfer_test.
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
    unsigned int timeout;
    uint64_t dsts_val;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Isochronous Transfer test run start");

    /* Step 3: Clear 20 DWORDs at Buffer_PointerLO and event_trb_addr */
    LOGT("Step 3: Clearing data buffer and event TRB memory regions");
    for (j = 0U; j < 20U; j++) {
        writel_reg((uintptr_t)Buffer_PointerLO + (uintptr_t)(j * DWORD), 0x0U);
        writel_reg((uintptr_t)event_trb_addr + (uintptr_t)(j * DWORD), 0x0U);
    }

    /* Step 4: Perform soft reset - write 0x40f00000 to MIZAR_USB_DCTL */
    LOGT("Step 4: Soft reset - writing 0x40f00000 to DCTL");
    writel_reg(MIZAR_USB_DCTL, 0x40f00000U);

    /* Step 5: Poll MIZAR_USB_DCTL until read value equals 0xf00000 */
    LOGT("Step 5: Polling DCTL for soft reset completion");
    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DCTL);
        if (timeout == 0U) {
            LOGE("Soft reset poll timeout: DCTL=0x%llx", (unsigned long long)rd_data);
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
        timeout--;
    } while (rd_data != 0xf00000U);
    LOGT("Soft reset complete: DCTL=0x%llx", (unsigned long long)rd_data);

    /* Step 6: Write 0x40002407 to GUSB2PHYCFG */
    LOGT("Step 6: USB2 PHY configuration");
    writel_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002407U);

    /* Step 7: Write Default_Event_Ring_Array to GEVNTADRLO */
    LOGT("Step 7: Event buffer address low");
    writel_reg(MIZAR_USB_GEVNTADRLO, (uint64_t)Default_Event_Ring_Array);

    /* Step 8: Write 0x0 to GEVNTADRHI */
    LOGT("Step 8: Event buffer address high");
    writel_reg(MIZAR_USB_GEVNTADRHI, 0x0U);

    /* Step 9: Write 0x30 to GEVNTSIZ */
    LOGT("Step 9: Event buffer size = 0x30 (48 bytes)");
    writel_reg(MIZAR_USB_GEVNTSIZ, 0x30U);

    /* Step 10: Write 0x0 to GEVNTCOUNT */
    LOGT("Step 10: Clear event count");
    writel_reg(MIZAR_USB_GEVNTCOUNT, 0x0U);

    /* Step 11: Read GCTL, then write 0x30c12214 */
    LOGT("Step 11: Configure GCTL for device mode");
    rd_data = readl_reg(MIZAR_USB_GCTL);
    LOGT("GCTL read=0x%llx, writing 0x30c12214", (unsigned long long)rd_data);
    writel_reg(MIZAR_USB_GCTL, 0x30c12214U);

    /* Step 12: Read DCFG, then write 0x480801 */
    LOGT("Step 12: Device configuration");
    rd_data = readl_reg(MIZAR_USB_DCFG);
    LOGT("DCFG read=0x%llx, writing 0x480801", (unsigned long long)rd_data);
    writel_reg(MIZAR_USB_DCFG, 0x480801U);

    /* Step 13: Write 0x1f to DEVTEN */
    LOGT("Step 13: Enable device events (DEVTEN=0x1f)");
    writel_reg(MIZAR_USB_DEVTEN, 0x1fU);

    /* Step 14: Read GUCTL, then write 0xa400010 */
    LOGT("Step 14: Configure GUCTL");
    rd_data = readl_reg(MIZAR_USB_GUCTL);
    LOGT("GUCTL read=0x%llx, writing 0xa400010", (unsigned long long)rd_data);
    writel_reg(MIZAR_USB_GUCTL, 0xa400010U);

    /* Step 15: START_NEW_CONFIGURATION command */
    LOGT("Step 15: set_configuration(0,0,0,0x409) - START_NEW_CONFIGURATION");
    usb_fs_device_isochronous_transfer_test_set_configuration(0x0U, 0x0U, 0x0U, 0x409U);

    /* Step 16: Configure 8 physical endpoints */
    LOGT("Step 16: Configure 8 physical endpoints via set_configuration()");
    // MANUAL_REVIEW: Endpoint Configuration Parameters
    // Original intent: Call set_configuration() 8 times with varying par1, par0 values
    // for physical endpoint configuration at offsets 0x00-0x70 with command 0x401.
    // The exact par1 and par0 values for each of the 8 endpoints are not fully
    // enumerated in the Meta TestPlan JSON. Using placeholder zero values.
    // PSV action: Fill in the correct DEPCMDPAR1 and DEPCMDPAR0 values for each endpoint.
    usb_fs_device_isochronous_transfer_test_set_configuration(0x00U, 0x0U, 0x0U, 0x401U);
    usb_fs_device_isochronous_transfer_test_set_configuration(0x10U, 0x0U, 0x0U, 0x401U);
    usb_fs_device_isochronous_transfer_test_set_configuration(0x20U, 0x0U, 0x0U, 0x401U);
    usb_fs_device_isochronous_transfer_test_set_configuration(0x30U, 0x0U, 0x0U, 0x401U);
    usb_fs_device_isochronous_transfer_test_set_configuration(0x40U, 0x0U, 0x0U, 0x401U);
    usb_fs_device_isochronous_transfer_test_set_configuration(0x50U, 0x0U, 0x0U, 0x401U);
    usb_fs_device_isochronous_transfer_test_set_configuration(0x60U, 0x0U, 0x0U, 0x401U);
    usb_fs_device_isochronous_transfer_test_set_configuration(0x70U, 0x0U, 0x0U, 0x401U);

    /* Step 17: TX resource allocation for 8 endpoints */
    LOGT("Step 17: TX resource allocation for 8 endpoints");
    for (i = 0U; i < 8U; i++) {
        writel_reg(MIZAR_USB_DEPCMDPAR0 + (uintptr_t)(i * 0x10U), 0x1U);
        writel_reg(MIZAR_USB_DEPCMD + (uintptr_t)(i * 0x10U), 0x402U);

        timeout = 10000U;
        do {
            rd_data = readl_reg(MIZAR_USB_DEPCMD + (uintptr_t)(i * 0x10U));
            if (timeout == 0U) {
                LOGE("TX resource alloc timeout for EP index %u", i);
                g_ctx.errors++;
                break;
            }
            timeout--;
        } while (rd_data == 0x402U);
        LOGT("TX resource allocated for EP index %u", i);
    }

    /* Step 18: Write 0x3 to DALEPENA (enable EP0 IN/OUT) */
    LOGT("Step 18: Enable EP0 IN/OUT (DALEPENA=0x3)");
    writel_reg(MIZAR_USB_DALEPENA, 0x3U);

    /* Step 19: Write 0x80f00000 to DCTL (run/stop with keep connect) */
    LOGT("Step 19: Start controller (DCTL=0x80f00000)");
    writel_reg(MIZAR_USB_DCTL, 0x80f00000U);

    /* Step 20: Write 0x80000000 to MIZAR_LSS_SYSREG_INTR_EN0 */
    LOGT("Step 20: Enable sysreg interrupt");
    writel_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000U);

    /* Step 21: Wait for link state connect/reset events */
    LOGT("Step 21: Waiting for link state connect/reset events");
    usb_fs_device_isochronous_transfer_test_wait_interrupt();
    usb_fs_device_isochronous_transfer_test_wait_interrupt();

    /* Step 22: Write 0x480801 to DCFG */
    LOGT("Step 22: Re-configure DCFG=0x480801");
    writel_reg(MIZAR_USB_DCFG, 0x480801U);

    /* Step 23: Wait for additional events if event_counter <= 0x4 */
    LOGT("Step 23: Wait for additional events if event_counter <= 4");
    while (event_counter <= 0x4U) {
        usb_fs_device_isochronous_transfer_test_wait_interrupt();
    }

    /* Step 24: Write progress marker 0xdeadbee0 */
    LOGT("Step 24: Progress marker 0xdeadbee0");
    writel_reg(0xA0243ffcU, 0xdeadbee0U);

    /* Step 25: Read DCFG and DSTS */
    LOGT("Step 25: Read DCFG and DSTS");
    rd_data = readl_reg(MIZAR_USB_DCFG);
    LOGT("DCFG=0x%llx", (unsigned long long)rd_data);
    dsts_val = readl_reg(MIZAR_USB_DSTS);
    LOGT("DSTS=0x%llx", (unsigned long long)dsts_val);

    /* Step 26: Write 0x480801 to DCFG, 0x80f00a00 to DCTL */
    LOGT("Step 26: DCFG=0x480801, DCTL=0x80f00a00");
    writel_reg(MIZAR_USB_DCFG, 0x480801U);
    writel_reg(MIZAR_USB_DCTL, 0x80f00a00U);

    /* Step 27: Write 0xff to DALEPENA (enable all 8 endpoints) */
    LOGT("Step 27: Enable all endpoints (DALEPENA=0xff)");
    writel_reg(MIZAR_USB_DALEPENA, 0xffU);

    /* Step 28: Wait 5000 cycles */
    LOGT("Step 28: Wait 5000 cycles");
    // MANUAL_REVIEW: DV wait_on(5000) delay mechanism
    // Original intent: Wait for 5000 cycles to allow controller stabilization.
    // Reason: No PSV/FV-native equivalent for cycle-accurate delay in the FV Template.
    // PSV action: Use platform-specific delay API or busy-wait if available.

    /* Step 29: Setup stage for first control transfer */
    LOGT("Step 29: Setup stage");
    usb_fs_device_isochronous_transfer_test_setup_stage();

    /* Step 30: Write 0x40002547 to GUSB2PHYCFG (update PHY config) */
    LOGT("Step 30: Update PHY config (GUSB2PHYCFG=0x40002547)");
    writel_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547U);

    /* Step 31: Wait for events */
    LOGT("Step 31: Wait for events");
    usb_fs_device_isochronous_transfer_test_wait_interrupt();
    usb_fs_device_isochronous_transfer_test_wait_interrupt();

    /* Step 32: SET_ADDRESS - write 0x480809 to DCFG */
    LOGT("Step 32: SET_ADDRESS (DCFG=0x480809)");
    writel_reg(MIZAR_USB_DCFG, 0x480809U);

    /* Step 33: Data stage TRB for SET_ADDRESS status on EP1 IN */
    LOGT("Step 33: SET_ADDRESS data stage TRB");
    writel_reg((uintptr_t)event_trb_addr, (uint64_t)Buffer_PointerLO);
    writel_reg((uintptr_t)event_trb_addr + 0x04U, 0x0U);
    writel_reg((uintptr_t)event_trb_addr + 0x08U, 0x0U);
    writel_reg((uintptr_t)event_trb_addr + 0x0CU, 0x853U);

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, 0x0U);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, (uint64_t)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);

    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10U);
        if (timeout == 0U) {
            LOGE("Step 33: DEPCMD poll timeout");
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (rd_data == 0x506U);
    LOGT("Step 33: SET_ADDRESS data stage complete");

    /* Step 34: Wait for events, poll 0xa0243ff4 until non-zero (handshake) */
    LOGT("Step 34: Wait for events and handshake polling");
    usb_fs_device_isochronous_transfer_test_wait_interrupt();

    // MANUAL_REVIEW: VIP/Testbench Synchronization Removed
    // Original intent: Poll memory-mapped address 0xa0243ff4 until non-zero
    // to synchronize with external VIP/testbench handshake.
    // Reason: VIP/testbench infrastructure dependency, not DUT hardware behavior.
    // PSV action: If this address maps to a real DUT status register, implement
    // DUT-level status polling. Otherwise, this synchronization may not be needed in PSV.
    timeout = 10000U;
    do {
        rd_data = readl_reg(0xa0243ff4U);
        if (timeout == 0U) {
            LOGE("Handshake 1 poll timeout at 0xa0243ff4");
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (rd_data == 0x0U);
    LOGT("Handshake 1 complete: rd_data=0x%llx", (unsigned long long)rd_data);

    /* Step 35: Enumeration */
    LOGT("Step 35: USB Enumeration");
    usb_fs_device_isochronous_transfer_test_enumeration();

    /* Step 36: Poll 0xa0243ff8 until non-zero (handshake 2) */
    LOGT("Step 36: Handshake 2 polling");
    // MANUAL_REVIEW: VIP/Testbench Synchronization Removed
    // Original intent: Poll memory-mapped address 0xa0243ff8 until non-zero
    // to synchronize with external VIP/testbench handshake.
    // Reason: VIP/testbench infrastructure dependency, not DUT hardware behavior.
    // PSV action: If this address maps to a real DUT status register, implement
    // DUT-level status polling. Otherwise, this synchronization may not be needed in PSV.
    timeout = 10000U;
    do {
        rd_data = readl_reg(0xa0243ff8U);
        if (timeout == 0U) {
            LOGE("Handshake 2 poll timeout at 0xa0243ff8");
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (rd_data == 0x0U);
    LOGT("Handshake 2 complete: rd_data=0x%llx", (unsigned long long)rd_data);

    /* Step 37: Poll DSTS until bits[11:3] (frame number) != 0 */
    LOGT("Step 37: Poll DSTS for valid frame number");
    timeout = 10000U;
    do {
        dsts_val = readl_reg(MIZAR_USB_DSTS);
        if (timeout == 0U) {
            LOGE("DSTS frame number poll timeout: DSTS=0x%llx", (unsigned long long)dsts_val);
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while ((dsts_val & 0xFF8U) == 0x0U);
    LOGT("DSTS frame number valid: DSTS=0x%llx, bits[11:3]=0x%llx",
         (unsigned long long)dsts_val, (unsigned long long)((dsts_val >> 3) & 0x1FFU));

    /* Step 38: Write progress marker 0xdeadbee6 */
    LOGT("Step 38: Progress marker 0xdeadbee6");
    writel_reg(0xA0243ffcU, 0xdeadbee6U);

    /* Step 39: ISOCHRONOUS OUT transfer 1 */
    LOGT("Step 39: Isochronous OUT transfer 1 on EP offset 0x60");
    writel_reg((uintptr_t)event_trb_addr, (uint64_t)Buffer_PointerLO_1);
    writel_reg((uintptr_t)event_trb_addr + 0x04U, 0x0U);
    writel_reg((uintptr_t)event_trb_addr + 0x08U, 0x3ffU);
    writel_reg((uintptr_t)event_trb_addr + 0x0CU, 0x869U);

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x60U, 0x0U);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x60U, (uint64_t)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x60U, 0x20506U);

    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x60U);
        if (timeout == 0U) {
            LOGE("ISO transfer 1 DEPCMD poll timeout");
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (rd_data == 0x20506U);
    LOGT("ISO transfer 1 command complete");

    /* Wait for interrupt */
    usb_fs_device_isochronous_transfer_test_wait_interrupt();

    /* Step 40: Poll DSTS until bits[5:3] == 0x2 (frame number 2) */
    LOGT("Step 40: Poll DSTS for frame number 2");
    timeout = 10000U;
    do {
        dsts_val = readl_reg(MIZAR_USB_DSTS);
        if (timeout == 0U) {
            LOGE("DSTS frame 2 poll timeout: DSTS=0x%llx", (unsigned long long)dsts_val);
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (((dsts_val >> 3) & 0x7U) != 0x2U);
    LOGT("DSTS frame number 2 reached: DSTS=0x%llx", (unsigned long long)dsts_val);

    /* Step 41: Write progress marker 0xdeadbee7, wait for interrupt */
    LOGT("Step 41: Progress marker 0xdeadbee7");
    writel_reg(0xA0243ffcU, 0xdeadbee7U);
    usb_fs_device_isochronous_transfer_test_wait_interrupt();

    /* Step 42: Poll DSTS until bits[5:3] == 0x3 (frame number 3) */
    LOGT("Step 42: Poll DSTS for frame number 3");
    timeout = 10000U;
    do {
        dsts_val = readl_reg(MIZAR_USB_DSTS);
        if (timeout == 0U) {
            LOGE("DSTS frame 3 poll timeout: DSTS=0x%llx", (unsigned long long)dsts_val);
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (((dsts_val >> 3) & 0x7U) != 0x3U);
    LOGT("DSTS frame number 3 reached: DSTS=0x%llx", (unsigned long long)dsts_val);

    /* Step 43: Write progress marker 0xdeadbee8, wait for interrupt */
    LOGT("Step 43: Progress marker 0xdeadbee8");
    writel_reg(0xA0243ffcU, 0xdeadbee8U);
    usb_fs_device_isochronous_transfer_test_wait_interrupt();

    /* Step 44: ISOCHRONOUS OUT transfer 2 on EP offset 0x70 */
    LOGT("Step 44: Isochronous OUT transfer 2 on EP offset 0x70");
    writel_reg((uintptr_t)event_trb_addr, (uint64_t)Buffer_PointerLO_1);
    writel_reg((uintptr_t)event_trb_addr + 0x04U, 0x0U);
    writel_reg((uintptr_t)event_trb_addr + 0x08U, 0x3ffU);
    writel_reg((uintptr_t)event_trb_addr + 0x0CU, 0x869U);

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x70U, 0x0U);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x70U, (uint64_t)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMD + 0x70U, 0x40506U);

    timeout = 10000U;
    do {
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x70U);
        if (timeout == 0U) {
            LOGE("ISO transfer 2 DEPCMD poll timeout");
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (rd_data == 0x40506U);
    LOGT("ISO transfer 2 command complete");

    /* Wait for interrupt */
    usb_fs_device_isochronous_transfer_test_wait_interrupt();

    /* Step 45: Poll DSTS until bits[5:3] == 0x4 (frame number 4) */
    LOGT("Step 45: Poll DSTS for frame number 4");
    timeout = 10000U;
    do {
        dsts_val = readl_reg(MIZAR_USB_DSTS);
        if (timeout == 0U) {
            LOGE("DSTS frame 4 poll timeout: DSTS=0x%llx", (unsigned long long)dsts_val);
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (((dsts_val >> 3) & 0x7U) != 0x4U);
    LOGT("DSTS frame number 4 reached: DSTS=0x%llx", (unsigned long long)dsts_val);

    /* Step 46: Write progress marker 0xdeadbee9, wait for interrupt */
    LOGT("Step 46: Progress marker 0xdeadbee9");
    writel_reg(0xA0243ffcU, 0xdeadbee9U);
    usb_fs_device_isochronous_transfer_test_wait_interrupt();

    /* Step 47: Test completion */
    // MANUAL_REVIEW: DV finish(0) converted to PSV/FV status reporting
    // Original intent: Call finish(0) to signal successful test completion.
    // Reason: finish() is a DV-specific completion mechanism.
    // PSV action: Test result is reported via out->status.
    LOGT("Step 47: Test completion");

    /* Set final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: usb_fs_device_isochronous_transfer_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for usb_fs_device_isochronous_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_isochronous_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Isochronous Transfer test teardown: errors=%u", g_ctx.errors);

    /* Validation summary */
    /* 1. Soft reset validated by polling DCTL == 0xf00000 */
    /* 2. All DEPCMD commands validated by polling until CmdAct clears */
    /* 3. Interrupt handling validated by int_pend flag transitions */
    /* 4. Handshake polling validated at 0xa0243ff4 and 0xa0243ff8 */
    /* 5. Frame number validated: DSTS bits[11:3] non-zero, bits[5:3] == 2,3,4 */
    /* 6. IRQ handler validated: GEVNTCOUNT acknowledged, sysreg cleared, GIC cleared */
    /* 7. Progress markers 0xdeadbee0-0xdeadbee9 written at each stage */
    /* 8. Test passes if all above completed with zero errors */

    return g_ctx.errors == 0U ? 0 : -1;
}
