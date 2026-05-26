// Author - AI Force 1.3.2. Date 26-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// -----------------------------------------------------------------------------
// Function: static void drive_pad_high(void)
// Purpose : Drive the GPIO pad output bus high to prepare for edge generation
// -----------------------------------------------------------------------------
static void drive_pad_high(void)
{
    // Write all ones to the pad output register to hold lines high
    write_reg(PAD_OUT_REG_ADDR, 0xFFFFFFFFU);
#ifdef DEBUG_DISPLAY
    printf("[DBG] drive_pad_high(): PAD_OUT_REG_ADDR=0x%08lX <- 0x%08X\n", (unsigned long)PAD_OUT_REG_ADDR, 0xFFFFFFFFU);
#endif
}

// -----------------------------------------------------------------------------
// Function: static void generate_falling_edge(unsigned int bit)
// Purpose : Create a falling edge on the selected GPIO line by pulling only
//           that bit low while keeping all others high
// -----------------------------------------------------------------------------
static void generate_falling_edge(unsigned int bit)
{
    unsigned int wr_val = (1U << bit);
    // Ensure bus is high first, then pull selected bit low to create negedge
    write_reg(PAD_OUT_REG_ADDR, 0xFFFFFFFFU);
    write_reg(PAD_OUT_REG_ADDR, ~wr_val);
#ifdef DEBUG_DISPLAY
    printf("[DBG] generate_falling_edge(): bit=%u, wr_val=0x%08X, PAD_OUT_REG_ADDR=0x%08lX\n",
           bit, wr_val, (unsigned long)PAD_OUT_REG_ADDR);
#endif
}

// Shared state for polling from test_case and clearing in Default_IRQHandler
static volatile unsigned int g_test_err = 0U;           // cumulative test failures
static volatile unsigned int g_isr_err = 0U;            // failures detected inside ISR
static volatile unsigned int g_timeout_err = 0U;        // timeout failures while polling
static volatile int g_int_pend = 0;                     // pending interrupt flag (cleared by ISR)

// -----------------------------------------------------------------------------
// Function: void Default_IRQHandler(void)
// Purpose : Service GPIO interrupt, validate per-pin and group status, and clear
//           sources per Meta description. Does not alter functionality under
//           DEBUG_DISPLAY (only prints).
// -----------------------------------------------------------------------------
void Default_IRQHandler(void)
{
    unsigned int sts = read_reg(gp0_intr_sts1_reg);
#ifdef DEBUG_DISPLAY
    printf("[DBG][ISR] STS1=0x%08X\n", sts);
#endif

    if (sts == 0U) {
        // Spurious/cleared interrupt; mark no pending and exit
        g_int_pend = 0;
        return;
    }

    for (unsigned int i = 0U; i < 32U; ++i) {
        unsigned int mask = (1U << i);
        if ((sts & mask) == 0U) {
            continue; // not our bit
        }

        uintptr_t raddr = gp0_pin_reg_addr[i];
        unsigned int rdata = read_reg(raddr);
#ifdef DEBUG_DISPLAY
        printf("[DBG][ISR] bit=%u raddr=0x%08lX rdata=0x%08X\n", i, (unsigned long)raddr, rdata);
#endif
        // Check data_in bit0 must be 0 on falling edge sample per Meta
        if ((rdata & 0x1U) != 0U) {
            g_isr_err++;
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] bit%u: data_in(bit0) expected 0 on negedge, got 1\n", i);
#endif
        }

        // If bit1 indicates edge/latched condition, group status must reflect it
        if ((rdata & 0x2U) != 0x0U) {
            unsigned int rdata_grp = read_reg(gp0_intr_sts1_reg);
            if ((rdata_grp & mask) == 0U) {
                g_isr_err++;
#ifdef DEBUG_DISPLAY
                printf("[ERR][ISR] bit%u: INTR_STS1 did not reflect latched edge (grp=0x%08X)\n", i, rdata_grp);
#endif
            }
        }

        // Clear per-pin latched/edge conditions and RAW status for this bit
        write_reg(raddr, (1U << 20) | (1U << 16));       // clear bits per Meta
        write_reg(gp0_rawstcr1_reg, mask);               // clear RAW status for this bit

        unsigned int rdata_grp_post = read_reg(gp0_intr_sts1_reg);
        if ((rdata_grp_post & mask) != 0U) {
            g_isr_err++;
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] bit%u: INTR_STS1 not cleared (post=0x%08X)\n", i, rdata_grp_post);
#endif
        }
    }

    // Restore outputs high and clear pending flag
    drive_pad_high();
    g_int_pend = 0;
}

// -----------------------------------------------------------------------------
// Function: static void configure_per_pin_for_negedge(void)
// Purpose : Configure each per-pin GPIO control register for negedge enable and
//           clear raw status as per Meta description
// -----------------------------------------------------------------------------
static void configure_per_pin_for_negedge(void)
{
    // Bits: (1<<20)|(1<<18)|(1<<16) per Meta Test Description
    const unsigned int cfg = (1U << 20) | (1U << 18) | (1U << 16);
    for (unsigned int i = 0U; i < 32U; ++i) {
        write_reg(gp0_pin_reg_addr[i], cfg);
#ifdef DEBUG_DISPLAY
        printf("[DBG] cfg negedge: idx=%u addr=0x%08lX val=0x%08X\n", i, (unsigned long)gp0_pin_reg_addr[i], cfg);
#endif
    }
}

// -----------------------------------------------------------------------------
// Function: int test_case(void)
// Purpose : Entry point. Implements the Meta Test Steps sequence deterministically
//           without reordering or optimization. Ends with finish(0/1) only.
// -----------------------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[INFO] test_gpio_negedge_intr_en: START\n");
#endif

    // Initialize output bus high
    drive_pad_high();

    // Configure per-pin control for negedge and clear raw
    configure_per_pin_for_negedge();

    // Test loop: for each pin bit 0..31
    for (unsigned int i = 0U; i < 32U; ++i) {
        unsigned int mask = (1U << i);

        // Clear group raw status and enable group interrupt output for selected bit
        write_reg(gp0_rawstcr1_reg, mask);
        write_reg(gp0_intr_en1_reg, mask);
#ifdef DEBUG_DISPLAY
        printf("[DBG] enable+clear: bit=%u RAWSTCR1|EN1 mask=0x%08X\n", i, mask);
#endif

        // Mark interrupt pending and generate falling edge on the selected pad
        g_int_pend = 1;
        generate_falling_edge(i);

        // Poll for ISR to clear pending with a deterministic timeout
        const unsigned int TIMEOUT_ITERS = 1000000U; // bounded polling as per requirement
        unsigned int t = 0U;
        while (g_int_pend != 0 && t < TIMEOUT_ITERS) {
            // Optional light-weight read to ensure bus activity
            (void)read_reg(gp0_intr_sts1_reg);
            ++t;
        }
        if (g_int_pend != 0) {
            // Timed out: count error and proceed
            g_timeout_err++;
            g_int_pend = 0; // force clear to remain deterministic for next iteration
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for ISR clear on bit %u after %u iters\n", i, t);
#endif
        }

        // Ensure bus is high again before next iteration
        drive_pad_high();
    }

    // Aggregate final result
    if ((g_test_err + g_isr_err + g_timeout_err) != 0U) {
#ifdef DEBUG_DISPLAY
        printf("[INFO] END: FAIL (test=%u, isr=%u, to=%u)\n", g_test_err, g_isr_err, g_timeout_err);
#endif
        finish(1);
        return 0; // unreachable but explicit
    }

#ifdef DEBUG_DISPLAY
    printf("[INFO] END: PASS\n");
#endif
    finish(0);
    return 0; // unreachable but explicit
}
