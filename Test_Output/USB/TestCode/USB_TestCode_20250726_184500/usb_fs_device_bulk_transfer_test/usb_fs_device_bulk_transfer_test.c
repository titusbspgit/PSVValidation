/*
 * File: usb_fs_device_bulk_transfer_test.c
 * Description: PSV testcase - USB FS Device Bulk Transfer Test
 * Performs CDC bulk data transfer.
 ****************************************************************************/

// MANUAL_REVIEW: Two set_configuration() calls (offsets 0x00 and 0x40) have
// redacted parameters (<PERSON> tags in Meta). Their parameter0, parameter1
// values are unknown. Additionally, Buffer_PointerLO+0x4 configuration
// descriptor value is redacted (0<MEDICAL_LICENSE>). A variable name in
// Default_IRQHandler is redacted (<LOCATION> tag). These prevent complete
// code generation.
#error "MANUAL_REVIEW: Redacted source values - set_configuration offsets 0x00 and 0x40 parameters unknown, Buffer_PointerLO+0x4 config descriptor value unknown (0<MEDICAL_LICENSE>), Default_IRQHandler redacted variable name"

#include "usb_fs_device_bulk_transfer_test.h"

#include <stdio.h>
#include <stdint.h>
#include "usb.h"
#include "reg_access.h"
#include "gic_funcs.h"
#include "log.h"

// Prototype for nic_programming (per project rules)
void nic_programming(void);

volatile int int_pend;
int event_counter;

// Forward declarations of testcase-owned helpers
static void setup_stage(void);
static void status_stage(void);
static void enumeration(void);
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd);

void Default_IRQHandler(void)
{
    int rd_data;
    // MANUAL_REVIEW: Original source declares a second variable here whose name
    // is redacted (<LOCATION> tag in Meta). Declare it when the name is known.
    int event_count;

    int_pend = 0;
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
    // Note: Source uses logical AND (&&) instead of bitwise AND (&).
    // Preserved as-is per source preservation rules.
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }
    GIC_ClearIRQ(84);

    (void)event_count;
}

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
        // wait_on(10)
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }
    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }
}

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
        // wait_on(10)
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }
    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }
    // wait_on(5)
    for (volatile int d = 0; d < 10000; d++);
}

static void enumeration(void)
{
    int rd_data;

    // First setup_stage
    setup_stage();

    // Debug marker write to 0xA0243ffc
    write_reg(0xA0243ffc, 0xdeadbee1);

    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }

    // Second setup_stage
    setup_stage();

    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            // wait_on(5)
            for (volatile int d = 0; d < 10000; d++);
        }
    }

    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            // wait_on(5)
            for (volatile int d = 0; d < 10000; d++);
        }
    }

    rd_data = read_reg(MIZAR_USB_DCFG);
    write_reg(MIZAR_USB_DCFG, 0x480801);
    rd_data = read_reg(MIZAR_USB_DSTS);
    write_reg(MIZAR_USB_DCTL, 0x80f00a00);
    write_reg(MIZAR_USB_DALEPENA, 0xff);

    // wait_on(5000)
    for (volatile int d = 0; d < 10000; d++);

    // GET DESCRIPTOR - Device Descriptor: setup_stage
    setup_stage();

    // Debug marker write to 0xA0243ffc
    write_reg(0xA0243ffc, 0xdeadbee3);

    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }

    // Device descriptor TRB setup
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Device descriptor data
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100);

    // Issue Start Transfer on endpoint offset 0x10
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        // wait_on(10)
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }

    status_stage();

    // GET DESCRIPTOR - Configuration Descriptor: setup_stage
    setup_stage();

    // Debug marker write to 0xA0243ffc
    write_reg(0xA0243ffc, 0xdeadbee5);

    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }

    // Configuration descriptor TRB setup
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Configuration descriptor data
    write_reg(Buffer_PointerLO, 0x003c0209);
    // MANUAL_REVIEW: Original value at Buffer_PointerLO+0x4 is redacted
    // (shown as 0<MEDICAL_LICENSE> in Meta). Actual value is unknown.
    write_reg(Buffer_PointerLO + 0x4, 0 /* MANUAL_REVIEW: redacted value 0<MEDICAL_LICENSE> */);
    write_reg(Buffer_PointerLO + 0x8, 0x00000032);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x1c, 0xff004002);
    write_reg(Buffer_PointerLO + 0x20, 0x01030507);
    write_reg(Buffer_PointerLO + 0x24, 0x070103ff);
    write_reg(Buffer_PointerLO + 0x28, 0x40038105);
    write_reg(Buffer_PointerLO + 0x2c, 0x05070100);
    write_reg(Buffer_PointerLO + 0x30, 0x00400282);
    write_reg(Buffer_PointerLO + 0x34, 0x830507ff);
    write_reg(Buffer_PointerLO + 0x38, 0x0103ff01);

    // Issue Start Transfer on endpoint offset 0x10
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        // wait_on(10)
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }

    status_stage();

    (void)rd_data;
}

static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
    int read_data;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
    read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    while (read_data == cmd) {
        // wait_on(30)
        for (volatile int d = 0; d < 10000; d++);
        read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    }
}

int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("USB FS Device Bulk Transfer test init");
    return 0;
}

int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    volatile unsigned int i;
    volatile unsigned int j;

    (void)cfg;

    if (out == 0) {
        LOGE("USB output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run start");

    // Step 1: Call nic_programming() and GIC_EnableAllIRQ()
    nic_programming();
    GIC_EnableAllIRQ();

    // Step 2: Clear 20 DWORD entries in Buffer_PointerLO and event_trb_addr
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }

    // Step 3: Configure event ring registers
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    // Step 4: Read GCTL
    rd_data = read_reg(MIZAR_USB_GCTL);

    // Step 5: Configure endpoint TX resources for 8 endpoints
    for (i = 0; i < 8; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        while (rd_data == 0x402) {
            // wait_on(10)
            for (volatile int d = 0; d < 10000; d++);
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        }
    }

    // Step 6: Write DALEPENA
    write_reg(MIZAR_USB_DALEPENA, 0x3);

    // Step 7: Write DCTL
    write_reg(MIZAR_USB_DCTL, 0x80f00000);

    // Step 8: Enable interrupt
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);

    // Step 9: Call enumeration()
    enumeration();

    // Step 10: Call set_configuration() 8 times
    // MANUAL_REVIEW: set_configuration offset 0x00 - parameters redacted
    // (<PERSON> tag in Meta step 15). parameter0, parameter1 values unknown.
    // Placeholder call with 0 values; replace with actual values.
    set_configuration(0x00, 0 /* MANUAL_REVIEW: redacted parameter0 */, 0 /* MANUAL_REVIEW: redacted parameter1 */, 0x401);

    set_configuration(0x10, 0x200, 0x2000700, 0x401);
    set_configuration(0x20, 0x206, 0x4000700, 0x401);
    set_configuration(0x30, 0x20206, 0x6000700, 0x401);

    // MANUAL_REVIEW: set_configuration offset 0x40 - parameters redacted
    // (<PERSON> tag in Meta step 19). parameter0, parameter1 values unknown.
    // Placeholder call with 0 values; replace with actual values.
    set_configuration(0x40, 0 /* MANUAL_REVIEW: redacted parameter0 */, 0 /* MANUAL_REVIEW: redacted parameter1 */, 0x401);

    set_configuration(0x50, 0x40204, 0xa000700, 0x401);
    set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
    set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

    // Step 11: Set up bulk TRB with Buffer_PointerLO_1
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);

    // Step 12: Issue Start Transfer on DEPCMD+0x40
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    while (rd_data == 0x506) {
        // wait_on(10)
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    }

    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }

    // Step 13: Issue Start Transfer on DEPCMD+0x50
    write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    while (rd_data == 0x506) {
        // wait_on(10)
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    }

    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }

    // Step 14: Debug marker write to 0xA0243ffc
    write_reg(0xA0243ffc, 0xdeadbee7);

    // Step 15: Wait for interrupt
    int_pend = 1;
    while (int_pend) {
        // wait_on(5)
        for (volatile int d = 0; d < 10000; d++);
    }

    // finish(0) replaced with PSV completion
    LOGT("USB FS Device Bulk Transfer test complete: PASS");
    out->status = 0;

    (void)rd_data;

    return out->status;
}

int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("USB FS Device Bulk Transfer test teardown");
    return 0;
}
