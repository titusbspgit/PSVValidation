// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

/* Global variables from DV source */
extern int int_pend;
int event_counter;

/*
 * Function: set_configuration
 * Description: Issue a DEPCMD configuration command and poll for completion.
 * Parameters:
 * trb_address - endpoint register offset
 * parameter0 - DEPCMDPAR0 value
 * parameter1 - DEPCMDPAR1 value
 * cmd - DEPCMD command value
 * Returns:
 * void
 */
static void set_configuration(int trb_address, int parameter0, int parameter1, int cmd)
{
 int read_data;
 write_reg(MIZAR_USB_DEPCMDPAR1 + trb_address, parameter1);
 write_reg(MIZAR_USB_DEPCMDPAR0 + trb_address, parameter0);
 write_reg(MIZAR_USB_DEPCMD + trb_address, cmd);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 while (read_data == cmd) {
 wait_on(30);
 read_data = read_reg(MIZAR_USB_DEPCMD + trb_address);
 }
}

/*
 * Function: setup_stage
 * Description: Submit a setup-stage TRB with control 0x823 and poll DEPCMD for completion.
 * Parameters:
 * none
 * Returns:
 * void
 */
static void setup_stage(void)
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
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 }
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }
}

/*
 * Function: status_stage
 * Description: Submit a status-stage TRB with control 0x843 and poll DEPCMD for completion.
 * Parameters:
 * none
 * Returns:
 * void
 */
static void status_stage(void)
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
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 }
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }
 wait_on(5);
}

/*
 * Function: Default_IRQHandler
 * Description: USB interrupt handler. Clears int_pend, reads system and event
 * registers, acknowledges event count, clears system interrupt,
 * and clears GIC IRQ 84.
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
 /* NOTE: Source uses logical AND (&&) not bitwise AND (&). Preserved as-is per No Silent Correction Rule. */
 if (rd_data && 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }
 GIC_ClearIRQ(84);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initialize USB FS device bulk transfer test. Calls nic_programming,
 * enables GIC IRQs, clears buffer and event TRB memory, programs
 * event ring and global/device configuration registers, configures
 * endpoint TX resources, enables endpoints, and starts the controller.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 int rd_data;
 int wr_data;
 int i;
 int j;

 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test init start");

 /* Step 1: Call nic_programming() to configure the NIC */
 nic_programming();

 /* Step 2: Enable all IRQs via GIC_EnableAllIRQ() */
 GIC_EnableAllIRQ();

 /* Step 3: Clear Buffer_PointerLO and event_trb_addr memory regions (20 DWORDs each) to 0x0 */
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }

 /* Step 4: Program event ring registers */
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

 /* Step 5: Read GCTL, then write 0x30c12214 to GCTL */
 rd_data = read_reg(MIZAR_USB_GCTL);
 wr_data = 0x30c12214;
 write_reg(MIZAR_USB_GCTL, wr_data);

 /* Step 6: Read DCFG, then write 0x480801 to DCFG */
 rd_data = read_reg(MIZAR_USB_DCFG);
 write_reg(MIZAR_USB_DCFG, 0x480801);

 /* Step 7: Write 0x1f to DEVTEN to enable device events */
 write_reg(MIZAR_USB_DEVTEN, 0x1f);

 /* Step 8: Read GUCTL, then write 0xa400010 to GUCTL */
 rd_data = read_reg(MIZAR_USB_GUCTL);
 write_reg(MIZAR_USB_GUCTL, 0xa400010);

 /* Step 9: Call set_configuration(0, 0, 0, 0x409) to start new configuration */
 set_configuration(0, 0, 0, 0x409);

 /* Step 10: Configure TX resources for 8 endpoints (i=0..7) */
 for (i = 0; i < 8; i++) {
 write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
 write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 while (rd_data == 0x402) {
 wait_on(30);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 }
 }

 /* Step 11: Write 0x3 to DALEPENA to enable physical endpoints 0 and 1 */
 write_reg(MIZAR_USB_DALEPENA, 0x3);

 /* Step 12: Write 0x80f00000 to DCTL to start the device controller */
 write_reg(MIZAR_USB_DCTL, 0x80f00000);

 LOGT("USB FS Device Bulk Transfer test init complete");

 (void)rd_data;
 (void)wr_data;

 return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Execute USB FS device bulk transfer test. Performs enumeration,
 * GET_DESCRIPTOR (Device), SET_ADDRESS, GET_DESCRIPTOR (Configuration),
 * and bulk data transfer.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput result
 * Returns:
 * 0 on success (PASS), -1 on failure (FAIL)
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
 int rd_data;
 int i;

 (void)cfg;

 if (out == 0) {
 LOGE("USB FS Device Bulk Transfer: output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run start");

 /* Step 13: Enumeration */
 /* Debug marker 0xdeadbee0 at 0xA0243ffc - DV debug marker, converted to PSV logging */
 LOGD("DV debug marker reached");

 rd_data = read_reg(MIZAR_USB_DCFG);
 rd_data = read_reg(MIZAR_USB_DSTS);
 LOGT("Enumeration: DSTS read = 0x%x", rd_data);
 write_reg(MIZAR_USB_DCFG, 0x480801);
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);
 write_reg(MIZAR_USB_DALEPENA, 0xff);
 LOGT("Buffer_PointerLO is 0x%x", Buffer_PointerLO);
 wait_on(5000);

 /* Step 14: GET_DESCRIPTOR (Device) TRB setup */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x12);
 write_reg(event_trb_addr + 0xc, 0x853);

 /* Write device descriptor data to Buffer_PointerLO */
 write_reg(Buffer_PointerLO, 0x02000012);
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);

 /* Submit via DEPCMDPAR1/DEPCMDPAR0 at offset 0x10 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 while (rd_data == 0x506) {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 /* Wait for 3 interrupt-driven int_pend loops */
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 /* Call status_stage() */
 status_stage();

 /* Step 15: SET_ADDRESS */
 write_reg(MIZAR_USB_DCFG, 0x480809);
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x853);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 /* Step 16: GET_DESCRIPTOR (Configuration) */
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 setup_stage();

 /* Debug marker 0xdeadbee5 at 0xA0243ffc - DV debug marker, converted to PSV logging */
 LOGD("DV debug marker reached");

 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 /* Data stage - configuration descriptor data */
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 /* Step 17: Bulk data transfer at endpoint offset 0x40 */
 /* MANUAL_REVIEW: Meta Test Steps lines 86-87 specify bulk data transfer with TRB control 0x813 */
 /* at endpoint offset 0x40 and DEPCMD 0x506 at offset 0x40, but the exact TRB buffer pointer, */
 /* transfer length, and complete TRB fields are not fully specified in Meta. */
 /* Implementing the bulk TRB submission and polling based on available Meta information. */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0); /* MANUAL_REVIEW: bulk transfer length not specified in Meta */
 write_reg(event_trb_addr + 0xc, 0x813);
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 while (rd_data == 0x506) {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 }

 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 LOGT("USB FS Device Bulk Transfer test run complete");

 out->status = 0;

 (void)rd_data;

 return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB FS device bulk transfer test.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test teardown: no additional cleanup required");
 return 0;
}
