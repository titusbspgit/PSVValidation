// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"

// Global variables
volatile int int_pend;
int event_counter;

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler - clears interrupt pending flag, reads status, acknowledges event count
 * Parameters:
 * None
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 int rd_data;
 int event_count;

 // STEP 1: Clear interrupt pending flag
 int_pend = 0;

 // STEP 2: Read masked interrupt status
 rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 LOGD("IRQ: MSK_STS0 = 0x%x", rd_data);

 // STEP 3: Read raw interrupt status
 rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
 LOGD("IRQ: RAW_STCR0 = 0x%x", rd_data);

 // STEP 4: Read event count
 event_count = read_reg(MIZAR_USB_GEVNTCOUNT);

 // STEP 5: Store event count globally
 event_counter = event_count;

 // STEP 6: Acknowledge event count
 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
 LOGD("IRQ: GEVNTCOUNT = 0x%x", event_count);

 // STEP 7: Check raw status and clear - preserving (rd_data && 0x80000000) per No Silent Correction Rule
 if (rd_data && 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }

 // STEP 8: Clear GIC IRQ
 GIC_ClearIRQ(84);
}

/
 * Function: setup_stage
 * Description: USB setup stage - programs TRB and starts transfer on endpoint 0
 * Parameters:
 * None
 * Returns:
 * void
 /
void setup_stage(void)
{
 int rd_data;
 volatile unsigned int timeout;

 // STEP 1
 write_reg(event_trb_addr, Buffer_PointerLO);

 // STEP 2
 write_reg(event_trb_addr + 0x8, 0x8);

 // STEP 3
 write_reg(event_trb_addr + 0xc, 0x823);

 // STEP 4
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);

 // STEP 5
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);

 // STEP 6
 write_reg(MIZAR_USB_DEPCMD, 0x506);

 // STEP 7
 rd_data = read_reg(MIZAR_USB_DEPCMD);

 // STEP 8: Poll DEPCMD until command completes
 timeout = 0;
 while (rd_data == 0x506) {
 // wait_on(10) adapted to busy-wait
 for (volatile int d = 0; d < 1000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout++;
 if (timeout > 10000) {
 LOGE("setup_stage: DEPCMD polling timeout");
 break;
 }
 }
 LOGD("setup_stage: DEPCMD = 0x%x", rd_data);

 // STEP 9: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("setup_stage: interrupt wait timeout");
 break;
 }
 }
}

/
 * Function: status_stage
 * Description: USB status stage - programs TRB and starts transfer on endpoint 0
 * Parameters:
 * None
 * Returns:
 * void
 /
void status_stage(void)
{
 int rd_data;
 volatile unsigned int timeout;

 // STEP 1
 write_reg(event_trb_addr, Buffer_PointerLO);

 // STEP 2
 write_reg(event_trb_addr + 0x8, 0x0);

 // STEP 3
 write_reg(event_trb_addr + 0xc, 0x843);

 // STEP 4
 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);

 // STEP 5
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);

 // STEP 6
 write_reg(MIZAR_USB_DEPCMD, 0x506);

 // STEP 7
 rd_data = read_reg(MIZAR_USB_DEPCMD);

 // STEP 8: Poll DEPCMD until command completes
 timeout = 0;
 while (rd_data == 0x506) {
 // wait_on(10) adapted to busy-wait
 for (volatile int d = 0; d < 1000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout++;
 if (timeout > 10000) {
 LOGE("status_stage: DEPCMD polling timeout");
 break;
 }
 }
 LOGD("status_stage: DEPCMD = 0x%x", rd_data);

 // STEP 9: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("status_stage: interrupt wait timeout");
 break;
 }
 }

 // STEP 10: wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
}

/
 * Function: set_configuration
 * Description: Writes DEPCMD parameters and polls for command completion
 * Parameters:
 * trb_address - endpoint offset
 * parameter0 - DEPCMDPAR0 value
 * parameter1 - DEPCMDPAR1 value
 * cmd - DEPCMD command value
 * Returns:
 * void
 /
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
 int read_data;
 volatile unsigned int timeout;

 // STEP 1
 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);

 // STEP 2
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);

 // STEP 3
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);

 // STEP 4
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);

 // STEP 5: Poll DEPCMD until command completes
 timeout = 0;
 while (read_data == cmd) {
 // wait_on(30) adapted to busy-wait
 for (volatile int d = 0; d < 3000; d++);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 timeout++;
 if (timeout > 10000) {
 LOGE("set_configuration: DEPCMD polling timeout at offset 0x%x", trb_address);
 break;
 }
 }
 LOGD("set_configuration: DEPCMD offset=0x%x read_data=0x%x", trb_address, read_data);
}

/
 * Function: enumeration
 * Description: USB enumeration helper
 * Parameters:
 * None
 * Returns:
 * void
 /
void enumeration(void)
{
 // MANUAL_REVIEW: enumeration() body not provided in Meta Test Steps - implement per USB enumeration requirements
}

/
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes USB FS Device Bulk Transfer test - NIC programming, GIC enable, buffer/event ring clear, event ring setup, controller configuration, endpoint setup
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 int rd_data;
 int wr_data;
 volatile unsigned int j;
 volatile unsigned int i;
 volatile unsigned int timeout;

 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test init start");

 // STEP 1: nic_programming
 nic_programming();
 LOGD("nic_programming complete");

 // STEP 2: Enable all GIC IRQs
 GIC_EnableAllIRQ();
 LOGD("GIC IRQ enabled");

 // STEP 3: Clear buffer and event TRB areas
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }
 LOGD("Buffer and event TRB areas cleared");

 // Event ring initialization (from first Meta JSON - required for GEVNTCOUNT in IRQ handler)
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);
 LOGD("Event ring configured: GEVNTADRLO=Default_Event_Ring_Array GEVNTSIZ=0x30");

 // STEP 4: Write GCTL
 wr_data = 0x30c12214;

 // STEP 5
 write_reg(MIZAR_USB_GCTL, wr_data);
 LOGD("GCTL = 0x%x", wr_data);

 // STEP 6: Read DCFG
 rd_data = read_reg(MIZAR_USB_DCFG);
 LOGD("DCFG = 0x%x", rd_data);

 // STEP 7: Write DCFG
 write_reg(MIZAR_USB_DCFG, 0x480801);

 // STEP 8: Write DEVTEN
 write_reg(MIZAR_USB_DEVTEN, 0x1f);
 LOGD("DEVTEN = 0x1f");

 // STEP 9: Read GUCTL
 rd_data = read_reg(MIZAR_USB_GUCTL);
 LOGD("GUCTL = 0x%x", rd_data);

 // STEP 10: Write GUCTL
 write_reg(MIZAR_USB_GUCTL, 0xa400010);

 // LSS interrupt enable (from first Meta JSON - required for int_pend polling to function)
 write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
 LOGD("LSS interrupt enabled");

 // STEP 11: set_configuration(0, 0, 0, 0x409)
 set_configuration(0, 0, 0, 0x409);

 // STEP 12: setup_stage
 setup_stage();

 // STEP 13: Write GUSB2PHYCFG
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);
 LOGD("GUSB2PHYCFG = 0x40002547");

 // STEP 14: Conditional interrupt wait based on event_counter
 if (event_counter <= 0x4) {
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 14)");
 break;
 }
 }
 }

 // STEP 15: Conditional interrupt wait based on event_counter (separate evaluation)
 if (event_counter <= 0x4) {
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 15)");
 break;
 }
 }
 }

 LOGT("USB FS Device Bulk Transfer test init complete");
 return 0;
}

/
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Executes USB FS Device Bulk Transfer test - enumeration, descriptor transfers, endpoint configuration, CDC bulk data transfer
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput result
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
 int rd_data;
 volatile unsigned int timeout;

 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run start");

 // STEP 16: DV debug marker write_reg(0xA0243ffc, 0xdeadbee0) replaced with PSV logging
 LOGD("DV debug marker reached: 0xdeadbee0 - enumeration start");

 // STEP 17: Read DCFG
 rd_data = read_reg(MIZAR_USB_DCFG);
 LOGD("DCFG = 0x%x", rd_data);

 // STEP 18: Read DSTS
 rd_data = read_reg(MIZAR_USB_DSTS);
 LOGD("DSTS = 0x%x", rd_data);

 // STEP 19: Write DCFG
 write_reg(MIZAR_USB_DCFG, 0x480801);

 // STEP 20: Write DCTL
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);

 // STEP 21: Write DALEPENA
 write_reg(MIZAR_USB_DALEPENA, 0xff);
 LOGD("Enumeration registers configured: DCFG=0x480801 DCTL=0x80f00a00 DALEPENA=0xff");

 // STEP 22: printf replaced with LOGD for PSV compatibility
 LOGD("Buffer_PointerLO is 0x%x", Buffer_PointerLO);

 // STEP 23: wait_on(5000) adapted to busy-wait
 for (volatile int d = 0; d < 500000; d++);

 // STEP 24: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 24)");
 out->status = -1;
 break;
 }
 }

 // STEP 25: GET DESCRIPTOR USB CONFIGURATION (short)
 setup_stage();

 // STEP 26: DV debug marker 0xdeadbee3 - not the exact prohibited 0xdeadbee0, preserving as-is
 // MANUAL_REVIEW: write_reg(0xA0243ffc, 0xdeadbee3) is a DV debug marker variant - verify if it should be preserved or converted
 write_reg(0xA0243ffc, 0xdeadbee3);

 // STEP 27: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 27)");
 out->status = -1;
 break;
 }
 }

 // STEP 28: Write event TRB pointer
 write_reg(event_trb_addr, Buffer_PointerLO);

 // STEP 29: Write event TRB size (short config descriptor)
 write_reg(event_trb_addr + 0x8, 0x9);

 // STEP 30: Write event TRB control
 write_reg(event_trb_addr + 0xc, 0x853);

 // STEP 31: Write short configuration descriptor data
 write_reg(Buffer_PointerLO, 0x003c0209);

 // STEP 32: Write DEPCMDPAR1 for endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);

 // STEP 33: Write DEPCMDPAR0 for endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 // STEP 34: Start Transfer on endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 // STEP 35: Read DEPCMD
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);

 // STEP 36: Poll DEPCMD until command completes
 timeout = 0;
 while (rd_data == 0x506) {
 // wait_on(10) adapted to busy-wait
 for (volatile int d = 0; d < 1000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout++;
 if (timeout > 10000) {
 LOGE("DEPCMD Start Transfer timeout at offset 0x10 (step 36)");
 out->status = -1;
 break;
 }
 }
 LOGD("DEPCMD+0x10 = 0x%x", rd_data);

 // STEP 37: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 37)");
 out->status = -1;
 break;
 }
 }

 // STEP 38: wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);

 // STEP 39: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 39)");
 out->status = -1;
 break;
 }
 }

 // STEP 40: Wait for interrupt - check xfernotready event
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 40 - xfernotready)");
 out->status = -1;
 break;
 }
 }

 // STEP 41: Status stage
 status_stage();

 // STEP 42: USB_SET_CONFIGURATION_OR_RESET_TT
 setup_stage();

 // STEP 43: DV debug marker 0xdeadbee4
 // MANUAL_REVIEW: write_reg(0xA0243ffc, 0xdeadbee4) is a DV debug marker variant - verify if it should be preserved or converted
 write_reg(0xA0243ffc, 0xdeadbee4);

 // STEP 44: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 44)");
 out->status = -1;
 break;
 }
 }

 // STEP 45: GET DESCRIPTOR USB CONFIGURATION (full)
 setup_stage();

 // STEP 46: DV debug marker 0xdeadbee5
 // MANUAL_REVIEW: write_reg(0xA0243ffc, 0xdeadbee5) is a DV debug marker variant - verify if it should be preserved or converted
 write_reg(0xA0243ffc, 0xdeadbee5);

 // STEP 47: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 47)");
 out->status = -1;
 break;
 }
 }

 // STEP 48: Data stage - write event TRB pointer
 write_reg(event_trb_addr, Buffer_PointerLO);

 // STEP 49: Write event TRB size (device descriptor)
 write_reg(event_trb_addr + 0x8, 0x12);

 // STEP 50: Write event TRB control
 write_reg(event_trb_addr + 0xc, 0x853);

 // STEP 51: Device Descriptor data
 write_reg(Buffer_PointerLO, 0x02000012);

 // STEP 52
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);

 // STEP 53
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);

 // STEP 54
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 // STEP 55
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);
 LOGD("Device descriptor data written");

 // STEP 56: Write DEPCMDPAR1 for endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);

 // STEP 57: Write DEPCMDPAR0 for endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 // STEP 58: Configuration descriptor data stage
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 LOGD("Configuration descriptor data written");

 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 // STEP 59: set_configuration(0x00, 0x200, 0x700, 0x401)
 set_configuration(0x00, 0x200, 0x700, 0x401);

 // STEP 60: set_configuration(0x10, 0x200, 0x2000700, 0x401)
 set_configuration(0x10, 0x200, 0x2000700, 0x401);

 // STEP 61: set_configuration(0x20, 0x206, 0x4000700, 0x401)
 set_configuration(0x20, 0x206, 0x4000700, 0x401);

 // STEP 62: set_configuration(0x30, 0x20206, 0x6000700, 0x401)
 set_configuration(0x30, 0x20206, 0x6000700, 0x401);

 // STEP 63: set_configuration(0x40, 0x204, 0x8000700, 0x401)
 set_configuration(0x40, 0x204, 0x8000700, 0x401);

 // STEP 64: set_configuration(0x50, 0x40204, 0xa000700, 0x401)
 set_configuration(0x50, 0x40204, 0xa000700, 0x401);

 // STEP 65: set_configuration(0x60, 0x1ffa, 0xc000700, 0x401)
 set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);

 // STEP 66: set_configuration(0x70, 0x61ffa, 0xe000700, 0x401)
 set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);
 LOGD("8 endpoints configured via set_configuration for CDC bulk transfer");

 // STEP 67: Bulk Transfer TRB setup (endpoint offset 0x40)
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);

 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 timeout = 0;
 while (rd_data == 0x506) {
 // wait_on(10) adapted to busy-wait
 for (volatile int d = 0; d < 1000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 timeout++;
 if (timeout > 10000) {
 LOGE("DEPCMD Start Transfer timeout at offset 0x40");
 out->status = -1;
 break;
 }
 }
 LOGD("DEPCMD+0x40 Start Transfer complete, rd_data=0x%x", rd_data);

 // Wait for interrupt after bulk endpoint 0x40 transfer
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt after bulk endpoint 0x40 transfer");
 out->status = -1;
 break;
 }
 }

 // STEP 68: Bulk Transfer DEPCMD (endpoint offset 0x50)
 write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
 timeout = 0;
 while (rd_data == 0x506) {
 // wait_on(10) adapted to busy-wait
 for (volatile int d = 0; d < 1000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
 timeout++;
 if (timeout > 10000) {
 LOGE("DEPCMD Start Transfer timeout at offset 0x50");
 out->status = -1;
 break;
 }
 }
 LOGD("DEPCMD+0x50 Start Transfer complete, rd_data=0x%x", rd_data);

 // Wait for interrupt after bulk endpoint 0x50 transfer
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt after bulk endpoint 0x50 transfer");
 out->status = -1;
 break;
 }
 }

 // STEP 69: DV debug marker 0xdeadbee7
 // MANUAL_REVIEW: write_reg(0xA0243ffc, 0xdeadbee7) is a DV debug marker variant - verify if it should be preserved or converted
 write_reg(0xA0243ffc, 0xdeadbee7);

 // STEP 70: Wait for interrupt
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 // wait_on(5) adapted to busy-wait
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt (step 70)");
 out->status = -1;
 break;
 }
 }

 // STEP 71: finish(0) converted to FV/PSV completion
 LOGT("USB FS Device Bulk Transfer test run complete: status=%d", out->status);
 return out->status;
}

/
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB FS Device Bulk Transfer test
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test teardown: no additional cleanup required");
 return 0;
}
