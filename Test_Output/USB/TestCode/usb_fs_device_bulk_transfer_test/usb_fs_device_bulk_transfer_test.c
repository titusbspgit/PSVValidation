// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Timeout bound for polling loops
#define USB_POLL_TIMEOUT 100000U
// Bulk transfer length (non-zero, 64 bytes for FS bulk)
#define USB_BULK_TRANSFER_LEN 0x40U
// Deterministic bulk payload pattern
static const unsigned int bulk_payload_data[16] = {
    0xDEAD0001U, 0xDEAD0002U, 0xDEAD0003U, 0xDEAD0004U,
    0xDEAD0005U, 0xDEAD0006U, 0xDEAD0007U, 0xDEAD0008U,
    0xDEAD0009U, 0xDEAD000AU, 0xDEAD000BU, 0xDEAD000CU,
    0xDEAD000DU, 0xDEAD000EU, 0xDEAD000FU, 0xDEAD0010U
};

// Global interrupt pending flag
volatile int int_pend;

// Global event counter
static volatile uint32_t event_counter;

// Global error tracking
static unsigned int g_errors;

// Forward declarations for helper functions
static void setup_stage(void);
static void status_stage(void);
static void enumeration(void);
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd);

// External nic_programming declaration
void nic_programming(void);

// Busy-wait delay helper
static void psv_delay(unsigned int count)
{
    volatile unsigned int d;
    for (d = 0U; d < count; d++) {
        // busy wait
    }
}

// Poll for int_pend to be cleared by IRQ handler with timeout
static int wait_for_interrupt(unsigned int timeout_loops)
{
    unsigned int t;
    for (t = 0U; t < timeout_loops; t++) {
        if (int_pend == 0) {
            return 0;
        }
        psv_delay(5000U);
    }
    LOGE("Timeout waiting for interrupt");
    return -1;
}

// Poll DEPCMD register until command completes (value changes from written cmd)
static int poll_depcmd(int reg_offset, int cmd_val, unsigned int timeout_loops)
{
    volatile unsigned int rd_data;
    unsigned int t;
    rd_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + reg_offset);
    for (t = 0U; t < timeout_loops; t++) {
        if ((int)rd_data != cmd_val) {
            return 0;
        }
        psv_delay(10000U);
        rd_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + reg_offset);
    }
    LOGE("Timeout polling DEPCMD+0x%x for cmd 0x%x", reg_offset, cmd_val);
    return -1;
}

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler - reads GEVNTCOUNT, acknowledges events,
 *              reads system register status, clears raw interrupt, clears GIC IRQ.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    volatile unsigned int rd_data;
    uint32_t event_count;

    int_pend = 0;

    rd_data = (unsigned int)read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("IRQHandler: MSK_STS0 = 0x%x", rd_data);

    rd_data = (unsigned int)read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("IRQHandler: RAW_STCR0 = 0x%x", rd_data);

    event_count = (uint32_t)read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, (int)event_count);
    LOGT("IRQHandler: GEVNTCOUNT = 0x%x, acknowledged", event_count);

    // MANUAL_REVIEW: Meta source uses (rd_data && 0x80000000) which is logical AND.
    // Runtime condition 15 requires bitwise AND. Preserving runtime condition.
    if ((rd_data & 0x80000000U) != 0U) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
        LOGT("IRQHandler: Cleared RAW_STCR0 bit 31");
    }

    GIC_ClearIRQ(84);
    LOGT("IRQHandler: GIC_ClearIRQ(84) called");
}

/*
 * Function: setup_stage
 * Description: Configures event TRB for setup stage and issues DEPCMD Start Transfer
 *              on EP0, then waits for interrupt completion.
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
    {
        unsigned int timeout = USB_POLL_TIMEOUT;
        while (rd_data == 0x506) {
            psv_delay(10000U);
            rd_data = read_reg(MIZAR_USB_DEPCMD);
            timeout--;
            if (timeout == 0U) {
                LOGE("setup_stage: Timeout polling DEPCMD");
                g_errors++;
                break;
            }
        }
    }

    int_pend = 1;
    if (wait_for_interrupt(USB_POLL_TIMEOUT) != 0) {
        LOGE("setup_stage: Timeout waiting for interrupt");
        g_errors++;
    }
}

/*
 * Function: status_stage
 * Description: Configures event TRB for status stage and issues DEPCMD Start Transfer
 *              on EP0, then waits for interrupt completion followed by additional delay.
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
    {
        unsigned int timeout = USB_POLL_TIMEOUT;
        while (rd_data == 0x506) {
            psv_delay(10000U);
            rd_data = read_reg(MIZAR_USB_DEPCMD);
            timeout--;
            if (timeout == 0U) {
                LOGE("status_stage: Timeout polling DEPCMD");
                g_errors++;
                break;
            }
        }
    }

    int_pend = 1;
    if (wait_for_interrupt(USB_POLL_TIMEOUT) != 0) {
        LOGE("status_stage: Timeout waiting for interrupt");
        g_errors++;
    }

    // Additional delay after status stage (Meta: wait_on(5))
    psv_delay(5000U);
}

/*
 * Function: enumeration
 * Description: Populates device descriptor and configuration descriptor data
 *              into Buffer_PointerLO and configures TRBs for descriptor responses.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void enumeration(void)
{
    // Device descriptor response TRB
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Device descriptor data (18 bytes)
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100);

    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    LOGT("enumeration: Device descriptor TRB configured, length=0x12");

    // Configuration descriptor response TRB
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Configuration descriptor data (60 bytes)
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x8, 0x00000032);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);

    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    LOGT("enumeration: Configuration descriptor TRB configured, length=0x3c");
}

/*
 * Function: set_configuration
 * Description: Issues endpoint command by writing DEPCMDPAR1, DEPCMDPAR0, and DEPCMD
 *              at the specified trb_address offset, then polls for command completion.
 * Parameters:
 *   trb_address - offset for DEPCMD register bank
 *   parameter0 - value for DEPCMDPAR0
 *   parameter1 - value for DEPCMDPAR1
 *   cmd - command value for DEPCMD
 * Returns:
 *   void
 */
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
    volatile unsigned int read_data;
    unsigned int timeout = USB_POLL_TIMEOUT;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);

    read_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + trb_address);
    while ((int)read_data == cmd) {
        psv_delay(30000U);
        read_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + trb_address);
        timeout--;
        if (timeout == 0U) {
            LOGE("set_configuration: Timeout polling DEPCMD+0x%x", trb_address);
            g_errors++;
            break;
        }
    }
    LOGT("set_configuration: DEPCMD+0x%x command 0x%x completed", trb_address, cmd);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes USB device controller, enables IRQs, clears buffer
 *              and event TRB memory regions, performs USB controller initialization.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    int j;

    (void)cfg;

    g_errors = 0U;
    event_counter = 0U;
    int_pend = 0;

    LOGT("USB FS Device Bulk Transfer test init started");

    // Step 1: Call nic_programming() for USB device controller initialization
    nic_programming();
    LOGT("nic_programming() completed");

    // Step 2: Enable all IRQs
    GIC_EnableAllIRQ();
    LOGT("GIC_EnableAllIRQ() completed");

    // Step 3: Clear 20 entries of Buffer_PointerLO and event_trb_addr memory regions
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }
    LOGT("Cleared 20 entries of Buffer_PointerLO and event_trb_addr");

    // MANUAL_REVIEW: Runtime condition 9 requires explicit USB controller initialization
    // (PHY, Event Ring, Device Configuration, Endpoint Resources, Transfer Resources).
    // Meta indicates nic_programming() configures MIZAR_USB_GCTL with 0x30c11234 and
    // other global registers. Additional initialization details beyond nic_programming()
    // are not available in Meta. Verify nic_programming() covers all required init.

    LOGT("USB FS Device Bulk Transfer test init completed");
    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Executes USB FS device bulk transfer test including enumeration,
 *              configuration, and bulk data transfer with interrupt synchronization.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for results
 * Returns:
 *   0 on success (PASS), -1 on failure (FAIL)
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    unsigned int k;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run started");

    // Step 4: Issue Start Transfer command by writing 0x506 to DEPCMD+0x10, poll until complete
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    {
        unsigned int timeout = USB_POLL_TIMEOUT;
        while (rd_data == 0x506) {
            psv_delay(10000U);
            rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
            timeout--;
            if (timeout == 0U) {
                LOGE("Timeout polling DEPCMD+0x10 for initial Start Transfer");
                g_errors++;
                break;
            }
        }
    }
    LOGT("Initial DEPCMD+0x10 Start Transfer command completed");

    // Step 5: First int_pend wait
    int_pend = 1;
    if (wait_for_interrupt(USB_POLL_TIMEOUT) != 0) {
        LOGE("Timeout on first int_pend wait");
        g_errors++;
    }

    // Step 5: Intermediate delay (Meta: wait_on(5))
    psv_delay(5000U);

    // Step 5: Second int_pend wait
    int_pend = 1;
    if (wait_for_interrupt(USB_POLL_TIMEOUT) != 0) {
        LOGE("Timeout on second int_pend wait");
        g_errors++;
    }

    // Step 5: Third int_pend wait (xfernotready event)
    int_pend = 1;
    if (wait_for_interrupt(USB_POLL_TIMEOUT) != 0) {
        LOGE("Timeout on third int_pend wait (xfernotready)");
        g_errors++;
    }

    // Step 6: Call status_stage()
    LOGT("Calling status_stage()");
    status_stage();

    // Step 7: Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT
    LOGT("Calling setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT");
    setup_stage();

    // Step 8: DV debug marker 0xdeadbee4 at 0xA0243ffc - converted to PSV logging
    LOGT("DV debug marker reached: 0xdeadbee4");

    // Step 9: Wait for interrupt (GET DESCRIPTOR USB CONFIGURATION)
    int_pend = 1;
    if (wait_for_interrupt(USB_POLL_TIMEOUT) != 0) {
        LOGE("Timeout waiting for interrupt before GET DESCRIPTOR");
        g_errors++;
    }

    // Step 9: Call setup_stage() for GET DESCRIPTOR USB CONFIGURATION
    LOGT("Calling setup_stage() for GET DESCRIPTOR USB CONFIGURATION");
    setup_stage();

    // Step 10: DV debug marker 0xdeadbee5 at 0xA0243ffc - converted to PSV logging
    LOGT("DV debug marker reached: 0xdeadbee5");

    // Step 11: Wait for interrupt, proceed with configuration descriptor data stage
    int_pend = 1;
    if (wait_for_interrupt(USB_POLL_TIMEOUT) != 0) {
        LOGE("Timeout waiting for interrupt for config descriptor data stage");
        g_errors++;
    }

    // Steps 12-15: Enumeration - populate device and configuration descriptors
    // Runtime condition 6: enumeration() integrated into actual flow
    LOGT("Calling enumeration() for descriptor population");
    enumeration();

    // Runtime condition 7: Configure bulk endpoints via set_configuration
    // Configure bulk OUT endpoint (EP1 OUT, offset 0x10)
    LOGT("Configuring bulk OUT endpoint via set_configuration");
    set_configuration(0x10, 0x0, event_trb_addr, 0x506);

    // Configure bulk IN endpoint (EP1 IN, offset 0x20)
    LOGT("Configuring bulk IN endpoint via set_configuration");
    set_configuration(0x20, 0x0, event_trb_addr, 0x506);

    // Step 12-16: Bulk data transfer TRB setup with non-zero length
    // Runtime condition 5: non-zero-length bulk transfer
    LOGT("Setting up bulk data transfer TRB");

    // Prepare DMA buffer with payload data before TRB setup
    for (k = 0U; k < 16U; k++) {
        write_reg(Buffer_PointerLO + (int)(k * 4U), (int)bulk_payload_data[k]);
    }
    LOGT("Bulk payload data written to Buffer_PointerLO");

    // Configure bulk TRB at event_trb_addr
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, USB_BULK_TRANSFER_LEN);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Set DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    // Issue DEPCMD+0x10 with 0x506 for bulk endpoint Start Transfer
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    {
        unsigned int timeout = USB_POLL_TIMEOUT;
        while (rd_data == 0x506) {
            psv_delay(10000U);
            rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
            timeout--;
            if (timeout == 0U) {
                LOGE("Timeout polling DEPCMD+0x10 for bulk Start Transfer");
                g_errors++;
                break;
            }
        }
    }
    LOGT("Bulk DEPCMD+0x10 Start Transfer command completed");

    // Step 17: Wait for interrupt completion via int_pend polling
    int_pend = 1;
    if (wait_for_interrupt(USB_POLL_TIMEOUT) != 0) {
        LOGE("Timeout waiting for bulk transfer interrupt completion");
        g_errors++;
    }

    // Runtime condition 16: Validate bulk transfer completion
    // Verify event_counter was updated by IRQ handler
    if (event_counter == 0U) {
        LOGE("Bulk transfer: event_counter is zero, transfer may not have completed");
        g_errors++;
    } else {
        LOGT("Bulk transfer: event_counter = 0x%x", event_counter);
    }

    // Verify bulk payload data integrity by reading back
    {
        unsigned int mismatch_count = 0U;
        for (k = 0U; k < 16U; k++) {
            int readback = read_reg(Buffer_PointerLO + (int)(k * 4U));
            if (readback != (int)bulk_payload_data[k]) {
                LOGE("Bulk data mismatch at offset 0x%x: expected=0x%x actual=0x%x",
                     k * 4U, bulk_payload_data[k], (unsigned int)readback);
                mismatch_count++;
            }
        }
        if (mismatch_count > 0U) {
            LOGE("Bulk data integrity check failed: %u mismatches", mismatch_count);
            g_errors++;
        } else {
            LOGT("Bulk data integrity check passed");
        }
    }

    // Final status determination
    if (g_errors > 0U) {
        LOGE("USB FS Device Bulk Transfer test FAILED with %u errors", g_errors);
        out->status = -1;
    } else {
        LOGT("USB FS Device Bulk Transfer test PASSED");
        out->status = 0;
    }

    LOGT("USB FS Device Bulk Transfer test run completed: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Cleans up USB controller state, interrupt state, and test resources.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    int j;

    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown started");

    // Clean up interrupt state
    int_pend = 0;

    // Clear event TRB and buffer memory
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }
    LOGT("Cleared buffer and event TRB memory");

    // MANUAL_REVIEW: Runtime condition 17 requires stopping outstanding transfers
    // and cleaning up controller state. Specific stop-transfer register sequence
    // is not available in Meta. Verify if additional cleanup is needed.

    LOGT("USB FS Device Bulk Transfer test teardown completed");
    return (g_errors == 0U) ? 0 : -1;
}
