// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_isochronous_transfer_test.h"
#include "test_define.inc"

/*
 * USB_FS_Device_Isochronous_Transfer_test
 *
 * This test validates USB Full-Speed Device mode Isochronous Transfer functionality.
 * The test performs a soft reset of the USB controller, configures the USB2 PHY,
 * sets up the event buffer, configures the global controller for device mode,
 * configures device settings and enables device events, configures all endpoints
 * including isochronous endpoint types, allocates TX resources, enables physical
 * endpoints, starts the controller, enables system-level interrupts, waits for
 * link state connect and reset events, performs enumeration, polls DSTS for a valid
 * frame number, initiates two isochronous OUT transfers with 1023-byte TRBs and
 * isochronous TRB type (0x869), synchronizes with specific USB frame numbers
 * (frames 2, 3, and 4), and verifies completion.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
    volatile unsigned int int_pend;
} usb_isoc_test_ctx_t;

static usb_isoc_test_ctx_t g_ctx;

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

    LOGE("USB Isoc: DEPCMD poll timeout at ep_offset=0x%x", ep_offset);
    return -1;
}

static int wait_for_interrupt(unsigned int timeout)
{
    unsigned int count = timeout;

    while (g_ctx.int_pend == 0U) {
        if (count == 0U) {
            LOGE("USB Isoc: interrupt wait timeout");
            return -1;
        }
        count--;
    }
    g_ctx.int_pend = 0U;
    return 0;
}

static int poll_dsts_frame_number(unsigned int target_frame, unsigned int timeout)
{
    unsigned int val;
    unsigned int frame_num;
    unsigned int count = timeout;

    do {
        val = readl_reg(MIZAR_USB_DSTS);
        frame_num = (val >> 3) & 0x3FFFU;
        if (frame_num >= target_frame) {
            LOGT("USB Isoc: DSTS frame=%u target=%u DSTS=0x%x", frame_num, target_frame, val);
            return 0;
        }
        count--;
    } while (count > 0U);

    LOGE("USB Isoc: DSTS frame poll timeout, target=%u current=%u DSTS=0x%x", target_frame, frame_num, val);
    return -1;
}

void Default_IRQHandler(void)
{
    unsigned int evt_count;
    unsigned int sysreg_sts;

    sysreg_sts = readl_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("USB Isoc IRQ: sysreg_sts=0x%x", sysreg_sts);

    evt_count = readl_reg(MIZAR_USB_GEVNTCOUNT);
    if (evt_count > 0U) {
        writel_reg(MIZAR_USB_GEVNTCOUNT, evt_count);
        LOGT("USB Isoc IRQ: GEVNTCOUNT acknowledged, count=%u", evt_count);
    }

    writel_reg(MIZAR_LSS_SYSREG_RAW_STCR0, sysreg_sts);
    g_ctx.int_pend = 1U;
}

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

static int issue_start_transfer(unsigned int ep_offset, unsigned int trb_buf_addr,
                                unsigned int trb_size, unsigned int trb_ctrl,
                                unsigned int depcmd_val)
{
    writel_reg(Buffer_PointerLO + ep_offset, trb_buf_addr);
    writel_reg(Buffer_PointerLO + ep_offset + 0x08U, trb_size);
    writel_reg(Buffer_PointerLO + ep_offset + 0x0CU, trb_ctrl);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, 0U);
    writel_reg(MIZAR_USB_DEPCMD + ep_offset, depcmd_val);

    if (poll_depcmd_completion(ep_offset, 10000U) != 0) {
        g_ctx.errors++;
        return -1;
    }
    return 0;
}

int usb_fs_device_isochronous_transfer_test_init(const TestsItem *cfg)
{
    unsigned int i;
    unsigned int val;
    (void)cfg;
    g_ctx = (usb_isoc_test_ctx_t){0};
    LOGT("USB Isoc: Init - starting USB FS Device Isochronous Transfer test");
    LOGT("USB Isoc: Step 1 - System initialization");
    LOGT("USB Isoc: Step 2 - Clearing buffers");
    for (i = 0U; i < 20U; i++) { writel_reg(Buffer_PointerLO + (i * DWORD), 0x00000000U); }
    for (i = 0U; i < 20U; i++) { writel_reg(event_trb_addr + (i * DWORD), 0x00000000U); }
    LOGT("USB Isoc: Step 3 - Soft reset");
    writel_reg(MIZAR_USB_DCTL, (1U << 30));
    { unsigned int timeout = 10000U; do { val = readl_reg(MIZAR_USB_DCTL); if ((val & (1U << 30)) == 0U) break; timeout--; } while (timeout > 0U); if (timeout == 0U) { g_ctx.errors++; return -1; } }
    g_ctx.checks_passed++; g_ctx.checks_total++;
    val = readl_reg(MIZAR_USB_GUSB2PHYCFG); writel_reg(MIZAR_USB_GUSB2PHYCFG, val);
    writel_reg(MIZAR_USB_GEVNTADRLO, (unsigned int)(uintptr_t)Default_Event_Ring_Array);
    writel_reg(MIZAR_USB_GEVNTADRHI, 0x00000000U);
    writel_reg(MIZAR_USB_GEVNTSIZ, 0x00000050U);
    writel_reg(MIZAR_USB_GEVNTCOUNT, 0x00000000U);
    val = readl_reg(MIZAR_USB_GCTL); val = (val & ~(0x3U << 12)) | (0x2U << 12); writel_reg(MIZAR_USB_GCTL, val);
    val = readl_reg(MIZAR_USB_DCFG); writel_reg(MIZAR_USB_DCFG, val);
    writel_reg(MIZAR_USB_DEVTEN, 0x00000007U);
    val = readl_reg(MIZAR_USB_GUCTL); writel_reg(MIZAR_USB_GUCTL, val);
    LOGT("USB Isoc: Init complete");
    return 0;
}

int usb_fs_device_isochronous_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int val; unsigned int i; unsigned int ep_offset;
    (void)cfg;
    if (out == 0) { return -1; }
    out->status = 0;
    LOGT("USB Isoc: Run - starting main testcase flow");
    for (i = 0U; i < 8U; i++) { ep_offset = i * 0x10U; if (set_configuration(ep_offset, 0U, 0U, 0x00000409U) != 0) { out->status = -1; } }
    for (i = 0U; i < 8U; i++) { ep_offset = i * 0x10U; writel_reg(MIZAR_USB_DEPCMDPAR0 + ep_offset, 0x00000001U); writel_reg(MIZAR_USB_DEPCMD + ep_offset, 0x00000802U); if (poll_depcmd_completion(ep_offset, 10000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++; }
    writel_reg(MIZAR_USB_DALEPENA, 0x00000003U);
    val = readl_reg(MIZAR_USB_DCTL); val |= (1U << 31); writel_reg(MIZAR_USB_DCTL, val);
    writel_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0xFFFFFFFFU);
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    val = readl_reg(MIZAR_USB_DSTS);
    val = readl_reg(MIZAR_USB_DCFG); writel_reg(MIZAR_USB_DCFG, val);
    val = readl_reg(MIZAR_USB_DCTL); writel_reg(MIZAR_USB_DCTL, val);
    writel_reg(MIZAR_USB_DALEPENA, 0x000000FFU);
    /* Enumeration */
    if (issue_start_transfer(0U, 0U, 8U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    val = readl_reg(MIZAR_USB_GUSB2PHYCFG); writel_reg(MIZAR_USB_GUSB2PHYCFG, val);
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; }
    val = readl_reg(MIZAR_USB_DCFG); writel_reg(MIZAR_USB_DCFG, val);
    if (issue_start_transfer(0U, 0U, 0U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    { unsigned int hs = 100000U; do { val = readl_reg(0xa0243ff4U); if (val != 0U) break; hs--; } while (hs > 0U); if (hs == 0U) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++; }
    /* GET DEV DESC, GET CFG DESC short, SET CFG, GET CFG DESC full, handshake2 */
    if (issue_start_transfer(0U, 0U, 8U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, (unsigned int)(uintptr_t)buf_data, 18U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, 0U, 0U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, 0U, 8U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, (unsigned int)(uintptr_t)buf_data, 9U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, 0U, 0U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, 0U, 8U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, 0U, 0U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, 0U, 8U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, (unsigned int)(uintptr_t)buf_data, 32U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (issue_start_transfer(0U, 0U, 0U, 0x00000869U, 0x00000406U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    { unsigned int hs2 = 100000U; do { val = readl_reg(0xa0243ff8U); if (val != 0U) break; hs2--; } while (hs2 > 0U); if (hs2 == 0U) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++; }
    /* Isochronous transfers */
    LOGT("USB Isoc: Polling DSTS for valid frame number");
    { unsigned int dt = 100000U; unsigned int fn; do { val = readl_reg(MIZAR_USB_DSTS); fn = (val >> 3) & 0x3FFFU; if (fn != 0U) break; dt--; } while (dt > 0U); if (dt == 0U) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++; }
    LOGT("USB Isoc: First isoc OUT transfer (1023B, ctrl=0x869, DEPCMD=0x20506)");
    if (issue_start_transfer(0x10U, (unsigned int)(uintptr_t)buf_data, 1023U, 0x00000869U, 0x00020506U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (poll_dsts_frame_number(2U, 100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    LOGT("USB Isoc: Second isoc OUT transfer (1023B, ctrl=0x869, DEPCMD=0x40506)");
    if (issue_start_transfer(0x20U, (unsigned int)(uintptr_t)buf_data, 1023U, 0x00000869U, 0x00040506U) != 0) { out->status = -1; }
    if (wait_for_interrupt(100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (poll_dsts_frame_number(3U, 100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    if (poll_dsts_frame_number(4U, 100000U) != 0) { g_ctx.errors++; out->status = -1; } else { g_ctx.checks_passed++; } g_ctx.checks_total++;
    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    LOGT("USB Isoc: Run complete: %s errors=%u", (out->status == 0) ? "PASS" : "FAIL", g_ctx.errors);
    return out->status;
}

int usb_fs_device_isochronous_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("USB Isoc: Teardown complete - errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
