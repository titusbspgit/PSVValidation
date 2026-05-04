/*
 * test_gpio_negedge_intr_en.c
 *
 * Implements Meta_data_sheet::test_gpio_negedge_intr_en exactly as specified.
 * Loops pins 8..39 (i=0..31), enables negedge detection, generates a falling
 * edge per pin, waits for ISR, validates DIN low and group status, and clears.
 *
 * Compile-time selection:
 *   - Define USE_GPIO0 to target GPIO0 path with GIC IRQ 87
 *   - Define USE_GPIO1 to target GPIO1 path with GIC IRQ 88
 *
 * Required macros/registers are expected to be provided by platform headers:
 *   MIZAR_GPIO_GP0_GPIO_8
 *   MIZAR_GPIO_GP0_INTR1_INTR_EN1
 *   MIZAR_GPIO_GP0_INTR1_INTR_STS1
 *   MIZAR_GPIO_GPIO_INTR_RAW_STCLR1
 *   MIZAR_LSS_SYSREG_INTR_EN1
 *   MIZAR_LSS_SYSREG_RAW_STCR1
 *   LSS_SYSREG_INTR_EN1_GPIO0_INTR / LSS_SYSREG_INTR_EN1_GPIO1_INTR
 *   LSS_SYSREG_RAW_STCR1_GPIO0_INTR / LSS_SYSREG_RAW_STCR1_GPIO1_INTR
 *
 * External pad driver register used for edge generation:
 *   0xA0243ffc
 */

#include <stdint.h>

/* Platform-provided functions (declared extern to avoid redefining) */
extern void     write_reg(uint32_t addr, uint32_t val);
extern uint32_t read_reg(uint32_t addr);
extern void     wait_on(uint32_t cycles);
extern void     finish(uint32_t status);
extern void     GIC_EnableIRQ(uint32_t irq);
extern void     GIC_ClearIRQ(uint32_t irq);

/* Platform headers that define MIZAR_* and LSS_* register macros (include if available) */
#if __has_include("mizar_gpio.h")
#include "mizar_gpio.h"
#endif
#if __has_include("lss_sysreg.h")
#include "lss_sysreg.h"
#endif

/* External pad driver register (edge generation) */
#define PAD_DRIVER_REG_ADDR   ((uint32_t)0xA0243ffcU)

/* Compile-time selection (default to GPIO0 if neither is defined) */
#if !defined(USE_GPIO0) && !defined(USE_GPIO1)
#define USE_GPIO0
#endif

#ifdef USE_GPIO0
#define SYS_INTR_EN_BIT     (LSS_SYSREG_INTR_EN1_GPIO0_INTR)
#define SYS_RAW_STCR_BIT    (LSS_SYSREG_RAW_STCR1_GPIO0_INTR)
#define GIC_IRQ_NUM         (87U)
#else
#define SYS_INTR_EN_BIT     (LSS_SYSREG_INTR_EN1_GPIO1_INTR)
#define SYS_RAW_STCR_BIT    (LSS_SYSREG_RAW_STCR1_GPIO1_INTR)
#define GIC_IRQ_NUM         (88U)
#endif

/* Shared state between main test loop and ISR */
static volatile uint32_t int_pend = 0;
static volatile uint32_t current_i = 0; /* 0..31 maps to GPIO[8..39] */
static volatile uint32_t test_err = 0;

/* Forward declaration of ISR symbol used by template/startup */
void Default_IRQHandler(void);

/* Interrupt Service Routine implementing Meta_data_sheet steps */
void Default_IRQHandler(void)
{
    /* int_pend=0; restore pad high */
    int_pend = 0;
    write_reg(PAD_DRIVER_REG_ADDR, 0xFFFFFFFFU);

    /* raddr = MIZAR_GPIO_GP0_GPIO_8 + (i*4); rdata = read_reg(raddr) */
    uint32_t raddr = (uint32_t)(MIZAR_GPIO_GP0_GPIO_8 + (current_i * 4U));
    uint32_t rdata = read_reg(raddr);

    /* Check DIN low: if ((rdata & 0x1) != 0) test_err++ */
    if ((rdata & 0x1U) != 0U) {
        test_err++;
    }

    /* Check raw bit set on the pin register (bit1) then validate group status */
    if ((rdata & 0x2U) != 0x0U) {
        uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & (1U << current_i)) == 0U) {
            test_err++;
        }

        /* Clear per-pin raw: write (1u<<20)|(1u<<16) to per-pin register */
        write_reg((uint32_t)(MIZAR_GPIO_GP0_GPIO_8 + (current_i * 4U)),
                  ((1U << 20) | (1U << 16)));

        /* Clear group raw for this bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1U << current_i));

        /* Verify group clear equals 0x0 */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0U) {
            test_err++;
        }

        /* Clear system raw and clear corresponding GIC IRQ */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, SYS_RAW_STCR_BIT);
        GIC_ClearIRQ(GIC_IRQ_NUM);
    } else {
        /* raw bit not set => error */
        test_err++;
    }
}

/* Test entry implementing Meta_data_sheet::Hidden_Test_Steps_Procedure */
void test_gpio_negedge_intr_en(void)
{
    test_err = 0U;

    /* Conditionally enable system interrupt and GIC line */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, SYS_INTR_EN_BIT);
    GIC_EnableIRQ(GIC_IRQ_NUM);

    /* Drive pad driver to known high */
    write_reg(PAD_DRIVER_REG_ADDR, 0xFFFFFFFFU);

    /* For i=0..31: configure each per-pin control register
       write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4),(1u<<20)|(1u<<18)|(1u<<16))
       doe=1 (input), neie=1, iclr=1 (clear raw) */
    for (uint32_t i = 0; i < 32U; ++i) {
        uint32_t addr1 = (uint32_t)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4U));
        write_reg(addr1, ((1U << 20) | (1U << 18) | (1U << 16)));
    }

    /* Loop i=0..31 per pin */
    for (uint32_t i = 0; i < 32U; ++i) {
        uint32_t wr_val = (1U << i);

        /* Pre-clear group raw for this bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);

        /* Enable only this bit in group interrupt enable */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);

        /* Arm wait and generate falling edge on this pin */
        current_i = i;
        int_pend = 1U;

        /* Ensure pad is high, then toggle the single bit low to generate negedge */
        write_reg(PAD_DRIVER_REG_ADDR, 0xFFFFFFFFU);
        wait_on(30U);
        write_reg(PAD_DRIVER_REG_ADDR, ~wr_val);

        /* Bounded wait with timeout=5000 while (int_pend && timeout--) wait_on(10) */
        uint32_t timeout = 5000U;
        while ((int_pend != 0U) && (timeout-- > 0U)) {
            wait_on(10U);
        }
        if (timeout == 0U && int_pend != 0U) {
            /* timeout => error */
            test_err++;
            /* attempt to restore pad high to avoid cascading errors */
            write_reg(PAD_DRIVER_REG_ADDR, 0xFFFFFFFFU);
            /* proceed to next pin */
        }
    }

    /* Finalize */
    finish(test_err);
}
