#include "test_define.c"

/*
Testcase: test_gpio_pedge_all_pads_en
Description (from Hidden_Test_Description):
- Enable IRQs for GPIO0/GPIO1.
- Enable system-level GPIO interrupt in LSS SYSREG.
- Program all 32 GPIO pins to rising-edge detection and IO control groups to 0x000000FF.
- Enable group interrupt mask.
- For each pad: drive low then high using GPIO_TEST_TOGGLE_REG with waits and a timeout loop waiting for ISR to clear int_pend.
- ISR reads group status, masks and clears, per-pin clear sequence, verifies group cleared, clears RAW_STCR1, re-enables mask, clears GIC lines.
- Finish with test_err-based pass/fail.
*/

static volatile int test_err = 0;
static volatile int int_pend = 0;

void Default_IRQHandler(void)
{
    unsigned int rdata_grp = 0;
    int j = 0;

    int_pend = 0; // indicate interrupt observed

    // Mask group interrupt
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); // GP0_INTR1_INTR_EN1 mask all

    // Read group status
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); // GP0_INTR1_INTR_STS1 status
    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        test_err++;
    }

    // Per-pin clear sequence: write 0x00010000 to GP0_GPIO_8..GP0_GPIO_39
    for (j = 0; j < 32; j++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4), 0x00010000u); // clear per-pin pending
    }

    wait_on(2);

    // Verify group status is 0 after per-pin clear
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000u) {
        test_err++;
    }

#ifdef GPIO0
    // RAW clear verify for GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    unsigned int rdata0 = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata0 & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
        test_err++;
    }
#endif
#ifdef GPIO1
    // RAW clear verify for GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    unsigned int rdata1 = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata1 & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
        test_err++;
    }
#endif

    // Re-enable group interrupt
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif
}

void test_case(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
    // Enable GPIO0 interrupt at system level
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    // Enable GPIO1 interrupt at system level
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    // Configure all 32 GPIO pins to rising edge detect
    for (int i = 0; i < 32; i++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00020000u); // set rising edge
    }

    wait_on(10);

    // IO control groups to 0x000000FF
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);

    wait_on(10);

    // Enable group interrupt mask
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Exercise all 32 pads: low -> wait -> set pend -> high -> wait for ISR -> low -> wait
    for (int i = 0; i < 32; i++) {
        // Drive low
        write_reg(GPIO_TEST_TOGGLE_REG, 0x00000000u);
        wait_on(10);

        // Arm and drive high
        int_pend = 1;
        write_reg(GPIO_TEST_TOGGLE_REG, 0xFFFFFFFFu);

        int timeout = 2000;
        while ((int_pend == 1) && (--timeout > 0)) {
            wait_on(10);
        }

        if (timeout == 0) {
            test_err++;
            break; // stop on timeout as per steps
        }

        // Drive low again
        write_reg(GPIO_TEST_TOGGLE_REG, 0x00000000u);
        wait_on(10);
    }

    if (test_err > 0) {
        finish(1);
    } else {
        finish(0);
    }
}
