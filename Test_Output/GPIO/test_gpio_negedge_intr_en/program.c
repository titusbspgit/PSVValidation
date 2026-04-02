#include "test_define.c"

/* Test: Negative-edge interrupt enable/service on GPIO8..GPIO39
 * - Configure per-pin as input with negedge enable and raw clear
 * - Generate falling edge per pin via 0xA0243FFC drive register
 * - Verify DIN low on negedge, group status set, clear paths work
 * - Bounded waits with timeouts; on any failure increment test_err
 */

static volatile int test_err = 0;
static volatile int int_pend = 0;
static volatile unsigned int curr_pin_idx = 0;

void Default_IRQHandler(void)
{
    unsigned int local_wr = (1u << curr_pin_idx);
    unsigned long raddr;
    unsigned int rdata;
    unsigned int rdata_grp;

    int_pend = 0; /* interrupt observed */

    /* Return pads to known-high state */
    write_reg(0xA0243FFC, 0xFFFFFFFFu);

    /* Per-pin status readback */
    raddr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + (curr_pin_idx * 4u));
    rdata = (unsigned int)read_reg(raddr);

    /* DIN should be 0 on negedge */
    if ((rdata & 0x1u) != 0u) {
        test_err++;
    }

    /* Check per-pin/event status via group register */
    if ((rdata & 0x2u) != 0u) {
        rdata_grp = (unsigned int)read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            test_err++;
        }

        /* Per-pin clear: iclr=1; also raw W1C clear in group */
        write_reg((unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + (curr_pin_idx * 4u)), (1u<<20) | (1u<<16));
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        /* Verify group cleared */
        rdata_grp = (unsigned int)read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0u) {
            test_err++;
        }

        /* Clear sysreg and GIC (GPIO0 assumed) */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
    } else {
        /* Event bit not set when expected */
        test_err++;
    }
}

void test_case(void)
{
    unsigned int i;

    test_err = 0;

    /* Enable GIC and sysreg interrupt (GPIO0 assumed) */
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);

    /* Drive all pads high (known state) */
    write_reg(0xA0243FFC, 0xFFFFFFFFu);

    /* Configure per-pin input + negedge + raw clear: doe=1, neie=1, iclr=1 */
    for (i = 0; i < 32u; ++i) {
        unsigned long addr1 = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, (1u<<20) | (1u<<18) | (1u<<16));
        wait_on(10);
    }

    /* Per-pin: generate falling edge and wait with timeout */
    for (i = 0; i < 32u; ++i) {
        unsigned int wr_val = (1u << i);
        unsigned int timeout = 5000u;

        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);           /* pre-clear W1C */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);              /* enable only this bit */
        wait_on(10);

        curr_pin_idx = i;
        int_pend = 1;

        /* Prepare rising then drop only this bit to create negedge */
        write_reg(0xA0243FFC, 0xFFFFFFFFu);
        wait_on(30);
        write_reg(0xA0243FFC, 0xFFFFFFFFu ^ wr_val);                   /* falling edge on bit i */

        while (int_pend && (timeout-- > 0u)) {
            wait_on(10);
        }
        if (timeout == 0u) {
            printf("Timeout waiting for GPIO negedge IRQ on bit %u\n", i);
            test_err++;
        }

#ifdef DEBUG_DISPLAY
        if (!int_pend) {
            printf("GPIO negedge IRQ serviced for bit %u\n", i);
        }
#endif

        /* Deassert enable for this bit before next iteration */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);
        wait_on(5);
    }

#ifdef DEBUG_DISPLAY
    if (test_err == 0) {
        printf("test_gpio_negedge_intr_en: PASS\n");
    } else {
        printf("test_gpio_negedge_intr_en: FAIL (errors=%d)\n", test_err);
    }
#endif

    finish((test_err == 0) ? 0 : 1);
}
