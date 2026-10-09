// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Forward declaration for nic_programming
void nic_programming(void);

// Forward declarations for internal helpers
static void setup_stage(void);
static void status_stage(void);
static void enumeration(void);

/*
 * Function: set_configuration
 * Description: Issues endpoint command by writing DEPCMDPAR1, DEPCMDPAR0,
 *   and DEPCMD registers at the given trb_address offset, then polls
 *   DEPCMD until command completes.
 * Parameters:
 *   trb_address - offset added to DEPCMD register base
 *   parameter0 - value for DEPCMDPAR0 + trb_address
 *   parameter1 - value for DEPCMDPAR1 + trb_address
 *   cmd - command value written to DEPCMD + trb_address
 * Returns:
 *   void
 */
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
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
}

/*
 * Function: Default_IRQHandler
 * Description: USB device interrupt handler. Clears int_pend, reads
 *   GEVNTCOUNT and stores in event_counter, writes back to acknowledge,
 *   reads system register status, clears raw interrupt status, and
 *   calls GIC_ClearIRQ(84).
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
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
    // MANUAL_REVIEW: Source uses logical AND (&&) instead of bitwise AND (&).
    // Preserving source behavior exactly as supplied.
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }
    GIC_ClearIRQ(84);
}

/*
 * Function: setup_stage
 * Description: Configures event TRB for setup stage with control field 0x823,
 *   issues DEPCMD 0x506, polls until command completes, then waits for
 *   interrupt via int_pend polling.
 * Parameters:
 *   None
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
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
}

/*
 * Function: status_stage
 * Description: Configures event TRB for status stage with control field 0x843,
 *   issues DEPCMD 0x506, polls until command completes, waits for interrupt
 *   via int_pend polling, then performs an additional delay.
 * Parameters:
 *   None
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
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    // Additional delay after status stage
    for (volatile int d = 0; d < 5000; d++);
}

/*
 * Function: enumeration
 * Description: Populates device descriptor and configuration descriptor
 *   response data in Buffer_PointerLO and configures event TRBs with
 *   DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void enumeration(void)
{
    // Device descriptor response
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);
    // data
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    // Configuration descriptor response
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
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes the USB FS device bulk transfer testcase.
 *   Calls nic_programming for controller init, enables GIC IRQs,
 *   clears 20 entries of Buffer_PointerLO and event_trb_addr memory.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    int j;

    (void)cfg;

    LOGT("usb_fs_device_bulk_transfer_test_init: starting init");

    // Step 1: Call nic_programming() for USB device controller initialization
    nic_programming();
    LOGT("usb_fs_device_bulk_transfer_test_init: nic_programming done");

    // Step 2: Enable all IRQs via GIC_EnableAllIRQ()
    GIC_EnableAllIRQ();
    LOGT("usb_fs_device_bulk_transfer_test_init: GIC_EnableAllIRQ done");

    // Step 3: Clear 20 entries of Buffer_PointerLO and event_trb_addr
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }
    LOGT("usb_fs_device_bulk_transfer_test_init: buffer and event TRB memory cleared");

    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Runs the USB FS device bulk transfer testcase.
 *   Issues Start Transfer commands, performs interrupt-driven polling,
 *   calls status_stage, setup_stage, enumeration, configures bulk TRBs,
 *   and performs bulk data transfer with interrupt synchronization.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;

    (void)cfg;

    if (out == 0) {
        LOGE("usb_fs_device_bulk_transfer_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("usb_fs_device_bulk_transfer_test_run: starting run");

    // Step 4: DEPCMD+0x10 Start Transfer command
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }
    LOGT("usb_fs_device_bulk_transfer_test_run: first DEPCMD+0x10 Start Transfer complete");

    // Step 5a: First int_pend poll
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("usb_fs_device_bulk_transfer_test_run: first interrupt received");

    // Step 5b: Intermediate delay (wait_on(5))
    for (volatile int d = 0; d < 5000; d++);

    // Step 5c: Second int_pend poll
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("usb_fs_device_bulk_transfer_test_run: second interrupt received");

    // Step 5d: Third int_pend poll (ADDED to check the xfernotready event)
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("usb_fs_device_bulk_transfer_test_run: third interrupt (xfernotready) received");

    // Step 6: Call status_stage()
    status_stage();
    LOGT("usb_fs_device_bulk_transfer_test_run: status_stage done");

    // Step 7: Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT
    setup_stage();
    LOGT("usb_fs_device_bulk_transfer_test_run: setup_stage (SET_CONFIGURATION) done");

    // Step 8: DV debug marker 0xdeadbee4 - converted to PSV logging
    LOGD("DV debug marker reached: 0xdeadbee4");

    // Step 9: GET DESCRIPTOR USB CONFIGURATION - wait for interrupt
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("usb_fs_device_bulk_transfer_test_run: interrupt before GET DESCRIPTOR received");

    // Step 9 continued: Call setup_stage() for GET DESCRIPTOR USB CONFIGURATION
    setup_stage();
    LOGT("usb_fs_device_bulk_transfer_test_run: setup_stage (GET DESCRIPTOR) done");

    // Step 10: DV debug marker 0xdeadbee5 - converted to PSV logging
    LOGD("DV debug marker reached: 0xdeadbee5");

    // Step 11: Wait for interrupt, proceed with configuration descriptor data stage
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("usb_fs_device_bulk_transfer_test_run: interrupt before config descriptor data stage received");

    // data stage - configuration descriptor
    // 09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff
    // 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301

    // enumeration() is commented out in source; preserving as comment
    // enumeration();

    // Steps 12-16: Bulk data transfer TRB setup
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
    LOGT("usb_fs_device_bulk_transfer_test_run: bulk DEPCMD+0x10 Start Transfer complete");

    // Bulk endpoint TRB with Buffer_PointerLO_1 (commented out in source)
    // write_reg(event_trb_addr, Buffer_PointerLO_1);
    // write_reg(event_trb_addr + 0x8, 0x40);
    // write_reg(event_trb_addr + 0xc, 0x813);
    // write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
    // write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
    // write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);

    // Step 17: Wait for interrupt completion via int_pend polling
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    LOGT("usb_fs_device_bulk_transfer_test_run: final bulk transfer interrupt received");

    LOGT("usb_fs_device_bulk_transfer_test_run: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB FS device bulk transfer testcase.
 *   Reports final status.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("usb_fs_device_bulk_transfer_test_teardown: teardown complete");
    return 0;
}
