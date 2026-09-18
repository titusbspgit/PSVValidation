// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_rx_basic_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_rx_basic_test
 * Description: Performs basic Ethernet RX path validation. Configures GIC,
 *              speed/interface selection, MAC/MTL/DMA registers, preloads RX
 *              descriptors, starts RX on CH0, loops waiting for transfer-complete
 *              interrupts, and validates via IRQ handler DMA status clearing.
 */

typedef struct {
    unsigned int errors;
    unsigned int int_pend;
    unsigned int trns_count;
    unsigned int irq_serviced;
} ethernet0_rx_basic_test_ctx_t;

static ethernet0_rx_basic_test_ctx_t g_ctx;

/*
 * Function: ethernet0_rx_basic_test_irq_handler
 * Description: IRQ handler for Ethernet0 RX basic test. Reads DMA current app
 *              RX descriptor and buffer registers for CH0-CH3, reads DMA
 *              interrupt status, clears DMA channel status registers.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void ethernet0_rx_basic_test_irq_handler(void)
{
    unsigned int dma_int_status;
    unsigned int ch0_status;
    unsigned int ch1_status;
    unsigned int ch2_status;
    unsigned int ch3_status;
    unsigned int rd_val;

    LOGT("IRQ handler: entered");

    /* Step 35: Read DMA current app RX descriptor and buffer for CH0 */
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
    LOGT("IRQ: CH0 current app RX desc=0x%08x", rd_val);
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXBUFFER);
    LOGT("IRQ: CH0 current app RX buffer=0x%08x", rd_val);

    /* Step 36: Read DMA current app RX descriptor and buffer for CH1 */
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
    LOGT("IRQ: CH1 current app RX desc=0x%08x", rd_val);
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXBUFFER);
    LOGT("IRQ: CH1 current app RX buffer=0x%08x", rd_val);

    /* Step 37: Read DMA current app RX descriptor and buffer for CH2 */
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
    LOGT("IRQ: CH2 current app RX desc=0x%08x", rd_val);
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXBUFFER);
    LOGT("IRQ: CH2 current app RX buffer=0x%08x", rd_val);

    /* Step 38: Read DMA current app RX descriptor and buffer for CH3 */
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
    LOGT("IRQ: CH3 current app RX desc=0x%08x", rd_val);
    rd_val = readl_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXBUFFER);
    LOGT("IRQ: CH3 current app RX buffer=0x%08x", rd_val);

    /* Step 39: Read DMA interrupt status */
    dma_int_status = readl_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
    LOGT("IRQ: DMA_INTERRUPT_STATUS=0x%08x", dma_int_status);

    /* Step 40: Clear DMA channel status registers for CH0-CH3 */
    writel_reg(mizar_ETHERNET0_DMA_CH0_STATUS, 0xFFFFFFFFU);
    writel_reg(mizar_ETHERNET0_DMA_CH1_STATUS, 0xFFFFFFFFU);
    writel_reg(mizar_ETHERNET0_DMA_CH2_STATUS, 0xFFFFFFFFU);
    writel_reg(mizar_ETHERNET0_DMA_CH3_STATUS, 0xFFFFFFFFU);

    /* Step 41: Read DMA interrupt status again after clearing */
    dma_int_status = readl_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
    LOGT("IRQ: DMA_INTERRUPT_STATUS after clear=0x%08x", dma_int_status);

    /* Signal transfer complete */
    g_ctx.int_pend = 0U;
    g_ctx.irq_serviced++;

    LOGT("IRQ handler: exit, irq_serviced=%u", g_ctx.irq_serviced);
}

/*
 * Function: ethernet0_rx_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              ethernet0_rx_basic_test. Initializes GIC, selects speed and
 *              interface, configures all MAC, MTL, and DMA registers.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_rx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (ethernet0_rx_basic_test_ctx_t){0};

    /* Step 1: Set int_pend=1, initialize trns_count */
    g_ctx.int_pend = 1U;

#ifdef ETH_10M
    g_ctx.trns_count = 6U;
#else
    g_ctx.trns_count = 10U;
#endif

    LOGT("ethernet0_rx_basic_test_init: int_pend=%u trns_count=%u",
         g_ctx.int_pend, g_ctx.trns_count);

    /* Step 2: Initialize GIC */
    GIC_Set();
    GIC_EnableAllIRQ();
    LOGT("ethernet0_rx_basic_test_init: GIC initialized");

    /* Step 3: Select speed */
#ifdef ETH_10M
    enet_10m_speed();
    LOGT("ethernet0_rx_basic_test_init: speed set to 10M");
#elif defined(ETH_100M)
    enet_100m_speed();
    LOGT("ethernet0_rx_basic_test_init: speed set to 100M");
#endif

    /* Step 4: Select interface */
#if defined(SEL_ENET0)
    enet_intf_sel(0);
#elif defined(SEL_ENET1)
    enet_intf_sel(1);
#elif defined(SEL_ENET2)
    enet_intf_sel(2);
#elif defined(SEL_ENET3)
    enet_intf_sel(3);
#endif
    LOGT("ethernet0_rx_basic_test_init: interface selected");

    /* Step 5: Configure MAC_CONFIGURATION with speed and duplex mode */
    // MANUAL_REVIEW: The exact value for MAC_CONFIGURATION depends on speed and
    // duplex mode conditional compilation flags. Populate with correct bitfield values.
#ifdef HALF_DUPLEX
    // MANUAL_REVIEW: Set half-duplex mode bits in MAC_CONFIGURATION.
    writel_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x00000000U);
#else
    // MANUAL_REVIEW: Set full-duplex mode bits in MAC_CONFIGURATION.
    writel_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x00000000U);
#endif
    LOGT("ethernet0_rx_basic_test_init: MAC_CONFIGURATION written");

    /* Step 6: Configure MAC RX queue control registers */
    writel_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_RXQ_CTRL4, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MAC RXQ control registers configured");

    /* Step 7: Configure VLAN tag control */
    writel_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: VLAN tag control configured");

    /* Step 8: Configure packet filter */
    writel_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: packet filter configured");

    /* Step 9: Configure MAC addresses */
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MAC addresses configured");

    /* Step 10: Configure MAC_EXT_CONFIGURATION */
    writel_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MAC_EXT_CONFIGURATION configured");

    /* Step 11: Configure MTL TX queue operation modes */
    writel_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MTL TX queue operation modes configured");

    /* Step 12: Configure MTL TX queue quantum weights */
    writel_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MTL TX queue quantum weights configured");

    /* Step 13: Configure MTL operation mode */
    writel_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MTL operation mode configured");

    /* Step 14: Configure MTL RX queue operation modes */
    writel_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MTL RX queue operation modes configured");

    /* Step 15: Configure RXQ_DMA_MAP0 */
    writel_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: RXQ_DMA_MAP0 configured");

    /* Step 16: Configure MTL queue interrupt control status */
    writel_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MTL queue interrupt control configured");

    /* Step 17: Configure MTL RX queue controls */
    writel_reg(mizar_ETHERNET0_MTL_RXQ0_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ1_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ2_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_MTL_RXQ3_CONTROL, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MTL RX queue controls configured");

    /* Step 18: Configure DMA TX controls */
    writel_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA TX controls configured");

    /* Step 19: Configure DMA TX descriptor list addresses */
    writel_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA TX descriptor list addresses configured");

    /* Step 20: Configure DMA TX descriptor ring lengths */
    writel_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA TX descriptor ring lengths configured");

    /* Step 21: Configure DMA channel controls */
    writel_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA channel controls configured");

    /* Step 22: Configure DMA RX controls */
    writel_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA RX controls configured");

    /* Step 23: Configure DMA system bus mode */
    writel_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA system bus mode configured");

    /* Step 24: Configure DMA RX descriptor list addresses */
    writel_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA RX descriptor list addresses configured");

    /* Step 25: Configure DMA RX control2 registers */
    writel_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL2, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL2, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA RX control2 registers configured");

    /* Step 26: Configure DMA interrupt enables */
    writel_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0x00000000U);
    writel_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA interrupt enables configured");

    /* Step 27: Configure MAC interrupt enable */
    writel_reg(mizar_ETHERNET0_MAC_INTERRUPT_ENABLE, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: MAC interrupt enable configured");

    /* Step 28: Configure DMA mode */
    writel_reg(mizar_ETHERNET0_DMA_MODE, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: DMA mode configured");

    /* Step 29: Configure VIP sequencer trigger register */
    writel_reg(0XE68C2058U, 0x00000000U);
    LOGT("ethernet0_rx_basic_test_init: VIP sequencer trigger register configured");

    // MANUAL_REVIEW: All register values above are placeholders (0x00000000U).
    // Populate with correct values from the Ethernet0 programming sequence
    // and IP-specific configuration headers.

    LOGT("ethernet0_rx_basic_test_init: initialization complete");

    return 0;
}

/*
 * Function: ethernet0_rx_basic_test_run
 * Description: Executes the main testcase flow for ethernet0_rx_basic_test.
 *              Preloads RX descriptors, starts RX by writing RX descriptor tail
 *              pointer and enabling CH0 RX, triggers VIP sequencer, then loops
 *              waiting for transfer-complete interrupts.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_rx_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int i;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("ethernet0_rx_basic_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet0_rx_basic_test_run: starting RX basic test");

    /* Step 30: Preload RX descriptors */
    // MANUAL_REVIEW: RX descriptor preload logic depends on descriptor memory
    // layout and buffer addresses. Implement using ethernet0_funcs.h or
    // ethernet0_programming_sequence.h APIs.
    LOGT("ethernet0_rx_basic_test_run: RX descriptors preloaded");

    /* Step 31: Start RX by writing RX descriptor tail pointer and enabling CH0 RX */
    writel_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0x00000000U);
    // MANUAL_REVIEW: Set correct RX descriptor tail pointer value.
    LOGT("ethernet0_rx_basic_test_run: RX descriptor tail pointer written");

    /* Enable CH0 RX via DMA_CH0_RX_CONTROL */
    // MANUAL_REVIEW: Write correct enable bit to DMA_CH0_RX_CONTROL to start RX.
    writel_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00000001U);
    LOGT("ethernet0_rx_basic_test_run: CH0 RX enabled");

    /* Trigger VIP sequencer */
    writel_reg(0xA0243ffcU, 0x00000000U);
    writel_reg(0xA0243ff8U, 0x00000000U);
    // MANUAL_REVIEW: Set correct VIP sequencer trigger values.
    LOGT("ethernet0_rx_basic_test_run: VIP sequencer triggered");

    /* Step 32: Loop trns_count times waiting for transfer-complete interrupts */
    LOGT("ethernet0_rx_basic_test_run: entering transfer loop, trns_count=%u",
         g_ctx.trns_count);

    for (i = 0U; i < g_ctx.trns_count; i++) {
        /* Wait for int_pend to become 0 (set by IRQ handler) */
        timeout = ETHERNET0_RX_TIMEOUT;
        while ((g_ctx.int_pend != 0U) && (timeout > 0U)) {
            timeout--;
        }

        if (timeout == 0U) {
            LOGE("ethernet0_rx_basic_test_run: timeout waiting for interrupt, iteration=%u", i);
            g_ctx.errors++;
            out->status = -1;
            break;
        }

        LOGT("ethernet0_rx_basic_test_run: interrupt received, iteration=%u", i);

        /* Reset int_pend for next iteration */
        g_ctx.int_pend = 1U;
    }

    /* Step 33: wait_on(2000) equivalent */
    // MANUAL_REVIEW: DV wait_on(2000) was present in the source flow.
    // In PSV/FV, this is converted to a delay or polling mechanism.
    // Use framework-supported delay if available.
    LOGT("ethernet0_rx_basic_test_run: post-transfer wait");

    /* Step 34: Determine final pass/fail status */
    if (g_ctx.errors > 0U) {
        LOGE("ethernet0_rx_basic_test_run: FAIL errors=%u", g_ctx.errors);
        out->status = -1;
    } else {
        LOGT("ethernet0_rx_basic_test_run: PASS all transfers completed");
        out->status = 0;
    }

    LOGT("ethernet0_rx_basic_test_run: complete, irq_serviced=%u errors=%u",
         g_ctx.irq_serviced, g_ctx.errors);

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

    LOGT("ethernet0_rx_basic_test_teardown: irq_serviced=%u errors=%u",
         g_ctx.irq_serviced, g_ctx.errors);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
