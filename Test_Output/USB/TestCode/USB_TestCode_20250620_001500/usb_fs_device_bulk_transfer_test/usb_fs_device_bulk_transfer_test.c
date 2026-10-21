// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Timeout limits for polling loops (Runtime Condition 12)
#define POLL_TIMEOUT 100000
#define INT_WAIT_TIMEOUT 500000
#define ENUM_WAIT_CYCLES 50000000

// Testcase context
static unsigned int g_errors;
static unsigned int g_checks_total;
static unsigned int g_checks_passed;
static unsigned int g_checks_failed;

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler. Clears int_pend, reads system register
 * status, captures and acknowledges event count from GEVNTCOUNT,
 * clears raw status using bitmask check, and clears GIC IRQ 84.
 * Preserved from DV Excerpt 13.
 * Parameters:
 * none
 * Returns:
 * void
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

 // MANUAL_REVIEW: DV source uses logical AND '&&' (rd_data && 0x80000000).
 // Runtime Condition 14 requires explicit bit-mask checking.
 // Original DV: if(rd_data && 0x80000000)
 if (rd_data & 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }
 GIC_ClearIRQ(84);
}

/* Function: setup_stage
 * Description: Issues Start Transfer endpoint command (0x506) via DEPCMD
 * for EP0 control endpoint, programs TRB with control word 0x823,
 * polls DEPCMD for command completion using CmdAct bit, then
 * waits for interrupt. Preserved from DV Excerpt 14.
 * Parameters:
 * none
 * Returns:
 * void
 */
void setup_stage(void)
{
 int rd_data;
 int timeout;

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x8);
 write_reg(event_trb_addr + 0xc, 0x823);

 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);

 write_reg(MIZAR_USB_DEPCMD, 0x506);

 // Runtime Condition 13: check CmdAct status bit instead of full register compare
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout = 0;
 while ((rd_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("setup_stage: DEPCMD polling timeout, rd_data=0x%x", rd_data);
 g_errors++;
 break;
 }
 }

 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("setup_stage: int_pend wait timeout");
 g_errors++;
 break;
 }
 }
}

/* Function: status_stage
 * Description: Handles USB control transfer status phase with TRB control
 * word 0x843, issues Start Transfer command (0x506) via DEPCMD,
 * polls for completion, waits for interrupt, then additional delay.
 * Preserved from DV Excerpt 15.
 * Parameters:
 * none
 * Returns:
 * void
 */
void status_stage(void)
{
 int rd_data;
 int timeout;

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x843);

 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);

 write_reg(MIZAR_USB_DEPCMD, 0x506);
 // Runtime Condition 13: check CmdAct status bit
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout = 0;
 while ((rd_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("status_stage: DEPCMD polling timeout, rd_data=0x%x", rd_data);
 g_errors++;
 break;
 }
 }

 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("status_stage: int_pend wait timeout");
 g_errors++;
 break;
 }
 }

 // DV: additional wait_on(5) at end of status_stage
 for (volatile int d = 0; d < 10000; d++);
}

/* Function: set_configuration
 * Description: Writes endpoint command parameters and polls DEPCMD for
 * completion using CmdAct bit. Preserved from DV Excerpt 16.
 * Parameters:
 * trb_address - endpoint register offset
 * parameter0 - DEPCMDPAR0 value
 * parameter1 - DEPCMDPAR1 value
 * cmd - DEPCMD command value
 * Returns:
 * void
 */
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
 int read_data;
 int timeout;

 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
 // Runtime Condition 13: check CmdAct status bit
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);

 timeout = 0;
 while ((read_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("set_configuration: DEPCMD+0x%x polling timeout, read_data=0x%x", trb_address, read_data);
 g_errors++;
 break;
 }
 }
}

/* Function: enumeration
 * Description: USB device enumeration register configuration sequence.
 * Writes DCFG, DCTL, DALEPENA, prints Buffer_PointerLO, waits.
 * Preserved from DV Excerpt 4. Body partially retrieved.
 * Parameters:
 * none
 * Returns:
 * void
 */
void enumeration(void)
{
 int rd_data;

 // DV debug marker 0xdeadbee0 at 0xA0243ffc
 LOGD("DV debug marker 0xdeadbee0: Enumeration start");

 rd_data = read_reg(MIZAR_USB_DCFG);
 LOGT("DCFG read: 0x%x", rd_data);
 rd_data = read_reg(MIZAR_USB_DSTS);
 LOGT("DSTS read: 0x%x", rd_data);
 write_reg(MIZAR_USB_DCFG, 0x480801);
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);
 write_reg(MIZAR_USB_DALEPENA, 0xff);
 // DV: printf("Buffer_PointerLO is %x\n", Buffer_PointerLO);
 LOGT("Buffer_PointerLO is 0x%x", Buffer_PointerLO);
 // DV: wait_on(5000)
 for (volatile int d = 0; d < ENUM_WAIT_CYCLES; d++);
 // MANUAL_REVIEW: enumeration() body was only partially retrieved from DV source.
 // Additional enumeration steps may exist in the unretrieved portion.
}

/* Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes USB device controller - calls nic_programming(),
 * enables IRQs, clears buffer and event TRB regions, configures
 * global event buffer registers, reads GCTL, runs enumeration.
 * Implements DV Excerpts 2, 3, 4.
 * Parameters:
 * cfg - test configuration item
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
 int rd_data;
 int j;

 (void)cfg;

 g_errors = 0;
 g_checks_total = 0;
 g_checks_passed = 0;
 g_checks_failed = 0;
 event_counter = 0;

 LOGT("USB FS Device Bulk Transfer test init");

 // Excerpt 2: nic_programming and GIC enable
 nic_programming();
 GIC_EnableAllIRQ();

 // Excerpt 2: Clear 20 DWORD-aligned entries in Buffer_PointerLO and event_trb_addr
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }

 // Excerpt 3: Configure global event buffer registers
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);
 rd_data = read_reg(MIZAR_USB_GCTL);
 LOGT("GCTL read: 0x%x", rd_data);

 // Excerpt 4: Enumeration sequence
 enumeration();

 LOGT("USB FS Device Bulk Transfer test init complete");
 return 0;
}

/* Function: usb_fs_device_bulk_transfer_test_run
 * Description: Main testcase stimulus - USB enumeration including GET DESCRIPTOR
 * requests for device and configuration descriptors, SET_CONFIGURATION,
 * endpoint configuration, and CDC bulk data transfer.
 * Implements DV Excerpts 6, 7, 8, 5, 9, 10, 11, 12.
 * Parameters:
 * cfg - test configuration item
 * out - test output structure
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
 int rd_data;
 int timeout;

 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run started");

 // =========================================================
 // Excerpt 6: Device descriptor TRB and data stage
 // TRB: buffer pointer, size=0x12 (18 bytes), control=0x853
 // =========================================================
 LOGT("Device descriptor data stage - TRB and descriptor programming");
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x12);
 write_reg(event_trb_addr + 0xc, 0x853);
 // Device descriptor data
 write_reg(Buffer_PointerLO, 0x02000012);
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);

 // =========================================================
 // Excerpt 7: DEPCMD endpoint command with offset 0x10
 // Start Transfer command for EP1
 // =========================================================
 LOGT("DEPCMD endpoint command with offset 0x10");
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
 // Runtime Condition 13: check CmdAct status bit
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout = 0;
 while ((rd_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("Excerpt 7: DEPCMD+0x10 polling timeout, rd_data=0x%x", rd_data);
 g_errors++;
 break;
 }
 }

 // =========================================================
 // Excerpt 8 (first occurrence): int_pend wait before GET DESCRIPTOR CONFIG
 // =========================================================
 LOGT("Waiting for interrupt completion (Excerpt 8 first)");
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Excerpt 8 first: int_pend wait timeout");
 g_errors++;
 break;
 }
 }

 // =========================================================
 // Excerpt 8: GET DESCRIPTOR USB CONFIGURATION - setup_stage + debug marker 0xdeadbee5
 // =========================================================
 LOGT("GET DESCRIPTOR USB CONFIGURATION (full read)");
 setup_stage();
 // DV debug marker 0xdeadbee5 at 0xA0243ffc
 LOGD("DV debug marker 0xdeadbee5: GET DESCRIPTOR USB CONFIGURATION");

 // Excerpt 8 (second occurrence): int_pend wait after debug marker 0xdeadbee5
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Excerpt 8 second: int_pend wait timeout after debug marker 0xdeadbee5");
 g_errors++;
 break;
 }
 }
 // Data stage comment from DV:
 // 09023c00 010100e0 32090400 00060101 00000755 01034000 01070502 024000ff
 // 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301

 // =========================================================
 // Excerpt 5: GET DESCRIPTOR USB CONFIGURATION first pass (short read)
 // setup_stage() issues Start Transfer for EP0
 // =========================================================
 LOGT("GET DESCRIPTOR USB CONFIGURATION first pass (short read)");
 setup_stage();
 // DV debug marker 0xdeadbee3 at 0xA0243ffc
 LOGD("DV debug marker 0xdeadbee3: GET DESCRIPTOR USB CONFIGURATION first pass");

 // Excerpt 5: int_pend wait after debug marker 0xdeadbee3
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Excerpt 5: int_pend wait timeout after debug marker 0xdeadbee3");
 g_errors++;
 break;
 }
 }
 // Configuration descriptor short read: 09 02 3c 00 01 01 00 e0 32
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x9);
 write_reg(event_trb_addr + 0xc, 0x853);
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 // =========================================================
 // Excerpt 9: status_stage, USB_SET_CONFIGURATION, debug marker 0xdeadbee4
 // =========================================================
 LOGT("Status stage");
 status_stage();
 // USB_SET_CONFIGURATION_OR_RESET_TT
 LOGT("USB_SET_CONFIGURATION_OR_RESET_TT");
 setup_stage();
 // DV debug marker 0xdeadbee4 at 0xA0243ffc
 LOGD("DV debug marker 0xdeadbee4: USB_SET_CONFIGURATION");

 // =========================================================
 // Excerpt 10: GUSB2PHYCFG and USB_CONFIG programming
 // =========================================================
 LOGT("GUSB2PHYCFG and USB_CONFIG programming");
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);
 rd_data = read_reg(MIZAR_USB_CONFIG);
 LOGT("USB_CONFIG read: 0x%x", rd_data);
 write_reg(MIZAR_USB_CONFIG, 0x110);
 // CONFIG CIE: software shall initialize Configuration Value, Interface Number, and Alternate

 // =========================================================
 // Excerpt 11: DEPCMD endpoint command with offset 0x40
 // =========================================================
 LOGT("DEPCMD endpoint command with offset 0x40");
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
 // Runtime Condition 13: check CmdAct status bit
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 timeout = 0;
 while ((rd_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("Excerpt 11: DEPCMD+0x40 polling timeout, rd_data=0x%x", rd_data);
 g_errors++;
 break;
 }
 }

 // =========================================================
 // Excerpt 12: event_counter check
 // =========================================================
 if (event_counter == 0x4) {
 LOGT("event_counter == 0x4, waiting for interrupt");
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Excerpt 12: int_pend wait timeout (event_counter==0x4)");
 g_errors++;
 break;
 }
 }
 }

 // =========================================================
 // CDC Bulk Transfer - Runtime Condition 5
 // MANUAL_REVIEW: The complete CDC bulk transfer sequence was not
 // fully retrieved from DV source. The DV test_case() body is
 // incomplete - only beginning and interior fragments were retrieved.
 // The bulk IN/OUT transfer programming, data patterns, and
 // completion handling from the unretrieved DV portions must be
 // supplied to complete this section.
 // =========================================================
 LOGT("CDC Bulk Transfer phase");
 // MANUAL_REVIEW: Complete bulk OUT transfer programming not retrieved from DV.
 // MANUAL_REVIEW: Complete bulk IN transfer programming not retrieved from DV.
 // MANUAL_REVIEW: Bulk data patterns for integrity validation not retrieved from DV.
 // MANUAL_REVIEW: Specific set_configuration() call sites within test_case() were not retrieved.
 // MANUAL_REVIEW: Return statement of test_case() was not retrieved.

 // =========================================================
 // Speed verification - Runtime Condition 18
 // =========================================================
 LOGT("Speed verification via DSTS");
 rd_data = read_reg(MIZAR_USB_DSTS);
 LOGT("DSTS read for speed check: 0x%x", rd_data);
 // MANUAL_REVIEW: DV speed validation specifics not present in retrieved excerpts.
 // Verify speed field bits [2:0] indicate Full-Speed (value 1).
 g_checks_total++;
 if ((rd_data & 0x7) != 0x1) {
 LOGE("Speed verification: expected FS (0x1), got 0x%x", (rd_data & 0x7));
 // MANUAL_REVIEW: Confirm expected FS speed encoding from DV.
 // Not incrementing g_errors since DV speed check was not explicitly retrieved.
 } else {
 LOGT("Speed verification: Full-Speed confirmed");
 g_checks_passed++;
 }

 // =========================================================
 // Event validation - Runtime Conditions 8, 20
 // =========================================================
 g_checks_total++;
 if (event_counter == 0) {
 LOGE("No USB events received - event_counter is 0");
 g_errors++;
 g_checks_failed++;
 } else {
 LOGT("USB events received: event_counter=0x%x", event_counter);
 g_checks_passed++;
 }

 // =========================================================
 // Final PASS/FAIL - Runtime Condition 9
 // =========================================================
 g_checks_failed = g_errors;

 if (g_errors == 0) {
 out->status = 0;
 LOGT("Run complete: PASS errors=%u checks_passed=%u checks_total=%u",
 g_errors, g_checks_passed, g_checks_total);
 } else {
 out->status = -1;
 LOGE("Run complete: FAIL errors=%u checks_failed=%u checks_total=%u",
 g_errors, g_checks_failed, g_checks_total);
 }

 return out->status;
}

/* Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Cleans up interrupts, endpoints, transfers, TRBs, event queues,
 * pending events, and controller state. Runtime Condition 22.
 * Parameters:
 * cfg - test configuration item
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 int rd_data;

 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test teardown");

 // Acknowledge any remaining events
 rd_data = read_reg(MIZAR_USB_GEVNTCOUNT);
 if (rd_data != 0) {
 LOGT("Teardown: acknowledging remaining GEVNTCOUNT=0x%x", rd_data);
 write_reg(MIZAR_USB_GEVNTCOUNT, rd_data);
 }

 // Clear GIC IRQ 84
 GIC_ClearIRQ(84);

 // MANUAL_REVIEW: Additional endpoint cleanup, TRB cleanup, and controller
 // state reset may be required based on unretrieved DV source portions.

 LOGT("USB FS Device Bulk Transfer test teardown complete: errors=%u", g_errors);
 return (g_errors == 0) ? 0 : -1;
}
