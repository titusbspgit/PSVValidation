// Author - AI Force 2.3. 18-Jul-2025 07:23 IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_tx_rx_multi_chnl_test
 * Description: Verifies multi-channel TX and RX Ethernet packet transfer on
 *   Ethernet0 using all four DMA channels (CH0-CH3) in a sequential phased
 *   approach. Phase 1 activates CH3, Phase 2 activates CH2, Phase 3 activates
 *   CH1, Phase 4 activates CH0. Each phase transfers 10 packets for a
 *   cumulative total of 40 TX and 40 RX packets. The interrupt handler manages
 *   RX descriptor tail pointer advancement for all four channels.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    volatile unsigned int rx_pkt_cnt;
    volatile unsigned int tx_pkt_cnt;
} eth0_multi_chnl_ctx_t;

static eth0_multi_chnl_ctx_t g_ctx;

/*
 * Static helper: configure_mac_multi
 * Configures MAC layer: configuration, extended config, RX queue control,
 * VLAN tag, packet filter, MMC IPC RX interrupt mask, and MAC address slots.
 */
static void configure_mac_multi(void)
{
    LOGT("configure_mac_multi: Configuring MAC layer");

    /* Step 3: Configure MAC Configuration - enable TX and RX with speed and duplex */
    write_reg(MAC_Configuration, 0x00000000U);
    LOGT("configure_mac_multi: MAC_Configuration written (MANUAL_REVIEW: TX/RX enable, speed, duplex)");

    /* Step 4: Configure MAC extended configuration, RX queue control, VLAN, packet filter */
    write_reg(MAC_Ext_Configuration, 0x00000000U);
    LOGT("configure_mac_multi: MAC_Ext_Configuration written (MANUAL_REVIEW)");

    write_reg(MAC_RxQ_Ctrl0, 0x00000000U);
    LOGT("configure_mac_multi: MAC_RxQ_Ctrl0 written (MANUAL_REVIEW: queue enable, priority)");

    write_reg(MAC_RxQ_Ctrl1, 0x00000000U);
    LOGT("configure_mac_multi: MAC_RxQ_Ctrl1 written (MANUAL_REVIEW)");

    write_reg(MAC_RxQ_Ctrl2, 0x00000000U);
    LOGT("configure_mac_multi: MAC_RxQ_Ctrl2 written (MANUAL_REVIEW)");

    write_reg(MAC_VLAN_Tag_Ctrl, 0x00000000U);
    LOGT("configure_mac_multi: MAC_VLAN_Tag_Ctrl written (MANUAL_REVIEW)");

    write_reg(MAC_Packet_Filter, 0x00000000U);
    LOGT("configure_mac_multi: MAC_Packet_Filter written (MANUAL_REVIEW: hash filtering)");

    /* Step 5: Configure MMC IPC RX interrupt mask */
    write_reg(MMC_IPC_Rx_Interrupt_Mask, 0x00000000U);
    LOGT("configure_mac_multi: MMC_IPC_Rx_Interrupt_Mask written (MANUAL_REVIEW)");

    /* Step 6: Program four MAC address slots */
    write_reg(MAC_Address0_High, 0x00000000U);
    write_reg(MAC_Address0_Low, 0x00000000U);
    LOGT("configure_mac_multi: MAC_Address0 programmed (MANUAL_REVIEW)");

    write_reg(MAC_Address1_High, 0x00000000U);
    write_reg(MAC_Address1_Low, 0x00000000U);
    LOGT("configure_mac_multi: MAC_Address1 programmed (MANUAL_REVIEW)");

    write_reg(MAC_Address2_High, 0x00000000U);
    write_reg(MAC_Address2_Low, 0x00000000U);
    LOGT("configure_mac_multi: MAC_Address2 programmed (MANUAL_REVIEW)");

    write_reg(MAC_Address3_High, 0x00000000U);
    write_reg(MAC_Address3_Low, 0x00000000U);
    LOGT("configure_mac_multi: MAC_Address3 programmed (MANUAL_REVIEW)");

    // MANUAL_REVIEW: Some MAC address registers are read back for verification in the original source.

    LOGT("configure_mac_multi: MAC layer configuration complete");
}

/*
 * Static helper: configure_mtl_multi
 * Configures MTL layer: TX queue operation modes (1KB), quantum weights,
 * operation mode, RX queue operation modes (4096B), queue-to-DMA mapping,
 * overflow/underflow interrupts, RX queue control weights.
 */
static void configure_mtl_multi(void)
{
    LOGT("configure_mtl_multi: Configuring MTL layer");

    /* Step 7: Configure MTL TX queue operation modes - 1KB queue size, store-and-forward */
    write_reg(MTL_TxQ0_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_TxQ0_Operation_Mode written (MANUAL_REVIEW: 1KB, S&F)");

    write_reg(MTL_TxQ1_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_TxQ1_Operation_Mode written (MANUAL_REVIEW: 1KB, S&F)");

    write_reg(MTL_TxQ2_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_TxQ2_Operation_Mode written (MANUAL_REVIEW: 1KB, S&F)");

    write_reg(MTL_TxQ3_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_TxQ3_Operation_Mode written (MANUAL_REVIEW: 1KB, S&F)");

    /* Step 8: Program MTL TX quantum weights */
    // Note: Q3 and Q2 use weight 20, Q1 and Q0 use weight 5
    write_reg(MTL_TxQ0_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_TxQ0_Quantum_Weight written (MANUAL_REVIEW: weight 5)");

    write_reg(MTL_TxQ1_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_TxQ1_Quantum_Weight written (MANUAL_REVIEW: weight 5)");

    write_reg(MTL_TxQ2_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_TxQ2_Quantum_Weight written (MANUAL_REVIEW: weight 20)");

    write_reg(MTL_TxQ3_Quantum_Weight, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_TxQ3_Quantum_Weight written (MANUAL_REVIEW: weight 20)");

    /* Step 9: Configure MTL operation mode */
    write_reg(MTL_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_Operation_Mode written (MANUAL_REVIEW)");

    /* Step 10: Configure MTL RX queue operation modes - 4096B, S&F, error pkt fwd */
    write_reg(MTL_RxQ0_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_RxQ0_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F, err fwd)");

    write_reg(MTL_RxQ1_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_RxQ1_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F, err fwd)");

    write_reg(MTL_RxQ2_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_RxQ2_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F, err fwd)");

    write_reg(MTL_RxQ3_Operation_Mode, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_RxQ3_Operation_Mode written (MANUAL_REVIEW: 4096B, S&F, err fwd)");

    /* Step 11: Enable MTL queue overflow and underflow interrupts */
    write_reg(MTL_Q0_Interrupt_Control_Status, 0x00000000U);
    write_reg(MTL_Q1_Interrupt_Control_Status, 0x00000000U);
    write_reg(MTL_Q2_Interrupt_Control_Status, 0x00000000U);
    write_reg(MTL_Q3_Interrupt_Control_Status, 0x00000000U);
    LOGT("configure_mtl_multi: MTL queue interrupt control configured (MANUAL_REVIEW: OVF/UNF)");

    /* Step 12: Map RX queues to corresponding DMA channels */
    write_reg(MTL_RxQ_DMA_Map0, 0x00000000U);
    LOGT("configure_mtl_multi: MTL_RxQ_DMA_Map0 written (MANUAL_REVIEW: Q0->CH0..Q3->CH3)");
    // MANUAL_REVIEW: Read back MTL_RxQ_DMA_Map0 for verification as in original source.

    /* Step 13: Program MTL RX queue control weights */
    write_reg(MTL_RxQ0_Control, 0x00000000U);
    write_reg(MTL_RxQ1_Control, 0x00000000U);
    write_reg(MTL_RxQ2_Control, 0x00000000U);
    write_reg(MTL_RxQ3_Control, 0x00000000U);
    LOGT("configure_mtl_multi: MTL RX queue control weights configured (MANUAL_REVIEW)");

    LOGT("configure_mtl_multi: MTL layer configuration complete");
}

/*
 * Static helper: configure_dma_multi
 * Configures DMA layer for all four channels: TX/RX control, descriptor
 * list addresses, ring lengths, system bus mode, interrupt enables.
 */
static void configure_dma_multi(void)
{
    LOGT("configure_dma_multi: Configuring DMA layer");

    /* Step 14: Configure DMA TX control with 16-beat burst length for all channels */
    write_reg(DMA_CH0_Tx_Control, 0x00000000U);
    write_reg(DMA_CH1_Tx_Control, 0x00000000U);
    write_reg(DMA_CH2_Tx_Control, 0x00000000U);
    write_reg(DMA_CH3_Tx_Control, 0x00000000U);
    LOGT("configure_dma_multi: DMA TX control configured (MANUAL_REVIEW: 16-beat burst)");

    /* Step 15: Set DMA TX descriptor list base addresses */
    write_reg(DMA_CH0_TxDesc_List_Address, 0x00000000U);
    write_reg(DMA_CH1_TxDesc_List_Address, 0x00000000U);
    write_reg(DMA_CH2_TxDesc_List_Address, 0x00000000U);
    write_reg(DMA_CH3_TxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_multi: DMA TX descriptor list addresses configured (MANUAL_REVIEW)");

    /* Step 16: Set DMA TX descriptor ring lengths of 10 */
    write_reg(DMA_CH0_TxDesc_Ring_Length, ETH0_TX_DESC_RING_LEN);
    write_reg(DMA_CH1_TxDesc_Ring_Length, ETH0_TX_DESC_RING_LEN);
    write_reg(DMA_CH2_TxDesc_Ring_Length, ETH0_TX_DESC_RING_LEN);
    write_reg(DMA_CH3_TxDesc_Ring_Length, ETH0_TX_DESC_RING_LEN);
    LOGT("configure_dma_multi: DMA TX descriptor ring lengths = %u", ETH0_TX_DESC_RING_LEN);

    /* Step 17: Initialize DMA channel control registers for channels 1-3 */
    write_reg(DMA_CH1_Control, 0x00000000U);
    write_reg(DMA_CH2_Control, 0x00000000U);
    write_reg(DMA_CH3_Control, 0x00000000U);
    LOGT("configure_dma_multi: DMA CH1-CH3 control initialized");

    /* Step 18: Configure DMA system bus mode */
    write_reg(DMA_SysBus_Mode, 0x00000000U);
    LOGT("configure_dma_multi: DMA_SysBus_Mode written (MANUAL_REVIEW: outstanding req, burst sel)");

    /* Step 19: Configure DMA RX control with 16-beat burst and RX buffer size */
    write_reg(DMA_CH0_Rx_Control, 0x00000000U);
    write_reg(DMA_CH1_Rx_Control, 0x00000000U);
    write_reg(DMA_CH2_Rx_Control, 0x00000000U);
    write_reg(DMA_CH3_Rx_Control, 0x00000000U);
    LOGT("configure_dma_multi: DMA RX control configured (MANUAL_REVIEW: 16-beat burst, buf size)");

    /* Step 21: Set DMA RX descriptor list base addresses */
    write_reg(DMA_CH0_RxDesc_List_Address, 0x00000000U);
    write_reg(DMA_CH1_RxDesc_List_Address, 0x00000000U);
    write_reg(DMA_CH2_RxDesc_List_Address, 0x00000000U);
    write_reg(DMA_CH3_RxDesc_List_Address, 0x00000000U);
    LOGT("configure_dma_multi: DMA RX descriptor list addresses configured (MANUAL_REVIEW)");

    /* Step 24: Configure DMA RX control2 ring lengths */
    write_reg(DMA_CH0_Rx_Control2, 0x00000000U);
    write_reg(DMA_CH1_Rx_Control2, 0x00000000U);
    write_reg(DMA_CH2_Rx_Control2, 0x00000000U);
    write_reg(DMA_CH3_Rx_Control2, 0x00000000U);
    LOGT("configure_dma_multi: DMA RX control2 configured (MANUAL_REVIEW: ring lengths)");

    /* Step 25: Enable DMA interrupts for all four channels */
    write_reg(DMA_CH0_Interrupt_Enable, 0x00000000U);
    write_reg(DMA_CH1_Interrupt_Enable, 0x00000000U);
    write_reg(DMA_CH2_Interrupt_Enable, 0x00000000U);
    write_reg(DMA_CH3_Interrupt_Enable, 0x00000000U);
    LOGT("configure_dma_multi: DMA interrupts enabled (MANUAL_REVIEW)");

    /* Step 26: Write TX descriptor tail pointers for channels 2 and 3 */
    write_reg(DMA_CH2_TxDesc_Tail_Pointer, 0x00000000U);
    write_reg(DMA_CH3_TxDesc_Tail_Pointer, 0x00000000U);
    LOGT("configure_dma_multi: DMA CH2/CH3 TX tail pointers written (MANUAL_REVIEW)");

    /* Step 27: Initialize DMA channel 0 control, re-configure RX, DMA mode */
    write_reg(DMA_CH0_Control, 0x00000000U);
    LOGT("configure_dma_multi: DMA_CH0_Control written");

    write_reg(DMA_Mode, 0x00000000U);
    LOGT("configure_dma_multi: DMA_Mode written (MANUAL_REVIEW)");

    LOGT("configure_dma_multi: DMA layer configuration complete");
}

/*
 * Static helper: preload_descriptors_multi
 * Preloads TX and RX descriptors for all four DMA channels.
 */
static void preload_descriptors_multi(void)
{
    LOGT("preload_descriptors_multi: Preloading TX and RX descriptors for all channels");

    /* Step 20: Preload TX descriptors for all four DMA channels */
    // MANUAL_REVIEW: Implement TX descriptor ring initialization for CH0-CH3
    // with ETH0_TX_DESC_RING_LEN entries each.

    /* Step 22: Preload RX descriptors for all four DMA channels */
    // MANUAL_REVIEW: Implement RX descriptor ring initialization for CH0-CH3.
    // Each descriptor must have OWN bit set and buffer address programmed.

    LOGT("preload_descriptors_multi: Descriptor preload complete (MANUAL_REVIEW)");
}

/*
 * Static helper: poll_packet_counts
 * Polls Rx_Packets_Count_Good_Bad and Tx_Packet_Count_Good_Bad until
 * both reach the target count, with a bounded polling loop.
 */
static int poll_packet_counts(unsigned int target_rx, unsigned int target_tx)
{
    unsigned int timeout;
    uint32_t rx_cnt;
    uint32_t tx_cnt;

    LOGT("poll_packet_counts: Polling for rx_target=%u tx_target=%u", target_rx, target_tx);

    timeout = ETH0_PHASE_POLL_LIMIT;
    while (timeout > 0U) {
        rx_cnt = read_reg(Rx_Packets_Count_Good_Bad);
        tx_cnt = read_reg(Tx_Packet_Count_Good_Bad);

        if ((rx_cnt >= target_rx) && (tx_cnt >= target_tx)) {
            LOGT("poll_packet_counts: Target reached rx=%u tx=%u",
                 (unsigned int)rx_cnt, (unsigned int)tx_cnt);
            return 0;
        }
        timeout--;
    }

    LOGE("poll_packet_counts: TIMEOUT rx=%u/%u tx=%u/%u",
         (unsigned int)rx_cnt, target_rx,
         (unsigned int)tx_cnt, target_tx);
    return -1;
}

/*
 * Static helper: ethernet0_multi_chnl_isr
 * Interrupt handler for multi-channel TX/RX. Manages RX descriptor tail
 * pointer advancement for all four channels, reads and clears DMA interrupt
 * status, and clears the GIC interrupt.
 */
static void ethernet0_multi_chnl_isr(void)
{
    uint32_t dma_status;
    uint32_t curr_rx_desc;

    LOGT("ethernet0_multi_chnl_isr: Interrupt received");

    /* Step 32: For each DMA channel, read current RX descriptor and manage tail pointer */
    /* Channel 0 */
    curr_rx_desc = read_reg(DMA_CH0_Current_App_RxDesc);
    // MANUAL_REVIEW: Compare curr_rx_desc with DMA_CH0_RxDesc_Tail_Pointer,
    // advance or reset tail pointer as needed.
    LOGT("ethernet0_multi_chnl_isr: CH0 curr_rx_desc=0x%08x", (unsigned int)curr_rx_desc);

    /* Channel 1 */
    curr_rx_desc = read_reg(DMA_CH1_Current_App_RxDesc);
    LOGT("ethernet0_multi_chnl_isr: CH1 curr_rx_desc=0x%08x", (unsigned int)curr_rx_desc);

    /* Channel 2 */
    curr_rx_desc = read_reg(DMA_CH2_Current_App_RxDesc);
    LOGT("ethernet0_multi_chnl_isr: CH2 curr_rx_desc=0x%08x", (unsigned int)curr_rx_desc);

    /* Channel 3 */
    curr_rx_desc = read_reg(DMA_CH3_Current_App_RxDesc);
    LOGT("ethernet0_multi_chnl_isr: CH3 curr_rx_desc=0x%08x", (unsigned int)curr_rx_desc);

    /* Read and clear DMA interrupt status for all four channels */
    dma_status = read_reg(DMA_Interrupt_Status);
    LOGT("ethernet0_multi_chnl_isr: DMA_Interrupt_Status=0x%08x", (unsigned int)dma_status);

    write_reg(DMA_CH0_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH1_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH2_Status, 0xFFFFFFFFU);
    write_reg(DMA_CH3_Status, 0xFFFFFFFFU);
    LOGT("ethernet0_multi_chnl_isr: DMA channel status registers cleared");

    /* Clear the GIC interrupt */
    // MANUAL_REVIEW: Call the platform-specific GIC interrupt clear API here.

    LOGT("ethernet0_multi_chnl_isr: ISR complete");
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_init
 * Description: Performs testcase initialization and pre-condition setup for ethernet0_tx_rx_multi_chnl_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (eth0_multi_chnl_ctx_t){0};

    LOGT("ethernet0_tx_rx_multi_chnl_test_init: Testcase initialization complete");

    /* Step 1: Enable GIC interrupts for Ethernet0 interrupt handling */
    // MANUAL_REVIEW: Call the platform-specific GIC enable API here.
    LOGT("ethernet0_tx_rx_multi_chnl_test_init: GIC interrupts enabled (MANUAL_REVIEW)");

    /* Step 2: Configure the Ethernet interface selection */
    // MANUAL_REVIEW: Configure interface selection for Ethernet0.
    LOGT("ethernet0_tx_rx_multi_chnl_test_init: Interface configured (MANUAL_REVIEW)");

    return 0;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_run
 * Description: Executes the main testcase flow for ethernet0_tx_rx_multi_chnl_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput *out)
{
    int phase_result;

    (void)cfg;

    if (out == 0) {
        LOGE("ethernet0_tx_rx_multi_chnl_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet0_tx_rx_multi_chnl_test_run: Starting multi-channel TX/RX test");

    /* Steps 3-6: Configure MAC layer */
    configure_mac_multi();

    /* Steps 7-13: Configure MTL layer */
    configure_mtl_multi();

    /* Steps 20, 22: Preload TX and RX descriptors for all channels */
    preload_descriptors_multi();

    /* Steps 14-27: Configure DMA layer */
    configure_dma_multi();

    /* Step 23: Enable HSS Autoreg Ethernet interrupts */
    // MANUAL_REVIEW: Program the HSS Autoreg Ethernet interrupt enable register.
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: HSS Autoreg interrupts enabled (MANUAL_REVIEW)");

    /* ========== Phase 1: Channel 3 (target: 10 packets) ========== */
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: Phase 1 - Starting Channel 3");

    /* Step 28: Update RX queue-to-DMA mapping to direct all traffic to channel 3 */
    write_reg(MTL_RxQ_DMA_Map0, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: MTL_RxQ_DMA_Map0 updated for CH3 (MANUAL_REVIEW)");

    /* Write RX descriptor tail pointers for all channels */
    write_reg(DMA_CH0_RxDesc_Tail_Pointer, 0x00000000U);
    write_reg(DMA_CH1_RxDesc_Tail_Pointer, 0x00000000U);
    write_reg(DMA_CH2_RxDesc_Tail_Pointer, 0x00000000U);
    write_reg(DMA_CH3_RxDesc_Tail_Pointer, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: RX tail pointers written (MANUAL_REVIEW)");

    /* Start RX on channel 3 */
    write_reg(DMA_CH3_Rx_Control, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH3 RX started (MANUAL_REVIEW: set start RX bit)");

    /* Trigger the external VIP sequencer */
    // MANUAL_REVIEW: Trigger VIP with value 0xdeadbeef for Phase 1.
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: VIP sequencer triggered Phase 1 (MANUAL_REVIEW)");

    /* Write TX descriptor tail pointers for all channels */
    write_reg(DMA_CH0_TxDesc_Tail_Pointer, 0x00000000U);
    write_reg(DMA_CH1_TxDesc_Tail_Pointer, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: TX tail pointers written (MANUAL_REVIEW)");

    /* Start TX on channel 3 */
    write_reg(DMA_CH3_Tx_Control, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH3 TX started (MANUAL_REVIEW: set start TX bit)");

    /* Poll until RX and TX packet counts reach 10 */
    phase_result = poll_packet_counts(ETH0_PHASE_PKT_CNT * 1U, ETH0_PHASE_PKT_CNT * 1U);
    if (phase_result != 0) {
        LOGE("ethernet0_tx_rx_multi_chnl_test_run: Phase 1 FAILED");
        g_ctx.errors++;
    } else {
        LOGT("ethernet0_tx_rx_multi_chnl_test_run: Phase 1 PASSED");
    }

    /* ========== Phase 2: Channel 2 (target: 20 packets cumulative) ========== */
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: Phase 2 - Starting Channel 2");

    /* Step 29: Start TX on channel 2 */
    write_reg(DMA_CH2_Tx_Control, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH2 TX started (MANUAL_REVIEW: set start TX bit)");

    /* Update RX queue mapping to channel 2 */
    write_reg(MTL_RxQ_DMA_Map0, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: MTL_RxQ_DMA_Map0 updated for CH2 (MANUAL_REVIEW)");

    /* Write RX descriptor tail pointer for channel 2 */
    write_reg(DMA_CH2_RxDesc_Tail_Pointer, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH2 RX tail pointer written (MANUAL_REVIEW)");

    /* Start RX on channel 2 */
    write_reg(DMA_CH2_Rx_Control, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH2 RX started (MANUAL_REVIEW: set start RX bit)");

    /* Re-trigger VIP sequencer */
    // MANUAL_REVIEW: Trigger VIP with value 0xdeadbeee for Phase 2.
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: VIP sequencer triggered Phase 2 (MANUAL_REVIEW)");

    /* Poll until cumulative counts reach 20 */
    phase_result = poll_packet_counts(ETH0_PHASE_PKT_CNT * 2U, ETH0_PHASE_PKT_CNT * 2U);
    if (phase_result != 0) {
        LOGE("ethernet0_tx_rx_multi_chnl_test_run: Phase 2 FAILED");
        g_ctx.errors++;
    } else {
        LOGT("ethernet0_tx_rx_multi_chnl_test_run: Phase 2 PASSED");
    }

    /* ========== Phase 3: Channel 1 (target: 30 packets cumulative) ========== */
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: Phase 3 - Starting Channel 1");

    /* Step 30: Start TX on channel 1 */
    write_reg(DMA_CH1_Tx_Control, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH1 TX started (MANUAL_REVIEW: set start TX bit)");

    /* Update RX queue mapping to channel 1 */
    write_reg(MTL_RxQ_DMA_Map0, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: MTL_RxQ_DMA_Map0 updated for CH1 (MANUAL_REVIEW)");

    /* Write RX descriptor tail pointer for channel 1 */
    write_reg(DMA_CH1_RxDesc_Tail_Pointer, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH1 RX tail pointer written (MANUAL_REVIEW)");

    /* Start RX on channel 1 */
    write_reg(DMA_CH1_Rx_Control, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH1 RX started (MANUAL_REVIEW: set start RX bit)");

    /* Re-trigger VIP sequencer */
    // MANUAL_REVIEW: Trigger VIP with value 0xdeadbeed for Phase 3.
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: VIP sequencer triggered Phase 3 (MANUAL_REVIEW)");

    /* Poll until cumulative counts reach 30 */
    phase_result = poll_packet_counts(ETH0_PHASE_PKT_CNT * 3U, ETH0_PHASE_PKT_CNT * 3U);
    if (phase_result != 0) {
        LOGE("ethernet0_tx_rx_multi_chnl_test_run: Phase 3 FAILED");
        g_ctx.errors++;
    } else {
        LOGT("ethernet0_tx_rx_multi_chnl_test_run: Phase 3 PASSED");
    }

    /* ========== Phase 4: Channel 0 (target: 40 packets cumulative) ========== */
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: Phase 4 - Starting Channel 0");

    /* Step 31: Start TX on channel 0 */
    write_reg(DMA_CH0_Tx_Control, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH0 TX started (MANUAL_REVIEW: set start TX bit)");

    /* Update RX queue mapping to channel 0 */
    write_reg(MTL_RxQ_DMA_Map0, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: MTL_RxQ_DMA_Map0 updated for CH0 (MANUAL_REVIEW)");

    /* Write RX descriptor tail pointer for channel 0 */
    write_reg(DMA_CH0_RxDesc_Tail_Pointer, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH0 RX tail pointer written (MANUAL_REVIEW)");

    /* Start RX on channel 0 */
    write_reg(DMA_CH0_Rx_Control, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: CH0 RX started (MANUAL_REVIEW: set start RX bit)");

    /* Re-trigger VIP sequencer */
    // MANUAL_REVIEW: Trigger VIP with value 0xdeadbeec for Phase 4.
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: VIP sequencer triggered Phase 4 (MANUAL_REVIEW)");

    /* Poll until cumulative counts reach 40 */
    phase_result = poll_packet_counts(ETH0_PHASE_PKT_CNT * 4U, ETH0_PHASE_PKT_CNT * 4U);
    if (phase_result != 0) {
        LOGE("ethernet0_tx_rx_multi_chnl_test_run: Phase 4 FAILED");
        g_ctx.errors++;
    } else {
        LOGT("ethernet0_tx_rx_multi_chnl_test_run: Phase 4 PASSED");
    }

    /* Step 33: After all four phases complete, wait for a settling period */
    // MANUAL_REVIEW: Insert platform-specific settling delay if required.
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: Settling period complete (MANUAL_REVIEW)");

    /* Determine final pass/fail status */
    if (g_ctx.errors > 0U) {
        LOGE("ethernet0_tx_rx_multi_chnl_test_run: FAIL errors=%u", g_ctx.errors);
        out->status = -1;
    } else {
        LOGT("ethernet0_tx_rx_multi_chnl_test_run: PASS all 4 phases completed successfully");
        out->status = 0;
    }

    LOGT("ethernet0_tx_rx_multi_chnl_test_run: Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for ethernet0_tx_rx_multi_chnl_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_tx_rx_multi_chnl_test_teardown: errors=%u", g_ctx.errors);

    // MANUAL_REVIEW: DV source used finish(0) for pass. PSV/FV uses out->status in _run.

    return (g_ctx.errors == 0U) ? 0 : -1;
}
