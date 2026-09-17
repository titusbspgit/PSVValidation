import json
import os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'Ethernet_TestPlan_{timestamp}.xlsx'

json_data = [
    {
        "Index": "1",
        "SS / Module": "Ethernet",
        "Test Case Name": "ethernet0_reg_wr_rd_test",
        "Feature": "Register Write-Read Verification",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_define.c\"; <test_common.h>; <ethernet0/ethernet0_def.h>; <ethernet0/ethernet0_offset.h>",
        "Meta Macros": "SOFT_RST_REG_ADDRESS; SOFT_RST_REG_DATA; CNT",
        "Meta Arrays": "addr_array[434]; default_value_array[434]; read_mask_array[434]; write_mask_array[434]; skip_array[434]; skip_rst_array[434]; chk_val[6]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs register default-value verification and write-read verification for Ethernet0 MAC registers. The test_case() function first calls chk_rst_val() which iterates over addr_array (containing mizar_ETHERNET0_MAC_CONFIGURATION, mizar_ETHERNET0_MAC_EXT_CONFIGURATION, mizar_ETHERNET0_MAC_PACKET_FILTER, mizar_ETHERNET0_MAC_WD_JB_TIMEOUT, mizar_ETHERNET0_MAC_HASH_TABLE_REG0) and reads each register using read_reg(addr). For each register, the read value is compared against the corresponding entry in default_value_array. Registers with read_mask_array[i] == 0x00000000 or skip_rst_array[i] == 1 are skipped. Any mismatch increments def_fail_cnt. Then chk_rd_wr() is called, which iterates over 6 test patterns in chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}. For each pattern, it writes data_wr to each register using write_reg(addr, data_wr), skipping entries where skip_array[i] == 1 or write_mask_array[i] == 0x00000000. Then it reads back each register using read_reg(addr), computing the expected value as exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = (write_mask_array[i] ^ 0xffffffff). If data_rd != exp_val, wr_fail_cnt is incremented. Finally, if def_fail_cnt > 0 or wr_fail_cnt > 0, finish(1) is called (fail); otherwise finish(0) is called (pass). The soft_reset_chk() function is commented out and not executed.",
        "Test Description": "This test verifies the default reset values and write-read integrity of Ethernet0 MAC registers including MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_WD_JB_Timeout, and MAC_Hash_Table_Reg0. First, each register is read and its value is compared against the expected default value. Then, a series of six predefined data patterns are written to each writable register and read back. The read-back value is validated against an expected value computed using the write mask, read mask, and default value for each register. The test passes only if all default value checks and all write-read checks succeed for every register across all data patterns.",
        "Meta Test Steps / Procedure": "1. test_case() is called as the entry point. 2. chk_rst_val() is invoked: iterates i from 0 to CNT-1 over addr_array[i]. 3. For each register address addr = addr_array[i]: if read_mask_array[i] == 0x00000000, skip (not readable). If skip_rst_array[i] == 1, skip. Otherwise call data_rd = read_reg(addr). 4. Compare data_rd against default_value_array[i]. If mismatch, increment def_fail_cnt and print failure message. If match, print pass under DEBUG_DISPLAY. 5. chk_rd_wr() is invoked: defines chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}. 6. Outer loop j from 0 to 5: set data_wr = chk_val[j]. 7. Inner write loop i from 0 to CNT-1: addr = addr_array[i]. If skip_array[i] == 1, skip. If write_mask_array[i] == 0x00000000, skip (not writable). Otherwise call write_reg(addr, data_wr). 8. Inner read loop i from 0 to CNT-1: addr = addr_array[i]. If skip_array[i] == 1, skip. If write_mask_array[i] == 0x00000000, skip. If read_mask_array[i] == 0x00000000, skip. Otherwise call data_rd = read_reg(addr). 9. Compute wr_n = (write_mask_array[i] ^ 0xffffffff). Compute exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). 10. Compare data_rd against exp_val. If mismatch, increment wr_fail_cnt and print failure. If match, print pass under DEBUG_DISPLAY. 11. After all iterations, check if def_fail_cnt > 0 or wr_fail_cnt > 0. If true, call finish(1) indicating test failure. Otherwise call finish(0) indicating test pass.",
        "Test Steps / Procedure": "1. Read each Ethernet0 MAC register and verify that its value matches the expected default reset value. Skip registers that are not readable or are marked to be skipped. 2. For each of six predefined test data patterns, write the pattern to each writable Ethernet0 MAC register. 3. After writing each pattern, read back each register and compute the expected value using the register's write mask, read mask, and default value. 4. Compare the read-back value against the computed expected value for each register. 5. Record any mismatches encountered during default value checks or write-read checks. 6. If all default value checks and all write-read checks pass for all registers across all patterns, report the test as passed; otherwise report the test as failed.",
        "Meta Impacted Registers": "mizar_ETHERNET0_MAC_CONFIGURATION; mizar_ETHERNET0_MAC_EXT_CONFIGURATION; mizar_ETHERNET0_MAC_PACKET_FILTER; mizar_ETHERNET0_MAC_WD_JB_TIMEOUT; mizar_ETHERNET0_MAC_HASH_TABLE_REG0",
        "Impacted Registers": "MAC_Configuration; MAC_Ext_Configuration; MAC_Packet_Filter; MAC_WD_JB_Timeout; MAC_Hash_Table_Reg0",
        "Meta Validation / Acceptance Criteria": "In chk_rst_val(): For each register, data_rd = read_reg(addr) is compared against default_value_array[i]. If data_rd != default_value_array[i], def_fail_cnt is incremented. In chk_rd_wr(): For each of 6 test patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000), after write_reg(addr, data_wr) and data_rd = read_reg(addr), the expected value is computed as exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = (write_mask_array[i] ^ 0xffffffff). If data_rd != exp_val, wr_fail_cnt is incremented. Final pass condition: def_fail_cnt == 0 AND wr_fail_cnt == 0 results in finish(0) (pass). Any non-zero failure count results in finish(1) (fail).",
        "Validation / Acceptance Criteria": "The test passes if all of the following conditions are met: 1. Every readable MAC register (MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_WD_JB_Timeout, MAC_Hash_Table_Reg0) returns its expected default reset value when read after reset. 2. For each of six test data patterns written to each writable register, the read-back value matches the expected value computed using the write mask, read mask, and default value of that register. 3. No default-value mismatches and no write-read mismatches are detected across all registers and all patterns. If any mismatch is detected, the test fails.",
        "Remarks": "The test uses six distinct data patterns (all-ones, alternating bits, inverted alternating bits, all-zeros, mixed pattern, and upper-half pattern) to exercise different bit combinations. The soft_reset_chk() function is present in source but is commented out and not executed. Registers are skipped based on skip_array and skip_rst_array flags, and non-readable or non-writable registers are automatically excluded from the respective checks. The addr_array is declared with size 434 but only 5 register entries are populated in the source."
    },
    {
        "Index": "2",
        "SS / Module": "Ethernet",
        "Test Case Name": "ethernet0_rx_basic_test",
        "Feature": "Basic RX Packet Reception",
        "Meta Headers": "<stdio.h>; <stdlib.h>; <math.h>; <ethernet0/ethernet0_def.h>; <test_common.h>; <ethernet0/ethernet0_offset.h>; <ethernet0/ethernet0_funcs.h>; <ethernet0/ethernet0_programming_sequence.h>",
        "Meta Macros": "ETH_10M; ETH_100M; HALF_DUPLEX; SEL_ENET0; SEL_ENET1; SEL_ENET2; SEL_ENET3; DEBUG_DISPLAY",
        "Meta Arrays": "NA",
        "Speed": "ETH_10M; ETH_100M",
        "Mode": "HALF_DUPLEX; Full Duplex",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "",
        "Test Description": "This test verifies basic Ethernet RX packet reception on Ethernet0 using interrupt-driven DMA transfers. The test configures the MAC layer including configuration, RX queue control, VLAN tag control, packet filter, and MAC addresses for four address slots. The MTL layer is configured with TX and RX queue operation modes, queue sizes of 4096 bytes, store-and-forward mode, quantum weights, queue-to-DMA channel mapping, and interrupt control for overflow and underflow. The DMA layer is configured across all four channels (CH0-CH3) including TX and RX control with 16-beat burst length, descriptor list addresses, descriptor ring lengths of 32, RX buffer sizes, system bus mode with outstanding request limits, and interrupt enables. RX descriptors are preloaded and the RX path is started by writing to the RX descriptor tail pointer and enabling the RX start bit. An external VIP sequencer is triggered to send packets. The test waits in a loop for a configurable number of transfer-complete interrupts. The interrupt handler reads current application RX descriptor and RX buffer addresses for all four DMA channels, reads the DMA interrupt status, clears all DMA channel status registers by writing all-ones, and clears the GIC interrupt. The test passes after all expected transfer-complete interrupts are received.",
        "Meta Test Steps / Procedure": "",
        "Test Steps / Procedure": "1. Enable GIC interrupts for Ethernet0 interrupt handling. 2. Configure the Ethernet speed (10M, 100M, or 1G) and interface selection. 3. Configure the MAC Configuration register to enable TX and RX with the selected speed and duplex mode. 4. Configure MAC RX queue control registers to enable queues for DCB/Generic with appropriate priorities. 5. Configure MAC VLAN tag control to enable VLAN tag in status with RX stripping. 6. Configure MAC packet filter to enable hash filtering and receive-all mode. 7. Program four MAC address slots (Address0 through Address3) with high and low address values and DMA channel selection. 8. Configure MAC extended configuration. 9. Configure MTL TX queue operation modes for all four queues with 4096-byte queue size and store-and-forward mode, enabling Queue 0. 10. Program MTL TX quantum weights for queue scheduling. 11. Configure MTL RX queue operation modes for all four queues with store-and-forward and error packet forwarding, then set 4096-byte RX queue sizes. 12. Map RX queues to corresponding DMA channels (Queue X to Channel X). 13. Enable MTL queue overflow and underflow interrupts for all four queues. 14. Program MTL RX queue control weights and arbitration. 15. Configure DMA channels 1-3 TX control with 16-beat burst length and channel weight. 16. Set DMA channels 1-3 TX descriptor list base addresses and ring lengths of 32 descriptors. 17. Configure DMA channels 1-3 RX control with 16-beat burst and RX buffer size. 18. Configure DMA system bus mode with outstanding request limits and burst selection. 19. Set DMA channels 1-3 RX descriptor list base addresses and ring lengths of 32 descriptors. 20. Preload RX descriptors for DMA channel 0. 21. Configure DMA channel 0 TX control, TX descriptor list address, and ring length. 22. Enable DMA interrupts for all four channels. 23. Configure DMA channel 0 RX control, RX descriptor list address, and RX ring length. 24. Enable MAC interrupts. 25. Configure DMA mode. 26. Write the RX descriptor tail pointer to start the RX DMA engine and enable RX start. 27. Trigger the external VIP sequencer to begin sending packets. 28. Wait for the expected number of transfer-complete interrupts in a polling loop. 29. In the interrupt handler, read current RX descriptor and buffer addresses for all channels, read and clear DMA interrupt status for all channels, and clear the GIC interrupt. 30. After all interrupts are received, wait for a settling period and report the test as passed.",
        "Meta Impacted Registers": "",
        "Impacted Registers": "MAC_Configuration; MAC_RxQ_Ctrl0; MAC_RxQ_Ctrl1; MAC_RxQ_Ctrl2; MAC_RxQ_Ctrl4; MAC_VLAN_Tag_Ctrl; MAC_Packet_Filter; MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MAC_Ext_Configuration; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_TxQ3_Quantum_Weight; MTL_TxQ2_Quantum_Weight; MTL_TxQ1_Quantum_Weight; MTL_Operation_Mode; MTL_RxQ3_Operation_Mode; MTL_RxQ2_Operation_Mode; MTL_RxQ1_Operation_Mode; MTL_TxQ0_Quantum_Weight; MTL_RxQ0_Operation_Mode; MTL_RxQ_DMA_Map0; MTL_Q3_Interrupt_Control_Status; MTL_Q2_Interrupt_Control_Status; MTL_Q1_Interrupt_Control_Status; MTL_RxQ3_Control; MTL_RxQ2_Control; MTL_RxQ1_Control; DMA_CH3_Tx_Control; DMA_CH2_Tx_Control; DMA_CH1_Tx_Control; DMA_CH3_TxDesc_List_Address; DMA_CH2_TxDesc_List_Address; DMA_CH1_TxDesc_List_Address; DMA_CH3_TxDesc_Ring_Length; DMA_CH2_TxDesc_Ring_Length; DMA_CH1_TxDesc_Ring_Length; MTL_Q0_Interrupt_Control_Status; DMA_CH3_Control; DMA_CH2_Control; DMA_CH1_Control; MTL_RxQ0_Control; DMA_CH3_Rx_Control; DMA_CH2_Rx_Control; DMA_CH1_Rx_Control; DMA_SysBus_Mode; DMA_CH3_RxDesc_List_Address; DMA_CH2_RxDesc_List_Address; DMA_CH1_RxDesc_List_Address; DMA_CH3_Rx_Control2; DMA_CH2_Rx_Control2; DMA_CH1_Rx_Control2; DMA_CH0_Tx_Control; DMA_CH0_TxDesc_List_Address; DMA_CH1_Interrupt_Enable; DMA_CH2_Interrupt_Enable; DMA_CH3_Interrupt_Enable; DMA_CH0_TxDesc_Ring_Length; DMA_CH0_Control; DMA_CH0_Rx_Control; DMA_CH0_RxDesc_List_Address; DMA_CH0_Rx_Control2; DMA_CH0_Interrupt_Enable; MAC_Interrupt_Enable; DMA_Mode; DMA_CH0_RxDesc_Tail_Pointer; DMA_CH0_Current_App_RxDesc; DMA_CH0_Current_App_RxBuffer; DMA_CH1_Current_App_RxDesc; DMA_CH1_Current_App_RxBuffer; DMA_CH2_Current_App_RxDesc; DMA_CH2_Current_App_RxBuffer; DMA_CH3_Current_App_RxDesc; DMA_CH3_Current_App_RxBuffer; DMA_Interrupt_Status; DMA_CH0_Status; DMA_CH1_Status; DMA_CH2_Status; DMA_CH3_Status",
        "Meta Validation / Acceptance Criteria": "",
        "Validation / Acceptance Criteria": "The test passes if all of the following conditions are met: 1. All expected transfer-complete interrupts (10 by default, or 6 for 10M speed) are received from the Ethernet DMA. 2. Each interrupt handler successfully reads the current RX descriptor and RX buffer addresses for all four DMA channels. 3. The DMA interrupt status is read and all DMA channel status registers are cleared successfully in each interrupt. 4. The GIC interrupt is cleared after each interrupt service. 5. The test completes the full interrupt polling loop without hanging or timing out. 6. After all transfers complete and a settling wait period, the test reports pass via finish(0).",
        "Remarks": "The test supports three speed configurations (10M, 100M, 1G) and two duplex modes (half duplex, full duplex) selected via compile-time preprocessor defines. The default configuration is 1G full duplex. The transfer count is reduced from 10 to 6 for 10M speed. Four DMA channels (CH0-CH3) are configured but only CH0 RX path is actively started for reception. An external VIP sequencer is triggered to send Ethernet packets. Four hardcoded hex addresses (0XE68C2058, 0xA0243ffc, 0xA0243ff8, 0XE68C2050) are used for external control and could not be mapped to named registers. The test has no explicit data payload comparison; it validates successful reception through interrupt completion only."
    },
    {
        "Index": "3",
        "SS / Module": "Ethernet",
        "Test Case Name": "ethernet0_tx_basic_test",
        "Feature": "Basic TX Packet Transmission",
        "Meta Headers": "",
        "Meta Macros": "",
        "Meta Arrays": "",
        "Speed": "",
        "Mode": "",
        "Memory Start Offset": "",
        "Memory End Offset": "",
        "Meta Test Description": "",
        "Test Description": "This test verifies basic Ethernet TX packet transmission on Ethernet0 using interrupt-driven DMA transfers. The test configures four MAC address slots with DMA channel selection and address enable bits. The MAC packet filter is set to hash filter mode with receive-all enabled. The MAC configuration register is programmed with the selected speed and duplex mode to enable both TX and RX. An external VIP sequencer is triggered. The MAC extended configuration is set. The MTL TX queue operation modes for all four queues are configured with 4096-byte queue size and store-and-forward mode. TX quantum weights are programmed for queue scheduling. MTL queue overflow and underflow interrupts are enabled for all four queues. The DMA system bus mode is configured with outstanding request limits and burst selection. TX descriptors are preloaded into memory. TX descriptor ring lengths of 10 are set for all four DMA channels. TX descriptor list base addresses and tail pointers are programmed for all four channels. DMA channel control registers are initialized. DMA interrupts are enabled for all four channels. The TX path is started on DMA channel 0 by writing to the TX control register with the start TX bit set. The test waits in a polling loop for the expected number of transfer-complete interrupts. The interrupt handler reads the DMA interrupt status, clears the DMA channel 0 status by writing all-ones, verifies the interrupt status is cleared, and clears the GIC interrupt. The test passes after all expected interrupts are received.",
        "Meta Test Steps / Procedure": "",
        "Test Steps / Procedure": "1. Enable GIC interrupts for Ethernet0 interrupt handling. 2. Configure the Ethernet speed (10M, 100M, or 1G) and interface selection. 3. Configure non-secure protection for the NIC. 4. Program four MAC address slots (Address0 through Address3) with high and low address values and DMA channel selection. 5. Configure MAC packet filter to enable hash filtering and receive-all mode. 6. Configure the MAC Configuration register to enable TX and RX with the selected speed and duplex mode. 7. Trigger the external VIP sequencer to prepare for packet transmission. 8. Configure MAC extended configuration. 9. Configure MTL TX queue operation modes for all four queues with 4096-byte queue size and store-and-forward mode. 10. Program MTL TX quantum weights for queue scheduling on all four queues. 11. Enable MTL queue overflow and underflow interrupts for all four queues. 12. Configure DMA system bus mode with outstanding request limits and burst selection. 13. Preload TX descriptors into memory for DMA channel 0. 14. Set TX descriptor ring lengths of 10 for all four DMA channels. 15. Program TX descriptor list base addresses for all four DMA channels. 16. Program TX descriptor tail pointers for all four DMA channels to indicate available descriptors. 17. Initialize DMA channel control registers for all four channels. 18. Enable DMA interrupts for all four channels. 19. Enable HSS Autoreg Ethernet interrupts. 20. Start TX on DMA channel 0 by writing to the TX control register with the start TX bit set. 21. Wait for the expected number of transfer-complete interrupts in a polling loop. 22. In the interrupt handler, read the DMA interrupt status, clear DMA channel 0 status by writing all-ones, verify interrupt status is cleared, and clear the GIC interrupt. 23. After all interrupts are received, wait for a settling period and report the test as passed.",
        "Meta Impacted Registers": "",
        "Impacted Registers": "MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MAC_Packet_Filter; MAC_Configuration; MAC_Ext_Configuration; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_TxQ3_Quantum_Weight; MTL_TxQ2_Quantum_Weight; MTL_TxQ1_Quantum_Weight; MTL_TxQ0_Quantum_Weight; MTL_Q3_Interrupt_Control_Status; MTL_Q2_Interrupt_Control_Status; MTL_Q1_Interrupt_Control_Status; MTL_Q0_Interrupt_Control_Status; DMA_SysBus_Mode; DMA_CH3_TxDesc_Ring_Length; DMA_CH2_TxDesc_Ring_Length; DMA_CH1_TxDesc_Ring_Length; DMA_CH0_TxDesc_Ring_Length; DMA_CH3_TxDesc_List_Address; DMA_CH2_TxDesc_List_Address; DMA_CH1_TxDesc_List_Address; DMA_CH0_TxDesc_List_Address; DMA_CH3_TxDesc_Tail_Pointer; DMA_CH2_TxDesc_Tail_Pointer; DMA_CH1_TxDesc_Tail_Pointer; DMA_CH0_TxDesc_Tail_Pointer; DMA_CH3_Control; DMA_CH2_Control; DMA_CH1_Control; DMA_CH0_Control; DMA_CH1_Interrupt_Enable; DMA_CH2_Interrupt_Enable; DMA_CH3_Interrupt_Enable; DMA_CH0_Interrupt_Enable; DMA_CH0_Tx_Control; DMA_Interrupt_Status; DMA_CH0_Status",
        "Meta Validation / Acceptance Criteria": "",
        "Validation / Acceptance Criteria": "The test passes if all of the following conditions are met: 1. All expected transfer-complete interrupts (10 by default, or 6 for 10M speed) are received from the Ethernet DMA after TX is started on channel 0. 2. Each interrupt handler successfully reads the DMA interrupt status register. 3. The DMA channel 0 status register is cleared successfully by writing all-ones in each interrupt. 4. The DMA interrupt status is verified as cleared after the channel status write. 5. The GIC interrupt is cleared after each interrupt service. 6. The test completes the full interrupt polling loop without hanging or timing out. 7. After all transfers complete and a settling wait period, the test reports pass.",
        "Remarks": "The test supports three speed configurations (10M, 100M, 1G) and two duplex modes (half duplex, full duplex) selected via compile-time preprocessor defines. The default configuration is 1G full duplex. The transfer count is reduced from 10 to 6 for 10M speed. Four DMA channels (CH0-CH3) have their descriptor ring lengths, list addresses, tail pointers, control, and interrupt enables configured, but only CH0 TX path is actively started for transmission (CH1/CH2/CH3 TX start lines are commented out). TX descriptors are preloaded via two calls to preload_descriptor. Four hardcoded hex addresses (0XE68C2058, 0xA0243ffc, 0xA0243ff8, 0XE68C2050) are used for external control and could not be mapped to named registers. The test has no explicit data payload comparison; it validates successful transmission through interrupt completion only. The CLRIRQ_42 define provides an alternative to clear a fixed GIC IRQ number 42 instead of the computed irq_no."
    },
    {
        "Index": "4",
        "SS / Module": "Ethernet",
        "Test Case Name": "ethernet0_tx_rx_multi_chnl_test",
        "Feature": "Multi-Channel TX and RX Packet Transfer",
        "Meta Headers": "",
        "Meta Macros": "",
        "Meta Arrays": "",
        "Speed": "",
        "Mode": "",
        "Memory Start Offset": "",
        "Memory End Offset": "",
        "Meta Test Description": "",
        "Test Description": "This test verifies multi-channel TX and RX Ethernet packet transfer on Ethernet0 using all four DMA channels (CH0-CH3) in a sequential phased approach. The test configures the MAC layer including configuration for speed and duplex, extended configuration, RX queue control, VLAN tag control, packet filter with hash filtering, MMC IPC RX interrupt mask, and four MAC address slots with DMA channel selection. The MTL layer is configured with TX queue operation modes using 1KB queue size and store-and-forward, TX quantum weights for queue scheduling, RX queue operation modes with 4096-byte queue size and store-and-forward with error packet forwarding, queue-to-DMA channel mapping, overflow and underflow interrupt control, and RX queue control weights. The DMA layer is configured across all four channels including TX and RX control with 16-beat burst length, TX and RX descriptor list addresses, TX descriptor ring lengths of 10, RX descriptor ring lengths, system bus mode with outstanding request limits, and interrupt enables. TX and RX descriptors are preloaded for all four channels. The test operates in four sequential phases, each activating one DMA channel at a time. In Phase 1, DMA channel 3 is started for both TX and RX, with the RX queue mapping directed to channel 3. The test polls the RX and TX packet count registers until both reach 10. In Phase 2, DMA channel 2 is activated and the RX queue mapping is updated to channel 2, polling until counts reach 20. In Phase 3, DMA channel 1 is activated with mapping to channel 1, polling until counts reach 30. In Phase 4, DMA channel 0 is activated with mapping to channel 0, polling until counts reach 40. The interrupt handler manages RX descriptor tail pointer advancement for all four channels by reading the current RX descriptor address and comparing it with the tail pointer, then conditionally advancing or resetting the tail pointer. The handler also reads and clears DMA interrupt status for all four channels. The test passes after all four phases complete successfully with the expected cumulative packet counts.",
        "Meta Test Steps / Procedure": "",
        "Test Steps / Procedure": "1. Enable GIC interrupts for Ethernet0 interrupt handling. 2. Configure the Ethernet interface selection. 3. Configure the MAC Configuration register to enable TX and RX with the selected speed and duplex mode. 4. Configure MAC extended configuration, RX queue control registers for queue enable and priority assignment, VLAN tag control, and packet filter with hash filtering. 5. Configure the MMC IPC RX interrupt mask register. 6. Program four MAC address slots (Address0 through Address3) with high and low address values and DMA channel selection, and verify by reading back selected address registers. 7. Configure MTL TX queue operation modes for all four queues with 1KB queue size and store-and-forward mode. 8. Program MTL TX quantum weights for queue scheduling on all four queues. 9. Configure MTL operation mode. 10. Configure MTL RX queue operation modes for all four queues with 4096-byte queue size, store-and-forward, and error packet forwarding. 11. Enable MTL queue overflow and underflow interrupts for all four queues. 12. Map RX queues to corresponding DMA channels and verify by reading back the mapping register. 13. Program MTL RX queue control weights for all four queues. 14. Configure DMA TX control with 16-beat burst length for all four channels. 15. Set DMA TX descriptor list base addresses for all four channels. 16. Set DMA TX descriptor ring lengths of 10 for channels 1-3 and channel 0. 17. Initialize DMA channel control registers for channels 1-3. 18. Configure DMA system bus mode with outstanding request limits and burst selection. 19. Configure DMA RX control with 16-beat burst and RX buffer size for all four channels. 20. Preload TX descriptors into memory for all four DMA channels. 21. Set DMA RX descriptor list base addresses for all four channels. 22. Preload RX descriptors into memory for all four DMA channels. 23. Enable HSS Autoreg Ethernet interrupts. 24. Configure DMA RX control2 ring lengths for channels 1-3 and channel 0. 25. Enable DMA interrupts for all four channels. 26. Write TX descriptor tail pointers for channels 2 and 3. 27. Initialize DMA channel 0 control. Re-configure DMA RX control for all channels. Configure DMA channel 0 RX control2 and interrupt enable. Configure DMA mode. 28. Phase 1: Update RX queue-to-DMA mapping to direct all traffic to channel 3. Write RX descriptor tail pointers for all channels. Start RX on channel 3. Trigger the external VIP sequencer. Write TX descriptor tail pointers for all channels. Start TX on channel 3. Poll the RX and TX packet count registers until both reach 10 packets. 29. Phase 2: Start TX on channel 2. Update RX queue mapping to channel 2. Write RX descriptor tail pointer for channel 2. Start RX on channel 2. Re-trigger VIP sequencer. Poll until cumulative RX and TX packet counts reach 20. 30. Phase 3: Start TX on channel 1. Update RX queue mapping to channel 1. Write RX descriptor tail pointer for channel 1. Start RX on channel 1. Re-trigger VIP sequencer. Poll until cumulative counts reach 30. 31. Phase 4: Start TX on channel 0. Update RX queue mapping to channel 0. Write RX descriptor tail pointer for channel 0. Start RX on channel 0. Re-trigger VIP sequencer. Poll until cumulative counts reach 40. 32. In the interrupt handler, for each DMA channel read the current RX descriptor address and tail pointer, advance or reset the tail pointer as needed, read and clear DMA interrupt status for all channels, and clear the GIC interrupt. 33. After all four phases complete, wait for a settling period and report the test as passed.",
        "Meta Impacted Registers": "",
        "Impacted Registers": "MAC_Configuration; MAC_Ext_Configuration; MAC_RxQ_Ctrl0; MAC_RxQ_Ctrl1; MAC_RxQ_Ctrl2; MAC_VLAN_Tag_Ctrl; MAC_Packet_Filter; MMC_IPC_Rx_Interrupt_Mask; MAC_Address3_High; MAC_Address2_High; MAC_Address1_High; MAC_Address0_High; MAC_Address3_Low; MAC_Address2_Low; MAC_Address1_Low; MAC_Address0_Low; MTL_TxQ3_Operation_Mode; MTL_TxQ2_Operation_Mode; MTL_TxQ1_Operation_Mode; MTL_TxQ0_Operation_Mode; MTL_Operation_Mode; MTL_RxQ3_Operation_Mode; MTL_RxQ2_Operation_Mode; MTL_RxQ1_Operation_Mode; MTL_RxQ0_Operation_Mode; MTL_RxQ_DMA_Map0; MTL_RxQ3_Control; MTL_RxQ2_Control; MTL_RxQ1_Control; MTL_RxQ0_Control; DMA_Mode; DMA_SysBus_Mode; DMA_Interrupt_Status; DMA_CH0_Status; DMA_CH1_Status; DMA_CH2_Status; DMA_CH3_Status; DMA_CH3_RxDesc_Tail_Pointer; DMA_CH2_RxDesc_Tail_Pointer; DMA_CH1_RxDesc_Tail_Pointer; DMA_CH0_RxDesc_Tail_Pointer; DMA_CH0_TxDesc_Tail_Pointer; DMA_CH1_TxDesc_Tail_Pointer; Rx_Packets_Count_Good_Bad; Tx_Packet_Count_Good_Bad; DMA_CH0_Current_App_RxDesc; DMA_CH1_Current_App_RxDesc; DMA_CH2_Current_App_RxDesc; DMA_CH3_Current_App_RxDesc",
        "Meta Validation / Acceptance Criteria": "",
        "Validation / Acceptance Criteria": "The test passes if all of the following conditions are met: 1. Phase 1 (Channel 3): The cumulative RX and TX good/bad packet counts both reach exactly 10 after DMA channel 3 TX and RX are started. 2. Phase 2 (Channel 2): The cumulative RX and TX packet counts both reach exactly 20 after DMA channel 2 TX and RX are started. 3. Phase 3 (Channel 1): The cumulative RX and TX packet counts both reach exactly 30 after DMA channel 1 TX and RX are started. 4. Phase 4 (Channel 0): The cumulative RX and TX packet counts both reach exactly 40 after DMA channel 0 TX and RX are started. 5. Each interrupt handler successfully reads the current RX descriptor and tail pointer for all four DMA channels and correctly advances or resets the tail pointer. 6. The DMA interrupt status is read and all DMA channel status registers are cleared successfully in each interrupt. 7. The GIC interrupt is cleared after each interrupt service. 8. The test completes all four polling phases without hanging or timing out within the 20-iteration limit per phase. 9. After all phases complete and a settling wait period, the test reports pass via finish(0).",
        "Remarks": "The test supports three speed configurations (10M, 100M, 1G) and two duplex modes (half duplex, full duplex) selected via compile-time preprocessor defines. The default configuration is 1G full duplex. The test uses a sequential phased approach where each DMA channel is activated one at a time (CH3 first, then CH2, CH1, CH0), with the MTL RX queue-to-DMA mapping dynamically updated between phases. Each phase transfers 10 packets for a cumulative total of 40 TX and 40 RX packets. The RX queue mapping register is reprogrammed before each phase to direct all RX traffic to the active channel. The interrupt handler includes RX descriptor tail pointer management logic that advances the tail pointer by one descriptor size or resets it to the base when the boundary is reached. The PRELOAD_AGAIN compile-time define optionally enables re-preloading of RX descriptors when the tail pointer reaches the boundary. Four hardcoded hex addresses (0XE68C2058, 0xA0243ffc, 0xA0243ff8, 0XE68C2050) are used for external control and could not be mapped to named registers. The VIP sequencer is triggered multiple times with different trigger values (0xdeadbeef, 0xdeadbeee, 0xdeadbeed, 0xdeadbeec) for each phase. TX quantum weights differ between queues: Q3 and Q2 use weight 20, while Q1 and Q0 use weight 5. Some MAC address registers are read back after writing for verification purposes."
    }
]

# TestPlan columns
tp_cols = ['Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
           'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
           'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
           'Code Generation']

# MetaData columns
md_cols = ['Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
           'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
           'Meta Headers', 'Meta Macros', 'Meta Arrays']

wb = Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# MetaData sheet
ws_md = wb.create_sheet('MetaData')

header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')

# Write TestPlan headers
for ci, col_name in enumerate(tp_cols, 1):
    cell = ws_tp.cell(row=1, column=ci, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Write TestPlan data
for ri, row_data in enumerate(json_data, 2):
    for ci, col_name in enumerate(tp_cols, 1):
        val = row_data.get(col_name, '')
        cell = ws_tp.cell(row=ri, column=ci, value=val)
        cell.alignment = wrap_align

# Write MetaData headers
for ci, col_name in enumerate(md_cols, 1):
    cell = ws_md.cell(row=1, column=ci, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Write MetaData data
for ri, row_data in enumerate(json_data, 2):
    for ci, col_name in enumerate(md_cols, 1):
        val = row_data.get(col_name, '')
        cell = ws_md.cell(row=ri, column=ci, value=val)
        cell.alignment = wrap_align

# Auto-size columns
for ws in [ws_tp, ws_md]:
    for col_cells in ws.columns:
        max_len = 0
        col_letter = get_column_letter(col_cells[0].column)
        for cell in col_cells:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
        adjusted = min(max_len + 2, 80)
        ws.column_dimensions[col_letter].width = max(adjusted, 12)

# Freeze first row
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save
output_dir = 'Test_Output/Ethernet/TestPlan'
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)
wb.save(output_path)

print(f'SUCCESS: {output_path}')
print(f'Filename: {filename}')
print(f'Rows TestPlan: {len(json_data)}')
print(f'Rows MetaData: {len(json_data)}')
