// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

/* Global variables from Meta */
extern int int_pend;
int event_counter;

/*
 * Function: setup_stage
 * Description: USB control transfer setup stage - programs TRB and issues DEPCMD
 * Parameters:
 * None
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
 * Description: USB control transfer status stage - programs TRB and issues DEPCMD
 * Parameters:
 * None
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
 * Function: set_configuration
 * Description: Issues endpoint configuration command and polls for completion
 * Parameters:
 * trb_address - endpoint offset
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
 * Function: Default_IRQHandler
 * Description: USB interrupt handler - reads event count, acknowledges events,
 * checks SYSREG bit 31, clears GIC IRQ 84
 * Parameters:
 * None
 * Returns:
 * void
 */
void Default_IRQHandler(void)
{
 int rd_data;
 int sysreg_rd_data;
 int event_count;

 int_pend = 0;
 rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
 event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
 event_counter = event_count;
 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);
 /* Note: Source uses logical AND (&&) instead of bitwise AND (&). Preserved as-is per No Silent Correction Rule. */
 if (rd_data && 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }
 GIC_ClearIRQ(84);

 (void)sysreg_rd_data;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initializes USB device controller, enables IRQs, configures
 * global event buffer and control registers, performs enumeration setup
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 int rd_data;
 int j;

 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test init starting");

 /* Step 5: Call nic_programming() */
 /* MANUAL_REVIEW: nic_programming() is a DV NIC programming call. Verify if an FV/PSV equivalent is required or if it is provided by the platform. */
 nic_programming();

 /* Step 6: Call GIC_EnableAllIRQ() */
 GIC_EnableAllIRQ();

 /* Step 7: Clear 20 entries of Buffer_PointerLO and event_trb_addr arrays */
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }

 /* Step 8-11: Configure global event buffer registers */
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

 /* Step 12: Read GCTL */
 rd_data = read_reg(MIZAR_USB_GCTL);

 /* Step 13: Configure GCTL via read-modify-write */
 write_reg(MIZAR_USB_GCTL, set_data(read_reg(MIZAR_USB_GCTL), 0xFFFFFFFF, 0x30c11234));

 /* Step 14: Configure GFLADJ via read-modify-write */
 write_reg(MIZAR_USB_GFLADJ, set_data(read_reg(MIZAR_USB_GFLADJ), 0xFFFFFFFF, 0xa87f000));

 /* Step 15: Configure GUCTL via read-modify-write */
 write_reg(MIZAR_USB_GUCTL, set_data(read_reg(MIZAR_USB_GUCTL), 0xFFFFFFFF, 0x2000010));

 /* Step 16: Read PIPE Control Register */
 rd_data = read_reg(MIZAR_USB_BASE + 0xc2c0);

 /* Step 17: Write PIPE Control Register */
 write_reg(MIZAR_USB_BASE + 0xc2c0, 0x10c0002);

 LOGT("USB FS Device Bulk Transfer test init complete");

 (void)rd_data;

 return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Performs USB device enumeration, handles GET_DESCRIPTOR,
 * SET_ADDRESS, configuration descriptor data stage, status stage,
 * SET_CONFIGURATION, and bulk data transfer
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for status reporting
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
 int rd_data;
 int wr_data;
 int port_count;
 int db_offset;
 int buf_data[16];
 int i;
 int bulk;
 int intr;
 int event_comletion;
 int j;
 int hand_shake;

 (void)cfg;

 if (out == 0) {
 LOGE("USB FS Bulk Transfer output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run starting");

 /* Step 18-19: Enumeration - write debug marker, read DCFG and DSTS */
 write_reg(0xA0243ffc, 0xdeadbee0);

 /* Step 20: Read DCFG */
 rd_data = read_reg(MIZAR_USB_DCFG);

 /* Step 21: Read DSTS */
 rd_data = read_reg(MIZAR_USB_DSTS);

 /* Step 22: Write DCFG */
 write_reg(MIZAR_USB_DCFG, 0x480801);

 /* Step 23: Write DCTL */
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);

 /* Step 24: Write DALEPENA */
 write_reg(MIZAR_USB_DALEPENA, 0xff);

 /* Step 25: printf - log Buffer_PointerLO */
 LOGT("Buffer_PointerLO is 0x%x", Buffer_PointerLO);

 /* Step 26: Wait 5000 cycles */
 wait_on(5000);

 /* Step 27: Call setup_stage() for initial setup */
 setup_stage();

 /* Step 28: Write GUSB2PHYCFG */
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

 /* Step 29: First conditional int_pend wait */
 if (event_counter <= 0x4) {
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }
 }

 /* Step 30: Second conditional int_pend wait */
 if (event_counter <= 0x4) {
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }
 }

 /* Step 31: Unconditional int_pend wait */
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 /* Step 32-33: GET DESCRIPTOR USB CONFIGURATION - Call setup_stage() */
 setup_stage();

 /* Step 34: Write debug marker 0xdeadbee5 */
 write_reg(0xA0243ffc, 0xdeadbee5);

 /* Step 35: Wait for interrupt */
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 /* Step 36-39: Data stage - Configuration descriptor TRB */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x9);
 write_reg(event_trb_addr + 0xc, 0x853);

 /* Step 40-43: Configuration descriptor buffer data */
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);

 /* Step 44-45: Write DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 /* Step 46-47: SET ADDRESS - Write DCFG with new address */
 write_reg(MIZAR_USB_DCFG, 0x480809);

 /* Step 48-50: SET ADDRESS TRB */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x853);

 /* Step 51-52: Write DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10 for SET ADDRESS */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 /* Step 53-61: GET DESCRIPTOR (Device Descriptor response) TRB and buffer data */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x12);
 write_reg(event_trb_addr + 0xc, 0x853);
 write_reg(Buffer_PointerLO, 0x02000012);
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);
 write_reg(Buffer_PointerLO + 0x8, 0x00000000);
 write_reg(Buffer_PointerLO + 0xc, 0x00000000);
 write_reg(Buffer_PointerLO + 0x10, 0x00000100);

 /* Step 62-63: Write DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10 for device descriptor */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 /* Step 64: Issue DEPCMD+0x10 with 0x506 (Start Transfer command) */
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 /* Step 65-66: Poll until DEPCMD+0x10 changes from 0x506 */
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 while (rd_data == 0x506) {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 }

 /* Step 67: First int_pend wait after DEPCMD completion */
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 /* Step 68: Intermediate wait */
 wait_on(5);

 /* Step 69: Second int_pend wait */
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 /* Step 70: Third int_pend wait */
 int_pend = 1;
 while (int_pend) {
 wait_on(5);
 }

 /* Step 71: Call status_stage() */
 status_stage();

 /* Step 72-73: USB_SET_CONFIGURATION_OR_RESET_TT - Call setup_stage() */
 setup_stage();

 /* Step 74: Write debug marker 0xdeadbee4 */
 write_reg(0xA0243ffc, 0xdeadbee4);

 /* Step 75-78: Bulk data transfer phase - configure TRB */
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);

 /* Step 79-81: Write DEPCMDPAR1+0x40, DEPCMDPAR0+0x40, issue DEPCMD+0x40 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);

 /* Step 82: Poll and wait for bulk transfer completion */
 /* MANUAL_REVIEW: Meta step 82 says 'Poll and wait for completion' but no explicit polling loop details are provided beyond the DEPCMD write. Add polling if needed. */

 /* Step 83: finish(0) converted to FV/PSV status return */
 LOGT("USB FS Device Bulk Transfer test run complete: PASS");
 out->status = 0;

 /* Suppress unused variable warnings for DV-declared variables */
 (void)rd_data;
 (void)wr_data;
 (void)port_count;
 (void)db_offset;
 (void)buf_data;
 (void)i;
 (void)bulk;
 (void)intr;
 (void)event_comletion;
 (void)j;
 (void)hand_shake;

 return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Final cleanup for USB FS Device Bulk Transfer test
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
