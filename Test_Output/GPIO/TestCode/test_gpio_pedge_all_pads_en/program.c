// Author - AI Force 1.3.2. Date 23-04-2026
// (EMBENGG-SYSAPPS)

/*
  Testcase: test_gpio_pedge_all_pads_en
  High-level description (from metadata):
  - Enable GPIO posedge (peie bit) for pads 8..39, configure IO control to input,
    enable group interrupt, and validate via configuration read-backs only.
  Constraint: LSS_SYSREG intentionally ignored. No external stimulus or magic addresses.
*/

#include "test_define.c"

/* Banner comment for function: test_case
   Purpose: Configure per-pin posedge enable, set optional group configuration, and
            validate by reading back programmed values. */
void test_case(void)
{
    int test_err = 0;
    int i;

#ifdef DEBUG_DISPLAY
    printf("[test_gpio_pedge_all_pads_en] Start\n");
#endif

    /* Optional: Configure IO control group to input mode where macros are available */
#if defined(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4) && defined(GPIO_GPIO_IO_CTRL_GROUP4_WRITE_MASK) && \
    defined(GPIO_GPIO_IO_CTRL_GROUP4_READ_MASK) && defined(GPIO_GPIO_IO_CTRL_GROUP4_DEFAULT_VAL)
    {
        unsigned int w = (0x000000FFu & GPIO_GPIO_IO_CTRL_GROUP4_WRITE_MASK);
        unsigned int rm = GPIO_GPIO_IO_CTRL_GROUP4_READ_MASK;
        unsigned int wm = GPIO_GPIO_IO_CTRL_GROUP4_WRITE_MASK;
        unsigned int wr_n = (wm ^ 0xFFFFFFFFu);
        write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, w);
        wait_on(10);
        unsigned int rd = (read_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4) & rm);
        unsigned int exp = ((w & rm & wm) | (wr_n & rm & GPIO_GPIO_IO_CTRL_GROUP4_DEFAULT_VAL));
        if (rd != exp) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[IO_CTRL_G4][FAIL] rd=0x%08X exp=0x%08X rm=0x%08X wm=0x%08X\n", rd, exp, rm, wm);
#endif
        }
    }
#endif

    /* Optional: Enable group interrupt where macros are available */
#if defined(MIZAR_GPIO_GP0_INTR1_INTR_EN1) && defined(GPIO_GP0_INTR1_INTR_EN1_WRITE_MASK) && \
    defined(GPIO_GP0_INTR1_INTR_EN1_READ_MASK) && defined(GPIO_GP0_INTR1_INTR_EN1_DEFAULT_VAL)
    {
        unsigned int w = (0xFFFFFFFFu & GPIO_GP0_INTR1_INTR_EN1_WRITE_MASK);
        unsigned int rm = GPIO_GP0_INTR1_INTR_EN1_READ_MASK;
        unsigned int wm = GPIO_GP0_INTR1_INTR_EN1_WRITE_MASK;
        unsigned int wr_n = (wm ^ 0xFFFFFFFFu);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, w);
        wait_on(10);
        unsigned int rd = (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1) & rm);
        unsigned int exp = ((w & rm & wm) | (wr_n & rm & GPIO_GP0_INTR1_INTR_EN1_DEFAULT_VAL));
        if (rd != exp) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[INTR1_EN1][FAIL] rd=0x%08X exp=0x%08X rm=0x%08X wm=0x%08X\n", rd, exp, rm, wm);
#endif
        }
    }
#endif

    /* Per-pin posedge enable and validation */
    for (i = 0; i < CNT; i++) {
        unsigned long addr = gpio_pin_addr_array[i];
        unsigned int rm = gpio_pin_read_mask_array[i];
        unsigned int wm = gpio_pin_write_mask_array[i];
        unsigned int wr_n = (wm ^ 0xFFFFFFFFu);
        unsigned int w = (gpio_pin_peie_value & wm);

        write_reg(addr, w);
        wait_on(10);

        if (rm == 0u) {
            continue; /* cannot validate readback without read mask */
        }
        /* Expected composed from default and written bits */
        unsigned int rd = (read_reg(addr) & rm);
        unsigned int exp = ((gpio_pin_peie_value & rm & wm) | (wr_n & rm & gpio_pin_default_value_array[i]));
        if (rd != exp) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[PIN][FAIL] idx=%d addr=0x%08lX rd=0x%08X exp=0x%08X rm=0x%08X wm=0x%08X\n",
                   i, addr, rd, exp, rm, wm);
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[test_gpio_pedge_all_pads_en] test_err=%d\n", test_err);
#endif

    if (test_err == 0) {
        finish(0); /* PASS */
    } else {
        finish(1); /* FAIL */
    }
}

#if 0
/* ISR-related code intentionally omitted to keep this testcase configuration-only. */
void Default_IRQHandler(void)
{
    /* Not used. */
}
#endif
