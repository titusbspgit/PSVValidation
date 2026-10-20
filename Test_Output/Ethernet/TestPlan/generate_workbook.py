#!/usr/bin/env python3
"""
Agent 7 - Excel Workbook Generator for Ethernet TestPlan
Generates a genuine .xlsx workbook with two sheets:
  1. TestPlan (visible)
  2. MetaData (veryHidden)
"""
import json
import datetime
import os

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    import subprocess, sys
    subprocess.check_call([sys.executable, "-m", "pip", "install", "openpyxl"])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment

# ============================================================
# INPUT DATA - TESTPLAN_JSON (from Agent 6B)
# ============================================================
TESTPLAN_JSON = [
    {
        "Index": 1,
        "SS / Module": "Ethernet",
        "Test Case Name": "ethernet0_tx_rx_multi_chnl_test",
        "Feature": "Multi-Channel TX/RX Operation",
        "Speed": "10M / 100M / 1G",
        "Mode": "Full Duplex / Half Duplex",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase validates multi-channel transmit and receive operation on the DWC Ethernet QoS IP (Ethernet0) across all four DMA channels (CH0 through CH3). The test exercises the full MAC, MTL, and DMA subsystem configuration including MAC speed and duplex mode selection, MAC address programming, MTL TX/RX queue operation modes with quantum weight assignment, RX queue-to-DMA channel mapping, and per-channel DMA descriptor ring setup. TX and RX descriptors are preloaded for all four channels. The test sequentially activates each DMA channel starting from CH3 down to CH0, triggering the Ethernet VIP sequencer for each channel. Each channel is expected to transfer 10 packets, and cumulative TX/RX packet counts are validated at thresholds of 10, 20, 30, and 40 packets respectively. Conditional logic supports RX descriptor ring reload when the tail pointer reaches the end of the ring. The test completes after all four channels have been validated.",
        "Test Steps / Procedure": "Step 1: Initialize the interrupt controller and enable all interrupts.\nStep 2: Select the Ethernet interface based on the build configuration.\nStep 3: Configure the MAC Configuration register for the desired speed and duplex mode.\nStep 4: Configure MAC Extended Configuration, Packet Filter, RX Queue Control registers, and VLAN Tag Control.\nStep 5: Program MAC Address registers for multi-channel address filtering.\nStep 6: Mask MMC IPC RX interrupts.\nStep 7: Configure MTL Operation Mode and TX Queue Operation Mode for all four queues. Assign quantum weights.\nStep 8: Configure RX Queue Operation Mode and RX Queue Control registers.\nStep 9: Configure MTL Queue Interrupt Control Status for all four queues.\nStep 10: Configure DMA Mode and DMA Channel Control for CH0.\nStep 11: Set DMA TX Control and RX Control for all four channels.\nStep 12: Program TX and RX Descriptor List Addresses for all four channels.\nStep 13: Set TX Descriptor Ring Length and RX Control2 for all four channels.\nStep 14: Set TX Descriptor Tail Pointers for all four channels.\nStep 15: Enable DMA interrupts for all four channels.\nStep 16: Configure RX Queue-to-DMA Channel mapping and verify by reading back.\nStep 17: Set RX Queue Control registers for all four queues.\nStep 18: Preload TX descriptors for all four channels.\nStep 19: Preload RX descriptors for all four channels.\nStep 20: Configure power management register.\nStep 21: Enable CH3 RX and trigger VIP sequencer.\nStep 22: Poll and validate cumulative packet count reaches 10.\nStep 23: Enable CH2 and trigger VIP sequencer.\nStep 24: Poll and validate cumulative packet count reaches 20.\nStep 25: Enable CH1 and trigger VIP sequencer.\nStep 26: Poll and validate cumulative packet count reaches 30.\nStep 27: Enable CH0 and trigger VIP sequencer.\nStep 28: Poll and validate cumulative packet count reaches 40.\nStep 29: Update CH2 and CH3 TX Descriptor Tail Pointers.\nStep 30: Wait for completion delay and finish the test.",
        "Impacted Registers": "MAC_Configuration, MAC_Ext_Configuration, MAC_Packet_Filter, MAC_RxQ_Ctrl0, MAC_RxQ_Ctrl1, MAC_RxQ_Ctrl2, MAC_VLAN_Tag_Ctrl, MAC_Address0_High, MAC_Address0_Low, MAC_Address1_High, MAC_Address1_Low, MAC_Address2_High, MAC_Address2_Low, MAC_Address3_High, MAC_Address3_Low, MMC_IPC_Rx_Interrupt_Mask, MTL_Operation_Mode, MTL_TxQ0_Operation_Mode, MTL_TxQ1_Operation_Mode, MTL_TxQ2_Operation_Mode, MTL_TxQ3_Operation_Mode, MTL_TxQ0_Quantum_Weight, MTL_TxQ1_Quantum_Weight, MTL_TxQ2_Quantum_Weight, MTL_TxQ3_Quantum_Weight, MTL_RxQ0_Operation_Mode, MTL_RxQ1_Operation_Mode, MTL_RxQ2_Operation_Mode, MTL_RxQ3_Operation_Mode, MTL_RxQ0_Control, MTL_RxQ1_Control, MTL_RxQ2_Control, MTL_RxQ3_Control, MTL_RxQ_DMA_Map0, MTL_Q0_Interrupt_Control_Status, MTL_Q1_Interrupt_Control_Status, MTL_Q2_Interrupt_Control_Status, MTL_Q3_Interrupt_Control_Status, DMA_Mode, DMA_SysBus_Mode, DMA_CH0_Control, DMA_CH1_Control, DMA_CH2_Control, DMA_CH3_Control, DMA_CH0_Tx_Control, DMA_CH1_Tx_Control, DMA_CH2_Tx_Control, DMA_CH3_Tx_Control, DMA_CH0_Rx_Control, DMA_CH1_Rx_Control, DMA_CH2_Rx_Control, DMA_CH3_Rx_Control, DMA_CH0_Rx_Control2, DMA_CH1_Rx_Control2, DMA_CH2_Rx_Control2, DMA_CH3_Rx_Control2, DMA_CH0_TxDesc_List_Address, DMA_CH1_TxDesc_List_Address, DMA_CH2_TxDesc_List_Address, DMA_CH3_TxDesc_List_Address, DMA_CH0_RxDesc_List_Address, DMA_CH1_RxDesc_List_Address, DMA_CH2_RxDesc_List_Address, DMA_CH3_RxDesc_List_Address, DMA_CH0_TxDesc_Ring_Length, DMA_CH1_TxDesc_Ring_Length, DMA_CH2_TxDesc_Ring_Length, DMA_CH3_TxDesc_Ring_Length, DMA_CH0_TxDesc_Tail_Pointer, DMA_CH1_TxDesc_Tail_Pointer, DMA_CH2_TxDesc_Tail_Pointer, DMA_CH3_TxDesc_Tail_Pointer, DMA_CH0_RxDesc_Tail_Pointer, DMA_CH1_RxDesc_Tail_Pointer, DMA_CH2_RxDesc_Tail_Pointer, DMA_CH3_RxDesc_Tail_Pointer, DMA_CH0_Interrupt_Enable, DMA_CH1_Interrupt_Enable, DMA_CH2_Interrupt_Enable, DMA_CH3_Interrupt_Enable, DMA_CH0_Current_App_TxDesc, DMA_CH0_Current_App_TxBuffer, DMA_CH1_Current_App_TxDesc, DMA_CH1_Current_App_TxBuffer, DMA_CH2_Current_App_TxDesc, DMA_CH2_Current_App_TxBuffer, DMA_CH3_Current_App_TxDesc, DMA_CH3_Current_App_TxBuffer, DMA_CH0_Current_App_RxDesc, DMA_CH1_Current_App_RxDesc, DMA_CH2_Current_App_RxDesc, DMA_CH3_Current_App_RxDesc",
        "Validation / Acceptance Criteria": "1. After CH3 activation, cumulative RX=10 and TX=10.\n2. After CH2 activation, cumulative RX=20 and TX=20.\n3. After CH1 activation, cumulative RX=30 and TX=30.\n4. After CH0 activation, cumulative RX=40 and TX=40.\n5. RX descriptor conditional reload when tail pointer reaches end of ring.\n6. MTL RXQ DMA Map0 readback verification.\n7. Test must complete without hanging in any polling loop.",
        "Remarks": "The test supports multiple speed configurations (10M, 100M, 1G) and duplex modes (Full, Half) selectable via build-time configuration. Default is 1G Full Duplex. Channels are activated sequentially from CH3 to CH0. Each channel handles 10 packets for a total of 40."
    }
]

# ============================================================
# INPUT DATA - META_JSON (from Agent 6A)
# ============================================================
META_JSON = [
    {
        "Test Case Name": "ethernet0_tx_rx_multi_chnl_test",
        "Meta Headers": "Source File: program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include <math.h>\n4. #include <test_common.h>\n5. #include <ethernet0/ethernet0_def.h>\n6. #include <ethernet0/ethernet0_offset.h>\n7. #include <ethernet0/ethernet0_funcs.h>\n8. #include <ethernet0/ethernet0_programming_sequence.h>",
        "Meta Macros": """Source File: program.c

Conditional Compilation Macros (used in #ifdef/#elif directives, definitions not supplied in source):
1. ETH_10M
2. ETH_100M
3. HALF_DUPLEX
4. PRELOAD_AGAIN
5. SEL_ENET0
6. SEL_ENET1
7. SEL_ENET2
8. SEL_ENET3

Register-address macros (definitions not supplied in this source file; resolved via header files):
mizar_ETHERNET0_MAC_CONFIGURATION
mizar_ETHERNET0_MAC_EXT_CONFIGURATION
mizar_ETHERNET0_MAC_PACKET_FILTER
mizar_ETHERNET0_MAC_RXQ_CTRL0
mizar_ETHERNET0_MAC_RXQ_CTRL1
mizar_ETHERNET0_MAC_RXQ_CTRL2
mizar_ETHERNET0_MAC_VLAN_TAG_CTRL
mizar_ETHERNET0_MAC_ADDRESS0_HIGH
mizar_ETHERNET0_MAC_ADDRESS0_LOW
mizar_ETHERNET0_MAC_ADDRESS1_HIGH
mizar_ETHERNET0_MAC_ADDRESS1_LOW
mizar_ETHERNET0_MAC_ADDRESS2_HIGH
mizar_ETHERNET0_MAC_ADDRESS2_LOW
mizar_ETHERNET0_MAC_ADDRESS3_HIGH
mizar_ETHERNET0_MAC_ADDRESS3_LOW
mizar_ETHERNET0_MMC_IPC_RX_INTERRUPT_MASK
mizar_ETHERNET0_MTL_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT
mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT
mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT
mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT
mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE
mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE
mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE
mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE
mizar_ETHERNET0_MTL_RXQ0_CONTROL
mizar_ETHERNET0_MTL_RXQ1_CONTROL
mizar_ETHERNET0_MTL_RXQ2_CONTROL
mizar_ETHERNET0_MTL_RXQ3_CONTROL
mizar_ETHERNET0_MTL_RXQ_DMA_MAP0
mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS
mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS
mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS
mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS
mizar_ETHERNET0_DMA_MODE
mizar_ETHERNET0_DMA_SYSBUS_MODE
mizar_ETHERNET0_DMA_CH0_CONTROL
mizar_ETHERNET0_DMA_CH1_CONTROL
mizar_ETHERNET0_DMA_CH2_CONTROL
mizar_ETHERNET0_DMA_CH3_CONTROL
mizar_ETHERNET0_DMA_CH0_TX_CONTROL
mizar_ETHERNET0_DMA_CH1_TX_CONTROL
mizar_ETHERNET0_DMA_CH2_TX_CONTROL
mizar_ETHERNET0_DMA_CH3_TX_CONTROL
mizar_ETHERNET0_DMA_CH0_RX_CONTROL
mizar_ETHERNET0_DMA_CH1_RX_CONTROL
mizar_ETHERNET0_DMA_CH2_RX_CONTROL
mizar_ETHERNET0_DMA_CH3_RX_CONTROL
mizar_ETHERNET0_DMA_CH0_RX_CONTROL2
mizar_ETHERNET0_DMA_CH1_RX_CONTROL2
mizar_ETHERNET0_DMA_CH2_RX_CONTROL2
mizar_ETHERNET0_DMA_CH3_RX_CONTROL2
mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH
mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH
mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH
mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH
mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE
mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE
mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE
mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE
mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXDESC
mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXBUFFER
mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXDESC
mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXBUFFER
mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXDESC
mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXBUFFER
mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC
mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER
mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC
mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC
mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC
mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC
mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD
mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD""",
        "Meta Arrays": "NA",
        "Meta Types": """TYPE_DEFINITIONS
No typedef, struct, union, or enum definitions visible in supplied source.

OTHER_DECLARATIONS
Source File: program.c
Function: test_case()

Local variable declarations:
  int wdata,rdata,i,txpkt,rxpkt;
  int address,read;

Global/external variable usage (declarations not visible in supplied source, used at file/function scope):
  int_pend
  enetsel_randvar
  sel_enet
  enet_sel
  rdata_tailpointer
  rdata_desc
  power""",
        "Meta Test Description": "This testcase (ethernet0_tx_rx_multi_chnl_test) validates multi-channel TX and RX operation on the DWC Ethernet QoS IP (Ethernet0). The test configures MAC, MTL, and DMA subsystems across 4 DMA channels (CH0-CH3). It sets up MAC configuration (speed/duplex via conditional compilation for 10M/100M/1G), MAC addresses with per-channel DMA selection, MTL TX/RX queue operation modes, quantum weights, RX queue-to-DMA channel mapping, and DMA channel control registers. TX and RX descriptors are preloaded for all 4 channels. The test then sequentially activates each channel (CH3 first, then CH2, CH1, CH0), triggers the ETH VIP sequencer via write_reg to 0xA0243ff8, and polls TX/RX packet count registers in while(int_pend) loops. Validation checks cumulative packet counts at 10, 20, 30, and 40 (10 packets per channel). Conditional PRELOAD_AGAIN logic reloads RX descriptors when the tail pointer reaches the end of the ring. The test completes with wait_on(2000) and finish(0).",
        "Meta Test Steps / Procedure": """SOURCE_FILE: program.c
SOURCE_CONTENT:
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <test_common.h>
#include<ethernet0/ethernet0_def.h>
#include<ethernet0/ethernet0_offset.h>
#include<ethernet0/ethernet0_funcs.h>
#include<ethernet0/ethernet0_programming_sequence.h>

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

int test_case()
{
    int_pend = 1;
    int wdata,rdata,i,txpkt,rxpkt;
    int address,read;
    enetsel_randvar = rand();
    sel_enet = (enetsel_randvar%4)*0x100000;
    GIC_Set();
    GIC_EnableAllIRQ();

#ifdef ETH_10M
  #ifdef HALF_DUPLEX
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0x8003);
  #else
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xA003);
  #endif
    write_reg((0xE68C205C + (enet_sel*4)),0x1A08);
    wait_on(300);
    write_reg((0xE68C205C + (enet_sel*4)),0x1EC8);
    wait_on(300);
    write_reg((0xE68C205C + (enet_sel*4)),0x18C8);
#elif ETH_100M
  #ifdef HALF_DUPLEX
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xC003);
  #else
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xE003);
  #endif
    write_reg((0xE68C205C + (enet_sel*4)),0x1A08);
    wait_on(300);
    write_reg((0xE68C205C + (enet_sel*4)),0x1E14);
    wait_on(300);
    write_reg((0xE68C205C + (enet_sel*4)),0x1814);
#else
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0x00002003);
#endif

    write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION,0x0);
    write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER,0x80000408);
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0,0x000000aa);
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1,0x00000000);
    write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2,0x08040201);
    write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL,0x01600000);

    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH,0x80000607);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW,0x08090a0b);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW,0x08090a0b);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW,0x08090a0b);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW,0x08090a0b);

    write_enet_reg(mizar_ETHERNET0_MMC_IPC_RX_INTERRUPT_MASK,0x80000400);
    write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE,0x00000000);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE,0x0003000a);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE,0x0003000a);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE,0x0003000a);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE,0x0003000a);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT,0x00000005);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT,0x00000005);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT,0x00000014);
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT,0x00000014);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE,0x00f00033);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE,0x00f00033);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE,0x00f00033);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE,0x00f00033);

    write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100);
    write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100);
    write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100);
    write_enet_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x01000100);

    write_enet_reg(mizar_ETHERNET0_DMA_MODE, 0x00000000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0xE6010000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0xE6020000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0xE6030000);

    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0xE6070000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0xE6060000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0xE6050000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);

    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x9);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH, 0x9);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH, 0x9);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH, 0x9);

    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL2, 0x9);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL2, 0x9);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL2, 0x9);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x9);

    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0xE6019EB4);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE6029EB4);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE6039EB4);

    write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0xF0DF);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0xF0DF);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0xF0DF);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0xF0DF);

    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x03020100);
    rdata = read_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_CONTROL, 0x00000007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_CONTROL, 0x00000007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_CONTROL, 0x00000007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_CONTROL, 0x00000007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);

    preload_descriptor(0xE6000000,0xE6001000,0x3c,0x5);
    preload_descriptor(0xE6000050,0xE600112c,0x5E8,0x5);
    preload_descriptor(0xE6010000,0xE6011000,0x3c,0x5);
    preload_descriptor(0xE6010050,0xE601112c,0x5E8,0x5);
    preload_descriptor(0xE6020000,0xE6021000,0x3c,0x5);
    preload_descriptor(0xE6020050,0xE602112c,0x5E8,0x5);
    preload_descriptor(0xE6030000,0xE6031000,0x3c,0x5);
    preload_descriptor(0xE6030050,0xE603112c,0x5E8,0x5);

    preload_descriptor_rx(0xE6040000,0xE6044000,0x10,0xA);
    preload_descriptor_rx(0xE6050000,0xE6054000,0x10,0xA);
    preload_descriptor_rx(0xE6060000,0xE6064000,0x10,0xA);
    preload_descriptor_rx(0xE6070000,0xE6074000,0x10,0xA);

    write_reg(0XE68C2058, power);

    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101001);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);
    write_reg(0xA0243ffc,enet_sel);
    write_reg(0xA0243ff8,0xdeadbeef);

    while(int_pend)
    {
        rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
        txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
        rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXDESC);
        rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXBUFFER);
        rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXDESC);
        rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXBUFFER);
        rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXDESC);
        rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXBUFFER);
        rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);
        rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);
        rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
        rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER);
        if(rdata_tailpointer == rdata_desc)
        {
            if(rdata_tailpointer == 0xE60700f0)
            {
#ifdef PRELOAD_AGAIN
                preload_descriptor_rx(0xE6070000, 0xE6074000, 0x10, 0x10);
#endif
                write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070000);
            }
        }
        if(rxpkt == 10 && txpkt == 10){ break; }
    }
    int_pend = 1;

    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000082);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101001);
    write_reg(0xA0243ff8, 0xdeadbeee);

    while(int_pend)
    {
        rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
        txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
        rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
        rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER);
        if(rdata_tailpointer == rdata_desc)
        {
            if(rdata_tailpointer == 0xE60600f0)
            {
#ifdef PRELOAD_AGAIN
                preload_descriptor_rx(0xE6060000, 0xE6064000, 0x10, 0x10);
#endif
            }
        }
        if(rxpkt == 20 && txpkt == 20){ break; }
    }
    int_pend = 1;

    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000081);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101001);
    write_reg(0xA0243ff8, 0xdeadbeed);

    while(int_pend)
    {
        rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
        txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
        rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
        rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER);
        if(rdata_tailpointer == rdata_desc)
        {
            if(rdata_tailpointer == 0xE60500f0)
            {
#ifdef PRELOAD_AGAIN
                preload_descriptor_rx(0xE6050000, 0xE6054000, 0x10, 0x10);
#endif
                write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050000);
            }
        }
        if(rxpkt == 30 && txpkt == 30){ break; }
    }
    int_pend = 1;

    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100007);
    write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000080);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040010);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101001);
    write_reg(0xA0243ff8, 0xdeadbeec);

    while(int_pend)
    {
        rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
        txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
        rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
        rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER);
        if(rdata_tailpointer == rdata_desc)
        {
            if(rdata_tailpointer == 0xE60400f0)
            {
#ifdef PRELOAD_AGAIN
                preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0x10);
#endif
                write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040000);
            }
        }
        if(rxpkt == 40 && txpkt == 40){ break; }
    }
    int_pend = 1;

    write_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER,0xE60298a4);
    write_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER,0xE60398a4);

    wait_on(2000);
    finish(0);
}
END_SOURCE_FILE""",
        "Meta Impacted Registers": """Agent 4 Output - Register Mappings (base_value=0xE6B00000 for all resolved registers):

1. ETH_10M - unresolved (conditional compilation flag)
2. ETH_100M - unresolved (conditional compilation flag)
3. HALF_DUPLEX - unresolved (conditional compilation flag)
4. PRELOAD_AGAIN - unresolved (conditional compilation flag)
5. mizar_ETHERNET0_MAC_CONFIGURATION -> MAC_Configuration (offset 0x0) - matched
6. mizar_ETHERNET0_MAC_EXT_CONFIGURATION -> MAC_Ext_Configuration (offset 0x4) - matched
7. mizar_ETHERNET0_MAC_PACKET_FILTER -> MAC_Packet_Filter (offset 0x8) - matched
8. mizar_ETHERNET0_MAC_RXQ_CTRL0 -> MAC_RxQ_Ctrl0 (offset 0xA0) - matched
9. mizar_ETHERNET0_MAC_RXQ_CTRL1 -> MAC_RxQ_Ctrl1 (offset 0xA4) - matched
10. mizar_ETHERNET0_MAC_RXQ_CTRL2 -> MAC_RxQ_Ctrl2 (offset 0xA8) - matched
11. mizar_ETHERNET0_MAC_VLAN_TAG_CTRL -> MAC_VLAN_Tag_Ctrl (offset 0x50) - matched
12. mizar_ETHERNET0_MAC_ADDRESS0_HIGH -> MAC_Address0_High (offset 0x300) - matched
13. mizar_ETHERNET0_MAC_ADDRESS0_LOW -> MAC_Address0_Low (offset 0x304) - matched
14. mizar_ETHERNET0_MAC_ADDRESS1_HIGH -> MAC_Address1_High (offset 0x308) - matched
15. mizar_ETHERNET0_MAC_ADDRESS1_LOW -> MAC_Address1_Low (offset 0x30C) - matched
16. mizar_ETHERNET0_MAC_ADDRESS2_HIGH -> MAC_Address2_High (offset 0x310) - matched
17. mizar_ETHERNET0_MAC_ADDRESS2_LOW -> MAC_Address2_Low (offset 0x314) - matched
18. mizar_ETHERNET0_MAC_ADDRESS3_HIGH -> MAC_Address3_High (offset 0x318) - matched
19. mizar_ETHERNET0_MAC_ADDRESS3_LOW -> MAC_Address3_Low (offset 0x31C) - matched
20. mizar_ETHERNET0_MMC_IPC_RX_INTERRUPT_MASK -> MMC_IPC_Rx_Interrupt_Mask (offset 0x800) - matched
21. mizar_ETHERNET0_MTL_OPERATION_MODE -> MTL_Operation_Mode (offset 0xC00) - matched
22. mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE -> MTL_TxQ0_Operation_Mode (offset 0xD00) - matched
23. mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE -> MTL_TxQ1_Operation_Mode (offset 0xD40) - matched
24. mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE -> MTL_TxQ2_Operation_Mode (offset 0xD80) - matched
25. mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE -> MTL_TxQ3_Operation_Mode (offset 0xDC0) - matched
26. mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT -> MTL_TxQ0_Quantum_Weight (offset 0xD18) - matched
27. mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT -> MTL_TxQ1_Quantum_Weight (offset 0xD58) - matched
28. mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT -> MTL_TxQ2_Quantum_Weight (offset 0xD98) - matched
29. mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT -> MTL_TxQ3_Quantum_Weight (offset 0xDD8) - matched
30. mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE -> MTL_RxQ0_Operation_Mode (offset 0xD30) - matched
31. mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE -> MTL_RxQ1_Operation_Mode (offset 0xD70) - matched
32. mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE -> MTL_RxQ2_Operation_Mode (offset 0xDB0) - matched
33. mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE -> MTL_RxQ3_Operation_Mode (offset 0xDF0) - matched
34. mizar_ETHERNET0_MTL_RXQ0_CONTROL -> MTL_RxQ0_Control (offset 0xD3C) - matched
35. mizar_ETHERNET0_MTL_RXQ1_CONTROL -> MTL_RxQ1_Control (offset 0xD7C) - matched
36. mizar_ETHERNET0_MTL_RXQ2_CONTROL -> MTL_RxQ2_Control (offset 0xDBC) - matched
37. mizar_ETHERNET0_MTL_RXQ3_CONTROL -> MTL_RxQ3_Control (offset 0xDFC) - matched
38. mizar_ETHERNET0_MTL_RXQ_DMA_MAP0 -> MTL_RxQ_DMA_Map0 (offset 0xC30) - matched
39. mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS -> MTL_Q0_Interrupt_Control_Status (offset 0xD2C) - matched
40. mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS -> MTL_Q1_Interrupt_Control_Status (offset 0xD6C) - matched
41. mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS -> MTL_Q2_Interrupt_Control_Status (offset 0xDAC) - matched
42. mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS -> MTL_Q3_Interrupt_Control_Status (offset 0xDEC) - matched
43. mizar_ETHERNET0_DMA_MODE -> DMA_Mode (offset 0x1000) - matched
44. mizar_ETHERNET0_DMA_SYSBUS_MODE -> DMA_SysBus_Mode (offset 0x1004) - matched
45. mizar_ETHERNET0_DMA_CH0_CONTROL -> DMA_CH0_Control (offset 0x1100) - matched
46. mizar_ETHERNET0_DMA_CH1_CONTROL -> DMA_CH1_Control (offset 0x1180) - matched
47. mizar_ETHERNET0_DMA_CH2_CONTROL -> DMA_CH2_Control (offset 0x1200) - matched
48. mizar_ETHERNET0_DMA_CH3_CONTROL -> DMA_CH3_Control (offset 0x1280) - matched
49. mizar_ETHERNET0_DMA_CH0_TX_CONTROL -> DMA_CH0_Tx_Control (offset 0x1104) - matched
50. mizar_ETHERNET0_DMA_CH1_TX_CONTROL -> DMA_CH1_Tx_Control (offset 0x1184) - matched
51. mizar_ETHERNET0_DMA_CH2_TX_CONTROL -> DMA_CH2_Tx_Control (offset 0x1204) - matched
52. mizar_ETHERNET0_DMA_CH3_TX_CONTROL -> DMA_CH3_Tx_Control (offset 0x1284) - matched
53. mizar_ETHERNET0_DMA_CH0_RX_CONTROL -> DMA_CH0_Rx_Control (offset 0x1108) - matched
54. mizar_ETHERNET0_DMA_CH1_RX_CONTROL -> DMA_CH1_Rx_Control (offset 0x1188) - matched
55. mizar_ETHERNET0_DMA_CH2_RX_CONTROL -> DMA_CH2_Rx_Control (offset 0x1208) - matched
56. mizar_ETHERNET0_DMA_CH3_RX_CONTROL -> DMA_CH3_Rx_Control (offset 0x1288) - matched
57. mizar_ETHERNET0_DMA_CH0_RX_CONTROL2 -> DMA_CH0_Rx_Control2 (offset 0x1130) - matched
58. mizar_ETHERNET0_DMA_CH1_RX_CONTROL2 -> DMA_CH1_Rx_Control2 (offset 0x11B0) - matched
59. mizar_ETHERNET0_DMA_CH2_RX_CONTROL2 -> DMA_CH2_Rx_Control2 (offset 0x1230) - matched
60. mizar_ETHERNET0_DMA_CH3_RX_CONTROL2 -> DMA_CH3_Rx_Control2 (offset 0x12B0) - matched
61. mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS -> DMA_CH0_TxDesc_List_Address (offset 0x1114) - matched
62. mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS -> DMA_CH1_TxDesc_List_Address (offset 0x1194) - matched
63. mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS -> DMA_CH2_TxDesc_List_Address (offset 0x1214) - matched
64. mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS -> DMA_CH3_TxDesc_List_Address (offset 0x1294) - matched
65. mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS -> DMA_CH0_RxDesc_List_Address (offset 0x111C) - matched
66. mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS -> DMA_CH1_RxDesc_List_Address (offset 0x119C) - matched
67. mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS -> DMA_CH2_RxDesc_List_Address (offset 0x121C) - matched
68. mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS -> DMA_CH3_RxDesc_List_Address (offset 0x129C) - matched
69. mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH -> DMA_CH0_TxDesc_Ring_Length (offset 0x112C) - matched
70. mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH -> DMA_CH1_TxDesc_Ring_Length (offset 0x11AC) - matched
71. mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH -> DMA_CH2_TxDesc_Ring_Length (offset 0x122C) - matched
72. mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH -> DMA_CH3_TxDesc_Ring_Length (offset 0x12AC) - matched
73. mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER -> DMA_CH0_TxDesc_Tail_Pointer (offset 0x1120) - matched
74. mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER -> DMA_CH1_TxDesc_Tail_Pointer (offset 0x11A0) - matched
75. mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER -> DMA_CH2_TxDesc_Tail_Pointer (offset 0x1220) - matched
76. mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER -> DMA_CH3_TxDesc_Tail_Pointer (offset 0x12A0) - matched
77. mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER -> DMA_CH0_RxDesc_Tail_Pointer (offset 0x1128) - matched
78. mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER -> DMA_CH1_RxDesc_Tail_Pointer (offset 0x11A8) - matched
79. mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER -> DMA_CH2_RxDesc_Tail_Pointer (offset 0x1228) - matched
80. mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER -> DMA_CH3_RxDesc_Tail_Pointer (offset 0x12A8) - matched
81. mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE -> DMA_CH0_Interrupt_Enable (offset 0x1134) - matched
82. mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE -> DMA_CH1_Interrupt_Enable (offset 0x11B4) - matched
83. mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE -> DMA_CH2_Interrupt_Enable (offset 0x1234) - matched
84. mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE -> DMA_CH3_Interrupt_Enable (offset 0x12B4) - matched
85. mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXDESC -> DMA_CH0_Current_App_TxDesc (offset 0x1144) - matched
86. mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXBUFFER -> DMA_CH0_Current_App_TxBuffer (offset 0x1154) - matched
87. mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXDESC -> DMA_CH1_Current_App_TxDesc (offset 0x11C4) - matched
88. mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXBUFFER -> DMA_CH1_Current_App_TxBuffer (offset 0x11D4) - matched
89. mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXDESC -> DMA_CH2_Current_App_TxDesc (offset 0x1244) - matched
90. mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXBUFFER -> DMA_CH2_Current_App_TxBuffer (offset 0x1254) - matched
91. mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC -> DMA_CH3_Current_App_TxDesc (offset 0x12C4) - matched
92. mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER -> DMA_CH3_Current_App_TxBuffer (offset 0x12D4) - matched
93. mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC -> DMA_CH0_Current_App_RxDesc (offset 0x114C) - matched
94. mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC -> DMA_CH1_Current_App_RxDesc (offset 0x11CC) - matched
95. mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC -> DMA_CH2_Current_App_RxDesc (offset 0x124C) - matched
96. mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC -> DMA_CH3_Current_App_RxDesc (offset 0x12CC) - matched

Additional registers used in source but not in Agent 4 macro list:
- mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD (read via read_enet_reg)
- mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD (read via read_enet_reg)

Total resolved: 92 of 96 tokens matched. 4 unresolved (conditional compilation flags).""",
        "Meta Validation / Acceptance Criteria": """Source File: program.c
Function: test_case()

Validation 1 - CH3 packet count check: if(rxpkt == 10 && txpkt == 10){ break; }
Validation 2 - CH2 packet count check: if(rxpkt == 20 && txpkt == 20){ break; }
Validation 3 - CH1 packet count check: if(rxpkt == 30 && txpkt == 30){ break; }
Validation 4 - CH0 packet count check: if(rxpkt == 40 && txpkt == 40){ break; }
Validation 5 - MTL_RXQ_DMA_MAP0 readback verification
Test Completion: wait_on(2000); finish(0);"""
    }
]

# ============================================================
# CONFIGURATION
# ============================================================
IP_NAME = "Ethernet"
IST_OFFSET = datetime.timedelta(hours=5, minutes=30)
now_ist = datetime.datetime.utcnow() + IST_OFFSET
TIMESTAMP = now_ist.strftime("%Y%m%d_%H%M%S")
FILENAME = f"{IP_NAME}_TestPlan_{TIMESTAMP}.xlsx"
OUTPUT_DIR = "/tmp"
FILEPATH = os.path.join(OUTPUT_DIR, FILENAME)

# ============================================================
# CROSS-DATASET VALIDATION
# ============================================================
meta_names = {m["Test Case Name"] for m in META_JSON}
tp_names = {t["Test Case Name"] for t in TESTPLAN_JSON}
assert tp_names.issubset(meta_names), f"Mismatch: {tp_names - meta_names}"

# ============================================================
# WORKBOOK CREATION
# ============================================================
wb = Workbook()

# --- Styles ---
header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# ============================================================
# TESTPLAN SHEET
# ============================================================
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks"
]

# Write header
for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write data rows
for row_idx, tc in enumerate(TESTPLAN_JSON, 2):
    for col_idx, col_name in enumerate(tp_columns, 1):
        val = tc.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = "A2"

# Auto-size columns with max width
for col_idx, col_name in enumerate(tp_columns, 1):
    max_len = len(str(col_name))
    for row in ws_tp.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
        for cell in row:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
    adjusted = min(max_len + 2, 80)
    ws_tp.column_dimensions[ws_tp.cell(row=1, column=col_idx).column_letter].width = adjusted

# ============================================================
# METADATA SHEET
# ============================================================
ws_meta = wb.create_sheet(title="MetaData")

meta_columns = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

# Write header
for col_idx, col_name in enumerate(meta_columns, 1):
    cell = ws_meta.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write data rows
for row_idx, mc in enumerate(META_JSON, 2):
    for col_idx, col_name in enumerate(meta_columns, 1):
        val = mc.get(col_name, "")
        cell = ws_meta.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_alignment

# Freeze first row
ws_meta.freeze_panes = "A2"

# Auto-size columns with max width
for col_idx, col_name in enumerate(meta_columns, 1):
    max_len = len(str(col_name))
    for row in ws_meta.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
        for cell in row:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
    adjusted = min(max_len + 2, 80)
    ws_meta.column_dimensions[ws_meta.cell(row=1, column=col_idx).column_letter].width = adjusted

# SET METADATA SHEET TO veryHidden
ws_meta.sheet_state = "veryHidden"

# ============================================================
# SAVE WORKBOOK
# ============================================================
wb.save(FILEPATH)
wb.close()

# ============================================================
# POST-SAVE VALIDATION
# ============================================================
assert os.path.exists(FILEPATH), "Workbook file does not exist"
assert os.path.getsize(FILEPATH) > 0, "Workbook file is empty"

wb_check = load_workbook(FILEPATH)
assert "TestPlan" in wb_check.sheetnames, "TestPlan sheet missing"
assert "MetaData" in wb_check.sheetnames, "MetaData sheet missing"
assert wb_check["MetaData"].sheet_state == "veryHidden", "MetaData not veryHidden"

tp_rows = wb_check["TestPlan"].max_row - 1  # minus header
meta_rows = wb_check["MetaData"].max_row - 1  # minus header
assert tp_rows == len(TESTPLAN_JSON), f"TestPlan rows mismatch: {tp_rows} vs {len(TESTPLAN_JSON)}"
assert meta_rows == len(META_JSON), f"MetaData rows mismatch: {meta_rows} vs {len(META_JSON)}"

wb_check.close()

# ============================================================
# OUTPUT
# ============================================================
import base64
with open(FILEPATH, "rb") as f:
    b64_content = base64.b64encode(f.read()).decode("utf-8")

# Write base64 to a text file for retrieval
b64_path = os.path.join(OUTPUT_DIR, FILENAME + ".b64")
with open(b64_path, "w") as f:
    f.write(b64_content)

print(f"FILENAME={FILENAME}")
print(f"FILEPATH={FILEPATH}")
print(f"FILESIZE={os.path.getsize(FILEPATH)}")
print(f"TP_ROWS={tp_rows}")
print(f"META_ROWS={meta_rows}")
print(f"B64_PATH={b64_path}")
print(f"B64_LEN={len(b64_content)}")
print("VALIDATION=PASSED")
