// Author - AI Force 1.3.2. Date 05-05-2026
// (EMBENGG-SYSAPPS)

/*
  Test: test_gpio_negedge_intr_en
  High-level description (from Hidden_Test_Description):
    Directed interrupt test that enables negative-edge interrupts per GPIO (8..39).
    It configures input mode (doe=1), enables negedge (neie=1), clears raw per-pin (iclr=1),
    enables group bit, induces a falling edge via 0xA0243FFC toggling, and verifies ISR behavior
    including DIN check, group status set/clear, sysreg raw clear, and GIC clear, with a bounded
    wait using int_pend and timeout.
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
extern unsigned int MIZAR_GPIO_GPIO_INTR_RAW_STCLR1;  /* Group raw clear */
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

/* Pad drive MMIO address for generating edges (from impacted registers) */
#define PAD_DRIVE_ADDR (0xA0243FFCu)

/* Globals for ISR/test synchronization */
static volatile int test_err = 0;
static volatile int int_pend = 0;
static volatile unsigned int i = 0; /* active pin index */

/*
  Function: Default_IRQHandler
  Purpose: Service the GPIO negedge interrupt as per Hidden_Test_Steps_Procedure, validating:
           - DIN low check (bit0),
           - Group status presence/clear,
           - Per-pin raw clear (iclr) and group raw clear,
           - SysReg raw clear and GIC clear.
*/
void Default_IRQHandler(void)
{
    unsigned int local_wr = (1u << i);
    int_pend = 0; /* signal wait loop to proceed */

    /* Restore pads high to quiescent */
    write_reg(PAD_DRIVE_ADDR, 0xFFFFFFFFu);

    /* Read per-pin control/status */
    unsigned int raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
    unsigned int rdata = read_reg(raddr);

    /* DIN check: LSB must be 0 after negedge */
    if ((rdata & 0x1u) != 0u) {
        ++test_err;
        #ifdef DEBUG_DISPLAY
        printf("ERROR: DIN not low after negedge on GPIO%u\n", (unsigned)(i + 8u));
        #endif
    }

    /* If per-pin raw set (bit1), validate and clear paths */
    if ((rdata & 0x2u) != 0u) {
        /* Group status must indicate the bit */
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            ++test_err;
            #ifdef DEBUG_DISPLAY
            printf("ERROR: Group status not set for bit %u\n", (unsigned)i);
            #endif
        }

        /* Clear per-pin raw while keeping doe=1 (bit20) and iclr=1 (bit16) */
        unsigned int raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(raddr2, ((1u << 20) | (1u << 16)));

        /* Clear group raw for this bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        /* Verify group status cleared */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            ++test_err;
            #ifdef DEBUG_DISPLAY
            printf("ERROR: Group status not cleared; sts=0x%08X\n", rdata_grp);
            #endif
        }

        /* Clear SysReg raw and GIC */
        #ifdef USE_GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87u);
        #elif defined(USE_GPIO1)
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88u);
        #endif
    } else {
        ++test_err;
        #ifdef DEBUG_DISPLAY
        printf("ERROR: Per-pin raw status not set for GPIO%u\n", (unsigned)(i + 8u));
        #endif
    }
}

/*
  Function: test_case
  Purpose: Main body converting Hidden_Test_Steps_Procedure steps 1..4 into code with
           bounded wait and error accumulation. Terminates per Acceptance Criteria.
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

    /* Drive all pads high */
    write_reg(PAD_DRIVE_ADDR, 0xFFFFFFFFu);

    /* Configure per-pin: doe=1 (bit20), neie=1 (bit18), iclr=1 (bit16) */
    for (i = 0; i < 32u; ++i) {
        unsigned int addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, ((1u << 20) | (1u << 18) | (1u << 16)));
        wait_on(10u);
    }

    /* For each bit, pre-clear group raw, enable only that bit, then create negedge */
    for (i = 0; i < 32u; ++i) {
        unsigned int wr_val = (1u << i);

        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,  wr_val);
        wait_on(10u);

        int_pend = 1;
        write_reg(PAD_DRIVE_ADDR, 0xFFFFFFFFu);
        wait_on(30u);
        write_reg(PAD_DRIVE_ADDR, ~wr_val);

        int timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10u);
        }
        if (timeout <= 0) {
            ++test_err;
            #ifdef DEBUG_DISPLAY
            printf("ERROR: Timeout waiting for GPIO%u negedge interrupt\n", (unsigned)(i + 8u));
            #endif
        }
    }

    if (test_err == 0) {
        finish(0); /* PASS */
    } else {
        finish(1); /* FAIL */
    }
}
