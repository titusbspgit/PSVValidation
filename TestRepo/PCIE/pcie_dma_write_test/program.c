#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

unsigned int data_rd,data_wr;
int err2 = 0;
int err1 = 0;
extern unsigned int int_pend;
  unsigned int src_addr0;
  unsigned int dst_addr0;
  unsigned int rd_addr0;
  unsigned int wr_addr0;
  unsigned int src_addr1;
  unsigned int dst_addr1;
  unsigned int rd_addr1;
  unsigned int wr_addr1;
  unsigned int len;
  unsigned int src_addr2;
  unsigned int dst_addr2;
  unsigned int rd_addr2;
  unsigned int wr_addr2;
  unsigned int src_addr3;
  unsigned int dst_addr3;
  unsigned int rd_addr3;
  unsigned int wr_addr3;
  unsigned int rd_wr_data1;
int test_case()
{
int i;
write_reg(0xE6004100,0x0);	
#ifdef DM0_RC
link_training_dm0_x4(4);  
#endif
#ifdef DM1_RC
link_training_dm1_x4(4);  
#endif
#ifdef DM0_EP
link_training_dm0_x4(4);  
#endif
#ifdef DM1_EP
link_training_dm1_x4(4);  
#endif
#ifdef DEBUG_DISPLAY
printf("before initial wait on\n");
#endif

//FOR DM0_RC
#ifdef DM0_RC
data_rd = read_sii0_reg(0xC0);
#ifdef DEBUG_DISPLAY
printf("TEST SII0 READ DATA data before entering = %x\n",data_rd);
#endif
while(((data_rd)&(0xD1))!=0xD1)
{
   data_rd = read_sii0_reg(0xC0);
   #ifdef DEBUG_DISPLAY
   printf("TEST SII0 READ DATA data = %x\n",data_rd);
   #endif
}
#endif


#ifdef DM0_RC 
#ifdef DEBUG_DISPLAY
printf("Reading Vendor ID\n");
#endif
rd_wr_data1 = read_pcie_slv0_reg(0x0);
printf("VENDOR ID : 0x%x\n",rd_wr_data1);
write_pcie_slv0_reg(0x4,0x7);
//rd_wr_data1 = read_pcie_slv0_reg(0x4);
//printf("COMMAND REG : 0x%x\n",rd_wr_data1);
bar_program_dm0_x4();
#ifdef DEBUG_DISPLAY
printf("\nMemory base Programming Started\n");
#endif
wait_on(10);
mem_base_program_dm0_x4();
#endif


//FOR DM1_RC
#ifdef DM1_RC
data_rd = read_sii1_reg(0xC0);
#ifdef DEBUG_DISPLAY
printf("TEST SII1 READ DATA data before entering = %x\n",data_rd);
#endif
while(((data_rd)&(0xD1))!=0xD1)
{
   data_rd = read_sii1_reg(0xC0);
   #ifdef DEBUG_DISPLAY
   printf("TEST SII1 READ DATA data = %x\n",data_rd);
   #endif
}
#endif


#ifdef DM1_RC 
#ifdef DEBUG_DISPLAY
printf("Reading Vendor ID DM0\n");
#endif
rd_wr_data1 = read_pcie_slv1_reg(0x0);
printf("VENDOR ID : 0x%x\n",rd_wr_data1);
write_pcie_slv1_reg(0x4,0x7);
//rd_wr_data1 = read_pcie_slv0_reg(0x4);
//printf("COMMAND REG : 0x%x\n",rd_wr_data1);
bar_program_dm1_x4();
#ifdef DEBUG_DISPLAY
printf("\nMemory base Programming Started\n");
#endif
wait_on(10);
mem_base_program_dm1_x4();
#endif




non_secure_prot_nic();
data_rd = read_reg(0xE6004100); 
while(data_rd != 0x12345678)
{
    wait_on(5);
    data_rd = read_reg(0xE6004100); 
   
}
#ifdef DEBUG_DISPLAY
printf("after initial wait on\n");
#endif

	len = 0x40;
         src_addr0 = 0xE6000000; 
#ifdef DM0_RC 
	wr_addr0 =  0xA7000000;
        rd_addr0 =  0xA7000000;
        wr_addr1 =  0xA7002000;
        rd_addr1 =  0xA7002000;
        wr_addr2 =  0xA7003000;
        rd_addr2 =  0xA7003000;
        wr_addr3 =  0xA7004000;
        rd_addr3 =  0xA7004000;
#endif
#ifdef DM1_RC 
	wr_addr0 =  0xC7000000;
        rd_addr0 =  0xC7000000;
	wr_addr1 =  0xC7002000;
	rd_addr1 =  0xC7002000;
	wr_addr2 =  0xC7003000;
        rd_addr2 =  0xC7003000;
        wr_addr3 =  0xC7004000;
        rd_addr3 =  0xC7004000;
#endif

        dst_addr0 = 0xE6001000;
        dst_addr1 = 0xE6020000; 
        dst_addr2 = 0xE6020000; 
        dst_addr3 = 0xE6020000; 

	//Prelaod data//
	for (int i=0;i<128;i++)
	{
	write_reg( (src_addr0 + 4*i),0xC0DEBEED );
	}
        
        for (int i=0;i<128;i++)
	{
	write_reg( ((src_addr0+400) + 4*i),0xF00DDEAF );
	}
int_pend = 1;
GIC_Set();
GIC_EnableAllIRQ();
#ifdef DM0_RC

//   Perform all channel write transaction

	//
	write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF,0x0);
	write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF,0x0);
	#ifdef DEBUG_DISPLAY
        printf("DMA Channel wr_addr: %X, rd_addr0: %X, src_addr0: %X, len: %0d\n",wr_addr0,rd_addr0,src_addr0,len);
	#endif

       //-----------------DMA CHANNEL0----------------//
        #ifdef DEBUG_DISPLAY
        printf("//Programming DM0 Channel0 write\n");
	#endif
       	program_dma_wch0(0x0,src_addr0,0x0,wr_addr0,0x0,len);	
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF,0x0);
	while(int_pend) {
       		wait_on(10);
  	  }
	  int_pend = 1;
	  wait_on(10);
		
	//---------------DMA CHANNEL1----------------//
	#ifdef DEBUG_DISPLAY
	printf("Programming DM0 Channel1 write\n");
	#endif
       	program_dma_wch1(0x0,src_addr1,0x0,wr_addr1,0x0,len);	
	write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF,0x1);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
		
        //---------------DMA CHANNEL2----------------//
	 #ifdef DEBUG_DISPLAY
         printf("Programming DM0 Channel2 write\n");
	 #endif
         program_dma_wch2(0x0,src_addr2,0x0,wr_addr2,0x0,len);	
	 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF,0x2);
	 while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
	    
	//---------------DMA CHANNEL3----------------//
	#ifdef DEBUG_DISPLAY
        printf("Programming DM0 Channel3 write\n");
	#endif
        program_dma_wch3(0x0,src_addr3,0x0,wr_addr3,0x0,len);	
	write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF,0x3);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
		
     //---------------Perform All Chanel Read transaction----------------// 
     
        //-----------------DMA CHANNEL0----------------//
	#ifdef DEBUG_DISPLAY
        printf("Programming DM0 Channel0 read\n");
	#endif
        program_dma_rch0(0x0,rd_addr0,0x0,dst_addr0,0x0,len);	
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF,0x0);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);

        //---------------DMA CHANNEL1----------------//
	#ifdef DEBUG_DISPLAY
	printf("Programming DM0 Channel1 read\n");
	#endif
        program_dma_rch1(0x0,rd_addr1,0x0,dst_addr1,0x0,len);	
	write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF,0x1);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
		

        //---------------DMA CHANNEL2----------------//
	#ifdef DEBUG_DISPLAY
        printf("Programming DM0 Channel2 read\n");
	#endif
        program_dma_rch2(0x0,rd_addr2,0x0,dst_addr2,0x0,len);	
	write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF,0x2);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);

	//---------------DMA CHANNEL3----------------//
	#ifdef DEBUG_DISPLAY
        printf("Programming DM0 Channel3 read\n");
	#endif
        program_dma_rch3(0x0,rd_addr3,0x0,dst_addr3,0x0,len);	
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF,0x3);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
		
#endif

#ifdef DM1_RC

//   Perform all channel write transaction
	write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF,0x0);
	write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF,0x0);

	#ifdef DEBUG_DISPLAY
        printf("DMA Channel wr_addr: %X, rd_addr0: %X, src_addr0: %X, len: %0d\n",wr_addr0,rd_addr0,src_addr0,len);
	#endif

       //-----------------DMA CHANNEL0----------------//
        #ifdef DEBUG_DISPLAY
        printf("//Programming DM1 Channel0 write\n");
	#endif
       	program_dma1_wch0(0x0,src_addr0,0x0,wr_addr0,0x0,len);	
	write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF,0x0);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
	
	//---------------DMA CHANNEL1----------------//+
	#ifdef DEBUG_DISPLAY
	printf("Programming DM1 Channel1 write\n");
	#endif
       	program_dma1_wch1(0x0,src_addr1,0x0,wr_addr1,0x0,len);	
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF,0x1);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
	
        //---------------DMA CHANNEL2----------------//
	 #ifdef DEBUG_DISPLAY
         printf("Programming DM1 Channel2 write\n");
	 #endif
         program_dma1_wch2(0x0,src_addr2,0x0,wr_addr2,0x0,len);	
         write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF,0x2);
	 while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
	
    
	//---------------DMA CHANNEL3----------------//
	#ifdef DEBUG_DISPLAY
        printf("Programming DM1 Channel3 write\n");
	#endif
        program_dma1_wch3(0x0,src_addr3,0x0,wr_addr3,0x0,len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF,0x3);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
	      
     //---------------Perform All Chanel Read transaction----------------// 
     
        //-----------------DMA CHANNEL0----------------//
	#ifdef DEBUG_DISPLAY
        printf("Programming DM1 Channel0 read\n");
	#endif
        program_dma1_rch0(0x0,rd_addr0,0x0,dst_addr0,0x0,len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF,0x0);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);

        //---------------DMA CHANNEL1----------------//
	#ifdef DEBUG_DISPLAY
	printf("Programming DM1 Channel1 read\n");
	#endif
        program_dma1_rch1(0x0,rd_addr1,0x0,dst_addr1,0x0,len);	
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF,0x1);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
	

        //---------------DMA CHANNEL2----------------//
	#ifdef DEBUG_DISPLAY
        printf("Programming DM1 Channel2 read\n");
	#endif
        program_dma1_rch2(0x0,rd_addr2,0x0,dst_addr2,0x0,len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF,0x2);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);

	//---------------DMA CHANNEL3----------------//
	#ifdef DEBUG_DISPLAY
        printf("Programming DM1 Channel3 read\n");
	#endif
        program_dma1_rch3(0x0,rd_addr3,0x0,dst_addr3,0x0,len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF,0x3);
	while(int_pend) {
       		wait_on(10);
  	  } 
	  int_pend = 1;
	  wait_on(10);
	
#endif
   
	wait_on(10);
	
        finish(0);

}


void Default_IRQHandler(){
unsigned int dma_wr_int_sts;
unsigned int dma_rd_int_sts;
int_pend = 0;
unsigned int data = 0;
#ifdef DEBUG_DISPLAY
printf("entered DEFAULT IRQ HANDLER \n");
#endif
#ifdef DM0_RC
data_rd = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF); 
dma_wr_int_sts = data_rd & 0x0000000F;
data_rd = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF); 
dma_rd_int_sts = data_rd & 0x0000000F;


//clearing interrupt
write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF,dma_wr_int_sts);
//write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF,dma_wr_int_sts);
write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF,dma_rd_int_sts);
//write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF,dma_rd_int_sts);

if(dma_wr_int_sts != 0) {
	#ifdef DEBUG_DISPLAY
	printf("DM0 Dma write interrupt cleared, status : x%x \n",dma_wr_int_sts);
	#endif
}	

if(dma_rd_int_sts != 0) {
	#ifdef DEBUG_DISPLAY
	printf("DM0 Dma read channel interrupt cleared, status : x%x \n",dma_rd_int_sts);
	#endif
}	

GIC_ClearIRQ(0x20);
#endif

#ifdef DM1_RC
data_rd = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF); 
dma_wr_int_sts = data_rd & 0x0000000F;
data_rd = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF); 
dma_rd_int_sts = data_rd & 0x0000000F;


//clearing interrupt
write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF,dma_wr_int_sts);
//write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF,dma_wr_int_sts);
write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF,dma_rd_int_sts);
//write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF,dma_rd_int_sts);

if(dma_wr_int_sts != 0) {
	#ifdef DEBUG_DISPLAY
	printf("DM0 Dma write interrupt cleared, status : x%x \n",dma_wr_int_sts);
	#endif
}	

if(dma_rd_int_sts != 0) {
	#ifdef DEBUG_DISPLAY
	printf("DM0 Dma read channel interrupt cleared, status : x%x \n",dma_rd_int_sts);
	#endif
}	

GIC_ClearIRQ(0x23);

#endif
}
