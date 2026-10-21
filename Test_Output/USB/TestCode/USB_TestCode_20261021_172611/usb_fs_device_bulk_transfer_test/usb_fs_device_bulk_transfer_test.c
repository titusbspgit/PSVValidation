// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Timeout limits for polling loops (Runtime Condition 12)
#define POLL_TIMEOUT 100000
#define INT_WAIT_TIMEOUT 500000
#define SOFT_RST_TIMEOUT 200000

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
 * Preserved from DV Meta Test Steps.
 * Parameters:
 * none
 * Returns:
 * void
 /
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

/
 * Function: setup_stage
 * Description: Issues Start Transfer endpoint command (0x506) via DEPCMD
 * for EP0 control endpoint, programs TRB with control word 0x823,
 * polls DEPCMD for command completion using CmdAct bit, then
 * waits for interrupt.
 * Parameters:
 * none
 * Returns:
 * void
 /
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

/
 * Function: status_stage
 * Description: Handles USB control transfer status phase with TRB control
 * word 0x843, issues Start Transfer command (0x506) via DEPCMD,
 * polls for completion, waits for interrupt.
 * Parameters:
 * none
 * Returns:
 * void
 /
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
}

/
 * Function: set_configuration
 * Description: Writes endpoint command parameters and polls DEPCMD for
 * completion using CmdAct bit.
 * Parameters:
 * trb_address - endpoint register offset
 * parameter0 - DEPCMDPAR0 value
 * parameter1 - DEPCMDPAR1 value
 * cmd - DEPCMD command value
 * Returns:
 * void
 /
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

/
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes USB device controller - calls nic_programming(),
 * enables IRQs, clears buffer and event TRB regions, performs
 * soft reset, configures PHY, event buffer, GCTL, DCFG, starts
 * new configuration, configures 8 endpoint TX resources, enables
 * physical endpoints, configures DCTL.
 * Implements Meta Test Steps 7-43.
 * Parameters:
 * cfg - test configuration item
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 int rd_data;
 int wr_data;
 int i;
 int j;
 int timeout;

 (void)cfg;

 g_errors = 0;
 g_checks_total = 0;
 g_checks_passed = 0;
 g_checks_failed = 0;
 event_counter = 0;

 LOGT("USB FS Device Bulk Transfer test init");

 // Step 7: nic_programming()
 nic_programming();

 // Step 8: GIC_EnableAllIRQ()
 GIC_EnableAllIRQ();

 // Steps 9-12: Clear 20 DWORD-aligned entries in Buffer_PointerLO and event_trb_addr
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }

 // Steps 13-14: Soft reset
 wr_data = 0x40f00000;
 write_reg(MIZAR_USB_DCTL, wr_data);

 // Steps 15-19: Poll until soft reset completes
 rd_data = read_reg(MIZAR_USB_DCTL);
 timeout = 0;
 while (rd_data != 0xf00000) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DCTL);
 timeout++;
 if (timeout >= SOFT_RST_TIMEOUT) {
 LOGE("Soft reset timeout, rd_data=0x%x", rd_data);
 g_errors++;
 break;
 }
 }

 // Step 20: printf("Soft Rst is done\n")
 LOGT("Soft Rst is done");

 // Steps 21-22: Configure USB 2.0 PHY
 wr_data = 0x40002407;
 write_reg(MIZAR_USB_GUSB2PHYCFG, wr_data);

 // Steps 23-26: Configure global event buffer registers
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

 // Steps 27-29: Read and configure GCTL for port direction
 rd_data = read_reg(MIZAR_USB_GCTL);
 LOGT("GCTL read: 0x%x", rd_data);
 wr_data = 0x30c12214;
 write_reg(MIZAR_USB_GCTL, wr_data);

 // Steps 30-31: Configure DCFG
 rd_data = read_reg(MIZAR_USB_DCFG);
 LOGT("DCFG read: 0x%x", rd_data);
 write_reg(MIZAR_USB_DCFG, 0x480801);

 // Step 32: Start new configuration
 set_configuration(0, 0, 0, 0x409);

 // Steps 33-41: Configure TX resources for 8 endpoints
 for (i = 0; i < 8; i++) {
 write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
 write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
 // Runtime Condition 13: check CmdAct status bit
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 timeout = 0;
 while ((rd_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("EP TX resource cmd timeout ep=%d, rd_data=0x%x", i, rd_data);
 g_errors++;
 break;
 }
 }
 }

 // Step 42: Enable physical endpoints 0 and 1
 write_reg(MIZAR_USB_DALEPENA, 0x3);

 // Step 43: Configure DCTL
 write_reg(MIZAR_USB_DCTL, 0x80f00000);

 LOGT("USB FS Device Bulk Transfer test init complete");
 return 0;
}

/
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Main testcase stimulus - USB enumeration, GET DESCRIPTOR
 * requests for device and configuration descriptors,
 * SET_CONFIGURATION, and Bulk OUT transfer.
 * Implements Meta Test Steps 44-103.
 * Parameters:
 * cfg - test configuration item
 * out - test output structure
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
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
 // Steps 44-45: Enumeration start
 // Step 45: DV debug marker write_reg(0xA0243ffc, 0xdeadbee0)
 // =========================================================
 LOGD("DV debug marker 0xdeadbee0: Enumeration start");

 // Step 46: Read DCFG
 rd_data = read_reg(MIZAR_USB_DCFG);
 LOGT("Enumeration DCFG read: 0x%x", rd_data);

 // Step 47: Read DSTS
 rd_data = read_reg(MIZAR_USB_DSTS);
 LOGT("Enumeration DSTS read: 0x%x", rd_data);

 // Step 48: Write DCFG
 write_reg(MIZAR_USB_DCFG, 0x480801);

 // Step 49: Write DCTL
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);

 // Step 50: Write DALEPENA
 write_reg(MIZAR_USB_DALEPENA, 0xff);

 // Step 51: setup_stage()
 LOGT("Enumeration: setup_stage()");
 setup_stage();

 // Step 52: Write GUSB2PHYCFG
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

 // Step 53: event_counter conditional check
 if (event_counter <= 0x4) {
 LOGT("event_counter <= 0x4 (0x%x), waiting for interrupt", event_counter);
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Step 53: int_pend wait timeout (event_counter<=0x4)");
 g_errors++;
 break;
 }
 }
 }

 // =========================================================
 // Steps 54-56: GET DESCRIPTOR (Device Descriptor)
 // =========================================================
 LOGT("GET DESCRIPTOR (Device Descriptor)");

 // Step 55: setup_stage()
 setup_stage();

 // Step 56: int_pend wait
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Step 56: int_pend wait timeout");
 g_errors++;
 break;
 }
 }

 // Steps 57-59: Device descriptor TRB
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x12);
 write_reg(event_trb_addr + 0xc, 0x853);

 // Steps 60-64: Device descriptor data
 write_reg(Buffer_PointerLO, 0x02000012);
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);

 // Steps 65-67: DEPCMD endpoint command with offset 0x10
 LOGT("DEPCMD Start Transfer on EP offset 0x10 (device descriptor data stage)");
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // Steps 68-69: Poll DEPCMD+0x10 for command completion
 // Runtime Condition 13: check CmdAct status bit
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout = 0;
 while ((rd_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("Step 69: DEPCMD+0x10 polling timeout, rd_data=0x%x", rd_data);
 g_errors++;
 break;
 }
 }

 // Step 70: int_pend wait
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Step 70: int_pend wait timeout");
 g_errors++;
 break;
 }
 }

 // Step 71: status_stage()
 LOGT("Device descriptor status stage");
 status_stage();

 // =========================================================
 // Steps 72-75: USB_SET_CONFIGURATION_OR_RESET_TT
 // =========================================================
 LOGT("USB_SET_CONFIGURATION_OR_RESET_TT");

 // Step 73: setup_stage()
 setup_stage();

 // Step 74: DV debug marker 0xdeadbee4
 LOGD("DV debug marker 0xdeadbee4: USB_SET_CONFIGURATION");

 // Step 75: int_pend wait
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Step 75: int_pend wait timeout");
 g_errors++;
 break;
 }
 }

 // =========================================================
 // Steps 76-93: GET DESCRIPTOR USB CONFIGURATION
 // =========================================================
 LOGT("GET DESCRIPTOR USB CONFIGURATION");

 // Step 77: setup_stage()
 setup_stage();

 // Step 78: DV debug marker 0xdeadbee5
 LOGD("DV debug marker 0xdeadbee5: GET DESCRIPTOR USB CONFIGURATION");

 // Step 79: int_pend wait
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Step 79: int_pend wait timeout");
 g_errors++;
 break;
 }
 }

 // Steps 80-83: Configuration descriptor data
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 // Steps 84-86: Configuration descriptor TRB
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x9);
 write_reg(event_trb_addr + 0xc, 0x853);

 // Steps 87-89: DEPCMD endpoint command with offset 0x10
 LOGT("DEPCMD Start Transfer on EP offset 0x10 (config descriptor data stage)");
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // Steps 90-91: Poll DEPCMD+0x10 for command completion
 // Runtime Condition 13: check CmdAct status bit
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout = 0;
 while ((rd_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("Step 91: DEPCMD+0x10 polling timeout, rd_data=0x%x", rd_data);
 g_errors++;
 break;
 }
 }

 // Step 92: int_pend wait
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Step 92: int_pend wait timeout");
 g_errors++;
 break;
 }
 }

 // Step 93: status_stage()
 LOGT("Configuration descriptor status stage");
 status_stage();

 // =========================================================
 // Steps 94-103: Bulk OUT Transfer
 // =========================================================
 LOGT("Bulk OUT Transfer: 64 bytes, TRB control 0x813");

 // Steps 95-97: Bulk OUT TRB setup
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);

 // Steps 98-100: DEPCMD endpoint command with offset 0x40
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);

 // Steps 101-102: Poll DEPCMD+0x40 for command completion
 // Runtime Condition 13: check CmdAct status bit
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 timeout = 0;
 while ((rd_data & 0x400) != 0) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 timeout++;
 if (timeout >= POLL_TIMEOUT) {
 LOGE("Step 102: DEPCMD+0x40 polling timeout, rd_data=0x%x", rd_data);
 g_errors++;
 break;
 }
 }

 // Step 103: int_pend wait for bulk transfer completion
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Step 103: int_pend wait timeout (Bulk OUT completion)");
 g_errors++;
 break;
 }
 }

 LOGT("Bulk OUT Transfer command completed");

 // =========================================================
 // Speed verification - Runtime Condition 18
 // =========================================================
 LOGT("Speed verification via DSTS");
 rd_data = read_reg(MIZAR_USB_DSTS);
 LOGT("DSTS read for speed check: 0x%x", rd_data);
 g_checks_total++;
 // MANUAL_REVIEW: Confirm expected FS speed encoding from DV.
 // DSTS bits [2:0] indicate speed: 1 = Full-Speed.
 if ((rd_data & 0x7) != 0x1) {
 LOGE("Speed verification: expected FS (0x1), got 0x%x", (rd_data & 0x7));
 g_errors++;
 g_checks_failed++;
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
 // Bulk OUT data integrity validation - Runtime Condition 10
 // MANUAL_REVIEW: DV source does not provide expected bulk OUT
 // payload data for comparison. The complete data integrity
 // validation requires the expected data pattern to be supplied.
 // =========================================================
 g_checks_total++;
 // MANUAL_REVIEW: Add bulk OUT data integrity check when expected data is available.
 LOGT("Bulk OUT data integrity: MANUAL_REVIEW - expected data pattern not available from DV");

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

/
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Cleans up interrupts, endpoints, event queues, and
 * controller state. Runtime Condition 22.
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
 // state reset may be required based on full DV source.

 LOGT("USB FS Device Bulk Transfer test teardown complete: errors=%u", g_errors);
 return (g_errors == 0) ? 0 : -1;
}
