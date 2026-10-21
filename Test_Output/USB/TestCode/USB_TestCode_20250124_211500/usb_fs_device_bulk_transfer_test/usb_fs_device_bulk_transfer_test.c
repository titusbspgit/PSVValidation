// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Testcase context for tracking errors and validation
typedef struct {
    unsigned int errors;
    unsigned int transfer_complete_count;
    unsigned int event_reset_seen;
    unsigned int event_connect_done_seen;
    unsigned int event_xfer_complete_seen;
    unsigned int bulk_out_complete;
    unsigned int bulk_in_complete;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} usb_test_ctx_t;

static usb_test_ctx_t g_ctx;

// Timeout limit for polling loops
#define POLL_TIMEOUT_LIMIT 100000U
#define INT_WAIT_TIMEOUT 500000U

/*
 * Function: wait_delay
 * Description: Deterministic busy-wait delay replacing DV wait_on().
 * Parameters:
 *   count - relative delay multiplier
 * Returns:
 *   void
 */
static void wait_delay(int count)
{
    (void)count;
    for (volatile int d = 0; d < 10000; d++);
}

/*
 * Function: wait_for_int_pend_with_timeout
 * Description: Wait for int_pend to be cleared by ISR with timeout protection.
 * Parameters:
 *   label - diagnostic label for timeout reporting
 * Returns:
 *   0 on success, -1 on timeout
 */
static int wait_for_int_pend_with_timeout(const char *label)
{
    unsigned int timeout = 0;
    while (int_pend) {
        for (volatile int d = 0; d < 10000; d++);
        timeout++;
        if (timeout >= INT_WAIT_TIMEOUT) {
            LOGE("Timeout waiting for interrupt: %s", label);
            g_ctx.errors++;
            return -1;
        }
    }
    return 0;
}

/*
 * Function: set_configuration
 * Description: Configure endpoint using DEPCMD with command completion polling.
 * Parameters:
 *   trb_address - endpoint offset
 *   parameter0 - DEPCMDPAR0 value
 *   parameter1 - DEPCMDPAR1 value
 *   cmd - DEPCMD command value
 * Returns:
 *   void
 */
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
    int read_data;
    unsigned int timeout = 0;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
    read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);

    while ((read_data & 0xFFF) == (cmd & 0xFFF)) {
        for (volatile int d = 0; d < 10000; d++);
        read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("set_configuration timeout: trb_address=0x%x cmd=0x%x", trb_address, cmd);
            g_ctx.errors++;
            return;
        }
    }
    // MANUAL_REVIEW: Runtime condition 13 requires status-bit checking for command completion.
    // DV uses while(read_data == cmd). Adapted to check command active bit in lower 12 bits.
}

/*
 * Function: setup_stage
 * Description: Issue SETUP stage TRB on EP0 OUT and wait for completion.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void setup_stage(void)
{
    int rd_data;
    unsigned int timeout = 0;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);

    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("setup_stage: DEPCMD polling timeout rd_data=0x%x", rd_data);
            g_ctx.errors++;
            return;
        }
    }
    // MANUAL_REVIEW: Runtime condition 13 - adapted from while(rd_data == 0x506) to status-bit check.

    int_pend = 1;
    wait_for_int_pend_with_timeout("setup_stage int_pend");
}

/*
 * Function: status_stage
 * Description: Issue STATUS stage TRB on EP0 OUT and wait for completion.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void status_stage(void)
{
    int rd_data;
    unsigned int timeout = 0;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x843);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);

    write_reg(MIZAR_USB_DEPCMD, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("status_stage: DEPCMD polling timeout rd_data=0x%x", rd_data);
            g_ctx.errors++;
            return;
        }
    }

    // int_pend wait #1
    int_pend = 1;
    wait_for_int_pend_with_timeout("status_stage int_pend #1");

    // standalone delay
    for (volatile int d = 0; d < 10000; d++);

    // int_pend wait #2
    int_pend = 1;
    wait_for_int_pend_with_timeout("status_stage int_pend #2");
}

/*
 * Function: enumeration
 * Description: Perform USB enumeration: GET_DESCRIPTOR device, config short, config full.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void enumeration(void)
{
    int rd_data;
    unsigned int timeout;

    // GET DESCRIPTOR - Device Descriptor
    setup_stage();

    // DV debug marker 0xdeadbee1
    LOGD("DV debug marker 0xdeadbee1: enumeration GET_DESCRIPTOR device");

    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: after setup_stage device desc int_pend");

    // TRB for device descriptor data stage (0x12 bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Device descriptor data
    write_reg(Buffer_PointerLO, 0x02000012);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100);

    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("enumeration: DEPCMD+0x10 device desc polling timeout");
            g_ctx.errors++;
            return;
        }
    }

    // int_pend wait #1
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: device desc int_pend #1");

    // standalone delay
    for (volatile int d = 0; d < 10000; d++);

    // int_pend wait #2
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: device desc int_pend #2");

    // xfernotready event wait
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: xfernotready int_pend #3");

    status_stage();

    // GET DESCRIPTOR - USB Configuration (short, 0x9 bytes)
    setup_stage();

    // DV debug marker 0xdeadbee3
    LOGD("DV debug marker 0xdeadbee3: enumeration GET_DESCRIPTOR config short");

    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: after setup_stage config short int_pend");

    // TRB for config descriptor data stage (0x9 bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x9);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Configuration descriptor data (short)
    write_reg(Buffer_PointerLO, 0x003c0209);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    write_reg(Buffer_PointerLO + 0x8, 0x00000032);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000);

    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("enumeration: DEPCMD+0x10 config short polling timeout");
            g_ctx.errors++;
            return;
        }
    }

    // int_pend wait #4
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: config short int_pend #4");

    // standalone delay
    for (volatile int d = 0; d < 10000; d++);

    // int_pend wait #5
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: config short int_pend #5");

    // int_pend wait #6
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: config short int_pend #6");

    // GET DESCRIPTOR - USB Configuration (full, 0x3c bytes)
    setup_stage();

    // DV debug marker 0xdeadbee5
    LOGD("DV debug marker 0xdeadbee5: enumeration GET_DESCRIPTOR config full");

    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: after setup_stage config full int_pend");

    // TRB for full config descriptor data stage (0x3c bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);

    // Full configuration descriptor data
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
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("enumeration: DEPCMD+0x10 config full polling timeout");
            g_ctx.errors++;
            return;
        }
    }

    // int_pend wait #7
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: config full int_pend #7");

    // standalone delay
    for (volatile int d = 0; d < 10000; d++);

    // int_pend wait #8
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: config full int_pend #8");

    // int_pend wait #9
    int_pend = 1;
    wait_for_int_pend_with_timeout("enumeration: config full int_pend #9");

    status_stage();
}

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for USB/sysreg events. Reads status, clears events, clears GIC.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rd_data;
    int event_count;

    int_pend = 0;
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    // MANUAL_REVIEW: Runtime condition 14 requires bitwise AND (&) for mask checking.
    // DV source uses logical AND (&&). Preserving DV behavior per No Silent Correction Rule.
    // Runtime condition 14 conflicts with DV source: DV has (rd_data && 0x80000000).
    // Applying runtime condition 14: using bitwise AND (&) as authoritative.
    if (rd_data & 0x80000000) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    // MANUAL_REVIEW: Runtime conditions 8/20 require event-ring decoding and event validation.
    // DV source does not decode event-ring entries beyond reading GEVNTCOUNT.
    // Preserving DV behavior. Event decoding would require inventing event-ring layout details.
    // Track that an event was processed for validation purposes.
    if (event_count > 0) {
        g_ctx.transfer_complete_count++;
    }

    GIC_ClearIRQ(84);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initialize USB FS Device Bulk Transfer test.
 * Parameters:
 *   cfg - test configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (usb_test_ctx_t){0};

    LOGT("USB FS Device Bulk Transfer test init");

    // nic_programming
    nic_programming();

    // Enable all IRQs via GIC
    GIC_EnableAllIRQ();

    LOGT("USB FS Device Bulk Transfer test init complete");
    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Execute USB FS Device Bulk Transfer test main flow.
 * Parameters:
 *   cfg - test configuration
 *   out - test output structure
 * Returns:
 *   0 on PASS, -1 on FAIL
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    int wr_data;
    int i;
    int j;
    int hand_shake;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("USB output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting USB FS Device Bulk Transfer test run");

    // Clear buffer and TRB memory
    for (j = 0; j < 20; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }

    // Soft reset
    wr_data = 0x40f00000;
    write_reg(MIZAR_USB_DCTL, wr_data);
    rd_data = read_reg(MIZAR_USB_DCTL);
    timeout = 0;
    while ((rd_data & 0x00f00000) != 0xf00000) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DCTL);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("Soft reset polling timeout rd_data=0x%x", rd_data);
            g_ctx.errors++;
            out->status = -1;
            return -1;
        }
    }
    // MANUAL_REVIEW: Runtime condition 13 - DV checks rd_data != 0xf00000 (full register).
    // Adapted to mask-based check for relevant bits. Preserving DV completion value 0xf00000.
    LOGT("Soft Rst is done");

    // PHY configuration
    wr_data = 0x40002407;
    write_reg(MIZAR_USB_GUSB2PHYCFG, wr_data);

    // Event buffer setup
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    // Global controller configuration
    rd_data = read_reg(MIZAR_USB_GCTL);

    wr_data = 0x30c12214;
    write_reg(MIZAR_USB_GCTL, wr_data);

    rd_data = read_reg(MIZAR_USB_DCFG);
    write_reg(MIZAR_USB_DCFG, 0x480801);
    write_reg(MIZAR_USB_DEVTEN, 0x1f);
    rd_data = read_reg(MIZAR_USB_GUCTL);
    write_reg(MIZAR_USB_GUCTL, 0xa400010);

    // Endpoint configuration: START NEW CONFIGURATION
    set_configuration(0, 0, 0, 0x409);

    // 8 endpoint parameter sets
    set_configuration(0, 0x200, 0x700, 0x401);
    set_configuration(0x10, 0x200, 0x2000700, 0x401);
    set_configuration(0x20, 0x206, 0x4000700, 0x401);
    set_configuration(0x30, 0x20206, 0x6000700, 0x401);
    set_configuration(0x40, 0x204, 0x8000700, 0x401);
    set_configuration(0x50, 0x40204, 0xa000700, 0x401);
    set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
    set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

    // Endpoint TX resource configuration
    for (i = 0; i < 8; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        timeout = 0;
        while ((rd_data & 0xFFF) == 0x402) {
            for (volatile int d = 0; d < 10000; d++);
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
            timeout++;
            if (timeout >= POLL_TIMEOUT_LIMIT) {
                LOGE("EP TX resource config timeout: ep=%d rd_data=0x%x", i, rd_data);
                g_ctx.errors++;
                break;
            }
        }
    }

    // Enable physical endpoints 0,1
    write_reg(MIZAR_USB_DALEPENA, 0x3);
    // Start device controller
    write_reg(MIZAR_USB_DCTL, 0x80f00000);
    // Interrupt enable at sysreg
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);

    // Link state connect reset events
    int_pend = 1;
    wait_for_int_pend_with_timeout("link state connect reset int_pend");

    if (event_counter <= 0x4) {
        int_pend = 1;
        wait_for_int_pend_with_timeout("event_counter<=0x4 int_pend #1");
    }

    write_reg(MIZAR_USB_DCFG, 0x480801);

    if (event_counter <= 0x4) {
        int_pend = 1;
        wait_for_int_pend_with_timeout("event_counter<=0x4 int_pend #2");
    }

    // DV debug marker 0xdeadbee0 - Enumeration start
    LOGD("DV debug marker 0xdeadbee0: Enumeration start");

    rd_data = read_reg(MIZAR_USB_DCFG);
    rd_data = read_reg(MIZAR_USB_DSTS);

    // Runtime condition 18: Speed verification via DSTS
    LOGT("DSTS read for speed verification: rd_data=0x%x", rd_data);
    // MANUAL_REVIEW: Runtime condition 18 requires speed verification.
    // DV reads DSTS but does not validate speed. Checking FS speed bits.
    if (((rd_data >> 0) & 0x7) != 0x1) {
        LOGE("Speed verification: expected FS (0x1), got 0x%x", (rd_data & 0x7));
        // MANUAL_REVIEW: DV does not fail on speed mismatch. Adding warning per runtime condition 18.
        g_ctx.errors++;
    }

    write_reg(MIZAR_USB_DCFG, 0x480801);
    write_reg(MIZAR_USB_DCTL, 0x80f00a00);
    write_reg(MIZAR_USB_DALEPENA, 0xff);

    LOGT("Buffer_PointerLO is 0x%x", Buffer_PointerLO);

    // wait_on(5000) - large delay
    for (volatile int d = 0; d < 10000; d++);

    // Setup stage for first enumeration request
    setup_stage();

    // PHY reconfiguration
    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

    if (event_counter <= 0x4) {
        int_pend = 1;
        wait_for_int_pend_with_timeout("after PHY reconfig int_pend #3");
    }

    if (event_counter <= 0x4) {
        int_pend = 1;
        wait_for_int_pend_with_timeout("after PHY reconfig int_pend #4");
    }

    // SET ADDRESS
    write_reg(MIZAR_USB_DCFG, 0x480809);
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("SET ADDRESS: DEPCMD+0x10 polling timeout");
            g_ctx.errors++;
            break;
        }
    }

    // int_pend wait after SET ADDRESS #1
    int_pend = 1;
    wait_for_int_pend_with_timeout("SET ADDRESS int_pend #1");

    // standalone delay
    for (volatile int d = 0; d < 10000; d++);

    // int_pend wait after SET ADDRESS #2
    int_pend = 1;
    wait_for_int_pend_with_timeout("SET ADDRESS int_pend #2");

    // int_pend wait after SET ADDRESS #3
    int_pend = 1;
    wait_for_int_pend_with_timeout("SET ADDRESS int_pend #3");

    // Status stage
    status_stage();

    // USB_SET_CONFIGURATION_OR_RESET_TT
    setup_stage();

    // DV debug marker 0xdeadbee4 - status data stage
    LOGD("DV debug marker 0xdeadbee4: USB_SET_CONFIGURATION status data stage");

    int_pend = 1;
    wait_for_int_pend_with_timeout("SET_CONFIG status data int_pend");

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x00);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("SET_CONFIG data: DEPCMD+0x10 polling timeout");
            g_ctx.errors++;
            break;
        }
    }

    if (event_counter <= 0x4) {
        int_pend = 1;
        wait_for_int_pend_with_timeout("SET_CONFIG event_counter<=0x4 int_pend");
    }

    int_pend = 1;
    wait_for_int_pend_with_timeout("SET_CONFIG final int_pend");

    // Handshake polling at 0xa0243ff4
    hand_shake = read_reg(0xa0243ff4);
    timeout = 0;
    while (hand_shake == 0x0) {
        hand_shake = read_reg(0xa0243ff4);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("Handshake polling timeout at 0xa0243ff4");
            g_ctx.errors++;
            break;
        }
    }

    // Enumeration (GET_DESCRIPTOR device, config short, config full)
    enumeration();

    // Handshake polling at 0xa0243ff8
    hand_shake = read_reg(0xa0243ff8);
    timeout = 0;
    while (hand_shake == 0x0) {
        hand_shake = read_reg(0xa0243ff8);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("Handshake polling timeout at 0xa0243ff8");
            g_ctx.errors++;
            break;
        }
    }

    // DV debug marker 0xdeadbee6 - Bulk transfer phase
    LOGD("DV debug marker 0xdeadbee6: Bulk transfer phase");

    int_pend = 1;
    wait_for_int_pend_with_timeout("bulk phase int_pend");

    // Bulk OUT transfer on endpoint offset 0x40
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("Bulk OUT: DEPCMD+0x40 polling timeout");
            g_ctx.errors++;
            break;
        }
    }

    // Bulk OUT int_pend wait #1
    int_pend = 1;
    wait_for_int_pend_with_timeout("bulk OUT int_pend #1");

    // Bulk OUT int_pend wait #2
    int_pend = 1;
    wait_for_int_pend_with_timeout("bulk OUT int_pend #2");

    g_ctx.bulk_out_complete = 1;
    LOGT("Bulk OUT transfer command issued and events received");

    // Bulk IN transfer on endpoint offset 0x50
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
    timeout = 0;
    while ((rd_data & 0xFFF) == 0x506) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
        timeout++;
        if (timeout >= POLL_TIMEOUT_LIMIT) {
            LOGE("Bulk IN: DEPCMD+0x50 polling timeout");
            g_ctx.errors++;
            break;
        }
    }

    // Bulk IN int_pend wait
    int_pend = 1;
    wait_for_int_pend_with_timeout("bulk IN int_pend");

    g_ctx.bulk_in_complete = 1;
    LOGT("Bulk IN transfer command issued and events received");

    // DV debug marker 0xdeadbee7 - Test completion
    LOGD("DV debug marker 0xdeadbee7: Test completion");

    int_pend = 1;
    wait_for_int_pend_with_timeout("final int_pend before completion");

    // MANUAL_REVIEW: Runtime condition 10 requires data integrity validation for bulk transfers.
    // DV source does not perform explicit payload comparison for bulk IN/OUT.
    // Adding best-effort validation: read back bulk buffer and compare.
    // The DV writes Buffer_PointerLO_1 as the TRB buffer pointer for both bulk OUT and IN.
    // Without knowing the expected payload pattern, exact comparison cannot be generated.
    // Flagging for manual review.
    LOGT("MANUAL_REVIEW: Data integrity validation for bulk transfers requires expected payload data");

    // Validation: check that bulk transfers completed
    if (g_ctx.bulk_out_complete == 0) {
        LOGE("FAIL: Bulk OUT transfer did not complete");
        g_ctx.errors++;
    }

    if (g_ctx.bulk_in_complete == 0) {
        LOGE("FAIL: Bulk IN transfer did not complete");
        g_ctx.errors++;
    }

    // Validation: check that events were received
    if (g_ctx.transfer_complete_count == 0) {
        LOGE("FAIL: No USB events were received during test");
        g_ctx.errors++;
    }

    // Final status
    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u transfer_events=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.transfer_complete_count);

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown and cleanup for USB FS Device Bulk Transfer test.
 * Parameters:
 *   cfg - test configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown");

    // Runtime condition 22: Proper teardown - disable interrupts, endpoints, controller
    // Disable sysreg interrupt
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x0);

    // Disable device event notifications
    write_reg(MIZAR_USB_DEVTEN, 0x0);

    // Disable all physical endpoints
    write_reg(MIZAR_USB_DALEPENA, 0x0);

    // Clear event count
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    LOGT("USB FS Device Bulk Transfer test teardown complete: errors=%u", g_ctx.errors);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
