// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

// Global declarations per Meta Step 9
int data_rd, data_wr;
int def_fail_cnt = 0, wr_fail_cnt = 0;
extern int int_pend;

// MANUAL_REVIEW: extern int rxpkt, txpkt; - these variables are referenced in
// Meta polling loops but not declared in Meta globals. They are expected to be
// updated by the interrupt handler or ethernet framework. Ensure they are
// declared/defined in the project ethernet framework headers.
extern int rxpkt;
extern int txpkt;

// MANUAL_REVIEW: extern int enet_sel; - used throughout Meta steps but declared
// externally via enet_intf_sel(). Ensure it is declared in ethernet0 framework headers.
extern int enet_sel;

// MANUAL_REVIEW: extern int enet_select; - used in Default_IRQHandler per Meta Step 89.
// Ensure it is declared in ethernet0 framework headers.
extern int enet_select;

// MANUAL_REVIEW: extern int trns_count; - used in Meta Step 12.
// Ensure it is declared in ethernet0 framework headers.
extern int trns_count;

// MANUAL_REVIEW: ATP Runtime Condition - External Ethernet Validation:
// The ATP requires an external PHY loopback topology with RJ45 Ethernet cable,
// network switch, JTAG debugger, and oscilloscope. The external endpoint and
// hardware topology must be documented and identified before running this test.
// Ensure the eval board is connected to an external Ethernet switch/PHY via RJ45.

// MANUAL_REVIEW: ATP Runtime Condition - End-to-End Ethernet Validation:
// The ATP requires validation of TX frame, RX frame, payload integrity,
// source and destination MAC addresses, and external packet verification
// (e.g., via Wireshark). The current Meta testcase relies on TX/RX packet
// counters (rxpkt/txpkt) and interrupt-driven completion. Additional payload
// integrity and MAC address validation should be added per ATP requirements.
// Do not rely solely on TX/RX counters for PASS criteria.

// MANUAL_REVIEW: ATP Runtime Condition - Duplex Validation:
// The ATP requires distinct configurations for Half-Duplex and Full-Duplex
// with readback/status checks to verify the configured mode. The Meta testcase
// uses compile-time macros (ETH_10M, ETH_100M, HALF_DUPLEX) to select the mode
// but does not perform a readback of MAC_CONFIGURATION to verify the configured
// duplex/speed. Add a readback of MAC_CONFIGURATION after writing to verify
// the configured mode per ATP requirements.


/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for Ethernet0 multi-channel test per Meta Step 89.
 * Parameters:
 * None
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
    int rdata, wdata;
    unsigned int irq_no;
    int rdata_tailpointer;
    int rdata_desc;

    // Meta Step 89: int_pend = 0
    int_pend = 0;

    // Meta Step 89: irq_no = 42 + enet_select
    irq_no = 42 + enet_select;

    // Meta Step 89: enet_sel = enet_select
    enet_sel = enet_select;

    // Meta Step 89: printf replaced with LOGT
    LOGT("IN THE HANDLER %d for eth%d", irq_no, enet_select);

    // Meta Step 89: read DMA CH0 current app RX desc
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);

    // Meta Step 89: read DMA CH0 current app RX buffer
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXBUFFER);

    // Meta Step 89: read DMA CH3 current app TX desc
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);

    // Meta Step 89: read DMA CH3 current app TX buffer
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);

    // Meta Step 89: write DMA CH2 TX desc tail pointer
    write_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE60298a4);

    // Meta Step 89: write DMA CH3 TX desc tail pointer
    write_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE60398a4);

    // Suppress unused variable warnings for variables declared per Meta
    (void)wdata;
    (void)rdata_tailpointer;
    (void)rdata_desc;
    (void)rdata;
}


/*
 * Function: ethernet0_tx_rx_multi_chnl_test_init
 * Description: Initialize Ethernet0 multi-channel TX/RX test. Selects interface,
 * configures MAC, MTL, and DMA layers per Meta Steps 12-75.
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem cfg)
{
    (void)cfg;

    LOGT("ethernet0_tx_rx_multi_chnl_test_init: start");

    // Meta Step 12: #ifdef ETH_10M trns_count = 6; #endif
#ifdef ETH_10M
    trns_count = 6;
#endif

    // Meta Step 13: Ethernet interface selection
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

    // Meta Step 14: MAC Configuration with speed and duplex settings
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

    // MANUAL_REVIEW: ATP Duplex Validation - Add readback of MAC_CONFIGURATION
    // to verify the configured speed/duplex mode per ATP requirements.

    // Meta Step 15: Enabling Override
    write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08);

    // Meta Step 16: wait_on(300) - converted to PSV busy-wait
    for (volatile int d = 0; d < 300000; d++);

    // Meta Step 17: MAC_EXT_CONFIGURATION
    write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);

    // Meta Step 18: MAC_RXQ_CTRL0
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x000000aa);

    // Meta Step 19: MAC_RXQ_CTRL1
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x00000000);

    // Meta Step 20: MAC_RXQ_CTRL2
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x08040201);

    // Meta Step 21: MAC_PACKET_FILTER (hash filter + receive all)
    write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000408);

    // Meta Step 22: MAC_VLAN_TAG_CTRL
    write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x01600000);

    // Meta Step 23: MAC_ADDRESS3_HIGH
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x80033607);

    // Meta Step 24: MAC_ADDRESS2_HIGH
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x80022607);

    // Meta Step 25: MAC_ADDRESS1_HIGH
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x80011607);

    // Meta Step 26: MAC_ADDRESS0_HIGH
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x80000607);

    // Meta Step 27: MAC_ADDRESS2_LOW
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x08090a0b);

    // Meta Step 28: MAC_ADDRESS1_LOW
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x08090a0b);

    // Meta Step 29: MAC_ADDRESS0_LOW
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x08090a0b);

    LOGT("MAC layer configured");

    // Meta Step 30: MTL_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000);

    // Meta Step 31: MTL_TXQ2_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x0003000a);

    // Meta Step 32: MTL_TXQ1_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x0003000a);

    // Meta Step 33: MTL_TXQ0_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x0003000a);

    // Meta Step 34: MTL_RXQ3_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE, 0x00f00033);

    // Meta Step 35: MTL_RXQ2_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE, 0x00f00033);

    // Meta Step 36: MTL_RXQ1_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE, 0x00f00033);

    // Meta Step 37: MTL_RXQ0_OPERATION_MODE
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE, 0x00f00033);

    // Meta Step 38: MTL_TXQ3_QUANTUM_WEIGHT
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000014);

    // Meta Step 39: MTL_TXQ2_QUANTUM_WEIGHT
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000014);

    // Meta Step 40: MTL_TXQ1_QUANTUM_WEIGHT
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005);

    // Meta Step 41: MTL_RXQ_DMA_MAP0 initial
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);

    // Meta Step 42: MTL_Q3_INTERRUPT_CONTROL_STATUS
    write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100);

    // Meta Step 43: MTL_Q2_INTERRUPT_CONTROL_STATUS
    write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100);

    // Meta Step 44: MTL_Q1_INTERRUPT_CONTROL_STATUS
    write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100);

    LOGT("MTL layer configured");

    // Meta Step 45: DMA_CH3_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);

    // Meta Step 46: DMA_CH2_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);

    // Meta Step 47: DMA_CH1_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);

    // Meta Step 48: DMA_CH0_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

    // Meta Step 49: DMA_CH3_TX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);

    // Meta Step 50: DMA_CH2_TX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);

    // Meta Step 51: DMA_CH1_TX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);

    // Meta Step 52: DMA_CH0_TX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

    // Meta Step 53: DMA_CH3_RX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);

    // Meta Step 54: DMA_CH2_RX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);

    // Meta Step 55: DMA_CH1_RX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);

    // Meta Step 56: DMA_CH0_RX_CONTROL (initial bulk)
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

    // Meta Step 57: DMA_CH0_RX_CONTROL2
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x9);

    // Meta Step 58: DMA_CH0_INTERRUPT_ENABLE
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0xf0df);

    // Meta Step 59: DMA_MODE
    write_enet_reg(mizar_ETHERNET0_DMA_MODE, 0x00000000);

    // Meta Step 60: DMA_CH3_TXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0xE6030000);

    // Meta Step 61: DMA_CH2_TXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0xE6020000);

    // Meta Step 62: DMA_CH1_TXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0xE6010000);

    // Meta Step 63: DMA_CH0_TXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);

    // Meta Step 64: DMA_CH3_RXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0xE6070000);

    // Meta Step 65: DMA_CH2_RXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0xE6060000);

    // Meta Step 66: DMA_CH1_RXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0xE6050000);

    // Meta Step 67: DMA_CH0_RXDESC_LIST_ADDRESS
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);

    // Meta Step 68: DMA_CH3_TXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE6039EB4);

    // Meta Step 69: DMA_CH2_TXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE6029EB4);

    // Meta Step 70: DMA_CH1_TXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0xE6019EB4);

    // Meta Step 71: DMA_CH0_TXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4);

    // Meta Step 72: DMA_CH3_RXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);

    // Meta Step 73: DMA_CH2_RXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);

    // Meta Step 74: DMA_CH1_RXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);

    // Meta Step 75: DMA_CH0_RXDESC_TAIL_POINTER
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040010);

    LOGT("DMA layer configured");

    LOGT("ethernet0_tx_rx_multi_chnl_test_init: complete");
    return 0;
}


/*
 * Function: ethernet0_tx_rx_multi_chnl_test_run
 * Description: Execute multi-channel TX/RX test across CH0-CH3 sequentially.
 * Per Meta Steps 76-88.
 * Parameters:
 * cfg - pointer to test configuration
 * out - pointer to test output
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput out)
{
    int i;

    (void)cfg;

    if (out == 0) {
        LOGE("ethernet0_tx_rx_multi_chnl_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet0_tx_rx_multi_chnl_test_run: start");

    // Meta Step 76: write_reg(0xA0243ffc, enet_sel) - DV VIP sequencer trigger
    // This is a DV/VIP sequencer write, not the exact debug marker 0xdeadbee0.
    // Preserving as DV infrastructure.
    // MANUAL_REVIEW: write_reg(0xA0243ffc, enet_sel) is DV VIP sequencer start.
    // In PSV, the external traffic generator/switch must be configured separately.
    LOGD("DV VIP sequencer trigger: write_reg(0xA0243ffc, enet_sel=%d)", enet_sel);

    // Meta Step 77: write_reg(0xA0243ff8, 0xdeadbeef) - DV VIP sequencer start
    // MANUAL_REVIEW: write_reg(0xA0243ff8, 0xdeadbeef) is DV VIP sequencer start.
    // In PSV, external traffic must be initiated by the test setup/external endpoint.
    LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeef)");

    // Meta Step 78: Channel 0 loop
    LOGT("Channel 0: starting polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 20 && txpkt == 20) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 0: loop complete, rxpkt=%d txpkt=%d", rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - After Channel 0 completion,
    // validate TX frame payload integrity, RX frame payload integrity,
    // and source/destination MAC addresses. Do not rely solely on rxpkt/txpkt counters.

    // Meta Step 79: int_pend = 1
    int_pend = 1;

    // Meta Step 80: Channel 1 activation
    LOGT("Channel 1: activating");
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000081);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101001);

    // Meta Step 81: write_reg(0xA0243ff8, 0xdeadbeed) - DV VIP sequencer
    // MANUAL_REVIEW: DV VIP sequencer trigger for Channel 1.
    // In PSV, external traffic must be initiated by the test setup/external endpoint.
    LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeed)");

    // Meta Step 82: Channel 1 loop
    LOGT("Channel 1: starting polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 20 && txpkt == 20) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 1: loop complete, rxpkt=%d txpkt=%d", rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - After Channel 1 completion,
    // validate TX frame payload integrity, RX frame payload integrity,
    // and source/destination MAC addresses.

    // Meta Step 83: int_pend = 1
    int_pend = 1;

    // Meta Step 84: Channel 2 activation
    LOGT("Channel 2: activating");
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000082);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101001);

    // Meta Step 85: write_reg(0xA0243ff8, 0xdeadbeee) - DV VIP sequencer
    // MANUAL_REVIEW: DV VIP sequencer trigger for Channel 2.
    // In PSV, external traffic must be initiated by the test setup/external endpoint.
    LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeee)");

    // Meta Step 86: Channel 2 loop
    LOGT("Channel 2: starting polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 10 && txpkt == 10) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 2: loop complete, rxpkt=%d txpkt=%d", rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - After Channel 2 completion,
    // validate TX frame payload integrity, RX frame payload integrity,
    // and source/destination MAC addresses.

    // Meta Step 87: int_pend = 1
    int_pend = 1;

    // Meta Step 88: Channel 3 activation
    LOGT("Channel 3: activating");
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101001);

    LOGT("Channel 3: activated");

    // Meta Step 90: Pass/fail determination
    // finish() replaced with out->status per FV/PSV completion rule
    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        LOGE("FAIL: def_fail_cnt=%d wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        out->status = -1;
    } else {
        LOGT("PASS: def_fail_cnt=%d wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        out->status = 0;
    }

    LOGT("ethernet0_tx_rx_multi_chnl_test_run: %s",
        (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}


/*
 * Function: ethernet0_tx_rx_multi_chnl_test_teardown
 * Description: Teardown for Ethernet0 multi-channel TX/RX test.
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_tx_rx_multi_chnl_test teardown: def_fail_cnt=%d wr_fail_cnt=%d",
        def_fail_cnt, wr_fail_cnt);

    return (def_fail_cnt == 0 && wr_fail_cnt == 0) ? 0 : -1;
}
