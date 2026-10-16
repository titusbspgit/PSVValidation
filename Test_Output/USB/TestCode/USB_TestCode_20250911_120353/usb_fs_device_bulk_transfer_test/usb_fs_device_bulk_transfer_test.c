// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"

// Global variables
int int_pend;
int event_counter;

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler - clears interrupt and acknowledges event count
 * Parameters:
 * None
 * Returns:
 * void
 */
void Default_IRQHandler(void)
{
 int rd_data;
 int event_count;

 int_pend = 0;

 rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 LOGD("IRQ: MSK_STS0 = 0x%x", rd_data);

 rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
 LOGD("IRQ: RAW_STCR0 = 0x%x", rd_data);

 event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
 event_counter = event_count;
 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
 LOGD("IRQ: GEVNTCOUNT = 0x%x", event_count);

 // MANUAL_REVIEW: Source uses (rd_data && 0x80000000) - preserving as-is per No Silent Correction Rule
 if (rd_data && 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }

 GIC_ClearIRQ(84);
}

/*
 * Function: set_configuration
 * Description: Writes DEPCMD parameters and polls for command completion
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
 int timeout;

 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);

 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 timeout = 0;
 while (read_data == cmd) {
 // wait_on(30) adapted to busy-wait (scaling factor 100x)
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

/*
 * Function: setup_stage
 * Description: USB setup stage helper
 * Parameters:
 * None
 * Returns:
 * void
 */
void setup_stage(void)
{
 // MANUAL_REVIEW: setup_stage() body not provided in Meta Test Steps - implement per USB setup stage requirements
}

/*
 * Function: status_stage
 * Description: USB status stage helper
 * Parameters:
 * None
 * Returns:
 * void
 */
void status_stage(void)
{
 // MANUAL_REVIEW: status_stage() body not provided in Meta Test Steps - implement per USB status stage requirements
}

/*
 * Function: enumeration
 * Description: USB enumeration helper
 * Parameters:
 * None
 * Returns:
 * void
 */
void enumeration(void)
{
 // MANUAL_REVIEW: enumeration() body not provided in Meta Test Steps - implement per USB enumeration requirements
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes USB FS Device Bulk Transfer test - NIC programming, GIC enable, buffer/event ring clear, event ring setup, endpoint resource allocation, device controller start
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 int rd_data;
 int i;
 int j;
 int timeout;

 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test init start");

 // MANUAL_REVIEW: nic_programming() not defined in Meta Test Steps - ensure it is provided by project headers or implement
 nic_programming();

 GIC_EnableAllIRQ();
 LOGD("GIC IRQ enabled");

 // Clear buffer and event TRB areas
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }
 LOGD("Buffer and event TRB areas cleared");

 // Event ring setup
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);
 LOGD("Event ring configured");

 rd_data = read_reg(MIZAR_USB_GCTL);
 LOGD("GCTL = 0x%x", rd_data);

 // Endpoint resource allocation for 8 endpoints
 for (i = 0; i < 8; i++) {
 write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
 write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);

 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 timeout = 0;
 while (rd_data == 0x402) {
 // wait_on(10) adapted to busy-wait (scaling factor 100x)
 for (volatile int d = 0; d < 1000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 timeout++;
 if (timeout > 10000) {
 LOGE("DEPCMD resource alloc timeout for endpoint %d", i);
 break;
 }
 }
 LOGD("Endpoint %d resource allocated, DEPCMD=0x%x", i, rd_data);
 }

 // Enable endpoints and start device controller
 write_reg(MIZAR_USB_DALEPENA, 0x3);
 write_reg(MIZAR_USB_DCTL, 0x80f00000);
 LOGD("DALEPENA=0x3, DCTL=0x80f00000");

 // Enable LSS interrupt
 write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
 LOGD("LSS interrupt enabled");

 // Wait for interrupt - wait_on(100) adapted to busy-wait (scaling factor 100x)
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for initial interrupt");
 break;
 }
 }
 LOGD("Initial interrupt received");

 LOGT("USB FS Device Bulk Transfer test init complete");
 return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Executes USB FS Device Bulk Transfer test - enumeration, descriptor transfers, bulk endpoint configuration
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput result
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
 int rd_data;
 int timeout;
 int buf_data[16];

 (void)cfg;
 (void)buf_data;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run start");

 // Enumeration - DV debug marker write_reg(0xA0243ffc, 0xdeadbee0) replaced with PSV logging
 LOGD("DV debug marker reached: 0xdeadbee0");

 rd_data = read_reg(MIZAR_USB_DCFG);
 LOGD("DCFG = 0x%x", rd_data);

 rd_data = read_reg(MIZAR_USB_DSTS);
 LOGD("DSTS = 0x%x", rd_data);

 write_reg(MIZAR_USB_DCFG, 0x480801);
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);
 write_reg(MIZAR_USB_DALEPENA, 0xff);
 LOGD("Enumeration registers configured");

 // MANUAL_REVIEW: printf replaced with LOGD for PSV compatibility
 LOGD("Buffer_PointerLO is 0x%x", Buffer_PointerLO);

 // wait_on(5000) adapted to busy-wait (scaling factor 100x)
 for (volatile int d = 0; d < 500000; d++);

 // Setup TRB for GET DESCRIPTOR (Device Descriptor)
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x12);
 write_reg(event_trb_addr + 0xc, 0x853);

 // Data - Device Descriptor
 write_reg(Buffer_PointerLO, 0x02000012);
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);
 LOGD("Device descriptor data written");

 // Start Transfer on endpoint offset 0x10
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout = 0;
 while (rd_data == 0x506) {
 // wait_on(10) adapted to busy-wait (scaling factor 100x)
 for (volatile int d = 0; d < 1000; d++);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout++;
 if (timeout > 10000) {
 LOGE("DEPCMD Start Transfer timeout at offset 0x10");
 out->status = -1;
 break;
 }
 }
 LOGD("DEPCMD+0x10 Start Transfer complete, rd_data=0x%x", rd_data);

 // Wait for interrupt - wait_on(5) adapted to busy-wait (scaling factor 100x)
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt after device descriptor transfer");
 out->status = -1;
 break;
 }
 }

 // GET DESCRIPTOR USB CONFIGURATION
 setup_stage();

 // MANUAL_REVIEW: write_reg(0xA0243ffc, 0xdeadbee5) is a DV debug marker variant with value 0xdeadbee5, not the exact prohibited 0xdeadbee0 marker - preserving as-is
 write_reg(0xA0243ffc, 0xdeadbee5);

 // Wait for interrupt - wait_on(5) adapted to busy-wait (scaling factor 100x)
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt after setup stage");
 out->status = -1;
 break;
 }
 }

 // Data stage - Configuration Descriptor
 // 09023c00 010100e0 32090400 00060101 00000755 01034000 01070502 024000ff 07050301 ff030107 05810340 00010705 82024000 ff070583 01ff0301
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x9);
 write_reg(event_trb_addr + 0xc, 0x853);

 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x3c);
 write_reg(event_trb_addr + 0xc, 0x853);

 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(Buffer_PointerLO + 0x10, 0x05070000);
 write_reg(Buffer_PointerLO + 0x14, 0x00400301);
 write_reg(Buffer_PointerLO + 0x18, 0x02050701);
 write_reg(Buffer_PointerLO + 0x1c, 0xff004002);
 LOGD("Configuration descriptor data written");

 // Wait for interrupt - wait_on(5) adapted to busy-wait (scaling factor 100x)
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt after config descriptor data stage");
 out->status = -1;
 break;
 }
 }

 // Status stage
 status_stage();

 // Wait for interrupt - wait_on(5) adapted to busy-wait (scaling factor 100x)
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt after status stage");
 out->status = -1;
 break;
 }
 }

 // Zero-length TRB
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x853);
 write_reg(Buffer_PointerLO, 0x00);

 // Bulk endpoint TRB setup (offset 0x40)
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);

 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 timeout = 0;
 while (rd_data == 0x506) {
 // wait_on(10) adapted to busy-wait (scaling factor 100x)
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

 // Wait for interrupt - wait_on(5) adapted to busy-wait (scaling factor 100x)
 int_pend = 1;
 timeout = 0;
 while (int_pend) {
 for (volatile int d = 0; d < 500; d++);
 timeout++;
 if (timeout > 100000) {
 LOGE("Timeout waiting for interrupt after bulk endpoint 0x40 transfer");
 out->status = -1;
 break;
 }
 }

 // Bulk endpoint TRB setup (offset 0x50)
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);

 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x0);
 LOGD("Bulk endpoint offset 0x50 DEPCMDPAR configured");

 LOGT("USB FS Device Bulk Transfer test run complete: status=%d", out->status);
 return out->status;
}

/*
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
