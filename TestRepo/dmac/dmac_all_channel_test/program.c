#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "dmac.h"

#define SRC_ADDR_CH0 0xE6001000
#define SRC_ADDR_CH1 0xE6001100
#define SRC_ADDR_CH2 0xE6001200
#define SRC_ADDR_CH3 0xE6001300
#define SRC_ADDR_CH4 0xE6001400
#define SRC_ADDR_CH5 0xE6001500
#define SRC_ADDR_CH6 0xE6001600
#define SRC_ADDR_CH7 0xE6001700

#define DST_ADDR_CH0 0xE6003000
#define DST_ADDR_CH1 0xE6003100
#define DST_ADDR_CH2 0xE6003200
#define DST_ADDR_CH3 0xE6003300
#define DST_ADDR_CH4 0xE6003400
#define DST_ADDR_CH5 0xE6003500
#define DST_ADDR_CH6 0xE6003600
#define DST_ADDR_CH7 0xE6003700

#define DESC_ADDR_CH0 0x98000000
#define DESC_ADDR_CH1 0x98000100 
#define DESC_ADDR_CH2 0x98000200
#define DESC_ADDR_CH3 0x98000300
#define DESC_ADDR_CH4 0x98000400
#define DESC_ADDR_CH5 0x98000500
#define DESC_ADDR_CH6 0x98000600
#define DESC_ADDR_CH7 0x98000700


#define DATA_LEN_IN_BYTES 16
#define SRC_BURST 4
#define DST_BURST 4
#define SRC_SIZE_IN_BITS 32
#define DST_SIZE_IN_BITS 32

int fail = 0;

int test_case()
{

	int i,j,cntr;
	long int dma_go_code;
	int inst0,inst1;	
	int iter = DATA_LEN_IN_BYTES / (SRC_BURST * (SRC_SIZE_IN_BITS/8));
	unsigned long int *loc;
	unsigned long int src_addr[8] = {SRC_ADDR_CH0,SRC_ADDR_CH1,SRC_ADDR_CH2,SRC_ADDR_CH3,SRC_ADDR_CH4,SRC_ADDR_CH5,SRC_ADDR_CH6,SRC_ADDR_CH7};
	unsigned long int dst_addr[8] = {DST_ADDR_CH0,DST_ADDR_CH1,DST_ADDR_CH2,DST_ADDR_CH3,DST_ADDR_CH4,DST_ADDR_CH5,DST_ADDR_CH6,DST_ADDR_CH7};
	unsigned long int desc_addr[8] = {DESC_ADDR_CH0,DESC_ADDR_CH1,DESC_ADDR_CH2,DESC_ADDR_CH3,DESC_ADDR_CH4,DESC_ADDR_CH5,DESC_ADDR_CH6,DESC_ADDR_CH7};
	unsigned long int desc_addr_actual[8] = {DESC_ADDR_CH0,DESC_ADDR_CH1,DESC_ADDR_CH2,DESC_ADDR_CH3,DESC_ADDR_CH4,DESC_ADDR_CH5,DESC_ADDR_CH6,DESC_ADDR_CH7};

	#ifdef MPS_DMA
    	GIC_EnableIRQ(0x7);
    	write_reg(MIZAR_MPS_SYSREG_INTR_EN,0x80);

	#elif APS_DMA
    	GIC_EnableIRQ(0x17);
    	write_reg(MIZAR_APS_SYSREG_INTR_EN,0x80);	
	#endif

	for(j = 0; j < 8; j++)
	{
		initialize_memory(desc_addr[i]);
	}

	printf("Inside Testcase\n");
	for(i = 0;i < 8; i++)
	{
		data_preloading(src_addr[i],DATA_LEN_IN_BYTES);
		loc = & desc_addr[i];
		DMA_MOV(SAR,src_addr[i],loc);
		DMA_MOV(DAR,dst_addr[i],loc);
		DMA_MOV_CCR(SRC_BURST,DST_BURST,SRC_SIZE_IN_BITS,DST_SIZE_IN_BITS,0,loc);
		DMA_LP(iter,0,loc);
		DMA_LD(loc);
		DMA_ST(loc);
		DMA_LP_END(0,loc);
		DMA_SEV(loc,i);			
		DMA_END(loc);
		write_reg(MIZAR_DMA_INTEN,(read_reg(MIZAR_DMA_INTEN) | (0x00000001 << i)));		
	}
//	DMA_SEV(loc,8);
	wait_on(10);
	for(i = 0;i < 8; i++)
	{
		dma_go_code = DMA_GO(i,0,desc_addr_actual[i]);
		inst0 = dbg_inst0(0,i,dma_go_code);
		printf("inst0 = 0x%x\n",inst0);
		inst1 = dbg_inst1(dma_go_code);
		printf("inst1 = 0x%x\n",inst1);
	int_pend = 1;
	
		write_reg(MIZAR_DMA_DBGINST0,inst0); //dbginst0
		write_reg(MIZAR_DMA_DBGINST1,inst1); //dbginst1
		write_reg(MIZAR_DMA_DBGCMD,0x0) ;//dbgcmd
		wait_for_int();			
		
	}
	/*while((read_reg(MIZAR_DMA_INTMIS) & 0x100) != 0x100)
	{
		printf("Waiting for Interrupt\n");
		wait_on(5);
	}*/
	
	printf("Got the interrupt\n");
	for(i = 0; i < 8; i++)
	{
		fail = data_integrity(src_addr[i],dst_addr[i],DATA_LEN_IN_BYTES);	
	}
	finish(fail);
}
	
	
	
	

	



