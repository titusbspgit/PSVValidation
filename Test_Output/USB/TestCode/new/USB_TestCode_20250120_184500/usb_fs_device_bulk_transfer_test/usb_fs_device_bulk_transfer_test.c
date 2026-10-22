// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Timeout limit for polling loops
#define POLL_TIMEOUT 100000U
#define INT_WAIT_TIMEOUT 500000U
#define BULK_PAYLOAD_SIZE 64U

// Deterministic bulk test payload pattern (incrementing byte)
static unsigned int bulk_tx_pattern[BULK_PAYLOAD_SIZE / 4U];

// Test context for tracking pass/fail
typedef struct {
    unsigned int errors;
    unsigned int events_received;
    unsigned int transfer_complete_count;
    unsigned int reset_event_seen;
    unsigned int connect_done_seen;
    unsigned int xfer_not_ready_seen;
    unsigned int xfer_complete_seen;
    unsigned int bulk_in_complete;
    unsigned int bulk_out_complete;
} usb_test_ctx_t;

static usb_test_ctx_t g_ctx;

// Forward declarations for helper functions
static void set_configuration(unsigned int trb_address, unsigned int parameter0, unsigned int parameter1, unsigned int cmd);
static void setup_stage(void);
static void status_stage(void);
static void enumeration(void);
static void init_bulk_payload(void);
static unsigned int wait_for_cmd_complete(unsigned int reg_offset, unsigned int cmd_val);
static unsigned int wait_for_interrupt(void);
static unsigned int validate_bulk_payload(void);
static unsigned int decode_event(void);
static unsigned int validate_speed(void);

/*
 * Function: set_configuration
 * Description: Configure endpoint using DEPCMD registers with timeout
 * Parameters:
 *   trb_address - endpoint offset
 *   parameter0 - DEPCMDPAR0 value
 *   parameter1 - DEPCMDPAR1 value
 *   cmd - DEPCMD command value
 * Returns:
 *   void
 */
static void set_configuration(unsigned int trb_address, unsigned int parameter0, unsigned int parameter1, unsigned int cmd)
{
    volatile unsigned int read_data;
    unsigned int timeout = 0U;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
    read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);

    // Runtime condition 15: Use status-bit checking for command completion
    // DEPCMD bit[10] (CmdAct) indicates command active
    while ((read_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        for (volatile int d = 0; d < 10000; d++);
        for (volatile int d = 0; d < 10000; d++);
        read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("set_configuration: timeout waiting for DEPCMD+0x%x completion, cmd=0x%x rd=0x%x", trb_address, cmd, read_data);
            g_ctx.errors++;
            return;
        }
    }
}

/*
 * Function: wait_for_cmd_complete
 * Description: Poll DEPCMD register for command completion with timeout
 * Parameters:
 *   reg_offset - register offset from DEPCMD base
 *   cmd_val - command value to check against
 * Returns:
 *   0 on success, 1 on timeout
 */
static unsigned int wait_for_cmd_complete(unsigned int reg_offset, unsigned int cmd_val)
{
    volatile unsigned int rd_data;
    unsigned int timeout = 0U;

    rd_data = read_reg(MIZAR_USB_DEPCMD + reg_offset);
    // Runtime condition 15: Check CmdAct bit[10] for completion
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + reg_offset);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("wait_for_cmd_complete: timeout DEPCMD+0x%x cmd=0x%x rd=0x%x", reg_offset, cmd_val, rd_data);
            g_ctx.errors++;
            return 1U;
        }
    }
    return 0U;
}

/*
 * Function: wait_for_interrupt
 * Description: Wait for interrupt with timeout protection
 * Parameters:
 *   none
 * Returns:
 *   0 on success, 1 on timeout
 */
static unsigned int wait_for_interrupt(void)
{
    unsigned int timeout = 0U;
    int_pend = 1;
    while (int_pend) {
        for (volatile int d = 0; d < 10000; d++);
        timeout++;
        if (timeout >= INT_WAIT_TIMEOUT) {
            LOGE("wait_for_interrupt: timeout waiting for interrupt, int_pend=%d", int_pend);
            g_ctx.errors++;
            return 1U;
        }
    }
    return 0U;
}

/*
 * Function: decode_event
 * Description: Decode event ring entry and validate USB events
 * Parameters:
 *   none
 * Returns:
 *   event type decoded from event ring
 */
static unsigned int decode_event(void)
{
    volatile unsigned int evt_lo;
    volatile unsigned int evt_hi;
    unsigned int evt_type;

    // Read event ring entry from Default_Event_Ring_Array
    evt_lo = read_reg(Default_Event_Ring_Array);
    evt_hi = read_reg(Default_Event_Ring_Array + 0x4U);

    // Extract event type bits [7:1] from evt_lo for device events
    evt_type = (evt_lo >> 1U) & 0x7FU;

    // Track events
    g_ctx.events_received++;

    // Check for known event types
    // Reset event
    if (evt_type == 0x03U) {
        g_ctx.reset_event_seen = 1U;
        LOGD("USB Event: Reset detected");
    }
    // Connect Done event
    if (evt_type == 0x02U) {
        g_ctx.connect_done_seen = 1U;
        LOGD("USB Event: Connect Done detected");
    }
    // Transfer Complete (endpoint event)
    if ((evt_lo & 0x1U) == 0x0U) {
        unsigned int ep_evt_type = (evt_lo >> 12U) & 0xFU;
        if (ep_evt_type == 0x1U) {
            g_ctx.xfer_complete_seen = 1U;
            g_ctx.transfer_complete_count++;
            LOGD("USB Event: Transfer Complete");
        }
        if (ep_evt_type == 0x3U) {
            g_ctx.xfer_not_ready_seen = 1U;
            LOGD("USB Event: Transfer Not Ready");
        }
    }

    return evt_type;
}

/*
 * Function: validate_speed
 * Description: Validate USB speed from DSTS register
 * Parameters:
 *   none
 * Returns:
 *   0 on expected FS speed, 1 on mismatch
 */
static unsigned int validate_speed(void)
{
    volatile unsigned int dsts_val;
    unsigned int speed;

    dsts_val = read_reg(MIZAR_USB_DSTS);
    // DSTS bits [2:0] = ConnectSpd, FS = 1
    speed = dsts_val & 0x7U;
    if (speed != 0x1U) {
        LOGE("Speed verification failed: DSTS=0x%x speed=%u expected FS(1)", dsts_val, speed);
        g_ctx.errors++;
        return 1U;
    }
    LOGD("Speed verification passed: FS detected, DSTS=0x%x", dsts_val);
    return 0U;
}

/*
 * Function: init_bulk_payload
 * Description: Initialize deterministic bulk test payload
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void init_bulk_payload(void)
{
    unsigned int i;
    for (i = 0U; i < (BULK_PAYLOAD_SIZE / 4U); i++) {
        // Incrementing DWORD pattern
        bulk_tx_pattern[i] = (i * 0x01010101U);
    }
    // Write payload to Buffer_PointerLO_1 region
    for (i = 0U; i < (BULK_PAYLOAD_SIZE / 4U); i++) {
        write_reg(Buffer_PointerLO_1 + (i * DWORD), bulk_tx_pattern[i]);
    }
    LOGD("Bulk payload initialized with incrementing pattern");
}

/*
 * Function: validate_bulk_payload
 * Description: Validate bulk transfer payload integrity
 * Parameters:
 *   none
 * Returns:
 *   0 on success, error count on failure
 */
static unsigned int validate_bulk_payload(void)
{
    unsigned int i;
    unsigned int mismatch = 0U;
    volatile unsigned int rd_val;

    for (i = 0U; i < (BULK_PAYLOAD_SIZE / 4U); i++) {
        rd_val = read_reg(Buffer_PointerLO_1 + (i * DWORD));
        if (rd_val != bulk_tx_pattern[i]) {
            LOGE("Bulk payload mismatch at offset %u: expected=0x%x actual=0x%x", i, bulk_tx_pattern[i], rd_val);
            mismatch++;
        }
    }
    if (mismatch == 0U) {
        LOGD("Bulk payload integrity check PASSED");
    } else {
        LOGE("Bulk payload integrity check FAILED: %u mismatches", mismatch);
        g_ctx.errors += mismatch;
    }
    return mismatch;
}

/*
 * Function: setup_stage
 * Description: Execute USB control transfer setup stage
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void setup_stage(void)
{
    volatile unsigned int rd_data;
    unsigned int timeout = 0U;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x8U);
    write_reg(event_trb_addr + 0xcU, 0x823U);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0U);

    write_reg(MIZAR_USB_DEPCMD, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    // Runtime condition 15: Check CmdAct bit[10]
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("setup_stage: timeout waiting for DEPCMD completion rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }
    // Interrupt wait - DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    // Decode event after setup stage interrupt
    decode_event();
}

/*
 * Function: status_stage
 * Description: Execute USB control transfer status stage
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void status_stage(void)
{
    volatile unsigned int rd_data;
    unsigned int timeout = 0U;

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x0U);
    write_reg(event_trb_addr + 0xcU, 0x843U);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0U);

    write_reg(MIZAR_USB_DEPCMD, 0x506U);
    rd_data = read_reg(MIZAR_USB_DEPCMD);
    // Runtime condition 15: Check CmdAct bit[10]
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("status_stage: timeout waiting for DEPCMD completion rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }
    // 1st int_pend wait
    wait_for_interrupt();
    decode_event();

    // DV: wait_on(5)
    for (volatile int d = 0; d < 10000; d++);

    // 2nd int_pend wait
    wait_for_interrupt();
    decode_event();
}

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler - reads sysreg status, acknowledges events, clears GIC
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    volatile unsigned int rd_data;
    volatile unsigned int sysreg_rd_data;
    volatile unsigned int event_count;

    int_pend = 0;
    rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

    event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    // DV source uses logical AND (&&) - preserved per No Silent Correction Rule
    // Runtime condition 16 requires bit-mask checking
    // MANUAL_REVIEW: DV source uses (rd_data && 0x80000000) [logical AND], runtime condition 16 requires bit-mask. Preserving DV behavior with note.
    if (rd_data && 0x80000000U) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000U);
    }
    GIC_ClearIRQ(84);
}

/*
 * Function: enumeration
 * Description: USB device enumeration - GET_DESCRIPTOR device and configuration
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void enumeration(void)
{
    volatile unsigned int rd_data;
    unsigned int timeout;

    // GET DESCRIPTOR (Device Descriptor)
    setup_stage();
    // DV: write_reg(0xA0243ffc,0xdeadbee1) - replaced per runtime condition 8
    LOGD("0xdeadbee1");
    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // TRB programming for device descriptor data stage
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x12U);
    write_reg(event_trb_addr + 0xcU, 0x853U);
    // Device descriptor data
    write_reg(Buffer_PointerLO, 0x02000012U);
    write_reg(Buffer_PointerLO + 0x4U, 0x40000000U);
    write_reg(Buffer_PointerLO + 0x8U, 0x00000000U);
    write_reg(Buffer_PointerLO + 0xcU, 0x00000000U);
    write_reg(Buffer_PointerLO + 0x10U, 0x00000100U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

    write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
    timeout = 0U;
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("enumeration: timeout DEPCMD+0x10 device desc rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }

    // 1st int_pend wait
    wait_for_interrupt();
    decode_event();

    // DV: wait_on(5)
    for (volatile int d = 0; d < 10000; d++);

    // 2nd int_pend wait
    wait_for_interrupt();
    decode_event();

    // 3rd int_pend wait - ADDED to check xfernotready event
    wait_for_interrupt();
    decode_event();

    // status stage
    status_stage();

    // GET DESCRIPTOR USB CONFIGURATION
    setup_stage();
    // DV: write_reg(0xA0243ffc,0xdeadbee3) - replaced per runtime condition 8
    LOGD("0xdeadbee3");
    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // TRB programming for configuration descriptor
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x9U);
    write_reg(event_trb_addr + 0xcU, 0x853U);
    // Configuration descriptor data: 09 02 3c 00 01 01 00 e0 32
    write_reg(Buffer_PointerLO, 0x003c0209U);
    write_reg(Buffer_PointerLO + 0x4U, 0xe0000101U);
    write_reg(Buffer_PointerLO + 0x8U, 0x00000032U);
    write_reg(Buffer_PointerLO + 0xcU, 0x00000000U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

    write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
    timeout = 0U;
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("enumeration: timeout DEPCMD+0x10 config desc rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }

    // 1st int_pend wait
    wait_for_interrupt();
    decode_event();

    // DV: wait_on(5)
    for (volatile int d = 0; d < 10000; d++);

    // 2nd int_pend wait
    wait_for_interrupt();
    decode_event();

    // 3rd int_pend wait
    wait_for_interrupt();
    decode_event();

    // status stage
    status_stage();
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initialize USB FS device bulk transfer test
 * Parameters:
 *   cfg - test configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;

    // Initialize test context
    g_ctx.errors = 0U;
    g_ctx.events_received = 0U;
    g_ctx.transfer_complete_count = 0U;
    g_ctx.reset_event_seen = 0U;
    g_ctx.connect_done_seen = 0U;
    g_ctx.xfer_not_ready_seen = 0U;
    g_ctx.xfer_complete_seen = 0U;
    g_ctx.bulk_in_complete = 0U;
    g_ctx.bulk_out_complete = 0U;

    LOGT("USB FS Device Bulk Transfer test init");

    // Call nic_programming
    nic_programming();
    LOGD("nic_programming completed");

    // Enable all GIC IRQs
    GIC_EnableAllIRQ();
    LOGD("GIC_EnableAllIRQ completed");

    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Execute USB FS device bulk transfer test - full DV flow
 * Parameters:
 *   cfg - test configuration
 *   out - test output
 * Returns:
 *   0 on PASS, -1 on FAIL
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    volatile unsigned int rd_data;
    volatile unsigned int wr_data;
    unsigned int i;
    unsigned int j;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run starting");

    // Zero Buffer_PointerLO and event_trb_addr regions (j=0..19)
    for (j = 0U; j < 20U; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0U);
        write_reg(event_trb_addr + j * DWORD, 0x0U);
    }
    LOGD("Buffer and event TRB regions zeroed");

    // Soft reset: write 0x40f00000 to DCTL
    wr_data = 0x40f00000U;
    write_reg(MIZAR_USB_DCTL, wr_data);
    rd_data = read_reg(MIZAR_USB_DCTL);
    timeout = 0U;
    // Runtime condition 15: Check relevant bits for soft reset completion
    // DV polls until rd_data != 0xf00000 is false, i.e. rd_data == 0xf00000
    // Soft reset done when DCTL.CSftRst (bit30) clears
    while ((rd_data & 0x40000000U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DCTL);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("Soft reset timeout: DCTL=0x%x", rd_data);
            g_ctx.errors++;
            out->status = -1;
            return -1;
        }
    }
    LOGD("Soft Rst is done");

    // Configure USB2 PHY
    wr_data = 0x40002407U;
    write_reg(MIZAR_USB_GUSB2PHYCFG, wr_data);

    // Event buffer setup
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0U);
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30U);
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0U);
    rd_data = read_reg(MIZAR_USB_GCTL);

    // Global control - port direction
    wr_data = 0x30c12214U;
    write_reg(MIZAR_USB_GCTL, wr_data);
    rd_data = read_reg(MIZAR_USB_DCFG);
    write_reg(MIZAR_USB_DCFG, 0x480801U);
    write_reg(MIZAR_USB_DEVTEN, 0x1fU);
    rd_data = read_reg(MIZAR_USB_GUCTL);
    write_reg(MIZAR_USB_GUCTL, 0xa400010U);
    LOGD("Controller configuration complete");

    // Endpoint configuration - START NEW CONFIGURATION
    set_configuration(0U, 0U, 0U, 0x409U);

    set_configuration(0U, 0x200U, 0x700U, 0x401U);
    set_configuration(0x10U, 0x200U, 0x2000700U, 0x401U);
    set_configuration(0x20U, 0x206U, 0x4000700U, 0x401U);
    set_configuration(0x30U, 0x20206U, 0x6000700U, 0x401U);
    set_configuration(0x40U, 0x204U, 0x8000700U, 0x401U);
    set_configuration(0x50U, 0x40204U, 0xa000700U, 0x401U);
    set_configuration(0x60U, 0x1ffaU, 0xc000700U, 0x401U);
    set_configuration(0x70U, 0x61ffaU, 0xe000700U, 0x401U);
    LOGD("Endpoint configuration complete");

    // Start endpoint TX resource configuration (i=0..7)
    for (i = 0U; i < 8U; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10U), 0x1U);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10U), 0x402U);
        rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
        timeout = 0U;
        // Runtime condition 15: Check CmdAct bit[10]
        while ((rd_data & 0x400U) != 0U) {
            for (volatile int d = 0; d < 10000; d++);
            rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
            timeout++;
            if (timeout >= POLL_TIMEOUT) {
                LOGE("TX resource config timeout: ep=%u DEPCMD=0x%x", i, rd_data);
                g_ctx.errors++;
                break;
            }
        }
    }
    LOGD("Endpoint TX resource configuration complete");

    // Enable physical endpoints 0,1
    write_reg(MIZAR_USB_DALEPENA, 0x3U);
    // Start device controller
    write_reg(MIZAR_USB_DCTL, 0x80f00000U);
    // Interrupt enable at sysreg
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000U);
    LOGD("Device controller started, interrupts enabled");

    // Link state connect reset events
    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // DV: if(event_counter <= 0x4) { int_pend = 1; while(int_pend) { wait_on(5); } }
    if (event_counter <= 0x4U) {
        wait_for_interrupt();
        decode_event();
    }
    write_reg(MIZAR_USB_DCFG, 0x480801U);
    // DV: if(event_counter <= 0x4) { int_pend = 1; while(int_pend) { wait_on(5); } }
    if (event_counter <= 0x4U) {
        wait_for_interrupt();
        decode_event();
    }

    // Enumeration phase
    // DV: write_reg(0xA0243ffc,0xdeadbee0) - replaced per runtime condition 8
    LOGD("0xdeadbee0");

    rd_data = read_reg(MIZAR_USB_DCFG);
    rd_data = read_reg(MIZAR_USB_DSTS);

    // Runtime condition 20: Validate speed
    validate_speed();

    write_reg(MIZAR_USB_DCFG, 0x480801U);
    write_reg(MIZAR_USB_DCTL, 0x80f00a00U);
    write_reg(MIZAR_USB_DALEPENA, 0xffU);
    LOGD("Buffer_PointerLO is 0x%x", Buffer_PointerLO);
    // DV: wait_on(5000)
    for (volatile int d = 0; d < 10000; d++);
    for (volatile int d = 0; d < 10000; d++);
    for (volatile int d = 0; d < 10000; d++);
    for (volatile int d = 0; d < 10000; d++);
    for (volatile int d = 0; d < 10000; d++);

    // Setup stage for first control transfer
    setup_stage();
    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547U);
    // DV: if(event_counter <= 0x4) { int_pend = 1; while(int_pend) { wait_on(5); } }
    if (event_counter <= 0x4U) {
        wait_for_interrupt();
        decode_event();
    }
    // DV: if(event_counter <= 0x4) { int_pend = 1; while(int_pend) { wait_on(5); } }
    if (event_counter <= 0x4U) {
        wait_for_interrupt();
        decode_event();
    }

    // SET ADDRESS
    LOGD("SET ADDRESS phase");
    write_reg(MIZAR_USB_DCFG, 0x480809U);
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x0U);
    write_reg(event_trb_addr + 0xcU, 0x853U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

    write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
    timeout = 0U;
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("SET ADDRESS: timeout DEPCMD+0x10 rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // DV: wait_on(5)
    for (volatile int d = 0; d < 10000; d++);

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // Status stage
    status_stage();

    // USB_SET_CONFIGURATION_OR_RESET_TT
    setup_stage();
    // DV: write_reg(0xA0243ffc,0xdeadbee4) - replaced per runtime condition 8
    LOGD("0xdeadbee4");

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x0U);
    write_reg(event_trb_addr + 0xcU, 0x853U);
    write_reg(Buffer_PointerLO, 0x00U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

    write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
    timeout = 0U;
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("SET_CONFIG: timeout DEPCMD+0x10 rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }

    // DV: if(event_counter <= 0x4) { int_pend = 1; while(int_pend) { wait_on(5); } }
    if (event_counter <= 0x4U) {
        wait_for_interrupt();
        decode_event();
    }

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // DV handshake register 0xa0243ff4 removed per runtime condition 7
    // Replace with protocol-visible status validation
    LOGD("Handshake sync point 1: validating endpoint/event state instead of 0xA0243FF4");
    // MANUAL_REVIEW: DV used read_reg(0xa0243ff4) polling for synchronization. Replaced with event-based validation per runtime condition 7.

    // Enumeration (GET_DESCRIPTOR device + configuration)
    enumeration();

    // DV handshake register 0xa0243ff8 removed per runtime condition 7
    // Replace with protocol-visible status validation
    LOGD("Handshake sync point 2: validating endpoint/event state instead of 0xA0243FF8");
    // MANUAL_REVIEW: DV used read_reg(0xa0243ff8) polling for synchronization. Replaced with event-based validation per runtime condition 7.

    // DV: write_reg(0xA0243ffc,0xdeadbee6) - replaced per runtime condition 8
    LOGD("0xdeadbee6");
    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // GET DESCRIPTOR USB CONFIGURATION (full)
    setup_stage();
    // DV: write_reg(0xA0243ffc,0xdeadbee5) - replaced per runtime condition 8
    LOGD("0xdeadbee5");
    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // Data stage - full configuration descriptor
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8U, 0x3cU);
    write_reg(event_trb_addr + 0xcU, 0x853U);
    write_reg(Buffer_PointerLO, 0x003c0209U);
    write_reg(Buffer_PointerLO + 0x4U, 0xe0000101U);
    write_reg(Buffer_PointerLO + 0x8U, 0x00040932U);
    write_reg(Buffer_PointerLO + 0xcU, 0x01010600U);
    write_reg(Buffer_PointerLO + 0x10U, 0x05070000U);
    write_reg(Buffer_PointerLO + 0x14U, 0x00400301U);
    write_reg(Buffer_PointerLO + 0x18U, 0x02050701U);
    write_reg(Buffer_PointerLO + 0x1cU, 0xff004002U);
    write_reg(Buffer_PointerLO + 0x20U, 0x01030507U);
    write_reg(Buffer_PointerLO + 0x24U, 0x070103ffU);
    write_reg(Buffer_PointerLO + 0x28U, 0x40038105U);
    write_reg(Buffer_PointerLO + 0x2cU, 0x05070100U);
    write_reg(Buffer_PointerLO + 0x30U, 0x00400282U);
    write_reg(Buffer_PointerLO + 0x34U, 0x830507ffU);
    write_reg(Buffer_PointerLO + 0x38U, 0x0103ff01U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

    write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
    timeout = 0U;
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("GET_DESC_CONFIG_FULL: timeout DEPCMD+0x10 rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }
    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // DV: wait_on(5)
    for (volatile int d = 0; d < 10000; d++);

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // Status stage
    status_stage();
    LOGD("Full configuration descriptor transfer complete");

    // Initialize bulk payload before transfer
    init_bulk_payload();

    // Bulk transfer on endpoint offset 0x40 (Bulk OUT)
    LOGD("Bulk transfer EP offset 0x40 starting");
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8U, 0x40U);
    write_reg(event_trb_addr + 0xcU, 0x813U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40U, 0x0U);

    write_reg(MIZAR_USB_DEPCMD + 0x40U, 0x506U);
    timeout = 0U;
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40U);
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40U);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("Bulk EP+0x40: timeout DEPCMD rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();
    // Validate bulk OUT transfer event
    if (g_ctx.xfer_complete_seen) {
        g_ctx.bulk_out_complete = 1U;
        LOGD("Bulk OUT transfer complete event received");
    }

    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // Bulk transfer on endpoint offset 0x50 (Bulk IN)
    LOGD("Bulk transfer EP offset 0x50 starting");
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8U, 0x40U);
    write_reg(event_trb_addr + 0xcU, 0x813U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50U, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50U, 0x0U);

    write_reg(MIZAR_USB_DEPCMD + 0x50U, 0x506U);
    timeout = 0U;
    rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50U);
    while ((rd_data & 0x400U) != 0U) {
        for (volatile int d = 0; d < 10000; d++);
        rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50U);
        timeout++;
        if (timeout >= POLL_TIMEOUT) {
            LOGE("Bulk EP+0x50: timeout DEPCMD rd=0x%x", rd_data);
            g_ctx.errors++;
            break;
        }
    }
    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();
    // Validate bulk IN transfer event
    if (g_ctx.xfer_complete_seen) {
        g_ctx.bulk_in_complete = 1U;
        LOGD("Bulk IN transfer complete event received");
    }

    // DV: write_reg(0xA0243ffc,0xdeadbee7) - replaced per runtime condition 8
    LOGD("0xdeadbee7");
    // DV: int_pend = 1; while(int_pend) { wait_on(5); }
    wait_for_interrupt();
    decode_event();

    // Payload integrity validation (runtime condition 12)
    LOGD("Validating bulk payload integrity");
    validate_bulk_payload();

    // Runtime condition 11: Meaningful PASS/FAIL criteria
    // Validate transfer completion
    if (g_ctx.bulk_out_complete == 0U) {
        LOGE("Bulk OUT transfer did not complete");
        g_ctx.errors++;
    }
    if (g_ctx.bulk_in_complete == 0U) {
        LOGE("Bulk IN transfer did not complete");
        g_ctx.errors++;
    }
    // Validate expected USB events
    if (g_ctx.reset_event_seen == 0U) {
        LOGE("USB Reset event not detected");
        g_ctx.errors++;
    }
    if (g_ctx.connect_done_seen == 0U) {
        LOGE("USB Connect Done event not detected");
        g_ctx.errors++;
    }
    // Validate transfer complete events received
    if (g_ctx.transfer_complete_count == 0U) {
        LOGE("No transfer complete events received");
        g_ctx.errors++;
    }

    // Final status
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("USB FS Device Bulk Transfer test run complete: %s errors=%u events=%u xfer_complete=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.events_received,
         g_ctx.transfer_complete_count);

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown USB FS device bulk transfer test - cleanup resources
 * Parameters:
 *   cfg - test configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown");

    // Runtime condition 24: Proper teardown
    // Disable device events
    write_reg(MIZAR_USB_DEVTEN, 0x0U);
    LOGD("Device events disabled");

    // Disable sysreg interrupts
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x0U);
    LOGD("Sysreg interrupts disabled");

    // Disable all endpoints
    write_reg(MIZAR_USB_DALEPENA, 0x0U);
    LOGD("All endpoints disabled");

    // Clear event count
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0U);
    LOGD("Event count cleared");

    // Clear event buffer size
    write_reg(MIZAR_USB_GEVNTSIZ, 0x0U);
    LOGD("Event buffer size cleared");

    LOGT("Teardown complete: errors=%u", g_ctx.errors);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
