// Author - AI Force 1.3.2. Date 29-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// Global test state
static volatile unsigned int int_pend = 0;           // Interrupt pending flag
static volatile unsigned int current_index = 0;      // Current GPIO index (0..31)
static int g_test_err = 0;                           // Global error counter

// Forward declaration of ISR handler used in polling context
static void Default_IRQHandler(void);

// wait_on, read_reg, write_reg, finish, GIC_* are expected from test_common.h via test_define.c includes

// ----------------------------------------------------------------------------
// Function: configure_per_pin_posedge
// Purpose : Configure per-pin control registers for positive-edge detection
// Notes   : Programs 32 pins starting at MIZAR_GPIO_GP0_GPIO_8 with stride 4
// ----------------------------------------------------------------------------
static void configure_per_pin_posedge(void)
{
    unsigned int i;
    for (i = 0; i < 32; i++) {
        unsigned long addr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8) + (i * 4u);
        write_reg(addr, 0x00020000u);  // Enable positive-edge detect (bit17)
        wait_on(10);
    }
}

// ----------------------------------------------------------------------------
// Function: configure_io_ctrl_groups
// Purpose : Configure IO control groups for pads 8..39
// ----------------------------------------------------------------------------
static void configure_io_ctrl_groups(void)
{
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);
}

// ----------------------------------------------------------------------------
// Function: Default_IRQHandler
// Purpose : Emulated ISR logic executed when group status indicates an interrupt
// ----------------------------------------------------------------------------
static void Default_IRQHandler(void)
{
    unsigned int rdata_grp;
    unsigned int rdata;
    unsigned int j;
    unsigned int local_wr = (1u << (current_index & 31u));

    // Clear pending flag first
    int_pend = 0u;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] ISR: Handling GPIO index=%u, local_wr=0x%08X\n", current_index, local_wr);
#endif

    // Read group status and mask group interrupts
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG][ERR] ISR: No group status set (STS1=0x%08X)\n", rdata_grp);
#endif
        g_test_err++;
    }

    // Clear per-pin raw status by writing iclr (bit16) to each pin control
    for (j = 0; j < 32; j++) {
        unsigned long raddr2 = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8) + (j * 4u);
        write_reg(raddr2, 0x00010000u);  // iclr
        wait_on(2);
    }

    // Verify group status cleared
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG][ERR] ISR: Group status not cleared (STS1=0x%08X)\n", rdata_grp);
#endif
        g_test_err++;
    }

    // Clear system-level RAW_STCR1 and verify deasserted
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG][ERR] ISR: RAW_STCR1 GPIO0 bit not cleared (0x%08X)\n", rdata);
#endif
        g_test_err++;
    }
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG][ERR] ISR: RAW_STCR1 GPIO1 bit not cleared (0x%08X)\n", rdata);
#endif
        g_test_err++;
    }
    GIC_ClearIRQ(88);
#endif

    // Re-enable group interrupts
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}

// ----------------------------------------------------------------------------
// Function: test_case
// Purpose : Entry point executing the positive-edge enable test across all pads
// Returns : 0 on success path (finish(0) called), 1 on failure path (finish(1))
// ----------------------------------------------------------------------------
int test_case(void)
{
    unsigned int i;

    // Initialization: enable GIC IRQs and SoC-level interrupts for GPIO block
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Start test_gpio_pedge_all_pads_en\n");
#endif

    // Configure per-pin positive-edge detection and IO control
    configure_per_pin_posedge();
    configure_io_ctrl_groups();

    // Enable all group interrupts
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Exercise each pin by generating a rising edge via pad control register
    for (i = 0; i < 32; i++) {
        unsigned int timeout = 2000u;
        current_index = i;              // Make index available to ISR

#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Iteration i=%u: drive pads low then high to create rising edge\n", i);
#endif

        // Drive pads low, wait, then set pending and drive high to create a rising edge
        write_reg(0xA0243ffcu, 0x00000000u);
        wait_on(10);
        int_pend = 1u;
        write_reg(0xA0243ffcu, 0xFFFFFFFFu);

        // Poll for interrupt using group status; call ISR when observed
        while ((int_pend == 1u) && (timeout > 0u)) {
            unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (sts != 0u) {
                Default_IRQHandler();
            } else {
                wait_on(10);
            }
            timeout--;
        }

        if (timeout == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][ERR] Timeout waiting for interrupt on GPIO%u\n", (i + 8u));
#endif
            g_test_err++;
            break; // As per steps, break on timeout
        }

        // Prepare for next iteration by driving pads low again
        write_reg(0xA0243ffcu, 0x00000000u);
        wait_on(10);
    }

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Test complete: errors=%d\n", g_test_err);
#endif

    if (g_test_err != 0) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }

    // Should not reach here; return value for completeness
    return (g_test_err != 0);
}
