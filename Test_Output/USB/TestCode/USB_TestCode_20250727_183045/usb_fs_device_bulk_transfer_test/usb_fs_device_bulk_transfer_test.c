// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Global volatile variables for interrupt-driven synchronization
volatile int int_pend = 0;
static volatile unsigned int event_counter = 0;
static unsigned int g_errors = 0;

// Forward declaration for nic_programming
void nic_programming(void);

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for USB events. Clears int_pend,
 *              reads system register status, acknowledges GEVNTCOUNT,
 *              clears raw status, and clears GIC IRQ 84.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    volatile unsigned int rd_data;
    volatile unsigned int event_count;

    int_pend = 0;
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
    // MANUAL_REVIEW: Source uses logical AND (&&) instead of bitwise AND (&). Preserved as-is per No Silent Correction Rule.
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }
    GIC_ClearIRQ(84);
}

/*
 * Function: setup_stage_get_desc_device
 * Description: Executes GET DESCRIPTOR (Device) setup stage TRB and DEPCMD.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void setup_stage_get_desc_device(void)
{
    volatile unsigned int rd_data;

    // Step 8: GET DESCRIPTOR (Device) - setup_stage()
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }
}

/*
 * Function: setup_stage_get_desc_config_short
 * Description: Executes GET DESCRIPTOR (Configuration) short read setup stage.
 *              This is step 18 setup_stage() call.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void setup_stage_get_desc_config_short(void)
{
    volatile unsigned int rd_data;

    // Step 18: GET DESCRIPTOR USB CONFIGURATION (short read) - setup_stage()
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }
}

/*
 * Function: setup_stage_get_desc_config_full
 * Description: Executes GET DESCRIPTOR (Configuration) full read setup stage.
 *              This is step 26 setup_stage() call.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void setup_stage_get_desc_config_full(void)
{
    volatile unsigned int rd_data;

    // Step 26: GET DESCRIPTOR USB CONFIGURATION (full read) - setup_stage()
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }
}

/*
 * Function: status_stage
 * Description: Executes USB control transfer status stage.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void status_stage(void)
{
    volatile unsigned int rd_data;

    // Step 15/23/31: status_stage()
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x843);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }
    // Trailing wait_on(5) from step 15
    for (volatile int d = 0; d < 500; d++);
}

/*
 * Function: nic_programming
 * Description: Initializes the USB controller: soft reset, PHY config,
 *              event ring setup, global/device config, endpoint config,
 *              TX resource config, endpoint enable, device run, interrupt enable.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void nic_programming(void)
{
    volatile unsigned int rd_data;
    unsigned int i;

    // Step 1a: Soft reset
    write_reg(MIZAR_USB_DCTL, 0x40f00000);
    rd_data = read_reg(MIZAR_USB_DCTL);
    while (rd_data != 0xf00000) {
        for (volatile int d = 0; d < 500; d++);
        rd_data = read_reg(MIZAR_USB_DCTL);
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
    for (i = 0; i < 8; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        while (rd_data == 0x402) {
            for (volatile int d = 0; d < 1000; d++);
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        }
    }

    // Step 1l: Enable endpoints
    write_reg(MIZAR_USB_DALEPENA, 0x3);

    // Step 1m: Device control
    write_reg(MIZAR_USB_DCTL, 0x80f00000);

    // Step 1n: Interrupt enable
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes the USB FS Device Bulk Transfer test.
 *              Calls nic_programming and enables GIC interrupts.
 * Parameters:
 *   cfg - pointer to test configuration
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_errors = 0;

    LOGT("USB FS Device Bulk Transfer test init");

    // Step 1: Call nic_programming()
    nic_programming();

    // Step 2: Call GIC_EnableAllIRQ()
    GIC_EnableAllIRQ();

    LOGT("USB FS Device Bulk Transfer test init complete");

    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Executes the USB FS Device Bulk Transfer test.
 *              Clears buffers, performs USB enumeration (GET DESCRIPTOR
 *              Device, GET DESCRIPTOR Configuration short and full,
 *              SET CONFIGURATION), and executes CDC bulk data transfer.
 * Parameters:
 *   cfg - pointer to test configuration
 *   out - pointer to test output
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    volatile unsigned int rd_data;
    unsigned int j;

    (void)cfg;

    if (out == 0) {
        LOGE("USB output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting USB FS Device Bulk Transfer test run");

    // Step 3: Buffer and TRB clearing loop
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }

    // Step 4: Enumeration start - DV debug marker converted to PSV logging
    LOGD("DV debug marker reached");

    // Step 5: Read DCFG and DSTS
    rd_data = read_reg(MIZAR_USB_DCFG);
    rd_data = read_reg(MIZAR_USB_DSTS);

    // Step 6: Configure enumeration registers
    write_reg(MIZAR_USB_DCFG, 0x480801);
    write_reg(MIZAR_USB_DCTL, 0x80f00a00);
    write_reg(MIZAR_USB_DALEPENA, 0xff);

    // Step 7: wait_on(5000) - busy-wait adaptation
    for (volatile unsigned int d = 0; d < 500000; d++);

    // Step 8: GET DESCRIPTOR (Device) - setup_stage()
    setup_stage_get_desc_device();

    // Step 9: Conditional event_counter wait
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            for (volatile int d = 0; d < 500; d++);
        }
    }

    // Step 10: GET DESCRIPTOR (Device) data stage - TRB setup
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Step 11: Write device descriptor data
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100);

    // Step 12: Issue DEPCMD for data stage (endpoint offset 0x10)
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    // Step 13: int_pend wait
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }

    // Step 14: XferNotReady event check
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }

    // Step 15: status_stage()
    status_stage();

    // Step 16: PHY reconfiguration
    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

    // Step 17: Conditional event_counter waits (two separate checks)
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            for (volatile int d = 0; d < 500; d++);
        }
    }

    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            for (volatile int d = 0; d < 500; d++);
        }
    }

    // Step 18: GET DESCRIPTOR USB CONFIGURATION (short read) - setup_stage()
    setup_stage_get_desc_config_short();
    // DV debug marker 0xdeadbee3 converted to PSV logging
    LOGD("DV debug marker: GET DESCRIPTOR Config short read");

    // Step 19: int_pend wait
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }

    // Step 20: Configuration descriptor data stage (9 bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x9);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x8, 0x00000032);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);

    // Step 21: Issue DEPCMD for config descriptor data stage
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    // Step 22: int_pend wait
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }

    // Step 23: status_stage()
    status_stage();

    // Step 24: SET_CONFIGURATION / RESET - DV debug marker 0xdeadbee4 converted to PSV logging
    LOGD("DV debug marker: SET_CONFIGURATION / RESET");

    // Step 25: int_pend wait
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }

    // Step 26: GET DESCRIPTOR USB CONFIGURATION (full read) - setup_stage()
    setup_stage_get_desc_config_full();
    // DV debug marker 0xdeadbee5 converted to PSV logging
    LOGD("DV debug marker: GET DESCRIPTOR Config full read");

    // Step 27: int_pend wait
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }

    // Step 28: Full configuration descriptor data stage (60 bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);
    // Full configuration descriptor raw data (60 bytes / 15 DWORDs)
    write_reg(Buffer_PointerLO + 0x00, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x04, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x08, 0x00040932);
    write_reg(Buffer_PointerLO + 0x0c, 0x01010600);
    write_reg(Buffer_PointerLO + 0x10, 0x05070000);
    write_reg(Buffer_PointerLO + 0x14, 0x00400301);
    write_reg(Buffer_PointerLO + 0x18, 0x02050701);
    write_reg(Buffer_PointerLO + 0x1c, 0xff004002);
    write_reg(Buffer_PointerLO + 0x20, 0x01030507);
    write_reg(Buffer_PointerLO + 0x24, 0x070103ff);
    write_reg(Buffer_PointerLO + 0x28, 0x40038105);
    write_reg(Buffer_PointerLO + 0x2c, 0x05070100);
    write_reg(Buffer_PointerLO + 0x30, 0x00400282);
    write_reg(Buffer_PointerLO + 0x34, 0x830507ff);
    write_reg(Buffer_PointerLO + 0x38, 0x0103ff01);

    // Step 29: Issue DEPCMD for full config descriptor
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    // Step 30: int_pend wait
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }

    // Step 31: status_stage()
    status_stage();

    // Step 32: Bulk transfer setup (CDC bulk data transfer)
    LOGT("Starting CDC bulk data transfer");
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);

    // Step 33: Issue bulk endpoint DEPCMD (endpoint offset 0x40)
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 1000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    }

    // Step 34: int_pend wait for bulk transfer completion
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 500; d++);
    }

    LOGT("CDC bulk data transfer complete");

    out->status = (g_errors == 0U) ? 0 : -1;

    LOGT("USB FS Device Bulk Transfer test run complete: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB FS Device Bulk Transfer test.
 * Parameters:
 *   cfg - pointer to test configuration
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown: no additional cleanup required");
    return g_errors == 0U ? 0 : -1;
}
