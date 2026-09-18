// Author - AI Force 2.3. 18-Jul-2025 07:23 IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_rx_basic_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_rx_basic_test
 * Description: Verifies basic Ethernet RX packet reception on Ethernet0 using
 *   interrupt-driven DMA transfers. Configures MAC, MTL, and DMA layers across
 *   all four channels (CH0-CH3), preloads RX descriptors, starts the RX path,
 *   triggers an external VIP sequencer, and waits for transfer-complete interrupts.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    volatile unsigned int transfer_complete_cnt;
    unsigned int expected_transfers;
} eth0_rx_test_ctx_t;

static eth0_rx_test_ctx_t g_ctx;

/*
 * Static helper: configure_mac_rx
 * Configures MAC layer for RX: MAC configuration, RX queue control,
 * VLAN tag control, packet filter, MAC address slots, extended config.
 */
static void configure_mac_rx(void)
{
    LOGT("configure_mac_rx: Configuring MAC layer for RX");

    /* Step 3: Configure MAC Configuration - enable TX and RX with speed and duplex */
    write_reg(MAC_Configuration, 0x00000000U);
    LOGT("configure_mac_rx: MAC_Configuration written (MANUAL_REVIEW: TX/RX enable, speed, duplex)");

    /* Step 4: Configure MAC RX queue control registers */
    write_reg(MAC_RxQ_Ctrl0, 0x00000000U);
    LOGT("configure_mac_rx: MAC_RxQ_Ctrl0 written (MANUAL_REVIEW: DCB/Generic queue enable)");

    write_reg(MAC_RxQ_Ctrl1, 0x00000000U);
    LOGT("configure_mac_rx: MAC_RxQ_Ctrl1 written (MANUAL_REVIEW: queue priorities)");

    write_reg(MAC_RxQ_Ctrl2, 0x00000000U);
    LOGT("configure_mac_rx: MAC_RxQ_Ctrl2 written (MANUAL_REVIEW: queue priorities)");

    write_reg(MAC_RxQ_Ctrl4, 0x00000000U);
    LOGT("configure_mac_rx: MAC_RxQ_Ctrl4 written (MANUAL_REVIEW: queue control)");

    /* Step 5: Configure MAC VLAN tag control */
    write_reg(MAC_VLAN_Tag_Ctrl, 0x00000000U);
    LOGT("configure_mac_rx: MAC_VLAN_Tag_Ctrl written (MANUAL_REVIEW: VLAN tag in status, RX stripping)");

    /* Step 6: Configure MAC packet filter */
    write_reg(MAC_Packet_Filter, 0x00000000U);
    LOGT("configure_mac_rx: MAC_Packet_Filter written (MANUAL_REVIEW: hash filtering, receive-all)");

    /* Step 7: Program four MAC address slots (Address0 through Address3) */
    write_reg(MAC_Address0_High, 0x00000000U);
    write_reg(MAC_Address0_Low, 0x00000000U);
    LOGT("configure_mac_rx: MAC_Address0 programmed (MANUAL_REVIEW: set address, DMA ch)");

    write_reg(MAC_Address1_High, 0x00000000U);
    write_reg(MAC_Address1_Low, 0x00000000U);
    LOGT("configure_mac_rx: MAC_Address1 programmed (MANUAL_REVIEW: set address, DMA ch)");

    write_reg(MAC_Address2_High, 0x00000000U);
    write_reg(MAC_Address2_Low, 0x00000000U);
    LOGT("configure_mac_rx: MAC_Address2 programmed (MANUAL_REVIEW: set address, DMA ch)");

    write_reg(MAC_Address3_High, 0x00000000U);
    write_reg(MAC_Address3_Low, 0x00000000U);
    LOGT("configure_mac_rx: MAC_Address3 programmed (MANUAL_REVIEW: set address, DMA ch)");

    /* Step 8: Configure MAC extended configuration */
    write_reg(MAC_Ext_Configuration, 0x00000000U);
    LOGT("configure_mac_rx: MAC_Ext_Configuration written (MANUAL_REVIEW)");

    LOGT("configure_mac_rx: MAC layer configuration complete");
}

/*
 * Static helper: configure_mtl_rx
 * Configures MTL layer: TX queue operation modes, quantum weights,
 * RX queue operation modes, queue-to-DMA mapping, overflow/underflow
 * interrupts, RX queue control weights.
 */
static void configure_mtl_rx(void)
{
    LOGT("configure_mtl_rx: Configuring MTL layer");

    /* Step 9: Configure MTL TX queue operation modes - 4096B, store-and-forward */
    write_reg(MTL_TxQ0_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_TxQ0_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F, enable Q0)");

    write_reg(MTL_TxQ1_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_TxQ1_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F)");

    write_reg(MTL_TxQ2_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_TxQ2_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F)");

    write_reg(MTL_TxQ3_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_TxQ3_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F)");

    /* Step 10: Program MTL TX quantum weights */
    write_reg(MTL_TxQ0_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_TxQ0_Quantum_Weight written (MANUAL_REVIEW)");

    write_reg(MTL_TxQ1_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_TxQ1_Quantum_Weight written (MANUAL_REVIEW)");

    write_reg(MTL_TxQ2_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_TxQ2_Quantum_Weight written (MANUAL_REVIEW)");

    write_reg(MTL_TxQ3_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_TxQ3_Quantum_Weight written (MANUAL_REVIEW)");

    /* Configure MTL operation mode */
    write_reg(MTL_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_Operation_Mode written (MANUAL_REVIEW)");

    /* Step 11: Configure MTL RX queue operation modes - S&F, error pkt fwd, 4096B */
    write_reg(MTL_RxQ0_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ0_Operation_Mode written (MANUAL_REVIEW: S&F, err fwd, 4096B)");

    write_reg(MTL_RxQ1_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ1_Operation_Mode written (MANUAL_REVIEW: S&F, err fwd, 4096B)");

    write_reg(MTL_RxQ2_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ2_Operation_Mode written (MANUAL_REVIEW: S&F, err fwd, 4096B)");

    write_reg(MTL_RxQ3_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ3_Operation_Mode written (MANUAL_REVIEW: S&F, err fwd, 4096B)");

    /* Step 12: Map RX queues to corresponding DMA channels */
    write_reg(MTL_RxQ_DMA_Map0, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ_DMA_Map0 written (MANUAL_REVIEW: Q0->CH0..Q3->CH3)");

    /* Step 13: Enable MTL queue overflow and underflow interrupts */
    write_reg(MTL_Q0_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_Q0_Interrupt_Control_Status written (MANUAL_REVIEW: OVF/UNF)");

    write_reg(MTL_Q1_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_Q1_Interrupt_Control_Status written (MANUAL_REVIEW: OVF/UNF)");

    write_reg(MTL_Q2_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_Q2_Interrupt_Control_Status written (MANUAL_REVIEW: OVF/UNF)");

    write_reg(MTL_Q3_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_Q3_Interrupt_Control_Status written (MANUAL_REVIEW: OVF/UNF)");

    /* Step 14: Program MTL RX queue control weights and arbitration */
    write_reg(MTL_RxQ0_Control, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ0_Control written (MANUAL_REVIEW: weight/arbitration)");

    write_reg(MTL_RxQ1_Control, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ1_Control written (MANUAL_REVIEW: weight/arbitration)");

    write_reg(MTL_RxQ2_Control, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ2_Control written (MANUAL_REVIEW: weight/arbitration)");

    write_reg(MTL_RxQ3_Control, 0x00000000U);
    LOGT("configure_mtl_rx: MTL_RxQ3_Control written (MANUAL_REVIEW: weight/arbitration)");

    LOGT("configure_mtl_rx: MTL layer configuration complete");
}

/*
 * Static helper: configure_dma_rx
 * Configures DMA layer for all four channels: TX/RX control, descriptor
 * list addresses, ring lengths, system bus mode, interrupt enables,
 * DMA mode, and MAC interrupt enable.
 */
static void configure_dma_rx(void)
{
    LOGT("configure_dma_rx: Configuring DMA layer");

    /* Step 15: Configure DMA channels 1-3 TX control with 16-beat burst */
    write_reg(DMA_CH1_Tx_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH1_Tx_Control written (MANUAL_REVIEW: 16-beat burst, weight)");

    write_reg(DMA_CH2_Tx_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH2_Tx_Control written (MANUAL_REVIEW: 16-beat burst, weight)");

    write_reg(DMA_CH3_Tx_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH3_Tx_Control written (MANUAL_REVIEW: 16-beat burst, weight)");

    /* Step 16: Set DMA channels 1-3 TX descriptor list addresses and ring lengths */
    write_reg(DMA_CH1_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH1_TxDesc_List_Address written (MANUAL_REVIEW)");

    write_reg(DMA_CH2_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH2_TxDesc_List_Address written (MANUAL_REVIEW)");

    write_reg(DMA_CH3_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH3_TxDesc_List_Address written (MANUAL_REVIEW)");

    write_reg(DMA_CH1_TxDesc_Ring_Length, ETH0_RX_DESC_RING_LEN);
    LOGT("configure_dma_rx: DMA_CH1_TxDesc_Ring_Length = %u", ETH0_RX_DESC_RING_LEN);

    write_reg(DMA_CH2_TxDesc_Ring_Length, ETH0_RX_DESC_RING_LEN);
    LOGT("configure_dma_rx: DMA_CH2_TxDesc_Ring_Length = %u", ETH0_RX_DESC_RING_LEN);

    write_reg(DMA_CH3_TxDesc_Ring_Length, ETH0_RX_DESC_RING_LEN);
    LOGT("configure_dma_rx: DMA_CH3_TxDesc_Ring_Length = %u", ETH0_RX_DESC_RING_LEN);

    /* Step 17: Configure DMA channels 1-3 RX control with 16-beat burst and RX buffer size */
    write_reg(DMA_CH1_Rx_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH1_Rx_Control written (MANUAL_REVIEW: 16-beat burst, buf size)");

    write_reg(DMA_CH2_Rx_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH2_Rx_Control written (MANUAL_REVIEW: 16-beat burst, buf size)");

    write_reg(DMA_CH3_Rx_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH3_Rx_Control written (MANUAL_REVIEW: 16-beat burst, buf size)");

    /* Step 18: Configure DMA system bus mode */
    write_reg(DMA_SysBus_Mode, 0x00000000U);
    LOGT("configure_dma_rx: DMA_SysBus_Mode written (MANUAL_REVIEW: outstanding req, burst sel)");

    /* Step 19: Set DMA channels 1-3 RX descriptor list addresses and ring lengths */
    write_reg(DMA_CH1_RxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH1_RxDesc_List_Address written (MANUAL_REVIEW)");

    write_reg(DMA_CH2_RxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH2_RxDesc_List_Address written (MANUAL_REVIEW)");

    write_reg(DMA_CH3_RxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH3_RxDesc_List_Address written (MANUAL_REVIEW)");

    write_reg(DMA_CH1_Rx_Control2, ETH0_RX_DESC_RING_LEN);
    LOGT("configure_dma_rx: DMA_CH1_Rx_Control2 = %u", ETH0_RX_DESC_RING_LEN);

    write_reg(DMA_CH2_Rx_Control2, ETH0_RX_DESC_RING_LEN);
    LOGT("configure_dma_rx: DMA_CH2_Rx_Control2 = %u", ETH0_RX_DESC_RING_LEN);

    write_reg(DMA_CH3_Rx_Control2, ETH0_RX_DESC_RING_LEN);
    LOGT("configure_dma_rx: DMA_CH3_Rx_Control2 = %u", ETH0_RX_DESC_RING_LEN);

    /* Initialize DMA channel control registers for channels 1-3 */
    write_reg(DMA_CH1_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH1_Control written");

    write_reg(DMA_CH2_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH2_Control written");

    write_reg(DMA_CH3_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH3_Control written");

    /* Step 21: Configure DMA channel 0 TX control, TX descriptor list address, ring length */
    write_reg(DMA_CH0_Tx_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH0_Tx_Control written (MANUAL_REVIEW)");

    write_reg(DMA_CH0_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH0_TxDesc_List_Address written (MANUAL_REVIEW)");

    write_reg(DMA_CH0_TxDesc_Ring_Length, ETH0_RX_DESC_RING_LEN);
    LOGT("configure_dma_rx: DMA_CH0_TxDesc_Ring_Length = %u", ETH0_RX_DESC_RING_LEN);

    /* Step 22: Enable DMA interrupts for all four channels */
    write_reg(DMA_CH0_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH0_Interrupt_Enable written (MANUAL_REVIEW: enable interrupts)");

    write_reg(DMA_CH1_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH1_Interrupt_Enable written (MANUAL_REVIEW: enable interrupts)");

    write_reg(DMA_CH2_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH2_Interrupt_Enable written (MANUAL_REVIEW: enable interrupts)");

    write_reg(DMA_CH3_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH3_Interrupt_Enable written (MANUAL_REVIEW: enable interrupts)");

    /* Initialize DMA channel 0 control */
    write_reg(DMA_CH0_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH0_Control written");

    /* Step 23: Configure DMA channel 0 RX control, RX descriptor list address, RX ring length */
    write_reg(DMA_CH0_Rx_Control, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH0_Rx_Control written (MANUAL_REVIEW: 16-beat burst, buf size)");

    write_reg(DMA_CH0_RxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_rx: DMA_CH0_RxDesc_List_Address written (MANUAL_REVIEW)");

    write_reg(DMA_CH0_Rx_Control2, ETH0_RX_DESC_RING_LEN);
    LOGT("configure_dma_rx: DMA_CH0_Rx_Control2 = %u", ETH0_RX_DESC_RING_LEN);

    /* Step 24: Enable MAC interrupts */
    write_reg(MAC_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_rx: MAC_Interrupt_Enable written (MANUAL_REVIEW)");

    /* Step 25: Configure DMA mode */
    write_reg(DMA_Mode, 0x00000000U);
    LOGT("configure_dma_rx: DMA_Mode written (MANUAL_REVIEW)");

    LOGT("configure_dma_rx: DMA layer configuration complete");
}

/*
 * Static helper: preload_rx_descriptors
 * Preloads RX descriptors into memory for DMA channel 0.
 */
static void preload_rx_descriptors(void)
{
    LOGT("preload_rx_descriptors: Preloading RX descriptors for DMA CH0");

    /* Step 20: Preload RX descriptors for DMA channel 0 */
    // MANUAL_REVIEW: RX descriptor preload logic depends on descriptor memory layout,
    // buffer addresses, and descriptor format. Implement the descriptor ring
    // initialization for CH0 with ETH0_RX_DESC_RING_LEN entries.
    // Each descriptor must have OWN bit set and buffer address programmed.

    LOGT("preload_rx_descriptors: RX descriptor preload complete (MANUAL_REVIEW)");
}

/*
 * Static helper: ethernet0_rx_isr
 * Interrupt service routine for Ethernet0 RX transfer-complete interrupts.
 * Reads current RX descriptor and buffer addresses for all four DMA channels,
 * reads and clears DMA interrupt status, and clears the GIC interrupt.
 */
static void ethernet0_rx_isr(void)
{
    uint32_t dma_status;
    uint32_t ch0_rx_desc;
    uint32_t ch0_rx_buf;
    uint32_t ch1_rx_desc;
    uint32_t ch1_rx_buf;
    uint32_t ch2_rx_desc;
    uint32_t ch2_rx_buf;
    uint32_t ch3_rx_desc;
    uint32_t ch3_rx_buf;

    LOGT("ethernet0_rx_isr: Interrupt received");

    /* Step 29: Read current RX descriptor and buffer addresses for all channels */
    ch0_rx_desc = read_reg(DMA_CH0_Current_App_RxDesc);
    ch0_rx_buf = read_reg(DMA_CH0_Current_App_RxBuffer);
    LOGT("ethernet0_rx_isr: CH0 rx_desc=0x%08x rx_buf=0x%08x",
         (unsigned int)ch0_rx_desc, (unsigned int)ch0_rx_buf);

    ch1_rx_desc = read_reg(DMA_CH1_Current_App_RxDesc);
    ch1_rx_buf = read_reg(DMA_CH1_Current_App_RxBuffer);
    LOGT("ethernet0_rx_isr: CH1 rx_desc=0x%08x rx_buf=0x%08x",
         (unsigned int)ch1_rx_desc, (unsigned int)ch1_rx_buf);

    ch2_rx_desc = read_reg(DMA_CH2_Current_App_RxDesc);
    ch2_rx_buf = read_reg(DMA_CH2_Current_App_RxBuffer);
    LOGT("ethernet0_rx_isr: CH2 rx_desc=0x%08x rx_buf=0x%08x",
         (unsigned int)ch2_rx_desc, (unsigned int)ch2_rx_buf);

    ch3_rx_desc = read_reg(DMA_CH3_Current_App_RxDesc);
    ch3_rx_buf = read_reg(DMA_CH3_Current_App_RxBuffer);
    LOGT("ethernet0_rx_isr: CH3 rx_desc=0x%08x rx_buf=0x%08x",
         (unsigned int)ch3_rx_desc, (unsigned int)ch3_rx_buf);

    /* Read the DMA interrupt status */
    dma_status = read_reg(DMA_Interrupt_Status);
    LOGT("ethernet0_rx_isr: DMA_Interrupt_Status = 0x%08x", (unsigned int)dma_status);

    /* Clear all DMA channel status registers by writing all-ones */
    write_reg(DMA_CH0_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH1_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH2_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH3_Status, 0xFFFFFFFFU);
    LOGT("ethernet0_rx_isr: DMA channel status registers cleared");

    /* Clear the GIC interrupt */
    // MANUAL_REVIEW: Call the platform-specific GIC interrupt clear API here.
    // Example: gic_clear_interrupt(ETHERNET0_IRQ_NUM);

    g_ctx.transfer_complete_cnt++;
    LOGT("ethernet0_rx_isr: transfer_complete_cnt = %u", g_ctx.transfer_complete_cnt);
}

/*
 * Function: ethernet0_rx_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for ethernet0_rx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_rx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (eth0_rx_test_ctx_t){0};
    g_ctx.expected_transfers = ETH0_EXPECTED_TRANSFER_CNT;

    LOGT("ethernet0_rx_basic_test_init: Testcase initialization complete");
    LOGT("ethernet0_rx_basic_test_init: expected_transfers=%u", g_ctx.expected_transfers);

    /* Step 1: Enable GIC interrupts for Ethernet0 interrupt handling */
    // MANUAL_REVIEW: Call the platform-specific GIC enable API here.
    // Example: gic_enable_interrupt(ETHERNET0_IRQ_NUM, ethernet0_rx_isr);
    LOGT("ethernet0_rx_basic_test_init: GIC interrupts enabled (MANUAL_REVIEW)");

    /* Step 2: Configure the Ethernet speed and interface selection */
    // MANUAL_REVIEW: Speed and interface selection depends on compile-time defines
    // (ETH_10M, ETH_100M, or default 1G). Configure the appropriate clock/interface.
    // Use SEL_ENET0/SEL_ENET1/SEL_ENET2/SEL_ENET3 for interface selection.
    LOGT("ethernet0_rx_basic_test_init: Speed and interface configured (MANUAL_REVIEW)");

    return 0;
}

/*
 * Function: ethernet0_rx_basic_test_run
 * Description: Executes the main testcase flow for ethernet0_rx_basic_test.
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

    LOGT("ethernet0_rx_basic_test_run: Starting Ethernet0 RX basic test");

    /* Steps 3-8: Configure MAC layer for RX */
    configure_mac_rx();

    /* Steps 9-14: Configure MTL layer */
    configure_mtl_rx();

    /* Step 20: Preload RX descriptors for DMA channel 0 */
    preload_rx_descriptors();

    /* Steps 15-25: Configure DMA layer */
    configure_dma_rx();

    /* Step 26: Write the RX descriptor tail pointer to start the RX DMA engine */
    write_reg(DMA_CH0_RxDesc_Tail_Pointer, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_run: DMA_CH0_RxDesc_Tail_Pointer written (MANUAL_REVIEW: set tail ptr)");

    /* Enable RX start on DMA channel 0 */
    // MANUAL_REVIEW: Set the RX start bit in DMA_CH0_Rx_Control.
    // write_reg(DMA_CH0_Rx_Control, <value with SR bit set>);
    LOGT("ethernet0_rx_basic_test_run: RX start enabled (MANUAL_REVIEW)");

    /* Step 27: Trigger the external VIP sequencer to begin sending packets */
    // MANUAL_REVIEW: The external VIP sequencer trigger mechanism is DV/platform-specific.
    // This is a DV-specific construct with no PSV/FV-native equivalent.
    LOGT("ethernet0_rx_basic_test_run: VIP sequencer triggered (MANUAL_REVIEW)");

    /* Step 28: Wait for the expected number of transfer-complete interrupts */
    LOGT("ethernet0_rx_basic_test_run: Waiting for %u transfer-complete interrupts",
         g_ctx.expected_transfers);

    timeout = ETH0_POLL_TIMEOUT;
    while ((g_ctx.transfer_complete_cnt < g_ctx.expected_transfers) && (timeout > 0U)) {
        timeout--;
    }

    if (g_ctx.transfer_complete_cnt < g_ctx.expected_transfers) {
        LOGE("ethernet0_rx_basic_test_run: TIMEOUT waiting for interrupts: received=%u expected=%u",
             g_ctx.transfer_complete_cnt,
             g_ctx.expected_transfers);
        g_ctx.errors++;
    } else {
        LOGT("ethernet0_rx_basic_test_run: All %u transfer-complete interrupts received",
             g_ctx.transfer_complete_cnt);
    }

    /* Step 30: After all interrupts are received, wait for a settling period */
    // MANUAL_REVIEW: Insert platform-specific settling delay if required.
    LOGT("ethernet0_rx_basic_test_run: Settling period complete (MANUAL_REVIEW)");

    /* Determine final pass/fail status */
    if (g_ctx.errors > 0U) {
        LOGE("ethernet0_rx_basic_test_run: FAIL errors=%u transfer_complete_cnt=%u",
             g_ctx.errors,
             g_ctx.transfer_complete_cnt);
        out->status = -1;
    } else {
        LOGT("ethernet0_rx_basic_test_run: PASS all transfers completed successfully");
        out->status = 0;
    }

    LOGT("ethernet0_rx_basic_test_run: Run complete: %s errors=%u transfers=%u/%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.transfer_complete_cnt,
         g_ctx.expected_transfers);

    return out->status;
}

/*
 * Function: ethernet0_rx_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for ethernet0_rx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_rx_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_rx_basic_test_teardown: errors=%u transfer_complete_cnt=%u expected=%u",
         g_ctx.errors,
         g_ctx.transfer_complete_cnt,
         g_ctx.expected_transfers);

    // MANUAL_REVIEW: DV source used finish(0) for pass. PSV/FV uses out->status in _run.

    return (g_ctx.errors == 0U) ? 0 : -1;
}
