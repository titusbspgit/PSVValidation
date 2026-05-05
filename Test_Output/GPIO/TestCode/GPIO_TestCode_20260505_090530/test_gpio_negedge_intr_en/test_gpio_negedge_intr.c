/*
  Test: test_gpio_negedge_intr_en

  Meta Hidden Description:
    Directed interrupt test that enables negative-edge interrupts per GPIO (8..39).
    It configures input mode (doe=1), enables negedge (neie=1), clears raw per-pin (iclr=1),
    enables group bit, induces a falling edge via 0xA0243ffc toggling, and verifies ISR behavior
    including DIN check, group status set/clear, sysreg raw clear, and GIC clear, with a bounded
    wait using int_pend and timeout.

  Acceptance Criteria:
    - Timeout criterion: For each i, the wait loop must exit before timeout; else log
      "ERROR: Timeout waiting for GPIO(i+8) negedge interrupt" and increment test_err.
    - DIN check: In ISR, (rdata & 0x1) must equal 0; else test_err++.
    - Group status set: In ISR, (read(MIZAR_GPIO_GP0_INTR1_INTR_STS1) & (1<<i)) must be non-zero; else test_err++.
    - Clear verification: After clearing per-pin and group raw, read(MIZAR_GPIO_GP0_INTR1_INTR_STS1) must be 0; else test_err++.
    - Sysreg/GIC clear: Corresponding RAW_STCR1 write and GIC clear are performed.
    - Final: finish(test_err) with 0 for pass if no errors accumulated.
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
extern uint32_t MIZAR_GPIO_GPIO_INTR_RAW_STCLR1;
extern uint32_t MIZAR_LSS_SYSREG_INTR_EN1;
extern uint32_t MIZAR_LSS_SYSREG_RAW_STCR1;

/* SysReg bit macros (assumed provided) */
extern uint32_t LSS_SYSREG_INTR_EN1_GPIO0_INTR;
extern uint32_t LSS_SYSREG_INTR_EN1_GPIO1_INTR;
extern uint32_t LSS_SYSREG_RAW_STCR1_GPIO0_INTR;
extern uint32_t LSS_SYSREG_RAW_STCR1_GPIO1_INTR;

/* Globals per metadata */
volatile int test_err = 0;
volatile int int_pend = 0;
/* NOTE: ISR relies on 'i' being the active pin index as per metadata text */
volatile uint32_t i = 0;

/* Pad drive MMIO address for generating edges */
#define PAD_DRIVE_ADDR  (0xA0243FFCu)

/* Default IRQ selection via compile-time switch */
#if !defined(USE_GPIO0) && !defined(USE_GPIO1)
#define USE_GPIO0
#endif

/* ISR as specified in Meta_data_sheet */
void Default_IRQHandler(void)
{
    uint32_t local_wr = (1u << i);
    int_pend = 0;

    /* Restore pads high to quiescent level */
    write_reg(PAD_DRIVE_ADDR, 0xFFFFFFFFu);

    /* Read per-pin control/status register */
    uint32_t raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
    uint32_t rdata = read_reg(raddr);

    /* DIN check: LSB should reflect low after negedge */
    if ((rdata & 0x1u) != 0u) {
        ++test_err;
        printf("ERROR: DIN not low after negedge on GPIO%u\n", (unsigned)(i + 8u));
    }

    /* If raw status present, perform group and per-pin clear and verify */
    if ((rdata & 0x2u) != 0x0u) {
        /* Verify group status bit is set */
        uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            ++test_err;
            printf("ERROR: Group status not set for bit %u\n", (unsigned)i);
        }

        /* Clear per-pin raw while keeping doe=1: write (1<<20 | 1<<16) */
        uint32_t raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(raddr2, ((1u << 20) | (1u << 16)));

        /* Clear group raw for this bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        /* Verify group status cleared */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            ++test_err;
            printf("ERROR: Group status not cleared after raw clears (0x%08X)\n", rdata_grp);
        }

        /* Clear system register raw and GIC IRQ */
        #ifdef USE_GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87u);
        #elif defined(USE_GPIO1)
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88u);
        #endif
    } else {
        ++test_err;
        printf("ERROR: Per-pin raw status not set for GPIO%u\n", (unsigned)(i + 8u));
    }
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

    /* Drive all pads high to a known state */
    write_reg(PAD_DRIVE_ADDR, 0xFFFFFFFFu);

    /* Configure per-pin: doe=1 (bit20), neie=1 (bit18), iclr=1 (bit16) */
    for (i = 0; i < 32u; ++i) {
        uint32_t addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, ((1u << 20) | (1u << 18) | (1u << 16)));
        wait_on(10u);
    }

    /* Exercise each bit: pre-clear group raw, enable one bit, then generate negedge */
    for (i = 0; i < 32u; ++i) {
        uint32_t wr_val = (1u << i);

        /* Pre-clear group raw and enable only this bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10u);

        /* Arm pending, then create negedge on this bit */
        int_pend = 1;
        write_reg(PAD_DRIVE_ADDR, 0xFFFFFFFFu);
        wait_on(30u);
        write_reg(PAD_DRIVE_ADDR, ~wr_val);

        /* Timed wait for ISR to clear int_pend */
        int timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10u);
        }
        if (timeout <= 0) {
            ++test_err;
            printf("ERROR: Timeout waiting for GPIO%u negedge interrupt\n", (unsigned)(i + 8u));
        }
    }

    finish(test_err);
}

/* Optional main for standalone builds */
#ifdef STANDALONE_MAIN
int main(void) { test_case(); return 0; }
#endif
