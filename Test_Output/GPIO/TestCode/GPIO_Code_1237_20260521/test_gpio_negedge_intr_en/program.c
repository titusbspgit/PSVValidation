// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

/* Include only test_define.c as per rules */
#include "test_define.c"

/* ===============================================================
 * Function: Default_IRQHandler
 * Description: ISR logic for GPIO negedge interrupt enable test.
 * Mirrors Meta Test Steps without reordering or optimization.
 * =============================================================== */
void Default_IRQHandler(void)
{
    /* Cache the local write mask for this interrupt context */
    unsigned int local_wr = (1U << g_isr_i);
    isr_local_wr = local_wr;  /* for debug visibility */

    /* Mark interrupt observed */
    int_pend = 0;

    /* Drive pad data high before reading status */
    write_reg(PAD_DATA_REG, 0xFFFFFFFFU);

    /* Read back current pad register for the bit under test */
    unsigned int raddr = gpio_pad_addr[g_isr_i];
    unsigned int rdata = read_reg(raddr);

#ifdef DEBUG_DISPLAY
    printf("[ISR] GPIO idx=%u raddr=0x%08X rdata=0x%08X\n", g_isr_i, raddr, rdata);
#endif

    /* Expect bit0 (data_in) to be 0 at negedge; flag error if set */
    if ((rdata & 0x1U) != 0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][ERR] data_in bit0 expected 0, got 1 (idx=%u)\n", g_isr_i);
#endif
    }

    /* Expect bit1 (status/edge) to be non-zero to indicate event */
    if ((rdata & 0x2U) != 0x0U) {
        unsigned int rdata_grp = read_reg(gpio_ctrl_regs[IDX_INTR_STS1]);
#ifdef DEBUG_DISPLAY
        printf("[ISR] INTR1_STS1=0x%08X, expect bitmask 0x%08X set\n", rdata_grp, local_wr);
#endif
        if ((rdata_grp & local_wr) == 0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ISR][ERR] Expected status bit not set for idx=%u\n", g_isr_i);
#endif
        }

        /* Reprogram pad register (bits 20 and 16 set as per Meta) */
        unsigned int raddr2 = gpio_pad_addr[g_isr_i];
        write_reg(raddr2, ((1U << 20) | (1U << 16)));

        /* Clear raw pad interrupt for this bit */
        write_reg(gpio_ctrl_regs[IDX_RAW_STCLR1], local_wr);

        /* Confirm group status clears to 0 */
        rdata_grp = read_reg(gpio_ctrl_regs[IDX_INTR_STS1]);
        if (rdata_grp != 0x0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ISR][ERR] INTR1_STS1 not cleared: 0x%08X (idx=%u)\n", rdata_grp, g_isr_i);
#endif
        }

        /* Clear system raw status and GIC IRQ per instance */
#ifdef GPIO0
        write_reg(sysreg_regs[IDX_SYS_RAW_STCR1], LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        write_reg(sysreg_regs[IDX_SYS_RAW_STCR1], LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif
    } else {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][ERR] Status bit1 not set for idx=%u\n", g_isr_i);
#endif
    }
}

/* ===============================================================
 * Function: main
 * Description: Entry point executing the Meta Test Steps verbatim.
 * =============================================================== */
int main(void)
{
    /* Initialization */
    test_err = 0;

    /* Conditionally enable GIC IRQ for GPIO instance */
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    /* Conditionally enable system interrupt for GPIO instance */
#ifdef GPIO0
    write_reg(sysreg_regs[IDX_SYS_INTR_EN1], LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(sysreg_regs[IDX_SYS_INTR_EN1], LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Preload pad driver: drive all ones */
    write_reg(PAD_DATA_REG, 0xFFFFFFFFU);

    /* Configure pads: write bits 20, 18, 16 for each pad */
    for (unsigned int i = 0U; i < 32U; i++) {
        unsigned int addr1 = gpio_pad_addr[i];
        write_reg(addr1, ((1U << 20) | (1U << 18) | (1U << 16)));
        wait_on(10);
#ifdef DEBUG_DISPLAY
        printf("[MAIN] Config pad idx=%u addr=0x%08X\n", i, addr1);
#endif
    }

    /* For each bit i=0..31: clear raw, enable intr, create falling edge, wait for ISR */
    for (unsigned int i = 0U; i < 32U; i++) {
        unsigned int wr_val = (1U << i);

        /* Clear any pending raw pad interrupt for this bit */
        write_reg(gpio_ctrl_regs[IDX_RAW_STCLR1], wr_val);

        /* Enable the corresponding interrupt in INTR1_INTR_EN1 */
        write_reg(gpio_ctrl_regs[IDX_INTR_EN1], wr_val);
        wait_on(10);

        /* Prepare ISR context and trigger edge */
        int_pend = 1;
        g_isr_i = i;

        /* Drive high then low on the specific bit to create a falling edge */
        write_reg(PAD_DATA_REG, 0xFFFFFFFFU);
        wait_on(30);
        write_reg(PAD_DATA_REG, ~wr_val);

        /* Wait (with timeout) for ISR to clear int_pend */
        int timeout = 5000;
        while ((int_pend != 0) && (timeout-- > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[MAIN][ERR] Timeout waiting for IRQ on GPIO index %u (GPIO_%u)\n", i, (i + 8U));
#endif
        }
    }

    /* Termination with PASS/FAIL per acceptance criteria */
    if (test_err > 0) {
        finish(1);
    } else {
        finish(0);
    }

    /* Unreachable */
    return 0;
}
