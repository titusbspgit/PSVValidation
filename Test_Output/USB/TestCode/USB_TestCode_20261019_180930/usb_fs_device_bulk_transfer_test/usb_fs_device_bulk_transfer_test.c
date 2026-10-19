/*
 * usb_fs_device_bulk_transfer_test.c
 *
 * PSV testcase: USB FS Device Bulk Transfer Test
 * Auto-generated from DV-derived Meta TestPlan.
 /

#include "usb_fs_device_bulk_transfer_test.h"
#include "reg_access.h"
#include "gic_funcs.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "usb.h"

/*
 * MANUAL_REVIEW: Multiple blocking issues in this testcase:
 * 1. No delay-to-loop conversion formula or iteration count supplied for
 *    wait_on() replacements (wait_on(5), wait_on(10), wait_on(30), wait_on(5000)).
 * 2. No maximum polling iteration count supplied for int_pend polling
 *    (interrupt-based synchronization) or DEPCMD polling loops.
 /
#error "MANUAL_REVIEW: No delay-to-loop conversion formula or polling iteration limits supplied for wait_on() and polling adaptations"


/ nic_programming prototype per USB Extra Conditions /
void nic_programming(void);


/*
 * Global declarations from original source.
 *
 * Original source declared: extern int int_pend;
 * USB Extra Conditions require: volatile int int_pend;
 * Preserving original extern linkage while adding volatile per Extra Conditions.
 * MANUAL_REVIEW: If the owning translation unit does not declare int_pend as
 * volatile, the volatile qualifier here may be insufficient for correct
 * ISR/main-context synchronization. Ensure the definition site also uses volatile.
 /
extern volatile int int_pend;

/*
 * event_counter is written in Default_IRQHandler (ISR context) and
 * read in main context (enumeration). Declared volatile per USB
 * Extra Conditions (change variables to volatile according to usage).
 /
volatile int event_counter;


/ Forward declarations of testcase-owned helpers /
static void setup_stage(void);
static void status_stage(void);
static void enumeration(void);
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd);


/*
 * Default_IRQHandler
 *
 * Preserves original source behavior including the && operator
 * (Remarks: original uses logical AND instead of bitwise AND).
 /
void Default_IRQHandler(void)
{
    volatile int rd_data;
    volatile int event_count;

    int_pend = 0;

    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    / Original source uses && (logical AND), preserved as-is per Section 5 /
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    GIC_ClearIRQ(84);
}


/*
 * setup_stage
 /
static void setup_stage(void)
{
    volatile int rd_data;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);

    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);

    rd_data = read_reg(MIZAR_USB_DEPCMD);
    / Original: while(rd_data==0x506){wait_on(10);rd_data=read_reg(MIZAR_USB_DEPCMD);} /
    // MANUAL_REVIEW: DEPCMD polling -- needs max iteration count and wait_on(10) conversion
    while (rd_data == 0x506) {
        / wait_on(10) -- MANUAL_REVIEW: no conversion formula supplied /
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }

    / Original: int_pend=1; while(int_pend){wait_on(5);} /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }
}


/*
 * status_stage
 /
static void status_stage(void)
{
    volatile int rd_data;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x843);

    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);

    rd_data = read_reg(MIZAR_USB_DEPCMD);
    / Original: while(rd_data==0x506){wait_on(10);rd_data=read_reg(MIZAR_USB_DEPCMD);} /
    // MANUAL_REVIEW: DEPCMD polling -- needs max iteration count and wait_on(10) conversion
    while (rd_data == 0x506) {
        / wait_on(10) -- MANUAL_REVIEW: no conversion formula supplied /
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }

    / Original: int_pend=1; while(int_pend){wait_on(5);} /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    / Original: wait_on(5) at end of status_stage /
    // MANUAL_REVIEW: elapsed delay wait_on(5) -- no conversion formula supplied
}


/*
 * set_configuration
 /
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
    volatile int read_data;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);

    read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    / Original: while(read_data==cmd){wait_on(30);read_data=read_reg(MIZAR_USB_DEPCMD+trb_address);} /
    // MANUAL_REVIEW: DEPCMD polling -- needs max iteration count and wait_on(30) conversion
    while (read_data == cmd) {
        / wait_on(30) -- MANUAL_REVIEW: no conversion formula supplied /
        read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    }
}


/*
 * enumeration
 /
static void enumeration(void)
{
    volatile int rd_data;

    / First setup_stage + debug marker + interrupt wait /
    setup_stage();

    / Debug marker write from original source /
    write_reg(0xA0243ffc, 0xdeadbee1);

    / Original: int_pend=1; while(int_pend){wait_on(5);} /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    / Second setup_stage /
    setup_stage();

    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

    / Original: if(event_counter<=0x4){int_pend=1;while(int_pend){wait_on(5);}} /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
        }
    }

    / Original: if(event_counter<=0x4){int_pend=1;while(int_pend){wait_on(5);}} (second) /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
        }
    }

    rd_data = read_reg(MIZAR_USB_DCFG);
    (void)rd_data;
    write_reg(MIZAR_USB_DCFG, 0x480801);

    rd_data = read_reg(MIZAR_USB_DSTS);
    (void)rd_data;

    write_reg(MIZAR_USB_DCTL, 0x80f00a00);
    write_reg(MIZAR_USB_DALEPENA, 0xff);

    / Original: wait_on(5000) -- elapsed delay /
    // MANUAL_REVIEW: elapsed delay wait_on(5000) -- no conversion formula supplied

    / GET DESCRIPTOR: Device Descriptor -- setup_stage /
    setup_stage();

    / Debug marker write from original source /
    write_reg(0xA0243ffc, 0xdeadbee3);

    / Original: int_pend=1; while(int_pend){wait_on(5);} /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    / Data stage: device descriptor TRB /
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);

    / Device descriptor data /
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100);

    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    / Original: while(rd_data==0x506){wait_on(10);rd_data=read_reg(MIZAR_USB_DEPCMD+0x10);} /
    // MANUAL_REVIEW: DEPCMD polling -- needs max iteration count and wait_on(10) conversion
    while (rd_data == 0x506) {
        / wait_on(10) -- MANUAL_REVIEW: no conversion formula supplied /
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    / Original: int_pend=1; while(int_pend){wait_on(5);} /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    status_stage();

    / GET DESCRIPTOR: Configuration Descriptor -- setup_stage /
    setup_stage();

    / Debug marker write from original source /
    write_reg(0xA0243ffc, 0xdeadbee5);

    / Original: int_pend=1; while(int_pend){wait_on(5);} /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    / Data stage: configuration descriptor TRB /
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);

    / Configuration descriptor data /
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
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

    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    / Original: while(rd_data==0x506){wait_on(10);rd_data=read_reg(MIZAR_USB_DEPCMD+0x10);} /
    // MANUAL_REVIEW: DEPCMD polling -- needs max iteration count and wait_on(10) conversion
    while (rd_data == 0x506) {
        / wait_on(10) -- MANUAL_REVIEW: no conversion formula supplied /
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    / Original: int_pend=1; while(int_pend){wait_on(5);} /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    status_stage();
}


/*
 * usb_fs_device_bulk_transfer_test_init
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test init");

    return 0;
}


/*
 * usb_fs_device_bulk_transfer_test_run
 *
 * CDC bulk data transfer testcase.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem cfg, TestOutput out)
{
    volatile int rd_data;
    unsigned int i;
    unsigned int j;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run start");

    / Step 2: nic_programming() /
    nic_programming();

    / Step 3: GIC_EnableAllIRQ() /
    GIC_EnableAllIRQ();

    / Step 4: Clear 20 DWORD entries in Buffer_PointerLO and event_trb_addr /
    for (j = 0U; j < 20U; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }

    / Step 5-8: Configure event ring /
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    / Step 9: Read GCTL /
    rd_data = read_reg(MIZAR_USB_GCTL);
    (void)rd_data;

    / Step 10: Configure endpoint TX resources for 8 endpoints /
    for (i = 0U; i < 8U; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10U), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10U), 0x402);

        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
        / Original: while(rd_data==0x402){wait_on(10);rd_data=read_reg(MIZAR_USB_DEPCMD+(i0x10));} /
        // MANUAL_REVIEW: DEPCMD polling -- needs max iteration count and wait_on(10) conversion
        while (rd_data == 0x402) {
            / wait_on(10) -- MANUAL_REVIEW: no conversion formula supplied /
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
        }
    }

    / Step 11: Write DALEPENA /
    write_reg(MIZAR_USB_DALEPENA, 0x3);

    / Step 12: Write DCTL /
    write_reg(MIZAR_USB_DCTL, 0x80f00000);

    / Step 13: Write LSS interrupt enable /
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);

    / Step 14: Enumeration /
    enumeration();

    / Steps 15-22: set_configuration() x8 /
    set_configuration(0, 0x200, 0x700, 0x401);
    set_configuration(0x10, 0x200, 0x2000700, 0x401);
    set_configuration(0x20, 0x206, 0x4000700, 0x401);
    set_configuration(0x30, 0x20206, 0x6000700, 0x401);
    set_configuration(0x40, 0x204, 0x8000700, 0x401);
    set_configuration(0x50, 0x40204, 0xa000700, 0x401);
    set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
    set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

    / Step 23: Set up bulk TRB for CDC bulk data transfer /
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);

    / Step 24: Issue Start Transfer on endpoint offset 0x40 /
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

    / Step 25: DEPCMD+0x40 with 0x506, poll /
    write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    / Original: while(rd_data==0x506){wait_on(10);rd_data=read_reg(MIZAR_USB_DEPCMD+0x40);} /
    // MANUAL_REVIEW: DEPCMD polling -- needs max iteration count and wait_on(10) conversion
    while (rd_data == 0x506) {
        / wait_on(10) -- MANUAL_REVIEW: no conversion formula supplied /
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    }

    / Step 26: Wait for interrupt /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    / Step 27: Issue Start Transfer on endpoint offset 0x50 /
    write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    / Original: while(rd_data==0x506){wait_on(10);rd_data=read_reg(MIZAR_USB_DEPCMD+0x50);} /
    // MANUAL_REVIEW: DEPCMD polling -- needs max iteration count and wait_on(10) conversion
    while (rd_data == 0x506) {
        / wait_on(10) -- MANUAL_REVIEW: no conversion formula supplied /
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    }

    / Step 28: Wait for interrupt /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    / Step 29: Debug marker write from original source /
    write_reg(0xA0243ffc, 0xdeadbee7);

    / Step 30: Wait for interrupt /
    // MANUAL_REVIEW: int_pend polling -- needs max iteration count and wait_on(5) conversion
    int_pend = 1;
    while (int_pend) {
        / wait_on(5) -- MANUAL_REVIEW: no conversion formula supplied /
    }

    / Step 31: Original: finish(0) -- replaced with PSV completion /
    LOGT("USB FS Device Bulk Transfer test run complete: PASS");
    out->status = 0;

    return out->status;
}


/*
 * usb_fs_device_bulk_transfer_test_teardown
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown: no additional cleanup required");
    return 0;
}
