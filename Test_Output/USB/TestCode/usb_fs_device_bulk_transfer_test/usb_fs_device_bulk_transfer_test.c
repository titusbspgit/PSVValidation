// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Global variables
volatile int int_pend;
int event_counter;

// Forward declaration for nic_programming
void nic_programming(void);

/*
 * Function: set_configuration
 * Description: Generic endpoint command helper. Writes DEPCMDPAR1, DEPCMDPAR0,
 * DEPCMD at given trb_address offset, polls DEPCMD until command
 * completes.
 * Parameters:
 * trb_address - endpoint offset
 * parameter0 - DEPCMDPAR0 value
 * parameter1 - DEPCMDPAR1 value
 * cmd - DEPCMD command value
 * Returns:
 * void
 */
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
 int read_data;

 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 while (read_data == cmd) {
 for (volatile int d = 0; d < 30000; d++);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 }
}

/* Function: Default_IRQHandler
 * Description: Interrupt handler. Clears int_pend flag, reads system register
 * status, reads and acknowledges GEVNTCOUNT, conditionally clears
 * raw status bit, clears GIC IRQ 84.
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
 // Note: source uses logical AND (&&) not bitwise AND (&) — preserved as-is
 if (rd_data && 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }
 GIC_ClearIRQ(84);
}

/* Function: setup_stage
 * Description: Configures a SETUP stage TRB at event_trb_addr with control
 * 0x823, issues DEPCMD Start Transfer command 0x506, polls for
 * command completion, waits for interrupt completion via int_pend.
 * Parameters:
 * none
 * Returns:
 * void
 */
void setup_stage(void)
{
 int rd_data;

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x8);
 write_reg(event_trb_addr + 0xc, 0x823);
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 write_reg(MIZAR_USB_DEPCMD, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 }
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }
}

/* Function: status_stage
 * Description: Configures a STATUS stage TRB at event_trb_addr with control
 * 0x843, issues DEPCMD Start Transfer command 0x506, polls for
 * command completion, waits for interrupt completion via int_pend,
 * then additional delay.
 * Parameters:
 * none
 * Returns:
 * void
 */
void status_stage(void)
{
 int rd_data;

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x843);
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 write_reg(MIZAR_USB_DEPCMD, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 }
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }
 for (volatile int d = 0; d < 5000; d++);
}

/* Function: nic_programming
 * Description: Programs the USB device controller. Configures event ring,
 * global control, PHY, device configuration, endpoint
 * configurations, and enables interrupts.
 * Parameters:
 * none
 * Returns:
 * void
 */
void nic_programming(void)
{
 int rd_data;
 volatile unsigned int i;

 // Step 1-4: Event ring configuration
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

 // Step 5-6: Global control
 rd_data = read_reg(MIZAR_USB_GCTL);
 write_reg(MIZAR_USB_GCTL, 0x30c12214);

 // Step 7: PHY configuration
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

 // Step 8: Device configuration
 write_reg(MIZAR_USB_DCFG, 0x480801);

 // Step 9: Device events enable
 write_reg(MIZAR_USB_DEVTEN, 0x1f);

 // Step 10: User control
 write_reg(MIZAR_USB_GUCTL, 0xa400010);

 // Step 11-18: Endpoint configurations via set_configuration
 set_configuration(0x00, 0x200, 0x700, 0x401);
 set_configuration(0x10, 0x200, 0x2000700, 0x401);
 set_configuration(0x20, 0x206, 0x4000700, 0x401);
 set_configuration(0x30, 0x20206, 0x6000700, 0x401);
 set_configuration(0x40, 0x204, 0x8000700, 0x401);
 set_configuration(0x50, 0x40204, 0xa000700, 0x401);
 set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
 set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

 // Step 19: Endpoint TX resource configuration loop (i=0 to 7)
 for (i = 0; i < 8; i++) {
 write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
 write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 while (rd_data == 0x402) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 }
 }

 // Step 20: Enable endpoints
 write_reg(MIZAR_USB_DALEPENA, 0x3);

 // Step 21: Device control
 write_reg(MIZAR_USB_DCTL, 0x80f00000);

 // Step 22: System interrupt enable
 write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
}

/* Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes USB device controller and enables GIC interrupts.
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test init");

 // Step 1: Call nic_programming
 nic_programming();

 // Step 2: Call GIC_EnableAllIRQ
 GIC_EnableAllIRQ();

 LOGT("USB FS Device Bulk Transfer test init complete");
 return 0;
}

/* Function: usb_fs_device_bulk_transfer_test_run
 * Description: Main testcase execution. Clears buffer/event TRB memory,
 * performs USB device enumeration (GET_DESCRIPTOR device descriptor,
 * GET_DESCRIPTOR configuration descriptor, SET_CONFIGURATION),
 * executes Bulk OUT and Bulk IN transfers.
 * Parameters:
 * cfg - pointer to test configuration
 * out - pointer to test output
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
 volatile int rd_data;
 volatile unsigned int j;

 (void)cfg;

 if (out == 0) {
 LOGE("USB output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("Starting USB FS Device Bulk Transfer test run");

 // Step 3: Buffer/Event TRB Clearing Loop
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }

 // Step 4: First setup_stage
 setup_stage();

 // Step 5: PHY configuration
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

 // Step 6: Conditional event_counter wait
 if (event_counter <= 0x4) {
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }
 }

 // Step 7: Conditional event_counter wait
 if (event_counter <= 0x4) {
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }
 }

 // Step 8: DV debug marker — converted to PSV logging
 LOGD("DV debug marker reached");

 // Step 9: Read DCFG
 rd_data = read_reg(MIZAR_USB_DCFG);

 // Step 10: Read DSTS
 rd_data = read_reg(MIZAR_USB_DSTS);

 // Step 11: Write DCFG
 write_reg(MIZAR_USB_DCFG, 0x480801);

 // Step 12: Write DCTL
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);

 // Step 13: Write DALEPENA
 write_reg(MIZAR_USB_DALEPENA, 0xff);

 // --- GET_DESCRIPTOR Device Descriptor Data Stage ---
 // Step 14: TRB buffer pointer
 write_reg(event_trb_addr, Buffer_PointerLO);

 // Step 15: TRB transfer length
 write_reg(event_trb_addr + 0x8, 0x12);

 // Step 16: TRB control
 write_reg(event_trb_addr + 0xc, 0x853);

 // Step 17-21: Device descriptor data
 write_reg(Buffer_PointerLO, 0x02000012);
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);

 // Step 22-24: Issue DEPCMD Start Transfer on endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // Step 25-26: Poll for command completion
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 // Step 27: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 28: Delay
 for (volatile int d = 0; d < 5000; d++);

 // Step 29: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 30: Interrupt wait — ADDED to check the XferNotReady event
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 31: Call status_stage
 status_stage();

 // --- GET_DESCRIPTOR USB Configuration (short read, 9 bytes) ---
 // Step 32: Call setup_stage
 setup_stage();

 // Step 33: Debug marker 0xdeadbee3
 write_reg(0xA0243ffc, 0xdeadbee3);

 // Step 34: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 35-37: TRB for config descriptor short read
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x9);
 write_reg(event_trb_addr + 0xc, 0x853);

 // Step 38-41: Config descriptor short read data
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 // Step 42-44: Issue DEPCMD Start Transfer on endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // Step 45-46: Poll for command completion
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 // Step 47: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 48: Call status_stage
 status_stage();

 // --- SET_CONFIGURATION ---
 // Step 49: Call setup_stage
 setup_stage();

 // Step 50: Debug marker 0xdeadbee4
 write_reg(0xA0243ffc, 0xdeadbee4);

 // Step 51: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 52: Issue DEPCMD Start Transfer on endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // Step 53-54: Poll for command completion
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 // Step 55: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 56: Call status_stage
 status_stage();

 // --- GET_DESCRIPTOR USB Configuration (full read) ---
 // Step 57: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 58: Call setup_stage
 setup_stage();

 // Step 59: Debug marker 0xdeadbee5
 write_reg(0xA0243ffc, 0xdeadbee5);

 // Step 60: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // --- Data Stage: Full Configuration Descriptor ---
 // Source comment: 09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301
 // Step 61-63: TRB for full config descriptor
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x3c);
 write_reg(event_trb_addr + 0xc, 0x853);

 // Step 64-71: Full configuration descriptor data
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00040932);
 write_reg(Buffer_PointerLO + 0xc, 0x01010600);
 write_reg(Buffer_PointerLO + 0x10, 0x05070000);
 write_reg(Buffer_PointerLO + 0x14, 0x00400301);
 write_reg(Buffer_PointerLO + 0x18, 0x02050701);
 write_reg(Buffer_PointerLO + 0x1c, 0xff004002);

 // Step 72-74: Issue DEPCMD Start Transfer on endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // Step 75-76: Poll for command completion
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 // Step 77: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // Step 78: Call status_stage
 status_stage();

 // --- Bulk OUT Transfer (endpoint offset 0x40) ---
 LOGT("Starting Bulk OUT transfer (endpoint offset 0x40)");

 // Step 79-81: TRB for Bulk OUT
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);

 // Step 82-84: Issue DEPCMD Start Transfer on endpoint offset 0x40
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);

 // Step 85-86: Poll for command completion
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 }

 // Step 87: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 LOGT("Bulk OUT transfer complete");

 // --- Bulk IN Transfer (endpoint offset 0x50) ---
 LOGT("Starting Bulk IN transfer (endpoint offset 0x50)");

 // Step 88-90: TRB for Bulk IN
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);

 // Step 91-93: Issue DEPCMD Start Transfer on endpoint offset 0x50
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);

 // Step 94-95: Poll for command completion
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
 }

 // Step 96: Interrupt wait
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 LOGT("Bulk IN transfer complete");

 // Step 97: Debug marker 0xdeadbee7
 write_reg(0xA0243ffc, 0xdeadbee7);

 // Step 98: finish(0) — converted to PSV completion
 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run complete: %s",
 (out->status == 0) ? "PASS" : "FAIL");

 return out->status;
}

/* Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB FS Device Bulk Transfer test.
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test teardown: no additional cleanup required");
 return 0;
}
