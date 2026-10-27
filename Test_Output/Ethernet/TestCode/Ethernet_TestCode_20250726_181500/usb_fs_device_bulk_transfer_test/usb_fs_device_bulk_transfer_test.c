// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Global variables from Meta
extern int int_pend;
int event_counter;

// PSV wait_on adaptation: DV wait_on(N) is converted to a deterministic busy-wait loop.
// Scaling: wait_on(N) -> for(volatile int d=0; d<N*1000; d++); to approximate relative delay.
// MANUAL_REVIEW: Adjust the scaling factor (1000) if needed for target platform timing.

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler. Clears int_pend, reads sysreg status,
 *              reads event count from GEVNTCOUNT, acknowledges events,
 *              conditionally clears RAW_STCR0, and clears GIC IRQ 84.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rd_data, sysreg_rd_data, event_count;
    (void)sysreg_rd_data;
    int_pend = 0;
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    // MANUAL_REVIEW: Source uses (rd_data && 0x80000000) which is logical-AND, not bitwise-AND. Preserved as-is.
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }
    GIC_ClearIRQ(84);
}

/*
 * Function: set_configuration
 * Description: Writes endpoint command parameters and issues a command,
 *              then polls DEPCMD for command completion.
 * Parameters:
 *   trb_address - endpoint register offset
 *   parameter0  - DEPCMDPAR0 value
 *   parameter1  - DEPCMDPAR1 value
 *   cmd         - DEPCMD command value
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
 * Function: setup_stage
 * Description: Prepares a setup-stage TRB, issues Start Transfer command (0x506),
 *              polls DEPCMD for completion, and waits for interrupt.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void setup_stage(void)
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
 * Description: Prepares a status-stage TRB, issues Start Transfer command (0x506),
 *              polls DEPCMD for completion, and waits for two interrupts with
 *              an intermediate delay.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void status_stage(void)
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

    for (volatile int d = 0; d < 5000; d++);

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
}

/*
 * Function: enumeration
 * Description: Performs USB enumeration: GET DESCRIPTOR (device descriptor),
 *              GET DESCRIPTOR (short configuration descriptor) with setup,
 *              data, and status stages.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void enumeration(void)
{
    int rd_data;

    setup_stage();
    write_reg(0xA0243ffc, 0xdeadbee1);
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

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

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    for (volatile int d = 0; d < 5000; d++);

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    // status
    // ADDED to check the xfernotready event
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    status_stage();

    // GET DESCRIPTOR USB CONFIGURATION
    setup_stage();
    // data
    write_reg(0xA0243ffc, 0xdeadbee3);
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    // 09 02 3c 00 01 01 00 e0 32
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x9);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x8, 0x00000032);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    for (volatile int d = 0; d < 5000; d++);

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    status_stage();
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes the USB FS Device Bulk Transfer test. Performs NIC
 *              programming, GIC enable, buffer/TRB clearing, soft reset, PHY
 *              config, event buffer setup, global controller config, endpoint
 *              configuration, TX resource allocation, endpoint enable, run state,
 *              and interrupt enable.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;
    int rd_data, wr_data;
    int i, j;

    LOGT("USB FS Device Bulk Transfer test init started");

    // Step 1: NIC programming and GIC enable
    nic_programming();
    GIC_EnableAllIRQ();

    // Step 2: Clear buffer and event TRB memory regions
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }

    // Step 3: USB controller soft reset
    wr_data = 0x40f00000;
    write_reg(MIZAR_USB_DCTL, wr_data);
    rd_data = read_reg(MIZAR_USB_DCTL);
    while (rd_data != 0xf00000) {
        for (volatile int d = 0; d < 5000; d++);
        rd_data = read_reg(MIZAR_USB_DCTL);
    }
    LOGT("Soft Rst is done");

    // Step 4: Configure USB 2.0 PHY
    wr_data = 0x40002407;
    write_reg(MIZAR_USB_GUSB2PHYCFG, wr_data);

    // Step 5: Set up event buffer
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    // Step 6: Read GCTL, then write port direction
    rd_data = read_reg(MIZAR_USB_GCTL);
    wr_data = 0x30c12214;
    write_reg(MIZAR_USB_GCTL, wr_data);

    // Step 7: Read and program DCFG for Full-Speed
    rd_data = read_reg(MIZAR_USB_DCFG);
    write_reg(MIZAR_USB_DCFG, 0x480801);

    // Step 8: Enable device events via DEVTEN
    write_reg(MIZAR_USB_DEVTEN, 0x1f);

    // Step 9: Read and program GUCTL
    rd_data = read_reg(MIZAR_USB_GUCTL);
    write_reg(MIZAR_USB_GUCTL, 0xa400010);

    // Step 10: START NEW CONFIGURATION
    set_configuration(0, 0, 0, 0x409);

    // Step 11: Configure 8 endpoints
    set_configuration(0, 0x200, 0x700, 0x401);
    set_configuration(0x10, 0x200, 0x2000700, 0x401);
    set_configuration(0x20, 0x206, 0x4000700, 0x401);
    set_configuration(0x30, 0x20206, 0x6000700, 0x401);
    set_configuration(0x40, 0x204, 0x8000700, 0x401);
    set_configuration(0x50, 0x40204, 0xa000700, 0x401);
    set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
    set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

    // Step 12: Endpoint TX resource configuration for 8 endpoints
    for (i = 0; i < 8; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        while (rd_data == 0x402) {
            for (volatile int d = 0; d < 10000; d++);
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        }
    }

    // Step 13: Enable physical endpoints 0 and 1
    write_reg(MIZAR_USB_DALEPENA, 0x3);

    // Step 14: Set controller to run state
    write_reg(MIZAR_USB_DCTL, 0x80f00000);

    // Step 15: Enable system-level USB interrupt
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);

    LOGT("USB FS Device Bulk Transfer test init completed");
    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Executes the main USB FS Device Bulk Transfer test flow including
 *              link state/connect/reset event handling, USB enumeration (SET ADDRESS,
 *              GET DESCRIPTOR, SET CONFIGURATION), handshake synchronization, and
 *              bulk transfers on two endpoints.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput result structure
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    int rd_data;
    int hand_shake;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run started");

    // Step 16: Link state connect reset events - wait for interrupt
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            for (volatile int d = 0; d < 5000; d++);
        }
    }

    // Step 17: Re-program DCFG after reset events
    write_reg(MIZAR_USB_DCFG, 0x480801);

    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            for (volatile int d = 0; d < 5000; d++);
        }
    }

    // Step 18: Setup stage for SET ADDRESS
    setup_stage();

    // Functional debug/handshake marker write (not 0xdeadbee0, preserved as-is)
    write_reg(0xA0243ffc, 0xdeadbee3);

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    // Step 19-20: SET ADDRESS data TRB and Start Transfer on IN endpoint
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x9);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x8, 0x00000032);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    for (volatile int d = 0; d < 5000; d++);

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    // status
    status_stage();

    // USB_SET_CONFIGURATION_OR_RESET_TT
    setup_stage();

    // status data stage
    // Functional debug/handshake marker write (not 0xdeadbee0, preserved as-is)
    write_reg(0xA0243ffc, 0xdeadbee4);

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

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

    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            for (volatile int d = 0; d < 5000; d++);
        }
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    // Step 21: Handshake synchronization (first)
    hand_shake = read_reg(0xa0243ff4);
    while (hand_shake == 0x0) {
        hand_shake = read_reg(0xa0243ff4);
    }

    // Step 22: Execute enumeration sequence (GET DESCRIPTOR device + short config)
    enumeration();

    // Step 23: Handshake synchronization (second)
    hand_shake = read_reg(0xa0243ff8);
    while (hand_shake == 0x0) {
        hand_shake = read_reg(0xa0243ff8);
    }

    // Functional debug/handshake marker write (not 0xdeadbee0, preserved as-is)
    write_reg(0xA0243ffc, 0xdeadbee6);
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    // Step 27: GET DESCRIPTOR full configuration descriptor
    setup_stage();

    // Functional debug/handshake marker write (not 0xdeadbee0, preserved as-is)
    write_reg(0xA0243ffc, 0xdeadbee5);
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    // data stage - full configuration descriptor with interface and endpoint descriptors
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x8, 0x00040932);
    write_reg(Buffer_PointerLO + 0xc, 0x01010600);
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
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    for (volatile int d = 0; d < 5000; d++);

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }
    status_stage();

    // Step 28: Bulk OUT transfer on first bulk endpoint (0x40)
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    // Step 29: Bulk OUT transfer on second bulk endpoint (0x50)
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    while (rd_data == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    }

    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    // Functional debug/handshake marker write (not 0xdeadbee0, preserved as-is)
    write_reg(0xA0243ffc, 0xdeadbee7);
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 5000; d++);
    }

    // Step 30: Test completion - finish(0) replaced with PSV status
    LOGT("USB FS Device Bulk Transfer test run completed: PASS");
    out->status = 0;

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Performs final cleanup for the USB FS Device Bulk Transfer test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown: no additional cleanup required");
    return 0;
}
