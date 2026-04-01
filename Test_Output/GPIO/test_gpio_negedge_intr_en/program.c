#include "test_define.c"

// High-level summary:
// GPIO negedge interrupt basic enable/clear sanity for GPIO_8 only (bit0),
// constrained to registers available via GP0_RAG: RAW_STCLR1 and INTR1_INTR_EN1.
// Steps:
// 1) Clear RAW_STCLR1 bit0 (write-1-clear), verify read shows 0.
// 2) Enable INTR1_EN1 bit0, verify readback has bit0 set.
// 3) Disable INTR1_EN1 bit0, verify readback has bit0 cleared.
// 4) Clear RAW_STCLR1 bit0 again and verify.
// Any mismatch increments error counter. Test ends with finish(0) on pass else finish(1).

void test_case(void)
{
    unsigned int errors = 0u;

#ifdef DEBUG_DISPLAY
    printf("[GPIO] Start: test_gpio_negedge_intr_en (limited: GPIO_8)\n");
#endif

    // 1) Clear RAW status for GPIO_8 and verify read reports 0 in bit0
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, GPIO_BIT_GPIO8);
    {
        unsigned int r = read_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1);
        if ((r & GPIO_BIT_GPIO8) != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] RAW_STCLR1 bit0 not cleared as expected (r=0x%08x)\n", r);
#endif
            errors++;
        } else {
            DBG_PRINTF("[OK ] RAW_STCLR1 clear verified (r=0x%08x)\n", r);
        }
    }

    // 2) Enable group interrupt for GPIO_8 and verify
    {
        unsigned int cur = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1);
        unsigned int newv = (cur | GPIO_BIT_GPIO8);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, newv);
        unsigned int rdv = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1);
        if ((rdv & GPIO_BIT_GPIO8) == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] INTR1_EN1 bit0 not set (rd=0x%08x)\n", rdv);
#endif
            errors++;
        } else {
            DBG_PRINTF("[OK ] INTR1_EN1 enable verified (rd=0x%08x)\n", rdv);
        }
    }

    // 3) Disable group interrupt for GPIO_8 and verify
    {
        unsigned int cur = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1);
        unsigned int newv = (cur & ~GPIO_BIT_GPIO8);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, newv);
        unsigned int rdv = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1);
        if ((rdv & GPIO_BIT_GPIO8) != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] INTR1_EN1 bit0 not cleared (rd=0x%08x)\n", rdv);
#endif
            errors++;
        } else {
            DBG_PRINTF("[OK ] INTR1_EN1 disable verified (rd=0x%08x)\n", rdv);
        }
    }

    // 4) Clear RAW status for GPIO_8 again and verify
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, GPIO_BIT_GPIO8);
    {
        unsigned int r = read_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1);
        if ((r & GPIO_BIT_GPIO8) != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] RAW_STCLR1 bit0 not cleared on second attempt (r=0x%08x)\n", r);
#endif
            errors++;
        } else {
            DBG_PRINTF("[OK ] RAW_STCLR1 re-clear verified (r=0x%08x)\n", r);
        }
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
