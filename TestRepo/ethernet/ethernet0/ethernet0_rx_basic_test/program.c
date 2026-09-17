
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include<ethernet0/ethernet0_def.h>
#include <test_common.h>
#include<ethernet0/ethernet0_offset.h>
#include<ethernet0/ethernet0_funcs.h>
#include<ethernet0/ethernet0_programming_sequence.h>
int data_rd,data_wr;
int def_fail_cnt = 0,wr_fail_cnt = 0;
unsigned int trns_count = 10;
extern int int_pend;
int test_case()
{
    int_pend = 1;
    int wdata,rdata,i=0;
    int address,read;
    GIC_Set();
    GIC_EnableAllIRQ(); //Enable all interrupt
	
    #ifdef ETH_10M
	enet_10m_speed();
    #elif ETH_100M
    	enet_100m_speed();
    #endif

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

    #ifdef ETH_10M
      trns_count = 6;
    #ifdef HALF_DUPLEX
        write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0x8003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Half Duplex mode selected,Speed
    #else
        write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xA003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed
    #endif
	

#elif ETH_100M
    #ifdef HALF_DUPLEX
        write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xC003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Half Duplex mode selected,Speed
    #else
        write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xE003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed
    #endif


#else
#ifdef HALF_DUPLEX
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0x0003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed
    #else
    write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0x2003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed

    #endif
#endif


write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0,0x00aa);       //RX Queue CTL0 => Queue enabled for DCB/Generic
write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1,0x000);        //RX Queue CTL1
write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2,0x08040201);   //RX Queue CTL2 => Priorities for RX Queues
write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL4,0x0);   //RX Queue CTL4 => Enabling/Disabling Queues for UDC,VFFQ,MFFW,UFFQ
write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL,0x01600000);   //VLAN_TAG_CTRL => Enabling VLAN TAG in status and VLAN TAG RX Stripping Enabled
write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER,0x80000400);   //MAC_PACKET_FILTER =>  HASH FILTER Enabled and Receive All packets regardless of filter

//DMA Channel selection,Address Enabled and upper 16 bits[47:32] of the 2nd 6-byte MAC address Programming
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH,0x80033607);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH,0x80022607);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH,0x80011607);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH,0x80000607);

//lower 32 bits of the 2nd 6-byte MAC address
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW,0x08090a00);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW,0x08090a00);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW,0x08090a00);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW,0x08090a00);


write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION,0x0);

write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE,0x000f0002);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q3 not Enabled
write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE,0x000f0002);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q2 not Enabled
write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE,0x000f0002);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q1 not Enabled
write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q0 Enabled


write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE,0x000f0002);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q3 not Enabled
write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE,0x000f0002);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q2 not Enabled
write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE,0x000f0002);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q1 not Enabled
write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q0 Enabled


write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT,0x00000014);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR 
write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT,0x00000014);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR
write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR


write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE,0x00000000);

//Enabling RX QUEUE THRESHLOD = 128 Bytes,RX QUEUE STR&FWD,Forward error packets
write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE,0x00000033);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE,0x00000033);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE,0x00000033);

//TXQUEUE 0 and RX QUEUE0
write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR
write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE,0x00000033);

//pROGRAMMING RX Queue Size to 4096 Bytes
write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE,0x0000033);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE,0x0000033);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE,0x0000033);


//MAPPING OF QUEUES TO DMA CHANNEL QUEUEX TO DMA CHANNEL X(i.e. 0 to 0,1 to 1...etc)
write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0,0x03020100);

//RX QUEUE Overflow and TX QUEUE Underflow Interrupt Enable
write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS,0x01000100);
write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS,0x01000100);
write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS,0x01000100);

//Programming Queue Weights and Enabling Recieve Queue Arbitration
write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_CONTROL,0x2);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_CONTROL,0x4);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_CONTROL,0x8);


//Burst Length Programmed with 16 Beats and Transmit Channel weight assigned is 6
write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL,0x00100006);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL,0x00100006);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL,0x00100006);

//pROGRAMMING RX Queue Size to 4096 Bytes

write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE,0x00f00033);

//Base address of the first descriptor in the Transmit descriptor list
write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS,0x00300000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS,0x00200000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS,0x00100000);

//Transmit Descriptor Ring Length programmed to 32 ,Can be programmed upto 1024
write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH,0x0000001f);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH,0x0000001f);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH,0x0000001f);

//RX QUEUE Overflow and TX QUEUE Underflow Interrupt Enable
write_enet_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS,0x01000100);


write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL,0x0);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL,0x0);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL,0x0);

//Programming Queue Weights and Enabling Recieve Queue Arbitration
write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_CONTROL,0x5);

//16 Beats and RX Buffer size 
write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL,0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL,0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL,0x00101000);

//Outstanding request for write =1 ,read = 4,FB,AALE,Burst selection to 16,8,4 and max possible = 8
write_enet_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE,0x0103000e);

//Transmit Descriptor Ring Length programmed to 32 ,Can be programmed upto 1024
write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS,0x00304000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS,0x00204000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS,0x00104000);

//Reciever Descriptor Ring Length programmed to 32 ,Can be programmed upto 1024
write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL2,0x0000001f);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL2,0x0000001f);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL2,0x0000001f);

preload_descriptor_rx(0xE6040000,0xE6044000,0x10,0xB);

//Burst Length Programmed with 16 Beats and Transmit Channel weight assigned is 6
write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL,0x00100006);


//Base address of the first descriptor in the Transmit descriptor list
write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS,0xE6000000);
#ifdef DEBUG_DISPLAY
	printf("Power of enet_sel : %d\n",power);
#endif
write_reg(0XE68C2058,power);

//Enabling Interrupts
write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE,0x000F9C0);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE,0x000F9C0);
write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE,0x000F9C0);

//Transmit Descriptor Ring Length programmed to 32 ,Can be programmed upto 1024
write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH,0x0000001f);


//16 Beats and RX Buffer size 
write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL,0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL,0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL,0x00101000);

write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL,0x00000000);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL,0x00101000);
//Transmit Descriptor Ring Length programmed to 32 ,Can be programmed upto 1024
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS,0xE6040000);
//Reciever Descriptor Ring Length programmed to 32 ,Can be programmed upto 1024
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2,0x00000009);
//Enabling Interrupts
write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE,0x000F9C0);
write_enet_reg(mizar_ETHERNET0_MAC_INTERRUPT_ENABLE,0x0004000);
write_enet_reg(mizar_ETHERNET0_DMA_MODE,0x0000000);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL,0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER,0xE6044800);//RX Change
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL,0x00101001);//RX Change Start RX
write_reg(0xA0243ffc,enet_sel);				//For ETH VIP sequencer start
write_reg(0xA0243ff8,0xdeadbeef);				//For ETH VIP sequencer start

for(i=0;i<trns_count;i++)
{
    while(int_pend)
    {
        printf("--> %d Waiting for transfer complete interrupt\n",i);
        wait_on(10);
    }
    int_pend = 1;
 }
 wait_on(2000);
    finish(0);
}
void Default_IRQHandler(){


    int rdata,wdata;
    unsigned int irq_no ;
    int_pend=0;
    irq_no = 42 + enet_select;
    enet_sel = enet_select;

    printf("IN THE HANDLER %d for eth%d\n",irq_no,enet_select);
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXBUFFER);
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXBUFFER);

    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXBUFFER);

    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXBUFFER);
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);

    printf("ETH DMA INT STS %x\n",rdata);
 
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_STATUS,wdata); //write 1 to clear status

    rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);

    printf("ETH DMA INT STS %x\n",rdata);
    write_reg(0XE68C2050,power);
    GIC_ClearIRQ(irq_no);

    printf("Clr IRQ\n\n");

}

