// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// -----------------------------------------------------------------------------
// Function: pre_configure_pads
// Purpose : Configure per-pad registers prior to interrupt tests
// Notes   : Sets bits (20, 18, 16) on each addressed GPIO register
// -----------------------------------------------------------------------------
static void pre_configure_pads(void)
{
    unsigned int i;
    const unsigned int cfg_val = ((1u << 20) | (1u << 18) | (1u << 16));
    for (i = 0; i < 20u; i++) {
        unsigned long addr1 = addr_array[i];
        write_reg(addr1, cfg_val);               // Program pad configuration
#ifdef DEBUG_DISPLAY
        printf("[DBG] pre_configure_pads: i=%u addr=0x%08lx val=0x%08x\n", i, addr1, cfg_val);
#endif
        wait_on(10);
    }
}

// -----------------------------------------------------------------------------
// Globals used across test and ISR
// -----------------------------------------------------------------------------
static volatile unsigned int test_err = 0u;      // Error counter
static volatile unsigned int int_pend = 0u;      // Interrupt pending flag (cleared by ISR)
static volatile unsigned int current_index = 0u; // Current pad index under test

// -----------------------------------------------------------------------------
// Function: Default_IRQHandler
// Purpose : Interrupt handler logic per Meta Test Steps / Acceptance Criteria
// Notes   : Uses ONLY impacted registers and per-pad address from addr_array
// -----------------------------------------------------------------------------
void Default_IRQHandler(void)
{
    unsigned int local_wr;
    unsigned long raddr;
    unsigned int rdata;
    unsigned int rdata_grp;

    local_wr = (1u << current_index);
    int_pend = 0u;  // Clear pending flag as first action
#ifdef DEBUG_DISPLAY
    printf("[DBG][ISR] Enter: idx=%u local_wr=0x%08x\n", current_index, local_wr);
#endif

    // Read per-pad register and validate level/status bits
    raddr = addr_array[current_index];
    rdata = read_reg(raddr);
#ifdef DEBUG_DISPLAY
    printf("[DBG][ISR] raddr=0x%08lx rdata=0x%08x\n", raddr, rdata);
#endif

    // Pad level check: bit[0] must be 0 after negative edge
    if ((rdata & 0x1u) != 0u) {
#ifdef DEBUG_DISPLAY
        printf("[ERR][ISR] Pad level bit[0] high. idx=%u rdata=0x%08x\n", current_index, rdata);
#endif
        test_err++;
    }

    // Edge/status check: bit[1] must be set
    if ((rdata & 0x2u) != 0u) {
        // Group status must reflect the specific pad
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
#ifdef DEBUG_DISPLAY
        printf("[DBG][ISR] INTR1_INTR_STS1=0x%08x\n", rdata_grp);
#endif
        if ((rdata_grp & local_wr) == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] Group status bit not set. need=0x%08x got=0x%08x\n", local_wr, rdata_grp);
#endif
            test_err++;
        }

        // Re-program per-pad configuration (bits 20 and 16)
        write_reg(raddr, ((1u << 20) | (1u << 16)));
#ifdef DEBUG_DISPLAY
        printf("[DBG][ISR] Re-prog pad cfg: raddr=0x%08lx val=0x%08x\n", raddr, ((1u << 20) | (1u << 16)));
#endif

        // Clear raw status for the tested pad and re-check group status clears to zero
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
#ifdef DEBUG_DISPLAY
        printf("[DBG][ISR] RAW_STCLR1 clr mask=0x%08x\n", local_wr);
#endif

        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] Group status not cleared. INTR1_INTR_STS1=0x%08x\n", rdata_grp);
#endif
            test_err++;
        }

        // Clear system raw status for the selected instance
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, local_wr);
#ifdef DEBUG_DISPLAY
        printf("[DBG][ISR] SYS RAW_STCR1 clr mask=0x%08x\n", local_wr);
#endif
    } else {
#ifdef DEBUG_DISPLAY
        printf("[ERR][ISR] Edge/status bit[1] not set. idx=%u rdata=0x%08x\n", current_index, rdata);
#endif
        test_err++;
    }
}

// -----------------------------------------------------------------------------
// Function: per_pad_sequence
// Purpose : Execute per-pad interrupt enable, raw clear, and wait-for-ISR flow
// Notes   : Polls on int_pend which is expected to be cleared by ISR
// -----------------------------------------------------------------------------
static void per_pad_sequence(void)
{
    unsigned int i;
    for (i = 0u; i < 20u; i++) {
        unsigned int wr_val = (1u << i);
        current_index = i;
#ifdef DEBUG_DISPLAY
        printf("[DBG] per_pad_sequence: i=%u mask=0x%08x\n", i, wr_val);
#endif

        // Clear any pending raw status for this pad
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        // Enable per-pad interrupt mask
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        // Set software pending flag; ISR is expected to clear upon real interrupt
        int_pend = 1u;

        // Poll with timeout until ISR clears int_pend
        {
            int timeout = 5000;
            while ((int_pend != 0u) && (timeout > 0)) {
                wait_on(10);
                timeout--;
            }
            if (int_pend != 0u) {
#ifdef DEBUG_DISPLAY
                printf("[ERR] Timeout waiting for ISR clear. i=%u\n", i);
#endif
                test_err++;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Function: test_case (Entry Point)
// Purpose : Overall test flow per Meta Test Steps
// -----------------------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DBG] test_gpio_negedge_intr_en: START\n");
#endif

    // Initialization: Pre-configure pads
    pre_configure_pads();

    // Execute per-pad sequence (enable, clear, wait for ISR)
    per_pad_sequence();

#ifdef DEBUG_DISPLAY
    printf("[DBG] test_gpio_negedge_intr_en: END. errors=%u\n", test_err);
#endif

    if (test_err != 0u) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }

    // Unreachable
    return 0;
}
