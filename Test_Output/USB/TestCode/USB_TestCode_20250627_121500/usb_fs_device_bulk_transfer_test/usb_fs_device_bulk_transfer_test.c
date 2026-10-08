// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

/* Extern interrupt pending flag used by IRQ handler */
extern int int_pend;

/* Global event counter updated by IRQ handler */
int event_counter;

/*
 * Function: setup_stage
 * Description: Configures event TRB for setup stage with control field 0x823,
 *              issues DEPCMD Start Transfer (0x506), polls for completion,
 *              then waits for interrupt via int_pend polling.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void setup_stage(void)
{
    int rd_data;
    int count;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);

    rd_data = read_reg(MIZAR_USB_DEPCMD);
    count = 0;
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
        count++;
        if (count > 100000) {
            LOGE("setup_stage: timeout polling DEPCMD");
            break;
        }
    }
    LOGT("setup_stage: DEPCMD poll complete, rd_data=0x%x", rd_data);

    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
        count++;
        if (count > 100000) {
            LOGE("setup_stage: timeout waiting for interrupt");
            break;
        }
    }
    LOGT("setup_stage: interrupt received");
}

/*
 * Function: status_stage
 * Description: Configures event TRB for status stage with control field 0x843,
 *              issues DEPCMD Start Transfer (0x506), polls for completion,
 *              waits for interrupt, then performs an additional delay.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void status_stage(void)
{
    int rd_data;
    int count;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x843);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);

    rd_data = read_reg(MIZAR_USB_DEPCMD);
    count = 0;
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
        count++;
        if (count > 100000) {
            LOGE("status_stage: timeout polling DEPCMD");
            break;
        }
    }
    LOGT("status_stage: DEPCMD poll complete, rd_data=0x%x", rd_data);

    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
        count++;
        if (count > 100000) {
            LOGE("status_stage: timeout waiting for interrupt");
            break;
        }
    }
    LOGT("status_stage: interrupt received");

    /* Additional wait_on(5) after int_pend loop */
    for (volatile int d = 0; d < 500; d++);
}

/*
 * Function: enumeration
 * Description: Populates device descriptor and configuration descriptor
 *              response data in event TRB and Buffer_PointerLO memory,
 *              and sets DEPCMDPAR1/DEPCMDPAR0 for transfer.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void enumeration(void)
{
    LOGT("enumeration: populating device descriptor response");

    /* Device descriptor response */
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);
    /* data */
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    LOGT("enumeration: populating configuration descriptor response");

    /* Configuration descriptor response */
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x8, 0x00000032);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    LOGT("enumeration: complete");
}

/*
 * Function: set_configuration
 * Description: Issues endpoint command by writing DEPCMDPAR1, DEPCMDPAR0,
 *              and DEPCMD registers at the specified trb_address offset,
 *              then polls for command completion.
 * Parameters:
 *   trb_address - offset added to DEPCMD register base
 *   parameter0  - value for DEPCMDPAR0 + trb_address
 *   parameter1  - value for DEPCMDPAR1 + trb_address
 *   cmd         - command value for DEPCMD + trb_address
 * Returns:
 *   void
 */
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
    int read_data;
    int count;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);

    read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    count = 0;
    while (read_data == cmd) {
        for (volatile int d = 0; d < 3000; d++);
        read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
        count++;
        if (count > 100000) {
            LOGE("set_configuration: timeout polling DEPCMD+0x%x", trb_address);
            break;
        }
    }
    LOGT("set_configuration: DEPCMD+0x%x poll complete, read_data=0x%x", trb_address, read_data);
}

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler. Clears int_pend, reads GEVNTCOUNT
 *              and stores in event_counter, writes back to acknowledge events,
 *              reads system register status, clears raw interrupt status,
 *              and calls GIC_ClearIRQ(84).
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rd_data, sysreg_rd_data, event_count;

    int_pend = 0;

    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("IRQHandler: MSK_STS0 = 0x%x", rd_data);

    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("IRQHandler: RAW_STCR0 = 0x%x", rd_data);

    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    LOGT("IRQHandler: GEVNTCOUNT = 0x%x", event_count);
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    /* NOTE: Source uses logical AND (&&) not bitwise AND (&). Preserved as-is per No Silent Correction Rule. */
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    GIC_ClearIRQ(84);
    LOGT("IRQHandler: GIC IRQ 84 cleared");
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes the USB device controller for Full-Speed Bulk
 *              Transfer test. Calls nic_programming(), enables IRQs,
 *              and clears buffer and event TRB memory regions.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
    int j;

    (void)cfg;

    LOGT("usb_fs_device_bulk_transfer_test_init: starting");

    /* Step 1: NIC / USB device controller initialization */
    nic_programming();
    LOGT("nic_programming() complete");

    /* Step 2: Enable all IRQs */
    GIC_EnableAllIRQ();
    LOGT("GIC_EnableAllIRQ() complete");

    /* Step 3: Clear 20 entries of Buffer_PointerLO and event_trb_addr */
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }
    LOGT("Buffer_PointerLO and event_trb_addr cleared (20 entries)");

    LOGT("usb_fs_device_bulk_transfer_test_init: complete");
    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Runs the USB Full-Speed Device Bulk Transfer test.
 *              Issues Start Transfer commands, performs interrupt-driven
 *              synchronization, executes control transfer stages,
 *              and performs bulk data transfer via TRBs.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
    int rd_data;
    int count;

    (void)cfg;

    if (out == 0) {
        LOGE("usb_fs_device_bulk_transfer_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("usb_fs_device_bulk_transfer_test_run: starting");

    /* Step 4: DEPCMD+0x10 Start Transfer command */
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    count = 0;
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        count++;
        if (count > 100000) {
            LOGE("Timeout polling DEPCMD+0x10 (first Start Transfer)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("DEPCMD+0x10 Start Transfer complete, rd_data=0x%x", rd_data);

    /* Step 5: First int_pend wait */
    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
        count++;
        if (count > 100000) {
            LOGE("Timeout waiting for interrupt (first int_pend)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("First int_pend wait complete");

    /* Step 5 continued: wait_on(5) between first and second int_pend */
    for (volatile int d = 0; d < 500; d++);

    /* Step 5 continued: Second int_pend wait */
    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
        count++;
        if (count > 100000) {
            LOGE("Timeout waiting for interrupt (second int_pend)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("Second int_pend wait complete");

    /* Step 5 continued: Third int_pend wait (xfernotready event) */
    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
        count++;
        if (count > 100000) {
            LOGE("Timeout waiting for interrupt (third int_pend - xfernotready)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("Third int_pend wait complete (xfernotready)");

    /* Step 6: Call status_stage() */
    status_stage();
    LOGT("status_stage() complete");

    /* Step 7: Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT */
    setup_stage();
    LOGT("setup_stage() complete (USB_SET_CONFIGURATION_OR_RESET_TT)");

    /* Step 8: DV debug marker write_reg(0xA0243ffc, 0xdeadbee4) */
    /* MANUAL_REVIEW: DV debug marker at 0xA0243ffc with value 0xdeadbee4 - converted to PSV logging */
    LOGD("DV debug marker reached: 0xdeadbee4");

    /* Step 9: GET DESCRIPTOR USB CONFIGURATION - wait for interrupt */
    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
        count++;
        if (count > 100000) {
            LOGE("Timeout waiting for interrupt (GET DESCRIPTOR USB CONFIGURATION)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("Interrupt received for GET DESCRIPTOR USB CONFIGURATION");

    /* Step 9 continued: Call setup_stage() for GET DESCRIPTOR USB CONFIGURATION */
    setup_stage();
    LOGT("setup_stage() complete (GET DESCRIPTOR USB CONFIGURATION)");

    /* Step 10: DV debug marker write_reg(0xA0243ffc, 0xdeadbee5) */
    /* MANUAL_REVIEW: DV debug marker at 0xA0243ffc with value 0xdeadbee5 - converted to PSV logging */
    LOGD("DV debug marker reached: 0xdeadbee5");

    /* Step 11: Wait for interrupt, proceed with configuration descriptor data stage */
    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
        count++;
        if (count > 100000) {
            LOGE("Timeout waiting for interrupt (config descriptor data stage)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("Interrupt received for configuration descriptor data stage");

    /* data stage - configuration descriptor */
    /* 09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301 */

    /* Source has enumeration() commented out: // enumeration(); */
    /* Preserving as comment per source */
    /* // enumeration(); */

    /* Steps 12-16: Bulk data transfer TRB setup */
    LOGT("Bulk data transfer TRB setup");
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x00);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    count = 0;
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        count++;
        if (count > 100000) {
            LOGE("Timeout polling DEPCMD+0x10 (bulk Start Transfer)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("Bulk DEPCMD+0x10 Start Transfer complete, rd_data=0x%x", rd_data);

    /* Commented-out Bulk endpoint TRB with Buffer_PointerLO_1 - preserved as source comments */
    /* write_reg(event_trb_addr, Buffer_PointerLO_1); */
    /* write_reg(event_trb_addr + 0x8, 0x40); */
    /* write_reg(event_trb_addr + 0xc, 0x813); */
    /* write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr); */
    /* write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0); */
    /* write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506); */

    /* Step 17: Final int_pend wait for bulk transfer completion */
    int_pend = 1;
    count = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
        count++;
        if (count > 100000) {
            LOGE("Timeout waiting for interrupt (bulk transfer completion)");
            out->status = -1;
            return -1;
        }
    }
    LOGT("Bulk transfer interrupt received");

    LOGT("usb_fs_device_bulk_transfer_test_run: complete, status=%d", out->status);
    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB Full-Speed Device Bulk Transfer test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("usb_fs_device_bulk_transfer_test_teardown: no additional cleanup required");
    return 0;
}
