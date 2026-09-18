// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_tx_rx_multi_chnl_test
 * Description: Multi-channel Ethernet TX and RX data path validation using all
 *              four DMA channels (CH0-CH3) sequentially. Operates in four phases:
 *              Phase 1 starts CH3, Phase 2 starts CH2, Phase 3 starts CH1,
 *              Phase 4 starts CH0, with dynamic RXQ_DMA_MAP0 remapping and
 *              polling of packet count registers until cumulative counts reach
 *              10, 20, 30, 40 respectively.
 */

typedef struct {
    unsigned int errors;
    unsigned int phase;
} ethernet0_tx_rx_multi_chnl_test_ctx_t;

static ethernet0_tx_rx_multi_chnl_test_ctx_t g_ctx;

static void ethernet0_tx_rx_multi_chnl_test_irq_handler(void)
{
    unsigned int dma_int_status;
    unsigned int rd_val;

    LOGT("IRQ handler: entered");
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
    LOGT("IRQ: CH0 current app RX desc=0x%08x", rd_val);
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
    LOGT("IRQ: CH1 current app RX desc=0x%08x", rd_val);
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
    LOGT("IRQ: CH2 current app RX desc=0x%08x", rd_val);
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
    LOGT("IRQ: CH3 current app RX desc=0x%08x", rd_val);
    writel_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0x00000000U);
    LOGT("IRQ: RX tail pointers advanced");
    dma_int_status = readl_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
    LOGT("IRQ: DMA_INTERRUPT_STATUS=0x%08x", dma_int_status);
    writel_reg(mizar_ETHERNET0_DMA_CH0_STATUS, 0xFFFFFFFFU);
    writel_reg(mizar_ETHERNET0_DMA_CH1_STATUS, 0xFFFFFFFFU);
    writel_reg(mizar_ETHERNET0_DMA_CH2_STATUS, 0xFFFFFFFFU);
    writel_reg(mizar_ETHERNET0_DMA_CH3_STATUS, 0xFFFFFFFFU);
    dma_int_status = readl_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
    LOGT("IRQ: DMA_INTERRUPT_STATUS after clear=0x%08x", dma_int_status);
    LOGT("IRQ handler: exit");
}

int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem *cfg)
{
    (void)cfg;
    g_ctx = (ethernet0_tx_rx_multi_chnl_test_ctx_t){0};
    LOGT("ethernet0_tx_rx_multi_chnl_test_init: starting initialization");
    GIC_Set();
    GIC_EnableAllIRQ();
    LOGT("ethernet0_tx_rx_multi_chnl_test_init: GIC initialized");
#ifdef ETH_10M
    enet_10m_speed();
#elif defined(ETH_100M)
    enet_100m_speed();
#endif
#if defined(SEL_ENET0)
    enet_intf_sel(0);
#elif defined(SEL_ENET1)
    enet_intf_sel(1);
#elif defined(SEL_ENET2)
    enet_intf_sel(2);
#elif defined(SEL_ENET3)
    enet_intf_sel(3);
#endif
    writel_reg(0xA0243ffcU, 0x00000000U);
#ifdef HALF_DUPLEX
    writel_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x00000000U);
#else
    writel_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x00000000U);
#endif
    writel_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MMC_IPC_RX_INTERRUPT_MASK, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ0_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ1_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ2_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ3_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(0XE68C2058U, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_MODE, 0x00000000U);
    LOGT("ethernet0_tx_rx_multi_chnl_test_init: initialization complete");
    return 0;
}

static int ethernet0_tx_rx_multi_chnl_test_poll_phase(unsigned int target_rxpkt, unsigned int target_txpkt)
{
    unsigned int rxpkt;
    unsigned int txpkt;
    unsigned int timeout;
    timeout = ETHERNET0_MULTI_CHNL_TIMEOUT;
    do {
        rxpkt = readl_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
        txpkt = readl_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
        if ((rxpkt >= target_rxpkt) && (txpkt >= target_txpkt)) {
            LOGT("Poll phase complete: rxpkt=%u txpkt=%u", rxpkt, txpkt);
            return 0;
        }
        timeout--;
    } while (timeout > 0U);
    LOGE("Poll phase TIMEOUT: rxpkt=%u txpkt=%u target_rx=%u target_tx=%u", rxpkt, txpkt, target_rxpkt, target_txpkt);
    return -1;
}

int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    if (out == 0) { LOGE("output pointer is NULL"); return -1; }
    out->status = 0;
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: starting multi-channel TX/RX test");
    g_ctx.phase = 1U;
    writel_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00000001U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00000001U);
    writel_reg(0xA0243ff8U, 0x00000000U);
    if (ethernet0_tx_rx_multi_chnl_test_poll_phase(10U, 10U) != 0) { g_ctx.errors++; out->status = -1; return out->status; }
    LOGT("Phase 1: PASS");
    g_ctx.phase = 2U;
    writel_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00000001U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00000001U);
    writel_reg(0xA0243ff8U, 0x00000000U);
    if (ethernet0_tx_rx_multi_chnl_test_poll_phase(20U, 20U) != 0) { g_ctx.errors++; out->status = -1; return out->status; }
    LOGT("Phase 2: PASS");
    g_ctx.phase = 3U;
    writel_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00000001U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00000001U);
    writel_reg(0xA0243ff8U, 0x00000000U);
    if (ethernet0_tx_rx_multi_chnl_test_poll_phase(30U, 30U) != 0) { g_ctx.errors++; out->status = -1; return out->status; }
    LOGT("Phase 3: PASS");
    g_ctx.phase = 4U;
    writel_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00000001U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00000001U);
    writel_reg(0xA0243ff8U, 0x00000000U);
    if (ethernet0_tx_rx_multi_chnl_test_poll_phase(40U, 40U) != 0) { g_ctx.errors++; out->status = -1; return out->status; }
    LOGT("Phase 4: PASS");
    if (g_ctx.errors > 0U) { out->status = -1; } else { out->status = 0; }
    LOGT("ethernet0_tx_rx_multi_chnl_test_run: complete, errors=%u", g_ctx.errors);
    return out->status;
}

int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("ethernet0_tx_rx_multi_chnl_test_teardown: phases=%u errors=%u", g_ctx.phase, g_ctx.errors);
    return (g_ctx.errors == 0U) ? 0 : -1;
}
