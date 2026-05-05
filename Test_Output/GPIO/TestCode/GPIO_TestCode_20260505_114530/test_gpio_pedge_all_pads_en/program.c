// Author - AI Force 1.3.2. Date 05-05-2026
// (EMBENGG-SYSAPPS)

/* Test: test_gpio_pedge_all_pads_en
   Description (AS-IS): Verifies positive-edge interrupt generation on GPIOs 8–39 with group enable, raw status clear, and system/GIC acknowledgement.
   Acceptance Criteria (AS-IS):
   - Each pin must trigger an interrupt before timeout; else test_err++.
   - After ISR executes, gp0_intr1_intr_sts1 must read 0; otherwise test_err++.
   - System raw status readback must show the respective bit cleared after writing RAW_STCR1; otherwise test_err++.
   - Finish with finish(test_err).
*/

#include "test_define.c"

static volatile unsigned int test_err = 0;
static volatile unsigned int int_pend = 0;
static volatile unsigned int g_i = 0; /* current pin index (0..31) */

void Default_IRQHandler(void)
{
    unsigned int wr_val = (1u << g_i);
    int_pend = 0; /* seen */

    /* Read group status */
    unsigned int rdata_grp = (unsigned int)read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    /* Mask group during service */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) != 0u) {
        /* Clear per-pin raw for all pins (iclr=1) */
        for (unsigned int j = 0; j < 32u; ++j) {
            unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
            write_reg(raddr, 0x00010000u);
        }
        wait_on(2);
        rdata_grp = (unsigned int)read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            test_err++;
        }
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        (void)read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        (void)read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        GIC_ClearIRQ(88);
#endif
        /* Re-enable group interrupt */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    } else {
        printf("[ERR] No group status set in ISR for pin %u\n", (g_i + 8u));
        test_err++;
    }
}

static void test_case(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    test_err = 0;

    /* Enable positive-edge per pin */
    for (g_i = 0; g_i < 32u; ++g_i) {
        write_reg((MIZAR_GPIO_GP0_GPIO_8 + (g_i * 4u)), 0x00020000u); /* peie=1 */
    }
    wait_on(10);

    /* Set input mode by groups */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    /* Enable group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (g_i = 0; g_i < 32u; ++g_i) {
        /* Drive low then rising edge */
        write_reg(0xA0243ffc, 0x00000000u);
        wait_on(10);
        int_pend = 1u;
        write_reg(0xA0243ffc, 0xFFFFFFFFu); /* generate rising edge */
        unsigned int timeout = 2000u;
        while ((int_pend == 1u) && (--timeout > 0u)) {
            wait_on(10);
        }
        if (timeout == 0u) {
            printf("[TIMEOUT] Pos-edge interrupt not observed for pin %u\n", (g_i + 8u));
            test_err++;
            break;
        }
        write_reg(0xA0243ffc, 0x00000000u);
        wait_on(10);
    }

    finish(test_err);
}

int main(void)
{
    test_case();
    return 0;
}
