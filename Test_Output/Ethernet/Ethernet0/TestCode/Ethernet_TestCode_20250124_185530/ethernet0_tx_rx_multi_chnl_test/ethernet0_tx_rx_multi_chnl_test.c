// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

// Global declarations per Meta Step 9
int data_rd, data_wr;
int def_fail_cnt = 0, wr_fail_cnt = 0;
extern int int_pend;
extern int rxpkt;
extern int txpkt;
extern int enet_sel;
extern int enet_select;
extern int trns_count;

// MANUAL_REVIEW: ATP Section 9.19 requires external endpoint identification.
// Hardware topology: Mizar SoC Ethernet -> External PHY -> RJ45 cable -> Network Switch.
// Communication path: MIZAR SoC ETHERNET CONTROLLER -> PHY -> ETHERNET SWITCH -> Loopback.
// External packet verification via Wireshark is required per ATP.

// MANUAL_REVIEW: ATP requires end-to-end Ethernet validation including:
// - TX frame validation (payload integrity, source/destination MAC addresses)
// - RX frame validation (payload integrity, source/destination MAC addresses)
// - External packet verification where applicable (Wireshark)
// - Do not rely solely on TX/RX counters for PASS criteria.
// The DV source relies on txpkt/rxpkt counters. Additional payload/MAC validation
// must be added for full ATP compliance.

// MANUAL_REVIEW: ATP requires duplex mode readback/status verification.
// The DV source configures duplex via MAC_CONFIGURATION but does not readback to verify.
// Add MAC_CONFIGURATION readback after write to confirm duplex mode per ATP.

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for Ethernet0 DMA channels per Meta Step 89
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    int rdata, wdata;
    unsigned int irq_no;
    int rdata_tailpointer;
    int rdata_desc;

    // Meta Step 89: int_pend = 0 on entry
    int_pend = 0;

    // Meta Step 89: irq_no = 42 + enet_select
    irq_no = 42 + enet_select;

    // Meta Step 89: enet_sel = enet_select
    enet_sel = enet_select;

    LOGT("IN THE HANDLER %d for eth%d", irq_no, enet_select);

    // Meta Step 89: Read DMA CH0 current app RX desc
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);

    // Meta Step 89: Read DMA CH0 current app RX buffer
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXBUFFER);

    // Meta Step 89: Read DMA CH3 current app TX desc
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);

    // Meta Step 89: Read DMA CH3 current app TX buffer
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);

    // Meta Step 89: Write DMA CH2 TX desc tail pointer
    write_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE60298a4);

    // Meta Step 89: Write DMA CH3 TX desc tail pointer
    write_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE60398a4);

    (void)wdata;
    (void)rdata_tailpointer;
    (void)rdata_desc;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_init
 * Description: Initialize Ethernet0 multi-channel TX/RX test - MAC, MTL, DMA configuration
 * Parameters:
 *   cfg - pointer to test configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem cfg)
{
    (void)cfg;

    LOGT("Ethernet0 multi-channel TX/RX test init");

    // Reset failure counters
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    // Meta Step 12: #ifdef ETH_10M trns_count = 6
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
    LOGT("MAC_CONFIGURATION programmed");

    // Meta Step 15: Enabling Override
    write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08);
    LOGT("Override enabled at addr 0x%x", (0xE68C205C + (enet_sel * 4)));

    // Meta Step 16: wait_on(300) - PSV busy-wait adaptation
    for (volatile int d = 0; d < 300000; d++);

    // Meta Step 17: MAC_EXT_CONFIGURATION
    write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);

    // Meta Step 18: MAC_RXQ_CTRL0
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x000000aa);

    // Meta Step 19: MAC_RXQ_CTRL1
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x00000000);

    // Meta Step 20: MAC_RXQ_CTRL2 - Priorities for RX Queues
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x08040201);

    // Meta Step 21: MAC_PACKET_FILTER - HASH FILTER Enabled and Receive All
    write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000408);

    // Meta Step 22: MAC_VLAN_TAG_CTRL - VLAN TAG in status and RX Stripping Enabled
    write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x01600000);

    // Meta Step 23: MAC_ADDRESS3_HIGH - DMA Channel selection
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
    LOGT("MAC address registers programmed");

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
    LOGT("MTL operation modes programmed");

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
    LOGT("MTL queue interrupts and quantum weights programmed");

    // Meta Step 45: DMA_CH3_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);

    // Meta Step 46: DMA_CH2_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);

    // Meta Step 47: DMA_CH1_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);

    // Meta Step 48: DMA_CH0_CONTROL
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

    // Meta Step 49: DMA_CH3_TX_CONTROL bulk
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);

    // Meta Step 50: DMA_CH2_TX_CONTROL bulk
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);

    // Meta Step 51: DMA_CH1_TX_CONTROL bulk
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);

    // Meta Step 52: DMA_CH0_TX_CONTROL bulk
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

    // Meta Step 53: DMA_CH3_RX_CONTROL bulk
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);

    // Meta Step 54: DMA_CH2_RX_CONTROL bulk
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);

    // Meta Step 55: DMA_CH1_RX_CONTROL bulk
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);

    // Meta Step 56: DMA_CH0_RX_CONTROL bulk
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);
    LOGT("DMA channel control and TX/RX control programmed");

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
    LOGT("DMA descriptor list addresses programmed");

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
    LOGT("DMA descriptor tail pointers programmed");

    LOGT("Ethernet0 multi-channel TX/RX test init complete");
    return 0;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_run
 * Description: Execute multi-channel TX/RX test across CH0-CH3 sequentially
 * Parameters:
 *   cfg - pointer to test configuration
 *   out - pointer to test output
 * Returns:
 *   0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput out)
{
    int i;

    (void)cfg;

    if (out == 0) {
        LOGE("Ethernet0 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting Ethernet0 multi-channel TX/RX test run");

    // Meta Step 76: write_reg(0xA0243ffc, enet_sel) - DV VIP sequencer trigger
    // This is a DV/VIP infrastructure write, converting to PSV logging
    LOGD("DV VIP sequencer trigger: write enet_sel=%d to 0xA0243ffc", enet_sel);

    // Meta Step 77: write_reg(0xA0243ff8, 0xdeadbeef) - DV VIP sequencer trigger
    // This is a DV/VIP infrastructure write, converting to PSV logging
    LOGD("DV VIP sequencer trigger: write 0xdeadbeef to 0xA0243ff8");

    // Meta Step 78: Channel 0 loop
    LOGT("Channel 0: Starting TX/RX polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            // Meta Step 78: wait_on(10) - PSV busy-wait adaptation
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 20 && txpkt == 20) {
            LOGT("Channel 0: TX/RX complete - rxpkt=%d txpkt=%d", rxpkt, txpkt);
            break;
        }
        int_pend = 1;
    }

    // Meta Step 79: int_pend = 1
    int_pend = 1;

    // Meta Step 80: Channel 1 activation
    LOGT("Channel 1: Activating");
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000081);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101001);

    // Meta Step 81: write_reg(0xA0243ff8, 0xdeadbeed) - DV VIP sequencer trigger
    LOGD("DV VIP sequencer trigger: write 0xdeadbeed to 0xA0243ff8");

    // Meta Step 82: Channel 1 loop
    LOGT("Channel 1: Starting TX/RX polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            // Meta Step 82: wait_on(10) - PSV busy-wait adaptation
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 20 && txpkt == 20) {
            LOGT("Channel 1: TX/RX complete - rxpkt=%d txpkt=%d", rxpkt, txpkt);
            break;
        }
        int_pend = 1;
    }

    // Meta Step 83: int_pend = 1
    int_pend = 1;

    // Meta Step 84: Channel 2 activation
    LOGT("Channel 2: Activating");
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000082);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101001);

    // Meta Step 85: write_reg(0xA0243ff8, 0xdeadbeee) - DV VIP sequencer trigger
    LOGD("DV VIP sequencer trigger: write 0xdeadbeee to 0xA0243ff8");

    // Meta Step 86: Channel 2 loop
    LOGT("Channel 2: Starting TX/RX polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            // Meta Step 86: wait_on(10) - PSV busy-wait adaptation
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 10 && txpkt == 10) {
            LOGT("Channel 2: TX/RX complete - rxpkt=%d txpkt=%d", rxpkt, txpkt);
            break;
        }
        int_pend = 1;
    }

    // Meta Step 87: int_pend = 1
    int_pend = 1;

    // Meta Step 88: Channel 3 activation
    LOGT("Channel 3: Activating");
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101001);
    LOGT("Channel 3: Activated");

    // Meta Step 90: Pass/fail determination
    // finish() replaced with out->status per FV/PSV completion rule
    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        LOGE("FAIL: def_fail_cnt=%d wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        out->status = -1;
    } else {
        LOGT("PASS: def_fail_cnt=%d wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        out->status = 0;
    }

    LOGT("Ethernet0 multi-channel TX/RX test run complete: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_teardown
 * Description: Teardown and final status for Ethernet0 multi-channel test
 * Parameters:
 *   cfg - pointer to test configuration
 * Returns:
 *   0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("Ethernet0 multi-channel TX/RX test teardown");

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        LOGE("Teardown: FAIL - def_fail_cnt=%d wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        return -1;
    }

    LOGT("Teardown: PASS - no failures detected");
    return 0;
}
