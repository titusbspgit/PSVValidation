// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * File: ethernet1_tx_basic_test.c
 * Description: FV testcase for Ethernet1 basic TX DMA packet transmission
 *              using interrupt-driven completion. Configures MAC addresses,
 *              packet filter, MAC configuration, MTL TX queues, DMA system
 *              bus mode, TX descriptors, DMA channels, and interrupts.
 *              Starts TX on DMA channel 0 and waits for 10 transfer-complete
 *              interrupts via Default_IRQHandler.
 */

#include "ethernet1_tx_basic_test.h"
#include "test_define.inc"

/*
 * Testcase context structure for ethernet1_tx_basic_test.
 * Tracks error counts, check statistics, and IRQ count.
 */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
    unsigned int irq_count;
} ethernet1_tx_basic_test_ctx_t;

static ethernet1_tx_basic_test_ctx_t g_ctx;

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for Ethernet1 TX DMA transfer-complete.
 *              Reads DMA CH0-CH3 Current_App_TxDesc and Current_App_TxBuffer,
 *              reads DMA_Interrupt_Status, clears CH0-CH3 status registers,
 *              re-reads DMA_Interrupt_Status, clears HSS autoreg interrupt,
 *              and sets int_pend to 0.
 */
void Default_IRQHandler(void)
{
    unsigned int data_rd;

    LOGT("Default_IRQHandler: Entered, irq_count=%u", g_ctx.irq_count);

    /* Read DMA CH0 Current App TxDesc and TxBuffer */
    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_CURRENT_APP_TXDESC);
    LOGT("IRQ: DMA_CH0_Current_App_TxDesc = 0x%08x", data_rd);

    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_CURRENT_APP_TXBUFFER);
    LOGT("IRQ: DMA_CH0_Current_App_TxBuffer = 0x%08x", data_rd);

    /* Read DMA CH1 Current App TxDesc and TxBuffer */
    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_CH1_CURRENT_APP_TXDESC);
    LOGT("IRQ: DMA_CH1_Current_App_TxDesc = 0x%08x", data_rd);

    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_CH1_CURRENT_APP_TXBUFFER);
    LOGT("IRQ: DMA_CH1_Current_App_TxBuffer = 0x%08x", data_rd);

    /* Read DMA CH2 Current App TxDesc and TxBuffer */
    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_CH2_CURRENT_APP_TXDESC);
    LOGT("IRQ: DMA_CH2_Current_App_TxDesc = 0x%08x", data_rd);

    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_CH2_CURRENT_APP_TXBUFFER);
    LOGT("IRQ: DMA_CH2_Current_App_TxBuffer = 0x%08x", data_rd);

    /* Read DMA CH3 Current App TxDesc and TxBuffer */
    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_CH3_CURRENT_APP_TXDESC);
    LOGT("IRQ: DMA_CH3_Current_App_TxDesc = 0x%08x", data_rd);

    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_CH3_CURRENT_APP_TXBUFFER);
    LOGT("IRQ: DMA_CH3_Current_App_TxBuffer = 0x%08x", data_rd);

    /* Read DMA Interrupt Status */
    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_INTERRUPT_STATUS);
    LOGT("IRQ: DMA_Interrupt_Status = 0x%08x", data_rd);

    /* Clear DMA CH0 through CH3 Status registers */
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_STATUS, (uint64_t)0xFFFFFFFFU);
    LOGT("IRQ: Cleared DMA_CH0_Status");

    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH1_STATUS, (uint64_t)0xFFFFFFFFU);
    LOGT("IRQ: Cleared DMA_CH1_Status");

    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH2_STATUS, (uint64_t)0xFFFFFFFFU);
    LOGT("IRQ: Cleared DMA_CH2_Status");

    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH3_STATUS, (uint64_t)0xFFFFFFFFU);
    LOGT("IRQ: Cleared DMA_CH3_Status");

    /* Re-read DMA Interrupt Status after clearing */
    data_rd = (unsigned int)readl_reg((uintptr_t)mizar_ETHERNET1_DMA_INTERRUPT_STATUS);
    LOGT("IRQ: DMA_Interrupt_Status (after clear) = 0x%08x", data_rd);

    /* Clear HSS autoreg interrupt */
    writel_reg((uintptr_t)HSS_AUTOREG_INT_CLEAR, (uint64_t)0xFFFFFFFFU);
    LOGT("IRQ: Cleared HSS autoreg interrupt");

    g_ctx.irq_count++;
    g_ctx.checks_total++;
    g_ctx.checks_passed++;
    int_pend = 0;

    LOGT("Default_IRQHandler: Exited, irq_count=%u", g_ctx.irq_count);
}

/*
 * Function: ethernet1_tx_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              ethernet1_tx_basic_test. Zeroes the context structure.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet1_tx_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (ethernet1_tx_basic_test_ctx_t){0};

    LOGT("ethernet1_tx_basic_test init: Ethernet1 basic TX DMA transmission test initialization");

    return 0;
}

/*
 * Function: ethernet1_tx_basic_test_run
 * Description: Executes the main testcase flow for ethernet1_tx_basic_test.
 *              Configures MAC, MTL, DMA, starts TX on CH0, and runs 10
 *              interrupt-driven TX completion cycles.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet1_tx_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int loop_cnt;

    (void)cfg;

    if (out == 0) {
        LOGE("ethernet1_tx_basic_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet1_tx_basic_test: Starting basic TX DMA transmission test");

    /* Step 1: Set int_pend = 1 */
    int_pend = 1;
    LOGT("Step 1: int_pend set to 1");

    /* Step 2: Configure MAC Address3 High/Low */
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_ADDRESS3_HIGH, (uint64_t)0x80030000U);
    LOGT("Step 2: MAC_Address3_High = 0x80030000");
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_ADDRESS3_LOW, (uint64_t)0x03030303U);
    LOGT("Step 2: MAC_Address3_Low = 0x03030303");

    /* Configure MAC Address2 High/Low */
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_ADDRESS2_HIGH, (uint64_t)0x80020000U);
    LOGT("Step 2: MAC_Address2_High = 0x80020000");
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_ADDRESS2_LOW, (uint64_t)0x02020202U);
    LOGT("Step 2: MAC_Address2_Low = 0x02020202");

    /* Configure MAC Address1 High/Low */
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_ADDRESS1_HIGH, (uint64_t)0x80010000U);
    LOGT("Step 2: MAC_Address1_High = 0x80010000");
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_ADDRESS1_LOW, (uint64_t)0x01010101U);
    LOGT("Step 2: MAC_Address1_Low = 0x01010101");

    /* Configure MAC Address0 High/Low */
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_ADDRESS0_HIGH, (uint64_t)0x80000000U);
    LOGT("Step 2: MAC_Address0_High = 0x80000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_ADDRESS0_LOW, (uint64_t)0x00000000U);
    LOGT("Step 2: MAC_Address0_Low = 0x00000000");

    /* Step 3: Configure MAC Packet Filter */
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_PACKET_FILTER, (uint64_t)0x80000400U);
    LOGT("Step 3: MAC_Packet_Filter = 0x80000400");

    /* Step 4: Configure MAC Configuration */
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_CONFIGURATION, (uint64_t)0x00002003U);
    LOGT("Step 4: MAC_Configuration = 0x00002003");

    /* Step 5: Configure MAC Extended Configuration */
    writel_reg((uintptr_t)mizar_ETHERNET1_MAC_EXT_CONFIGURATION, (uint64_t)0x00000000U);
    LOGT("Step 5: MAC_Ext_Configuration = 0x00000000");

    /* Step 6: Configure MTL TxQ0 through TxQ3 Operation Mode */
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_TXQ0_OPERATION_MODE, (uint64_t)0x000F000AU);
    LOGT("Step 6: MTL_TxQ0_Operation_Mode = 0x000f000a");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_TXQ1_OPERATION_MODE, (uint64_t)0x000F000AU);
    LOGT("Step 6: MTL_TxQ1_Operation_Mode = 0x000f000a");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_TXQ2_OPERATION_MODE, (uint64_t)0x000F000AU);
    LOGT("Step 6: MTL_TxQ2_Operation_Mode = 0x000f000a");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_TXQ3_OPERATION_MODE, (uint64_t)0x000F000AU);
    LOGT("Step 6: MTL_TxQ3_Operation_Mode = 0x000f000a");

    /* Step 7: Configure MTL TxQ0 through TxQ3 Quantum Weight */
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_TXQ0_QUANTUM_WEIGHT, (uint64_t)0x00000000U);
    LOGT("Step 7: MTL_TxQ0_Quantum_Weight = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_TXQ1_QUANTUM_WEIGHT, (uint64_t)0x00000000U);
    LOGT("Step 7: MTL_TxQ1_Quantum_Weight = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_TXQ2_QUANTUM_WEIGHT, (uint64_t)0x00000000U);
    LOGT("Step 7: MTL_TxQ2_Quantum_Weight = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_TXQ3_QUANTUM_WEIGHT, (uint64_t)0x00000000U);
    LOGT("Step 7: MTL_TxQ3_Quantum_Weight = 0x00000000");

    /* Step 8: Enable MTL Q0 through Q3 Interrupt Control Status */
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_Q0_INTERRUPT_CONTROL_STATUS, (uint64_t)0x00000003U);
    LOGT("Step 8: MTL_Q0_Interrupt_Control_Status = 0x00000003");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_Q1_INTERRUPT_CONTROL_STATUS, (uint64_t)0x00000003U);
    LOGT("Step 8: MTL_Q1_Interrupt_Control_Status = 0x00000003");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_Q2_INTERRUPT_CONTROL_STATUS, (uint64_t)0x00000003U);
    LOGT("Step 8: MTL_Q2_Interrupt_Control_Status = 0x00000003");
    writel_reg((uintptr_t)mizar_ETHERNET1_MTL_Q3_INTERRUPT_CONTROL_STATUS, (uint64_t)0x00000003U);
    LOGT("Step 8: MTL_Q3_Interrupt_Control_Status = 0x00000003");

    /* Step 9: Configure DMA System Bus Mode */
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_SYSBUS_MODE, (uint64_t)0x0103000EU);
    LOGT("Step 9: DMA_SysBus_Mode = 0x0103000e");

    /* Step 10: Preload TX descriptors */
    preload_descriptor();
    LOGT("Step 10: TX descriptors preloaded via preload_descriptor()");

    /* Step 11: Configure DMA CH0 through CH3 TxDesc Ring Length */
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_TXDESC_RING_LENGTH, (uint64_t)0x0000000AU);
    LOGT("Step 11: DMA_CH0_TxDesc_Ring_Length = 0x0000000A");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH1_TXDESC_RING_LENGTH, (uint64_t)0x0000000AU);
    LOGT("Step 11: DMA_CH1_TxDesc_Ring_Length = 0x0000000A");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH2_TXDESC_RING_LENGTH, (uint64_t)0x0000000AU);
    LOGT("Step 11: DMA_CH2_TxDesc_Ring_Length = 0x0000000A");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH3_TXDESC_RING_LENGTH, (uint64_t)0x0000000AU);
    LOGT("Step 11: DMA_CH3_TxDesc_Ring_Length = 0x0000000A");

    /* Step 12: Program DMA CH0 through CH3 TxDesc List Address */
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_TXDESC_LIST_ADDRESS, (uint64_t)0x00000000U);
    LOGT("Step 12: DMA_CH0_TxDesc_List_Address = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH1_TXDESC_LIST_ADDRESS, (uint64_t)0x00000000U);
    LOGT("Step 12: DMA_CH1_TxDesc_List_Address = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH2_TXDESC_LIST_ADDRESS, (uint64_t)0x00000000U);
    LOGT("Step 12: DMA_CH2_TxDesc_List_Address = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH3_TXDESC_LIST_ADDRESS, (uint64_t)0x00000000U);
    LOGT("Step 12: DMA_CH3_TxDesc_List_Address = 0x00000000");

    /* Step 13: Program DMA CH0 through CH3 TxDesc Tail Pointer */
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_TXDESC_TAIL_POINTER, (uint64_t)0x00000000U);
    LOGT("Step 13: DMA_CH0_TxDesc_Tail_Pointer = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH1_TXDESC_TAIL_POINTER, (uint64_t)0x00000000U);
    LOGT("Step 13: DMA_CH1_TxDesc_Tail_Pointer = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH2_TXDESC_TAIL_POINTER, (uint64_t)0x00000000U);
    LOGT("Step 13: DMA_CH2_TxDesc_Tail_Pointer = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH3_TXDESC_TAIL_POINTER, (uint64_t)0x00000000U);
    LOGT("Step 13: DMA_CH3_TxDesc_Tail_Pointer = 0x00000000");

    /* Step 14: Configure DMA CH0 through CH3 Control */
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_CONTROL, (uint64_t)0x00000000U);
    LOGT("Step 14: DMA_CH0_Control = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH1_CONTROL, (uint64_t)0x00000000U);
    LOGT("Step 14: DMA_CH1_Control = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH2_CONTROL, (uint64_t)0x00000000U);
    LOGT("Step 14: DMA_CH2_Control = 0x00000000");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH3_CONTROL, (uint64_t)0x00000000U);
    LOGT("Step 14: DMA_CH3_Control = 0x00000000");

    /* Step 15: Enable DMA CH0 through CH3 Interrupt Enable */
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_INTERRUPT_ENABLE, (uint64_t)0x00000001U);
    LOGT("Step 15: DMA_CH0_Interrupt_Enable = 0x00000001");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH1_INTERRUPT_ENABLE, (uint64_t)0x00000001U);
    LOGT("Step 15: DMA_CH1_Interrupt_Enable = 0x00000001");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH2_INTERRUPT_ENABLE, (uint64_t)0x00000001U);
    LOGT("Step 15: DMA_CH2_Interrupt_Enable = 0x00000001");
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH3_INTERRUPT_ENABLE, (uint64_t)0x00000001U);
    LOGT("Step 15: DMA_CH3_Interrupt_Enable = 0x00000001");

    /* Step 16: Enable HSS autoreg Ethernet interrupt */
    writel_reg((uintptr_t)HSS_AUTOREG_INT_ENABLE, (uint64_t)0x00000001U);
    LOGT("Step 16: HSS_AUTOREG_INT_ENABLE (0xE68C2058) = 0x00000001");

    /* Step 17: Start TX transmission on DMA Channel 0 */
    writel_reg((uintptr_t)mizar_ETHERNET1_DMA_CH0_TX_CONTROL, (uint64_t)0x00100007U);
    LOGT("Step 17: DMA_CH0_Tx_Control = 0x00100007 - TX started");

    /* Step 18: Interrupt-driven TX completion loop (10 iterations) */
    LOGT("Step 18: Entering interrupt-driven TX completion loop (%u iterations)", ETH1_TX_IRQ_LOOP_COUNT);

    for (loop_cnt = 0U; loop_cnt < ETH1_TX_IRQ_LOOP_COUNT; loop_cnt++) {
        LOGT("Iteration %u: Waiting for transfer-complete interrupt", loop_cnt + 1U);
        int_pend = 1;

        // MANUAL_REVIEW: DV wait_on() was present in the source flow. Using polling with timeout for PSV/FV-native equivalent.
        {
            unsigned int timeout = ETH1_TX_IRQ_TIMEOUT;
            while ((int_pend != 0) && (timeout > 0U)) {
                timeout--;
            }
            if (timeout == 0U) {
                LOGE("Iteration %u: Timeout waiting for interrupt", loop_cnt + 1U);
                g_ctx.errors++;
                break;
            }
        }

        LOGT("Iteration %u: Interrupt received and handled", loop_cnt + 1U);
    }

    /* Step 19: Final pass/fail determination */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to out->status PSV/FV-native reporting.
    if (g_ctx.irq_count < ETH1_TX_IRQ_LOOP_COUNT) {
        LOGE("Expected %u interrupts but received %u", ETH1_TX_IRQ_LOOP_COUNT, g_ctx.irq_count);
        g_ctx.errors++;
    }

    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u irq_count=%u checks_passed=%u checks_total=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.irq_count,
         g_ctx.checks_passed,
         g_ctx.checks_total);

    return out->status;
}

/*
 * Function: ethernet1_tx_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for ethernet1_tx_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet1_tx_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet1_tx_basic_test teardown: errors=%u irq_count=%u checks_total=%u",
         g_ctx.errors, g_ctx.irq_count, g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
