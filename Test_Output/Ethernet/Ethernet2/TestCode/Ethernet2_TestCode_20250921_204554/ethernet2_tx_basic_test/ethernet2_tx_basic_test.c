// Author - AI Force 2.3. 21-Sep-2025 20:45 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name : ethernet2_tx_basic_test
 * Test Description: Basic Ethernet2 TX transmission using DMA channel 0.
 *                   Configures MAC addresses, packet filter, MAC config,
 *                   MTL TX queues, DMA system bus mode, descriptors,
 *                   DMA channels, interrupts, and starts TX on CH0.
 *                   Waits for 10 transfer-complete interrupts.
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
 *              status registers, and acknowledges external HSS autoreg interrupt.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void ethernet2_tx_basic_test_isr_handler(void)
{
    unsigned int rdata;

    /* Step 53: Signal main loop that interrupt has been serviced */
    g_ctx.int_pend = 0U;
    g_ctx.irq_count++;

    LOGT("ISR: Entering ethernet2_tx_basic_test_isr_handler, irq_count=%u", g_ctx.irq_count);

    /* Step 54-61: Read current app TX descriptor and TX buffer for all channels */
    rdata = readl_reg(mizar_ETHERNET2_DMA_CH0_CURRENT_APP_TXDESC);
    LOGT("ISR: DMA_CH0_CURRENT_APP_TXDESC=0x%08x", rdata);

    rdata = readl_reg(mizar_ETHERNET2_DMA_CH0_CURRENT_APP_TXBUFFER);
    LOGT("ISR: DMA_CH0_CURRENT_APP_TXBUFFER=0x%08x", rdata);

    rdata = readl_reg(mizar_ETHERNET2_DMA_CH1_CURRENT_APP_TXDESC);
    LOGT("ISR: DMA_CH1_CURRENT_APP_TXDESC=0x%08x", rdata);

    rdata = readl_reg(mizar_ETHERNET2_DMA_CH1_CURRENT_APP_TXBUFFER);
    LOGT("ISR: DMA_CH1_CURRENT_APP_TXBUFFER=0x%08x", rdata);

    rdata = readl_reg(mizar_ETHERNET2_DMA_CH2_CURRENT_APP_TXDESC);
    LOGT("ISR: DMA_CH2_CURRENT_APP_TXDESC=0x%08x", rdata);

    rdata = readl_reg(mizar_ETHERNET2_DMA_CH2_CURRENT_APP_TXBUFFER);
    LOGT("ISR: DMA_CH2_CURRENT_APP_TXBUFFER=0x%08x", rdata);

    rdata = readl_reg(mizar_ETHERNET2_DMA_CH3_CURRENT_APP_TXDESC);
    LOGT("ISR: DMA_CH3_CURRENT_APP_TXDESC=0x%08x", rdata);

    rdata = readl_reg(mizar_ETHERNET2_DMA_CH3_CURRENT_APP_TXBUFFER);
    LOGT("ISR: DMA_CH3_CURRENT_APP_TXBUFFER=0x%08x", rdata);

    /* Step 62: Read DMA interrupt status */
    rdata = readl_reg(mizar_ETHERNET2_DMA_INTERRUPT_STATUS);
    LOGT("ISR: DMA_INTERRUPT_STATUS (before clear)=0x%08x", rdata);

    /* Step 63-66: Clear all DMA channel status registers (write-1-to-clear) */
    writel_reg(mizar_ETHERNET2_DMA_CH0_STATUS, 0xFFFFFFFFU);
    writel_reg(mizar_ETHERNET2_DMA_CH1_STATUS, 0xFFFFFFFFU);
    writel_reg(mizar_ETHERNET2_DMA_CH2_STATUS, 0xFFFFFFFFU);
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

    /* Step 2-5: Configure MAC addresses (upper 16-bit with addr_en and DMA channel selection) */
    LOGT("ethernet2_tx_basic_test_run: Configuring MAC addresses (high)");
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS3_HIGH, 0x80033607U);
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS2_HIGH, 0x80022607U);
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS1_HIGH, 0x80011607U);
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS0_HIGH, 0x80000607U);

    /* Step 6-9: Configure MAC addresses (lower 32-bit) */
    LOGT("ethernet2_tx_basic_test_run: Configuring MAC addresses (low)");
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS3_LOW, 0x08090A00U);
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS2_LOW, 0x08090A00U);
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS1_LOW, 0x08090A00U);
    writel_reg(mizar_ETHERNET2_MAC_ADDRESS0_LOW, 0x08090A00U);

    /* Step 10: Configure MAC Packet Filter - hash filter enabled, receive all */
    LOGT("ethernet2_tx_basic_test_run: Configuring MAC Packet Filter");
    writel_reg(mizar_ETHERNET2_MAC_PACKET_FILTER, 0x80000400U);

    /* Step 11: Configure MAC Configuration - TX/RX enable, full duplex */
    LOGT("ethernet2_tx_basic_test_run: Configuring MAC Configuration");
    writel_reg(mizar_ETHERNET2_MAC_CONFIGURATION, 0x2003U);

    /* Step 12: Configure MAC Extended Configuration */
    LOGT("ethernet2_tx_basic_test_run: Configuring MAC Ext Configuration");
    writel_reg(mizar_ETHERNET2_MAC_EXT_CONFIGURATION, 0x0U);

    /* Step 13-16: Configure MTL TX Queue Operation Modes (4096B queue, store-and-forward) */
    LOGT("ethernet2_tx_basic_test_run: Configuring MTL TX Queue Operation Modes");
    writel_reg(mizar_ETHERNET2_MTL_TXQ3_OPERATION_MODE, 0x000F000AU);
    writel_reg(mizar_ETHERNET2_MTL_TXQ2_OPERATION_MODE, 0x000F000AU);
    writel_reg(mizar_ETHERNET2_MTL_TXQ1_OPERATION_MODE, 0x000F000AU);
    writel_reg(mizar_ETHERNET2_MTL_TXQ0_OPERATION_MODE, 0x000F000AU);

    /* Step 17-20: Program Quantum Weights */
    LOGT("ethernet2_tx_basic_test_run: Programming Quantum Weights");
    writel_reg(mizar_ETHERNET2_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000005U);
    writel_reg(mizar_ETHERNET2_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000005U);
    writel_reg(mizar_ETHERNET2_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005U);
    writel_reg(mizar_ETHERNET2_MTL_TXQ0_QUANTUM_WEIGHT, 0x00000005U);

    /* Step 21-24: Enable MTL Queue Overflow/Underflow Interrupts */
    LOGT("ethernet2_tx_basic_test_run: Enabling MTL Queue Interrupts");
    writel_reg(mizar_ETHERNET2_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100U);
    writel_reg(mizar_ETHERNET2_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100U);
    writel_reg(mizar_ETHERNET2_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100U);
    writel_reg(mizar_ETHERNET2_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x01000100U);

    /* Step 25: Configure DMA System Bus Mode */
    LOGT("ethernet2_tx_basic_test_run: Configuring DMA System Bus Mode");
    writel_reg(mizar_ETHERNET2_DMA_SYSBUS_MODE, 0x0103000EU);

    /* Step 26-27: Preload TX Descriptors */
    LOGT("ethernet2_tx_basic_test_run: Preloading TX Descriptors");
    preload_descriptor(0xE6000000U, 0xE6008000U, 0x3CU, 0x5U);
    preload_descriptor(0xE6000050U, 0xE600812CU, 0x5E8U, 0x5U);

    /* Step 28-31: Set TX Descriptor Ring Lengths */
    LOGT("ethernet2_tx_basic_test_run: Setting TX Descriptor Ring Lengths");
    writel_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_RING_LENGTH, 0xAU);
    writel_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_RING_LENGTH, 0xAU);
    writel_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_RING_LENGTH, 0xAU);
    writel_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_RING_LENGTH, 0xAU);

    /* Step 32-35: Program TX Descriptor List Base Addresses */
    LOGT("ethernet2_tx_basic_test_run: Programming TX Descriptor List Addresses");
    writel_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00300000U);
    writel_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00200000U);
    writel_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00100000U);
    writel_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000U);

    /* Step 36-39: Program TX Descriptor Tail Pointers */
    LOGT("ethernet2_tx_basic_test_run: Programming TX Descriptor Tail Pointers");
    writel_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_TAIL_POINTER, 0x00309730U);
    writel_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_TAIL_POINTER, 0x00209730U);
    writel_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_TAIL_POINTER, 0x00109730U);
    writel_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4U);

    /* Step 40-43: Configure DMA Channel Control Registers */
    LOGT("ethernet2_tx_basic_test_run: Configuring DMA Channel Controls");
    writel_reg(mizar_ETHERNET2_DMA_CH3_CONTROL, 0x0U);
    writel_reg(mizar_ETHERNET2_DMA_CH2_CONTROL, 0x0U);
    writel_reg(mizar_ETHERNET2_DMA_CH1_CONTROL, 0x0U);
    writel_reg(mizar_ETHERNET2_DMA_CH0_CONTROL, 0x00000000U);

    /* Step 44-47: Enable DMA Channel Interrupts */
    LOGT("ethernet2_tx_basic_test_run: Enabling DMA Channel Interrupts");
    writel_reg(mizar_ETHERNET2_DMA_CH1_INTERRUPT_ENABLE, 0x000F0C7U);
    writel_reg(mizar_ETHERNET2_DMA_CH2_INTERRUPT_ENABLE, 0x000F0C7U);
    writel_reg(mizar_ETHERNET2_DMA_CH3_INTERRUPT_ENABLE, 0x000F0C7U);
    writel_reg(mizar_ETHERNET2_DMA_CH0_INTERRUPT_ENABLE, 0x000F0C7U);

    /* Step 48: Enable external HSS autoreg Ethernet interrupt */
    LOGT("ethernet2_tx_basic_test_run: Enabling HSS autoreg Ethernet interrupt");
    writel_reg(0xE68C2058U, 0x4U);

    /* Step 49: Start TX transmission on DMA Channel 0 */
    LOGT("ethernet2_tx_basic_test_run: Starting TX transmission on DMA Channel 0");
    writel_reg(mizar_ETHERNET2_DMA_CH0_TX_CONTROL, 0x00100007U);

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

        /* Call ISR handler for PSV/FV simulation of interrupt servicing */
        // MANUAL_REVIEW: In actual PSV/FV platform, the ISR is invoked by hardware.
        // If platform does not auto-invoke ISR, call ethernet2_tx_basic_test_isr_handler() here.
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
