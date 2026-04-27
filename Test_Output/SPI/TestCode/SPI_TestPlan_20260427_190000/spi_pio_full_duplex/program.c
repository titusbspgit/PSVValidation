// Author - AI Force 1.3.2. Date 27-04-2026
// (EMBENGG-SYSAPPS)

/*
 High-level description (verbatim derived):
 Interrupt-driven full-duplex style flow: test_case() enables sysreg SPIx intr (0xA000001C) and GIC IRQ (76–79), calls spi_cntrl_config(), then spi_vip_handshake(), loops 8 times setting int_pend=1 and while(int_pend){wait_on(10);}; ISR Default_IRQHandler() sets int_pend=0, reads MIZAR_SPI_MIS, if RX bit (0x2) set then reads MIZAR_SPI_DATA_REG SPI_RX_FIFO_THLD times; else if TX bit (0x1) set then writes MIZAR_SPI_DATA_REG SPI_TX_FIFO_THLD times with 'count'; finally writes 0xA0000018 and calls GIC_ClearIRQ(). After the loop, err1=spi_vip_scbd_status(); finish(err1).
*/

/* Only include the generated definitions as mandated */
#include "test_define.c"

/*
 * Function: test_case
 * Purpose : Deterministic translation of the procedure using only impacted registers.
 *           - Initialize controller and VIP handshake via provided APIs
 *           - Poll MIZAR_SPI_MIS
 *           - On RX mis bit set: perform a single read from MIZAR_SPI_DATA_REG
 *           - On TX mis bit set: perform a single write to MIZAR_SPI_DATA_REG
 *           - Insert wait_on(10) between iterations as timing wait from the steps
 * Notes   : No external interrupts/sysreg/GIC manipulation is performed here per rule to
 *           restrict access to impacted registers only.
 */
void test_case(void)
{
    unsigned int count = 1u;       /* TX data pattern base */

    /* Initialization per description */
    spi_cntrl_config();
    spi_vip_handshake();

    for (unsigned int j = 0u; j < 8u; ++j)
    {
        /* Poll masked interrupt status */
        unsigned int mis = read_reg(MIZAR_SPI_MIS); /* bits: [0]=TX, [1]=RX */
#ifdef DEBUG_DISPLAY
        printf("[spi_pio_full_duplex] Iter %u: MIS=0x%08X\n", j, mis);
#endif
        if ((mis & 0x2u) == 0x2u)
        {
            /* RX path: read once from DATA register */
            volatile unsigned int rd_data = read_reg(MIZAR_SPI_DATA_REG); /* MIZAR_SPI_DATA_REG.fifo */
            (void)rd_data; /* value consumed for determinism */
#ifdef DEBUG_DISPLAY
            printf("  RX event: read DATA_REG=0x%08X\n", rd_data);
#endif
        }
        else if ((mis & 0x1u) == 0x1u)
        {
            /* TX path: write once to DATA register */
            write_reg(MIZAR_SPI_DATA_REG, count); /* MIZAR_SPI_DATA_REG.fifo */
#ifdef DEBUG_DISPLAY
            printf("  TX event: wrote DATA_REG=0x%08X\n", count);
#endif
            count++;
        }
        else
        {
            /* No RX/TX MIS bits set - nothing to act on; keep deterministic wait */
#ifdef DEBUG_DISPLAY
            printf("  No RX/TX MIS bits set\n");
#endif
        }

        /* Deterministic wait per step's while(int_pend){wait_on(10);} */
        wait_on(10u);
    }

    /* Acceptance criteria: pass if err1 == 0 where err1 = spi_vip_scbd_status() */
    {
        int err1 = spi_vip_scbd_status();
#ifdef DEBUG_DISPLAY
        printf("[spi_pio_full_duplex] spi_vip_scbd_status()=%d\n", err1);
#endif
        if (err1 == 0)
        {
            finish(0); /* PASS */
        }
        else
        {
            finish(1); /* FAIL */
        }
    }
}
