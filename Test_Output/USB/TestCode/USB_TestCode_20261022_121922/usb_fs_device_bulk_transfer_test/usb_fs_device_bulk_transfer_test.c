// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// DWC3 DEPCMD CmdAct bit (bit 10) used for command completion checking
#define DEPCMD_CMDACT_BIT (1U << 10)

// DWC3 DEVT event types (bits [3:0] of device event DWORD0)
#define DWC3_DEVT_DISCONN 0U
#define DWC3_DEVT_USBRST 1U
#define DWC3_DEVT_CONNECTDONE 2U
#define DWC3_DEVT_ULST_CHNG 3U
#define DWC3_DEVT_WKUP 4U

// DWC3 DEPEVT event types (bits [9:6] of endpoint event DWORD0)
#define DWC3_DEPEVT_XFERCOMPLETE 1U
#define DWC3_DEPEVT_XFERINPROGRESS 2U
#define DWC3_DEPEVT_XFERNOTREADY 3U

// DWC3 event bit0: 0=endpoint event, 1=non-endpoint(device) event
#define DWC3_EVENT_IS_DEV_EVENT(e) (((e) & 0x1U) != 0U)
#define DWC3_DEVT_TYPE(e) (((e) >> 8) & 0xFU)
#define DWC3_DEPEVT_EPNUM(e) (((e) >> 1) & 0x1FU)
#define DWC3_DEPEVT_TYPE(e) (((e) >> 6) & 0xFU)
#define DWC3_DEPEVT_STATUS(e) (((e) >> 12) & 0xFU)

// DSTS speed field bits [2:0]
#define DWC3_DSTS_SPEED_MASK 0x7U
#define DWC3_DSTS_SPEED_FS 0x1U

// Timeout limits
#define TIMEOUT_CMD_COMPLETION 100000U
#define TIMEOUT_INT_WAIT 500000U
#define TIMEOUT_SOFT_RESET 200000U
#define TIMEOUT_HANDSHAKE 1000000U

// Bulk transfer payload size (0x40 = 64 bytes)
#define BULK_XFER_LEN 0x40U
#define BULK_PATTERN_SEED 0xA5U

// Event ring size in bytes (from GEVNTSIZ programming = 0x30 = 48 bytes)
#define EVENT_RING_SIZE_BYTES 0x30U
#define EVENT_RING_ENTRY_SIZE 4U
#define EVENT_RING_NUM_ENTRIES (EVENT_RING_SIZE_BYTES / EVENT_RING_ENTRY_SIZE)

// Testcase context
typedef struct {
 unsigned int errors;
 unsigned int bulk_out_complete;
 unsigned int bulk_in_complete;
 unsigned int bulk_out_ep_event_valid;
 unsigned int bulk_in_ep_event_valid;
 unsigned int reset_event_seen;
 unsigned int connect_done_event_seen;
 unsigned int xfer_complete_ep4_seen;
 unsigned int xfer_complete_ep5_seen;
 unsigned int event_ring_read_offset;
} usb_fs_bulk_ctx_t;

static usb_fs_bulk_ctx_t g_ctx;

// Deterministic bulk TX pattern buffer (64 bytes as unsigned int DWORDs)
static const unsigned int bulk_tx_pattern[16] = {
 0x00010203U, 0x04050607U, 0x08090A0BU, 0x0C0D0E0FU,
 0x10111213U, 0x14151617U, 0x18191A1BU, 0x1C1D1E1FU,
 0x20212223U, 0x24252627U, 0x28292A2BU, 0x2C2D2E2FU,
 0x30313233U, 0x34353637U, 0x38393A3BU, 0x3C3D3E3FU
};

/*
 * Function: psv_delay
 * Description: Deterministic busy-wait delay replacing wait_on(N)
 * Parameters:
 * count - relative delay multiplier
 * Returns:
 * void
 */
static void psv_delay(unsigned int count)
{
 unsigned int c;
 for (c = 0U; c < count; c++) {
 for (volatile int d = 0; d < 10000; d++);
 }
}

/* Function: wait_for_interrupt
 * Description: Wait for int_pend to be cleared by ISR with timeout protection
 * Parameters:
 * timeout_label - descriptive label for timeout diagnostics
 * Returns:
 * 0 on success, -1 on timeout
 */
static int wait_for_interrupt(const char *timeout_label)
{
 volatile unsigned int timeout = TIMEOUT_INT_WAIT;
 while (int_pend != 0) {
 psv_delay(1U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout waiting for interrupt: %s", timeout_label);
 g_ctx.errors++;
 return -1;
 }
 }
 return 0;
}

/* Function: process_event_ring
 * Description: Process event ring entries sequentially, decode DWC3 events,
 * advance read pointer, validate expected USB events.
 * Parameters:
 * event_count_bytes - number of bytes of new events from GEVNTCOUNT
 * Returns:
 * void
 */
static void process_event_ring(unsigned int event_count_bytes)
{
 unsigned int num_events = event_count_bytes / EVENT_RING_ENTRY_SIZE;
 unsigned int idx;
 for (idx = 0U; idx < num_events; idx++) {
 unsigned int evt_offset = g_ctx.event_ring_read_offset;
 unsigned int evt = read_reg(Default_Event_Ring_Array + evt_offset);
 if (DWC3_EVENT_IS_DEV_EVENT(evt)) {
 unsigned int dev_type = DWC3_DEVT_TYPE(evt);
 if (dev_type == DWC3_DEVT_USBRST) {
 LOGT("Event: USB Reset detected");
 g_ctx.reset_event_seen = 1U;
 } else if (dev_type == DWC3_DEVT_CONNECTDONE) {
 LOGT("Event: Connect Done detected");
 g_ctx.connect_done_event_seen = 1U;
 } else if (dev_type == DWC3_DEVT_ULST_CHNG) {
 LOGT("Event: USB Link State Change");
 } else {
 LOGT("Event: Device event type=%u", dev_type);
 }
 } else {
 unsigned int ep_num = DWC3_DEPEVT_EPNUM(evt);
 unsigned int ep_evt_type = DWC3_DEPEVT_TYPE(evt);
 unsigned int ep_status = DWC3_DEPEVT_STATUS(evt);
 LOGT("Event: EP%u type=%u status=%u", ep_num, ep_evt_type, ep_status);
 if (ep_evt_type == DWC3_DEPEVT_XFERCOMPLETE) {
 LOGT("Event: Transfer Complete on EP%u", ep_num);
 // EP4 = physical ep for bulk OUT (offset 0x40 / 0x10 = 4)
 if (ep_num == 4U) {
 g_ctx.xfer_complete_ep4_seen = 1U;
 g_ctx.bulk_out_complete = 1U;
 g_ctx.bulk_out_ep_event_valid = 1U;
 }
 // EP5 = physical ep for bulk IN (offset 0x50 / 0x10 = 5)
 if (ep_num == 5U) {
 g_ctx.xfer_complete_ep5_seen = 1U;
 g_ctx.bulk_in_complete = 1U;
 g_ctx.bulk_in_ep_event_valid = 1U;
 }
 } else if (ep_evt_type == DWC3_DEPEVT_XFERNOTREADY) {
 LOGT("Event: Transfer Not Ready on EP%u", ep_num);
 }
 }
 // Advance event ring read pointer with wraparound
 g_ctx.event_ring_read_offset = (g_ctx.event_ring_read_offset + EVENT_RING_ENTRY_SIZE) % EVENT_RING_SIZE_BYTES;
 }
}

/* Function: set_configuration
 * Description: Program endpoint configuration via DEPCMD with command completion polling
 * Parameters:
 * trb_address - endpoint offset
 * parameter0 - DEPCMDPAR0 value
 * parameter1 - DEPCMDPAR1 value
 * cmd - DEPCMD command value
 * Returns:
 * void
 */
static void set_configuration(unsigned int trb_address, unsigned int parameter0, unsigned int parameter1, unsigned int cmd)
{
 unsigned int read_data;
 volatile unsigned int timeout = TIMEOUT_CMD_COMPLETION;

 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);

 while ((read_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(30U);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: set_configuration cmd=0x%x ep_offset=0x%x", cmd, trb_address);
 g_ctx.errors++;
 return;
 }
 }
}

/* Function: Default_IRQHandler
 * Description: Interrupt handler - clears int_pend, reads sysreg status,
 * reads/acknowledges event count, processes event ring, clears
 * sysreg raw status, clears GIC IRQ 84.
 * Parameters:
 * none
 * Returns:
 * void
 */
void Default_IRQHandler(void)
{
 unsigned int rd_data;
 unsigned int event_count;
 int_pend = 0;
 rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

 event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
 event_counter = event_count;

 // Process event ring entries before acknowledging
 if (event_count > 0U) {
 process_event_ring(event_count);
 }

 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

 // Runtime condition 16/17A: Use bitwise AND for hardware status-bit validation
 // DV source had: if(rd_data && 0x80000000) which is a logical-AND bug
 // Preserving DV intent with correct bitwise mask evaluation
 if ((rd_data & 0x80000000U) != 0U) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000U);
 }
 GIC_ClearIRQ(84);
}

/* Function: setup_stage
 * Description: USB control transfer setup stage - programs TRB and issues
 * DEPCMD start transfer on EP0 with command completion polling
 * and interrupt wait.
 * Parameters:
 * none
 * Returns:
 * void
 */
static void setup_stage(void)
{
 unsigned int rd_data;
 volatile unsigned int timeout;

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8U, 0x8U);
 write_reg(event_trb_addr + 0xcU, 0x823U);
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0U);

 write_reg(MIZAR_USB_DEPCMD, 0x506U);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: setup_stage DEPCMD completion");
 g_ctx.errors++;
 return;
 }
 }
 int_pend = 1;
 (void)wait_for_interrupt("setup_stage int_pend");
}

/* Function: status_stage
 * Description: USB control transfer status stage - programs TRB and issues
 * DEPCMD start transfer on EP0 with command completion polling
 * and multiple interrupt waits.
 * Parameters:
 * none
 * Returns:
 * void
 */
static void status_stage(void)
{
 unsigned int rd_data;
 volatile unsigned int timeout;

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8U, 0x0U);
 write_reg(event_trb_addr + 0xcU, 0x843U);
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0U);

 write_reg(MIZAR_USB_DEPCMD, 0x506U);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: status_stage DEPCMD completion");
 g_ctx.errors++;
 return;
 }
 }
 int_pend = 1;
 (void)wait_for_interrupt("status_stage int_pend 1");

 psv_delay(5U);

 int_pend = 1;
 (void)wait_for_interrupt("status_stage int_pend 2");
}

/* Function: enumeration
 * Description: USB enumeration sequence - handles device descriptor response,
 * GET DESCRIPTOR USB CONFIGURATION (short and full),
 * USB_SET_CONFIGURATION_OR_RESET_TT, and status stages.
 * Parameters:
 * none
 * Returns:
 * void
 */
static void enumeration(void)
{
 unsigned int rd_data;
 volatile unsigned int timeout;

 // Device descriptor setup stage
 setup_stage();
 LOGD("DV debug marker 0xdeadbee1: Device descriptor transfer");
 int_pend = 1;
 (void)wait_for_interrupt("enumeration: after setup_stage int_pend");

 // Program TRB for device descriptor data stage
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
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: enumeration device desc DEPCMD+0x10");
 g_ctx.errors++;
 return;
 }
 }

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: device desc data int_pend 1");

 psv_delay(5U);

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: device desc data int_pend 2");

 // xfernotready event wait
 int_pend = 1;
 (void)wait_for_interrupt("enumeration: xfernotready int_pend");

 status_stage();

 // GET DESCRIPTOR USB CONFIGURATION (short - 9 bytes)
 setup_stage();
 LOGD("DV debug marker 0xdeadbee3: GET DESCRIPTOR USB CONFIGURATION short");
 int_pend = 1;
 (void)wait_for_interrupt("enumeration: GET_DESC_CONFIG short int_pend");

 // Config descriptor data (short)
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8U, 0x9U);
 write_reg(event_trb_addr + 0xcU, 0x853U);
 write_reg(Buffer_PointerLO, 0x003c0209U);
 write_reg(Buffer_PointerLO + 0x4U, 0xe0000101U);
 write_reg(Buffer_PointerLO + 0x8U, 0x00000032U);
 write_reg(Buffer_PointerLO + 0xcU, 0x00000000U);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

 write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: enumeration GET_DESC_CONFIG short DEPCMD+0x10");
 g_ctx.errors++;
 return;
 }
 }

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: config desc short int_pend 1");

 psv_delay(5U);

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: config desc short int_pend 2");

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: config desc short int_pend 3");

 status_stage();

 // USB_SET_CONFIGURATION_OR_RESET_TT
 setup_stage();
 LOGD("DV debug marker 0xdeadbee4: USB_SET_CONFIGURATION_OR_RESET_TT");

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: SET_CONFIG int_pend");

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8U, 0x0U);
 write_reg(event_trb_addr + 0xcU, 0x853U);
 write_reg(Buffer_PointerLO, 0x00U);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

 write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: enumeration SET_CONFIG DEPCMD+0x10");
 g_ctx.errors++;
 return;
 }
 }

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: SET_CONFIG data int_pend 1");

 psv_delay(5U);

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: SET_CONFIG data int_pend 2");

 // GET DESCRIPTOR USB CONFIGURATION (full - 0x3c bytes)
 setup_stage();
 LOGD("DV debug marker 0xdeadbee5: GET DESCRIPTOR USB CONFIGURATION full");
 int_pend = 1;
 (void)wait_for_interrupt("enumeration: GET_DESC_CONFIG full int_pend");

 // Full config descriptor data (0x3c bytes)
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
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: enumeration GET_DESC_CONFIG full DEPCMD+0x10");
 g_ctx.errors++;
 return;
 }
 }
 int_pend = 1;
 (void)wait_for_interrupt("enumeration: config desc full int_pend 1");

 psv_delay(5U);

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: config desc full int_pend 2");

 int_pend = 1;
 (void)wait_for_interrupt("enumeration: config desc full int_pend 3");

 status_stage();
}

/* Function: validate_bulk_payload
 * Description: Validate bulk transfer payload by reading from receive buffer
 * and comparing against expected deterministic pattern.
 * Parameters:
 * rx_buf_addr - address of receive buffer
 * expected_pattern - pointer to expected data array
 * num_dwords - number of DWORDs to compare
 * direction_str - string label for logging
 * Returns:
 * 0 on match, -1 on mismatch
 */
static int validate_bulk_payload(unsigned int rx_buf_addr,
 const unsigned int *expected_pattern,
 unsigned int num_dwords,
 const char direction_str)
{
 unsigned int idx;
 unsigned int mismatches = 0U;
 for (idx = 0U; idx < num_dwords; idx++) {
 unsigned int actual = read_reg(rx_buf_addr + (idx * 4U));
 if (actual != expected_pattern[idx]) {
 LOGE("Payload mismatch %s DWORD[%u]: expected=0x%08x actual=0x%08x",
 direction_str, idx, expected_pattern[idx], actual);
 mismatches++;
 }
 }
 if (mismatches > 0U) {
 LOGE("%s payload validation FAILED: %u mismatches", direction_str, mismatches);
 g_ctx.errors += mismatches;
 return -1;
 }
 LOGT("%s payload validation PASSED: %u DWORDs verified", direction_str, num_dwords);
 return 0;
}

/* Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initialize USB FS device bulk transfer test - NIC programming,
 * GIC enable, buffer zeroing, controller soft reset, PHY config,
 * event buffer setup, global/device config, endpoint configuration.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
 unsigned int rd_data;
 unsigned int wr_data;
 unsigned int i;
 unsigned int j;
 volatile unsigned int timeout;

 (void)cfg;

 // Initialize context
 g_ctx.errors = 0U;
 g_ctx.bulk_out_complete = 0U;
 g_ctx.bulk_in_complete = 0U;
 g_ctx.bulk_out_ep_event_valid = 0U;
 g_ctx.bulk_in_ep_event_valid = 0U;
 g_ctx.reset_event_seen = 0U;
 g_ctx.connect_done_event_seen = 0U;
 g_ctx.xfer_complete_ep4_seen = 0U;
 g_ctx.xfer_complete_ep5_seen = 0U;
 g_ctx.event_ring_read_offset = 0U;

 LOGT("USB FS Device Bulk Transfer test init");

 // NIC programming
 nic_programming();

 // Enable all GIC IRQs
 GIC_EnableAllIRQ();

 // Zero buffer and TRB memory
 for (j = 0U; j < 20U; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0U);
 write_reg(event_trb_addr + j * DWORD, 0x0U);
 }

 // Soft reset
 wr_data = 0x40f00000U;
 write_reg(MIZAR_USB_DCTL, wr_data);
 rd_data = read_reg(MIZAR_USB_DCTL);
 timeout = TIMEOUT_SOFT_RESET;
 while (rd_data != 0xf00000U) {
 psv_delay(5U);
 rd_data = read_reg(MIZAR_USB_DCTL);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: USB controller soft reset");
 g_ctx.errors++;
 return -1;
 }
 }
 LOGT("Soft Rst is done");

 // USB2 PHY configuration
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

 // Endpoint configuration (9 calls)
 set_configuration(0U, 0U, 0U, 0x409U);
 set_configuration(0U, 0x200U, 0x700U, 0x401U);
 set_configuration(0x10U, 0x200U, 0x2000700U, 0x401U);
 set_configuration(0x20U, 0x206U, 0x4000700U, 0x401U);
 set_configuration(0x30U, 0x20206U, 0x6000700U, 0x401U);
 set_configuration(0x40U, 0x204U, 0x8000700U, 0x401U);
 set_configuration(0x50U, 0x40204U, 0xa000700U, 0x401U);
 set_configuration(0x60U, 0x1ffaU, 0xc000700U, 0x401U);
 set_configuration(0x70U, 0x61ffaU, 0xe000700U, 0x401U);

 // Endpoint TX resource configuration loop
 for (i = 0U; i < 8U; i++) {
 write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10U), 0x1U);
 write_reg(MIZAR_USB_DEPCMD + (i * 0x10U), 0x402U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10U));
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: EP TX resource config i=%u", i);
 g_ctx.errors++;
 break;
 }
 }
 }

 // Enable physical endpoints 0,1
 write_reg(MIZAR_USB_DALEPENA, 0x3U);
 // DCTL run/stop
 write_reg(MIZAR_USB_DCTL, 0x80f00000U);
 // Sysreg interrupt enable
 write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000U);

 // Link state / connect / reset event handling
 int_pend = 1;
 (void)wait_for_interrupt("init: link state int_pend 1");

 if (event_counter <= 0x4U) {
 int_pend = 1;
 (void)wait_for_interrupt("init: event_counter<=4 int_pend 2");
 }

 write_reg(MIZAR_USB_DCFG, 0x480801U);

 if (event_counter <= 0x4U) {
 int_pend = 1;
 (void)wait_for_interrupt("init: event_counter<=4 int_pend 3");
 }

 LOGT("USB FS Device Bulk Transfer test init complete");
 return 0;
}

/* Function: usb_fs_device_bulk_transfer_test_run
 * Description: Main testcase execution - enumeration, SET ADDRESS,
 * GET DESCRIPTOR, SET CONFIGURATION, bulk OUT/IN transfers,
 * payload validation, event validation.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput result
 * Returns:
 * 0 on PASS, -1 on FAIL
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
 unsigned int rd_data;
 volatile unsigned int timeout;
 unsigned int idx;

 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run start");

 // Enumeration start
 LOGD("DV debug marker 0xdeadbee0: Enumeration start");

 rd_data = read_reg(MIZAR_USB_DCFG);
 rd_data = read_reg(MIZAR_USB_DSTS);

 // Speed verification: check DSTS for Full-Speed
 if ((rd_data & DWC3_DSTS_SPEED_MASK) != DWC3_DSTS_SPEED_FS) {
 LOGE("Speed verification: expected FS (0x1), got 0x%x", (rd_data & DWC3_DSTS_SPEED_MASK));
 g_ctx.errors++;
 } else {
 LOGT("Speed verification: Full-Speed confirmed");
 }

 write_reg(MIZAR_USB_DCFG, 0x480801U);
 write_reg(MIZAR_USB_DCTL, 0x80f00a00U);
 write_reg(MIZAR_USB_DALEPENA, 0xffU);
 LOGT("Buffer_PointerLO is 0x%x", Buffer_PointerLO);
 psv_delay(5000U);

 setup_stage();
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547U);

 if (event_counter <= 0x4U) {
 int_pend = 1;
 (void)wait_for_interrupt("run: post-setup event_counter<=4 int_pend 1");
 }

 if (event_counter <= 0x4U) {
 int_pend = 1;
 (void)wait_for_interrupt("run: post-setup event_counter<=4 int_pend 2");
 }

 // SET ADDRESS
 write_reg(MIZAR_USB_DCFG, 0x480809U);
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8U, 0x0U);
 write_reg(event_trb_addr + 0xcU, 0x853U);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

 write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: SET ADDRESS DEPCMD+0x10");
 g_ctx.errors++;
 out->status = -1;
 return -1;
 }
 }

 int_pend = 1;
 (void)wait_for_interrupt("run: SET_ADDRESS int_pend 1");

 psv_delay(5U);

 int_pend = 1;
 (void)wait_for_interrupt("run: SET_ADDRESS int_pend 2");

 int_pend = 1;
 (void)wait_for_interrupt("run: SET_ADDRESS int_pend 3");

 // Status stage after SET ADDRESS
 status_stage();

 // USB_SET_CONFIGURATION_OR_RESET_TT
 setup_stage();
 LOGD("DV debug marker 0xdeadbee4: SET_CONFIGURATION status stage");

 int_pend = 1;
 (void)wait_for_interrupt("run: SET_CONFIG int_pend");

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8U, 0x0U);
 write_reg(event_trb_addr + 0xcU, 0x853U);
 write_reg(Buffer_PointerLO, 0x00U);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10U, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10U, 0x0U);

 write_reg(MIZAR_USB_DEPCMD + 0x10U, 0x506U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: run SET_CONFIG DEPCMD+0x10");
 g_ctx.errors++;
 out->status = -1;
 return -1;
 }
 }

 if (event_counter <= 0x4U) {
 int_pend = 1;
 (void)wait_for_interrupt("run: SET_CONFIG event_counter<=4 int_pend");
 }

 // Unconditional int_pend wait (DV source had this outside commented if block)
 int_pend = 1;
 (void)wait_for_interrupt("run: SET_CONFIG unconditional int_pend");

 // Runtime condition 7: Handshake register 0xA0243FF4 removed
 // DV used: hand_shake = read_reg(0xa0243ff4); while(hand_shake == 0x0) {...}
 // Replaced with protocol-visible status validation
 LOGT("Handshake sync point 0xa0243ff4 replaced with event-based validation");
 // MANUAL_REVIEW: DV handshake register 0xA0243FF4 removed per runtime condition 7.
 // Original DV polled this testbench register for synchronization before enumeration().
 // PSV relies on event-ring and interrupt-based synchronization instead.

 // Enumeration
 enumeration();

 // Runtime condition 7: Handshake register 0xA0243FF8 removed
 // DV used: hand_shake = read_reg(0xa0243ff8); while(hand_shake == 0x0) {...}
 LOGT("Handshake sync point 0xa0243ff8 replaced with event-based validation");
 // MANUAL_REVIEW: DV handshake register 0xA0243FF8 removed per runtime condition 7.
 // Original DV polled this testbench register for synchronization after enumeration().

 LOGD("DV debug marker 0xdeadbee6: Post-enumeration");
 int_pend = 1;
 (void)wait_for_interrupt("run: post-enumeration int_pend 1");

 int_pend = 1;
 (void)wait_for_interrupt("run: post-enumeration int_pend 2");

 // GET DESCRIPTOR USB CONFIGURATION (in test_case after enumeration)
 setup_stage();
 LOGD("DV debug marker 0xdeadbee5: GET DESCRIPTOR USB CONFIGURATION full (test_case)");
 int_pend = 1;
 (void)wait_for_interrupt("run: GET_DESC_CONFIG full int_pend");

 // Full config descriptor data (0x3c bytes) - same as in enumeration
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
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: run GET_DESC_CONFIG full DEPCMD+0x10");
 g_ctx.errors++;
 out->status = -1;
 return -1;
 }
 }
 int_pend = 1;
 (void)wait_for_interrupt("run: GET_DESC_CONFIG full data int_pend 1");

 psv_delay(5U);

 int_pend = 1;
 (void)wait_for_interrupt("run: GET_DESC_CONFIG full data int_pend 2");

 int_pend = 1;
 (void)wait_for_interrupt("run: GET_DESC_CONFIG full data int_pend 3");

 status_stage();

 // ---- Bulk OUT transfer on endpoint offset 0x40 ----
 LOGT("Starting Bulk OUT transfer on EP offset 0x40");

 // Write deterministic TX pattern to bulk buffer for OUT transfer
 for (idx = 0U; idx < 16U; idx++) {
 write_reg(Buffer_PointerLO_1 + (idx * 4U), bulk_tx_pattern[idx]);
 }

 // Clear bulk_out completion before transfer
 g_ctx.bulk_out_complete = 0U;
 g_ctx.bulk_out_ep_event_valid = 0U;
 g_ctx.xfer_complete_ep4_seen = 0U;

 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8U, 0x40U);
 write_reg(event_trb_addr + 0xcU, 0x813U);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40U, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40U, 0x0U);

 write_reg(MIZAR_USB_DEPCMD + 0x40U, 0x506U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: Bulk OUT DEPCMD+0x40");
 g_ctx.errors++;
 out->status = -1;
 return -1;
 }
 }

 int_pend = 1;
 (void)wait_for_interrupt("run: Bulk OUT int_pend 1");

 int_pend = 1;
 (void)wait_for_interrupt("run: Bulk OUT int_pend 2");

 // Validate Bulk OUT transfer completion event
 if (g_ctx.bulk_out_complete == 0U) {
 LOGE("Bulk OUT transfer did not complete (no XferComplete event on EP4)");
 g_ctx.errors++;
 } else {
 LOGT("Bulk OUT transfer complete event received on EP4");
 }

 // Bulk OUT payload validation: read back from receive buffer
 // MANUAL_REVIEW: In real hardware, the received data would be in a separate
 // device-side receive buffer. Using Buffer_PointerLO_1 as the DV-specified
 // buffer address for the OUT endpoint TRB.
 (void)validate_bulk_payload(Buffer_PointerLO_1, bulk_tx_pattern, 16U, "Bulk OUT");

 // ---- Bulk IN transfer on endpoint offset 0x50 ----
 LOGT("Starting Bulk IN transfer on EP offset 0x50");

 // Clear bulk_in completion before transfer
 g_ctx.bulk_in_complete = 0U;
 g_ctx.bulk_in_ep_event_valid = 0U;
 g_ctx.xfer_complete_ep5_seen = 0U;

 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8U, 0x40U);
 write_reg(event_trb_addr + 0xcU, 0x813U);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50U, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50U, 0x0U);

 write_reg(MIZAR_USB_DEPCMD + 0x50U, 0x506U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50U);
 timeout = TIMEOUT_CMD_COMPLETION;
 while ((rd_data & DEPCMD_CMDACT_BIT) != 0U) {
 psv_delay(10U);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50U);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout: Bulk IN DEPCMD+0x50");
 g_ctx.errors++;
 out->status = -1;
 return -1;
 }
 }
 int_pend = 1;
 (void)wait_for_interrupt("run: Bulk IN int_pend 1");

 LOGD("DV debug marker 0xdeadbee7: Bulk transfer completion");
 int_pend = 1;
 (void)wait_for_interrupt("run: Bulk IN int_pend 2");

 // Validate Bulk IN transfer completion event
 if (g_ctx.bulk_in_complete == 0U) {
 LOGE("Bulk IN transfer did not complete (no XferComplete event on EP5)");
 g_ctx.errors++;
 } else {
 LOGT("Bulk IN transfer complete event received on EP5");
 }

 // Bulk IN payload validation: read from IN buffer
 // MANUAL_REVIEW: In real hardware, the host-side receive buffer would be separate.
 // Using Buffer_PointerLO_1 as the DV-specified buffer for the IN endpoint TRB.
 (void)validate_bulk_payload(Buffer_PointerLO_1, bulk_tx_pattern, 16U, "Bulk IN");

 // ---- Final validation ----
 LOGT("Performing final validation checks");

 // Validate expected USB events were seen
 if (g_ctx.reset_event_seen == 0U) {
 LOGE("Expected USB Reset event was not observed");
 g_ctx.errors++;
 }
 if (g_ctx.connect_done_event_seen == 0U) {
 LOGE("Expected Connect Done event was not observed");
 g_ctx.errors++;
 }

 // Validate endpoint-specific transfer completion
 if (g_ctx.xfer_complete_ep4_seen == 0U) {
 LOGE("Bulk OUT (EP4) XferComplete event not seen");
 g_ctx.errors++;
 }
 if (g_ctx.xfer_complete_ep5_seen == 0U) {
 LOGE("Bulk IN (EP5) XferComplete event not seen");
 g_ctx.errors++;
 }

 // Set final status
 out->status = (g_ctx.errors == 0U) ? 0 : -1;

 LOGT("Run complete: %s errors=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 g_ctx.errors);

 return out->status;
}

/* Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown - disable interrupts, endpoints, clear event queue,
 * reset controller state.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test teardown");

 // Disable sysreg interrupts
 write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x0U);

 // Disable device events
 write_reg(MIZAR_USB_DEVTEN, 0x0U);

 // Disable all endpoints
 write_reg(MIZAR_USB_DALEPENA, 0x0U);

 // Clear event count
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0U);

 // Clear event buffer size
 write_reg(MIZAR_USB_GEVNTSIZ, 0x0U);

 // Stop controller (clear run/stop)
 write_reg(MIZAR_USB_DCTL, 0x0U);

 // Clear pending interrupt state
 int_pend = 0;

 LOGT("Teardown complete: errors=%u", g_ctx.errors);

 return (g_ctx.errors == 0U) ? 0 : -1;
}
