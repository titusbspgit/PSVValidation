// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Forward declaration for nic_programming
void nic_programming(void);

// Global interrupt pending flag
volatile int int_pend;

// Global event counter
int event_counter;

// Testcase error tracking
static int g_errors = 0;

/*
 * Function: setup_stage
 * Description: Configures event TRB for setup stage, issues DEPCMD
 * Start Transfer command (0x506), polls for completion,
 * then waits for interrupt via int_pend polling.
 * Parameters:
 * None
 * Returns:
 * void
 /
void setup_stage(void)
{
 int rd_data;
 int count;

 LOGT("setup_stage: configuring event TRB for setup stage");

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x8);
 write_reg(event_trb_addr + 0xc, 0x823);
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 write_reg(MIZAR_USB_DEPCMD, 0x506);

 rd_data = read_reg(MIZAR_USB_DEPCMD);
 count = 0;
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 count++;
 if (count > 100000) {
 LOGE("setup_stage: timeout polling DEPCMD");
 g_errors++;
 break;
 }
 }
 LOGT("setup_stage: DEPCMD poll complete, count=%d", count);

 int_pend = 1;
 count = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 count++;
 if (count > 100000) {
 LOGE("setup_stage: timeout waiting for interrupt");
 g_errors++;
 break;
 }
 }
 LOGT("setup_stage: interrupt wait complete");
}

/
 * Function: status_stage
 * Description: Configures event TRB for status stage, issues DEPCMD
 * Start Transfer command (0x506), polls for completion,
 * waits for interrupt, then performs additional delay.
 * Parameters:
 * None
 * Returns:
 * void
 /
void status_stage(void)
{
 int rd_data;
 int count;

 LOGT("status_stage: configuring event TRB for status stage");

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x843);
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 write_reg(MIZAR_USB_DEPCMD, 0x506);

 rd_data = read_reg(MIZAR_USB_DEPCMD);
 count = 0;
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 count++;
 if (count > 100000) {
 LOGE("status_stage: timeout polling DEPCMD");
 g_errors++;
 break;
 }
 }
 LOGT("status_stage: DEPCMD poll complete, count=%d", count);

 int_pend = 1;
 count = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 count++;
 if (count > 100000) {
 LOGE("status_stage: timeout waiting for interrupt");
 g_errors++;
 break;
 }
 }
 LOGT("status_stage: interrupt wait complete");

 // Additional delay after status stage (wait_on(5))
 for (volatile int d = 0; d < 5000; d++);
 LOGT("status_stage: post-interrupt delay complete");
}

/
 * Function: enumeration
 * Description: Populates device descriptor and configuration descriptor
 * response data via event TRBs and Buffer_PointerLO.
 * Parameters:
 * None
 * Returns:
 * void
 /
void enumeration(void)
{
 LOGT("enumeration: populating device descriptor response");

 // Device descriptor response
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

 LOGT("enumeration: populating configuration descriptor response");

 // Configuration descriptor response
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x3c);
 write_reg(event_trb_addr + 0xc, 0x853);
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 LOGT("enumeration: descriptor responses complete");
}

/
 * Function: set_configuration
 * Description: Issues endpoint command by writing DEPCMDPAR1, DEPCMDPAR0,
 * and DEPCMD registers with polling for command completion.
 * Parameters:
 * trb_address - offset for DEPCMD register
 * parameter0 - value for DEPCMDPAR0
 * parameter1 - value for DEPCMDPAR1
 * cmd - command value for DEPCMD
 * Returns:
 * void
 /
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
 int read_data;
 int count;

 LOGT("set_configuration: trb_address=0x%x param0=0x%x param1=0x%x cmd=0x%x",
 trb_address, parameter0, parameter1, cmd);

 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);

 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 count = 0;
 while (read_data == cmd) {
 for (volatile int d = 0; d < 30000; d++);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 count++;
 if (count > 100000) {
 LOGE("set_configuration: timeout polling DEPCMD+0x%x", trb_address);
 g_errors++;
 break;
 }
 }
 LOGT("set_configuration: command complete, count=%d", count);
}

/
 * Function: Default_IRQHandler
 * Description: Interrupt handler that clears int_pend, reads GEVNTCOUNT,
 * stores in event_counter, writes back to acknowledge events,
 * reads system register status, clears raw interrupt status,
 * and clears GIC IRQ 84.
 * Parameters:
 * None
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 int rd_data, sysreg_rd_data, event_count;

 int_pend = 0;

 rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

 event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
 event_counter = event_count;
 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

 // MANUAL_REVIEW: Source uses logical AND (&&) instead of bitwise AND (&).
 // Preserving source behavior exactly as supplied.
 if (rd_data && 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }

 GIC_ClearIRQ(84);

 LOGT("Default_IRQHandler: event_count=%d, GIC IRQ 84 cleared", event_count);
}

/
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes the USB device controller via nic_programming(),
 * enables all IRQs, clears buffer and event TRB memory regions.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 int j;

 (void)cfg;

 g_errors = 0;

 LOGT("usb_fs_device_bulk_transfer_test_init: starting USB device controller initialization");

 // Step 1: NIC programming for USB device controller initialization
 nic_programming();
 LOGT("nic_programming() called");

 // Step 2: Enable all IRQs
 GIC_EnableAllIRQ();
 LOGT("GIC_EnableAllIRQ() called");

 // Step 3: Clear 20 entries of Buffer_PointerLO and event_trb_addr memory regions
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }
 LOGT("Cleared 20 entries of Buffer_PointerLO and event_trb_addr");

 LOGT("usb_fs_device_bulk_transfer_test_init: initialization complete");

 return 0;
}

/
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Runs the USB Full-Speed Device Bulk Transfer testcase:
 * issues Start Transfer commands, performs interrupt-driven
 * synchronization, calls status_stage/setup_stage,
 * configures bulk data transfer TRBs, and polls for completion.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
 int rd_data, wr_data, port_count, db_offset;
 int i, bulk, intr, event_comletion, j;
 int hand_shake;
 int count;

 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("usb_fs_device_bulk_transfer_test_run: starting");

 // Step 4: DEPCMD+0x10 Start Transfer command
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 count = 0;
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 count++;
 if (count > 100000) {
 LOGE("Timeout polling DEPCMD+0x10 (first Start Transfer)");
 g_errors++;
 break;
 }
 }
 LOGT("DEPCMD+0x10 first Start Transfer poll complete, count=%d", count);

 // Step 5a: First int_pend poll with wait_on(5)
 int_pend = 1;
 count = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 count++;
 if (count > 100000) {
 LOGE("Timeout waiting for interrupt (first poll)");
 g_errors++;
 break;
 }
 }
 LOGT("First interrupt wait complete, count=%d", count);

 // Intermediate delay (wait_on(5))
 for (volatile int d = 0; d < 5000; d++);

 // Step 5b: Second int_pend poll with wait_on(5)
 int_pend = 1;
 count = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 count++;
 if (count > 100000) {
 LOGE("Timeout waiting for interrupt (second poll)");
 g_errors++;
 break;
 }
 }
 LOGT("Second interrupt wait complete, count=%d", count);

 // Step 5c: Third int_pend poll (ADDED to check the xfernotready event)
 int_pend = 1;
 count = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 count++;
 if (count > 100000) {
 LOGE("Timeout waiting for interrupt (third poll - xfernotready)");
 g_errors++;
 break;
 }
 }
 LOGT("Third interrupt wait (xfernotready) complete, count=%d", count);

 // Step 6: Call status_stage()
 status_stage();
 LOGT("status_stage() called");

 // Step 7: Call setup_stage() for USB_SET_CONFIGURATION_OR_RESET_TT
 setup_stage();
 LOGT("setup_stage() called for USB_SET_CONFIGURATION_OR_RESET_TT");

 // Step 8: DV debug marker write_reg(0xA0243ffc, 0xdeadbee4) -> PSV logging
 LOGD("DV debug marker reached: 0xdeadbee4");

 // Step 9: GET DESCRIPTOR USB CONFIGURATION - wait for interrupt
 int_pend = 1;
 count = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 count++;
 if (count > 100000) {
 LOGE("Timeout waiting for interrupt (GET DESCRIPTOR CONFIG)");
 g_errors++;
 break;
 }
 }
 LOGT("Interrupt wait for GET DESCRIPTOR CONFIG complete, count=%d", count);

 // Step 9 continued: Call setup_stage() for GET DESCRIPTOR USB CONFIGURATION
 setup_stage();
 LOGT("setup_stage() called for GET DESCRIPTOR USB CONFIGURATION");

 // Step 10: DV debug marker write_reg(0xA0243ffc, 0xdeadbee5) -> PSV logging
 LOGD("DV debug marker reached: 0xdeadbee5");

 // Step 11: Wait for interrupt, proceed with configuration descriptor data stage
 int_pend = 1;
 count = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 count++;
 if (count > 100000) {
 LOGE("Timeout waiting for interrupt (config descriptor data stage)");
 g_errors++;
 break;
 }
 }
 LOGT("Interrupt wait for config descriptor data stage complete, count=%d", count);

 // data stage - configuration descriptor
 //09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301

 // enumeration() is commented out in source; preserving as comment
 // enumeration();

 // Step 12-15: Bulk data transfer TRB setup
 LOGT("Configuring bulk data transfer TRB");
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x853);
 write_reg(Buffer_PointerLO, 0x00);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 // Step 16: Issue DEPCMD+0x10 with 0x506 for bulk endpoint Start Transfer
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 count = 0;
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 count++;
 if (count > 100000) {
 LOGE("Timeout polling DEPCMD+0x10 (bulk Start Transfer)");
 g_errors++;
 break;
 }
 }
 LOGT("Bulk DEPCMD+0x10 Start Transfer poll complete, count=%d", count);

 // Bulk endpoint TRB with Buffer_PointerLO_1 (commented out in source)
 // write_reg(event_trb_addr, Buffer_PointerLO_1);
 // write_reg(event_trb_addr + 0x8, 0x40);
 // write_reg(event_trb_addr + 0xc, 0x813);
 // write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 // write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
 // write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);

 // Step 17: Wait for interrupt completion via int_pend polling
 int_pend = 1;
 count = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 count++;
 if (count > 100000) {
 LOGE("Timeout waiting for interrupt (final bulk transfer)");
 g_errors++;
 break;
 }
 }
 LOGT("Final bulk transfer interrupt wait complete, count=%d", count);

 // Final status
 if (g_errors != 0) {
 out->status = -1;
 }

 LOGT("usb_fs_device_bulk_transfer_test_run: complete, status=%d, errors=%d",
 out->status, g_errors);

 return out->status;
}

/
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB Full-Speed Device Bulk Transfer testcase.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("usb_fs_device_bulk_transfer_test_teardown: errors=%d", g_errors);

 return (g_errors == 0) ? 0 : -1;
}
