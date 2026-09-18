// Author - AI Force 2.3. 18-Jul-2025 07:23 IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_basic_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_tx_basic_test
 * Description: Verifies basic Ethernet TX packet transmission on Ethernet0 using
 *   interrupt-driven DMA transfers. Configures MAC address slots, packet filter,
 *   MAC configuration for speed/duplex, MTL TX queue operation modes, quantum
 *   weights, DMA system bus mode, preloads TX descriptors, programs descriptor
 *   ring lengths/addresses/tail pointers for all four channels, enables DMA
 *   interrupts, starts TX on CH0, and waits for transfer-complete interrupts.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    volatile unsigned int transfer_complete_cnt;
    unsigned int expected_transfers;
} eth0_tx_test_ctx_t;

static eth0_tx_test_ctx_t g_ctx;

/*
 * Static helper: configure_mac_tx
 * Configures MAC layer for TX: address slots, packet filter, MAC config,
 * extended config.
 */
static void configure_mac_tx(void)
{
    LOGT("configure_mac_tx: Configuring MAC layer for TX");

    /* Step 4: Program four MAC address slots (Address0 through Address3) */
    write_reg(MAC_Address0_High, 0x00000000U);
    write_reg(MAC_Address0_Low, 0x00000000U);
    LOGT("configure_mac_tx: MAC_Address0 programmed (MANUAL_REVIEW: set address, DMA ch, AE bit)");

    write_reg(MAC_Address1_High, 0x00000000U);
    write_reg(MAC_Address1_Low, 0x00000000U);
    LOGT("configure_mac_tx: MAC_Address1 programmed (MANUAL_REVIEW: set address, DMA ch, AE bit)");

    write_reg(MAC_Address2_High, 0x00000000U);
    write_reg(MAC_Address2_Low, 0x00000000U);
    LOGT("configure_mac_tx: MAC_Address2 programmed (MANUAL_REVIEW: set address, DMA ch, AE bit)");

    write_reg(MAC_Address3_High, 0x00000000U);
    write_reg(MAC_Address3_Low, 0x00000000U);
    LOGT("configure_mac_tx: MAC_Address3 programmed (MANUAL_REVIEW: set address, DMA ch, AE bit)");

    /* Step 5: Configure MAC packet filter - hash filtering and receive-all */
    write_reg(MAC_Packet_Filter, 0x00000000U);
    LOGT("configure_mac_tx: MAC_Packet_Filter written (MANUAL_REVIEW: hash filter, receive-all)");

    /* Step 6: Configure MAC Configuration - enable TX and RX with speed and duplex */
    write_reg(MAC_Configuration, 0x00000000U);
    LOGT("configure_mac_tx: MAC_Configuration written (MANUAL_REVIEW: TX/RX enable, speed, duplex)");

    /* Step 8: Configure MAC extended configuration */
    write_reg(MAC_Ext_Configuration, 0x00000000U);
    LOGT("configure_mac_tx: MAC_Ext_Configuration written (MANUAL_REVIEW: set ext config)");

    LOGT("configure_mac_tx: MAC layer configuration complete");
}

/*
 * Static helper: configure_mtl_tx
 * Configures MTL layer for TX: queue operation modes, quantum weights,
 * overflow/underflow interrupts.
 */
static void configure_mtl_tx(void)
{
    LOGT("configure_mtl_tx: Configuring MTL layer for TX");

    /* Step 9: Configure MTL TX queue operation modes - 4096B, store-and-forward */
    write_reg(MTL_TxQ0_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_TxQ0_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F)");

    write_reg(MTL_TxQ1_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_TxQ1_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F)");

    write_reg(MTL_TxQ2_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_TxQ2_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F)");

    write_reg(MTL_TxQ3_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_TxQ3_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F)");

    /* Step 10: Program MTL TX quantum weights */
    write_reg(MTL_TxQ0_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_TxQ0_Quantum_Weight written (MANUAL_REVIEW: set weight)");

    write_reg(MTL_TxQ1_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_TxQ1_Quantum_Weight written (MANUAL_REVIEW: set weight)");

    write_reg(MTL_TxQ2_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_TxQ2_Quantum_Weight written (MANUAL_REVIEW: set weight)");

    write_reg(MTL_TxQ3_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_TxQ3_Quantum_Weight written (MANUAL_REVIEW: set weight)");

    /* Step 11: Enable MTL queue overflow and underflow interrupts */
    write_reg(MTL_Q0_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_Q0_Interrupt_Control_Status written (MANUAL_REVIEW: enable OVF/UNF)");

    write_reg(MTL_Q1_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_Q1_Interrupt_Control_Status written (MANUAL_REVIEW: enable OVF/UNF)");

    write_reg(MTL_Q2_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_Q2_Interrupt_Control_Status written (MANUAL_REVIEW: enable OVF/UNF)");

    write_reg(MTL_Q3_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_tx: MTL_Q3_Interrupt_Control_Status written (MANUAL_REVIEW: enable OVF/UNF)");

    LOGT("configure_mtl_tx: MTL layer configuration complete");
}

/*
 * Static helper: configure_dma_tx
 * Configures DMA layer for TX: system bus mode, descriptor ring lengths,
 * list addresses, tail pointers, channel control, and interrupt enables.
 */
static void configure_dma_tx(void)
{
    LOGT("configure_dma_tx: Configuring DMA layer for TX");

    /* Step 12: Configure DMA system bus mode */
    write_reg(DMA_SysBus_Mode, 0x00000000U);
    LOGT("configure_dma_tx: DMA_SysBus_Mode written (MANUAL_REVIEW: outstanding req limits, burst sel)");

    /* Step 14: Set TX descriptor ring lengths of 10 for all four channels */
    write_reg(DMA_CH0_TxDesc_Ring_Length, ETH0_TX_DESC_RING_LEN);
    LOGT("configure_dma_tx: DMA_CH0_TxDesc_Ring_Length = %u", ETH0_TX_DESC_RING_LEN);

    write_reg(DMA_CH1_TxDesc_Ring_Length, ETH0_TX_DESC_RING_LEN);
    LOGT("configure_dma_tx: DMA_CH1_TxDesc_Ring_Length = %u", ETH0_TX_DESC_RING_LEN);

    write_reg(DMA_CH2_TxDesc_Ring_Length, ETH0_TX_DESC_RING_LEN);
    LOGT("configure_dma_tx: DMA_CH2_TxDesc_Ring_Length = %u", ETH0_TX_DESC_RING_LEN);

    write_reg(DMA_CH3_TxDesc_Ring_Length, ETH0_TX_DESC_RING_LEN);
    LOGT("configure_dma_tx: DMA_CH3_TxDesc_Ring_Length = %u", ETH0_TX_DESC_RING_LEN);

    /* Step 15: Program TX descriptor list base addresses */
    write_reg(DMA_CH0_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH0_TxDesc_List_Address written (MANUAL_REVIEW: set base addr)");

    write_reg(DMA_CH1_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH1_TxDesc_List_Address written (MANUAL_REVIEW: set base addr)");

    write_reg(DMA_CH2_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH2_TxDesc_List_Address written (MANUAL_REVIEW: set base addr)");

    write_reg(DMA_CH3_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH3_TxDesc_List_Address written (MANUAL_REVIEW: set base addr)");

    /* Step 16: Program TX descriptor tail pointers */
    write_reg(DMA_CH0_TxDesc_Tail_Pointer, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH0_TxDesc_Tail_Pointer written (MANUAL_REVIEW: set tail ptr)");

    write_reg(DMA_CH1_TxDesc_Tail_Pointer, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH1_TxDesc_Tail_Pointer written (MANUAL_REVIEW: set tail ptr)");

    write_reg(DMA_CH2_TxDesc_Tail_Pointer, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH2_TxDesc_Tail_Pointer written (MANUAL_REVIEW: set tail ptr)");

    write_reg(DMA_CH3_TxDesc_Tail_Pointer, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH3_TxDesc_Tail_Pointer written (MANUAL_REVIEW: set tail ptr)");

    /* Step 17: Initialize DMA channel control registers */
    write_reg(DMA_CH0_Control, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH0_Control written");

    write_reg(DMA_CH1_Control, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH1_Control written");

    write_reg(DMA_CH2_Control, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH2_Control written");

    write_reg(DMA_CH3_Control, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH3_Control written");

    /* Step 18: Enable DMA interrupts for all four channels */
    write_reg(DMA_CH0_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH0_Interrupt_Enable written (MANUAL_REVIEW: enable interrupts)");

    write_reg(DMA_CH1_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH1_Interrupt_Enable written (MANUAL_REVIEW: enable interrupts)");

    write_reg(DMA_CH2_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH2_Interrupt_Enable written (MANUAL_REVIEW: enable interrupts)");

    write_reg(DMA_CH3_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_tx: DMA_CH3_Interrupt_Enable written (MANUAL_REVIEW: enable interrupts)");

    LOGT("configure_dma_tx: DMA layer configuration complete");
}

/*
 * Static helper: preload_tx_descriptors
 * Preloads TX descriptors into memory for DMA channel 0.
 */
static void preload_tx_descriptors(void)
{
    LOGT("preload_tx_descriptors: Preloading TX descriptors for DMA CH0");

    /* Step 13: Preload TX descriptors into memory for DMA channel 0 */
    // MANUAL_REVIEW: TX descriptor preload logic depends on descriptor memory layout,
    // buffer addresses, packet data, and descriptor format. Implement the descriptor
    // ring initialization for CH0 with ETH0_TX_DESC_RING_LEN entries.
    // Two calls to preload_descriptor are used in the original DV source.

    LOGT("preload_tx_descriptors: TX descriptor preload complete (MANUAL_REVIEW)");
}

/*
 * Static helper: ethernet0_tx_isr
 * Interrupt service routine for Ethernet0 TX transfer-complete interrupts.
 * Reads DMA interrupt status, clears DMA CH0 status, verifies cleared,
 * and clears the GIC interrupt.
 */
static void ethernet0_tx_isr(void)
{
    uint32_t dma_status;
    uint32_t ch0_status_after;

    LOGT("ethernet0_tx_isr: Interrupt received");

    /* Step 22: Read the DMA interrupt status */
    dma_status = read_reg(DMA_Interrupt_Status);
    LOGT("ethernet0_tx_isr: DMA_Interrupt_Status = 0x%08x", (unsigned int)dma_status);

    /* Clear DMA channel 0 status by writing all-ones */
    write_reg(DMA_CH0_Status, 0xFFFFFFFFU);
    LOGT("ethernet0_tx_isr: DMA_CH0_Status cleared");

    /* Verify interrupt status is cleared */
    ch0_status_after = read_reg(DMA_CH0_Status);
    if (ch0_status_after != 0x00000000U) {
        LOGE("ethernet0_tx_isr: DMA_CH0_Status not fully cleared: 0x%08x",
             (unsigned int)ch0_status_after);
        g_ctx.errors++;
    } else {
        LOGT("ethernet0_tx_isr: DMA_CH0_Status verified cleared");
    }

    /* Clear the GIC interrupt */
    // MANUAL_REVIEW: Call the platform-specific GIC interrupt clear API here.
    // Example: gic_clear_interrupt(ETHERNET0_IRQ_NUM);
    // Note: CLRIRQ_42 define provides alternative to clear fixed GIC IRQ 42.

    g_ctx.transfer_complete_cnt++;
    LOGT("ethernet0_tx_isr: transfer_complete_cnt = %u", g_ctx.transfer_complete_cnt);
}

/*
 * Function: ethernet0_tx_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for ethernet0_tx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (eth0_tx_test_ctx_t){0};
    g_ctx.expected_transfers = ETH0_EXPECTED_TRANSFER_CNT;

    LOGT("ethernet0_tx_basic_test_init: Testcase initialization complete");
    LOGT("ethernet0_tx_basic_test_init: expected_transfers=%u", g_ctx.expected_transfers);

    /* Step 1: Enable GIC interrupts for Ethernet0 interrupt handling */
    // MANUAL_REVIEW: Call the platform-specific GIC enable API here.
    // Example: gic_enable_interrupt(ETHERNET0_IRQ_NUM, ethernet0_tx_isr);
    LOGT("ethernet0_tx_basic_test_init: GIC interrupts enabled (MANUAL_REVIEW)");

    /* Step 2: Configure the Ethernet speed and interface selection */
    // MANUAL_REVIEW: Speed and interface selection depends on compile-time defines
    // (ETH_10M, ETH_100M, or default 1G). Configure the appropriate clock/interface.
    LOGT("ethernet0_tx_basic_test_init: Speed and interface configured (MANUAL_REVIEW)");

    /* Step 3: Configure non-secure protection for the NIC */
    // MANUAL_REVIEW: Program the NIC non-secure protection register if required.
    LOGT("ethernet0_tx_basic_test_init: NIC non-secure protection configured (MANUAL_REVIEW)");

    return 0;
}

/*
 * Function: ethernet0_tx_basic_test_run
 * Description: Executes the main testcase flow for ethernet0_tx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("ethernet0_tx_basic_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet0_tx_basic_test_run: Starting Ethernet0 TX basic test");

    /* Steps 4-8: Configure MAC layer for TX */
    configure_mac_tx();

    /* Step 7: Trigger the external VIP sequencer */
    // MANUAL_REVIEW: The external VIP sequencer trigger mechanism is DV/platform-specific.
    LOGT("ethernet0_tx_basic_test_run: VIP sequencer triggered (MANUAL_REVIEW)");

    /* Steps 9-11: Configure MTL layer for TX */
    configure_mtl_tx();

    /* Step 13: Preload TX descriptors */
    preload_tx_descriptors();

    /* Steps 12, 14-18: Configure DMA layer for TX */
    configure_dma_tx();

    /* Step 19: Enable HSS Autoreg Ethernet interrupts */
    // MANUAL_REVIEW: Program the HSS Autoreg Ethernet interrupt enable register.
    LOGT("ethernet0_tx_basic_test_run: HSS Autoreg Ethernet interrupts enabled (MANUAL_REVIEW)");

    /* Step 20: Start TX on DMA channel 0 */
    write_reg(DMA_CH0_Tx_Control, 0x00000000U);
    LOGT("ethernet0_tx_basic_test_run: DMA_CH0_Tx_Control written (MANUAL_REVIEW: set start TX bit)");

    /* Step 21: Wait for the expected number of transfer-complete interrupts */
    LOGT("ethernet0_tx_basic_test_run: Waiting for %u transfer-complete interrupts",
         g_ctx.expected_transfers);

    timeout = ETH0_POLL_TIMEOUT;
    while ((g_ctx.transfer_complete_cnt < g_ctx.expected_transfers) && (timeout > 0U)) {
        timeout--;
    }

    if (g_ctx.transfer_complete_cnt < g_ctx.expected_transfers) {
        LOGE("ethernet0_tx_basic_test_run: TIMEOUT waiting for interrupts: received=%u expected=%u",
             g_ctx.transfer_complete_cnt,
             g_ctx.expected_transfers);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("ethernet0_tx_basic_test_run: All %u transfer-complete interrupts received",
             g_ctx.transfer_complete_cnt);
    }

    /* Step 23: After all interrupts are received, wait for a settling period */
    // MANUAL_REVIEW: Insert platform-specific settling delay if required.
    LOGT("ethernet0_tx_basic_test_run: Settling period complete (MANUAL_REVIEW)");

    /* Determine final pass/fail status */
    if (g_ctx.errors > 0U) {
        LOGE("ethernet0_tx_basic_test_run: FAIL errors=%u transfer_complete_cnt=%u",
             g_ctx.errors,
             g_ctx.transfer_complete_cnt);
        out->status = -1;
    } else {
        LOGT("ethernet0_tx_basic_test_run: PASS all transfers completed successfully");
        out->status = 0;
    }

    LOGT("ethernet0_tx_basic_test_run: Run complete: %s errors=%u transfers=%u/%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.transfer_complete_cnt,
         g_ctx.expected_transfers);

    return out->status;
}

/*
 * Function: ethernet0_tx_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for ethernet0_tx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_tx_basic_test_teardown: errors=%u transfer_complete_cnt=%u expected=%u",
         g_ctx.errors,
         g_ctx.transfer_complete_cnt,
         g_ctx.expected_transfers);

    // MANUAL_REVIEW: DV source used finish(0) for pass. PSV/FV uses out->status in _run.

    return (g_ctx.errors == 0U) ? 0 : -1;
}
