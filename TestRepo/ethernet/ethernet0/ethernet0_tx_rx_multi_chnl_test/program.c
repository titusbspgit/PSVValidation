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
extern int int_pend;

int test_case()
{
    int_pend = 1;
    int wdata,rdata,i,txpkt,rxpkt;
    int address,read;
    enetsel_randvar = rand();
    sel_enet = (enetsel_randvar%4)*0x100000;
    GIC_Set();
    GIC_EnableAllIRQ(); //Enable all interrupt

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
    //addr --> 010203040506
    //102030405060
    //605040302010
    write_reg(0xA0243ffc,enet_sel);
   // write_reg(0xA0243ff8,0xdeadbeef);				//For ETH VIP sequencer start

    //DMA Channel selection,Address Enabled and upper 16 bits[47:32] of the 2nd 6-byte MAC address Programming
/*    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH,0x80033607); //addr_en and dma_channel_sel = 3 and addr[47:32] = 3607 
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH,0x80022607); //addr_en and dma_channel_sel = 2 and addr[47:32] = 2607
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH,0x80011607);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH,0x80000607);

    //lower 32 bits of the 2nd 6-byte MAC address
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW,0x08090a0b); //addr[31:0]
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW,0x08090a0b);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW,0x08090a0b);
    write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW,0x08090a0b);


    write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER,0x00000408);   //MAC_PACKET_FILTER =>  HASH FILTER Enabled and Receive All packets regardless of filter
    */

#ifdef ETH_10M
    #ifdef HALF_DUPLEX
        write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0x8003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed
    #else
        write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xA003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed
    #endif
    write_reg((0xE68C205C + (enet_sel*4)),0x1A08); 	//Enabling Override
    wait_on(300); 	             //wait_on applied
    write_reg((0xE68C205C + (enet_sel*4)),0x1EC8); 	//reg_reprogram enabled ,1700 MHz/136 = 12.5 half pulse
    wait_on(300); 	                 //wait_on applied
    write_reg((0xE68C205C + (enet_sel*4)),0x18C8); 	//reg_reprogram enabled ,1700 MHz/136 = 12.5 half pulse

#elif ETH_100M
    #ifdef HALF_DUPLEX
        write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xC003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed
    #else
        write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION,0xE003); 	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed
    #endif
    write_reg((0xE68C205C + (enet_sel*4)),0x1A08); 	//Enabling Override
    wait_on(300); 	                //wait_on applied
    write_reg((0xE68C205C + (enet_sel*4)),0x1E14); 	//reg_reprogram enabled ,1700 MHz/136 = 12.5 half pulse
    wait_on(300); 	                //wait_on applied
    write_reg((0xE68C205C + (enet_sel*4)),0x1814); 	//reg_reprogram release,500 MHz

#else
	write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION , 0x00002003);
     	//MAC_CONFIGURATION REGISTER i.e Enabling TX and RX ,Full Duplex mode selected,Speed
#endif

write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION,0x0); //0x0
write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0 , 0x000000aa); //A0
write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1 , 0x00000000); //A4
write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2 , 0x08040201); //A8 
//94 not programmed
write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL , 0x01600000); //50
write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER,0x80000408); //0x8 
write_enet_reg(mizar_ETHERNET0_MMC_IPC_RX_INTERRUPT_MASK , 0x80000400);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH , 0x80033607);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH , 0x80022607);
rdata = read_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH , 0x80011607);
rdata = read_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH , 0x80000607);
rdata = read_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW , 0x08090a0b);
rdata = read_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW , 0x08090a0b);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW , 0x08090a0b);
rdata = read_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW);
write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW , 0x08090a0b);
rdata = read_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW);
//060708090a00
write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE , 0x0003000a);//1kb
write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE , 0x0003000a);
write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE , 0x0003000a);
write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE , 0x0003000a);
write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT , 0x00000014);
write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT , 0x00000014);
write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT , 0x00000005);
write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE , 0x00000000); //0xc00
write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT , 0x00000005);

write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE , 0x00f00033);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE , 0x00f00033);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE , 0x00f00033);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE , 0x00f00033);

write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS , 0x01000100);
write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS , 0x01000100);
write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS , 0x01000100);
write_enet_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS , 0x01000100);

write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0 , 0x03020100);
rdata = read_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_CONTROL , 0x00000007);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_CONTROL , 0x00000007);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_CONTROL , 0x00000007);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_CONTROL , 0x00000007);

write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL , 0x00100006);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL , 0x00100006);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL , 0x00100006);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL , 0x00100006);

write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS , 0xE6030000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS , 0xE6020000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS , 0xE6010000);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS ,0xE6000000);

write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH , 0x9);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH , 0x9);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH , 0x9);
write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL , 0x00000000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL , 0x00000000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL , 0x00000000);
write_enet_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE, 0x0103000e);
write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL , 0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL , 0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL , 0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL , 0x00101000);

    preload_descriptor(0xE6000000,0xE6001000,0x3c,0x5); 
    preload_descriptor(0xE6000050,0xE600112c,0x5E8,0x5); 
    preload_descriptor(0xE6010000,0xE6011000,0x3c,0x5); 
    preload_descriptor(0xE6010050,0xE601112c,0x5E8,0x5); 
    preload_descriptor(0xE6020000,0xE6021000,0x3c,0x5); 
    preload_descriptor(0xE6020050,0xE602112c,0x5E8,0x5); 
    preload_descriptor(0xE6030000,0xE6031000,0x3c,0x5); 
    preload_descriptor(0xE6030050,0xE603112c,0x5E8,0x5);

write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS ,0xE6070000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS ,0xE6060000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS ,0xE6050000);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS , 0xE6040000);

preload_descriptor_rx(0xE6040000,0xE6044000,0x10,0xA);
preload_descriptor_rx(0xE6050000,0xE6054000,0x10,0xA);
preload_descriptor_rx(0xE6060000,0xE6064000,0x10,0xA);
preload_descriptor_rx(0xE6070000,0xE6074000,0x10,0xA);
 write_reg(0XE68C2058,power);

write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL2 ,0x9);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL2 ,0x9);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL2 ,0x9);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH ,0x9);
write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE ,0xf0df);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE ,0xf0df);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE ,0xf0df); 
write_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER,0xE60298a4);//1220
write_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER,0xE60398a4);

write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL , 0x00000000);
write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL , 0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL , 0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL , 0x00101000);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL , 0x00101000);

write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2 , 0x9);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE ,0xf0df);
write_enet_reg(mizar_ETHERNET0_DMA_MODE , 0x00000000);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL , 0x00101000);
//write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER , 0xE6079EB4);
//write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER , 0xE6069EB4);
//write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER , 0xE6059EB4);
//write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER , 0xE6049EB4);

write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0 , 0x00000083);
write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER , 0xE6070010);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER , 0xE6060010);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER , 0xE6050010);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER , 0xE6040010);
write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL , 0x00101001);
/*write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL , 0x00101001);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL , 0x00101001);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL , 0x00101001);*/
write_reg(0xA0243ffc,enet_sel);
    write_reg(0xA0243ff8,0xdeadbeef);				//For ETH VIP sequencer start

write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER , 0xE6039EB4);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER , 0xE6029EB4);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER , 0xE6019EB4);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER , 0xE6009EB4);

write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL , 0x00100007);
    
    for(i=0;i<20;i++)
    {
    while(int_pend)
    {
      //  printf("---->%d Waiting for transfer complete interrupt\n",i);
        wait_on(10);

    rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
   // printf("mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD = %x\n", rxpkt);
    txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
   // printf("mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD = %x\n", txpkt);

    if(rxpkt == 10 && txpkt == 10){
   //     printf("This is TRUE\n");
	break;
	}
    }
    int_pend = 1;
    }
write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL , 0x00100007);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0 , 0x00000082);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER , 0xE6060010);
write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL , 0x00101001);
write_reg(0xA0243ff8,0xdeadbeee);				//For ETH VIP sequencer start
    for(i=0;i<20;i++)
    {
    while(int_pend)
    {
      //  printf("---->%d Waiting for transfer complete interrupt\n",i);
        wait_on(10);

    rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
  //  printf("mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD = %x\n", rxpkt);
    txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
  //  printf("mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD = %x\n", txpkt);

    if(rxpkt == 20 && txpkt == 20){
  //      printf("This is TRUE\n");
	break;
	}
    }
    int_pend = 1;
    }
write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL , 0x00100007);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0 , 0x00000081);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER , 0xE6050010);
write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL , 0x00101001);
write_reg(0xA0243ff8,0xdeadbeed);				//For ETH VIP sequencer start
    for(i=0;i<20;i++)
    {
    while(int_pend)
    {
       // printf("---->%d Waiting for transfer complete interrupt\n",i);
        wait_on(10);

    rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
  //  printf("mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD = %x\n", rxpkt);
    txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
  //  printf("mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD = %x\n", txpkt);

    if(rxpkt == 30  && txpkt == 30){
      //  printf("This is TRUE\n");
	break;
	}
    }
    int_pend = 1;
    }
write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL , 0x00100007);
write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0 , 0x00000080);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER , 0xE6040010);
write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL , 0x00101001);
write_reg(0xA0243ff8,0xdeadbeec);				//For ETH VIP sequencer start
    for(i=0;i<20;i++)
    {
    while(int_pend)
    {
   //     printf("---->%d Waiting for transfer complete interrupt\n",i);
        wait_on(10);

    rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 //   printf("mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD = %x\n", rxpkt);
    txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 //   printf("mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD = %x\n", txpkt);

    if(rxpkt == 40  && txpkt == 40){
 //       printf("This is TRUE\n");
	break;
	}
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
    int rdata_tailpointer;
    int rdata_desc;
    irq_no = 42 + enet_select;
    enet_sel = enet_select;
//    printf("IN THE HANDLER");

/*    rdata = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
  //  printf("mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD = %x\n", rdata);
    rdata = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
//    printf("mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD = %x\n", rdata);

//    printf("printing pointers :\n");
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXDESC);
//    printf("mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXDESC = %x", rdata);	
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXBUFFER);
//    printf("mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXBUFFER = %x\n", rdata);

    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXDESC);
//    printf("mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXDESC = %x", rdata);	
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXBUFFER);
//    printf("mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXBUFFER = %x\n", rdata);

    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXDESC);
//    printf("mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXDESC = %x", rdata);	
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXBUFFER);
 //   printf("mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXBUFFER = %x\n", rdata);

    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);
//    printf("mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC = %x", rdata);	
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);
//    printf("mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER = %x\n", rdata);
    */
    rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
//    printf("mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC = %x", rdata_desc);
    rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER);
   if(rdata_tailpointer == rdata_desc)
   {
   if(rdata_tailpointer == 0xE60400f0)
   {
      #ifdef PRELOAD_AGAIN
	preload_descriptor_rx(0xE6040000,0xE6044000,0x10,0x10);
   #endif

   write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER , 0xE6040000);
   }
   else
   {
   write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER , (rdata_tailpointer + 0x10));
   }

   }

  rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
    printf("mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC = %x", rdata_desc);
    rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER);
   if(rdata_tailpointer == rdata_desc)
   {
   if(rdata_tailpointer == 0xE60500f0)
   {
   #ifdef PRELOAD_AGAIN
	preload_descriptor_rx(0xE6050000,0xE6054000,0x10,0x10);
   #endif
   write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER , 0xE6050000);
   }
   else
   {
   write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER , (rdata_tailpointer + 0x10));
   }

   }

 rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
    printf("mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC = %x", rdata_desc);
    rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER);
   if(rdata_tailpointer == rdata_desc)
   {
   if(rdata_tailpointer == 0xE60600f0)
   {
      #ifdef PRELOAD_AGAIN
	preload_descriptor_rx(0xE6060000,0xE6064000,0x10,0x10);
   #endif

   write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER , 0xE6060000);
   }
   else
   {
   write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER , (rdata_tailpointer + 0x10));
   }

   }

  rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
   // printf("mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC = %x", rdata_desc);
    rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER);
   if(rdata_tailpointer == rdata_desc)
   {
   if(rdata_tailpointer == 0xE60700f0)
   {
      #ifdef PRELOAD_AGAIN
	preload_descriptor_rx(0xE6070000,0xE6074000,0x10,0x10);
   #endif

   write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER , 0xE6070000);
   }
   else
   {
   write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER , (rdata_tailpointer + 0x10));
   }

   }


 //   printf("printing dma int status : \n");
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
 //   printf("mizar_ETHERNET0_DMA_INTERRUPT_STATUS = %x\n",rdata);

  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH0_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH1_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH2_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH3_STATUS = %x", rdata);

  //  printf("clearing individual dma int status : \n");
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH1_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH2_STATUS,wdata); //write 1 to clear status
    wdata = 0xffffffff;
    write_enet_reg(mizar_ETHERNET0_DMA_CH3_STATUS,wdata); //write 1 to clear status

 //   printf("printing dma int status : \n");
    rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
 //   printf("mizar_ETHERNET0_DMA_INTERRUPT_STATUS = %x\n",rdata);

  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH0_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH1_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH2_STATUS = %x", rdata);
  //  rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_STATUS);
  //  printf("mizar_ETHERNET0_DMA_CH3_STATUS = %x", rdata);
    write_reg(0XE68C2050,power);
    GIC_ClearIRQ(42+enet_sel);
    printf("Clearing Interruprt\n\n");
}

