// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "usb_fs_device_bulk_transfer_test.h"
#include "test_define.inc"

/*
 * USB_FS_Device_Bulk_Transfer_test
 * Validates USB Full-Speed (FS) Device mode Bulk Transfer operation.
 * Implements all 68 test steps including soft reset, PHY config, event ring
 * setup, endpoint configuration, USB enumeration, and Bulk OUT/IN transfers
 * with interrupt-driven event handling.
 */

/* Testcase context */
typedef struct {
    unsigned int errors;
} usb_bulk_test_ctx_t;

static usb_bulk_test_ctx_t g_ctx;

/* Global variables used by test steps and IRQ handler */
static volatile int int_pend = 0;
static volatile unsigned int event_counter = 0;
static unsigned int rd_data = 0;
static unsigned int event_count = 0;

/*
 * Helper Function: set_configuration
 * Writes DEPCMDPAR1, DEPCMDPAR0, DEPCMD at parameterized offsets and polls
 * for command completion.
 */
static void set_configuration(unsigned int offset, unsigned int par0_val, unsigned int par1_val, unsigned int cmd_val)
{
    LOGT("set_configuration: offset=0x%x PAR0=0x%x PAR1=0x%x CMD=0x%x",
         offset, par0_val, par1_val, cmd_val);

    writel_reg(MIZAR_USB_DEPCMDPAR1 + offset, par1_val);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + offset, par0_val);
    writel_reg(MIZAR_USB_DEPCMD + offset, cmd_val);

    rd_data = readl_reg(MIZAR_USB_DEPCMD + offset);
    while (rd_data == cmd_val) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + offset);
    }
    LOGT("set_configuration complete at offset 0x%x, DEPCMD=0x%x", offset, rd_data);
}

/*
 * Helper Function: setup_stage
 * Sets up a TRB at event_trb_addr pointing to Buffer_PointerLO with size 0x8
 * and control 0x823, issues Start Transfer command (0x506) to MIZAR_USB_DEPCMD,
 * polls for completion, then waits for interrupt.
 */
static void setup_stage(void)
{
    LOGT("setup_stage: Setting up TRB for setup stage");

    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x8, 0x8);
    writel_reg(event_trb_addr + 0xc, 0x823);

    writel_reg(MIZAR_USB_DEPCMDPAR1, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    writel_reg(MIZAR_USB_DEPCMD, 0x506);

    rd_data = readl_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD);
    }
    LOGT("setup_stage: Start Transfer complete, DEPCMD=0x%x", rd_data);

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("setup_stage: Interrupt received");
}

/*
 * Helper Function: status_stage
 * Sets up a status TRB and issues a Start Transfer command with interrupt waits.
 */
static void status_stage(void)
{
    LOGT("status_stage: Setting up status TRB");

    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x8, 0x0);
    writel_reg(event_trb_addr + 0xc, 0x853);

    writel_reg(MIZAR_USB_DEPCMDPAR1, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0, 0x0);
    writel_reg(MIZAR_USB_DEPCMD, 0x506);

    rd_data = readl_reg(MIZAR_USB_DEPCMD);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD);
    }
    LOGT("status_stage: Start Transfer complete");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("status_stage: First interrupt received");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("status_stage: Second interrupt received");
}

/*
 * Helper Function: enumeration
 * Performs GET DEVICE DESCRIPTOR, GET CONFIGURATION DESCRIPTOR,
 * SET CONFIGURATION, and GET FULL CONFIGURATION DESCRIPTOR phases.
 */
static void enumeration(void)
{
    LOGT("enumeration: Starting USB enumeration sequence");

    /* ---- GET DEVICE DESCRIPTOR ---- */
    LOGT("enumeration: GET DEVICE DESCRIPTOR - setup_stage");
    setup_stage();

    writel_reg(0xA0243ffc, 0xdeadbee1);
    LOGT("enumeration: Write marker 0xdeadbee1");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }

    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x8, 0x12);
    writel_reg(event_trb_addr + 0xc, 0x853);

    writel_reg(Buffer_PointerLO, 0x02000012);
    writel_reg(Buffer_PointerLO + 0x4, 0x40000000);
    writel_reg(Buffer_PointerLO + 0x8, 0x00000000);
    writel_reg(Buffer_PointerLO + 0xc, 0x00000000);
    writel_reg(Buffer_PointerLO + 0x10, 0x00000100);
    LOGT("enumeration: Device descriptor data written");

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    writel_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    }
    LOGT("enumeration: GET DEVICE DESCRIPTOR Start Transfer complete");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
    }

    LOGT("enumeration: GET DEVICE DESCRIPTOR - status_stage");
    status_stage();

    /* ---- GET CONFIGURATION DESCRIPTOR ---- */
    LOGT("enumeration: GET CONFIGURATION DESCRIPTOR - setup_stage");
    setup_stage();

    writel_reg(0xA0243ffc, 0xdeadbee3);
    LOGT("enumeration: Write marker 0xdeadbee3");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }

    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x8, 0x9);
    writel_reg(event_trb_addr + 0xc, 0x853);

    writel_reg(Buffer_PointerLO, 0x003c0209);
    writel_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    writel_reg(Buffer_PointerLO + 0x8, 0x00000032);
    LOGT("enumeration: Configuration descriptor data written");

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    writel_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    }
    LOGT("enumeration: GET CONFIGURATION DESCRIPTOR Start Transfer complete");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
    }

    LOGT("enumeration: GET CONFIGURATION DESCRIPTOR - status_stage");
    status_stage();

    /* ---- SET CONFIGURATION ---- */
    LOGT("enumeration: SET CONFIGURATION - setup_stage");
    setup_stage();

    writel_reg(0xA0243ffc, 0xdeadbee4);
    LOGT("enumeration: Write marker 0xdeadbee4");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }

    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x8, 0x0);
    writel_reg(event_trb_addr + 0xc, 0x853);

    writel_reg(Buffer_PointerLO, 0x00);
    LOGT("enumeration: SET CONFIGURATION data written");

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    writel_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    }
    LOGT("enumeration: SET CONFIGURATION Start Transfer complete");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
    }

    /* ---- GET FULL CONFIGURATION DESCRIPTOR ---- */
    LOGT("enumeration: GET FULL CONFIGURATION DESCRIPTOR - setup_stage");
    setup_stage();

    writel_reg(0xA0243ffc, 0xdeadbee5);
    LOGT("enumeration: Write marker 0xdeadbee5");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }

    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x8, 0x3c);
    writel_reg(event_trb_addr + 0xc, 0x853);

    writel_reg(Buffer_PointerLO, 0x003c0209);
    writel_reg(Buffer_PointerLO + 0x4, 0xe0000101);
    writel_reg(Buffer_PointerLO + 0x8, 0x00000032);
    writel_reg(Buffer_PointerLO + 0xc, 0x00020904);
    writel_reg(Buffer_PointerLO + 0x10, 0x00030200);
    writel_reg(Buffer_PointerLO + 0x14, 0x02050700);
    writel_reg(Buffer_PointerLO + 0x18, 0x00004002);
    writel_reg(Buffer_PointerLO + 0x1c, 0x05070000);
    writel_reg(Buffer_PointerLO + 0x20, 0x00400282);
    writel_reg(Buffer_PointerLO + 0x24, 0x07000000);
    writel_reg(Buffer_PointerLO + 0x28, 0x40030205);
    writel_reg(Buffer_PointerLO + 0x2c, 0x00000000);
    writel_reg(Buffer_PointerLO + 0x30, 0x03820507);
    writel_reg(Buffer_PointerLO + 0x34, 0x00000040);
    writel_reg(Buffer_PointerLO + 0x38, 0x00000000);
    LOGT("enumeration: Full configuration descriptor data (15 DWORDs) written");

    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);
    writel_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    }
    LOGT("enumeration: GET FULL CONFIGURATION DESCRIPTOR Start Transfer complete");

    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
    }

    LOGT("enumeration: GET FULL CONFIGURATION DESCRIPTOR - status_stage");
    status_stage();

    LOGT("enumeration: USB enumeration sequence complete");
}

/*
 * IRQ Handler: Default_IRQHandler
 * Interrupt handling with GIC_ClearIRQ(84)
 */
void Default_IRQHandler(void)
{
    int_pend = 0;

    rd_data = readl_reg(MIZAR_LSS_SYSREG_MSK_STS0);
    rd_data = readl_reg(MIZAR_LSS_SYSREG_RAW_STCR0);

    event_count = readl_reg(MIZAR_USB_GEVNTCOUNT);
    event_counter = event_count;
    writel_reg(MIZAR_USB_GEVNTCOUNT, event_count);

    if (rd_data & 0x80000000) {
        writel_reg(MIZAR_LSS_SYSREG_RAW_STCR0, 0x80000000);
    }

    GIC_ClearIRQ(84);
}

/*
 * Function: usb_fs_device_bulk_transfer_test_init
 * Description: Performs testcase initialization and pre-condition setup for usb_fs_device_bulk_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (usb_bulk_test_ctx_t){0};

    LOGT("USB FS Device Bulk Transfer test init");

    /* Step 1: Call nic_programming() for NIC initialization */
    LOGT("Step 1: NIC programming initialization");
    nic_programming();

    /* Step 2: Call GIC_EnableAllIRQ() to enable all IRQs */
    LOGT("Step 2: Enable all GIC IRQs");
    GIC_EnableAllIRQ();

    return 0;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_run
 * Description: Executes the main testcase flow for usb_fs_device_bulk_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out)
{
    int j, i;

    (void)cfg;

    if (out == 0) {
        LOGE("USB FS Device Bulk Transfer output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting USB FS Device Bulk Transfer test run");

    /* Step 3: Clear buffer and event TRB memory (20 entries) */
    LOGT("Step 3: Clearing buffer and event TRB memory (20 entries)");
    for (j = 0; j < 20; j++) {
        writel_reg(Buffer_PointerLO + j * 4, 0x0);
        writel_reg(event_trb_addr + j * 4, 0x0);
    }

    /* Step 4: Write 0x40f00000 to MIZAR_USB_DCTL to initiate soft reset */
    LOGT("Step 4: Initiate USB controller soft reset");
    writel_reg(MIZAR_USB_DCTL, 0x40f00000);

    /* Step 5: Poll MIZAR_USB_DCTL until value equals 0xf00000 */
    LOGT("Step 5: Polling MIZAR_USB_DCTL for soft reset completion");
    rd_data = readl_reg(MIZAR_USB_DCTL);
    while (rd_data != 0xf00000) {
        wait_on(5);
        rd_data = readl_reg(MIZAR_USB_DCTL);
    }
    LOGT("Step 5: Soft reset complete, MIZAR_USB_DCTL=0x%x", rd_data);

    /* Step 6: Write 0x40002407 to MIZAR_USB_GUSB2PHYCFG */
    LOGT("Step 6: Configure USB2 PHY");
    writel_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002407);

    /* Step 7: Write Default_Event_Ring_Array to MIZAR_USB_GEVNTADRLO */
    LOGT("Step 7: Set event ring address low");
    writel_reg(MIZAR_USB_GEVNTADRLO, (unsigned int)Default_Event_Ring_Array);

    /* Step 8: Write 0x0 to MIZAR_USB_GEVNTADRHI */
    LOGT("Step 8: Set event ring address high = 0x0");
    writel_reg(MIZAR_USB_GEVNTADRHI, 0x0);

    /* Step 9: Write 0x30 to MIZAR_USB_GEVNTSIZ */
    LOGT("Step 9: Set event ring size = 0x30");
    writel_reg(MIZAR_USB_GEVNTSIZ, 0x30);

    /* Step 10: Write 0x0 to MIZAR_USB_GEVNTCOUNT */
    LOGT("Step 10: Clear event count register");
    writel_reg(MIZAR_USB_GEVNTCOUNT, 0x0);

    /* Step 11: Read MIZAR_USB_GCTL */
    LOGT("Step 11: Read MIZAR_USB_GCTL");
    rd_data = readl_reg(MIZAR_USB_GCTL);
    LOGT("Step 11: MIZAR_USB_GCTL = 0x%x", rd_data);

    /* Step 12: Write 0x30c12214 to MIZAR_USB_GCTL */
    LOGT("Step 12: Configure GCTL for port direction");
    writel_reg(MIZAR_USB_GCTL, 0x30c12214);

    /* Step 13: Read MIZAR_USB_DCFG */
    LOGT("Step 13: Read MIZAR_USB_DCFG");
    rd_data = readl_reg(MIZAR_USB_DCFG);
    LOGT("Step 13: MIZAR_USB_DCFG = 0x%x", rd_data);

    /* Step 14: Write 0x480801 to MIZAR_USB_DCFG */
    LOGT("Step 14: Configure device settings");
    writel_reg(MIZAR_USB_DCFG, 0x480801);

    /* Step 15: Write 0x1f to MIZAR_USB_DEVTEN */
    LOGT("Step 15: Enable device events (DEVTEN=0x1f)");
    writel_reg(MIZAR_USB_DEVTEN, 0x1f);

    /* Step 16: Read MIZAR_USB_GUCTL */
    LOGT("Step 16: Read MIZAR_USB_GUCTL");
    rd_data = readl_reg(MIZAR_USB_GUCTL);
    LOGT("Step 16: MIZAR_USB_GUCTL = 0x%x", rd_data);

    /* Step 17: Write 0xa400010 to MIZAR_USB_GUCTL */
    LOGT("Step 17: Configure user control settings");
    writel_reg(MIZAR_USB_GUCTL, 0xa400010);

    /* Step 18: START NEW CONFIGURATION command */
    LOGT("Step 18: set_configuration - START NEW CONFIGURATION");
    set_configuration(0, 0, 0, 0x409);

    /* Step 19: Endpoint 0 configuration */
    LOGT("Step 19: set_configuration - Endpoint 0");
    set_configuration(0, 0x200, 0x700, 0x401);

    /* Step 20: Endpoint at offset 0x10 */
    LOGT("Step 20: set_configuration - Endpoint offset 0x10");
    set_configuration(0x10, 0x200, 0x2000700, 0x401);

    /* Step 21: Endpoint at offset 0x20 */
    LOGT("Step 21: set_configuration - Endpoint offset 0x20");
    set_configuration(0x20, 0x206, 0x4000700, 0x401);

    /* Step 22: Endpoint at offset 0x30 */
    LOGT("Step 22: set_configuration - Endpoint offset 0x30");
    set_configuration(0x30, 0x20206, 0x6000700, 0x401);

    /* Step 23: Endpoint at offset 0x40 */
    LOGT("Step 23: set_configuration - Endpoint offset 0x40");
    set_configuration(0x40, 0x204, 0x8000700, 0x401);

    /* Step 24: Endpoint at offset 0x50 */
    LOGT("Step 24: set_configuration - Endpoint offset 0x50");
    set_configuration(0x50, 0x40204, 0xa000700, 0x401);

    /* Step 25: Endpoint at offset 0x60 */
    LOGT("Step 25: set_configuration - Endpoint offset 0x60");
    set_configuration(0x60, 0x1ffa, 0xc000700, 0x401);

    /* Step 26: Endpoint at offset 0x70 */
    LOGT("Step 26: set_configuration - Endpoint offset 0x70");
    set_configuration(0x70, 0x61ffa, 0xe000700, 0x401);

    /* Step 27: TX resource allocation for 8 endpoints */
    LOGT("Step 27: Allocate TX resources for 8 endpoints");
    for (i = 0; i < 8; i++) {
        writel_reg(MIZAR_USB_DEPCMDPAR0 + (i * 0x10), 0x1);
        writel_reg(MIZAR_USB_DEPCMD + (i * 0x10), 0x402);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        while (rd_data == 0x402) {
            wait_on(10);
            rd_data = readl_reg(MIZAR_USB_DEPCMD + (i * 0x10));
        }
        LOGT("Step 27: TX resource allocated for endpoint %d", i);
    }

    /* Step 28: Write 0x3 to MIZAR_USB_DALEPENA */
    LOGT("Step 28: Enable physical endpoints 0 and 1 (DALEPENA=0x3)");
    writel_reg(MIZAR_USB_DALEPENA, 0x3);

    /* Step 29: Write 0x80f00000 to MIZAR_USB_DCTL */
    LOGT("Step 29: Start device controller (DCTL=0x80f00000)");
    writel_reg(MIZAR_USB_DCTL, 0x80f00000);

    /* Step 30: Write 0x80000000 to MIZAR_LSS_SYSREG_INTR_EN0 */
    LOGT("Step 30: Enable sysreg interrupt");
    writel_reg(MIZAR_LSS_SYSREG_INTR_EN0, 0x80000000);

    /* Step 31: Wait for interrupt (link state connect/reset events) */
    LOGT("Step 31: Waiting for link state connect/reset event interrupt");
    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("Step 31: Link state event interrupt received");

    /* Step 32: If event_counter <= 0x4, wait for another interrupt */
    LOGT("Step 32: Check event_counter and wait if needed");
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
        LOGT("Step 32: Additional interrupt received");
    }

    /* Step 33: Write 0x480801 to MIZAR_USB_DCFG */
    LOGT("Step 33: Rewrite MIZAR_USB_DCFG = 0x480801");
    writel_reg(MIZAR_USB_DCFG, 0x480801);

    /* Step 34: If event_counter <= 0x4, wait for another interrupt */
    LOGT("Step 34: Check event_counter and wait if needed");
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
        LOGT("Step 34: Additional interrupt received");
    }

    /* Step 35: Write 0xdeadbee0 to 0xA0243ffc */
    LOGT("Step 35: Write enumeration marker 0xdeadbee0");
    writel_reg(0xA0243ffc, 0xdeadbee0);

    /* Step 36: Read MIZAR_USB_DCFG */
    LOGT("Step 36: Read MIZAR_USB_DCFG");
    rd_data = readl_reg(MIZAR_USB_DCFG);
    LOGT("Step 36: MIZAR_USB_DCFG = 0x%x", rd_data);

    /* Step 37: Read MIZAR_USB_DSTS */
    LOGT("Step 37: Read MIZAR_USB_DSTS");
    rd_data = readl_reg(MIZAR_USB_DSTS);
    LOGT("Step 37: MIZAR_USB_DSTS = 0x%x", rd_data);

    /* Step 38: Write 0x480801 to MIZAR_USB_DCFG */
    LOGT("Step 38: Rewrite MIZAR_USB_DCFG = 0x480801");
    writel_reg(MIZAR_USB_DCFG, 0x480801);

    /* Step 39: Write 0x80f00a00 to MIZAR_USB_DCTL */
    LOGT("Step 39: Write MIZAR_USB_DCTL = 0x80f00a00");
    writel_reg(MIZAR_USB_DCTL, 0x80f00a00);

    /* Step 40: Write 0xff to MIZAR_USB_DALEPENA */
    LOGT("Step 40: Enable all physical endpoints (DALEPENA=0xff)");
    writel_reg(MIZAR_USB_DALEPENA, 0xff);

    /* Step 41: Print Buffer_PointerLO value */
    LOGT("Step 41: Buffer_PointerLO = 0x%x", (unsigned int)Buffer_PointerLO);

    /* Step 42: Call wait_on(5000) */
    LOGT("Step 42: wait_on(5000)");
    wait_on(5000);

    /* Step 43: Call setup_stage() */
    LOGT("Step 43: setup_stage()");
    setup_stage();

    /* Step 44: Write 0x40002547 to MIZAR_USB_GUSB2PHYCFG */
    LOGT("Step 44: Reconfigure USB2 PHY (GUSB2PHYCFG=0x40002547)");
    writel_reg(MIZAR_USB_GUSB2PHYCFG, 0x40002547);

    /* Step 45: If event_counter <= 0x4, wait for interrupt (twice) */
    LOGT("Step 45: Check event_counter and wait for interrupts if needed");
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
        LOGT("Step 45: Two additional interrupts received");
    }

    /* Step 46: Write 0x480809 to MIZAR_USB_DCFG (SET ADDRESS) */
    LOGT("Step 46: SET ADDRESS - Write MIZAR_USB_DCFG = 0x480809");
    writel_reg(MIZAR_USB_DCFG, 0x480809);

    /* Step 47: Set up TRB for SET ADDRESS */
    LOGT("Step 47: Set up TRB for SET ADDRESS");
    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO);
    writel_reg(event_trb_addr + 0x8, 0x0);
    writel_reg(event_trb_addr + 0xc, 0x853);

    /* Step 48: Write to DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10 */
    LOGT("Step 48: Write DEPCMDPAR1+0x10 and DEPCMDPAR0+0x10");
    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x10, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x10, 0x0);

    /* Step 49: Write 0x506 to DEPCMD+0x10, poll until not 0x506 */
    LOGT("Step 49: Start Transfer on endpoint 1 (DEPCMD+0x10)");
    writel_reg(MIZAR_USB_DEPCMD + 0x10, 0x506);
    rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x10);
    }
    LOGT("Step 49: Start Transfer complete on endpoint 1");

    /* Step 50: If event_counter <= 0x4, wait for interrupt */
    LOGT("Step 50: Check event_counter and wait if needed");
    if (event_counter <= 0x4) {
        int_pend = 1;
        while (int_pend == 1) {
            wait_on(5);
        }
        LOGT("Step 50: Interrupt received");
    }

    /* Step 51: Wait for interrupt */
    LOGT("Step 51: Wait for interrupt");
    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("Step 51: Interrupt received");

    /* Step 52: Poll 0xa0243ff4 until non-zero (handshake) */
    LOGT("Step 52: Polling handshake register 0xa0243ff4");
    rd_data = readl_reg(0xa0243ff4);
    while (rd_data == 0x0) {
        wait_on(5);
        rd_data = readl_reg(0xa0243ff4);
    }
    LOGT("Step 52: Handshake register 0xa0243ff4 = 0x%x (non-zero)", rd_data);

    /* Step 53: Call enumeration() */
    LOGT("Step 53: Call enumeration()");
    enumeration();

    /* Step 54: Poll 0xa0243ff8 until non-zero (handshake) */
    LOGT("Step 54: Polling handshake register 0xa0243ff8");
    rd_data = readl_reg(0xa0243ff8);
    while (rd_data == 0x0) {
        wait_on(5);
        rd_data = readl_reg(0xa0243ff8);
    }
    LOGT("Step 54: Handshake register 0xa0243ff8 = 0x%x (non-zero)", rd_data);

    /* Step 55: Write 0xdeadbee6 to 0xA0243ffc */
    LOGT("Step 55: Write marker 0xdeadbee6");
    writel_reg(0xA0243ffc, 0xdeadbee6);

    /* Step 56: Wait for interrupt */
    LOGT("Step 56: Wait for interrupt");
    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("Step 56: Interrupt received");

    /* Step 57: Set up Bulk OUT TRB */
    LOGT("Step 57: Set up Bulk OUT TRB");
    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO_1);
    writel_reg(event_trb_addr + 0x8, 0x40);
    writel_reg(event_trb_addr + 0xc, 0x813);

    /* Step 58: Write to DEPCMDPAR1+0x40 and DEPCMDPAR0+0x40 */
    LOGT("Step 58: Write DEPCMDPAR1+0x40 and DEPCMDPAR0+0x40 for Bulk OUT");
    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x40, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x40, 0x0);

    /* Step 59: Write 0x506 to DEPCMD+0x40, poll until not 0x506 */
    LOGT("Step 59: Start Transfer on Bulk OUT endpoint (DEPCMD+0x40)");
    writel_reg(MIZAR_USB_DEPCMD + 0x40, 0x506);
    rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x40);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x40);
    }
    LOGT("Step 59: Bulk OUT Start Transfer complete");

    /* Step 60: Wait for interrupt */
    LOGT("Step 60: Wait for interrupt (Bulk OUT)");
    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("Step 60: Bulk OUT interrupt received");

    /* Step 61: Wait for interrupt */
    LOGT("Step 61: Wait for second interrupt (Bulk OUT)");
    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("Step 61: Bulk OUT second interrupt received");

    /* Step 62: Set up Bulk IN TRB */
    LOGT("Step 62: Set up Bulk IN TRB");
    writel_reg(event_trb_addr, (unsigned int)Buffer_PointerLO_1);
    writel_reg(event_trb_addr + 0x8, 0x40);
    writel_reg(event_trb_addr + 0xc, 0x813);

    /* Step 63: Write to DEPCMDPAR1+0x50 and DEPCMDPAR0+0x50 */
    LOGT("Step 63: Write DEPCMDPAR1+0x50 and DEPCMDPAR0+0x50 for Bulk IN");
    writel_reg(MIZAR_USB_DEPCMDPAR1 + 0x50, (unsigned int)event_trb_addr);
    writel_reg(MIZAR_USB_DEPCMDPAR0 + 0x50, 0x0);

    /* Step 64: Write 0x506 to DEPCMD+0x50, poll until not 0x506 */
    LOGT("Step 64: Start Transfer on Bulk IN endpoint (DEPCMD+0x50)");
    writel_reg(MIZAR_USB_DEPCMD + 0x50, 0x506);
    rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x50);
    while (rd_data == 0x506) {
        wait_on(10);
        rd_data = readl_reg(MIZAR_USB_DEPCMD + 0x50);
    }
    LOGT("Step 64: Bulk IN Start Transfer complete");

    /* Step 65: Wait for interrupt */
    LOGT("Step 65: Wait for interrupt (Bulk IN)");
    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("Step 65: Bulk IN interrupt received");

    /* Step 66: Write 0xdeadbee7 to 0xA0243ffc (final marker) */
    LOGT("Step 66: Write final marker 0xdeadbee7");
    writel_reg(0xA0243ffc, 0xdeadbee7);

    /* Step 67: Wait for final interrupt */
    LOGT("Step 67: Wait for final interrupt");
    int_pend = 1;
    while (int_pend == 1) {
        wait_on(5);
    }
    LOGT("Step 67: Final interrupt received");

    /* Step 68: DV finish(0) converted to PSV/FV status reporting */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to FV-native out->status PASS reporting.
    out->status = 0;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: usb_fs_device_bulk_transfer_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for usb_fs_device_bulk_transfer_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("USB FS Device Bulk Transfer teardown: no additional cleanup required");
    return g_ctx.errors == 0U ? 0 : -1;
}
