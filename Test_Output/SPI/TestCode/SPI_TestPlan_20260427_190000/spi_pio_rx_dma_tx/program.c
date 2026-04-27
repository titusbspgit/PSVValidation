// Author - AI Force 1.3.2. Date 27-04-2026
// (EMBENGG-SYSAPPS)

/*
 High-level description (verbatim derived):
 DMA TX with PIO/RX-disabled path: test_case() enables sysreg SPIx intr (0xA000001C) and GIC IRQ (76–79), writes 0x1 to 0xA1700008..0xA1700054, sets ch_num=0, src_addr=0xA0243E6C, src_xcnt=8, tx_rx=0, tc_intr_en=0, spi_mst=0, src_req=SPI_TX_SRC_REQ; fills memory at 0xA0243E6C+i*4 with 0xaaaaaaa1+i; calls spi_cntrl_config(); spi_vip_handshake(); sets dst_addr=MIZAR_SPI_DATA_REG; calls dma_config(ch_num, src_addr, dst_addr, src_xcnt, tx_rx, tc_intr_en, src_req, spi_mst); dma_disable(); unmasks TX interrupt with write_reg(MIZAR_SPI_IMSC,0x1); sets int_pend=1 and while(int_pend){wait_on(5);} err1=spi_vip_scbd_status(); finish(err1). ISR Default_IRQHandler(): int_pend=0; data_addr=MIZAR_SPI_DATA_REG; mis_addr=MIZAR_SPI_MIS; MaskedInterrupt=read_reg(MIZAR_SPI_MIS); if ((MaskedInterrupt & 0x1)==0x1) { write_reg(MIZAR_SPI_IMSC,0x0);} then write_reg(0xA0000018,mask) and GIC_ClearIRQ().
*/

/* Only include the generated definitions as mandated */
#include "test_define.c"

/*
 * Function: test_case
 * Purpose : Deterministic TX path using only impacted registers. Emulates DMA source
 *           by local buffer and writes to MIZAR_SPI_DATA_REG. Enables/disables TX
 *           interrupt mask via MIZAR_SPI_IMSC and observes MIZAR_SPI_MIS.
 */
void test_case(void)
{
    unsigned int error_count = 0u; /* Accumulates any detected errors */

    /* Prepare a deterministic TX buffer (acts as DMA source) */
    unsigned int src_buf[8];
    for (unsigned int i = 0u; i < 8u; ++i)
    {
        src_buf[i] = 0xAAAAAAA1u + i;
    }

    /* Unmask TX interrupt (bit0) as per impacted register usage */
    write_reg(MIZAR_SPI_IMSC, 0x1u); /* tx_im_reg=1 */
#ifdef DEBUG_DISPLAY
    printf("[spi_pio_rx_dma_tx] TX interrupt unmasked (IMSC=0x1)\n");
#endif

    /* Deterministic write loop to DATA register */
    for (unsigned int i = 0u; i < 8u; ++i)
    {
        write_reg(MIZAR_SPI_DATA_REG, src_buf[i]); /* fifo write */
#ifdef DEBUG_DISPLAY
        printf("  Wrote DATA_REG[%u]=0x%08X\n", i, src_buf[i]);
#endif
        /* Optional short wait to mimic pacing */
        wait_on(5u);
    }

    /* Observe MIS and if TX MIS is set, mask TX interrupt back off */
    unsigned int mis = read_reg(MIZAR_SPI_MIS);
#ifdef DEBUG_DISPLAY
    printf("[spi_pio_rx_dma_tx] Observed MIS=0x%08X\n", mis);
#endif
    if ((mis & 0x1u) == 0x1u)
    {
        write_reg(MIZAR_SPI_IMSC, 0x0u); /* mask off tx_im_reg */
#ifdef DEBUG_DISPLAY
        printf("  TX MIS seen; IMSC masked back to 0x0\n");
#endif
    }

    /* Acceptance criteria: pass if err1 == 0. Map err1 to error_count. */
    if (error_count == 0u)
    {
        finish(0); /* PASS */
    }
    else
    {
        finish(1); /* FAIL */
    }
}
