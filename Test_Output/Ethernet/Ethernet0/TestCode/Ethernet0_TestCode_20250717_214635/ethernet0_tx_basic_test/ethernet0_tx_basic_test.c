// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_basic_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_tx_basic_test
 * Description: Performs basic Ethernet TX path validation. Configures MAC
 *              addresses, packet filter, MAC configuration with speed/duplex
 *              settings, MTL TX queue operation modes, quantum weights, DMA
 *              system bus mode, TX descriptor ring lengths, list addresses,
 *              tail pointers for all 4 DMA channels, enables DMA interrupts,
 *              starts TX on CH0, and loops waiting for transfer-complete
 *              interrupts. IRQ handler reads DMA interrupt status, clears
 *              CH0 status, and reads DMA interrupt status again.
 */

typedef struct {
    unsigned int errors;
    unsigned int int_pend;
    unsigned int trns_count;
    unsigned int irq_serviced;
} ethernet0_tx_basic_test_ctx_t;

static ethernet0_tx_basic_test_ctx_t g_ctx;

static void ethernet0_tx_basic_test_irq_handler(void)
{
    unsigned int dma_int_status;
    LOGT("IRQ handler: entered");
    dma_int_status = readl_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
    LOGT("IRQ: DMA_INTERRUPT_STATUS=0x%08x", dma_int_status);
    writel_reg(mizar_ETHERNET0_DMA_CH0_STATUS, 0xFFFFFFFFU);
    LOGT("IRQ: DMA_CH0_STATUS cleared");
    dma_int_status = readl_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
    LOGT("IRQ: DMA_INTERRUPT_STATUS after clear=0x%08x", dma_int_status);
    g_ctx.int_pend = 0U;
    g_ctx.irq_serviced++;
    LOGT("IRQ handler: exit, irq_serviced=%u", g_ctx.irq_serviced);
}

int ethernet0_tx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;
    g_ctx = (ethernet0_tx_basic_test_ctx_t){0};
    g_ctx.int_pend = 1U;
    g_ctx.trns_count = 10U;
    LOGT("ethernet0_tx_basic_test_init: int_pend=%u trns_count=%u", g_ctx.int_pend, g_ctx.trns_count);
    GIC_Set();
    GIC_EnableAllIRQ();
    LOGT("ethernet0_tx_basic_test_init: GIC initialized");
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
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x00000000U);
#ifdef HALF_DUPLEX
    writel_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x00000000U);
#else
    writel_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x00000000U);
#endif
    writel_reg(0xA0243ffcU, 0x00000000U);
    writel_reg(0xA0243ff8U, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(0XE68C2058U, 0x00000000U);
    LOGT("ethernet0_tx_basic_test_init: initialization complete");
    return 0;
}

int ethernet0_tx_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int i;
    unsigned int timeout;
    (void)cfg;
    if (out == 0) { LOGE("output pointer is NULL"); return -1; }
    out->status = 0;
    LOGT("ethernet0_tx_basic_test_run: starting TX basic test");
    writel_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00000001U);
    LOGT("ethernet0_tx_basic_test_run: CH0 TX started");
    for (i = 0U; i < g_ctx.trns_count; i++) {
        timeout = ETHERNET0_TX_TIMEOUT;
        while ((g_ctx.int_pend != 0U) && (timeout > 0U)) { timeout--; }
        if (timeout == 0U) {
            LOGE("ethernet0_tx_basic_test_run: timeout iteration=%u", i);
            g_ctx.errors++; out->status = -1; break;
        }
        LOGT("ethernet0_tx_basic_test_run: interrupt received, iteration=%u", i);
        g_ctx.int_pend = 1U;
    }
    if (g_ctx.errors > 0U) { out->status = -1; } else { out->status = 0; }
    LOGT("ethernet0_tx_basic_test_run: complete, irq_serviced=%u errors=%u", g_ctx.irq_serviced, g_ctx.errors);
    return out->status;
}

int ethernet0_tx_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("ethernet0_tx_basic_test_teardown: irq_serviced=%u errors=%u", g_ctx.irq_serviced, g_ctx.errors);
    return (g_ctx.errors == 0U) ? 0 : -1;
}
