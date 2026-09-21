// Author - AI Force 2.3. 21-Sep-2025 20:45 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name : ethernet2_tx_basic_test
 * Test Description: Basic Ethernet2 TX transmission using DMA channel 0.
 *                   Configures MAC addresses, packet filter, MAC config,
 *                   MTL TX queues, DMA system bus mode, preloads descriptors,
 *                   configures DMA channels, enables interrupts, starts TX
 *                   on CH0, and waits for 10 transfer-complete interrupts.
 *                   ISR reads DMA descriptor/buffer pointers, clears DMA
 *                   channel status, and acknowledges external HSS autoreg
 *                   interrupt.
 */

#include "ethernet2_tx_basic_test.h"
#include "test_define.inc"

/* Testcase context structure */
typedef struct {
    unsigned int errors;
    unsigned int irq_count;
    volatile unsigned int int_pend;
} ethernet2_tx_basic_test_ctx_t;

static ethernet2_tx_basic_test_ctx_t g_ctx;

/*
 * Function: ethernet2_tx_basic_test_isr_handler
 * Description: Interrupt service routine for TX transfer complete.
 *              Reads DMA descriptor and buffer pointers for all 4 channels,
 *              reads and clears DMA interrupt status, clears all DMA channel
 *              status registers via write-1-to-clear, and acknowledges the
 *              external HSS autoreg interrupt.
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
static void ethernet2_tx_basic_test_isr_handler(void)
{
    unsigned int rdata;

    /* Step 53: Signal main loop that interrupt has been serviced */
    g_ctx.int_pend = 0U;
    g_ctx.irq_count++;

    LOGT("ISR: Entering ethernet2_tx_basic_test_isr_handler, irq_count=%u", g_ctx.irq_count);

    /* Step 54: Read DMA CH0 current app TX descriptor */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH0_CURRENT_APP_TXDESC);
    LOGT("ISR: DMA_CH0_CURRENT_APP_TXDESC=0x%08x", rdata);

    /* Step 55: Read DMA CH0 current app TX buffer */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH0_CURRENT_APP_TXBUFFER);
    LOGT("ISR: DMA_CH0_CURRENT_APP_TXBUFFER=0x%08x", rdata);

    /* Step 56: Read DMA CH1 current app TX descriptor */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH1_CURRENT_APP_TXDESC);
    LOGT("ISR: DMA_CH1_CURRENT_APP_TXDESC=0x%08x", rdata);

    /* Step 57: Read DMA CH1 current app TX buffer */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH1_CURRENT_APP_TXBUFFER);
    LOGT("ISR: DMA_CH1_CURRENT_APP_TXBUFFER=0x%08x", rdata);

    /* Step 58: Read DMA CH2 current app TX descriptor */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH2_CURRENT_APP_TXDESC);
    LOGT("ISR: DMA_CH2_CURRENT_APP_TXDESC=0x%08x", rdata);

    /* Step 59: Read DMA CH2 current app TX buffer */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH2_CURRENT_APP_TXBUFFER);
    LOGT("ISR: DMA_CH2_CURRENT_APP_TXBUFFER=0x%08x", rdata);

    /* Step 60: Read DMA CH3 current app TX descriptor */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH3_CURRENT_APP_TXDESC);
    LOGT("ISR: DMA_CH3_CURRENT_APP_TXDESC=0x%08x", rdata);

    /* Step 61: Read DMA CH3 current app TX buffer */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH3_CURRENT_APP_TXBUFFER);
    LOGT("ISR: DMA_CH3_CURRENT_APP_TXBUFFER=0x%08x", rdata);

    /* Step 62: Read DMA interrupt status before clearing */
    rdata = readl_reg(mizar_ETHERNET2_DMA_INTERRUPT_STATUS);
    LOGT("ISR: DMA_INTERRUPT_STATUS (before clear)=0x%08x", rdata);

    /* Step 63: Clear DMA CH0 status (write-1-to-clear) */
    writel_reg(mizar_ETHERNET2_DMA_CH0_STATUS, 0xFFFFFFFFU);

    /* Step 64: Clear DMA CH1 status (write-1-to-clear) */
    writel_reg(mizar_ETHERNET2_DMA_CH1_STATUS, 0xFFFFFFFFU);

    /* Step 65: Clear DMA CH2 status (write-1-to-clear) */
    writel_reg(mizar_ETHERNET2_DMA_CH2_STATUS, 0xFFFFFFFFU);

    /* Step 66: Clear DMA CH3 status (write-1-to-clear) */
    writel_reg(mizar_ETHERNET2_DMA_CH3_STATUS, 0xFFFFFFFFU);

    /* Step 67: Re-read DMA interrupt status after clearing */
    rdata = readl_reg(mizar_ETHERNET2_DMA_INTERRUPT_STATUS);
    LOGT("ISR: DMA_INTERRUPT_STATUS (after clear)=0x%08x", rdata);

    /* Step 68: Clear external HSS autoreg interrupt */
    writel_reg(0xE68C2050U, 0x4U);

    LOGT("ISR: Exiting ethernet2_tx_basic_test_isr_handler");
}

/*
 * Function: ethernet2_tx_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for ethernet2_tx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet2_tx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (ethernet2_tx_basic_test_ctx_t){0};

    LOGT("ethernet2_tx_basic_test_init: Initialization complete");

    return 0;
}

/*
 * Function: ethernet2_tx_basic_test_run
 * Description: Executes the main testcase flow for ethernet2_tx_basic_test.
 *              Configures MAC addresses, packet filter, MAC configuration,
 *              MTL TX queues, DMA system bus mode, preloads descriptors,
 *              configures DMA channels, enables interrupts, starts TX on CH0,
 *              and waits for 10 transfer-complete interrupts.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet2_tx_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int i;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("ethernet2_tx_basic_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet2_tx_basic_test_run: Starting testcase execution");

    /* Step 1: Set int_pend = 1 */
    g_ctx.int_pend = 1U;

    /* Step 2: Configure MAC Address3 High - addr_en, dma_channel_sel=3 */
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS3_HIGH, 0x80033607U);

    /* Step 3: Configure MAC Address2 High - addr_en, dma_channel_sel=2 */
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS2_HIGH, 0x80022607U);

    /* Step 4: Configure MAC Address1 High */
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS1_HIGH, 0x80011607U);

    /* Step 5: Configure MAC Address0 High */
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS0_HIGH, 0x80000607U);

    LOGT("ethernet2_tx_basic_test_run: MAC addresses (high) configured");

    /* Step 6: Configure MAC Address3 Low - addr[31:0] */
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS3_LOW, 0x08090A00U);

    /* Step 7: Configure MAC Address2 Low */
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS2_LOW, 0x08090A00U);

    /* Step 8: Configure MAC Address1 Low */
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS1_LOW, 0x08090A00U);

    /* Step 9: Configure MAC Address0 Low */
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS0_LOW, 0x08090A00U);

    LOGT("ethernet2_tx_basic_test_run: MAC addresses (low) configured");

    /* Step 10: Configure MAC Packet Filter - hash filter enabled, receive all */
    writel_reg(mizar_ETHERNET2_MAC_PACKET_FILTER, 0x80000400U);
    LOGT("ethernet2_tx_basic_test_run: MAC Packet Filter configured");

    /* Step 11: Configure MAC Configuration - TX/RX enable, full duplex */
    writel_reg(mizar_ETHERNET2_MAC_CONFIGURATION, 0x2003U);
    LOGT("ethernet2_tx_basic_test_run: MAC Configuration configured");

    /* Step 12: Configure MAC Extended Configuration */
    writel_reg(mizar_ETHERNET2_MAC_EXT_CONFIGURATION, 0x0U);
    LOGT("ethernet2_tx_basic_test_run: MAC Ext Configuration configured");

    /* Step 13: Configure MTL TXQ3 Operation Mode - 4096B queue, store-and-forward */
    writel_reg(mizar_ETHERNET2_MTL_TXQ3_OPERATION_MODE, 0x000F000AU);

    /* Step 14: Configure MTL TXQ2 Operation Mode */
    writel_reg(mizar_ETHERNET2_MTL_TXQ2_OPERATION_MODE, 0x000F000AU);

    /* Step 15: Configure MTL TXQ1 Operation Mode */
    writel_reg(mizar_ETHERNET2_MTL_TXQ1_OPERATION_MODE, 0x000F000AU);

    /* Step 16: Configure MTL TXQ0 Operation Mode */
    writel_reg(mizar_ETHERNET2_MTL_TXQ0_OPERATION_MODE, 0x000F000AU);

    LOGT("ethernet2_tx_basic_test_run: MTL TX Queue Operation Modes configured");

    /* Step 17: Program MTL TXQ3 Quantum Weight */
    writel_reg(mizar_ETHERNET2_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000005U);

    /* Step 18: Program MTL TXQ2 Quantum Weight */
    writel_reg(mizar_ETHERNET2_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000005U);

    /* Step 19: Program MTL TXQ1 Quantum Weight */
    writel_reg(mizar_ETHERNET2_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005U);

    /* Step 20: Program MTL TXQ0 Quantum Weight */
    writel_reg(mizar_ETHERNET2_MTL_TXQ0_QUANTUM_WEIGHT, 0x00000005U);

    LOGT("ethernet2_tx_basic_test_run: Quantum Weights programmed");

    /* Step 21: Enable MTL Q3 overflow/underflow interrupts */
    writel_reg(mizar_ETHERNET2_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100U);

    /* Step 22: Enable MTL Q2 overflow/underflow interrupts */
    writel_reg(mizar_ETHERNET2_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100U);

    /* Step 23: Enable MTL Q1 overflow/underflow interrupts */
    writel_reg(mizar_ETHERNET2_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100U);

    /* Step 24: Enable MTL Q0 overflow/underflow interrupts */
    writel_reg(mizar_ETHERNET2_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x01000100U);

    LOGT("ethernet2_tx_basic_test_run: MTL Queue Interrupts enabled");

    /* Step 25: Configure DMA System Bus Mode - outstanding requests, burst config */
    writel_reg(mizar_ETHERNET2_DMA_SYSBUS_MODE, 0x0103000EU);
    LOGT("ethernet2_tx_basic_test_run: DMA System Bus Mode configured");

    /* Step 26: Preload TX Descriptors (first set) */
    preload_descriptor(0xE6000000U, 0xE6008000U, 0x3CU, 0x5U);

    /* Step 27: Preload TX Descriptors (second set) */
    preload_descriptor(0xE6000050U, 0xE600812CU, 0x5E8U, 0x5U);

    LOGT("ethernet2_tx_basic_test_run: TX Descriptors preloaded");

    /* Step 28: Set DMA CH3 TX Descriptor Ring Length */
    writel_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_RING_LENGTH, 0xAU);

    /* Step 29: Set DMA CH2 TX Descriptor Ring Length */
    writel_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_RING_LENGTH, 0xAU);

    /* Step 30: Set DMA CH1 TX Descriptor Ring Length */
    writel_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_RING_LENGTH, 0xAU);

    /* Step 31: Set DMA CH0 TX Descriptor Ring Length */
    writel_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_RING_LENGTH, 0xAU);

    LOGT("ethernet2_tx_basic_test_run: TX Descriptor Ring Lengths set");

    /* Step 32: Program DMA CH3 TX Descriptor List Address */
    writel_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00300000U);

    /* Step 33: Program DMA CH2 TX Descriptor List Address */
    writel_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00200000U);

    /* Step 34: Program DMA CH1 TX Descriptor List Address */
    writel_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00100000U);

    /* Step 35: Program DMA CH0 TX Descriptor List Address */
    writel_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000U);

    LOGT("ethernet2_tx_basic_test_run: TX Descriptor List Addresses programmed");

    /* Step 36: Program DMA CH3 TX Descriptor Tail Pointer */
    writel_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_TAIL_POINTER, 0x00309730U);

    /* Step 37: Program DMA CH2 TX Descriptor Tail Pointer */
    writel_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_TAIL_POINTER, 0x00209730U);

    /* Step 38: Program DMA CH1 TX Descriptor Tail Pointer */
    writel_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_TAIL_POINTER, 0x00109730U);

    /* Step 39: Program DMA CH0 TX Descriptor Tail Pointer */
    writel_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4U);

    LOGT("ethernet2_tx_basic_test_run: TX Descriptor Tail Pointers programmed");

    /* Step 40: Configure DMA CH3 Control */
    writel_reg(mizar_ETHERNET2_DMA_CH3_CONTROL, 0x0U);

    /* Step 41: Configure DMA CH2 Control */
    writel_reg(mizar_ETHERNET2_DMA_CH2_CONTROL, 0x0U);

    /* Step 42: Configure DMA CH1 Control */
    writel_reg(mizar_ETHERNET2_DMA_CH1_CONTROL, 0x0U);

    /* Step 43: Configure DMA CH0 Control */
    writel_reg(mizar_ETHERNET2_DMA_CH0_CONTROL, 0x00000000U);

    LOGT("ethernet2_tx_basic_test_run: DMA Channel Controls configured");

    /* Step 44: Enable DMA CH1 Interrupts */
    writel_reg(mizar_ETHERNET2_DMA_CH1_INTERRUPT_ENABLE, 0x000F0C7U);

    /* Step 45: Enable DMA CH2 Interrupts */
    writel_reg(mizar_ETHERNET2_DMA_CH2_INTERRUPT_ENABLE, 0x000F0C7U);

    /* Step 46: Enable DMA CH3 Interrupts */
    writel_reg(mizar_ETHERNET2_DMA_CH3_INTERRUPT_ENABLE, 0x000F0C7U);

    /* Step 47: Enable DMA CH0 Interrupts */
    writel_reg(mizar_ETHERNET2_DMA_CH0_INTERRUPT_ENABLE, 0x000F0C7U);

    LOGT("ethernet2_tx_basic_test_run: DMA Channel Interrupts enabled");

    /* Step 48: Enable external HSS autoreg Ethernet interrupt */
    writel_reg(0xE68C2058U, 0x4U);
    LOGT("ethernet2_tx_basic_test_run: HSS autoreg Ethernet interrupt enabled");

    /* Step 49: Start TX transmission on DMA Channel 0 */
    writel_reg(mizar_ETHERNET2_DMA_CH0_TX_CONTROL, 0x00100007U);
    LOGT("ethernet2_tx_basic_test_run: TX transmission started on DMA CH0");

    /* Step 50: Wait for 10 transfer-complete interrupts */
    LOGT("ethernet2_tx_basic_test_run: Waiting for 10 transfer-complete interrupts");
    for (i = 0U; i < 10U; i++) {
        g_ctx.int_pend = 1U;
        timeout = ETH2_TX_IRQ_TIMEOUT;

        while (g_ctx.int_pend != 0U) {
            /* MANUAL_REVIEW: DV wait_on(10) was present in the source flow.
             * In PSV/FV, this polling loop uses a timeout counter to avoid
             * unbounded waiting. The ISR handler ethernet2_tx_basic_test_isr_handler()
             * must be registered with the platform interrupt framework to clear int_pend. */
            timeout--;
            if (timeout == 0U) {
                LOGE("ethernet2_tx_basic_test_run: Timeout waiting for interrupt iteration %u", i);
                g_ctx.errors++;
                out->status = -1;
                return out->status;
            }
        }

        LOGT("ethernet2_tx_basic_test_run: Transfer complete interrupt %u received", i + 1U);
    }

    /* Step 51: Final wait */
    // MANUAL_REVIEW: DV wait_on(2000) was present in the source flow.
    // No PSV/FV-native delay equivalent was available in the FV Template.

    LOGT("ethernet2_tx_basic_test_run: All 10 interrupts received");

    /* Step 52: Final pass/fail determination */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow, but PSV/FV-native
    // equivalent uses out->status based PASS/FAIL reporting.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("ethernet2_tx_basic_test_run: Run complete: %s errors=%u irq_count=%u",
         (out->status == 0) ? "PASS" : "FAIL", g_ctx.errors, g_ctx.irq_count);

    return out->status;
}

/*
 * Function: ethernet2_tx_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for ethernet2_tx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet2_tx_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet2_tx_basic_test_teardown: errors=%u irq_count=%u",
         g_ctx.errors, g_ctx.irq_count);

    LOGT("ethernet2_tx_basic_test_teardown: Teardown complete");

    return g_ctx.errors == 0U ? 0 : -1;
}
