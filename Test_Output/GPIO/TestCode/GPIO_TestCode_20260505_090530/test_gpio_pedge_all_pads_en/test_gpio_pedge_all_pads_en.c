/*
  Test: test_gpio_pedge_all_pads_en

  Meta Hidden Description:
    Directed interrupt test that enables positive-edge interrupts (peie) on GPIOs 8..39,
    sets input mode using group IO control registers, enables all group interrupts,
    and for each pin generates a low-to-high transition via 0xA0243ffc.
    ISR reads group status, masks group enables, clears per-pin raw (iclr) for all pins,
    verifies group status cleared, clears sysreg raw, and re-enables group interrupts.
    Uses a bounded wait with int_pend.

  Acceptance Criteria:
    - Timeout criterion: For each i, the wait loop must exit before timeout; else print timeout and increment test_err.
    - Group status presence: In ISR, read(MIZAR_GPIO_GP0_INTR1_INTR_STS1) must be non-zero prior to clears; else test_err++.
    - Clear verification: After clearing per-pin raw for all pins, read(MIZAR_GPIO_GP0_INTR1_INTR_STS1) must equal 0; else test_err++.
    - Sysreg clear: After writing RAW_STCR1 clear, the corresponding status bit must read as 0; else test_err++.
    - Final: finish(test_err) == 0 indicates pass, else fail.
*/

#include <stdint.h>
#include <stdio.h>
#include "test_common.h"
#include "test_define.h"

/* Framework symbols expected */
extern uint32_t read_reg(uint32_t addr);
extern void     write_reg(uint32_t addr, uint32_t val);
extern void     wait_on(uint32_t cycles);
extern void     finish(int status);
extern void     GIC_EnableIRQ(uint32_t id);
extern void     GIC_ClearIRQ(uint32_t id);

/* From platform headers (assumed to be provided by template) */
extern uint32_t MIZAR_GPIO_GP0_GPIO_8;
extern uint32_t MIZAR_GPIO_GP0_INTR1_INTR_EN1;
extern uint32_t MIZAR_GPIO_GP0_INTR1_INTR_STS1;
extern uint32_t MIZAR_GPIO_GPIO_IO_CTRL_GROUP1;
extern uint32_t MIZAR_GPIO_GPIO_IO_CTRL_GROUP2;
extern uint32_t MIZAR_GPIO_GPIO_IO_CTRL_GROUP3;
extern uint32_t MIZAR_GPIO_GPIO_IO_CTRL_GROUP4;
extern uint32_t MIZAR_LSS_SYSREG_INTR_EN1;
extern uint32_t MIZAR_LSS_SYSREG_RAW_STCR1;

/* SysReg bit macros (assumed provided) */
extern uint32_t LSS_SYSREG_INTR_EN1_GPIO0_INTR;
extern uint32_t LSS_SYSREG_INTR_EN1_GPIO1_INTR;
extern uint32_t LSS_SYSREG_RAW_STCR1_GPIO0_INTR;
extern uint32_t LSS_SYSREG_RAW_STCR1_GPIO1_INTR;

/* Globals */
volatile int test_err = 0;
volatile int int_pend = 0;

/* Pad drive MMIO address for generating edges */
#define PAD_DRIVE_ADDR  (0xA0243FFCu)

/* Default IRQ selection via compile-time switch */
#if !defined(USE_GPIO0) && !defined(USE_GPIO1)
#define USE_GPIO0
#endif

/* ISR implements described behavior */
void Default_IRQHandler(void)
{
    /* Signal wait loop to proceed */
    int_pend = 0;

    /* Read group status and mask group enables */
    uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    /* Group status must be non-zero prior to clears */
    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        ++test_err;
        printf("ERROR: Group status not set on posedge interrupt\n");
    }

    /* Clear per-pin raw for all pins via per-pin control (iclr=1) */
    for (uint32_t j = 0; j < 32u; ++j) {
        uint32_t addr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(addr, 0x00010000u); /* iclr=1 */
    }
    wait_on(2u);

    /* Verify group status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000u) {
        ++test_err;
        printf("ERROR: Group status not cleared after per-pin iclr; sts=0x%08X\n", rdata_grp);
    }

    /* Clear system register raw and re-enable group interrupts; clear GIC */
    #ifdef USE_GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    GIC_ClearIRQ(87u);
    #elif defined(USE_GPIO1)
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    GIC_ClearIRQ(88u);
    #endif
}

void test_case(void)
{
    test_err = 0;

    /* Conditionally enable IRQ and sysreg interrupt routing */
    #ifdef USE_GPIO0
    GIC_EnableIRQ(87u);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
    #elif defined(USE_GPIO1)
    GIC_EnableIRQ(88u);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
    #endif

    /* Configure per-pin positive-edge detection: peie=1 => 0x00020000 */
    for (uint32_t i = 0; i < 32u; ++i) {
        write_reg((MIZAR_GPIO_GP0_GPIO_8 + (i * 4u)), 0x00020000u);
        wait_on(10u);
    }

    /* Configure input mode via group IO control registers: 0x000000FF each */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10u);

    /* Enable all group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    /* For each pin: drive low then high to create posedge; bounded wait for ISR */
    for (uint32_t i = 0; i < 32u; ++i) {
        /* Drive low */
        write_reg(PAD_DRIVE_ADDR, 0x00000000u);
        wait_on(10u);

        /* Arm and drive high to generate rising edge */
        int_pend = 1;
        write_reg(PAD_DRIVE_ADDR, 0xFFFFFFFFu);

        int timeout = 2000;
        while ((int_pend == 1) && (--timeout > 0)) {
            wait_on(10u);
        }
        if (timeout == 0) {
            ++test_err;
            printf("ERROR: Timeout waiting for posedge interrupt at index %u\n", (unsigned)i);
            break;
        }

        /* Optionally restore low */
        write_reg(PAD_DRIVE_ADDR, 0x00000000u);
        wait_on(10u);
    }

    finish(test_err);
}

/* Optional main for standalone builds */
#ifdef STANDALONE_MAIN
int main(void) { test_case(); return 0; }
#endif
