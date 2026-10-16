/*
 * usb_fs_device_bulk_transfer_test.c
 *
 * PSV testcase: USB Full-Speed Device Bulk Transfer test
 /

#include "usb_fs_device_bulk_transfer_test.h"

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include "reg_access.h"
#include "mmio.h"
#include "log.h"

/
 * MANUAL_REVIEW: gic_funcs.h is the established project header mapping
 * for GIC/IRQ/LSS APIs per Section 8. If GIC_EnableAllIRQ and GIC_ClearIRQ
 * are provided through a different header (e.g. usb.h), remove this include.
 /
#include "gic_funcs.h"

/
 * MANUAL_REVIEW: wait_on() has no supplied conversion formula or
 * iteration count for PSV busy-wait replacement. All polling and
 * delay loops that depend on wait_on() are affected. Provide a
 * delay-to-loop conversion formula or explicit iteration counts
 * for each wait_on() usage.
 /
#error "MANUAL_REVIEW: No conversion formula or iteration count supplied for wait_on() replacement"

/
 * MANUAL_REVIEW: enumeration() body was not supplied in retrieved source.
 * Cannot generate implementation.
 /
#error "MANUAL_REVIEW: enumeration() body not supplied - cannot generate implementation"

/
 * MANUAL_REVIEW: No maximum polling iteration count supplied for
 * int_pend condition-based waits or DEPCMD polling loops. A timeout
 * mechanism or maximum iteration count is required.
 /
#error "MANUAL_REVIEW: No maximum polling iteration count supplied for int_pend and DEPCMD polling timeouts"

/ File-scope declarations /
volatile int int_pend;
volatile int event_counter;

/ Forward declaration for nic_programming (implementation not supplied) /
void nic_programming(void);

/ Forward declarations for testcase-owned helpers (non-static, preserving original linkage) /
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd);
void setup_stage(void);
void status_stage(void);
void enumeration(void);
int test_case(void);

/
 * Default_IRQHandler
 * Interrupt handler for USB / LSS system interrupt.
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

    / Source uses logical AND (&&) not bitwise AND (&). Preserved as written. /
    if (rd_data && 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    GIC_ClearIRQ(84);
}

/
 * setup_stage
 * Prepares a setup-stage TRB and issues DEPCMD 0x506 on endpoint 0.
 /
void setup_stage(void)
{
    volatile int rd_data;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);

    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);

    /
     * MANUAL_REVIEW: The following polling and int_pend wait are inferred
     * from Meta description ("polls until command completes, then waits for
     * interrupt"). The actual source for this portion was not retrieved.
     * Verify against the complete original source.
     /
    / Polling DEPCMD until command 0x506 completes /
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        / MANUAL_REVIEW: wait_on(10) - no conversion formula supplied /
        / wait_on(10); /
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }

    / Wait for interrupt /
    int_pend = 1;
    while (int_pend) {
        / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
        / wait_on(5); /
    }
}

/
 * status_stage
 * Prepares a status-stage TRB and issues DEPCMD 0x506 on endpoint 0.
 /
void status_stage(void)
{
    volatile int rd_data;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x843);

    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    write_reg(MIZAR_USB_DEPCMD, 0x506);

    /
     * MANUAL_REVIEW: The following polling and int_pend wait are inferred
     * from Meta description ("polls until command completes, then waits for
     * interrupt"). The actual source for this portion was not retrieved.
     * Verify against the complete original source.
     /
    / Polling DEPCMD until command 0x506 completes /
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        / MANUAL_REVIEW: wait_on(10) - no conversion formula supplied /
        / wait_on(10); /
        rd_data = read_reg(MIZAR_USB_DEPCMD);
    }

    / Wait for interrupt /
    int_pend = 1;
    while (int_pend) {
        / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
        / wait_on(5); /
    }
}

/
 * set_configuration
 * Issues a DEPCMD with given parameters and polls for completion.
 /
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
    volatile int read_data;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);

    read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    while (read_data == cmd) {
        / MANUAL_REVIEW: wait_on(30) - no conversion formula supplied /
        / wait_on(30); /
        read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
    }
}

/
 * enumeration
 * MANUAL_REVIEW: Body not supplied in retrieved source.
 /
void enumeration(void)
{
    / MANUAL_REVIEW: enumeration() body not supplied - cannot implement /
}

/
 * test_case - original DV entry point, called from _run.
 * Returns 0 on success, -1 on failure.
 /
int test_case(void)
{
    volatile int rd_data;
    volatile int wr_data;
    unsigned int i;
    unsigned int j;

    nic_programming();
    GIC_EnableAllIRQ();

    / Clear 20 entries of buffer pointers and event TRB addresses /
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }

    / Configure global event buffer registers /
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    rd_data = read_reg(MIZAR_USB_GCTL);

    wr_data = 0x30c12214; / port direction /
    write_reg(MIZAR_USB_GCTL, wr_data);

    rd_data = read_reg(MIZAR_USB_DCFG);
    write_reg(MIZAR_USB_DCFG, 0x480801);

    write_reg(MIZAR_USB_DEVTEN, 0x1f);

    rd_data = read_reg(MIZAR_USB_GUCTL);
    write_reg(MIZAR_USB_GUCTL, 0xa400010);

    / START NEW CONFIGURATION /
    set_configuration(0, 0, 0, 0x409);

    / SET ENDPOINT CONFIGURATION for 8 endpoints /
    set_configuration(0, 0x200, 0x700, 0x401);
    set_configuration(0x10, 0x200, 0x2000700, 0x401);
    set_configuration(0x20, 0x206, 0x4000700, 0x401);
    set_configuration(0x30, 0x20206, 0x6000700, 0x401);
    set_configuration(0x40, 0x204, 0x8000700, 0x401);
    set_configuration(0x50, 0x40204, 0xa000700, 0x401);
    set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
    set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

    / Endpoint TX resource configuration: DEPCMD 0x402 for endpoints 0..7 /
    for (i = 0; i < 8; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        while (rd_data == 0x402) {
            / MANUAL_REVIEW: wait_on(10) - no conversion formula supplied /
            / wait_on(10); /
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        }
    }

    / Enable endpoints, set device control, enable system interrupts /
    write_reg(MIZAR_USB_DALEPENA, 0x3);
    write_reg(MIZAR_USB_DCTL, 0x80f00000);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);

    / Issue DEPCMD 0x506 at endpoint offset 0x10 and poll for completion /
    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        / MANUAL_REVIEW: wait_on(10) - no conversion formula supplied /
        / wait_on(10); /
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    }

    / Wait for interrupt /
    int_pend = 1;
    while (int_pend) {
        / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
        / wait_on(5); /
    }

    / ADDED to check the xfernotready event /
    int_pend = 1;
    while (int_pend) {
        / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
        / wait_on(5); /
    }

    / [intermediate source not fully retrieved] /

    / Data TRB /
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x853);

    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    / [intermediate source not fully retrieved] /

    / Device descriptor data stage /
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);

    / Device descriptor data /
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100);

    / [intermediate source not fully retrieved] /

    / GET DESCRIPTOR USB CONFIGURATION /
    setup_stage();
    write_reg(0xA0243ffc, 0xdeadbee3);

    int_pend = 1;
    while (int_pend) {
        / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
        / wait_on(5); /
    }

    / Data stage /
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x9);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x003c0209);

    / [intermediate source not fully retrieved] /

    status_stage();

    / USB_SET_CONFIGURATION_OR_RESET_TT /
    setup_stage();
    write_reg(0xA0243ffc, 0xdeadbee4);

    int_pend = 1;
    while (int_pend) {
        / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
        / wait_on(5); /
    }

    / GET DESCRIPTOR USB CONFIGURATION /
    setup_stage();
    write_reg(0xA0243ffc, 0xdeadbee5);

    int_pend = 1;
    while (int_pend) {
        / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
        / wait_on(5); /
    }

    / Data stage: configuration descriptor /
    / 09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff /
    / 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301 /
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

    / [intermediate source not fully retrieved] /

    / Enumeration /
    / DV debug marker replaced with PSV log per Section 9D /
    / Original: write_reg(0xA0243ffc, 0xdeadbee0) /
    LOGT("DV marker: write_reg(0xA0243ffc, 0xdeadbee0) - Enumeration");

    rd_data = read_reg(MIZAR_USB_DCFG);
    rd_data = read_reg(MIZAR_USB_DSTS);
    write_reg(MIZAR_USB_DCFG, 0x480801);
    write_reg(MIZAR_USB_DCTL, 0x80f00a00);
    write_reg(MIZAR_USB_DALEPENA, 0xff);

    printf("Buffer_PointerLO is %x\n", Buffer_PointerLO);

    / MANUAL_REVIEW: wait_on(5000) - no conversion formula supplied /
    / wait_on(5000); /

    / [intermediate source not fully retrieved] /

    / CDC Bulk data transfer /
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);

    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
    write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    while (rd_data == 0x506) {
        / MANUAL_REVIEW: wait_on(10) - no conversion formula supplied /
        / wait_on(10); /
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    }

    int_pend = 1;
    while (int_pend) {
        / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
        / wait_on(5); /
    }

    / [intermediate source not fully retrieved] /

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x00);

    / [intermediate source not fully retrieved] /

    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend) {
            / MANUAL_REVIEW: wait_on(5) - no conversion formula supplied /
            / wait_on(5); /
        }
    }

    / [remainder of test_case() not fully retrieved] /

    return 0;
}

/
 * Framework entry points
 */

int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB_FS_Device_Bulk_Transfer_test init");
    return 0;
}

int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    int result;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;
    out->actual_len = 0;
    out->actual_pattern[0] = 0;

    LOGT("USB_FS_Device_Bulk_Transfer_test run start");

    result = test_case();

    if (result != 0) {
        out->status = -1;
    }

    LOGT("USB_FS_Device_Bulk_Transfer_test run complete: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

int usb_fs_device_bulk_transfer_test_teardown(const TestsItem cfg)
{
    (void)cfg;

    LOGT("USB_FS_Device_Bulk_Transfer_test teardown");
    return 0;
}
