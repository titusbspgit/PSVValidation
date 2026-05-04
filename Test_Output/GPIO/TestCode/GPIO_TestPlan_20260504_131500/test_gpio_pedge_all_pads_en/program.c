// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)
#include "test_define.c"

/*
 High-level Description (from META):
 Enable posedge interrupts for GPIO[8..39], toggle pads to create rising edges one by one, wait for ISR via volatile int_pend, and validate group status, per-pin clear, and system clear.
*/

static volatile int int_pend = 0;
static unsigned int test_err = 0;

/*
 * Default_IRQHandler
 * Purpose: Minimal ISR for posedge path: validate group status non-zero, clear per-pin and group raw, and clear system RAW.
 */
void Default_IRQHandler(void)
{
    unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (sts != 0u) {
#ifdef DEBUG_DISPLAY
        printf("[ISR] INTR1_STS1=0x%08X\n", sts);
#endif
        for (unsigned int i = 0; i < 32; i++) {
            if (sts & (1u << i)) {
                unsigned long pin_reg = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
                /* Clear per-pin raw (iclr bit) */
                write_reg(pin_reg, (1u << 16));
                /* Clear group raw status for this bit */
                write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << i));
            }
        }
        /* Verify group status cleared */
        unsigned int sts_after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (sts_after != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] Group status not cleared: 0x%08X\n", sts_after);
#endif
            test_err++;
        }
        /* Clear system RAW and confirm */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0xFFFFFFFFu);
        if (read_reg(MIZAR_LSS_SYSREG_RAW_STCR1) != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] System RAW_STCR1 not cleared to 0\n");
#endif
            test_err++;
        }
        int_pend = 0; /* handshake to main loop */
    }
}

/*
 * test_case
 * Purpose: Configure posedge path, IO control groups and group enable; per-pin wait for ISR with timeout; finish per META.
 */
void test_case(void)
{
    test_err = 0;

    /* Enable system-level GPIO interrupts */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0xFFFFFFFFu);

    /* Configure posedge on GPIO[8..39] */
    for (unsigned int i = 0; i < 32; i++) {
        unsigned long pin_reg = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(pin_reg, 0x00020000u); /* peie=1 per META */
    }
    wait_on(10);

    /* Configure input mode via group IO control */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    /* Enable group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (unsigned int i = 0; i < 32; i++) {
        /* External stimulus expected to create rising edge on bit i */
        int_pend = 1;
        unsigned int timeout = 2000u;
        while (int_pend && timeout > 0u) {
            timeout--;
        }
        if (timeout == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for posedge interrupt on bit %u\n", i);
#endif
            test_err++;
        }
    }

    if (test_err == 0u) {
        finish(0); /* PASS */
    } else {
        finish(1); /* FAIL */
    }
}
