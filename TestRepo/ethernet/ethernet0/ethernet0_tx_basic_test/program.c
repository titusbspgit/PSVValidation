#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <test_common.h>
#include<ethernet0/ethernet0_def.h>
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
    int wdata,rdata,i;
    int address,read;
    GIC_Set();
    GIC_EnableAllIRQ(); //Enable all interrupt

    #ifdef ETH_10M
	enet_10m_speed();
    #elif ETH_100M
    	enet_100m_speed();
    #endif

    #ifdef ETH_10M
	trns_count = 6;
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

	non_secure_prot_nic();    
    //DMA Channel selection,Address Enabled and upper 16 bits[47:32] of the 2nd 6-byte MAC address Programming
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH,0x80033607); //addr_en and dma_channel_sel = 3 and addr[47:32] = 3607 
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH,0x80022607); //addr_en and dma_channel_sel = 2 and addr[47:32] = 2607
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH,0x80011607);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH,0x80000607);

    //lower 32 bits of the 2nd 6-byte MAC address
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW,0x08090a00); //addr[31:0]
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW,0x08090a00);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW,0x08090a00);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW,0x08090a00);


    write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER,0x80000400);   //MAC_PACKET_FILTER =>  HASH FILTER Enabled and Receive All packets regardless of filter


    #ifdef ETH_10M
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

    write_reg(0xA0243ffc,enet_sel);				//For ETH VIP sequencer start
    write_reg(0xA0243ff8,0xdeadbeef);				//For ETH VIP sequencer start

    write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION,0x0);

    //MTL Register programming

    write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q3 not Enabled
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q2 not Enabled
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q1 not Enabled
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q0 Enabled

    write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR 
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR
    write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR

    //RX QUEUE Overflow and TX QUEUE Underflow Interrupt Enable
    write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS,0x01000100);
    write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS,0x01000100);
    write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS,0x01000100);
    write_enet_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS,0x01000100);

    //DMA register programming

    //Outstanding request for write =1 ,read = 4,FB,AALE,Burst selection to 16,8,4 and max possible = 8
    write_enet_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE,0x0103000e);

    preload_descriptor(0xE6000000,0xE6008000,0x3c,0x5); 
    preload_descriptor(0xE6000050,0xE600812c,0x5E8,0x5); 
//    preload_descriptor(0xE6000000,0xE600812C,0x3c,0x5); 
    //preload_descriptor(0x00200000,0x00208000,0xe,0x60); 
    //preload_descriptor(0x00100000,0x00108000,0xe,0x60); 
    //preload_descriptor(0x00000000,0x00008000,0xe,0x60); 

    //Transmit Descriptor Ring Length programmed to 32 ,Can be programmed upto 1024
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH,0x000000A);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH,0x000000A);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH,0x000000A);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH,0x000000A);

    //Base address of the first descriptor in the Transmit descriptor list
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS,0x00300000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS,0x00200000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS,0x00100000);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS,0xE6000000);

    write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER,0x00309730);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER,0x00209730);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER,0x00109730);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER,0xE6009EB4);

    write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL,0x0);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL,0x0);
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL,0x0);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL,0x00000000);

    //Enabling Interrupts
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE,0x000F0C7);
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE,0x000F0C7);
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE,0x000F0C7);
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE,0x000F0C7);
   //Enabling HSS Autoreg Ethernet Interrupts
   #ifdef DEBUG_DISPLAY   
   	printf("Power of enet_sel : %d\n",power);
   #endif
    write_reg(0XE68C2058,power);

    //write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL,0x00100007); //START TX
    //write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL,0x00100007); //START TX
    //write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL,0x00100007); //START TX
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL,0x00100007); //START TX
    for(i=0;i<trns_count;i++)
    {
    while(int_pend)
    {
        #ifdef DEBUG_DISPLAY
	printf("---->%d  trnsf intrpt\n",i);
	#endif
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

#ifdef DEBUG_DISPLAY
    printf("ENET IRQ NO = %x\n", irq_no);	
    printf("printing pointers :\n");
#endif
   
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
#ifdef DEBUG_DISPLAY
    printf("mizar_ETHERNET0_DMA_INTERRUPT_STATUS = %x\n",rdata);
    printf("clearing individual dma int status : \n");
#endif
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS,wdata); //write 1 to clear status
   
#ifdef DEBUG_DISPLAY
    printf("printing dma int status : \n");
#endif
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
#ifdef DEBUG_DISPLAY
    printf("mizar_ETHERNET0_DMA_INTERRUPT_STATUS = %x\n",rdata);
#endif

  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH0_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH1_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH2_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH3_STATUS = %x", rdata);
    write_reg(0XE68C2050,power);
    #ifdef CLRIRQ_42
    GIC_ClearIRQ(42);
    #else
    GIC_ClearIRQ(irq_no);
    #endif
    

}

