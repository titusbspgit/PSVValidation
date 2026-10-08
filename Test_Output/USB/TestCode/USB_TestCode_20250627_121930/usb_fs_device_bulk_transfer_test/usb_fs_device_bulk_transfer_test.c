// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

/* Global variables from DV testcase */
extern int int_pend;
int event_counter;

/*
 * Function: psv_delay
 * Description: Deterministic busy-wait delay to replace DV wait_on().
 * Parameters:
 *   count - relative delay iteration count
 * Returns:
 *   void
 */
static void psv_delay(int count)
{
    volatile int d;
    for (d = 0; d < count * 1000; d++);
}

/*
 * Function: setup_stage
 * Description: Configures event TRB for setup stage and issues DEPCMD
 *              Start Transfer command with polling for completion.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void setup_stage(void)
{
    int rd_data;
    int timeout;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    timeout = 0;
    while (rd_data == 0x506) {
        psv_delay(10);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
        timeout++;
        if (timeout > 10000) {
            LOGE("setup_stage: DEPCMD polling timeout");
            break;
        }
    }
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(5);
        timeout++;
        if (timeout > 10000) {
            LOGE("setup_stage: int_pend polling timeout");
            break;
        }
    }
}

/*
 * Function: status_stage
 * Description: Configures event TRB for status stage and issues DEPCMD
 *              Start Transfer command with polling for completion.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void status_stage(void)
{
    int rd_data;
    int timeout;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x843);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    timeout = 0;
    while (rd_data == 0x506) {
        psv_delay(10);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
        timeout++;
        if (timeout > 10000) {
            LOGE("status_stage: DEPCMD polling timeout");
            break;
        }
    }
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(5);
        timeout++;
        if (timeout > 10000) {
            LOGE("status_stage: int_pend polling timeout");
            break;
        }
    }
    psv_delay(5);
}

/*
 * Function: enumeration
 * Description: Programs device descriptor and configuration descriptor
 *              response data into TRBs and buffer memory.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void enumeration(void)
{
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
}

/*
 * Function: set_configuration
 * Description: Issues endpoint command by writing DEPCMDPAR1, DEPCMDPAR0,
 *              and DEPCMD registers with polling for command completion.
 * Parameters:
 *   trb_address - offset added to DEPCMD register base
 *   parameter0  - value for DEPCMDPAR0
 *   parameter1  - value for DEPCMDPAR1
 *   cmd         - command value for DEPCMD
 * Returns:
 *   void
 */
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
    int read_data;
    int timeout;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
    read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    timeout = 0;
    while (read_data == cmd) {
        psv_delay(30);
        read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
        timeout++;
        if (timeout > 10000) {
            LOGE("set_configuration: DEPCMD polling timeout");
            break;
        }
    }
}

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler that clears int_pend, reads GEVNTCOUNT,
 *              stores in event_counter, writes back to acknowledge events,
 *              reads system register status, clears raw interrupt status,
 *              and calls GIC_ClearIRQ(84).
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rd_data, sysreg_rd_data, event_count;

    int_pend = 0;
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
    /* NOTE: Source uses logical AND (&&) not bitwise AND (&). Preserved as-is per No Silent Correction Rule. */
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }
    GIC_ClearIRQ(84);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes USB device controller, enables IRQs, and clears
 *              buffer and event TRB memory regions.
 * Parameters:
 *   cfg - pointer to test configuration structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    int j;

    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test init start");

    /* Step 1: Call nic_programming() for USB device controller initialization */
    nic_programming();

    /* Step 2: Enable all IRQs via GIC_EnableAllIRQ() */
    GIC_EnableAllIRQ();

    /* Step 3: Clear 20 entries of Buffer_PointerLO and event_trb_addr memory regions */
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }

    LOGT("USB FS Device Bulk Transfer test init complete");

    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Executes the USB Full-Speed Device Bulk Transfer test flow
 *              including enumeration, setup/status stages, bulk TRB setup,
 *              and interrupt-driven synchronization.
 * Parameters:
 *   cfg - pointer to test configuration structure
 *   out - pointer to test output structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("USB output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run start");

    /* Step 4: Issue Start Transfer command by writing 0x506 to DEPCMD+0x10, poll until complete */
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    timeout = 0;
    while (rd_data == 0x506) {
        psv_delay(10);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        timeout++;
        if (timeout > 10000) {
            LOGE("DEPCMD+0x10 Start Transfer polling timeout");
            out->status = -1;
            break;
        }
    }

    /* Step 5: First int_pend wait */
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(5);
        timeout++;
        if (timeout > 10000) {
            LOGE("int_pend polling timeout (1st)");
            out->status = -1;
            break;
        }
    }

    psv_delay(5);

    /* Step 5: Second int_pend wait */
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(5);
        timeout++;
        if (timeout > 10000) {
            LOGE("int_pend polling timeout (2nd)");
            out->status = -1;
            break;
        }
    }

    /* Step 5: Third int_pend wait - ADDED to check the xfernotready event */
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(5);
        timeout++;
        if (timeout > 10000) {
            LOGE("int_pend polling timeout (3rd - xfernotready)");
            out->status = -1;
            break;
        }
    }

    /* Step 6: Call status_stage() to complete status phase of control transfer */
    status_stage();

    /* Step 7: Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT */
    setup_stage();

    /* Step 8: DV debug marker 0xdeadbee4 - converted to PSV logging */
    LOGD("DV debug marker reached: 0xdeadbee4");

    /* Step 9: Wait for interrupt (GET DESCRIPTOR USB CONFIGURATION) */
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(5);
        timeout++;
        if (timeout > 10000) {
            LOGE("int_pend polling timeout (GET DESCRIPTOR)");
            out->status = -1;
            break;
        }
    }

    /* Step 9: Call setup_stage() for GET DESCRIPTOR USB CONFIGURATION */
    setup_stage();

    /* Step 10: DV debug marker 0xdeadbee5 - converted to PSV logging */
    LOGD("DV debug marker reached: 0xdeadbee5");

    /* Step 11: Wait for interrupt, proceed with configuration descriptor data stage */
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(5);
        timeout++;
        if (timeout > 10000) {
            LOGE("int_pend polling timeout (config descriptor data stage)");
            out->status = -1;
            break;
        }
    }

    /* data stage - configuration descriptor */
    /* 09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301 */

    /* enumeration() call is commented out in source - preserved as comment */
    /* enumeration(); */

    /* Step 12-15: Bulk data transfer TRB setup */
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x00);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    /* Step 16: Issue DEPCMD+0x10 with 0x506 for bulk endpoint Start Transfer, poll until complete */
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    timeout = 0;
    while (rd_data == 0x506) {
        psv_delay(10);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        timeout++;
        if (timeout > 10000) {
            LOGE("Bulk DEPCMD+0x10 Start Transfer polling timeout");
            out->status = -1;
            break;
        }
    }

    /* Bulk endpoint TRB with Buffer_PointerLO_1 - commented out in source */
    /* write_reg(event_trb_addr, Buffer_PointerLO_1); */
    /* write_reg(event_trb_addr + 0x8, 0x40); */
    /* write_reg(event_trb_addr + 0xc, 0x813); */
    /* write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr); */
    /* write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0); */
    /* write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506); */

    /* Step 17: Wait for interrupt completion via int_pend polling */
    int_pend = 1;
    timeout = 0;
    while (int_pend) {
        psv_delay(5);
        timeout++;
        if (timeout > 10000) {
            LOGE("int_pend polling timeout (final bulk transfer)");
            out->status = -1;
            break;
        }
    }

    LOGT("USB FS Device Bulk Transfer test run complete: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Performs final cleanup for the USB FS Device Bulk Transfer test.
 * Parameters:
 *   cfg - pointer to test configuration structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown: no additional cleanup required");

    return 0;
}
