// Author - AI Force 1.3.2. Date 05-05-2026
// (EMBENGG-SYSAPPS)

/*
  Test: test_gpio_pedge_all_pads_en
  High-level description (from Hidden_Test_Description):
    Directed interrupt test that enables positive-edge interrupts (peie) on GPIOs 8..39,
    sets input mode using group IO control registers, enables all group interrupts,
    and for each pin generates a low-to-high transition via 0xA0243FFC.
    ISR reads group status, masks group enables, clears per-pin raw (iclr) for all pins,
    verifies group status cleared, clears sysreg raw, and re-enables group interrupts.
    Uses a bounded wait with int_pend.
*/

#include "test_define.c"

/* Helper prototypes (from test_common.h) */
extern unsigned int read_reg(unsigned int addr);
extern void         write_reg(unsigned int addr, unsigned int val);
extern void         wait_on(unsigned int cycles);
extern void         finish(int status);
extern void         GIC_EnableIRQ(unsigned int id);
extern void         GIC_ClearIRQ(unsigned int id);

/* Register macros (addresses provided by included headers) */
extern unsigned int MIZAR_GPIO_GP0_GPIO_8;            /* Base: per-pin registers start at GPIO_8 */
extern unsigned int MIZAR_GPIO_GP0_INTR1_INTR_EN1;    /* Group interrupt enable */
extern unsigned int MIZAR_GPIO_GP0_INTR1_INTR_STS1;   /* Group interrupt status */
extern unsigned int MIZAR_GPIO_GPIO_IO_CTRL_GROUP1;   /* IO control group1 */
extern unsigned int MIZAR_GPIO_GPIO_IO_CTRL_GROUP2;   /* IO control group2 */
extern unsigned int MIZAR_GPIO_GPIO_IO_CTRL_GROUP3;   /* IO control group3 */
extern unsigned int MIZAR_GPIO_GPIO_IO_CTRL_GROUP4;   /* IO control group4 */
extern unsigned int MIZAR_LSS_SYSREG_INTR_EN1;        /* SysReg interrupt enable */
extern unsigned int MIZAR_LSS_SYSREG_RAW_STCR1;       /* SysReg raw clear */

/* SysReg bit macros (expected from platform headers) */
extern unsigned int LSS_SYSREG_INTR_EN1_GPIO0_INTR;
extern unsigned int LSS_SYSREG_INTR_EN1_GPIO1_INTR;
extern unsigned int LSS_SYSREG_RAW_STCR1_GPIO0_INTR;
extern unsigned int LSS_SYSREG_RAW_STCR1_GPIO1_INTR;

/* Select default IRQ path if none provided */
#if !defined(USE_GPIO0) && !defined(USE_GPIO1)
#define USE_GPIO0
#endif

/* Pad drive MMIO address for generating edges */
#define PAD_DRIVE_ADDR (0xA0243FFCu)

/* Globals */
static volatile int test_err = 0;
static volatile int int_pend = 0;

/*
  Function: Default_IRQHandler
  Purpose: Handle posedge interrupts: capture group status, mask group, clear all per-pin raw,
           verify group status cleared, clear SysReg, re-enable group, and clear GIC.
*/
void Default_IRQHandler(void)
{
    int_pend = 0; /* allow main loop to proceed */

    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u); /* mask group */

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        ++test_err;
        #ifdef DEBUG_DISPLAY
        printf("ERROR: Group status not set on posedge interrupt\n");
        #endif
    }

    /* Clear per-pin raw for all pins (iclr=1) */
    for (unsigned int j = 0; j < 32u; ++j) {
        unsigned int addr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(addr, 0x00010000u);
    }
    wait_on(2u);

    /* Verify group status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000u) {
        ++test_err;
        #ifdef DEBUG_DISPLAY
        printf("ERROR: Group status not cleared; sts=0x%08X\n", rdata_grp);
        #endif
    }

    /* Clear SysReg and re-enable group; clear GIC */
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

/*
  Function: test_case
  Purpose: Convert Hidden_Test_Steps_Procedure steps into code: enable IRQ, configure peie and IO
           control groups, enable group interrupts, generate rising edges, wait with timeout, then
           terminate per Acceptance Criteria.
*/
void test_case(void)
{
    test_err = 0;

    /* Enable IRQ route and SysReg interrupt */
    #ifdef USE_GPIO0
    GIC_EnableIRQ(87u);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
    #elif defined(USE_GPIO1)
    GIC_EnableIRQ(88u);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
    #endif

    /* Configure per-pin positive-edge detection: peie=1 (bit17) */
    for (unsigned int i = 0; i < 32u; ++i) {
        write_reg((MIZAR_GPIO_GP0_GPIO_8 + (i * 4u)), 0x00020000u);
        wait_on(10u);
    }

    /* Configure input mode via group IO control registers */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10u);

    /* Enable all group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    /* For each pin: drive low -> arm -> drive high; timeout wait for ISR */
    for (unsigned int i = 0; i < 32u; ++i) {
        write_reg(PAD_DRIVE_ADDR, 0x00000000u);
        wait_on(10u);

        int_pend = 1;
        write_reg(PAD_DRIVE_ADDR, 0xFFFFFFFFu);

        int timeout = 2000;
        while ((int_pend == 1) && (--timeout > 0)) {
            wait_on(10u);
        }
        if (timeout == 0) {
            ++test_err;
            #ifdef DEBUG_DISPLAY
            printf("ERROR: Timeout waiting for posedge interrupt at index %u\n", (unsigned)i);
            #endif
            break;
        }

        /* Optionally restore low */
        write_reg(PAD_DRIVE_ADDR, 0x00000000u);
        wait_on(10u);
    }

    if (test_err == 0) {
        finish(0); /* PASS */
    } else {
        finish(1); /* FAIL */
    }
}
