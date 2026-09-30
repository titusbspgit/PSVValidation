// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

#define DWORD 4U
#define POLL_TIMEOUT 100000U

static volatile unsigned int int_pend;
static volatile unsigned int event_counter;
static unsigned int g_errors;

/*
 * Function: wait_for_interrupt
 * Description: Wait for interrupt by polling int_pend flag
 * Parameters:
 * None
 * Returns:
 * void
 */
static void wait_for_interrupt(void)
{
 unsigned int timeout = POLL_TIMEOUT;
 while (int_pend != 0U) {
 wait_on(5);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout waiting for interrupt");
 g_errors++;
 break;
 }
 }
}

/*
 * Function: set_configuration
 * Description: Configure endpoint by writing to DEPCMDPAR1, DEPCMDPAR0, DEPCMD and polling for completion
 * Parameters:
 * offset - endpoint register offset
 * par0_val - DEPCMDPAR0 value
 * par1_val - DEPCMDPAR1 value
 * cmd_val - DEPCMD command value
 * Returns:
 * void
 */
static void set_configuration(unsigned int offset, unsigned int par0_val, unsigned int par1_val, unsigned int cmd_val)
{
 unsigned int rd_data;
 unsigned int timeout = POLL_TIMEOUT;

 write_reg(MIZAR_USB_DEPCMDPAR1 + offset, par1_val);
 write_reg(MIZAR_USB_DEPCMDPAR0 + offset, par0_val);
 write_reg(MIZAR_USB_DEPCMD + offset, cmd_val);

 LOGT("set_configuration: offset=0x%x par0=0x%x par1=0x%x cmd=0x%x",
 offset, par0_val, par1_val, cmd_val);

 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + offset);
 timeout--;
 if (timeout == 0U) {
 LOGE("set_configuration: timeout polling DEPCMD at offset 0x%x", offset);
 g_errors++;
 break;
 }
 } while (rd_data == cmd_val);
}

/*
 * Function: setup_stage
 * Description: Set up a TRB for setup stage and issue Start Transfer command on endpoint 0
 * Parameters:
 * None
 * Returns:
 * void
 */
static void setup_stage(void)
{
 unsigned int rd_data;
 unsigned int timeout = POLL_TIMEOUT;

 LOGT("setup_stage: programming TRB at event_trb_addr");

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x8);
 write_reg(event_trb_addr + 0xc, 0x823);

 write_reg(MIZAR_USB_DEPCMDPAR1, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
 write_reg(MIZAR_USB_DEPCMD, 0x506);

 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD);
 timeout--;
 if (timeout == 0U) {
 LOGE("setup_stage: timeout polling DEPCMD");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);

 int_pend = 1;
 wait_for_interrupt();
}

/*
 * Function: status_stage
 * Description: Set up a TRB for status stage and issue Start Transfer command on endpoint 1
 * Parameters:
 * None
 * Returns:
 * void
 */
static void status_stage(void)
{
 unsigned int rd_data;
 unsigned int timeout = POLL_TIMEOUT;

 LOGT("status_stage: programming TRB at event_trb_addr");

 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x853);

 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout--;
 if (timeout == 0U) {
 LOGE("status_stage: timeout polling DEPCMD+0x10");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);

 int_pend = 1;
 wait_for_interrupt();
}

/*
 * Function: enumeration
 * Description: Perform USB enumeration sequence: GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR,
 * SET CONFIGURATION, GET FULL CONFIGURATION DESCRIPTOR
 * Parameters:
 * None
 * Returns:
 * void
 */
static void enumeration(void)
{
 unsigned int rd_data;
 unsigned int timeout;

 /* --- GET DEVICE DESCRIPTOR --- */
 setup_stage();

 write_reg(0xA0243ffc, 0xdeadbee1);
 LOGT("enumeration: marker 0xdeadbee1 written");

 int_pend = 1;
 wait_for_interrupt();

 /* Set up GET DEVICE DESCRIPTOR TRB */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x12);
 write_reg(event_trb_addr + 0xc, 0x853);

 /* Write device descriptor data to Buffer_PointerLO */
 write_reg(Buffer_PointerLO, 0x02000012);
 write_reg(Buffer_PointerLO + 0x4, 0x40000000);
 /* MANUAL_REVIEW: Meta describes device descriptor data as (0x02000012, 0x40000000, etc.) */
 /* Remaining device descriptor DWORDs not fully specified in Meta; preserving known values */

 /* Issue Start Transfer on MIZAR_USB_DEPCMD+0x10 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout--;
 if (timeout == 0U) {
 LOGE("enumeration: GET DEV DESC timeout polling DEPCMD+0x10");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);

 int_pend = 1;
 wait_for_interrupt();

 int_pend = 1;
 wait_for_interrupt();

 status_stage();

 /* --- GET CONFIGURATION DESCRIPTOR --- */
 setup_stage();

 write_reg(0xA0243ffc, 0xdeadbee3);
 LOGT("enumeration: marker 0xdeadbee3 written");

 /* Set up GET CONFIGURATION DESCRIPTOR TRB */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x9);
 write_reg(event_trb_addr + 0xc, 0x853);

 /* Write configuration descriptor data to Buffer_PointerLO */
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x4, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x8, 0x00000032);

 /* Issue Start Transfer on MIZAR_USB_DEPCMD+0x10 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout--;
 if (timeout == 0U) {
 LOGE("enumeration: GET CFG DESC timeout polling DEPCMD+0x10");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);

 int_pend = 1;
 wait_for_interrupt();

 int_pend = 1;
 wait_for_interrupt();

 status_stage();

 /* --- SET CONFIGURATION --- */
 setup_stage();

 write_reg(0xA0243ffc, 0xdeadbee4);
 LOGT("enumeration: marker 0xdeadbee4 written");

 /* Set up SET CONFIGURATION TRB */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x853);

 /* Write 0x00 to Buffer_PointerLO */
 write_reg(Buffer_PointerLO, 0x00);

 /* Issue Start Transfer on MIZAR_USB_DEPCMD+0x10 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout--;
 if (timeout == 0U) {
 LOGE("enumeration: SET CFG timeout polling DEPCMD+0x10");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);

 int_pend = 1;
 wait_for_interrupt();

 int_pend = 1;
 wait_for_interrupt();

 /* --- GET FULL CONFIGURATION DESCRIPTOR --- */
 setup_stage();

 write_reg(0xA0243ffc, 0xdeadbee5);
 LOGT("enumeration: marker 0xdeadbee5 written");

 /* Set up GET FULL CONFIGURATION DESCRIPTOR TRB */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x3c);
 write_reg(event_trb_addr + 0xc, 0x853);

 /* Write full configuration descriptor data (15 DWORDs) to Buffer_PointerLO */
 /* MANUAL_REVIEW: Meta specifies 15 DWORDs of full configuration descriptor data */
 /* but does not provide all 15 values explicitly. Preserving known partial data. */
 write_reg(Buffer_PointerLO, 0x003c0209);
 write_reg(Buffer_PointerLO + 0x04, 0xe0000101);
 write_reg(Buffer_PointerLO + 0x08, 0x00000032);
 /* MANUAL_REVIEW: DWORDs 4-15 of full configuration descriptor not fully specified in Meta */

 /* Issue Start Transfer on MIZAR_USB_DEPCMD+0x10 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);

 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout--;
 if (timeout == 0U) {
 LOGE("enumeration: GET FULL CFG DESC timeout polling DEPCMD+0x10");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);

 int_pend = 1;
 wait_for_interrupt();

 int_pend = 1;
 wait_for_interrupt();

 status_stage();
}

/*
 * Function: Default_IRQHandler
 * Description: IRQ handler for USB and sysreg interrupts
 * Parameters:
 * None
 * Returns:
 * void
 */
void Default_IRQHandler(void)
{
 unsigned int rd_data;
 unsigned int event_count;

 int_pend = 0;

 rd_data = read_reg(MIZAR_LSS_SYSREG_MSK_STS0);
 LOGT("IRQHandler: MSK_STS0=0x%x", rd_data);

 rd_data = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0);
 LOGT("IRQHandler: RAW_STCR0=0x%x", rd_data);

 event_count = read_reg(MIZAR_USB_GEVNTCOUNT);
 LOGT("IRQHandler: GEVNTCOUNT=0x%x", event_count);

 event_counter = event_count;

 write_reg(MIZAR_USB_GEVNTCOUNT, event_count);

 /* Preserve original Meta condition: rd_data && 0x80000000 */
 if (rd_data && 0x80000000) {
 write_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
 }

 GIC_ClearIRQ(84);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Initialize USB FS Device Bulk Transfer test
 * Parameters:
 * cfg - test configuration item
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem cfg)
{
 (void)cfg;

 g_errors = 0;
 int_pend = 0;
 event_counter = 0;

 LOGT("USB FS Device Bulk Transfer test init");

 /* Step 1: Call nic_programming() for NIC initialization */
 nic_programming();

 /* Step 2: Call GIC_EnableAllIRQ() to enable all IRQs */
 GIC_EnableAllIRQ();

 return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Execute USB FS Device Bulk Transfer test main flow
 * Parameters:
 * cfg - test configuration item
 * out - test output structure
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned int j;
 unsigned int i;
 unsigned int rd_data;
 unsigned int timeout;

 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("USB FS Device Bulk Transfer test run start");

 /* Step 3: Loop j=0 to 19: clear Buffer_PointerLO and event_trb_addr memory */
 for (j = 0; j < 20; j++) {
 write_reg(Buffer_PointerLO + j * DWORD, 0x0);
 write_reg(event_trb_addr + j * DWORD, 0x0);
 }
 LOGT("Buffer and event TRB memory cleared");

 /* Step 4: Write 0x40f00000 to MIZAR_USB_DCTL to initiate soft reset */
 write_reg(MIZAR_USB_DCTL, 0x40f00000);
 LOGT("Soft reset initiated: wrote 0x40f00000 to MIZAR_USB_DCTL");

 /* Step 5: Poll MIZAR_USB_DCTL until value equals 0xf00000 */
 timeout = POLL_TIMEOUT;
 do {
 wait_on(5);
 rd_data = read_reg(MIZAR_USB_DCTL);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout polling MIZAR_USB_DCTL for soft reset completion");
 g_errors++;
 break;
 }
 } while (rd_data != 0xf00000);
 LOGT("Soft reset complete: MIZAR_USB_DCTL=0x%x", rd_data);

 /* Step 6: Write 0x40002407 to MIZAR_USB_GUSB2PHYCFG */
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002407);
 LOGT("USB2 PHY configured: 0x40002407");

 /* Step 7: Write Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO */
 write_reg(MIZAR_USB_GEVNTADRLO, Default_Event_Ring_Array);

 /* Step 8: Write 0x0 to MIZAR_USB_GEVNTADRHI */
 write_reg(MIZAR_USB_GEVNTADRHI, 0x0);

 /* Step 9: Write 0x30 to MIZAR_USB_GEVNTSIZ */
 write_reg(MIZAR_USB_GEVNTSIZ, 0x30);

 /* Step 10: Write 0x0 to MIZAR_USB_GEVNTCOUNT */
 write_reg(MIZAR_USB_GEVNTCOUNT, 0x0);
 LOGT("Event ring configured");

 /* Step 11: Read MIZAR_USB_GCTL */
 rd_data = read_reg(MIZAR_USB_GCTL);
 LOGT("MIZAR_USB_GCTL read: 0x%x", rd_data);

 /* Step 12: Write 0x30c12214 to MIZAR_USB_GCTL */
 write_reg(MIZAR_USB_GCTL, 0x30c12214);
 LOGT("MIZAR_USB_GCTL written: 0x30c12214");

 /* Step 13: Read MIZAR_USB_DCFG */
 rd_data = read_reg(MIZAR_USB_DCFG);
 LOGT("MIZAR_USB_DCFG read: 0x%x", rd_data);

 /* Step 14: Write 0x480801 to MIZAR_USB_DCFG */
 write_reg(MIZAR_USB_DCFG, 0x480801);

 /* Step 15: Write 0x1f to MIZAR_USB_DEVTEN */
 write_reg(MIZAR_USB_DEVTEN, 0x1f);
 LOGT("Device events enabled: 0x1f");

 /* Step 16: Read MIZAR_USB_GUCTL */
 rd_data = read_reg(MIZAR_USB_GUCTL);
 LOGT("MIZAR_USB_GUCTL read: 0x%x", rd_data);

 /* Step 17: Write 0xa400010 to MIZAR_USB_GUCTL */
 write_reg(MIZAR_USB_GUCTL, 0xa400010);

 /* Step 18: set_configuration(0,0,0,0x409) - START NEW CONFIGURATION */
 set_configuration(0, 0, 0, 0x409);

 /* Step 19: set_configuration(0,0x200,0x700,0x401) - endpoint 0 */
 set_configuration(0, 0x200, 0x700, 0x401);

 /* Step 20: set_configuration(0x10,0x200,0x2000700,0x401) */
 set_configuration(0x10, 0x200, 0x2000700, 0x401);

 /* Step 21: set_configuration(0x20,0x206,0x4000700,0x401) */
 set_configuration(0x20, 0x206, 0x4000700, 0x401);

 /* Step 22: set_configuration(0x30,0x20206,0x6000700,0x401) */
 set_configuration(0x30, 0x20206, 0x6000700, 0x401);

 /* Step 23: set_configuration(0x40,0x204,0x8000700,0x401) */
 set_configuration(0x40, 0x204, 0x8000700, 0x401);

 /* Step 24: set_configuration(0x50,0x40204,0xa000700,0x401) */
 set_configuration(0x50, 0x40204, 0xa000700, 0x401);

 /* Step 25: set_configuration(0x60,0x1ffa,0xc000700,0x401) */
 set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);

 /* Step 26: set_configuration(0x70,0x61ffa,0xe000700,0x401) */
 set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

 LOGT("Endpoint configuration complete");

 /* Step 27: Loop i=0 to 7: TX resource configuration */
 for (i = 0; i < 8; i++) {
 write_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
 write_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);

 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + (i * 0x10));
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout polling DEPCMD for TX resource config, endpoint %u", i);
 g_errors++;
 break;
 }
 } while (rd_data == 0x402);
 }
 LOGT("TX resource configuration complete");

 /* Step 28: Write 0x3 to MIZAR_USB_DALEPENA */
 write_reg(MIZAR_USB_DALEPENA, 0x3);
 LOGT("DALEPENA=0x3: endpoints 0 and 1 enabled");

 /* Step 29: Write 0x80f00000 to MIZAR_USB_DCTL */
 write_reg(MIZAR_USB_DCTL, 0x80f00000);
 LOGT("Device controller started: DCTL=0x80f00000");

 /* Step 30: Write 0x80000000 to MIZAR_LSS_SYSREG_INTR_EN0 */
 write_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);
 LOGT("Sysreg interrupt enabled");

 /* Step 31: Set int_pend=1 and wait for interrupt */
 int_pend = 1;
 wait_for_interrupt();
 LOGT("Link state connect/reset event received");

 /* Step 32: If event_counter <= 0x4, wait for another interrupt */
 if (event_counter <= 0x4) {
 int_pend = 1;
 wait_for_interrupt();
 }

 /* Step 33: Write 0x480801 to MIZAR_USB_DCFG */
 write_reg(MIZAR_USB_DCFG, 0x480801);

 /* Step 34: If event_counter <= 0x4, wait for another interrupt */
 if (event_counter <= 0x4) {
 int_pend = 1;
 wait_for_interrupt();
 }

 /* Step 35: Write 0xdeadbee0 to 0xA0243ffc */
 write_reg(0xA0243ffc, 0xdeadbee0);
 LOGT("Enumeration marker 0xdeadbee0 written");

 /* Step 36: Read MIZAR_USB_DCFG */
 rd_data = read_reg(MIZAR_USB_DCFG);
 LOGT("MIZAR_USB_DCFG read: 0x%x", rd_data);

 /* Step 37: Read MIZAR_USB_DSTS */
 rd_data = read_reg(MIZAR_USB_DSTS);
 LOGT("MIZAR_USB_DSTS read: 0x%x", rd_data);

 /* Step 38: Write 0x480801 to MIZAR_USB_DCFG */
 write_reg(MIZAR_USB_DCFG, 0x480801);

 /* Step 39: Write 0x80f00a00 to MIZAR_USB_DCTL */
 write_reg(MIZAR_USB_DCTL, 0x80f00a00);

 /* Step 40: Write 0xff to MIZAR_USB_DALEPENA */
 write_reg(MIZAR_USB_DALEPENA, 0xff);
 LOGT("All physical endpoints enabled: DALEPENA=0xff");

 /* Step 41: Print Buffer_PointerLO value */
 LOGT("Buffer_PointerLO=0x%x", Buffer_PointerLO);

 /* Step 42: Call wait_on(5000) */
 wait_on(5000);

 /* Step 43: Call setup_stage() */
 setup_stage();
 LOGT("Initial setup_stage complete");

 /* Step 44: Write 0x40002547 to MIZAR_USB_GUSB2PHYCFG */
 write_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);
 LOGT("USB2 PHY reconfigured: 0x40002547");

 /* Step 45: If event_counter <= 0x4, wait for interrupt (twice) */
 if (event_counter <= 0x4) {
 int_pend = 1;
 wait_for_interrupt();

 int_pend = 1;
 wait_for_interrupt();
 }

 /* Step 46: Write 0x480809 to MIZAR_USB_DCFG (SET ADDRESS) */
 write_reg(MIZAR_USB_DCFG, 0x480809);
 LOGT("SET ADDRESS: DCFG=0x480809");

 /* Step 47: Set up TRB for SET ADDRESS status */
 write_reg(event_trb_addr, Buffer_PointerLO);
 write_reg(event_trb_addr + 0x8, 0x0);
 write_reg(event_trb_addr + 0xc, 0x853);

 /* Step 48: Write to DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

 /* Step 49: Write 0x506 to MIZAR_USB_DEPCMD+0x10, poll until not 0x506 */
 write_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x10);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout polling DEPCMD+0x10 for SET ADDRESS");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);

 /* Step 50: If event_counter <= 0x4, wait for interrupt */
 if (event_counter <= 0x4) {
 int_pend = 1;
 wait_for_interrupt();
 }

 /* Step 51: Set int_pend=1 and wait for interrupt */
 int_pend = 1;
 wait_for_interrupt();

 /* Step 52: Poll 0xa0243ff4 until non-zero (handshake) */
 LOGT("Polling 0xa0243ff4 for handshake");
 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(0xa0243ff4);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout polling 0xa0243ff4 for handshake");
 g_errors++;
 break;
 }
 } while (rd_data == 0);
 LOGT("Handshake at 0xa0243ff4 received: 0x%x", rd_data);

 /* Step 53: Call enumeration() */
 enumeration();
 LOGT("Enumeration complete");

 /* Step 54: Poll 0xa0243ff8 until non-zero (handshake) */
 LOGT("Polling 0xa0243ff8 for handshake");
 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(0xa0243ff8);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout polling 0xa0243ff8 for handshake");
 g_errors++;
 break;
 }
 } while (rd_data == 0);
 LOGT("Handshake at 0xa0243ff8 received: 0x%x", rd_data);

 /* Step 55: Write 0xdeadbee6 to 0xA0243ffc */
 write_reg(0xA0243ffc, 0xdeadbee6);
 LOGT("Marker 0xdeadbee6 written");

 /* Step 56: Set int_pend=1 and wait for interrupt */
 int_pend = 1;
 wait_for_interrupt();

 /* Step 57: Set up Bulk OUT TRB */
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);
 LOGT("Bulk OUT TRB programmed");

 /* Step 58: Write to DEPCMDPAR1+0x40 and DEPCMDPAR0+0x40 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

 /* Step 59: Write 0x506 to MIZAR_USB_DEPCMD+0x40, poll until not 0x506 */
 write_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x40);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout polling DEPCMD+0x40 for Bulk OUT");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);
 LOGT("Bulk OUT Start Transfer command complete");

 /* Step 60: Set int_pend=1 and wait for interrupt */
 int_pend = 1;
 wait_for_interrupt();

 /* Step 61: Set int_pend=1 and wait for interrupt */
 int_pend = 1;
 wait_for_interrupt();

 /* Step 62: Set up Bulk IN TRB */
 write_reg(event_trb_addr, Buffer_PointerLO_1);
 write_reg(event_trb_addr + 0x8, 0x40);
 write_reg(event_trb_addr + 0xc, 0x813);
 LOGT("Bulk IN TRB programmed");

 /* Step 63: Write to DEPCMDPAR1+0x50 and DEPCMDPAR0+0x50 */
 write_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, event_trb_addr);
 write_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x0);

 /* Step 64: Write 0x506 to MIZAR_USB_DEPCMD+0x50, poll until not 0x506 */
 write_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
 timeout = POLL_TIMEOUT;
 do {
 wait_on(10);
 rd_data = read_reg(MIZAR_USB_DEPCMD + 0x50);
 timeout--;
 if (timeout == 0U) {
 LOGE("Timeout polling DEPCMD+0x50 for Bulk IN");
 g_errors++;
 break;
 }
 } while (rd_data == 0x506);
 LOGT("Bulk IN Start Transfer command complete");

 /* Step 65: Set int_pend=1 and wait for interrupt */
 int_pend = 1;
 wait_for_interrupt();

 /* Step 66: Write 0xdeadbee7 to 0xA0243ffc (final marker) */
 write_reg(0xA0243ffc, 0xdeadbee7);
 LOGT("Final marker 0xdeadbee7 written");

 /* Step 67: Set int_pend=1 and wait for interrupt */
 int_pend = 1;
 wait_for_interrupt();

 /* Step 68: finish(0) converted to FV/PSV status return */
 /* MANUAL_REVIEW: DV finish(0) replaced with FV/PSV completion status */

 out->status = (g_errors == 0U) ? 0 : -1;

 LOGT("Run complete: %s errors=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 g_errors);

 return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Teardown for USB FS Device Bulk Transfer test
 * Parameters:
 * cfg - test configuration item
 * Returns:
 * FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("USB FS Device Bulk Transfer test teardown");
 return g_errors == 0U ? 0 : -1;
}
