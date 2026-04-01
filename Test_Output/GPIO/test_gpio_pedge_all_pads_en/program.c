#include "test_define.c"

// GPIO posedge interrupt on all pads (8..39)
// High-level flow derived from CSV Test Steps / Procedure:
// 1) Enable LSS SYSREG interrupt outputs for GPIO instances.
// 2) Configure per-pin posedge detection: write 0x00020000 to each pin register (GPIO_8 + i*4).
// 3) Configure GPIOs 8..39 as inputs using group IO control registers (0x000000FF each group).
// 4) Enable group interrupt for all pads.
// 5) For i=0..31: drive a rising edge via PAD_DRIVER_ADDR and poll group status with timeout.
// 6) On assertion, mask group enable, clear per-pin raw for all pads, verify group clear,
//    clear LSS SYSREG RAW_STCR1, re-enable group interrupt.
// 7) Overall pass/fail based on absence of timeouts or verification errors.

void test_case(void)
{
    unsigned int errors = 0u;

#ifdef DEBUG_DISPLAY
    printf("[GPIO] Start: test_gpio_pedge_all_pads_en\n");
#endif

    // 1) Enable LSS SYSREG interrupt outputs (both instances if present)
    // Using CSV-listed bits; details are provided by platform headers.
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1,
              (unsigned int)(LSS_SYSREG_INTR_EN1_GPIO0_INTR | LSS_SYSREG_INTR_EN1_GPIO1_INTR));
#ifdef DEBUG_DISPLAY
    DBG_PRINTF("[DBG] SYSREG INTR_EN1 set\n");
#endif

    // 2) Configure per-pin posedge detection for pads 8..39
    for (unsigned int i = 0u; i < 32u; ++i) {
        write_reg(gpio_pin_addr(i), GPIO_POSEDGE_EN_VAL);
    }
    wait_on(10);
#ifdef DEBUG_DISPLAY
    DBG_PRINTF("[DBG] Per-pin posedge detection configured\n");
#endif

    // 3) Configure GPIOs 8..39 as inputs via group IO control registers
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, GPIO_GROUP_IO_IN_MASK);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, GPIO_GROUP_IO_IN_MASK);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, GPIO_GROUP_IO_IN_MASK);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, GPIO_GROUP_IO_IN_MASK);
    wait_on(10);
#ifdef DEBUG_DISPLAY
    DBG_PRINTF("[DBG] IO_CTRL groups configured for input\n");
#endif

    // 4) Enable group interrupt for all bits
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, GPIO_INT_EN_ALL);
#ifdef DEBUG_DISPLAY
    DBG_PRINTF("[DBG] Group interrupt enabled for all pads\n");
#endif

    // 5) Iterate across pads and generate posedge, then verify via group status
    for (unsigned int i = 0u; i < 32u; ++i) {
        // Drive known low, then rising edge
        write_reg(PAD_DRIVER_ADDR, 0x00000000u);
        wait_on(10);
        write_reg(PAD_DRIVER_ADDR, 0xFFFFFFFFu);

        // Poll with timeout for group status assertion
        unsigned int timeout = 2000u;
        unsigned int grp_sts = 0u;
        while (timeout-- > 0u) {
            grp_sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (grp_sts != 0u) break;
            wait_on(10);
        }
        if (grp_sts == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for group INT STS (pad=%u)\n", (i + 8u));
#endif
            errors++;
            break; // break out; further pads likely to fail similarly
        }
#ifdef DEBUG_DISPLAY
        DBG_PRINTF("[OK ] Group INT STS asserted (pad=%u, sts=0x%08x)\n", (i + 8u), grp_sts);
#endif

        // Mask group enable during service
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

        // Clear per-pin raw for all pads, then verify group clear
        for (unsigned int j = 0u; j < 32u; ++j) {
            write_reg(gpio_pin_addr(j), GPIO_PERPIN_RAW_ICLR_VAL);
            wait_on(2);
        }
        unsigned int grp_sts_after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (grp_sts_after != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Group INT STS not cleared after per-pin raw clear (0x%08x)\n", grp_sts_after);
#endif
            errors++;
        } else {
            DBG_PRINTF("[OK ] Group INT STS cleared after per-pin raw clear\n");
        }

        // Clear LSS SYSREG RAW status for GPIO instances
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1,
                  (unsigned int)(LSS_SYSREG_RAW_STCR1_GPIO0_INTR | LSS_SYSREG_RAW_STCR1_GPIO1_INTR));

        // Re-enable group interrupt for next iteration
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, GPIO_INT_EN_ALL);

        // Drive low for next iteration
        write_reg(PAD_DRIVER_ADDR, 0x00000000u);
        wait_on(10);
    }

#ifdef DEBUG_DISPLAY
    if (errors == 0u) {
        printf("[GPIO] PASS\n");
    } else {
        printf("[GPIO] FAIL: errors=%u\n", errors);
    }
#endif

    finish(errors ? 1 : 0);
}
