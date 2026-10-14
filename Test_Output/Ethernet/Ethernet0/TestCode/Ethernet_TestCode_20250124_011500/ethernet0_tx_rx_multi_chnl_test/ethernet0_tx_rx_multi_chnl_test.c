// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

// Global declarations per Meta Step 9
int data_rd;
int data_wr;
int def_fail_cnt = 0;
int wr_fail_cnt = 0;
extern int int_pend;
extern int rxpkt;
extern int txpkt;
extern int enet_sel;
extern int enet_select;

// MANUAL_REVIEW: ATP Runtime Condition - External Ethernet Validation:
// The ATP (Section 9.19) requires an external endpoint (Traffic Generator / Ethernet Switch)
// connected via RJ45 cable with External PHY Loopback. Hardware topology must be defined
// and communication path documented. Ensure the external loopback or switch is connected
// before running this test.

// MANUAL_REVIEW: ATP Runtime Condition - End-to-End Ethernet Validation:
// The ATP requires validation of TX frame, RX frame, payload integrity, source and
// destination MAC addresses, and external packet verification (e.g., Wireshark capture).
// The DV-derived Meta testcase relies on TX/RX packet counters (rxpkt/txpkt) for PASS
// criteria. Additional payload integrity and MAC address validation should be added
// per ATP requirements. Do not rely solely on TX/RX counters for PASS criteria.

// MANUAL_REVIEW: ATP Runtime Condition - Duplex Validation:
// The ATP requires distinct configurations for Half-Duplex and Full-Duplex with
// readback/status checks to verify the configured mode. The Meta testcase uses
// compile-time macros (ETH_10M, ETH_100M, HALF_DUPLEX) to select duplex mode but
// does not perform readback verification of the configured duplex mode. Add
// MAC_CONFIGURATION readback and verification after writing the speed/duplex setting.

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for Ethernet0 DMA channels per Meta Step 89.
 *             Clears int_pend, reads DMA channel status registers, and updates
 *             TX descriptor tail pointers for CH2 and CH3.
 * Parameters:
 *   None
 * Returns:
 *   void
 /
void Default_IRQHandler(void)
{
    int rdata;
    int wdata;
    unsigned int irq_no;
    int rdata_tailpointer;
    int rdata_desc;

    // Meta Step 89: int_pend = 0
    int_pend = 0;

    // Meta Step 89: irq_no = 42 + enet_select
    irq_no = 42 + enet_select;

    // Meta Step 89: enet_sel = enet_select
    enet_sel = enet_select;

    // Meta Step 89: printf replaced with LOGT for PSV
    LOGT("IN THE HANDLER %d for eth%d", irq_no, enet_select);

    // Meta Step 89: Read DMA CH0 current app RX descriptor
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);

    // Meta Step 89: Read DMA CH0 current app RX buffer
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXBUFFER);

    // Meta Step 89: Read DMA CH3 current app TX descriptor
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);

    // Meta Step 89: Read DMA CH3 current app TX buffer
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);

    // Meta Step 89: Write DMA CH2 TX descriptor tail pointer
    write_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE60298a4);

    // Meta Step 89: Write DMA CH3 TX descriptor tail pointer
    write_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE60398a4);

    (void)wdata;
    (void)rdata_tailpointer;
    (void)rdata_desc;
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_init
 * Description: Initializes Ethernet0 multi-channel TX/RX test. Selects Ethernet
 *             interface, configures MAC layer (speed, duplex, addresses, filters),
 *             MTL layer (queue modes, weights, DMA mapping, interrupts), and DMA
 *             layer (channel control, TX/RX control, descriptor addresses, tail pointers).
 * Parameters:
 *   cfg - pointer to test configuration structure
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem cfg)
{
    (void)cfg;

    LOGT("Ethernet0 multi-channel TX/RX test init started");

    // Meta Step 12: Set trns_count if ETH_10M
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

    // MANUAL_REVIEW: ATP Duplex Validation - Add readback of MAC_CONFIGURATION
    // to verify the configured speed/duplex mode per ATP requirement.

    // Meta Step 15: Enabling Override
    write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08);

    // Meta Step 16: wait_on(300) - PSV busy-wait adaptation
    for (volatile int d = 0; d < 300000; d++);

    // Meta Step 17: MAC Extended Configuration
    write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);

    // Meta Step 18: MAC RXQ Control 0
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x000000aa);

    // Meta Step 19: MAC RXQ Control 1
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x00000000);

    // Meta Step 20: MAC RXQ Control 2 - Priorities for RX Queues
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x08040201);

    // Meta Step 21: MAC Packet Filter - Hash Filter + Receive All
    write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000408);

    // Meta Step 22: MAC VLAN Tag Control - VLAN TAG in status, RX stripping
    write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x01600000);

    // Meta Steps 23-26: MAC Address HIGH registers with DMA channel selection
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x80033607);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x80022607);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x80011607);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x80000607);

    // Meta Steps 27-29: MAC Address LOW registers
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x08090a0b);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x08090a0b);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x08090a0b);

    // Meta Step 30: MTL Operation Mode
    write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000);

    // Meta Steps 31-33: MTL TX Queue Operation Modes
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x0003000a);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x0003000a);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x0003000a);

    // Meta Steps 34-37: MTL RX Queue Operation Modes
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE, 0x00f00033);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE, 0x00f00033);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE, 0x00f00033);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE, 0x00f00033);

    // Meta Steps 38-40: MTL TX Queue Quantum Weights
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000014);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000014);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005);

    // Meta Step 41: MTL RXQ DMA Map0 - initial mapping
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);

    // Meta Steps 42-44: MTL Queue Interrupt Control Status
    write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100);
    write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100);
    write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100);

    // Meta Steps 45-48: DMA Channel Control registers
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

    // Meta Steps 49-52: DMA TX Control - initial bulk configuration
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

    // Meta Steps 53-56: DMA RX Control - initial bulk configuration
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

    // Meta Step 57: DMA CH0 RX Control2
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x9);

    // Meta Step 58: DMA CH0 Interrupt Enable
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0xf0df);

    // Meta Step 59: DMA Mode
    write_enet_reg(mizar_ETHERNET0_DMA_MODE, 0x00000000);

    // Meta Steps 60-63: DMA TX Descriptor List Addresses
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0xE6030000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0xE6020000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0xE6010000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);

    // Meta Steps 64-67: DMA RX Descriptor List Addresses
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0xE6070000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0xE6060000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0xE6050000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);

    // Meta Steps 68-71: DMA TX Descriptor Tail Pointers
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE6039EB4);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE6029EB4);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0xE6019EB4);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4);

    // Meta Steps 72-75: DMA RX Descriptor Tail Pointers
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040010);

    LOGT("Ethernet0 multi-channel TX/RX test init complete");

    return 0;
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_run
 * Description: Executes multi-channel TX/RX test. Triggers VIP sequencer,
 *             runs Channel 0 through Channel 3 sequentially with interrupt-driven
 *             polling loops. Each channel activation updates DMA TX control,
 *             MTL RXQ DMA mapping, RX descriptor tail pointer, and RX control.
 * Parameters:
 *   cfg - pointer to test configuration structure
 *   out - pointer to test output structure
 * Returns:
 *   FV/template-compatible status.
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

    // Meta Step 76: VIP sequencer trigger - write enet_sel to 0xA0243ffc
    // MANUAL_REVIEW: write_reg(0xA0243ffc, enet_sel) is a DV/VIP sequencer trigger.
    // This is NOT the debug marker write_reg(0xA0243ffc, 0xdeadbee0).
    // Preserving as DV/VIP infrastructure - may need PSV adaptation.
    LOGD("DV VIP sequencer trigger: writing enet_sel to 0xA0243ffc");
    write_reg(0xA0243ffc, enet_sel);

    // Meta Step 77: VIP sequencer trigger - write 0xdeadbeef to 0xA0243ff8
    // MANUAL_REVIEW: write_reg(0xA0243ff8, 0xdeadbeef) is a DV/VIP sequencer trigger.
    // Preserving as DV/VIP infrastructure - may need PSV adaptation.
    LOGD("DV VIP sequencer trigger: writing 0xdeadbeef to 0xA0243ff8");
    write_reg(0xA0243ff8, 0xdeadbeef);

    // Meta Step 78: Channel 0 loop
    LOGT("Channel 0: Starting TX/RX polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 20 && txpkt == 20) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 0: Loop exited at iteration %d, rxpkt=%d, txpkt=%d", i, rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - Channel 0: Validate TX/RX frame
    // payload integrity and MAC addresses, not just rxpkt/txpkt counters.

    // Meta Step 79: int_pend = 1
    int_pend = 1;

    // Meta Step 80: Channel 1 activation
    LOGT("Channel 1: Activating");
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000081);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101001);

    // Meta Step 81: VIP sequencer trigger for Channel 1
    // MANUAL_REVIEW: write_reg(0xA0243ff8, 0xdeadbeed) is a DV/VIP sequencer trigger.
    LOGD("DV VIP sequencer trigger: writing 0xdeadbeed to 0xA0243ff8");
    write_reg(0xA0243ff8, 0xdeadbeed);

    // Meta Step 82: Channel 1 loop
    LOGT("Channel 1: Starting TX/RX polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 20 && txpkt == 20) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 1: Loop exited at iteration %d, rxpkt=%d, txpkt=%d", i, rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - Channel 1: Validate TX/RX frame
    // payload integrity and MAC addresses, not just rxpkt/txpkt counters.

    // Meta Step 83: int_pend = 1
    int_pend = 1;

    // Meta Step 84: Channel 2 activation
    LOGT("Channel 2: Activating");
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000082);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101001);

    // Meta Step 85: VIP sequencer trigger for Channel 2
    // MANUAL_REVIEW: write_reg(0xA0243ff8, 0xdeadbeee) is a DV/VIP sequencer trigger.
    LOGD("DV VIP sequencer trigger: writing 0xdeadbeee to 0xA0243ff8");
    write_reg(0xA0243ff8, 0xdeadbeee);

    // Meta Step 86: Channel 2 loop
    LOGT("Channel 2: Starting TX/RX polling loop");
    for (i = 0; i < 20; i++) {
        while (int_pend) {
            for (volatile int d = 0; d < 10000; d++);
        }
        if (rxpkt == 10 && txpkt == 10) {
            break;
        }
        int_pend = 1;
    }
    LOGT("Channel 2: Loop exited at iteration %d, rxpkt=%d, txpkt=%d", i, rxpkt, txpkt);

    // MANUAL_REVIEW: ATP End-to-End Validation - Channel 2: Validate TX/RX frame
    // payload integrity and MAC addresses, not just rxpkt/txpkt counters.

    // Meta Step 87: int_pend = 1
    int_pend = 1;

    // Meta Step 88: Channel 3 activation
    LOGT("Channel 3: Activating");
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101001);

    // MANUAL_REVIEW: ATP End-to-End Validation - Channel 3: Channel 3 is activated
    // but Meta does not include a polling loop for CH3 completion. Validate CH3
    // packet processing per ATP requirements.

    // Meta Step 90: Pass/fail determination - finish() replaced with out->status
    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        LOGE("Test FAILED: def_fail_cnt=%d, wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        out->status = -1;
    } else {
        LOGT("Test PASSED: def_fail_cnt=%d, wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
        out->status = 0;
    }

    LOGT("Ethernet0 multi-channel TX/RX test run complete: %s",
         (out->status == 0) ? "PASS" : "FAIL");

    return out->status;
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_teardown
 * Description: Teardown for Ethernet0 multi-channel TX/RX test.
 *             Reports final failure counters and returns status.
 * Parameters:
 *   cfg - pointer to test configuration structure
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("Ethernet0 teardown: def_fail_cnt=%d, wr_fail_cnt=%d",
         def_fail_cnt, wr_fail_cnt);

    return (def_fail_cnt == 0 && wr_fail_cnt == 0) ? 0 : -1;
}
