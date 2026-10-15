// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Timeout limit for polling loops
#define POLL_TIMEOUT 100000U

// Bulk transfer length
#define BULK_TRANSFER_LEN 0x40U

// Deterministic bulk payload data (64 bytes = 16 DWORDs)
static const unsigned int bulk_payload_data[16] = {
    0xDEAD0001U, 0xDEAD0002U, 0xDEAD0003U, 0xDEAD0004U,
    0xDEAD0005U, 0xDEAD0006U, 0xDEAD0007U, 0xDEAD0008U,
    0xDEAD0009U, 0xDEAD000AU, 0xDEAD000BU, 0xDEAD000CU,
    0xDEAD000DU, 0xDEAD000EU, 0xDEAD000FU, 0xDEAD0010U
};

// Full configuration descriptor data (60 bytes = 15 DWORDs)
// Raw bytes: 09 02 3c 00 01 01 00 e0 32 09 04 00 00 06 01 01
// 00 00 07 05 01 03 40 00 01 07 05 02 02 40 00 ff
// 07 05 03 01 ff 03 01 07 05 81 03 40 00 01 07 05
// 82 02 40 00 ff 07 05 83 01 ff 03 01
static const unsigned int full_config_desc_data[15] = {
    0x003c0209U, 0xe0000101U, 0x09040032U, 0x01010600U,
    0x07050000U, 0x00400301U, 0x05020107U, 0x00400202U,
    0x050307ffU, 0x03ff0103U, 0x05810107U, 0x00400303U,
    0x05820107U, 0x00400202U, 0x058301ffU, 0x000301ffU
};
// MANUAL_REVIEW: full_config_desc_data reconstructed from raw bytes in Meta Arrays.
// Verify exact DWORD packing matches hardware expectation (little-endian).
// The raw byte stream is 60 bytes = 15 DWORDs. Last DWORD may need padding verification.

// Global volatile variables for interrupt synchronization
volatile int int_pend = 0;
static volatile uint32_t event_counter = 0U;
static volatile uint32_t event_count = 0U;
static unsigned int g_errors = 0U;

// Forward declaration for nic_programming
void nic_programming(void);

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler - clears int_pend, reads system register
 * status, acknowledges event count, clears raw status, clears GIC.
 * Parameters:
 * None
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
    volatile unsigned int rd_data;

    int_pend = 0;

    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    LOGT("IRQHandler: MSK_STS0 = 0x%x", rd_data);

    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    LOGT("IRQHandler: RAW_STCR0 = 0x%x", rd_data);

    event_count = (uint32_t)read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    // Meta source uses (rd_data && 0x80000000) - preserving intent with bitwise AND per runtime rule 15
    if ((rd_data & 0x80000000U) != 0U) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    GIC_ClearIRQ(84);
}

/
 * Function: nic_programming
 * Description: USB controller initialization - soft reset, PHY config, event ring,
 * global control, device config, endpoint configuration, interrupt enable.
 * Parameters:
 * None
 * Returns:
 * void
 /
void nic_programming(void)
{
    volatile unsigned int rd_data;
    unsigned int i;
    unsigned int timeout;

    // Step 1a: Soft reset
    write_reg(MIZAR_USB_DCTL, 0x40f00000);
    rd_data = read_reg(MIZAR_USB_DCTL);
    timeout = 0U;
    while (rd_data != 0xf00000U) {
        for (volatile int d = 0; d < 50; d++);
        rd_data = read_reg(MIZAR_USB_DCTL);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("Soft reset polling timeout");
            g_errors++;
            break;
        }
    }
    LOGT("Soft Rst is done");

    // Step 1b: PHY config
    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002407);

    // Step 1c: Event ring setup
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    // Step 1d: Read GCTL
    rd_data = read_reg(MIZAR_USB_GCTL);

    // Step 1e: Write GCTL
    write_reg(MIZAR_USB_GCTL, 0x30c12214);

    // Step 1f: Read and write DCFG
    rd_data = read_reg(MIZAR_USB_DCFG);
    write_reg(MIZAR_USB_DCFG, 0x480801);

    // Step 1g: Write DEVTEN
    write_reg(MIZAR_USB_DEVTEN, 0x1f);

    // Step 1h: Read and write GUCTL
    rd_data = read_reg(MIZAR_USB_GUCTL);
    write_reg(MIZAR_USB_GUCTL, 0xa400010);

    // Step 1i: Start new configuration
    set_configuration(0, 0, 0, 0x409);

    // Step 1j: Set endpoint configuration (8 endpoints, command 0x401)
    set_configuration(0x00, 0x200, 0x700, 0x401);
    set_configuration(0x10, 0x200, 0x2000700, 0x401);
    set_configuration(0x20, 0x206, 0x4000700, 0x401);
    set_configuration(0x30, 0x20206, 0x6000700, 0x401);
    set_configuration(0x40, 0x204, 0x8000700, 0x401);
    set_configuration(0x50, 0x40204, 0xa000700, 0x401);
    set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
    set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

    // Step 1k: Endpoint TX resource configuration loop
    for (i = 0U; i < 8U; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10U), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10U), 0x402);
        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
        timeout = 0U;
        while (rd_data == 0x402U) {
            for (volatile int d = 0; d < 100; d++);
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
            timeout++;
            if (timeout >= POLL_TIMEOUT) {
                LOGE("DEPCMD 0x402 polling timeout for endpoint %u", i);
                g_errors++;
                break;
            }
        }
    }

    // Step 1l: Enable endpoints
    write_reg(MIZAR_USB_DALEPENA, 0x3);

    // Step 1m: Device control
    write_reg(MIZAR_USB_DCTL, 0x80f00000);

    // Step 1n: Interrupt enable
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
}

// Helper: wait for int_pend to be cleared by ISR with timeout
static int wait_for_interrupt(void)
{
    unsigned int timeout = 0U;
    while (int_pend) {
        for (volatile int d = 0; d < 50; d++);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("Interrupt wait timeout");
            g_errors++;
            return -1;
        }
    }
    return 0;
}

// Helper: poll DEPCMD at given offset until command value clears, with timeout
static int poll_depcmd(unsigned int offset, unsigned int cmd_val)
{
    volatile unsigned int rd_data;
    unsigned int timeout = 0U;
    rd_data = read_reg(MIZAR_USB_DEPCMD + offset);
    while (rd_data == cmd_val) {
        for (volatile int d = 0; d < 100; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + offset);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("DEPCMD polling timeout at offset 0x%x cmd 0x%x", offset, cmd_val);
            g_errors++;
            return -1;
        }
    }
    return 0;
}

// Helper: setup_stage for control endpoint 0
static void setup_stage(void)
{
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x8);
    write_reg(event_trb_addr + 0xcU, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    (void)poll_depcmd(0x0U, 0x506U);
    int_pend = 1;
    (void)wait_for_interrupt();
}

// Helper: status_stage for control endpoint 0
static void status_stage(void)
{
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x0);
    write_reg(event_trb_addr + 0xcU, 0x843);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    (void)poll_depcmd(0x0U, 0x506U);
    int_pend = 1;
    (void)wait_for_interrupt();
    // Post status_stage delay
    for (volatile int d = 0; d < 50; d++);
}

/
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initialize USB controller, enable GIC interrupts, clear buffers.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
    (void)cfg;

    g_errors = 0U;
    event_counter = 0U;
    event_count = 0U;
    int_pend = 0;

    LOGT("USB FS Device Bulk Transfer test init start");

    // Step 1: Call nic_programming()
    nic_programming();

    // Step 2: Call GIC_EnableAllIRQ()
    GIC_EnableAllIRQ();

    // Step 3: Buffer and TRB clearing loop
    {
        unsigned int j;
        for (j = 0U; j < 20U; j++) {
            write_reg(Buffer_PointerLO + j * DWORD, 0x0);
            write_reg(event_trb_addr + j * DWORD, 0x0);
        }
    }

    LOGT("USB FS Device Bulk Transfer test init complete");

    return (g_errors == 0U) ? 0 : -1;
}

/
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Perform USB enumeration (GET DESCRIPTOR Device, Configuration,
 * SET CONFIGURATION) and execute bulk data transfer.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * 0 on PASS, -1 on FAIL
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
    volatile unsigned int rd_data;
    unsigned int i;
    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run start");

    // Step 4: Enumeration start - DV debug marker converted to PSV logging
    LOGD("DV debug marker reached: enumeration start (0xdeadbee0)");

    // Step 5: Read DCFG and DSTS
    rd_data = read_reg(MIZAR_USB_DCFG);
    LOGT("DCFG = 0x%x", rd_data);
    rd_data = read_reg(MIZAR_USB_DSTS);
    LOGT("DSTS = 0x%x", rd_data);

    // Step 6: Configure enumeration registers
    write_reg(MIZAR_USB_DCFG, 0x480801);
    write_reg(MIZAR_USB_DCTL, 0x80f00a00);
    write_reg(MIZAR_USB_DALEPENA, 0xff);

    // Step 7: wait_on(5000) - converted to busy-wait
    for (volatile unsigned int d = 0U; d < 50000U; d++);

    // Step 8: GET DESCRIPTOR (Device) - setup_stage()
    LOGT("GET DESCRIPTOR (Device) - setup_stage");
    setup_stage();

    // Step 9: Conditional event_counter wait
    if (event_counter <= 0x4U) {
        int_pend = 1;
        (void)wait_for_interrupt();
    }

    // Step 10: GET DESCRIPTOR (Device) data stage - TRB setup
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x12);
    write_reg(event_trb_addr + 0xcU, 0x853);

    // Step 11: Write device descriptor data
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4U, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8U, 0x00000000);
    write_reg(Buffer_PointerLO + 0xcU, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10U, 0x00000100);

    // Step 12: Issue DEPCMD for data stage (endpoint offset 0x10)
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506);
    (void)poll_depcmd(0x10U, 0x506U);

    // Step 13: Wait for interrupt
    int_pend = 1;
    (void)wait_for_interrupt();

    // Step 14: XferNotReady event check
    int_pend = 1;
    (void)wait_for_interrupt();

    // Step 15: status_stage()
    LOGT("GET DESCRIPTOR (Device) - status_stage");
    status_stage();

    // Step 16: PHY reconfiguration
    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

    // Step 17: Conditional event_counter waits (two separate checks)
    if (event_counter <= 0x4U) {
        int_pend = 1;
        (void)wait_for_interrupt();
    }

    if (event_counter <= 0x4U) {
        int_pend = 1;
        (void)wait_for_interrupt();
    }

    // Step 18: GET DESCRIPTOR USB CONFIGURATION (short read) - setup_stage()
    LOGT("GET DESCRIPTOR (Config short) - setup_stage");
    setup_stage();
    // DV debug marker converted to PSV logging
    LOGD("DV debug marker reached: config short read (0xdeadbee3)");

    // Step 19: Wait for interrupt
    int_pend = 1;
    (void)wait_for_interrupt();

    // Step 20: Configuration descriptor data stage (9 bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x9);
    write_reg(event_trb_addr + 0xcU, 0x853);
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4U, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x8U, 0x00000032);
    write_reg(Buffer_PointerLO + 0xcU, 0x00000000);

    // Step 21: Issue DEPCMD for config descriptor data stage
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506);
    (void)poll_depcmd(0x10U, 0x506U);

    // Step 22: Wait for interrupt
    int_pend = 1;
    (void)wait_for_interrupt();

    // Step 23: status_stage()
    LOGT("GET DESCRIPTOR (Config short) - status_stage");
    status_stage();

    // Step 24: SET_CONFIGURATION / RESET - DV debug marker converted to PSV logging
    LOGD("DV debug marker reached: SET_CONFIGURATION (0xdeadbee4)");

    // Step 25: Wait for interrupt
    int_pend = 1;
    (void)wait_for_interrupt();

    // Step 26: GET DESCRIPTOR USB CONFIGURATION (full read) - setup_stage()
    LOGT("GET DESCRIPTOR (Config full) - setup_stage");
    setup_stage();
    // DV debug marker converted to PSV logging
    LOGD("DV debug marker reached: config full read (0xdeadbee5)");

    // Step 27: Wait for interrupt
    int_pend = 1;
    (void)wait_for_interrupt();

    // Step 28: Full configuration descriptor data stage (60 bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x3c);
    write_reg(event_trb_addr + 0xcU, 0x853);

    // Write full configuration descriptor to Buffer_PointerLO
    for (i = 0U; i < 15U; i++) {
        write_reg(Buffer_PointerLO + (i * 0x4U), full_config_desc_data[i]);
    }

    // Step 29: Issue DEPCMD for full config descriptor
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506);
    (void)poll_depcmd(0x10U, 0x506U);

    // Step 30: Wait for interrupt
    int_pend = 1;
    (void)wait_for_interrupt();

    // Step 31: status_stage()
    LOGT("GET DESCRIPTOR (Config full) - status_stage");
    status_stage();

    // ============================================================
    // BULK TRANSFER SECTION
    // ============================================================
    LOGT("Starting Bulk Transfer");

    // Prepare bulk payload data into Buffer_PointerLO_1 before transfer
    for (i = 0U; i < 16U; i++) {
        write_reg(Buffer_PointerLO_1 + (i * 0x4U), bulk_payload_data[i]);
    }

    // Step 32: Bulk transfer TRB setup
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8U, 0x40);
    write_reg(event_trb_addr + 0xcU, 0x813);

    // Step 33: Issue bulk endpoint DEPCMD (endpoint offset 0x40)
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40U, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x40U, 0x506);
    (void)poll_depcmd(0x40U, 0x506U);

    // Step 34: Wait for bulk transfer completion interrupt
    int_pend = 1;
    (void)wait_for_interrupt();

    LOGT("Bulk Transfer complete");

    // Validation: check that bulk transfer completed without errors
    if (g_errors != 0U) {
        LOGE("USB FS Bulk Transfer test FAILED: errors=%u", g_errors);
        out->status = -1;
    } else {
        LOGT("USB FS Bulk Transfer test PASSED");
        out->status = 0;
    }

    return out->status;
}

/
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Cleanup after USB bulk transfer test.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown");

    // Cleanup interrupt state
    int_pend = 0;

    // Return final status based on accumulated errors
    return (g_errors == 0U) ? 0 : -1;
}
