// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

#define POLL_TIMEOUT_COUNT 100000U
#define INT_WAIT_TIMEOUT 500000U
#define DELAY_5_LOOPS 5000
#define DELAY_10_LOOPS 10000
#define DELAY_30_LOOPS 30000
#define DELAY_5000_LOOPS 500000

// Runtime condition: declare int_pend as volatile int
volatile int int_pend;
int event_counter;

// Runtime condition: nic_programming prototype before any call
void nic_programming(void);

// USB test context for tracking errors and validation
typedef struct {
 unsigned int errors;
 unsigned int cmd_completions;
 unsigned int irq_count;
 unsigned int bulk_tx_complete;
 unsigned int bulk_rx_complete;
 unsigned int enum_complete;
} usb_test_ctx_t;

static usb_test_ctx_t g_usb_ctx;


/*
 * Function: wait_for_cmd_completion
 * Description: Poll DEPCMD register for command completion with timeout.
 * Uses status-bit checking per runtime condition.
 * Parameters:
 * reg_addr - DEPCMD register address (with offset)
 * cmd_val - command value written
 * Returns:
 * 0 on success, -1 on timeout
 /
static int wait_for_cmd_completion(unsigned int reg_addr, unsigned int cmd_val)
{
 volatile unsigned int rd_data;
 volatile unsigned int timeout = 0U;

 rd_data = (unsigned int)read_reg(reg_addr);
 // Runtime condition: check command active bit instead of full value comparison
 while ((rd_data & 0x400U) != 0U) {
 for (volatile int d = 0; d < DELAY_10_LOOPS; d++);
 rd_data = (unsigned int)read_reg(reg_addr);
 timeout++;
 if (timeout >= POLL_TIMEOUT_COUNT) {
 LOGE("Timeout waiting for cmd 0x%x completion at reg 0x%x", cmd_val, reg_addr);
 g_usb_ctx.errors++;
 return -1;
 }
 }
 g_usb_ctx.cmd_completions++;
 return 0;
}


/
 * Function: wait_for_interrupt
 * Description: Wait for int_pend to be cleared by ISR with timeout protection.
 * Runtime condition: interrupt race prevention - set int_pend before
 * starting the transfer, not after.
 * Parameters:
 * None
 * Returns:
 * 0 on success, -1 on timeout
 /
static int wait_for_interrupt(void)
{
 volatile unsigned int timeout = 0U;

 while (int_pend) {
 for (volatile int d = 0; d < DELAY_5_LOOPS; d++);
 timeout++;
 if (timeout >= INT_WAIT_TIMEOUT) {
 LOGE("Timeout waiting for interrupt, int_pend still set");
 g_usb_ctx.errors++;
 return -1;
 }
 }
 g_usb_ctx.irq_count++;
 return 0;
}


/
 * Function: set_configuration
 * Description: Configure endpoint using DEPCMDPAR1, DEPCMDPAR0, DEPCMD with polling.
 * Parameters:
 * trb_address - endpoint offset
 * parameter0 - DEPCMDPAR0 value
 * parameter1 - DEPCMDPAR1 value
 * cmd - command value
 * Returns:
 * void
 /
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
 (void)wait_for_cmd_completion(MIZAR_USB_DEPCMD + trb_address, (unsigned int)cmd);
}


/
 * Function: setup_stage
 * Description: Perform USB control transfer setup stage.
 * Programs TRB and issues Start Transfer command 0x506.
 * Parameters:
 * None
 * Returns:
 * void
 /
static void setup_stage(void)
{
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x8);
 write_reg(event_trb_addr + 0xc, 0x823);
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 // Runtime condition: set int_pend before issuing command to prevent race
 int_pend = 1;
 write_reg(MIZAR_USB_DEPCMD, 0x506);
 (void)wait_for_cmd_completion(MIZAR_USB_DEPCMD, 0x506U);
 (void)wait_for_interrupt();
}


/
 * Function: status_stage
 * Description: Perform USB control transfer status stage.
 * Programs TRB and issues Start Transfer command 0x506.
 * Parameters:
 * None
 * Returns:
 * void
 /
static void status_stage(void)
{
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x843);
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 // Runtime condition: set int_pend before issuing command to prevent race
 int_pend = 1;
 write_reg(MIZAR_USB_DEPCMD, 0x506);
 (void)wait_for_cmd_completion(MIZAR_USB_DEPCMD, 0x506U);
 (void)wait_for_interrupt();
 // Meta step: wait_on(5) after status_stage interrupt wait
 for (volatile int d = 0; d < DELAY_5_LOOPS; d++);
}


/
 * Function: enumeration
 * Description: Perform USB device enumeration sequence including event buffer
 * configuration, device/configuration descriptor data stages,
 * setup and status stages.
 * Parameters:
 * None
 * Returns:
 * void
 /
static void enumeration(void)
{
 volatile unsigned int rd_data;

 // Step 21: write event buffer address low
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 // Step 22: write event buffer address high
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 // Step 23: write event buffer size
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 // Step 24: write event count
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

 // Step 25: read GCTL
 rd_data = (unsigned int)read_reg(MIZAR_USB_GCTL);
 LOGT("GCTL read: 0x%x", rd_data);

 // Step 26: DV debug marker 0xdeadbee0 - convert to PSV logging
 LOGD("DV debug marker reached: 0xdeadbee0 - enumeration event buffer configured");

 // Step 27: read DCFG
 rd_data = (unsigned int)read_reg(MIZAR_USB_DCFG);
 LOGT("DCFG read: 0x%x", rd_data);

 // Step 28: read DSTS
 rd_data = (unsigned int)read_reg(MIZAR_USB_DSTS);
 LOGT("DSTS read: 0x%x", rd_data);

 // Runtime condition: speed verification - check DSTS for FS mode
 // DSTS[2:0] = connection speed; FS = 1
 if ((rd_data & 0x7U) != 0x1U) {
 LOGE("Expected Full-Speed (0x1) in DSTS, got 0x%x", (rd_data & 0x7U));
 // MANUAL_REVIEW: Verify DSTS speed field encoding for this platform
 g_usb_ctx.errors++;
 }

 // Step 29: write DCFG
 write_reg(MIZAR_USB_DCFG, 0x480801);
 // Step 30: write DCTL
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);
 // Step 31: write DALEPENA
 write_reg(MIZAR_USB_DALEPENA, 0xff);

 // Step 32: printf - convert to PSV logging
 LOGT("Buffer_PointerLO is 0x%x", Buffer_PointerLO);

 // Step 33: wait_on(5000) - convert to delay loop
 for (volatile int d = 0; d < DELAY_5000_LOOPS; d++);

 // Step 34: setup_stage() - GET_DESCRIPTOR (Device) setup stage
 LOGT("Enumeration: GET_DESCRIPTOR (Device) - setup stage");
 setup_stage();

 // Step 35: DV debug marker 0xdeadbee1
 LOGD("DV debug marker reached: 0xdeadbee1 - setup stage complete");

 // Step 36: int_pend wait - wait for host SETUP packet event
 int_pend = 1;
 (void)wait_for_interrupt();

 // Step 37: Configure TRB for device descriptor data stage
 LOGT("Enumeration: Device descriptor data stage TRB config");
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x12);
 write_reg(event_trb_addr + 0xc, 0x853);

 // Step 38: Write device descriptor data to Buffer_PointerLO
 write_reg(Buffer_PointerLO, 0x02000012);
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);

 // Step 39: Issue Start Transfer on DEPCMD+0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 // Step 40: Start Transfer command 0x506 on DEPCMD+0x10
 int_pend = 1;
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
 (void)wait_for_cmd_completion(MIZAR_USB_DEPCMD + 0x10, 0x506U);

 // Step 41: int_pend wait
 (void)wait_for_interrupt();

 // Step 42: int_pend wait
 int_pend = 1;
 (void)wait_for_interrupt();

 // Step 43: int_pend wait
 int_pend = 1;
 (void)wait_for_interrupt();

 // Step 44: status_stage() - device descriptor status stage
 LOGT("Enumeration: Device descriptor - status stage");
 status_stage();

 // Step 45: setup_stage() - SET_ADDRESS setup stage
 LOGT("Enumeration: SET_ADDRESS - setup stage");
 setup_stage();

 // Step 46: DV debug marker 0xdeadbee4
 LOGD("DV debug marker reached: 0xdeadbee4 - SET_ADDRESS setup complete");

 // Step 47: setup_stage() - SET_CONFIGURATION / GET_DESCRIPTOR(Config) setup stage
 LOGT("Enumeration: GET_DESCRIPTOR (Configuration) - setup stage");
 setup_stage();

 // Step 48: DV debug marker 0xdeadbee5
 LOGD("DV debug marker reached: 0xdeadbee5 - config descriptor setup complete");

 // Step 49: int_pend wait
 int_pend = 1;
 (void)wait_for_interrupt();

 // Step 50: Configure TRB for configuration descriptor (9-byte)
 LOGT("Enumeration: Configuration descriptor data stage TRB config (9-byte)");
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x9);
 write_reg(event_trb_addr + 0xc, 0x853);

 // Step 51: Write configuration descriptor data
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 // Step 52: Reconfigure TRB for full configuration descriptor (60-byte)
 LOGT("Enumeration: Configuration descriptor data stage TRB config (60-byte)");
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x3c);
 write_reg(event_trb_addr + 0xc, 0x853);

 // Step 53: Write endpoint descriptor data
 write_reg(Buffer_PointerLO + 0x1c, 0xff004002);
 write_reg(Buffer_PointerLO + 0x20, 0x01030507);
 write_reg(Buffer_PointerLO + 0x24, 0x070103ff);
 write_reg(Buffer_PointerLO + 0x28, 0x40038105);

 // Step 54: status_stage() - configuration descriptor status stage
 LOGT("Enumeration: Configuration descriptor - status stage");
 status_stage();

 g_usb_ctx.enum_complete = 1U;
 LOGT("Enumeration complete");
}


/
 * Function: Default_IRQHandler
 * Description: USB interrupt handler. Clears int_pend, reads system interrupt
 * status, acknowledges event count, clears system interrupt,
 * clears GIC IRQ 84.
 * Parameters:
 * None
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 volatile unsigned int rd_data;
 volatile unsigned int event_count;

 // Meta step: int_pend = 0
 int_pend = 0;

 // Read system interrupt status registers
 rd_data = (unsigned int)read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 rd_data = (unsigned int)read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

 // Read and acknowledge event count
 event_count = (unsigned int)read_reg(MIZAR_USB_GEVNTCOUNT);
 event_counter = (int)event_count;
 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

 // Runtime condition: use proper bitmask operation instead of logical AND
 // Meta source has: if(rd_data && 0x80000000) - preserved as noted bug
 // Runtime condition overrides: use bitwise AND
 if ((rd_data & 0x80000000U) != 0U) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }

 GIC_ClearIRQ(84);
}


/
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initialize USB FS device bulk transfer test.
 * Calls nic_programming, enables GIC, clears buffers,
 * configures endpoints.
 * Parameters:
 * cfg - test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 volatile unsigned int rd_data;
 volatile unsigned int i;
 volatile unsigned int j;

 (void)cfg;

 // Initialize test context
 g_usb_ctx.errors = 0U;
 g_usb_ctx.cmd_completions = 0U;
 g_usb_ctx.irq_count = 0U;
 g_usb_ctx.bulk_tx_complete = 0U;
 g_usb_ctx.bulk_rx_complete = 0U;
 g_usb_ctx.enum_complete = 0U;

 LOGT("USB FS Device Bulk Transfer test init");

 // Step 5: nic_programming() - controller initialization
 nic_programming();
 LOGT("nic_programming complete");

 // Step 6: GIC_EnableAllIRQ()
 GIC_EnableAllIRQ();
 LOGT("GIC interrupts enabled");

 // Step 7: Clear 20 DWORD entries in Buffer_PointerLO and event_trb_addr
 for (j = 0U; j < 20U; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }
 LOGT("Buffer and event TRB regions cleared");

 // Steps 8-15: Configure 8 endpoints using set_configuration with command 0x401
 LOGT("Configuring 8 endpoints with command 0x401");
 set_configuration(0, 0x200, 0x700, 0x401);
 set_configuration(0x10, 0x200, 0x2000700, 0x401);
 set_configuration(0x20, 0x206, 0x4000700, 0x401);
 set_configuration(0x30, 0x20206, 0x6000700, 0x401);
 set_configuration(0x40, 0x204, 0x8000700, 0x401);
 set_configuration(0x50, 0x40204, 0xa000700, 0x401);
 set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
 set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);
 LOGT("8 endpoint configurations (0x401) complete");

 // Step 16: Endpoint TX resource configuration for 8 endpoints with command 0x402
 LOGT("Configuring 8 endpoint TX resources with command 0x402");
 for (i = 0U; i < 8U; i++) {
 write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10U), 0x1);
 write_reg(MIZAR_USB_DEPCMD + (i * 0x10U), 0x402);
 if (wait_for_cmd_completion(MIZAR_USB_DEPCMD + (i * 0x10U), 0x402U) != 0) {
 LOGE("Endpoint %u TX resource config (0x402) failed", i);
 return -1;
 }
 }
 LOGT("8 endpoint TX resource configurations (0x402) complete");

 // Step 17: write DALEPENA
 write_reg(MIZAR_USB_DALEPENA, 0x3);

 // Step 18: write DCTL
 write_reg(MIZAR_USB_DCTL, 0x80f00000);

 // Step 19: write LSS interrupt enable
 write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
 LOGT("Init complete: DALEPENA=0x3, DCTL=0x80f00000, INTR_EN0=0x80000000");

 return 0;
}


/
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Execute USB FS device bulk transfer test.
 * Performs enumeration, PHY configuration, and bulk transfers.
 * Parameters:
 * cfg - test configuration
 * out - test output
 * Returns:
 * 0 on PASS, -1 on FAIL
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
 volatile unsigned int rd_data;

 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run starting");

 // Step 20: enumeration()
 LOGT("Starting USB enumeration");
 enumeration();

 // Step 55: Return from enumeration() to test_case()
 LOGT("Enumeration returned, continuing test_case flow");

 // Step 56: setup_stage() - SET_CONFIGURATION setup stage
 LOGT("Post-enumeration: setup_stage (SET_CONFIGURATION)");
 setup_stage();

 // Step 57: write GUSB2PHYCFG
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);
 LOGT("GUSB2PHYCFG written: 0x40002547");

 // Step 58: if(event_counter <= 0x4) { int_pend = 1; wait for interrupt }
 if (event_counter <= 0x4) {
 int_pend = 1;
 (void)wait_for_interrupt();
 }

 // Step 59: if(event_counter <= 0x4) { int_pend = 1; wait for interrupt }
 if (event_counter <= 0x4) {
 int_pend = 1;
 (void)wait_for_interrupt();
 }

 // Step 60: Configure bulk transfer TRB with Buffer_PointerLO_1
 LOGT("Configuring bulk transfer TRB: buf=Buffer_PointerLO_1, len=0x40, ctrl=0x813");
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);

 // Step 61: Program DEPCMDPAR1+0x40 and DEPCMDPAR0+0x40
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

 // Step 62: Start Transfer 0x506 on DEPCMD+0x40
 // Runtime condition: set int_pend before issuing command to prevent race
 int_pend = 1;
 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
 (void)wait_for_cmd_completion(MIZAR_USB_DEPCMD + 0x40, 0x506U);

 // Step 63: wait for interrupt
 (void)wait_for_interrupt();
 g_usb_ctx.bulk_tx_complete = 1U;
 LOGT("Bulk transfer on endpoint offset 0x40 complete");

 // Runtime condition: data integrity validation for bulk transfer
 // Read back event TRB status to validate transfer completion
 rd_data = (unsigned int)read_reg(event_trb_addr + 0x8);
 // Check transfer length residual - should be 0 for complete transfer
 if ((rd_data & 0x00FFFFFFU) != 0U) {
 LOGE("Bulk TX transfer length residual non-zero: 0x%x", (rd_data & 0x00FFFFFFU));
 g_usb_ctx.errors++;
 }
 rd_data = (unsigned int)read_reg(event_trb_addr + 0xc);
 // Validate TRB status field for completion
 LOGT("Bulk TX TRB status: 0x%x", rd_data);

 // Step 64: Start Transfer 0x506 on DEPCMD+0x50
 // Runtime condition: set int_pend before issuing command to prevent race
 int_pend = 1;
 write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
 (void)wait_for_cmd_completion(MIZAR_USB_DEPCMD + 0x50, 0x506U);

 // Step 65: wait for interrupt
 (void)wait_for_interrupt();
 g_usb_ctx.bulk_rx_complete = 1U;
 LOGT("Bulk transfer on endpoint offset 0x50 complete");

 // Runtime condition: data integrity validation for bulk RX transfer
 rd_data = (unsigned int)read_reg(event_trb_addr + 0x8);
 if ((rd_data & 0x00FFFFFFU) != 0U) {
 LOGE("Bulk RX transfer length residual non-zero: 0x%x", (rd_data & 0x00FFFFFFU));
 g_usb_ctx.errors++;
 }
 rd_data = (unsigned int)read_reg(event_trb_addr + 0xc);
 LOGT("Bulk RX TRB status: 0x%x", rd_data);

 // Step 66: DV debug marker 0xdeadbee7
 LOGD("DV debug marker reached: 0xdeadbee7 - bulk transfers complete");

 // Step 67: int_pend wait
 int_pend = 1;
 (void)wait_for_interrupt();

 // Step 68: finish(0) - converted to PSV status return
 // Runtime condition: meaningful pass/fail criteria
 if (g_usb_ctx.errors != 0U) {
 LOGE("Test FAIL: errors=%u", g_usb_ctx.errors);
 out->status = -1;
 } else if (g_usb_ctx.enum_complete != 1U) {
 LOGE("Test FAIL: enumeration did not complete");
 out->status = -1;
 } else if (g_usb_ctx.bulk_tx_complete != 1U) {
 LOGE("Test FAIL: bulk TX transfer on endpoint 0x40 did not complete");
 out->status = -1;
 } else if (g_usb_ctx.bulk_rx_complete != 1U) {
 LOGE("Test FAIL: bulk RX transfer on endpoint 0x50 did not complete");
 out->status = -1;
 } else {
 LOGT("Test PASS: enumeration and bulk transfers completed successfully");
 out->status = 0;
 }

 LOGT("Run complete: %s errors=%u cmd_completions=%u irq_count=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 g_usb_ctx.errors,
 g_usb_ctx.cmd_completions,
 g_usb_ctx.irq_count);

 return out->status;
}


/
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown USB FS device bulk transfer test.
 * Cleans up interrupts, endpoints, event queues, controller state.
 * Parameters:
 * cfg - test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 volatile unsigned int i;

 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test teardown");

 // Runtime condition: proper teardown - disable interrupts
 write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x0);
 LOGT("System interrupts disabled");

 // Clear event count to stop event processing
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);
 write_reg(MIZAR_USB_GEVNTSIZ, 0x0);
 LOGT("Event queue cleared");

 // Disable all endpoints
 write_reg(MIZAR_USB_DALEPENA, 0x0);
 LOGT("All endpoints disabled");

 // Clear pending int_pend
 int_pend = 0;

 // Clear buffer and event TRB regions
 for (i = 0U; i < 20U; i++) {
 write_reg(Buffer_PointerLO + i * DWORD, 0x0);
 write_reg(event_trb_addr + i * DWORD, 0x0);
 }
 LOGT("Buffer and event TRB regions cleared");

 LOGT("Teardown complete: errors=%u", g_usb_ctx.errors);
 return (g_usb_ctx.errors == 0U) ? 0 : -1;
}
