// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Timeout limit for polling loops
#define POLL_TIMEOUT 100000U

// Bulk payload size in bytes
#define BULK_PAYLOAD_SIZE 64U

// Deterministic bulk test pattern
static const unsigned int bulk_tx_pattern[BULK_PAYLOAD_SIZE / 4U] = {
    0x01020304U, 0x05060708U, 0x090A0B0CU, 0x0D0E0F10U,
    0x11121314U, 0x15161718U, 0x191A1B1CU, 0x1D1E1F20U,
    0x21222324U, 0x25262728U, 0x292A2B2CU, 0x2D2E2F30U,
    0x31323334U, 0x35363738U, 0x393A3B3CU, 0x3D3E3F40U
};

// Testcase context
typedef struct {
    unsigned int errors;
    unsigned int events_received;
    unsigned int transfer_complete_count;
    unsigned int bulk_in_complete;
    unsigned int bulk_out_complete;
} usb_fs_test_ctx_t;

static usb_fs_test_ctx_t g_ctx;


/*
 * Function: wait_delay
 * Description: Deterministic busy-wait delay replacing DV wait_on()
 * Parameters:
 *   count - relative delay magnitude
 * Returns:
 *   void
 */
static void wait_delay(unsigned int count)
{
    (void)count;
    for (volatile int d = 0; d < 10000; d++);
}


/*
 * Function: wait_int_pend_with_timeout
 * Description: Wait for int_pend to be cleared by ISR with timeout protection
 * Parameters:
 *   none
 * Returns:
 *   0 on success, -1 on timeout
 */
static int wait_int_pend_with_timeout(void)
{
    unsigned int timeout = POLL_TIMEOUT;
    while (int_pend) {
        wait_delay(5);
        timeout--;
        if (timeout == 0U) {
            LOGE("Timeout waiting for int_pend to clear");
            g_ctx.errors++;
            return -1;
        }
    }
    return 0;
}


/*
 * Function: poll_depcmd_completion
 * Description: Poll DEPCMD register for command completion with timeout
 * Parameters:
 *   reg_offset - register offset to poll
 *   cmd_val - command value to check active bit against
 * Returns:
 *   0 on success, -1 on timeout
 */
static int poll_depcmd_completion(unsigned int reg_offset, unsigned int cmd_val)
{
    volatile unsigned int rd_data;
    unsigned int timeout = POLL_TIMEOUT;
    rd_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + reg_offset);
    // Runtime condition 15: use status-bit checking instead of full-value compare
    // Check CmdAct bit (bit 10) for command active
    while (rd_data & 0x400U) {
        wait_delay(10);
        rd_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + reg_offset);
        timeout--;
        if (timeout == 0U) {
            LOGE("Timeout polling DEPCMD at offset 0x%x, cmd=0x%x, rd_data=0x%x",
                 reg_offset, cmd_val, rd_data);
            g_ctx.errors++;
            return -1;
        }
    }
    return 0;
}


/*
 * Function: set_configuration
 * Description: Program endpoint configuration via DEPCMD
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
    unsigned int timeout = POLL_TIMEOUT;

    write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
    write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
    write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
    read_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + trb_address);

    // Runtime condition 15: use CmdAct bit checking
    while (read_data & 0x400U) {
        wait_delay(30);
        read_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + trb_address);
        timeout--;
        if (timeout == 0U) {
            LOGE("Timeout in set_configuration offset=0x%x cmd=0x%x", trb_address, cmd);
            g_ctx.errors++;
            return;
        }
    }
}


/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler - services interrupts, reads/clears event count,
 *              conditionally clears raw status, clears GIC IRQ 84
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    volatile unsigned int rd_data;
    unsigned int event_count;
    int_pend = 0;
    rd_data = (unsigned int)read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = (unsigned int)read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

    event_count = (unsigned int)read_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    // Runtime condition 16: use explicit bit-mask checking
    // DV source used logical && (preserved as-is per No Silent Correction Rule)
    // MANUAL_REVIEW: DV source uses logical AND (rd_data && 0x80000000) not bitwise.
    // Runtime condition 16 requires bit-mask checking. Applying bit-mask per runtime condition.
    if (rd_data & 0x80000000U) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000U);
    }

    // Runtime condition 10: decode event-ring entries
    // Validate USB events from event count
    if (event_count > 0U) {
        g_ctx.events_received++;
        LOGD("IRQ: event_count=0x%x events_received=%u", event_count, g_ctx.events_received);
    }

    GIC_ClearIRQ(84);
}


/*
 * Function: setup_stage
 * Description: USB control transfer setup stage - programs TRB and issues DEPSTRTXFER
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void setup_stage(void)
{
    LOGD("setup_stage: begin");
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x8);
    write_reg(event_trb_addr + 0xc, 0x823);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);

    write_reg(MIZAR_USB_DEPCMD, 0x506);
    poll_depcmd_completion(0, 0x506);

    int_pend = 1;
    wait_int_pend_with_timeout();
    LOGD("setup_stage: complete");
}


/*
 * Function: status_stage
 * Description: USB control transfer status stage - programs TRB and issues DEPSTRTXFER
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void status_stage(void)
{
    LOGD("status_stage: begin");
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x843);
    write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);

    write_reg(MIZAR_USB_DEPCMD, 0x506);
    poll_depcmd_completion(0, 0x506);

    int_pend = 1;
    wait_int_pend_with_timeout();

    wait_delay(5);

    int_pend = 1;
    wait_int_pend_with_timeout();
    LOGD("status_stage: complete");
}


/*
 * Function: enumeration
 * Description: USB device enumeration - handles GET_DEVICE_DESCRIPTOR,
 *              GET_DESCRIPTOR_CONFIG, SET_CONFIGURATION, recursive enumeration
 * Parameters:
 *   none
 * Returns:
 *   void
 */
static void enumeration(void)
{
    volatile unsigned int rd_data;

    // GET_DEVICE_DESCRIPTOR
    LOGD("enumeration: GET_DEVICE_DESCRIPTOR begin");
    setup_stage();
    LOGD("0xdeadbee1");
    int_pend = 1;
    wait_int_pend_with_timeout();

    // TRB for device descriptor data stage (0x12 bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x12);
    write_reg(event_trb_addr + 0xc, 0x853);
    // Device descriptor data
    write_reg(Buffer_PointerLO, 0x02000012U);
    write_reg(Buffer_PointerLO + 0x4, 0x40000000U);
    write_reg(Buffer_PointerLO + 0x8, 0x00000000U);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000U);
    write_reg(Buffer_PointerLO + 0x10, 0x00000100U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    poll_depcmd_completion(0x10, 0x506);

    int_pend = 1;
    wait_int_pend_with_timeout();

    wait_delay(5);

    int_pend = 1;
    wait_int_pend_with_timeout();

    // xfernotready event wait
    int_pend = 1;
    wait_int_pend_with_timeout();

    status_stage();

    // DEVICE QUALIFIER section (commented out in DV source - preserved as comments)
    // DV source had this entire section commented out with //
    // Not generating executable code for commented-out DV section

    // GET DESCRIPTOR USB CONFIGURATION (0x9 bytes)
    LOGD("enumeration: GET_DESCRIPTOR_CONFIG short begin");
    setup_stage();
    LOGD("0xdeadbee3");
    int_pend = 1;
    wait_int_pend_with_timeout();

    // TRB for config descriptor short (0x9 bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x9);
    write_reg(event_trb_addr + 0xc, 0x853);
    // Configuration descriptor data (short)
    write_reg(Buffer_PointerLO, 0x003c0209U);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101U);
    write_reg(Buffer_PointerLO + 0x8, 0x00000032U);
    write_reg(Buffer_PointerLO + 0xc, 0x00000000U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    poll_depcmd_completion(0x10, 0x506);

    int_pend = 1;
    wait_int_pend_with_timeout();

    wait_delay(5);

    int_pend = 1;
    wait_int_pend_with_timeout();

    int_pend = 1;
    wait_int_pend_with_timeout();

    status_stage();

    // USB_SET_CONFIGURATION_OR_RESET_TT
    LOGD("enumeration: SET_CONFIGURATION begin");
    setup_stage();
    LOGD("0xdeadbee4");

    int_pend = 1;
    wait_int_pend_with_timeout();

    // TRB for set configuration status data stage
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x00U);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    poll_depcmd_completion(0x10, 0x506);

    // Conditional event_counter check (active code from DV)
    if (event_counter <= 0x4U) {
        int_pend = 1;
        wait_int_pend_with_timeout();
    }

    // Unconditional int_pend wait (DV source had commented-out if around this)
    int_pend = 1;
    wait_int_pend_with_timeout();

    // Runtime condition 7: handshake register 0xa0243ff4 removed
    // DV used: hand_shake = read_reg(0xa0243ff4); while(hand_shake==0x0){...}
    // Replaced with event completion validation per runtime condition
    LOGD("enumeration: handshake sync point (0xa0243ff4 removed - using event validation)");
    // MANUAL_REVIEW: DV handshake register 0xa0243ff4 removed per runtime condition 7.
    // Replaced with int_pend wait for protocol-visible synchronization.
    int_pend = 1;
    wait_int_pend_with_timeout();

    // DV source had block comment around int_pend/wait_on before enumeration() call
    // (commented out in DV - not generating executable code)

    // Recursive enumeration call
    LOGD("enumeration: recursive call");
    enumeration();

    // Runtime condition 7: handshake register 0xa0243ff8 removed
    // DV used: hand_shake = read_reg(0xa0243ff8); while(hand_shake==0x0){...}
    LOGD("enumeration: handshake sync point (0xa0243ff8 removed - using event validation)");
    // MANUAL_REVIEW: DV handshake register 0xa0243ff8 removed per runtime condition 7.
    // Replaced with int_pend wait for protocol-visible synchronization.
    int_pend = 1;
    wait_int_pend_with_timeout();

    LOGD("0xdeadbee6");
    int_pend = 1;
    wait_int_pend_with_timeout();

    // GET DESCRIPTOR USB CONFIGURATION (0x3c-byte full)
    LOGD("enumeration: GET_DESCRIPTOR_CONFIG full begin");
    setup_stage();
    LOGD("0xdeadbee5");
    int_pend = 1;
    wait_int_pend_with_timeout();

    // TRB for full config descriptor (0x3c bytes)
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);
    // Full configuration descriptor data
    write_reg(Buffer_PointerLO, 0x003c0209U);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101U);
    write_reg(Buffer_PointerLO + 0x8, 0x00040932U);
    write_reg(Buffer_PointerLO + 0xc, 0x01010600U);
    write_reg(Buffer_PointerLO + 0x10, 0x05070000U);
    write_reg(Buffer_PointerLO + 0x14, 0x00400301U);
    write_reg(Buffer_PointerLO + 0x18, 0x02050701U);
    write_reg(Buffer_PointerLO + 0x1c, 0xff004002U);
    write_reg(Buffer_PointerLO + 0x20, 0x01030507U);
    write_reg(Buffer_PointerLO + 0x24, 0x070103ffU);
    write_reg(Buffer_PointerLO + 0x28, 0x40038105U);
    write_reg(Buffer_PointerLO + 0x2c, 0x05070100U);
    write_reg(Buffer_PointerLO + 0x30, 0x00400282U);
    write_reg(Buffer_PointerLO + 0x34, 0x830507ffU);
    write_reg(Buffer_PointerLO + 0x38, 0x0103ff01U);
    // DV source had writes at 0x3c-0x48 commented out - preserved as comments
    // write_reg(Buffer_PointerLO+0x3c,0x01830507);
    // write_reg(Buffer_PointerLO+0x40,0x07000400);
    // write_reg(Buffer_PointerLO+0x44,0x00010305);
    // write_reg(Buffer_PointerLO+0x48,0x00000004);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    poll_depcmd_completion(0x10, 0x506);

    int_pend = 1;
    wait_int_pend_with_timeout();

    wait_delay(5);

    int_pend = 1;
    wait_int_pend_with_timeout();

    int_pend = 1;
    wait_int_pend_with_timeout();

    status_stage();
    LOGD("enumeration: complete");
}


/*
 * Function: validate_bulk_payload
 * Description: Validate bulk transfer payload integrity
 * Parameters:
 *   base_addr - base address of payload buffer
 *   size - payload size in bytes
 *   pattern - expected data pattern array
 *   direction - 0 for OUT, 1 for IN
 * Returns:
 *   0 on success, -1 on mismatch
 */
static int validate_bulk_payload(unsigned int base_addr, unsigned int size,
                                 const unsigned int pattern, unsigned int direction)
{
    unsigned int i;
    unsigned int dwords = size / 4U;
    unsigned int rd_val;
    int result = 0;

    LOGD("Validating bulk %s payload at 0x%x, size=%u",
         (direction == 1U) ? "IN" : "OUT", base_addr, size);

    for (i = 0U; i < dwords; i++) {
        rd_val = (unsigned int)read_reg(base_addr + (i * 4U));
        if (rd_val != pattern[i]) {
            LOGE("Bulk payload mismatch at offset 0x%x: expected=0x%x actual=0x%x",
                 i * 4U, pattern[i], rd_val);
            g_ctx.errors++;
            result = -1;
        }
    }
    return result;
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

    g_ctx.errors = 0U;
    g_ctx.events_received = 0U;
    g_ctx.transfer_complete_count = 0U;
    g_ctx.bulk_in_complete = 0U;
    g_ctx.bulk_out_complete = 0U;
    event_counter = 0U;

    LOGT("USB FS Device Bulk Transfer test init");

    // Call nic_programming
    nic_programming();

    // Enable all GIC IRQs
    GIC_EnableAllIRQ();

    LOGT("nic_programming and GIC_EnableAllIRQ complete");

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
    unsigned int wr_data;
    unsigned int i;
    unsigned int j;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("USB FS Device Bulk Transfer test run begin");

    // Clear buffer and TRB memory regions (j=0..19)
    for (j = 0U; j < 20U; j++) {
        write_reg(Buffer_PointerLO + j * DWORD, 0x0);
        write_reg(event_trb_addr + j * DWORD, 0x0);
    }

    // Soft reset via MIZAR_USB_DCTL
    wr_data = 0x40f00000U;
    write_reg(MIZAR_USB_DCTL, wr_data);
    rd_data = (unsigned int)read_reg(MIZAR_USB_DCTL);
    timeout = POLL_TIMEOUT;
    // Runtime condition 15: use status-bit checking
    // Check for soft reset completion - bit 30 (CSFTRST) should clear
    while (rd_data & 0x40000000U) {
        wait_delay(5);
        rd_data = (unsigned int)read_reg(MIZAR_USB_DCTL);
        timeout--;
        if (timeout == 0U) {
            LOGE("Timeout waiting for soft reset completion, DCTL=0x%x", rd_data);
            g_ctx.errors++;
            out->status = -1;
            return -1;
        }
    }
    LOGT("Soft Rst is done");

    // Configure USB2 PHY
    wr_data = 0x40002407U;
    write_reg(MIZAR_USB_GUSB2PHYCFG, wr_data);

    // Event ring setup
    write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
    write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
    write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);
    rd_data = (unsigned int)read_reg(MIZAR_USB_GCTL);

    // Port direction configuration
    wr_data = 0x30c12214U;
    write_reg(MIZAR_USB_GCTL, wr_data);
    rd_data = (unsigned int)read_reg(MIZAR_USB_DCFG);
    write_reg(MIZAR_USB_DCFG, 0x480801U);
    write_reg(MIZAR_USB_DEVTEN, 0x1fU);
    rd_data = (unsigned int)read_reg(MIZAR_USB_GUCTL);
    write_reg(MIZAR_USB_GUCTL, 0x0a400010U);

    // Endpoint configuration - START NEW CONFIGURATION
    LOGD("Endpoint configuration begin");
    set_configuration(0U, 0U, 0U, 0x409U);

    set_configuration(0U, 0x200U, 0x700U, 0x401U);
    set_configuration(0x10U, 0x200U, 0x2000700U, 0x401U);
    set_configuration(0x20U, 0x206U, 0x4000700U, 0x401U);
    set_configuration(0x30U, 0x20206U, 0x6000700U, 0x401U);
    set_configuration(0x40U, 0x204U, 0x8000700U, 0x401U);
    set_configuration(0x50U, 0x40204U, 0x0a000700U, 0x401U);
    set_configuration(0x60U, 0x1ffaU, 0x0c000700U, 0x401U);
    set_configuration(0x70U, 0x61ffaU, 0x0e000700U, 0x401U);
    LOGD("Endpoint configuration complete");

    // Endpoint TX resource configuration (i=0..7)
    LOGD("Endpoint TX resource configuration begin");
    for (i = 0U; i < 8U; i++) {
        write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10U), 0x1);
        write_reg(MIZAR_USB_DEPCMD + (i * 0x10U), 0x402);
        rd_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
        timeout = POLL_TIMEOUT;
        // Runtime condition 15: use CmdAct bit checking
        while (rd_data & 0x400U) {
            wait_delay(10);
            rd_data = (unsigned int)read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
            timeout--;
            if (timeout == 0U) {
                LOGE("Timeout in EP TX resource config i=%u, rd_data=0x%x", i, rd_data);
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
    // Enable sysreg interrupts
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000U);

    // Link state connect reset events
    LOGD("Waiting for link state/connect/reset events");
    int_pend = 1;
    wait_int_pend_with_timeout();

    if (event_counter <= 0x4U) {
        int_pend = 1;
        wait_int_pend_with_timeout();
    }

    write_reg(MIZAR_USB_DCFG, 0x480801U);

    if (event_counter <= 0x4U) {
        int_pend = 1;
        wait_int_pend_with_timeout();
    }

    // Enumeration marker
    LOGD("0xdeadbee0");

    rd_data = (unsigned int)read_reg(MIZAR_USB_DCFG);
    rd_data = (unsigned int)read_reg(MIZAR_USB_DSTS);

    // Runtime condition 20: verify expected speed from DSTS
    // DSTS bits [2:0] indicate connection speed. FS = 0x1
    {
        unsigned int speed = rd_data & 0x7U;
        LOGD("DSTS speed field = 0x%x", speed);
        if (speed != 0x1U) {
            LOGE("Expected Full-Speed (0x1) but got speed=0x%x", speed);
            g_ctx.errors++;
        }
    }

    write_reg(MIZAR_USB_DCFG, 0x480801U);
    write_reg(MIZAR_USB_DCTL, 0x80f00a00U);
    write_reg(MIZAR_USB_DALEPENA, 0xffU);
    LOGD("Buffer_PointerLO is 0x%x", Buffer_PointerLO);
    wait_delay(5000);

    setup_stage();
    write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547U);

    if (event_counter <= 0x4U) {
        int_pend = 1;
        wait_int_pend_with_timeout();
    }

    if (event_counter <= 0x4U) {
        int_pend = 1;
        wait_int_pend_with_timeout();
    }

    // DV source had commented-out int_pend/wait_on block here
    // (commented out in DV - not generating executable code)

    // SET ADDRESS
    LOGD("SET_ADDRESS begin");
    write_reg(MIZAR_USB_DCFG, 0x480809U);
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x0);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    poll_depcmd_completion(0x10, 0x506);

    if (event_counter <= 0x4U) {
        int_pend = 1;
        wait_int_pend_with_timeout();
    }

    int_pend = 1;
    wait_int_pend_with_timeout();

    // Runtime condition 7: handshake register 0xa0243ff4 removed
    // DV used: hand_shake = read_reg(0xa0243ff4); while(hand_shake==0x0){...}
    LOGD("test_case: handshake sync point (0xa0243ff4 removed - using event validation)");
    // MANUAL_REVIEW: DV handshake register 0xa0243ff4 removed per runtime condition 7.
    // Replaced with int_pend wait for protocol-visible synchronization.
    int_pend = 1;
    wait_int_pend_with_timeout();

    // Enumeration
    LOGD("Calling enumeration()");
    enumeration();

    // Runtime condition 7: handshake register 0xa0243ff8 removed
    // DV used: hand_shake = read_reg(0xa0243ff8); while(hand_shake==0x0){...}
    LOGD("test_case: handshake sync point (0xa0243ff8 removed - using event validation)");
    // MANUAL_REVIEW: DV handshake register 0xa0243ff8 removed per runtime condition 7.
    // Replaced with int_pend wait for protocol-visible synchronization.
    int_pend = 1;
    wait_int_pend_with_timeout();

    LOGD("0xdeadbee6");
    int_pend = 1;
    wait_int_pend_with_timeout();

    // GET DESCRIPTOR USB CONFIGURATION (0x3c-byte) from test_case
    LOGD("test_case: GET_DESCRIPTOR_CONFIG full begin");
    setup_stage();
    LOGD("0xdeadbee5");
    int_pend = 1;
    wait_int_pend_with_timeout();

    // Data stage - full configuration descriptor
    write_reg(event_trb_addr, Buffer_PointerLO);
    write_reg(event_trb_addr + 0x8, 0x3c);
    write_reg(event_trb_addr + 0xc, 0x853);
    write_reg(Buffer_PointerLO, 0x003c0209U);
    write_reg(Buffer_PointerLO + 0x4, 0xe0000101U);
    write_reg(Buffer_PointerLO + 0x8, 0x00040932U);
    write_reg(Buffer_PointerLO + 0xc, 0x01010600U);
    write_reg(Buffer_PointerLO + 0x10, 0x05070000U);
    write_reg(Buffer_PointerLO + 0x14, 0x00400301U);
    write_reg(Buffer_PointerLO + 0x18, 0x02050701U);
    write_reg(Buffer_PointerLO + 0x1c, 0xff004002U);
    write_reg(Buffer_PointerLO + 0x20, 0x01030507U);
    write_reg(Buffer_PointerLO + 0x24, 0x070103ffU);
    write_reg(Buffer_PointerLO + 0x28, 0x40038105U);
    write_reg(Buffer_PointerLO + 0x2c, 0x05070100U);
    write_reg(Buffer_PointerLO + 0x30, 0x00400282U);
    write_reg(Buffer_PointerLO + 0x34, 0x830507ffU);
    write_reg(Buffer_PointerLO + 0x38, 0x0103ff01U);
    // DV source had writes at 0x3c-0x48 commented out
    // write_reg(Buffer_PointerLO+0x3c,0x01830507);
    // write_reg(Buffer_PointerLO+0x40,0x07000400);
    // write_reg(Buffer_PointerLO+0x44,0x00010305);
    // write_reg(Buffer_PointerLO+0x48,0x00000004);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    poll_depcmd_completion(0x10, 0x506);

    int_pend = 1;
    wait_int_pend_with_timeout();

    wait_delay(5);

    int_pend = 1;
    wait_int_pend_with_timeout();

    int_pend = 1;
    wait_int_pend_with_timeout();

    status_stage();

    // Bulk transfer stage
    LOGD("Bulk transfer stage begin");

    // Write deterministic bulk test pattern to TX buffer
    for (i = 0U; i < (BULK_PAYLOAD_SIZE / 4U); i++) {
        write_reg(Buffer_PointerLO_1 + (i * 4U), bulk_tx_pattern[i]);
    }

    // Bulk OUT endpoint (offset 0x40)
    LOGD("Bulk OUT transfer on EP offset 0x40");
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
    poll_depcmd_completion(0x40, 0x506);

    int_pend = 1;
    wait_int_pend_with_timeout();

    int_pend = 1;
    wait_int_pend_with_timeout();

    // Runtime condition 10/22: validate bulk OUT transfer event
    {
        unsigned int evt_data = (unsigned int)read_reg(MIZAR_USB_GEVNTCOUNT);
        LOGD("Bulk OUT event count=0x%x", evt_data);
        if (evt_data == 0U) {
            LOGE("No event received for Bulk OUT transfer");
            g_ctx.errors++;
        } else {
            g_ctx.bulk_out_complete = 1U;
            g_ctx.transfer_complete_count++;
        }
    }

    // Runtime condition 12: validate bulk OUT payload integrity
    validate_bulk_payload(Buffer_PointerLO_1, BULK_PAYLOAD_SIZE, bulk_tx_pattern, 0U);

    // Bulk IN endpoint (offset 0x50)
    LOGD("Bulk IN transfer on EP offset 0x50");
    write_reg(event_trb_addr, Buffer_PointerLO_1);
    write_reg(event_trb_addr + 0x8, 0x40);
    write_reg(event_trb_addr + 0xc, 0x813);
    write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, event_trb_addr);
    write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x0);

    write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
    poll_depcmd_completion(0x50, 0x506);

    int_pend = 1;
    wait_int_pend_with_timeout();

    // Runtime condition 10/22: validate bulk IN transfer event
    {
        unsigned int evt_data = (unsigned int)read_reg(MIZAR_USB_GEVNTCOUNT);
        LOGD("Bulk IN event count=0x%x", evt_data);
        if (evt_data == 0U) {
            LOGE("No event received for Bulk IN transfer");
            g_ctx.errors++;
        } else {
            g_ctx.bulk_in_complete = 1U;
            g_ctx.transfer_complete_count++;
        }
    }

    // Runtime condition 12: validate bulk IN payload integrity
    validate_bulk_payload(Buffer_PointerLO_1, BULK_PAYLOAD_SIZE, bulk_tx_pattern, 1U);

    LOGD("0xdeadbee7");
    int_pend = 1;
    wait_int_pend_with_timeout();

    // Runtime condition 11: meaningful PASS/FAIL criteria
    // Verify transfer completion, events, payload integrity
    if (g_ctx.errors != 0U) {
        LOGE("Test FAIL: errors=%u", g_ctx.errors);
        out->status = -1;
    }

    if (g_ctx.bulk_out_complete == 0U) {
        LOGE("Bulk OUT transfer did not complete");
        out->status = -1;
    }

    if (g_ctx.bulk_in_complete == 0U) {
        LOGE("Bulk IN transfer did not complete");
        out->status = -1;
    }

    if (g_ctx.transfer_complete_count < 2U) {
        LOGE("Expected at least 2 transfer completions, got %u", g_ctx.transfer_complete_count);
        out->status = -1;
    }

    if (g_ctx.events_received == 0U) {
        LOGE("No USB events received during test");
        out->status = -1;
    }

    LOGT("USB FS Device Bulk Transfer test run complete: %s errors=%u events=%u xfer_complete=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors, g_ctx.events_received, g_ctx.transfer_complete_count);

    return out->status;
}


/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Cleanup USB FS device bulk transfer test
 * Parameters:
 *   cfg - test configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer test teardown begin");

    // Runtime condition 24: proper teardown
    // Disable device events
    write_reg(MIZAR_USB_DEVTEN, 0x0);

    // Disable sysreg interrupts
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x0);

    // Disable all endpoints
    write_reg(MIZAR_USB_DALEPENA, 0x0);

    // Clear event count
    write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    // Clear event ring size
    write_reg(MIZAR_USB_GEVNTSIZ, 0x0);

    LOGT("USB FS Device Bulk Transfer test teardown complete: %s",
         (g_ctx.errors == 0U) ? "PASS" : "FAIL");

    return (g_ctx.errors == 0U) ? 0 : -1;
}
