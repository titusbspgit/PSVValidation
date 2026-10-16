// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

// Forward declaration for nic_programming
void nic_programming(void);


/*
 * Function: set_configuration
 * Description: Generic endpoint command helper. Writes DEPCMDPAR1, DEPCMDPAR0,
 * DEPCMD at given trb_address offset, polls DEPCMD until command
 * completes.
 * Parameters:
 * trb_address - endpoint register offset
 * parameter0 - DEPCMDPAR0 value
 * parameter1 - DEPCMDPAR1 value
 * cmd - DEPCMD command value
 * Returns:
 * void
 /
static void set_configuration(unsigned int trb_address, unsigned int parameter0, unsigned int parameter1, unsigned int cmd)
{
 volatile unsigned int read_data;

 // Step 1: write DEPCMDPAR1
 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
 // Step 2: write DEPCMDPAR0
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
 // Step 3: write DEPCMD
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
 // Step 4: read DEPCMD
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 // Step 5: poll until command completes
 while (read_data == cmd) {
 for (volatile int d = 0; d < 30000; d++);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 }
}


/
 * Function: Default_IRQHandler
 * Description: Interrupt handler. Clears int_pend flag, reads system register
 * status, reads and acknowledges GEVNTCOUNT, conditionally clears
 * raw status bit, clears GIC IRQ 84.
 * Parameters:
 * none
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 volatile unsigned int rd_data;
 volatile unsigned int event_count;

 // Step 1: clear int_pend
 int_pend = 0;
 // Step 2: read masked status
 rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 // Step 3: read raw status
 rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
 // Step 4: read event count
 event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
 // Step 5: store event count globally
 event_counter = event_count;
 // Step 6: acknowledge events
 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
 // Step 7: conditionally clear raw status (source uses logical AND &&)
 if (rd_data && 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }
 // Step 8: clear GIC IRQ 84
 GIC_ClearIRQ(84);
}


/
 * Function: setup_stage
 * Description: Configures a SETUP stage TRB at event_trb_addr with control
 * 0x823, issues DEPCMD Start Transfer command 0x506 on endpoint
 * offset 0x0, polls for command completion, waits for interrupt
 * completion via int_pend.
 * Parameters:
 * none
 * Returns:
 * void
 /
static void setup_stage(void)
{
 volatile unsigned int rd_data;

 // Step 1: TRB buffer pointer
 write_reg(event_trb_addr, Buffer_PointerLO);
 // Step 2: TRB transfer length
 write_reg(event_trb_addr + 0x8, 0x8);
 // Step 3: TRB control 0x823
 write_reg(event_trb_addr + 0xc, 0x823);
 // Step 4: DEPCMDPAR1 = event_trb_addr
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 // Step 5: DEPCMDPAR0 = 0
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 // Step 6: DEPCMD Start Transfer 0x506
 write_reg(MIZAR_USB_DEPCMD, 0x506);
 // Step 7: read DEPCMD
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 // Step 8: poll until command completes
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 }
 // Step 9: set int_pend
 int_pend = 1;
 // Step 10: wait for interrupt completion
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }
}


/
 * Function: status_stage
 * Description: Configures a STATUS stage TRB at event_trb_addr with control
 * 0x843, issues DEPCMD Start Transfer command 0x506 on endpoint
 * offset 0x0, polls for command completion, waits for interrupt
 * completion via int_pend, then additional delay.
 * Parameters:
 * none
 * Returns:
 * void
 /
static void status_stage(void)
{
 volatile unsigned int rd_data;

 // Step 1: TRB buffer pointer
 write_reg(event_trb_addr, Buffer_PointerLO);
 // Step 2: TRB transfer length = 0
 write_reg(event_trb_addr + 0x8, 0x0);
 // Step 3: TRB control 0x843
 write_reg(event_trb_addr + 0xc, 0x843);
 // Step 4: DEPCMDPAR1 = event_trb_addr
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 // Step 5: DEPCMDPAR0 = 0
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 // Step 6: DEPCMD Start Transfer 0x506
 write_reg(MIZAR_USB_DEPCMD, 0x506);
 // Step 7: read DEPCMD
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 // Step 8: poll until command completes
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 }
 // Step 9: set int_pend
 int_pend = 1;
 // Step 10: wait for interrupt completion
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }
 // Step 11: additional delay
 for (volatile int d = 0; d < 5000; d++);
}


/
 * Function: nic_programming
 * Description: Programs the USB device controller. Configures event ring,
 * global control, PHY, device configuration, endpoint
 * configurations, and enables interrupts.
 * Parameters:
 * none
 * Returns:
 * void
 /
void nic_programming(void)
{
 volatile unsigned int rd_data;
 unsigned int i;

 // Step 1: Configure event ring address low
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 // Step 2: Configure event ring address high
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 // Step 3: Configure event ring size
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 // Step 4: Clear event count
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);
 // Step 5: Read GCTL
 rd_data = read_reg(MIZAR_USB_GCTL);
 // Step 6: Write GCTL
 write_reg(MIZAR_USB_GCTL, 0x30c12214);
 // Step 7: Configure USB2 PHY
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);
 // Step 8: Configure device
 write_reg(MIZAR_USB_DCFG, 0x480801);
 // Step 9: Enable device events
 write_reg(MIZAR_USB_DEVTEN, 0x1f);
 // Step 10: Configure user control
 write_reg(MIZAR_USB_GUCTL, 0xa400010);
 // Step 11: set_configuration endpoint 0x00
 set_configuration(0x00, 0x200, 0x700, 0x401);
 // Step 12: set_configuration endpoint 0x10
 set_configuration(0x10, 0x200, 0x2000700, 0x401);
 // Step 13: set_configuration endpoint 0x20
 set_configuration(0x20, 0x206, 0x4000700, 0x401);
 // Step 14: set_configuration endpoint 0x30
 set_configuration(0x30, 0x20206, 0x6000700, 0x401);
 // Step 15: set_configuration endpoint 0x40
 set_configuration(0x40, 0x204, 0x8000700, 0x401);
 // Step 16: set_configuration endpoint 0x50
 set_configuration(0x50, 0x40204, 0xa000700, 0x401);
 // Step 17: set_configuration endpoint 0x60
 set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);
 // Step 18: set_configuration endpoint 0x70
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


/
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes the USB FS Device Bulk Transfer test.
 * Calls nic_programming and enables GIC interrupts.
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test init: starting");

 // test_case Step 1: Call nic_programming()
 nic_programming();

 // test_case Step 2: Call GIC_EnableAllIRQ()
 GIC_EnableAllIRQ();

 LOGT("USB FS Device Bulk Transfer test init: complete");

 return 0;
}


/
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Main test execution. Clears buffer/event TRB memory, performs
 * USB device enumeration (GET_DESCRIPTOR device descriptor,
 * GET_DESCRIPTOR configuration descriptor, SET_CONFIGURATION),
 * executes CDC Bulk OUT and Bulk IN transfers.
 * Parameters:
 * cfg - pointer to test configuration
 * out - pointer to test output
 * Returns:
 * 0 on success (PASS), -1 on failure (FAIL)
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
 volatile unsigned int rd_data;
 unsigned int j;

 (void)cfg;

 if (out == 0) {
 LOGE("USB output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run: starting");

 // test_case Step 3: Buffer/Event TRB Clearing Loop
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }

 // test_case Step 4: First setup_stage
 setup_stage();

 // test_case Step 5: write USB2 PHY config
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

 // test_case Step 6: Conditional event_counter wait (first)
 if (event_counter <= 0x4) {
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }
 }

 // test_case Step 7: Conditional event_counter wait (second)
 if (event_counter <= 0x4) {
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }
 }

 // test_case Step 8: DV debug marker - converted to PSV logging
 LOGD("DV debug marker reached: enumeration start (0xdeadbee0)");

 // test_case Step 9: Read DCFG
 rd_data = read_reg(MIZAR_USB_DCFG);

 // test_case Step 10: Read DSTS
 rd_data = read_reg(MIZAR_USB_DSTS);

 // test_case Step 11: Write DCFG
 write_reg(MIZAR_USB_DCFG, 0x480801);

 // test_case Step 12: Write DCTL
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);

 // test_case Step 13: Write DALEPENA
 write_reg(MIZAR_USB_DALEPENA, 0xff);

 // --- GET_DESCRIPTOR Device Descriptor Data Stage ---
 // test_case Step 14: TRB buffer pointer
 write_reg(event_trb_addr, Buffer_PointerLO);

 // test_case Step 15: TRB transfer length = 0x12
 write_reg(event_trb_addr + 0x8, 0x12);

 // test_case Step 16: TRB control 0x853
 write_reg(event_trb_addr + 0xc, 0x853);

 // test_case Step 17: Device descriptor data word 0
 write_reg(Buffer_PointerLO, 0x02000012);

 // test_case Step 18: Device descriptor data word 1
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);

 // test_case Step 19: Device descriptor data word 2
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);

 // test_case Step 20: Device descriptor data word 3
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 // test_case Step 21: Device descriptor data word 4
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);

 // test_case Step 22: DEPCMDPAR1 at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);

 // test_case Step 23: DEPCMDPAR0 at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 // test_case Step 24: DEPCMD Start Transfer at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // test_case Step 25: Read DEPCMD at endpoint offset 0x10
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);

 // test_case Step 26: Poll until command completes
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 // --- Interrupt waits and XferNotReady handling ---
 // test_case Step 27: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 28: Additional delay
 for (volatile int d = 0; d < 5000; d++);

 // test_case Step 29: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 30: Wait for XferNotReady event
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 31: Call status_stage()
 status_stage();

 // --- GET_DESCRIPTOR USB Configuration (short read, 9 bytes) ---
 // test_case Step 32: Call setup_stage()
 setup_stage();

 // test_case Step 33: DV debug marker - converted to PSV logging
 LOGD("DV debug marker reached: GET_DESCRIPTOR config short read (0xdeadbee3)");

 // test_case Step 34: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 35: TRB buffer pointer
 write_reg(event_trb_addr, Buffer_PointerLO);

 // test_case Step 36: TRB transfer length = 0x9
 write_reg(event_trb_addr + 0x8, 0x9);

 // test_case Step 37: TRB control 0x853
 write_reg(event_trb_addr + 0xc, 0x853);

 // test_case Step 38: Config descriptor short read data word 0
 write_reg(Buffer_PointerLO, 0x003c0209);

 // test_case Step 39: Config descriptor short read data word 1
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);

 // test_case Step 40: Config descriptor short read data word 2
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);

 // test_case Step 41: Config descriptor short read data word 3
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 // test_case Step 42: DEPCMDPAR1 at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);

 // test_case Step 43: DEPCMDPAR0 at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 // test_case Step 44: DEPCMD Start Transfer at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // test_case Step 45: Read DEPCMD at endpoint offset 0x10
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);

 // test_case Step 46: Poll until command completes
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 // test_case Step 47: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 48: Call status_stage()
 status_stage();

 // --- SET_CONFIGURATION ---
 // test_case Step 49: Call setup_stage()
 setup_stage();

 // test_case Step 50: DV debug marker - converted to PSV logging
 LOGD("DV debug marker reached: SET_CONFIGURATION (0xdeadbee4)");

 // test_case Step 51: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 52: DEPCMD Start Transfer at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // test_case Step 53: Read DEPCMD at endpoint offset 0x10
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);

 // test_case Step 54: Poll until command completes
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 // test_case Step 55: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 56: Call status_stage()
 status_stage();

 // --- GET_DESCRIPTOR USB Configuration (full read) ---
 // test_case Step 57: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 58: Call setup_stage()
 setup_stage();

 // test_case Step 59: DV debug marker - converted to PSV logging
 LOGD("DV debug marker reached: GET_DESCRIPTOR config full read (0xdeadbee5)");

 // test_case Step 60: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // --- Data Stage: Full Configuration Descriptor ---
 // 09023c00 010100e0 32090400 00060101 00000705 01034000 01070502 024000ff
 // test_case Step 61: TRB buffer pointer
 write_reg(event_trb_addr, Buffer_PointerLO);

 // test_case Step 62: TRB transfer length = 0x3c
 write_reg(event_trb_addr + 0x8, 0x3c);

 // test_case Step 63: TRB control 0x853
 write_reg(event_trb_addr + 0xc, 0x853);

 // test_case Step 64: Full config descriptor data word 0
 write_reg(Buffer_PointerLO, 0x003c0209);

 // test_case Step 65: Full config descriptor data word 1
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);

 // test_case Step 66: Full config descriptor data word 2
 write_reg(Buffer_PointerLO + 0x8, 0x00040932);

 // test_case Step 67: Full config descriptor data word 3
 write_reg(Buffer_PointerLO + 0xc, 0x01010600);

 // test_case Step 68: Full config descriptor data word 4
 write_reg(Buffer_PointerLO + 0x10, 0x05070000);

 // test_case Step 69: Full config descriptor data word 5
 write_reg(Buffer_PointerLO + 0x14, 0x00400301);

 // test_case Step 70: Full config descriptor data word 6
 write_reg(Buffer_PointerLO + 0x18, 0x02050701);

 // test_case Step 71: Full config descriptor data word 7
 write_reg(Buffer_PointerLO + 0x1c, 0xff004002);

 // test_case Step 72: DEPCMDPAR1 at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);

 // test_case Step 73: DEPCMDPAR0 at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 // test_case Step 74: DEPCMD Start Transfer at endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // test_case Step 75: Read DEPCMD at endpoint offset 0x10
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);

 // test_case Step 76: Poll until command completes
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 // test_case Step 77: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // test_case Step 78: Call status_stage()
 status_stage();

 // --- CDC Bulk OUT Transfer (endpoint offset 0x40) ---
 LOGT("CDC Bulk OUT Transfer: 64 bytes via endpoint offset 0x40");

 // test_case Step 79: TRB buffer pointer (Bulk OUT)
 write_reg(event_trb_addr, Buffer_PointerLO_1);

 // test_case Step 80: TRB transfer length = 0x40
 write_reg(event_trb_addr + 0x8, 0x40);

 // test_case Step 81: TRB control 0x813
 write_reg(event_trb_addr + 0xc, 0x813);

 // test_case Step 82: DEPCMDPAR1 at endpoint offset 0x40
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);

 // test_case Step 83: DEPCMDPAR0 at endpoint offset 0x40
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

 // test_case Step 84: DEPCMD Start Transfer at endpoint offset 0x40
 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);

 // test_case Step 85: Read DEPCMD at endpoint offset 0x40
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);

 // test_case Step 86: Poll until command completes
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 }

 // test_case Step 87: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // --- CDC Bulk IN Transfer (endpoint offset 0x50) ---
 LOGT("CDC Bulk IN Transfer: 64 bytes via endpoint offset 0x50");

 // test_case Step 88: TRB buffer pointer (Bulk IN)
 write_reg(event_trb_addr, Buffer_PointerLO_1);

 // test_case Step 89: TRB transfer length = 0x40
 write_reg(event_trb_addr + 0x8, 0x40);

 // test_case Step 90: TRB control 0x813
 write_reg(event_trb_addr + 0xc, 0x813);

 // test_case Step 91: DEPCMDPAR1 at endpoint offset 0x50
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, event_trb_addr);

 // test_case Step 92: DEPCMDPAR0 at endpoint offset 0x50
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x0);

 // test_case Step 93: DEPCMD Start Transfer at endpoint offset 0x50
 write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);

 // test_case Step 94: Read DEPCMD at endpoint offset 0x50
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);

 // test_case Step 95: Poll until command completes
 while (rd_data == 0x506) {
 for (volatile int d = 0; d < 10000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
 }

 // test_case Step 96: Wait for interrupt completion
 int_pend = 1;
 while (int_pend) {
 for (volatile int d = 0; d < 5000; d++);
 }

 // --- Debug marker and completion ---
 // test_case Step 97: DV debug marker - converted to PSV logging
 LOGD("DV debug marker reached: test completion (0xdeadbee7)");

 // test_case Step 98: finish(0) converted to PSV status
 LOGT("USB FS Device Bulk Transfer test run: PASS");
 out->status = 0;

 return out->status;
}


/
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB FS Device Bulk Transfer test.
 * No additional cleanup required.
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * 0 on success
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test teardown: complete");
 return 0;
}
