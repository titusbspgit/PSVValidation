// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// ------------------------------------------------------------
// Function: Default_IRQHandler
// Purpose : Handle GPIO negedge interrupt; validate per-pin and group status;
//           clear latched statuses and system interrupt; update error counter.
// ------------------------------------------------------------
void Default_IRQHandler(void)
{
    // Local state used for validation and clearing
    extern volatile unsigned int int_pend;
    extern volatile unsigned int g_curr_index; // current GPIO index under test [0..N-1]
    extern int test_err;

#ifdef DEBUG_DISPLAY
    printf("[DBG] ISR entered for index=%u\n", g_curr_index);
#endif

    unsigned int local_wr = (1U << g_curr_index); // Bit corresponding to current index

    // Mark interrupt handled progression and drive pads to safe level (all high)
    int_pend = 0U;
    write_reg(0xA0243FFC, 0xFFFFFFFFU);

    // Read per-pin control register and validate input/latched bits
    unsigned long raddr = addr_array[g_curr_index];
    unsigned int rdata = read_reg(raddr);

#ifdef DEBUG_DISPLAY
    printf("[DBG] ISR pin raddr=0x%08lx rdata=0x%08x\n", raddr, rdata);
#endif

    // Per-pin input should be low after falling edge
    if ((rdata & 0x1U) != 0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[DBG][ERR] Input not low after negedge (idx=%u)\n", g_curr_index);
#endif
    }

    // Check latched per-pin interrupt status (bit1)
    if ((rdata & 0x2U) != 0U) {
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[DBG][ERR] Group status bit not set (grp=0x%08x, bit=0x%08x)\n", rdata_grp, local_wr);
#endif
        }

        // Clear per-pin status: write bits 20 and 16 to the same pin control register
        write_reg(raddr, ((1U << 20) | (1U << 16)));

        // Clear RAW group status for this pin
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        // Verify group status cleared
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[DBG][ERR] Group status not cleared post-raw-clear (grp=0x%08x)\n", rdata_grp);
#endif
        }

        // Clear system raw interrupt and corresponding GIC IRQ
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif
    } else {
        // No per-pin latched status observed -> error
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[DBG][ERR] Per-pin latched status not set (idx=%u)\n", g_curr_index);
#endif
    }
}

// ------------------------------------------------------------
// Function: test_case
// Purpose : Program GPIO per-pin control, enable interrupts, generate negedge
//           per pin, wait for ISR to clear pending flag, track failures.
// ------------------------------------------------------------
void test_case(void)
{
    // Error tracking and synchronization flags
    int test_err = 0;
    volatile unsigned int timeout = 0U;

    // Expose to ISR
    volatile unsigned int int_pend = 0U;
    volatile unsigned int g_curr_index = 0U;

#ifdef DEBUG_DISPLAY
    printf("[DBG] test_gpio_negedge_intr_en: start\n");
#endif

    // Conditionally enable GIC IRQs for GPIO blocks
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    // Conditionally enable system interrupt for selected GPIO instance
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    // Initialize external pad control (drive all high)
    write_reg(0xA0243FFC, 0xFFFFFFFFU);

    // Configure per-pin control: set bits 20,18,16 for each impacted GPIO register
    unsigned int i;
    for (i = 0U; i < (sizeof(addr_array) / sizeof(addr_array[0])); i++) {
        unsigned long r = addr_array[i];
        write_reg(r, ((1U << 20) | (1U << 18) | (1U << 16)));
#ifdef DEBUG_DISPLAY
        printf("[DBG] Config pin idx=%u addr=0x%08lx val=0x%08x\n", i, r, ((1U << 20) | (1U << 18) | (1U << 16)));
#endif
        wait_on(10);
    }

    // For each pin, enable and stimulate falling-edge interrupt then wait for ISR
    for (i = 0U; i < (sizeof(addr_array) / sizeof(addr_array[0])); i++) {
        unsigned int wr_val = (1U << i);

        // Clear any previous raw group status for this bit
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);

        // Enable group interrupt for this bit
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        // Prepare ISR synchronization
        g_curr_index = i;
        int_pend = 1U;

        // Drive pads high, then toggle to create falling edge on target bit
        write_reg(0xA0243FFC, 0xFFFFFFFFU);
        wait_on(30);
        write_reg(0xA0243FFC, ~wr_val);

        // Poll for ISR to clear pending flag with timeout
        timeout = 5000U;
        while ((int_pend != 0U) && (timeout-- > 0U)) {
            wait_on(10);
        }
        if (timeout == 0U) {
            // Timeout indicates ISR did not service interrupt
            printf("ERROR: Timeout waiting for GPIO%u negedge interrupt\n", (i + 8U));
            test_err++;
        }
    }

    // Final verdict
    if (test_err == 0) {
        finish(0);
    } else {
        finish(1);
    }
}
