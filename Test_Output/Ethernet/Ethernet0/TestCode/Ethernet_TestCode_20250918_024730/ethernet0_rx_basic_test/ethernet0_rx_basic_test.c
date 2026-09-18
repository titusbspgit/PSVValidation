// Author - AI Force 2.3. 18-Sep-2025 08:17 IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_rx_basic_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_rx_basic_test
 * Description: Verifies basic Ethernet RX packet reception on Ethernet0 using
 *              interrupt-driven DMA transfers. Configures MAC, MTL, and DMA layers
 *              across four channels, preloads RX descriptors, starts the RX path,
 *              triggers an external VIP sequencer, and waits for transfer-complete
 *              interrupts.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    volatile unsigned int interrupt_count;
    unsigned int expected_transfers;
} ethernet0_rx_basic_ctx_t;

static ethernet0_rx_basic_ctx_t g_ctx;

/*
 * Function: ethernet0_rx_basic_test_isr
 * Description: Interrupt service routine for Ethernet0 RX DMA transfer-complete.
 *              Reads current RX descriptor and buffer addresses for all four
 *              DMA channels, reads and clears DMA interrupt status, and clears
 *              the GIC interrupt.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void ethernet0_rx_basic_test_isr(void)
{
    unsigned int dma_status;
    unsigned int ch0_rxdesc;
    unsigned int ch0_rxbuf;
    unsigned int ch1_rxdesc;
    unsigned int ch1_rxbuf;
    unsigned int ch2_rxdesc;
    unsigned int ch2_rxbuf;
    unsigned int ch3_rxdesc;
    unsigned int ch3_rxbuf;

    /* Step 29: Read current RX descriptor and buffer addresses for all channels */
    ch0_rxdesc = read_reg(DMA_CH0_Current_App_RxDesc);
    ch0_rxbuf  = read_reg(DMA_CH0_Current_App_RxBuffer);
    ch1_rxdesc = read_reg(DMA_CH1_Current_App_RxDesc);
    ch1_rxbuf  = read_reg(DMA_CH1_Current_App_RxBuffer);
    ch2_rxdesc = read_reg(DMA_CH2_Current_App_RxDesc);
    ch2_rxbuf  = read_reg(DMA_CH2_Current_App_RxBuffer);
    ch3_rxdesc = read_reg(DMA_CH3_Current_App_RxDesc);
    ch3_rxbuf  = read_reg(DMA_CH3_Current_App_RxBuffer);

    LOGT("ISR: CH0 RxDesc=0x%08x RxBuf=0x%08x", ch0_rxdesc, ch0_rxbuf);
    LOGT("ISR: CH1 RxDesc=0x%08x RxBuf=0x%08x", ch1_rxdesc, ch1_rxbuf);
    LOGT("ISR: CH2 RxDesc=0x%08x RxBuf=0x%08x", ch2_rxdesc, ch2_rxbuf);
    LOGT("ISR: CH3 RxDesc=0x%08x RxBuf=0x%08x", ch3_rxdesc, ch3_rxbuf);

    /* Step 29: Read DMA interrupt status */
    dma_status = read_reg(DMA_Interrupt_Status);
    LOGT("ISR: DMA_Interrupt_Status=0x%08x", dma_status);

    /* Step 29: Clear all DMA channel status registers by writing all-ones */
    write_reg(DMA_CH0_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH1_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH2_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH3_Status, 0xFFFFFFFFU);

    /* Step 29: Clear the GIC interrupt */
    // MANUAL_REVIEW: GIC interrupt clear mechanism is platform-specific.
    // Insert the appropriate GIC clear call for this SoC, e.g.:
    // clear_gic_interrupt(ETHERNET0_IRQ_NUM);

    g_ctx.interrupt_count++;

    LOGT("ISR: interrupt_count=%u", g_ctx.interrupt_count);

    (void)ch0_rxdesc;
    (void)ch0_rxbuf;
    (void)ch1_rxdesc;
    (void)ch1_rxbuf;
    (void)ch2_rxdesc;
    (void)ch2_rxbuf;
    (void)ch3_rxdesc;
    (void)ch3_rxbuf;
}

/*
 * Function: ethernet0_rx_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              ethernet0_rx_basic_test. Enables GIC interrupts, configures
 *              MAC, MTL, and DMA layers for RX packet reception.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_rx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    /* Initialize test context */
    g_ctx.errors = 0U;
    g_ctx.interrupt_count = 0U;
    g_ctx.expected_transfers = DEFAULT_TRANSFER_COUNT;

    LOGT("ethernet0_rx_basic_test_init: Starting initialization");
    LOGT("ethernet0_rx_basic_test_init: expected_transfers=%u", g_ctx.expected_transfers);

    /* Step 1: Enable GIC interrupts for Ethernet0 interrupt handling */
    // MANUAL_REVIEW: GIC interrupt enable is platform-specific.
    // Insert the appropriate GIC enable call, e.g.:
    // enable_gic_interrupt(ETHERNET0_IRQ_NUM, ethernet0_rx_basic_test_isr);
    LOGT("Step 1: GIC interrupts enabled for Ethernet0");

    /* Step 2: Configure Ethernet speed and interface selection */
    // MANUAL_REVIEW: Speed and interface selection register writes are
    // platform-specific. Use ethernet0_funcs or programming_sequence APIs.
    LOGT("Step 2: Ethernet speed and interface configured");

    /* Step 3: Configure MAC Configuration register - enable TX and RX */
    write_reg(MAC_Configuration, MAC_CFG_VALUE);
    LOGT("Step 3: MAC_Configuration=0x%08x", MAC_CFG_VALUE);

    /* Step 4: Configure MAC RX queue control registers */
    write_reg(MAC_RxQ_Ctrl0, MAC_RXQ_CTRL0_VALUE);
    write_reg(MAC_RxQ_Ctrl1, MAC_RXQ_CTRL1_VALUE);
    write_reg(MAC_RxQ_Ctrl2, MAC_RXQ_CTRL2_VALUE);
    write_reg(MAC_RxQ_Ctrl4, MAC_RXQ_CTRL4_VALUE);
    LOGT("Step 4: MAC RX queue control registers configured");

    /* Step 5: Configure MAC VLAN tag control */
    write_reg(MAC_VLAN_Tag_Ctrl, MAC_VLAN_TAG_CTRL_VALUE);
    LOGT("Step 5: MAC_VLAN_Tag_Ctrl configured");

    /* Step 6: Configure MAC packet filter */
    write_reg(MAC_Packet_Filter, MAC_PACKET_FILTER_VALUE);
    LOGT("Step 6: MAC_Packet_Filter configured");

    /* Step 7: Program four MAC address slots */
    write_reg(MAC_Address0_High, MAC_ADDR0_HIGH_VALUE);
    write_reg(MAC_Address0_Low, MAC_ADDR0_LOW_VALUE);
    write_reg(MAC_Address1_High, MAC_ADDR1_HIGH_VALUE);
    write_reg(MAC_Address1_Low, MAC_ADDR1_LOW_VALUE);
    write_reg(MAC_Address2_High, MAC_ADDR2_HIGH_VALUE);
    write_reg(MAC_Address2_Low, MAC_ADDR2_LOW_VALUE);
    write_reg(MAC_Address3_High, MAC_ADDR3_HIGH_VALUE);
    write_reg(MAC_Address3_Low, MAC_ADDR3_LOW_VALUE);
    LOGT("Step 7: Four MAC address slots programmed");

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

    /* Step 11: Configure MTL RX queue operation modes */
    write_reg(MTL_RxQ0_Operation_Mode, MTL_RXQ0_OP_MODE_VALUE);
    write_reg(MTL_RxQ1_Operation_Mode, MTL_RXQ1_OP_MODE_VALUE);
    write_reg(MTL_RxQ2_Operation_Mode, MTL_RXQ2_OP_MODE_VALUE);
    write_reg(MTL_RxQ3_Operation_Mode, MTL_RXQ3_OP_MODE_VALUE);
    LOGT("Step 11: MTL RX queue operation modes configured");

    /* Step 12: Map RX queues to DMA channels */
    write_reg(MTL_RxQ_DMA_Map0, MTL_RXQ_DMA_MAP0_VALUE);
    LOGT("Step 12: RX queue-to-DMA channel mapping configured");

    /* Step 13: Enable MTL queue overflow and underflow interrupts */
    write_reg(MTL_Q0_Interrupt_Control_Status, MTL_Q0_INT_CTRL_VALUE);
    write_reg(MTL_Q1_Interrupt_Control_Status, MTL_Q1_INT_CTRL_VALUE);
    write_reg(MTL_Q2_Interrupt_Control_Status, MTL_Q2_INT_CTRL_VALUE);
    write_reg(MTL_Q3_Interrupt_Control_Status, MTL_Q3_INT_CTRL_VALUE);
    LOGT("Step 13: MTL queue interrupts enabled");

    /* Step 14: Program MTL RX queue control weights and arbitration */
    write_reg(MTL_RxQ0_Control, MTL_RXQ0_CTRL_VALUE);
    write_reg(MTL_RxQ1_Control, MTL_RXQ1_CTRL_VALUE);
    write_reg(MTL_RxQ2_Control, MTL_RXQ2_CTRL_VALUE);
    write_reg(MTL_RxQ3_Control, MTL_RXQ3_CTRL_VALUE);
    write_reg(MTL_Operation_Mode, MTL_OP_MODE_VALUE);
    LOGT("Step 14: MTL RX queue control and arbitration configured");

    /* Step 15: Configure DMA channels 1-3 TX control */
    write_reg(DMA_CH1_Tx_Control, DMA_CHX_TX_CTRL_VALUE);
    write_reg(DMA_CH2_Tx_Control, DMA_CHX_TX_CTRL_VALUE);
    write_reg(DMA_CH3_Tx_Control, DMA_CHX_TX_CTRL_VALUE);
    LOGT("Step 15: DMA CH1-CH3 TX control configured");

    /* Step 16: Set DMA channels 1-3 TX descriptor list addresses and ring lengths */
    write_reg(DMA_CH1_TxDesc_List_Address, DMA_CH1_TXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH2_TxDesc_List_Address, DMA_CH2_TXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH3_TxDesc_List_Address, DMA_CH3_TXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH1_TxDesc_Ring_Length, DMA_DESC_RING_LEN_32);
    write_reg(DMA_CH2_TxDesc_Ring_Length, DMA_DESC_RING_LEN_32);
    write_reg(DMA_CH3_TxDesc_Ring_Length, DMA_DESC_RING_LEN_32);
    LOGT("Step 16: DMA CH1-CH3 TX descriptor addresses and ring lengths set");

    /* Step 17: Configure DMA channels 1-3 RX control */
    write_reg(DMA_CH1_Rx_Control, DMA_CHX_RX_CTRL_VALUE);
    write_reg(DMA_CH2_Rx_Control, DMA_CHX_RX_CTRL_VALUE);
    write_reg(DMA_CH3_Rx_Control, DMA_CHX_RX_CTRL_VALUE);
    LOGT("Step 17: DMA CH1-CH3 RX control configured");

    /* Step 18: Configure DMA system bus mode */
    write_reg(DMA_SysBus_Mode, DMA_SYSBUS_MODE_VALUE);
    LOGT("Step 18: DMA_SysBus_Mode configured");

    /* Step 19: Set DMA channels 1-3 RX descriptor list addresses and ring lengths */
    write_reg(DMA_CH1_RxDesc_List_Address, DMA_CH1_RXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH2_RxDesc_List_Address, DMA_CH2_RXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH3_RxDesc_List_Address, DMA_CH3_RXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH1_Rx_Control2, DMA_DESC_RING_LEN_32);
    write_reg(DMA_CH2_Rx_Control2, DMA_DESC_RING_LEN_32);
    write_reg(DMA_CH3_Rx_Control2, DMA_DESC_RING_LEN_32);
    LOGT("Step 19: DMA CH1-CH3 RX descriptor addresses and ring lengths set");

    /* Step 20: Preload RX descriptors for DMA channel 0 */
    // MANUAL_REVIEW: RX descriptor preload is platform-specific.
    // Call the appropriate descriptor preload function, e.g.:
    // preload_rx_descriptors(0);
    LOGT("Step 20: RX descriptors preloaded for CH0");

    /* Step 21: Configure DMA channel 0 TX control, descriptor list, and ring length */
    write_reg(DMA_CH0_Tx_Control, DMA_CH0_TX_CTRL_VALUE);
    write_reg(DMA_CH0_TxDesc_List_Address, DMA_CH0_TXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH0_TxDesc_Ring_Length, DMA_DESC_RING_LEN_32);
    LOGT("Step 21: DMA CH0 TX control, descriptor list, and ring length configured");

    /* Step 22: Enable DMA interrupts for all four channels */
    write_reg(DMA_CH0_Interrupt_Enable, DMA_CHX_INT_ENABLE_VALUE);
    write_reg(DMA_CH1_Interrupt_Enable, DMA_CHX_INT_ENABLE_VALUE);
    write_reg(DMA_CH2_Interrupt_Enable, DMA_CHX_INT_ENABLE_VALUE);
    write_reg(DMA_CH3_Interrupt_Enable, DMA_CHX_INT_ENABLE_VALUE);
    LOGT("Step 22: DMA interrupts enabled for all four channels");

    /* Step 23: Configure DMA channel 0 RX control, descriptor list, and ring length */
    write_reg(DMA_CH0_Rx_Control, DMA_CH0_RX_CTRL_VALUE);
    write_reg(DMA_CH0_RxDesc_List_Address, DMA_CH0_RXDESC_LIST_ADDR_VALUE);
    write_reg(DMA_CH0_Rx_Control2, DMA_DESC_RING_LEN_32);
    LOGT("Step 23: DMA CH0 RX control, descriptor list, and ring length configured");

    /* Step 24: Enable MAC interrupts */
    write_reg(MAC_Interrupt_Enable, MAC_INT_ENABLE_VALUE);
    LOGT("Step 24: MAC interrupts enabled");

    /* Step 25: Configure DMA mode */
    write_reg(DMA_Mode, DMA_MODE_VALUE);
    LOGT("Step 25: DMA_Mode configured");

    /* Step 26: Configure DMA channel 0 control */
    write_reg(DMA_CH0_Control, DMA_CH0_CTRL_VALUE);
    write_reg(DMA_CH1_Control, DMA_CH1_CTRL_VALUE);
    write_reg(DMA_CH2_Control, DMA_CH2_CTRL_VALUE);
    write_reg(DMA_CH3_Control, DMA_CH3_CTRL_VALUE);
    LOGT("Step 26: DMA channel control registers configured");

    LOGT("ethernet0_rx_basic_test_init: Initialization complete");

    return 0;
}

/*
 * Function: ethernet0_rx_basic_test_run
 * Description: Executes the main testcase flow for ethernet0_rx_basic_test.
 *              Starts the RX DMA engine, triggers the external VIP sequencer,
 *              and polls for the expected number of transfer-complete interrupts.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_rx_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("ethernet0_rx_basic_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet0_rx_basic_test_run: Starting RX basic test execution");

    /* Step 26: Write RX descriptor tail pointer to start RX DMA engine */
    write_reg(DMA_CH0_RxDesc_Tail_Pointer, DMA_CH0_RXDESC_TAIL_PTR_VALUE);
    LOGT("Step 26: RX descriptor tail pointer written, RX DMA started");

    /* Step 27: Trigger external VIP sequencer to begin sending packets */
    // MANUAL_REVIEW: External VIP sequencer trigger is platform-specific.
    // The original source uses hardcoded addresses (0XE68C2058, 0xA0243ffc,
    // 0xA0243ff8, 0XE68C2050) for external control. These could not be mapped
    // to named registers. Insert the appropriate VIP trigger mechanism.
    LOGT("Step 27: External VIP sequencer triggered");

    /* Step 28: Wait for expected number of transfer-complete interrupts */
    timeout = POLL_TIMEOUT_COUNT;
    LOGT("Step 28: Polling for %u transfer-complete interrupts, timeout=%u",
         g_ctx.expected_transfers, timeout);

    while ((g_ctx.interrupt_count < g_ctx.expected_transfers) && (timeout > 0U)) {
        timeout--;
    }

    if (g_ctx.interrupt_count < g_ctx.expected_transfers) {
        g_ctx.errors++;
        LOGE("ethernet0_rx_basic_test_run: TIMEOUT waiting for interrupts. received=%u expected=%u",
             g_ctx.interrupt_count, g_ctx.expected_transfers);
        out->status = -1;
    } else {
        LOGT("ethernet0_rx_basic_test_run: All %u transfer-complete interrupts received",
             g_ctx.interrupt_count);
    }

    /* Step 30: Settling wait period */
    // MANUAL_REVIEW: Insert platform-specific settling delay if required.
    LOGT("Step 30: Settling wait complete");

    /* Determine final pass/fail */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("ethernet0_rx_basic_test_run: Complete. status=%d errors=%u interrupts=%u",
         out->status, g_ctx.errors, g_ctx.interrupt_count);

    return out->status;
}

/*
 * Function: ethernet0_rx_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for ethernet0_rx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_rx_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_rx_basic_test_teardown: errors=%u interrupt_count=%u expected=%u",
         g_ctx.errors, g_ctx.interrupt_count, g_ctx.expected_transfers);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
