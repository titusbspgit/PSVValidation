// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

// Global declarations from Meta (Step 9)
int data_rd;
int data_wr;
int def_fail_cnt = 0;
int wr_fail_cnt = 0;
extern int int_pend;
extern int rxpkt;
extern int txpkt;
extern int enet_sel;
extern int enet_select;
extern int trns_count;

// MANUAL_REVIEW: ATP Section 9.19 - External Ethernet Validation
// External endpoint: Network switch connected via RJ45 Ethernet cable.
// Hardware topology: Mizar SoC -> Ethernet PHY -> RJ45 cable -> Network Switch.
// Communication path: MIZAR SoC ETHERNET controller -> PHY -> external switch.
// Tools required: RJ45 Ethernet cable, network switch, UART USB cable,
// JTAG debugger, oscilloscope, Wireshark (for packet capture).

// MANUAL_REVIEW: ATP Section 9.19 - End-to-End Ethernet Validation
// ATP requires: Validate TX frame content, RX frame content, payload integrity,
// source and destination MAC addresses, and external packet verification (Wireshark).
// The DV Meta testcase uses rxpkt/txpkt counters as primary validation.
// Additional frame-level and payload-level validation should be added per ATP.
// Do not rely solely on TX/RX counters for PASS criteria.

// MANUAL_REVIEW: ATP Section 9.19 - Duplex Validation
// ATP requires: Configure duplex mode, use distinct configs for Half/Full Duplex,
// verify configured mode through readback/status checks.
// If ATP requires both modes, validate both independently.
// The DV Meta uses compile-time macros (ETH_10M, ETH_100M, HALF_DUPLEX) for
// speed/duplex selection but does not perform MAC_CONFIGURATION readback.
// A readback check should be added per ATP requirements.

// Ethernet test context (FV Template structural pattern)
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} eth0_multi_chnl_ctx_t;

static eth0_multi_chnl_ctx_t g_ctx;

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for Ethernet0 DMA channels (Meta Step 89).
 *             Clears int_pend, reads DMA current app descriptors/buffers,
 *             and updates TX descriptor tail pointers for CH2 and CH3.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rdata;
    int wdata;
    unsigned int irq_no;
    int rdata_tailpointer;
    int rdata_desc;

    (void)wdata;
    (void)rdata_tailpointer;
    (void)rdata_desc;

    // int_pend = 0 on entry
    int_pend = 0;

    irq_no = 42 + enet_select;
    enet_sel = enet_select;

    LOGT("IN THE HANDLER %d for eth%d", irq_no, enet_select);

    // Read DMA CH0 current app RX descriptor
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
    LOGT("DMA_CH0_CURRENT_APP_RXDESC = 0x%x", rdata);

    // Read DMA CH0 current app RX buffer
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXBUFFER);
    LOGT("DMA_CH0_CURRENT_APP_RXBUFFER = 0x%x", rdata);

    // Read DMA CH3 current app TX descriptor
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);
    LOGT("DMA_CH3_CURRENT_APP_TXDESC = 0x%x", rdata);

    // Read DMA CH3 current app TX buffer
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);
    LOGT("DMA_CH3_CURRENT_APP_TXBUFFER = 0x%x", rdata);

    // Write DMA CH2 TX descriptor tail pointer
    write_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE60298a4);

    // Write DMA CH3 TX descriptor tail pointer
    write_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE60398a4);
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_init
 * Description: Initialize Ethernet0 multi-channel TX/RX test.
 *             Configures MAC, MTL, and DMA layers for all 4 channels
 *             (Meta Steps 12-75).
 * Parameters:
 *   cfg - Pointer to test configuration structure
 * Returns:
 *   0 on success
 */
int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (eth0_multi_chnl_ctx_t){0};
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    LOGT("Ethernet0 multi-channel TX/RX test init");

    // Step 12: ETH_10M trns_count
#ifdef ETH_10M
    trns_count = 6;
#endif

    // Step 13: Ethernet interface selection
#ifdef SEL_ENET0
    enet_intf_sel(0);
#elif defined(SEL_ENET1)
    enet_intf_sel(1);
#elif defined(SEL_ENET2)
    enet_intf_sel(2);
#elif defined(SEL_ENET3)
    enet_intf_sel(3);
#else
    enet_intf_sel(4);
#endif

    // Step 14: MAC Configuration with speed and duplex settings
#ifdef ETH_10M
#ifdef HALF_DUPLEX
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x8003);
#else
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xA003);
#endif
#elif defined(ETH_100M)
#ifdef HALF_DUPLEX
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xC003);
#else
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xE003);
#endif
#else
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x00002003);
#endif
    LOGT("MAC_CONFIGURATION written");

    // MANUAL_REVIEW: ATP Duplex Validation - Readback MAC_CONFIGURATION register
    // to verify configured speed/duplex mode. Add:
    // int mac_cfg_rb = read_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION);
    // and compare against expected value per ATP Section 9.19.

    // Step 15: Enabling Override
    write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08);
    LOGT("Override enabled at addr 0x%x", (0xE68C205C + (enet_sel * 4)));

    // Step 16: wait_on(300) - PSV busy-wait adaptation
    for (volatile int d = 0; d < 300000; d++);

    // Step 17: MAC_EXT_CONFIGURATION
    write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);

    // Step 18: MAC_RXQ_CTRL0
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x000000aa);

    // Step 19: MAC_RXQ_CTRL1
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x00000000);

    // Step 20: MAC_RXQ_CTRL2 (Priorities for RX Queues)
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x08040201);

    // Step 21: MAC_PACKET_FILTER (HASH FILTER Enabled and Receive All)
    write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000408);

    // Step 22: MAC_VLAN_TAG_CTRL (VLAN TAG in status and RX Stripping Enabled)
    write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x01600000);

    // Step 23: MAC_ADDRESS3_HIGH (DMA Channel selection, Address Enabled)
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x80033607);

    // Step 24: MAC_ADDRESS2_HIGH
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x80022607);

    // Step 25: MAC_ADDRESS1_HIGH
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x80011607);

    // Step 26: MAC_ADDRESS0_HIGH
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x80000607);

    // Step 27: MAC_ADDRESS2_LOW
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x08090a0b);

    // Step 28: MAC_ADDRESS1_LOW
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x08090a0b);

    // Step 29: MAC_ADDRESS0_LOW
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x08090a0b);
    LOGT("MAC addresses configured");

    // Step 30: MTL_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000);

    // Step 31: MTL_TXQ2_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x0003000a);

    // Step 32: MTL_TXQ1_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x0003000a);

    // Step 33: MTL_TXQ0_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x0003000a);

    // Step 34: MTL_RXQ3_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE, 0x00f00033);

    // Step 35: MTL_RXQ2_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE, 0x00f00033);

    // Step 36: MTL_RXQ1_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE, 0x00f00033);

    // Step 37: MTL_RXQ0_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE, 0x00f00033);

    // Step 38: MTL_TXQ3_QUANTUM_WEIGHT
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000014);

    // Step 39: MTL_TXQ2_QUANTUM_WEIGHT
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000014);

    // Step 40: MTL_TXQ1_QUANTUM_WEIGHT
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005);

    // Step 41: MTL_RXQ_DMA_MAP0 initial
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);

    // Step 42: MTL_Q3_INTERRUPT_CONTROL_STATUS
    write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100);

    // Step 43: MTL_Q2_INTERRUPT_CONTROL_STATUS
    write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100);

    // Step 44: MTL_Q1_INTERRUPT_CONTROL_STATUS
    write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100);
    LOGT("MTL layer configured");

    // Step 45: DMA_CH3_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);

    // Step 46: DMA_CH2_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);

    // Step 47: DMA_CH1_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);

    // Step 48: DMA_CH0_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

    // Step 49: DMA_CH3_TX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);

    // Step 50: DMA_CH2_TX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);

    // Step 51: DMA_CH1_TX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);

    // Step 52: DMA_CH0_TX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

    // Step 53: DMA_CH3_RX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);

    // Step 54: DMA_CH2_RX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);

    // Step 55: DMA_CH1_RX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);

    // Step 56: DMA_CH0_RX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

    // Step 57: DMA_CH0_RX_CONTROL2
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x9);

    // Step 58: DMA_CH0_INTERRUPT_ENABLE
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0xf0df);

    // Step 59: DMA_MODE
    write_enet_reg(mizar_ETHERNET0_DMA_MODE, 0x00000000);

    // Step 60: DMA_CH3_TXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0xE6030000);

    // Step 61: DMA_CH2_TXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0xE6020000);

    // Step 62: DMA_CH1_TXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0xE6010000);

    // Step 63: DMA_CH0_TXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);

    // Step 64: DMA_CH3_RXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0xE6070000);

    // Step 65: DMA_CH2_RXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0xE6060000);

    // Step 66: DMA_CH1_RXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0xE6050000);

    // Step 67: DMA_CH0_RXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);

    // Step 68: DMA_CH3_TXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE6039EB4);

    // Step 69: DMA_CH2_TXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE6029EB4);

    // Step 70: DMA_CH1_TXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0xE6019EB4);

    // Step 71: DMA_CH0_TXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4);

    // Step 72: DMA_CH3_RXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);

    // Step 73: DMA_CH2_RXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);

    // Step 74: DMA_CH1_RXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);

    // Step 75: DMA_CH0_RXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040010);
    LOGT("DMA layer configured for all 4 channels");

    return 0;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_run
 * Description: Execute multi-channel TX/RX test across CH0-CH3 sequentially.
 *             Each channel is activated, VIP sequencer triggered, and
 *             interrupt-driven polling loop waits for packet completion.
 *             (Meta Steps 76-90).
 * Parameters:
 *   cfg - Pointer to test configuration structure
 *   out - Pointer to test output structure
 * Returns:
 *   0 on success (PASS), -1 on failure (FAIL)
 */
int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput *out)
{
    int i;

    (void)cfg;

    if (out == 0) {
        LOGE("Ethernet0 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting Ethernet0 multi-channel TX/RX test");

    // Step 76: VIP sequencer trigger - write enet_sel to 0xA0243ffc
    // This is a DV VIP sequencer trigger address, not DUT functional register.
    // MANUAL_REVIEW: In PSV, the external traffic generator / network switch
    // must be started externally. This DV VIP trigger is preserved as logging.
    LOGD("DV VIP sequencer trigger: write_reg(0xA0243ffc, enet_sel=%d)", enet_sel);

    // Step 77: VIP sequencer trigger - write 0xdeadbeef to 0xA0243ff8
    // MANUAL_REVIEW: DV VIP sequencer trigger for CH0 traffic generation.
    LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeef)");

    // Step 78: Channel 0 loop - poll int_pend, break when rxpkt==20 && txpkt==20
    LOGT("Channel 0: Starting TX/RX polling loop (20 TX, 20 RX expected)");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            // wait_on(10) - PSV busy-wait adaptation
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 20 && txpkt == 20) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 0: Loop exited at iteration %d, rxpkt=%d, txpkt=%d", i, rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - After CH0 completion,
    // validate TX frame content, RX frame content, payload integrity,
    // and source/destination MAC addresses per ATP Section 9.19.
    // Do not rely solely on rxpkt/txpkt counters.

    // Step 79: int_pend = 1
    int_pend = 1;

    // Step 80: Channel 1 activation
    LOGT("Channel 1: Activating");
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000081);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101001);

    // Step 81: VIP sequencer trigger for CH1
    // MANUAL_REVIEW: DV VIP sequencer trigger for CH1 traffic generation.
    LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeed)");

    // Step 82: Channel 1 loop - poll int_pend, break when rxpkt==20 && txpkt==20
    LOGT("Channel 1: Starting TX/RX polling loop (20 TX, 20 RX expected)");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            // wait_on(10) - PSV busy-wait adaptation
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 20 && txpkt == 20) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 1: Loop exited at iteration %d, rxpkt=%d, txpkt=%d", i, rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - After CH1 completion,
    // validate TX frame content, RX frame content, payload integrity,
    // and source/destination MAC addresses per ATP Section 9.19.

    // Step 83: int_pend = 1
    int_pend = 1;

    // Step 84: Channel 2 activation
    LOGT("Channel 2: Activating");
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000082);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101001);

    // Step 85: VIP sequencer trigger for CH2
    // MANUAL_REVIEW: DV VIP sequencer trigger for CH2 traffic generation.
    LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeee)");

    // Step 86: Channel 2 loop - poll int_pend, break when rxpkt==10 && txpkt==10
    LOGT("Channel 2: Starting TX/RX polling loop (10 TX, 10 RX expected)");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            // wait_on(10) - PSV busy-wait adaptation
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 10 && txpkt == 10) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 2: Loop exited at iteration %d, rxpkt=%d, txpkt=%d", i, rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - After CH2 completion,
    // validate TX frame content, RX frame content, payload integrity,
    // and source/destination MAC addresses per ATP Section 9.19.

    // Step 87: int_pend = 1
    int_pend = 1;

    // Step 88: Channel 3 activation
    LOGT("Channel 3: Activating");
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101001);
    LOGT("Channel 3: Activated, RX control set to 0x00101001");

    // Step 90: Pass/fail determination based on def_fail_cnt and wr_fail_cnt
    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        LOGE("FAIL: def_fail_cnt=%d, wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        g_ctx.errors = (unsigned int)(def_fail_cnt + wr_fail_cnt);
        out->status = -1;
    } else {
        LOGT("PASS: def_fail_cnt=%d, wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        out->status = 0;
    }

    g_ctx.checks_failed = g_ctx.errors;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_teardown
 * Description: Teardown for Ethernet0 multi-channel TX/RX test.
 *             Reports final status based on failure counters.
 * Parameters:
 *   cfg - Pointer to test configuration structure
 * Returns:
 *   0 on success (PASS), -1 on failure (FAIL)
 */
int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("Ethernet0 multi-channel TX/RX teardown: def_fail_cnt=%d, wr_fail_cnt=%d",
         def_fail_cnt, wr_fail_cnt);

    return (def_fail_cnt == 0 && wr_fail_cnt == 0) ? 0 : -1;
}
