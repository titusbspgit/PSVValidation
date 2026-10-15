// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

// MANUAL_REVIEW: ATP requires external endpoint identification for Ethernet validation.
// ATP Hardware topology: External PHY Loopback, RJ45 Ethernet cable, network switch,
// UART USB cable, JTAG debugger, oscilloscope. Ensure hardware is connected per ATP 9.19.

// MANUAL_REVIEW: ATP requires end-to-end Ethernet validation including:
// - TX frame validation
// - RX frame validation
// - Payload integrity validation
// - Source and destination MAC address validation
// - External packet verification (e.g., Wireshark)
// The Meta DV testcase does not include explicit frame/payload/MAC validation logic.
// These validations must be added or verified externally per ATP requirements.

// MANUAL_REVIEW: ATP requires duplex mode readback/status verification.
// Meta configures duplex via MAC_Configuration register writes but does not
// include a readback to verify the configured mode. Add readback verification
// per ATP 9.19 requirements.

static unsigned int g_errors;

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_init
 * Description: Initialize Ethernet0 multi-channel TX/RX DMA test.
 * Selects Ethernet interface, configures MAC for speed/duplex,
 * programs MAC, MTL, and DMA registers.
 * Parameters:
 * cfg - pointer to test configuration item
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem cfg)
{
 (void)cfg;

 g_errors = 0;

 LOGT("Ethernet0 multi-channel TX/RX DMA test init");

 // Step 1: Ethernet interface selection via conditional compilation
#ifdef SEL_ENET0
 enet_intf_sel(0);
#elif SEL_ENET1
 enet_intf_sel(1);
#elif SEL_ENET2
 enet_intf_sel(2);
#elif SEL_ENET3
 enet_intf_sel(3);
#else
 enet_intf_sel(4);
#endif

 LOGT("Ethernet interface selected");

 // Step 4: MAC Configuration based on speed and duplex
#ifdef ETH_100M
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xC003);
 LOGT("MAC_Configuration: 100M Half-Duplex (0xC003)");
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xE003);
 LOGT("MAC_Configuration: 100M Full-Duplex (0xE003)");
#endif
#endif

#ifdef ETH_10M
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x8003);
 LOGT("MAC_Configuration: 10M Half-Duplex (0x8003)");
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xA003);
 LOGT("MAC_Configuration: 10M Full-Duplex (0xA003)");
#endif
#endif

 // Step 5: Override enable
 write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08);
 LOGT("Override enable register written with 0x1A08");

 // Step 6: wait_on(300) - PSV busy-wait adaptation
 for (volatile int d = 0; d < 300000; d++);
 LOGT("Wait after override enable completed");

 // Step 7: MAC_Ext_Configuration
 write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);
 LOGT("MAC_Ext_Configuration: 0x0");

 // Step 8: MAC_Packet_Filter
 write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000408);
 LOGT("MAC_Packet_Filter: 0x80000408");

 // Step 9: MAC_VLAN_Tag_Ctrl
 write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x01600000);
 LOGT("MAC_VLAN_Tag_Ctrl: 0x01600000");

 // Step 10: MAC_RxQ_Ctrl0
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x000000aa);
 LOGT("MAC_RxQ_Ctrl0: 0x000000aa");

 // Step 11: MAC_RxQ_Ctrl1
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x00000000);
 LOGT("MAC_RxQ_Ctrl1: 0x00000000");

 // Step 12: MAC_RxQ_Ctrl2
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x08040201);
 LOGT("MAC_RxQ_Ctrl2: 0x08040201");

 // Step 13: MMC_IPC_Rx_Interrupt_Mask
 write_enet_reg(mizar_ETHERNET0_MMC_IPC_RX_INTERRUPT_MASK, 0x80000400);
 LOGT("MMC_IPC_Rx_Interrupt_Mask: 0x80000400");

 // Step 14: MTL_Operation_Mode
 write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000);
 LOGT("MTL_Operation_Mode: 0x00000000");

 // Step 15: MTL_TxQ2_Operation_Mode
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x0003000a);
 LOGT("MTL_TxQ2_Operation_Mode: 0x0003000a");

 // Step 16: MTL_TxQ1_Operation_Mode
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x0003000a);
 LOGT("MTL_TxQ1_Operation_Mode: 0x0003000a");

 // Step 17: MTL_TxQ0_Operation_Mode
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x0003000a);
 LOGT("MTL_TxQ0_Operation_Mode: 0x0003000a");

 // Step 18: MTL_TxQ3_Quantum_Weight
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000014);
 LOGT("MTL_TxQ3_Quantum_Weight: 0x00000014");

 // Step 19: MTL_TxQ2_Quantum_Weight
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000014);
 LOGT("MTL_TxQ2_Quantum_Weight: 0x00000014");

 // Step 20: MTL_TxQ1_Quantum_Weight
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005);
 LOGT("MTL_TxQ1_Quantum_Weight: 0x00000005");

 // Step 21: MTL_RxQ_DMA_Map0
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);
 LOGT("MTL_RxQ_DMA_Map0: 0x00000083");

 // Step 22: DMA_Mode
 write_enet_reg(mizar_ETHERNET0_DMA_MODE, 0x00000000);
 LOGT("DMA_Mode: 0x00000000");

 // Step 23: DMA_CH0_RX_Control2
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x9);
 LOGT("DMA_CH0_RX_Control2: 0x9");

 // Step 24: DMA_CH0_Interrupt_Enable
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0xf0df);
 LOGT("DMA_CH0_Interrupt_Enable: 0xf0df");

 // Step 25: DMA_CH3_TX_Control
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);
 LOGT("DMA_CH3_TX_Control: 0x00100006");

 // Step 26: DMA_CH2_TX_Control
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);
 LOGT("DMA_CH2_TX_Control: 0x00100006");

 // Step 27: DMA_CH1_TX_Control
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);
 LOGT("DMA_CH1_TX_Control: 0x00100006");

 // Step 28: DMA_CH0_TX_Control
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);
 LOGT("DMA_CH0_TX_Control: 0x00100006");

 // Step 29: DMA_CH0_RX_Control
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);
 LOGT("DMA_CH0_RX_Control: 0x00101000");

 // Step 30: DMA_CH3_TxDesc_List_Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00300000);
 LOGT("DMA_CH3_TxDesc_List_Address: 0x00300000");

 // Step 31: DMA_CH2_TxDesc_List_Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00200000);
 LOGT("DMA_CH2_TxDesc_List_Address: 0x00200000");

 // Step 32: DMA_CH1_TxDesc_List_Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00100000);
 LOGT("DMA_CH1_TxDesc_List_Address: 0x00100000");

 // Step 33: DMA_CH0_TxDesc_List_Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);
 LOGT("DMA_CH0_TxDesc_List_Address: 0xE6000000");

 // Step 34: DMA_CH0_RxDesc_List_Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);
 LOGT("DMA_CH0_RxDesc_List_Address: 0xE6040000");

 // Step 35: DMA_CH1_RxDesc_List_Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0xE6050000);
 LOGT("DMA_CH1_RxDesc_List_Address: 0xE6050000");

 // Step 36: DMA_CH2_RxDesc_List_Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0xE6060000);
 LOGT("DMA_CH2_RxDesc_List_Address: 0xE6060000");

 // Step 37: DMA_CH3_RxDesc_List_Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0xE6070000);
 LOGT("DMA_CH3_RxDesc_List_Address: 0xE6070000");

 LOGT("Ethernet0 multi-channel TX/RX DMA test init complete");

 return 0;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_run
 * Description: Execute multi-channel TX/RX DMA data transfer on Ethernet0.
 * Preloads RX descriptors, starts TX on CH0-CH3 and RX on CH0,
 * writes TX descriptor tail pointers, polls for transfer completion.
 * Parameters:
 * cfg - pointer to test configuration item
 * out - pointer to test output structure
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned int i;
 unsigned int rdata_tailpointer;
 unsigned int rdata_desc;
 extern int int_pend;

 (void)cfg;

 if (out == 0) {
 LOGE("Ethernet0 output pointer is NULL");
 return -1;
 }

 out->status = 0;

 // Step 2: Variable declarations
 unsigned int trns_count = 10;

 // Step 3: Adjust transfer count for 10M speed
#ifdef ETH_10M
 trns_count = 6;
#endif

 LOGT("Transfer count set to %u", trns_count);

 // Step 38: Preload RX descriptors
 preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0x10);
 LOGT("RX descriptors preloaded");

 // Step 39: Start TX on all channels
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100007);
 LOGT("DMA_CH0_TX_Control start TX: 0x00100007");

 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
 LOGT("DMA_CH1_TX_Control start TX: 0x00100007");

 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
 LOGT("DMA_CH2_TX_Control start TX: 0x00100007");

 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100007);
 LOGT("DMA_CH3_TX_Control start TX: 0x00100007");

 // Step 40: Start RX on CH0
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101001);
 LOGT("DMA_CH0_RX_Control start RX: 0x00101001");

 // Step 41: Write TX descriptor tail pointers
 write_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE60298a4);
 LOGT("DMA_CH2_TxDesc_Tail_Pointer: 0xE60298a4");

 write_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE60398a4);
 LOGT("DMA_CH3_TxDesc_Tail_Pointer: 0xE60398a4");

 // Step 42: Commented out RX descriptor tail pointers for CH1-CH3 (preserved as in source)
 //write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6079EB4);
 //write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6069EB4);
 //write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6059EB4);

 // Step 43: Transfer loop
 for (i = 0; i < trns_count; i++) {
 LOGT("Transfer iteration %u of %u", i, trns_count);

 // Step 44: Poll int_pend with busy-wait (PSV adaptation of wait_on(10))
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 }
 LOGT("int_pend cleared for iteration %u", i);

 // Step 45: Read DMA_CH0_RxDesc_Tail_Pointer
 rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER);
 LOGT("DMA_CH0_RxDesc_Tail_Pointer read: 0x%08x", rdata_tailpointer);

 // Step 46: Compare rdata_tailpointer with rdata_desc
 if (rdata_tailpointer == rdata_desc) {
 LOGT("RX tail pointer matches expected descriptor address");

 // Step 47: Check if tail pointer reached end of ring
 if (rdata_tailpointer == 0xE60400f0) {
 LOGT("RX tail pointer reached end of ring (0xE60400f0)");

 // Step 48-49: Conditionally reload RX descriptors
#ifdef PRELOAD_AGAIN
 preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0x10);
 LOGT("RX descriptors reloaded under PRELOAD_AGAIN");
#endif
 }
 }

 // Step 50: Optional debug display
#ifdef DEBUG_DISPLAY
 LOGT("DEBUG: iteration=%u rdata_tailpointer=0x%08x rdata_desc=0x%08x",
 i, rdata_tailpointer, rdata_desc);
#endif
 }

 LOGT("All %u transfer iterations completed", trns_count);

 // MANUAL_REVIEW: ATP requires end-to-end TX/RX frame validation, payload integrity,
 // and source/destination MAC address validation. The Meta DV testcase relies on
 // interrupt pending polling and tail pointer checks. Add explicit frame/payload
 // validation per ATP 9.19 requirements.

 // MANUAL_REVIEW: ATP requires duplex mode readback verification.
 // Add MAC_Configuration readback to verify configured duplex mode per ATP 9.19.

 out->status = (g_errors == 0U) ? 0 : -1;

 LOGT("Run complete: %s errors=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 g_errors);

 return out->status;
}

/*
 * Function: ethernet0_tx_rx_multi_chnl_test_teardown
 * Description: Teardown for Ethernet0 multi-channel TX/RX DMA test.
 * Parameters:
 * cfg - pointer to test configuration item
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("Ethernet0 multi-channel TX/RX DMA test teardown");
 return g_errors == 0U ? 0 : -1;
}
