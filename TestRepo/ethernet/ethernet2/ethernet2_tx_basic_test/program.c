#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include<ethernet2/ethernet2_def.h>
#include<ethernet2/ethernet2_offset.h>
#include<ethernet0/ethernet0_programming_sequence.h>
int data_rd,data_wr;
int def_fail_cnt = 0,wr_fail_cnt = 0;
extern int int_pend;


int test_case()
{
    int_pend = 1;
    int wdata,rdata,i;
    int address,read;
    //DMA Channel selection,Address Enabled and upper 16 bits[47:32] of the 2nd 6-byte MAC address Programming
    write_reg(mizar_ETHERNET2_MAC_ADDRESS3_HIGH,0x80033607); //addr_en and dma_channel_sel = 3 and addr[47:32] = 3607 
    write_reg(mizar_ETHERNET2_MAC_ADDRESS2_HIGH,0x80022607); //addr_en and dma_channel_sel = 2 and addr[47:32] = 2607
    write_reg(mizar_ETHERNET2_MAC_ADDRESS1_HIGH,0x80011607);
    write_reg(mizar_ETHERNET2_MAC_ADDRESS0_HIGH,0x80000607);

    //lower 32 bits of the 2nd 6-byte MAC address
    write_reg(mizar_ETHERNET2_MAC_ADDRESS3_LOW,0x08090a00); //addr[31:0]
    write_reg(mizar_ETHERNET2_MAC_ADDRESS2_LOW,0x08090a00);
    write_reg(mizar_ETHERNET2_MAC_ADDRESS1_LOW,0x08090a00);
    write_reg(mizar_ETHERNET2_MAC_ADDRESS0_LOW,0x08090a00);


    write_reg(mizar_ETHERNET2_MAC_PACKET_FILTER,0x80000400);   //MAC_PACKET_FILTER =>  HASH FILTER Enabled and Receive All packets regardless of filter

    write_reg(mizar_ETHERNET2_MAC_CONFIGURATION,0x2003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed

    write_reg(mizar_ETHERNET2_MAC_EXT_CONFIGURATION,0x0);

    //MTL Register programming

    write_reg(mizar_ETHERNET2_MTL_TXQ3_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q3 not Enabled
    write_reg(mizar_ETHERNET2_MTL_TXQ2_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q2 not Enabled
    write_reg(mizar_ETHERNET2_MTL_TXQ1_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q1 not Enabled
    write_reg(mizar_ETHERNET2_MTL_TXQ0_OPERATION_MODE,0x000f000a);//4096 Bytes TX QUEUE Size,STR&FWD Enabled,TX Q0 Enabled

    write_reg(mizar_ETHERNET2_MTL_TXQ3_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR 
    write_reg(mizar_ETHERNET2_MTL_TXQ2_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR
    write_reg(mizar_ETHERNET2_MTL_TXQ1_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR
    write_reg(mizar_ETHERNET2_MTL_TXQ0_QUANTUM_WEIGHT,0x00000005);//Quamtum/Weight Values need to be programmed based on WRR,WFQ,DWRR

    //RX QUEUE Overflow and TX QUEUE Underflow Interrupt Enable
    write_reg(mizar_ETHERNET2_MTL_Q3_INTERRUPT_CONTROL_STATUS,0x01000100);
    write_reg(mizar_ETHERNET2_MTL_Q2_INTERRUPT_CONTROL_STATUS,0x01000100);
    write_reg(mizar_ETHERNET2_MTL_Q1_INTERRUPT_CONTROL_STATUS,0x01000100);
    write_reg(mizar_ETHERNET2_MTL_Q0_INTERRUPT_CONTROL_STATUS,0x01000100);

    //DMA register programming

    //Outstanding request for write =1 ,read = 4,FB,AALE,Burst selection to 16,8,4 and max possible = 8
    write_reg(mizar_ETHERNET2_DMA_SYSBUS_MODE,0x0103000e);

    preload_descriptor(0xE6000000,0xE6008000,0x3c,0x5); 
    preload_descriptor(0xE6000050,0xE600812c,0x5E8,0x5); 
    //preload_descriptor(0x00200000,0x00208000,0xe,0x60); 
    //preload_descriptor(0x00100000,0x00108000,0xe,0x60); 
    //preload_descriptor(0x00000000,0x00008000,0xe,0x60); 

    //Transmit Descriptor Ring Length programmed to 32 ,Can be programmed upto 1024
    write_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_RING_LENGTH,0x000000A);
    write_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_RING_LENGTH,0x000000A);
    write_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_RING_LENGTH,0x000000A);
    write_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_RING_LENGTH,0x000000A);

    //Base address of the first descriptor in the Transmit descriptor list
    write_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_LIST_ADDRESS,0x00300000);
    write_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_LIST_ADDRESS,0x00200000);
    write_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_LIST_ADDRESS,0x00100000);
    write_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_LIST_ADDRESS,0xE6000000);

    write_reg(mizar_ETHERNET2_DMA_CH3_TXDESC_TAIL_POINTER,0x00309730);
    write_reg(mizar_ETHERNET2_DMA_CH2_TXDESC_TAIL_POINTER,0x00209730);
    write_reg(mizar_ETHERNET2_DMA_CH1_TXDESC_TAIL_POINTER,0x00109730);
    write_reg(mizar_ETHERNET2_DMA_CH0_TXDESC_TAIL_POINTER,0xE6009EB4);

    write_reg(mizar_ETHERNET2_DMA_CH3_CONTROL,0x0);
    write_reg(mizar_ETHERNET2_DMA_CH2_CONTROL,0x0);
    write_reg(mizar_ETHERNET2_DMA_CH1_CONTROL,0x0);
    write_reg(mizar_ETHERNET2_DMA_CH0_CONTROL,0x00000000);

    //Enabling Interrupts
    write_reg(mizar_ETHERNET2_DMA_CH1_INTERRUPT_ENABLE,0x000F0C7);
    write_reg(mizar_ETHERNET2_DMA_CH2_INTERRUPT_ENABLE,0x000F0C7);
    write_reg(mizar_ETHERNET2_DMA_CH3_INTERRUPT_ENABLE,0x000F0C7);
    write_reg(mizar_ETHERNET2_DMA_CH0_INTERRUPT_ENABLE,0x000F0C7);
   //Enabling HSS Autoreg Ethernet Interrupts 
write_reg(0XE68C2058,0x4);

    //write_reg(mizar_ETHERNET2_DMA_CH3_TX_CONTROL,0x00100007); //START TX
    //write_reg(mizar_ETHERNET2_DMA_CH2_TX_CONTROL,0x00100007); //START TX
    //write_reg(mizar_ETHERNET2_DMA_CH1_TX_CONTROL,0x00100007); //START TX
    write_reg(mizar_ETHERNET2_DMA_CH0_TX_CONTROL,0x00100007); //START TX
    for(i=0;i<10;i++)
    {
    while(int_pend)
    {
        printf("---->%d Waiting for transfer complete interrupt\n",i);
        wait_on(10);
    }
    int_pend = 1;
    }
    wait_on(2000);
    finish(0);
}
void Default_IRQHandler(){


    int rdata,wdata;
    int_pend=0;

    printf("IN THE HANDLER");
    printf("printing pointers :\n");
    read_reg(mizar_ETHERNET2_DMA_CH0_CURRENT_APP_TXDESC, rdata);
    printf("mizar_ETHERNET2_DMA_CH0_CURRENT_APP_TXDESC = %x", rdata);	
    read_reg(mizar_ETHERNET2_DMA_CH0_CURRENT_APP_TXBUFFER, rdata);
    printf("mizar_ETHERNET2_DMA_CH0_CURRENT_APP_TXBUFFER = %x\n", rdata);

    read_reg(mizar_ETHERNET2_DMA_CH1_CURRENT_APP_TXDESC, rdata);
    printf("mizar_ETHERNET2_DMA_CH1_CURRENT_APP_TXDESC = %x", rdata);	
    read_reg(mizar_ETHERNET2_DMA_CH1_CURRENT_APP_TXBUFFER, rdata);
    printf("mizar_ETHERNET2_DMA_CH1_CURRENT_APP_TXBUFFER = %x\n", rdata);

    read_reg(mizar_ETHERNET2_DMA_CH2_CURRENT_APP_TXDESC, rdata);
    printf("mizar_ETHERNET2_DMA_CH2_CURRENT_APP_TXDESC = %x", rdata);	
    read_reg(mizar_ETHERNET2_DMA_CH2_CURRENT_APP_TXBUFFER, rdata);
    printf("mizar_ETHERNET2_DMA_CH2_CURRENT_APP_TXBUFFER = %x\n", rdata);

    read_reg(mizar_ETHERNET2_DMA_CH3_CURRENT_APP_TXDESC, rdata);
    printf("mizar_ETHERNET2_DMA_CH3_CURRENT_APP_TXDESC = %x", rdata);	
    read_reg(mizar_ETHERNET2_DMA_CH3_CURRENT_APP_TXBUFFER, rdata);
    printf("mizar_ETHERNET2_DMA_CH3_CURRENT_APP_TXBUFFER = %x\n", rdata);

    printf("printing dma int status : \n");
    read_reg(mizar_ETHERNET2_DMA_INTERRUPT_STATUS,rdata);
    printf("mizar_ETHERNET2_DMA_INTERRUPT_STATUS = %x\n",rdata);

  //  read_reg(mizar_ETHERNET2_DMA_CH0_STATUS, rdata);
  //  printf("mizar_ETHERNET2_DMA_CH0_STATUS = %x", rdata);
  //  read_reg(mizar_ETHERNET2_DMA_CH1_STATUS, rdata);
  //  printf("mizar_ETHERNET2_DMA_CH1_STATUS = %x", rdata);
  //  read_reg(mizar_ETHERNET2_DMA_CH2_STATUS, rdata);
  //  printf("mizar_ETHERNET2_DMA_CH2_STATUS = %x", rdata);
  //  read_reg(mizar_ETHERNET2_DMA_CH3_STATUS, rdata);
  //  printf("mizar_ETHERNET2_DMA_CH3_STATUS = %x", rdata);

    printf("clearing individual dma int status : \n");
    wdata = 0xffffffff;
    write_reg(mizar_ETHERNET2_DMA_CH0_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_reg(mizar_ETHERNET2_DMA_CH1_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_reg(mizar_ETHERNET2_DMA_CH2_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_reg(mizar_ETHERNET2_DMA_CH3_STATUS,wdata); //write 1 to clear status

    printf("printing dma int status : \n");
    read_reg(mizar_ETHERNET2_DMA_INTERRUPT_STATUS,rdata);
    printf("mizar_ETHERNET2_DMA_INTERRUPT_STATUS = %x\n",rdata);

  //  read_reg(mizar_ETHERNET2_DMA_CH0_STATUS, rdata);
  //  printf("mizar_ETHERNET2_DMA_CH0_STATUS = %x", rdata);
  //  read_reg(mizar_ETHERNET2_DMA_CH1_STATUS, rdata);
  //  printf("mizar_ETHERNET2_DMA_CH1_STATUS = %x", rdata);
  //  read_reg(mizar_ETHERNET2_DMA_CH2_STATUS, rdata);
  //  printf("mizar_ETHERNET2_DMA_CH2_STATUS = %x", rdata);
  //  read_reg(mizar_ETHERNET2_DMA_CH3_STATUS, rdata);
  //  printf("mizar_ETHERNET2_DMA_CH3_STATUS = %x", rdata);
    write_reg(0XE68C2050,0x4);

    printf("Clearing Interruprt\n\n");

}
