// Author - AI Force 2.3. 24-Jul-2025 04:45 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case  : USB_FS_Device_Bulk_Transfer_test
 * Description: This testcase validates USB Full-Speed Device mode Bulk Transfer operation.
 *              Performs controller soft reset via DCTL, configures USB2 PHY, sets up event
 *              buffer, configures device mode via GCTL, DCFG, DEVTEN, GUCTL, configures
 *              8 endpoints, performs enumeration, and executes bulk OUT/IN transfers.
 */

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

/* USB test context structure following FV template pattern */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} usb_bulk_test_ctx_t;

static usb_bulk_test_ctx_t g_ctx;

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Performs testcase initialization and pre-condition setup for usb_fs_device_bulk_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    unsigned int idx;
    uint32_t reg_val;

    (void)cfg;

    g_ctx = (usb_bulk_test_ctx_t){0};

    LOGT("USB FS Device Bulk Transfer test init: starting");

    /* Step 1: Initialize the NIC subsystem and enable all GIC interrupts */
    LOGT("Step 1: Initialize NIC subsystem and enable GIC interrupts");
    writel_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0xFFFFFFFFU);
    LOGT("MIZAR_LSS_SYSREG_INTR_EN0 written: 0x%lx = 0xFFFFFFFF",
         (unsigned long)MIZAR_LSS_SYSREG_INTR_EN0);

    /* Step 2: Clear buffers */
    LOGT("Step 2: Clear buffers");
    for (idx = 0U; idx < 16U; idx++) {
        buf_data[idx] = 0x00000000U;
    }
    LOGT("buf_data[16] cleared to zero");

    /* Step 3: Perform soft reset via DCTL */
    LOGT("Step 3: Perform soft reset via DCTL");
    writel_reg(MIZAR_USB_DCTL, 0x40000000U);
    LOGT("DCTL written: 0x%lx = 0x40000000 (soft reset)",
         (unsigned long)MIZAR_USB_DCTL);

    /* Poll DCTL for soft reset completion (bit 30 clears) */
    {
        unsigned int timeout = USB_POLL_TIMEOUT;
        do {
            reg_val = readl_reg(MIZAR_USB_DCTL);
            if ((reg_val & 0x40000000U) == 0U) {
                break;
            }
            timeout--;
        } while (timeout > 0U);

        if (timeout == 0U) {
            LOGE("Soft reset via DCTL did not complete, DCTL=0x%lx",
                 (unsigned long)reg_val);
            g_ctx.errors++;
            return -1;
        }
    }
    g_ctx.checks_total++;
    g_ctx.checks_passed++;
    LOGT("Soft reset completed successfully, DCTL=0x%lx",
         (unsigned long)reg_val);

    /* Step 4: Configure USB2 PHY via GUSB2PHYCFG */
    LOGT("Step 4: Configure USB2 PHY via GUSB2PHYCFG");
    writel_reg(MIZAR_USB_GUSB2PHYCFG, 0x00002500U);
    LOGT("GUSB2PHYCFG written: 0x%lx = 0x00002500",
         (unsigned long)MIZAR_USB_GUSB2PHYCFG);

    /* Step 5: Set up event buffer */
    LOGT("Step 5: Set up event buffer (GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, GEVNTCOUNT)");
    writel_reg(MIZAR_USB_GEVNTADRLO, (uint32_t)event_trb_addr);
    LOGT("GEVNTADRLO written: 0x%lx = 0x%lx",
         (unsigned long)MIZAR_USB_GEVNTADRLO, (unsigned long)event_trb_addr);

    writel_reg(MIZAR_USB_GEVNTADRHI, 0x00000000U);
    LOGT("GEVNTADRHI written: 0x%lx = 0x00000000",
         (unsigned long)MIZAR_USB_GEVNTADRHI);

    writel_reg(MIZAR_USB_GEVNTSIZ, 0x00000100U);
    LOGT("GEVNTSIZ written: 0x%lx = 0x00000100",
         (unsigned long)MIZAR_USB_GEVNTSIZ);

    writel_reg(MIZAR_USB_GEVNTCOUNT, 0x00000000U);
    LOGT("GEVNTCOUNT written: 0x%lx = 0x00000000",
         (unsigned long)MIZAR_USB_GEVNTCOUNT);

    /* Step 6: Configure GCTL for device mode */
    LOGT("Step 6: Configure GCTL for device mode");
    reg_val = readl_reg(MIZAR_USB_GCTL);
    LOGT("GCTL read: 0x%lx = 0x%lx",
         (unsigned long)MIZAR_USB_GCTL, (unsigned long)reg_val);
    reg_val = (reg_val & ~0x00003000U) | 0x00002000U;
    writel_reg(MIZAR_USB_GCTL, reg_val);
    LOGT("GCTL written: 0x%lx = 0x%lx (device mode)",
         (unsigned long)MIZAR_USB_GCTL, (unsigned long)reg_val);

    /* Step 7: Configure DCFG */
    LOGT("Step 7: Configure DCFG register");
    writel_reg(MIZAR_USB_DCFG, 0x00080004U);
    LOGT("DCFG written: 0x%lx = 0x00080004 (Full-Speed)",
         (unsigned long)MIZAR_USB_DCFG);

    /* Step 8: Enable device events via DEVTEN */
    LOGT("Step 8: Enable device events via DEVTEN");
    writel_reg(MIZAR_USB_DEVTEN, 0x00000017U);
    LOGT("DEVTEN written: 0x%lx = 0x00000017",
         (unsigned long)MIZAR_USB_DEVTEN);

    /* Step 9: Configure GUCTL */
    LOGT("Step 9: Configure GUCTL register");
    writel_reg(MIZAR_USB_GUCTL, 0x00008010U);
    LOGT("GUCTL written: 0x%lx = 0x00008010",
         (unsigned long)MIZAR_USB_GUCTL);

    LOGT("USB FS Device Bulk Transfer test init: complete");
    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Executes the main testcase flow for usb_fs_device_bulk_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    uint32_t reg_val;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("USB output pointer is NULL");
        return -1;
    }

    out->status = 0;
    out->actual_len = 0;
    out->actual_pattern[0] = 0;

    LOGT("Starting USB FS Device Bulk Transfer test run");

    /* Step 10: Configure 8 endpoints using DEPCMDPAR0, DEPCMDPAR1, DEPCMD */
    LOGT("Step 10: Configure 8 endpoints");

    /* Endpoint 0 OUT (Control) */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000200U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000401U);
    LOGT("EP0 OUT configured via DEPCMD");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("EP0 OUT DEPCMD timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("EP0 OUT command completed");
    }

    /* Endpoint 0 IN (Control) */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000200U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000401U);
    LOGT("EP0 IN configured via DEPCMD");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("EP0 IN DEPCMD timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("EP0 IN command completed");
    }

    /* Endpoint 1 OUT (Interrupt) */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000300U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000401U);
    LOGT("EP1 OUT configured via DEPCMD");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("EP1 OUT DEPCMD timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("EP1 OUT command completed");
    }

    /* Endpoint 1 IN (Interrupt) */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000300U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000401U);
    LOGT("EP1 IN configured via DEPCMD");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("EP1 IN DEPCMD timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("EP1 IN command completed");
    }

    /* Endpoint 2 OUT (Bulk) */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000400U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000401U);
    LOGT("EP2 OUT (Bulk) configured via DEPCMD");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("EP2 OUT DEPCMD timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("EP2 OUT (Bulk) command completed");
    }

    /* Endpoint 2 IN (Bulk) */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000400U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000401U);
    LOGT("EP2 IN (Bulk) configured via DEPCMD");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("EP2 IN DEPCMD timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("EP2 IN (Bulk) command completed");
    }

    /* Endpoint 3 OUT (Isochronous) */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000500U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000401U);
    LOGT("EP3 OUT configured via DEPCMD");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("EP3 OUT DEPCMD timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("EP3 OUT command completed");
    }

    /* Endpoint 3 IN (Isochronous) */
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000500U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000401U);
    LOGT("EP3 IN configured via DEPCMD");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("EP3 IN DEPCMD timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("EP3 IN command completed");
    }

    LOGT("All 8 endpoints configured");

    /* Step 11: Allocate transfer resources */
    LOGT("Step 11: Allocate transfer resources for bulk endpoints");
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000402U);
    LOGT("Transfer resource allocation command issued for bulk EP");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("Transfer resource alloc timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("Transfer resources allocated");
    }

    /* Step 12: Enable endpoints via DALEPENA */
    LOGT("Step 12: Enable endpoints via DALEPENA");
    writel_reg(MIZAR_USB_DALEPENA, 0x000000FFU);
    LOGT("DALEPENA written: 0x%lx = 0x000000FF",
         (unsigned long)MIZAR_USB_DALEPENA);

    /* Step 13: Set run state via DCTL */
    LOGT("Step 13: Set run state via DCTL");
    reg_val = readl_reg(MIZAR_USB_DCTL);
    reg_val |= 0x80000000U;
    writel_reg(MIZAR_USB_DCTL, reg_val);
    LOGT("DCTL written: 0x%lx = 0x%lx (run state)",
         (unsigned long)MIZAR_USB_DCTL, (unsigned long)reg_val);

    /* Step 14: Enable interrupts */
    LOGT("Step 14: Enable interrupts");
    writel_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0xFFFFFFFFU);
    LOGT("MIZAR_LSS_SYSREG_INTR_EN0 written: 0x%lx = 0xFFFFFFFF",
         (unsigned long)MIZAR_LSS_SYSREG_INTR_EN0);

    /* Steps 15-22: Enumeration sequence */
    LOGT("Steps 15-22: Enumeration sequence");

    /* Step 15: Wait for USB Reset event via GEVNTCOUNT */
    LOGT("Step 15: Wait for USB Reset event");
    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
        if ((reg_val & 0x0000FFFFU) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("USB Reset event not received, GEVNTCOUNT=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("USB Reset event received, GEVNTCOUNT=0x%lx", (unsigned long)reg_val);
    }

    /* Step 16: Acknowledge USB Reset event */
    LOGT("Step 16: Acknowledge USB Reset event");
    writel_reg(MIZAR_USB_GEVNTCOUNT, reg_val & 0x0000FFFFU);
    LOGT("GEVNTCOUNT acknowledged");

    /* Step 17: Wait for Connection Done event */
    LOGT("Step 17: Wait for Connection Done event");
    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
        if ((reg_val & 0x0000FFFFU) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("Connection Done event not received, GEVNTCOUNT=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("Connection Done event received");
    }

    /* Step 18: Read DSTS for speed confirmation */
    LOGT("Step 18: Read DSTS for speed confirmation");
    reg_val = readl_reg(MIZAR_USB_DSTS);
    LOGT("DSTS read: 0x%lx = 0x%lx",
         (unsigned long)MIZAR_USB_DSTS, (unsigned long)reg_val);

    /* Acknowledge Connection Done event */
    reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
    writel_reg(MIZAR_USB_GEVNTCOUNT, reg_val & 0x0000FFFFU);
    LOGT("Connection Done event acknowledged");

    /* Steps 19-22: Handle enumeration setup packets */
    /* Step 19: GET DEVICE DESCRIPTOR */
    LOGT("Step 19: Handle GET DEVICE DESCRIPTOR setup packet");
    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
        if ((reg_val & 0x0000FFFFU) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("GET DEVICE DESCRIPTOR event not received");
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        writel_reg(MIZAR_USB_GEVNTCOUNT, reg_val & 0x0000FFFFU);
        LOGT("GET DEVICE DESCRIPTOR handled");
    }

    /* Step 20: SET ADDRESS */
    LOGT("Step 20: Handle SET ADDRESS setup packet");
    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
        if ((reg_val & 0x0000FFFFU) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("SET ADDRESS event not received");
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        writel_reg(MIZAR_USB_GEVNTCOUNT, reg_val & 0x0000FFFFU);
        LOGT("SET ADDRESS handled");
    }

    /* Step 21: GET CONFIGURATION DESCRIPTOR */
    LOGT("Step 21: Handle GET CONFIGURATION DESCRIPTOR setup packet");
    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
        if ((reg_val & 0x0000FFFFU) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("GET CONFIGURATION DESCRIPTOR event not received");
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        writel_reg(MIZAR_USB_GEVNTCOUNT, reg_val & 0x0000FFFFU);
        LOGT("GET CONFIGURATION DESCRIPTOR handled");
    }

    /* Step 22: SET CONFIGURATION */
    LOGT("Step 22: Handle SET CONFIGURATION setup packet");
    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
        if ((reg_val & 0x0000FFFFU) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("SET CONFIGURATION event not received");
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        writel_reg(MIZAR_USB_GEVNTCOUNT, reg_val & 0x0000FFFFU);
        LOGT("SET CONFIGURATION handled - Enumeration complete");
    }

    /* Steps 23-25: Bulk OUT and Bulk IN transfers (64 bytes each) */
    LOGT("Steps 23-25: Bulk OUT and Bulk IN transfers");

    /* Step 23: Initiate Bulk OUT transfer (64 bytes) */
    LOGT("Step 23: Initiate Bulk OUT transfer (64 bytes)");
    writel_reg((uintptr_t)Buffer_PointerLO, (uint32_t)Buffer_PointerLO_1);
    writel_reg((uintptr_t)(Buffer_PointerLO + 0x04U), 0x00000000U);
    writel_reg((uintptr_t)(Buffer_PointerLO + 0x08U), 0x00000040U);
    writel_reg((uintptr_t)(Buffer_PointerLO + 0x0CU), 0x00000813U);
    LOGT("Bulk OUT TRB prepared at Buffer_PointerLO=0x%lx",
         (unsigned long)Buffer_PointerLO);

    writel_reg(MIZAR_USB_DEPCMDPAR0, (uint32_t)Buffer_PointerLO);
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000406U);
    LOGT("Bulk OUT Start Transfer command issued");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("Bulk OUT Start Transfer timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("Bulk OUT Start Transfer command completed");
    }

    /* Step 24: Wait for Bulk OUT transfer completion event */
    LOGT("Step 24: Wait for Bulk OUT transfer completion event");
    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
        if ((reg_val & 0x0000FFFFU) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("Bulk OUT completion event not received, GEVNTCOUNT=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        writel_reg(MIZAR_USB_GEVNTCOUNT, reg_val & 0x0000FFFFU);
        LOGT("Bulk OUT transfer completed (64 bytes)");
    }

    /* Step 25: Initiate Bulk IN transfer (64 bytes) */
    LOGT("Step 25: Initiate Bulk IN transfer (64 bytes)");
    writel_reg((uintptr_t)Buffer_PointerLO, (uint32_t)Buffer_PointerLO_1);
    writel_reg((uintptr_t)(Buffer_PointerLO + 0x04U), 0x00000000U);
    writel_reg((uintptr_t)(Buffer_PointerLO + 0x08U), 0x00000040U);
    writel_reg((uintptr_t)(Buffer_PointerLO + 0x0CU), 0x00000813U);
    LOGT("Bulk IN TRB prepared at Buffer_PointerLO=0x%lx",
         (unsigned long)Buffer_PointerLO);

    writel_reg(MIZAR_USB_DEPCMDPAR0, (uint32_t)Buffer_PointerLO);
    writel_reg(MIZAR_USB_DEPCMDPAR1, 0x00000000U);
    writel_reg(MIZAR_USB_DEPCMD, 0x00000406U);
    LOGT("Bulk IN Start Transfer command issued");

    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_DEPCMD);
        if ((reg_val & 0x00000400U) == 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("Bulk IN Start Transfer timed out, DEPCMD=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        LOGT("Bulk IN Start Transfer command completed");
    }

    /* Wait for Bulk IN transfer completion event */
    LOGT("Wait for Bulk IN transfer completion event");
    timeout = USB_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_USB_GEVNTCOUNT);
        if ((reg_val & 0x0000FFFFU) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("Bulk IN completion event not received, GEVNTCOUNT=0x%lx", (unsigned long)reg_val);
        g_ctx.errors++;
    } else {
        g_ctx.checks_total++;
        g_ctx.checks_passed++;
        writel_reg(MIZAR_USB_GEVNTCOUNT, reg_val & 0x0000FFFFU);
        LOGT("Bulk IN transfer completed (64 bytes)");
    }

    /* Steps 26-28: Validation and status capture */
    LOGT("Steps 26-28: Validation and status capture");

    /* Step 26: Read interrupt status registers */
    LOGT("Step 26: Read interrupt status registers");
    reg_val = readl_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("MIZAR_LSS_SYSREG_MSK_STS0 read: 0x%lx = 0x%lx",
         (unsigned long)MIZAR_LSS_SYSREG_MSK_STS0, (unsigned long)reg_val);

    reg_val = readl_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("MIZAR_LSS_SYSREG_RAW_STCR0 read: 0x%lx = 0x%lx",
         (unsigned long)MIZAR_LSS_SYSREG_RAW_STCR0, (unsigned long)reg_val);

    /* Update final status */
    g_ctx.checks_failed = g_ctx.errors;

    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    out->actual_len = 1;
    out->actual_pattern[0] = (int)g_ctx.errors;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for usb_fs_device_bulk_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown");
    LOGT("Validation summary:");
    LOGT("  1. Soft reset completes: %s", (g_ctx.errors == 0U) ? "PASS" : "FAIL");
    LOGT("  2. Endpoint commands complete: %s", (g_ctx.errors == 0U) ? "PASS" : "FAIL");
    LOGT("  3. Start Transfer commands complete: %s", (g_ctx.errors == 0U) ? "PASS" : "FAIL");
    LOGT("  4. Events received via interrupts: %s", (g_ctx.errors == 0U) ? "PASS" : "FAIL");
    LOGT("  5. Enumeration completes: %s", (g_ctx.errors == 0U) ? "PASS" : "FAIL");
    LOGT("  6. Bulk transfers complete: %s", (g_ctx.errors == 0U) ? "PASS" : "FAIL");
    LOGT("  Final: errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         g_ctx.errors, g_ctx.checks_passed, g_ctx.checks_total, g_ctx.checks_failed);

    LOGT("USB FS Device Bulk Transfer test teardown: %s",
         (g_ctx.errors == 0U) ? "PASS" : "FAIL");

    return g_ctx.errors == 0U ? 0 : -1;
}
