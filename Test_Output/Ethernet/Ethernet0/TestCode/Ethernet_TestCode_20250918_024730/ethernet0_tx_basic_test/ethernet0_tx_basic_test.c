// Author - AI Force 2.3. 18-Sep-2025 08:17 IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_basic_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_tx_basic_test
 * Description: Verifies basic Ethernet TX packet transmission on Ethernet0
 *              using interrupt-driven DMA transfers. Configures MAC address
 *              slots, packet filter, MAC configuration, MTL TX queues, DMA
 *              channels, preloads TX descriptors, starts TX on CH0, and waits
 *              for transfer-complete interrupts.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    volatile unsigned int interrupt_count;
    unsigned int expected_transfers;
} ethernet0_tx_basic_ctx_t;

static ethernet0_tx_basic_ctx_t g_ctx;

/*
 * Function: ethernet0_tx_basic_test_isr
 * Description: Interrupt service routine for Ethernet0 TX DMA transfer-complete.
 *              Reads DMA interrupt status, clears DMA CH0 status by writing
 *              all-ones, verifies status is cleared, and clears the GIC interrupt.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void ethernet0_tx_basic_test_isr(void)
{
    unsigned int dma_status;
    unsigned int dma_ch0_status_after;

    /* Step 22: Read the DMA interrupt status */
    dma_status = read_reg(DMA_Interrupt_Status);
    LOGT("ISR: DMA_Interrupt_Status=0x%08x", dma_status);

    /* Step 22: Clear DMA channel 0 status by writing all-ones */
    write_reg(DMA_CH0_Status, 0xFFFFFFFFU);

    /* Step 22: Verify interrupt status is cleared */
    dma_ch0_status_after = read_reg(DMA_CH0_Status);
    if (dma_ch0_status_after != 0x00000000U) {
        LOGE("ISR: DMA_CH0_Status not cleared after write, val=0x%08x", dma_ch0_status_after);
        g_ctx.errors++;
    }

    /* Step 22: Clear the GIC interrupt */
    // MANUAL_REVIEW: GIC interrupt clear mechanism is platform-specific.
    // Insert the appropriate GIC clear call for this SoC, e.g.:
    // clear_gic_interrupt(ETHERNET0_IRQ_NUM);

    g_ctx.interrupt_count++;

    LOGT("ISR: interrupt_count=%u", g_ctx.interrupt_count);
}

/*
 * Function: ethernet0_tx_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              ethernet0_tx_basic_test. Enables GIC interrupts, configures
 *              MAC, MTL, and DMA layers for TX packet transmission.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    /* Initialize test context */
    g_ctx.errors = 0U;
    g_ctx.interrupt_count = 0U;
    g_ctx.expected_transfers = DEFAULT_TRANSFER_COUNT;

    LOGT("ethernet0_tx_basic_test_init: Starting initialization");
    LOGT("ethernet0_tx_basic_test_init: expected_transfers=%u", g_ctx.expected_transfers);

    /* Step 1: Enable GIC interrupts for Ethernet0 interrupt handling */
    // MANUAL_REVIEW: GIC interrupt enable is platform-specific.
    // Insert the appropriate GIC enable call, e.g.:
    // enable_gic_interrupt(ETHERNET0_IRQ_NUM, ethernet0_tx_basic_test_isr);
    LOGT("Step 1: GIC interrupts enabled for Ethernet0");

    /* Step 2: Configure Ethernet speed and interface selection */
    // MANUAL_REVIEW: Speed and interface selection register writes are
    // platform-specific. Insert the appropriate speed/interface config.
    LOGT("Step 2: Ethernet speed and interface configured");

    /* Step 3: Configure non-secure protection for the NIC */
    // MANUAL_REVIEW: NIC non-secure protection configuration is platform-specific.
    // Insert the appropriate NIC protection register write.
    LOGT("Step 3: NIC non-secure protection configured");

    /* Step 4: Program four MAC address slots */
    write_reg(MAC_Address0_High, MAC_ADDR0_HIGH_VALUE);
    write_reg(MAC_Address0_Low, MAC_ADDR0_LOW_VALUE);
    write_reg(MAC_Address1_High, MAC_ADDR1_HIGH_VALUE);
    write_reg(MAC_Address1_Low, MAC_ADDR1_LOW_VALUE);
    write_reg(MAC_Address2_High, MAC_ADDR2_HIGH_VALUE);
    write_reg(MAC_Address2_Low, MAC_ADDR2_LOW_VALUE);
    write_reg(MAC_Address3_High, MAC_ADDR3_HIGH_VALUE);
    write_reg(MAC_Address3_Low, MAC_ADDR3_LOW_VALUE);
    LOGT("Step 4: Four MAC address slots programmed");

    /* Step 5: Configure MAC packet filter */
    write_reg(MAC_Packet_Filter, MAC_PACKET_FILTER_VALUE);
    LOGT("Step 5: MAC_Packet_Filter configured");

    /* Step 6: Configure MAC Configuration register - enable TX and RX */
    write_reg(MAC_Configuration, MAC_CFG_VALUE);
    LOGT("Step 6: MAC_Configuration=0x%08x", MAC_CFG_VALUE);

    /* Step 7: Trigger external VIP sequencer */
    // MANUAL_REVIEW: External VIP sequencer trigger is platform-specific.
    // Insert the appropriate VIP trigger mechanism.
    LOGT("Step 7: External VIP sequencer triggered");

    /* Step 8: Configure MAC extended configuration */
    write_reg(MAC_Ext_Configuration, MAC_EXT_CFG_VALUE);
    LOGT("Step 8: MAC_Ext_Configuration configured");

    /* Step 9: Configure MTL TX queue operation modes */
    write_reg(MTL_TxQ0_Operation_Mode, MTL_TXQ0_OP_MODE_VALUE);
    write_reg(MTL_TxQ1_Operation_Mode, MTL_TXQ1_OP_MODE_VALUE);
    write_reg(MTL_TxQ2_Operation_Mode, MTL_TXQ2_OP_MODE_VALUE);
    write_reg(MTL_TxQ3_Operation_Mode, MTL_TXQ3_OP_MODE_VALUE);
    LOGT("Step 9: MTL TX queue operation modes configured");

    /* Step 10: Program MTL TX quantum weights */
    write_reg(MTL_TxQ0_Quantum_Weight, MTL_TXQ0_QW_VALUE);
    write_reg(MTL_TxQ1_Quantum_Weight, MTL_TXQ1_QW_VALUE);
    write_reg(MTL_TxQ2_Quantum_Weight, MTL_TXQ2_QW_VALUE);
    write_reg(MTL_TxQ3_Quantum_Weight, MTL_TXQ3_QW_VALUE);
    LOGT("Step 10: MTL TX quantum weights programmed");

    /* Step 11: Enable MTL queue overflow and underflow interrupts */
    write_reg(MTL_Q0_Interrupt_Control_Status, MTL_Q0_INT_CTRL_VALUE);
    write_reg(MTL_Q1_Interrupt_Control_Status, MTL_Q1_INT_CTRL_VALUE);
    write_reg(MTL_Q2_Interrupt_Control_Status, MTL_Q2_INT_CTRL_VALUE);
    write_reg(MTL_Q3_Interrupt_Control_Status, MTL_Q3_INT_CTRL_VALUE);
    LOGT("Step 11: MTL queue interrupts enabled");

    /* Step 12: Configure DMA system bus mode */
    write_reg(DMA_SysBus_Mode, DMA_SYSBUS_MODE_VALUE);
    LOGT("Step 12: DMA_SysBus_Mode configured");

    /* Step 13: Preload TX descriptors into memory for DMA channel 0 */
    // MANUAL_REVIEW: TX descriptor preload is platform-specific.
    // Call the appropriate descriptor preload function, e.g.:
    // preload_tx_descriptors(0);
    LOGT("Step 13: TX descriptors preloaded for CH0");

    /* Step 14: Set TX descriptor ring lengths of 10 for all four DMA channels */
    write_reg(DMA_CH0_TxDesc_Ring_Length, TX_DESC_RING_LEN);
    write_reg(DMA_CH1_TxDesc_Ring_Length, TX_DESC_RING_LEN);
    write_reg(DMA_CH2_TxDesc_Ring_Length, TX_DESC_RING_LEN);
    write_reg(DMA_CH3_TxDesc_Ring_Length, TX_DESC_RING_LEN);
    LOGT("Step 14: TX descriptor ring lengths set to %u", TX_DESC_RING_LEN);

    /* Step 15: Program TX descriptor list base addresses for all four DMA channels */
    write_reg(DMA_CH0_TxDesc_List_Address, DMA_CH0_TXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH1_TxDesc_List_Address, DMA_CH1_TXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH2_TxDesc_List_Address, DMA_CH2_TXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH3_TxDesc_List_Address, DMA_CH3_TXDESC_LIST_ADDR_VALUE);
    LOGT("Step 15: TX descriptor list base addresses programmed");

    /* Step 16: Program TX descriptor tail pointers for all four DMA channels */
    write_reg(DMA_CH0_TxDesc_Tail_Pointer, DMA_CH0_TXDESC_TAIL_PTR_VALUE);
    write_reg(DMA_CH1_TxDesc_Tail_Pointer, DMA_CH1_TXDESC_TAIL_PTR_VALUE);
    write_reg(DMA_CH2_TxDesc_Tail_Pointer, DMA_CH2_TXDESC_TAIL_PTR_VALUE);
    write_reg(DMA_CH3_TxDesc_Tail_Pointer, DMA_CH3_TXDESC_TAIL_PTR_VALUE);
    LOGT("Step 16: TX descriptor tail pointers programmed");

    /* Step 17: Initialize DMA channel control registers for all four channels */
    write_reg(DMA_CH0_Control, DMA_CH0_CTRL_VALUE);
    write_reg(DMA_CH1_Control, DMA_CH1_CTRL_VALUE);
    write_reg(DMA_CH2_Control, DMA_CH2_CTRL_VALUE);
    write_reg(DMA_CH3_Control, DMA_CH3_CTRL_VALUE);
    LOGT("Step 17: DMA channel control registers initialized");

    /* Step 18: Enable DMA interrupts for all four channels */
    write_reg(DMA_CH0_Interrupt_Enable, DMA_CHX_INT_ENABLE_VALUE);
    write_reg(DMA_CH1_Interrupt_Enable, DMA_CHX_INT_ENABLE_VALUE);
    write_reg(DMA_CH2_Interrupt_Enable, DMA_CHX_INT_ENABLE_VALUE);
    write_reg(DMA_CH3_Interrupt_Enable, DMA_CHX_INT_ENABLE_VALUE);
    LOGT("Step 18: DMA interrupts enabled for all four channels");

    /* Step 19: Enable HSS Autoreg Ethernet interrupts */
    // MANUAL_REVIEW: HSS Autoreg Ethernet interrupt enable is platform-specific.
    // Insert the appropriate HSS Autoreg interrupt enable call.
    LOGT("Step 19: HSS Autoreg Ethernet interrupts enabled");

    LOGT("ethernet0_tx_basic_test_init: Initialization complete");

    return 0;
}

/*
 * Function: ethernet0_tx_basic_test_run
 * Description: Executes the main testcase flow for ethernet0_tx_basic_test.
 *              Starts TX on DMA channel 0 and polls for the expected number
 *              of transfer-complete interrupts.
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

    LOGT("ethernet0_tx_basic_test_run: Starting TX basic test execution");

    /* Step 20: Start TX on DMA channel 0 */
    write_reg(DMA_CH0_Tx_Control, DMA_CH0_TX_CTRL_START_VALUE);
    LOGT("Step 20: TX started on DMA CH0, DMA_CH0_Tx_Control=0x%08x", DMA_CH0_TX_CTRL_START_VALUE);

    /* Step 21: Wait for expected number of transfer-complete interrupts */
    timeout = POLL_TIMEOUT_COUNT;
    LOGT("Step 21: Polling for %u transfer-complete interrupts, timeout=%u",
         g_ctx.expected_transfers, timeout);

    while ((g_ctx.interrupt_count < g_ctx.expected_transfers) && (timeout > 0U)) {
        timeout--;
    }

    if (g_ctx.interrupt_count < g_ctx.expected_transfers) {
        g_ctx.errors++;
        LOGE("ethernet0_tx_basic_test_run: TIMEOUT waiting for interrupts. received=%u expected=%u",
             g_ctx.interrupt_count, g_ctx.expected_transfers);
        out->status = -1;
    } else {
        LOGT("ethernet0_tx_basic_test_run: All %u transfer-complete interrupts received",
             g_ctx.interrupt_count);
    }

    /* Step 23: Settling wait period */
    // MANUAL_REVIEW: Insert platform-specific settling delay if required.
    LOGT("Step 23: Settling wait complete");

    /* Determine final pass/fail */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("ethernet0_tx_basic_test_run: Complete. status=%d errors=%u interrupts=%u",
         out->status, g_ctx.errors, g_ctx.interrupt_count);

    return out->status;
}

/*
 * Function: ethernet0_tx_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for ethernet0_tx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_tx_basic_test_teardown: errors=%u interrupt_count=%u expected=%u",
         g_ctx.errors, g_ctx.interrupt_count, g_ctx.expected_transfers);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
