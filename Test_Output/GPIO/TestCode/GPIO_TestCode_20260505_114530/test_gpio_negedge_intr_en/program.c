// Author - AI Force 1.3.2. Date 05-05-2026
// (EMBENGG-SYSAPPS)

/* Test: test_gpio_negedge_intr_en
   Description (AS-IS): Verifies negative-edge interrupt generation for GPIOs 8–39, including raw status set/clear and group status handling with ISR.
   Acceptance Criteria (AS-IS):
   - Timeout must not occur while waiting for interrupt per pin; else test_err++.
   - In ISR: (rdata & 0x1) must be 0 (DIN low); (rdata & 0x2) must be non-zero (raw set) else test_err++.
   - Group status must indicate the pin, then clear per-pin raw and group raw, and verify gp0_intr1_intr_sts1 == 0.
   - System raw status must be cleared via MIZAR_LSS_SYSREG_RAW_STCR1.
   - Finish with finish(test_err).
*/

#include "test_define.c"

static volatile unsigned int test_err = 0;
static volatile unsigned int int_pend = 0;
static volatile unsigned int g_i = 0; /* tracks current pin index (0..31) */

/* ISR prototype assumed from platform; using Default_IRQHandler name per META */
void Default_IRQHandler(void)
{
    unsigned int local_wr = (1u << g_i);
    int_pend = 0; /* indicate ISR observed */

    /* Return pads to known state (drive high) */
    write_reg(0xA0243ffc, 0xFFFFFFFFu);

    /* Read back per-pin reg and validate DIN low and RAW set */
    unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (g_i * 4u));
    unsigned int rdata  = (unsigned int)read_reg(raddr);
    if ((rdata & 0x1u) != 0x0u) {
        test_err++; /* DIN not low */
    }

    if ((rdata & 0x2u) != 0x0u) {
        /* Group status must reflect the pin */
        unsigned int rdata_grp = (unsigned int)read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            test_err++;
        }
        /* Clear per-pin raw (keep input mode) -> iclr=1 (bit16) and doe=1 (bit20) per META */
        unsigned long raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (g_i * 4u));
        write_reg(raddr2, (1u << 20) | (1u << 16));
        /* Clear group raw for this pin */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
        /* Group status must be 0 after clear */
        rdata_grp = (unsigned int)read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            test_err++;
        }
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif
    } else {
        test_err++; /* raw not set */
    }
}

static void test_case(void)
{
    test_err = 0;

#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Drive all high (known state) */
    write_reg(0xA0243ffc, 0xFFFFFFFFu);

    /* Phase 1: per-pin configuration for neg-edge input + clear */
    for (g_i = 0; g_i < 32u; ++g_i) {
        unsigned long addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (g_i * 4u)); /* per-pin ctrl */
        write_reg(addr1, (1u << 20) | (1u << 18) | (1u << 16)); /* doe=1, neie=1, iclr=1 */
        wait_on(10);
    }

    /* Phase 2: per-pin interrupt generation */
    for (g_i = 0; g_i < 32u; ++g_i) {
        unsigned int wr_val = (1u << g_i);
        /* Clear any latched raw and enable this pin */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,  wr_val);
        wait_on(10);

        int_pend = 1u;
        /* Create falling edge for pin (8+g_i) */
        write_reg(0xA0243ffc, 0xFFFFFFFFu);
        wait_on(30);
        write_reg(0xA0243ffc, (~wr_val)); /* drive only this bit low */

        /* Wait for ISR */
        unsigned int timeout = 5000u;
        while ((int_pend == 1u) && (timeout-- > 0u)) {
            wait_on(10);
        }
        if (timeout == 0u) {
            printf("[TIMEOUT] Neg-edge interrupt not observed for pin %u\n", (g_i + 8u));
            test_err++;
        }
    }

    finish(test_err);
}

int main(void)
{
    test_case();
    return 0;
}
