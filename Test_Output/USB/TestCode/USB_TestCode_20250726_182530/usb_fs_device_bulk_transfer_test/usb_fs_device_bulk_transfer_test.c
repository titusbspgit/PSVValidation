// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

/* Forward declaration for nic_programming */
void nic_programming(void);

/* Global volatile interrupt pending flag */
volatile int int_pend;

/* Global event counter */
int event_counter;

/* Global error tracking */
static int g_errors = 0;

/*
 * Function: setup_stage
 * Description: Configures event TRB for setup stage with control field 0x823,
 *              issues DEPCMD 0x506, polls until command completes,
 *              then waits for interrupt via int_pend polling.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void setup_stage(void)
{
    int rd_data;
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }
    LOGT("setup_stage: DEPCMD poll complete, rd_data=0x%x", rd_data);
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("setup_stage: interrupt received");
}

/*
 * Function: status_stage
 * Description: Configures event TRB for status stage with control field 0x843,
 *              issues DEPCMD 0x506, polls until command completes,
 *              waits for interrupt via int_pend polling, then adds
 *              an additional delay.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void status_stage(void)
{
    int rd_data;
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x843);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }
    LOGT("status_stage: DEPCMD poll complete, rd_data=0x%x", rd_data);
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("status_stage: interrupt received");
    /* Additional delay after status stage */
    for (volatile int d = 0; d < 5000; d++);
}

/*
 * Function: enumeration
 * Description: Populates device descriptor and configuration descriptor
 *              response data in Buffer_PointerLO and configures event TRBs
 *              with DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10.
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
    LOGT("enumeration: device descriptor response configured");

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
    LOGT("enumeration: configuration descriptor response configured");
}

/*
 * Function: set_configuration
 * Description: Issues endpoint command by writing DEPCMDPAR1, DEPCMDPAR0,
 *              and DEPCMD registers with the given offsets and values,
 *              then polls until command completes.
 * Parameters:
 *   trb_address - register offset for DEPCMD/DEPCMDPAR registers
 *   parameter0 - value for DEPCMDPAR0+trb_address
 *   parameter1 - value for DEPCMDPAR1+trb_address
 *   cmd - command value for DEPCMD+trb_address
 * Returns:
 *   void
 */
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
    int read_data;
    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
    read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    while (read_data == cmd) {
        for (volatile int d = 0; d < 30000; d++);
        read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    }
    LOGT("set_configuration: DEPCMD+0x%x poll complete, read_data=0x%x", trb_address, read_data);
}

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler that clears int_pend, reads GEVNTCOUNT
 *              and stores in event_counter, writes back to acknowledge,
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
    LOGT("Default_IRQHandler: MSK_STS0 = 0x%x", rd_data);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("Default_IRQHandler: RAW_STCR0 = 0x%x", rd_data);
    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    LOGT("Default_IRQHandler: GEVNTCOUNT = 0x%x", event_count);
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
    /* MANUAL_REVIEW: Source uses logical AND (&&) instead of bitwise AND (&). */
    /* Preserving source behavior exactly as: (rd_data && 0x80000000) */
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }
    GIC_ClearIRQ(84);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes the USB FS Device Bulk Transfer testcase.
 *              Calls nic_programming and enables all IRQs.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_errors = 0;

    LOGT("usb_fs_device_bulk_transfer_test_init: starting NIC programming");
    nic_programming();

    LOGT("usb_fs_device_bulk_transfer_test_init: enabling all IRQs");
    GIC_EnableAllIRQ();

    LOGT("usb_fs_device_bulk_transfer_test_init: init complete");
    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Main testcase execution. Clears buffer and event TRB memory,
 *              issues Start Transfer commands, performs USB device enumeration
 *              including setup/status stages, configures bulk data transfer
 *              TRBs, and uses interrupt-driven synchronization.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data, wr_data, port_count, db_offset;
    int i, bulk, intr, event_comletion, j;
    int hand_shake;

    (void)cfg;

    if (out == 0) {
        LOGE("usb_fs_device_bulk_transfer_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    /* Step 3: Clear 20 entries of Buffer_PointerLO and event_trb_addr */
    LOGT("Clearing 20 entries of Buffer_PointerLO and event_trb_addr");
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }
    LOGT("Buffer and event TRB memory cleared");

    /* Step 4: DEPCMD+0x10 Start Transfer command */
    LOGT("Issuing Start Transfer command to DEPCMD+0x10");
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }
    LOGT("DEPCMD+0x10 Start Transfer complete, rd_data=0x%x", rd_data);

    /* Step 5: First int_pend wait */
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("First int_pend wait complete");

    /* Intermediate delay (wait_on(5)) */
    for (volatile int d = 0; d < 5000; d++);

    /* Step 5: Second int_pend wait */
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("Second int_pend wait complete");

    /* Step 5: Third int_pend wait - ADDED to check the xfernotready event */
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("Third int_pend wait complete (xfernotready event)");

    /* Step 6: Call status_stage() */
    LOGT("Calling status_stage");
    status_stage();

    /* Step 7: USB_SET_CONFIGURATION_OR_RESET_TT - Call setup_stage() */
    LOGT("Calling setup_stage for USB_SET_CONFIGURATION_OR_RESET_TT");
    setup_stage();

    /* Step 8: DV debug marker write_reg(0xA0243ffc, 0xdeadbee4) - converted to PSV logging */
    LOGD("DV debug marker reached: 0xdeadbee4");

    /* Step 9: GET DESCRIPTOR USB CONFIGURATION - wait for interrupt */
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("Interrupt received for GET DESCRIPTOR USB CONFIGURATION");

    /* Step 9: Call setup_stage() for GET DESCRIPTOR USB CONFIGURATION */
    LOGT("Calling setup_stage for GET DESCRIPTOR USB CONFIGURATION");
    setup_stage();

    /* Step 10: DV debug marker write_reg(0xA0243ffc, 0xdeadbee5) - converted to PSV logging */
    LOGD("DV debug marker reached: 0xdeadbee5");

    /* Step 11: Wait for interrupt, proceed with configuration descriptor data stage */
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("Interrupt received for configuration descriptor data stage");

    /* data stage - configuration descriptor */
    /* 09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301 */

    /* enumeration() is commented out in source, preserving as comment */
    /* enumeration(); */

    /* Step 12-16: Bulk data transfer TRB setup */
    LOGT("Configuring bulk data transfer TRB");
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x00);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }
    LOGT("Bulk Start Transfer complete, rd_data=0x%x", rd_data);

    /* Commented-out Bulk endpoint TRB with Buffer_PointerLO_1 - preserved as comments */
    /* write_reg(event_trb_addr, Buffer_PointerLO_1); */
    /* write_reg(event_trb_addr + 0x8, 0x40); */
    /* write_reg(event_trb_addr + 0xc, 0x813); */
    /* write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr); */
    /* write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0); */
    /* write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506); */

    /* Step 17: Final int_pend wait */
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("Final int_pend wait complete");

    LOGT("usb_fs_device_bulk_transfer_test_run: test sequence complete");
    out->status = 0;

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Final teardown and status reporting for the USB FS Device
 *              Bulk Transfer testcase.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("usb_fs_device_bulk_transfer_test_teardown: teardown complete");
    return g_errors == 0 ? 0 : -1;
}
