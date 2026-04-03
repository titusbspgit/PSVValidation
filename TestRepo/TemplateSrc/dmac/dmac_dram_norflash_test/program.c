#include<stdio.h>
#include<stdlib.h>
#include<dmac.h>
#include <test_common.h>
#include "lpddr4.h"

#ifdef MPS_DMA
#define FLASH_MEM_BASE 0x110000000
#define FLASH_BASE 0x11D014000 
#define MPS_SYSREG_BASE 0x11D000000

#define SRC_ADDR_PORT0 0x1000
#define SRC_ADDR_PORT1 0x11A0010000
#define DST_ADDR_PORT1 0x14A0001000
#define DST_ADDR_PORT0 0x50001000
#define DST_ADDR_PORT1_1 0x15A0010000

#define DEST_BASE_ADDR_1_CH0 /*0xE6001000 //*/0x10000 
#define DEST_BASE_ADDR_1_CH1 /*0xE6001100 //*/0x20000
#define DEST_BASE_ADDR_1_CH2 /*0xE6001200 //*/0x30000
#define DEST_BASE_ADDR_1_CH3 /*0xE6001300 //*/0x40000
#define DEST_BASE_ADDR_1_CH4 /*0xE6001400 //*/0x50000
#define DEST_BASE_ADDR_1_CH5 /*0xE6001500 //*/0x60000
#define DEST_BASE_ADDR_1_CH6 /*0xE6001600 //*/0x70000
#define DEST_BASE_ADDR_1_CH7 /*0xE6001700 //*/0x80000

#define SRC_BASE_ADDR_1_CH0 0x110000000  
#define SRC_BASE_ADDR_1_CH1 0x111000000 
#define SRC_BASE_ADDR_1_CH2 0x112000000 
#define SRC_BASE_ADDR_1_CH3 0x113000000 
#define SRC_BASE_ADDR_1_CH4 0x114000000 
#define SRC_BASE_ADDR_1_CH5 0x115000000 
#define SRC_BASE_ADDR_1_CH6 0x116000000 
#define SRC_BASE_ADDR_1_CH7 0x117000000 

#define SRC_BASE_ADDR_CH0  /*0xE6001000 //*/0x10000 
#define SRC_BASE_ADDR_CH1  /*0xE6001100 //*/0x20000
#define SRC_BASE_ADDR_CH2  /*0xE6001200 //*/0x30000
#define SRC_BASE_ADDR_CH3  /*0xE6001300 //*/0x40000
#define SRC_BASE_ADDR_CH4  /*0xE6001400 //*/0x50000
#define SRC_BASE_ADDR_CH5  /*0xE6001500 //*/0x60000
#define SRC_BASE_ADDR_CH6  /*0xE6001600 //*/0x70000
#define SRC_BASE_ADDR_CH7  /*0xE6001700 //*/0x80000

#define DEST_BASE_ADDR_CH0  0x110000000 
#define DEST_BASE_ADDR_CH1  0x111000000
#define DEST_BASE_ADDR_CH2  0x112000000
#define DEST_BASE_ADDR_CH3  0x113000000
#define DEST_BASE_ADDR_CH4  0x114000000
#define DEST_BASE_ADDR_CH5  0x115000000
#define DEST_BASE_ADDR_CH6  0x116000000
#define DEST_BASE_ADDR_CH7  0x117000000

#endif

#ifdef APS_DMA
#define DEST_BASE_ADDR_1_CH0 0x10000 
#define DEST_BASE_ADDR_1_CH1 0x20000
#define DEST_BASE_ADDR_1_CH2 0x30000
#define DEST_BASE_ADDR_1_CH3 0x40000
#define DEST_BASE_ADDR_1_CH4 0x50000
#define DEST_BASE_ADDR_1_CH5 0x60000
#define DEST_BASE_ADDR_1_CH6 0x70000
#define DEST_BASE_ADDR_1_CH7 0x80000

#define FLASH_MEM_BASE 0x90000000
#define FLASH_BASE 0x9ee14000
#define APS_SYSREG_BASE 0x9ee00000

#define SRC_BASE_ADDR_1_CH0 0x90000000  
#define SRC_BASE_ADDR_1_CH1 0x91000000 
#define SRC_BASE_ADDR_1_CH2 0x92000000 
#define SRC_BASE_ADDR_1_CH3 0x93000000 
#define SRC_BASE_ADDR_1_CH4 0x94000000 
#define SRC_BASE_ADDR_1_CH5 0x95000000 
#define SRC_BASE_ADDR_1_CH6 0x96000000 
#define SRC_BASE_ADDR_1_CH7 0x97000000 

#define SRC_ADDR_PORT1 0x15A0010000
#define DST_ADDR_PORT1 0x17A0001000
#define SRC_ADDR_PORT0 0x5000
#define DST_ADDR_PORT0 0x60001000
#define DST_ADDR_PORT0_1 0x11A0010000

#define SRC_BASE_ADDR_CH0  /*0xE6001000 //*/0x15A0010000 
#define SRC_BASE_ADDR_CH1  /*0xE6001100 //*/0x15A0020000
#define SRC_BASE_ADDR_CH2  /*0xE6001200 //*/0x15A0030000
#define SRC_BASE_ADDR_CH3  /*0xE6001300 //*/0x15A0040000
#define SRC_BASE_ADDR_CH4  /*0xE6001400 //*/0x15A0050000
#define SRC_BASE_ADDR_CH5  /*0xE6001500 //*/0x15A0060000
#define SRC_BASE_ADDR_CH6  /*0xE6001600 //*/0x15A0070000
#define SRC_BASE_ADDR_CH7  /*0xE6001700 //*/0x15A0080000

#define DEST_BASE_ADDR_CH0 0x90000000   
#define DEST_BASE_ADDR_CH1 0x91000000 
#define DEST_BASE_ADDR_CH2 0x92000000 
#define DEST_BASE_ADDR_CH3 0x93000000 
#define DEST_BASE_ADDR_CH4 0x94000000 
#define DEST_BASE_ADDR_CH5 0x95000000 
#define DEST_BASE_ADDR_CH6 0x96000000 
#define DEST_BASE_ADDR_CH7 0x97000000

#endif


#define BIT_DQ1 0x02
#define BIT_DQ2 0x04
#define BIT_DQ5 0x20
#define BIT_DQ6 0x40
#define BIT_DQ7 0x80

#define DATA_LEN_IN_BYTES 16
#define SRC_BURST 4
#define DST_BURST 4
#define SRC_SIZE_IN_BITS 32
#define DST_SIZE_IN_BITS 32
#define ITER (DATA_LEN_IN_BYTES/(SRC_BURST *(SRC_SIZE_IN_BITS/8)))

#define DESC_ADDR_CH0 0xA0242000
#define DESC_ADDR_CH1 0xA0242100
#define DESC_ADDR_CH2 0xA0242200
#define DESC_ADDR_CH3 0xA0242300
#define DESC_ADDR_CH4 0xA0242400
#define DESC_ADDR_CH5 0xA0242500
#define DESC_ADDR_CH6 0xA0242600
#define DESC_ADDR_CH7 0xA0242700

int fail = 0;
int cs,r1,r2;
unsigned int err0;
unsigned long int port0_addr;
unsigned long int port1_addr;

int test_case()
{
	#ifdef MPS_DMA
    	GIC_EnableIRQ(0x7);
    	write_reg(MIZAR_MPS_SYSREG_INTR_EN,0x80);

	#elif APS_DMA
    	GIC_EnableIRQ(0x17);
    	write_reg(MIZAR_APS_SYSREG_INTR_EN,0x80);	
	#endif
	int i,cntr;
	long int dma_go_code;
	int inst0,inst1;
	
    err0 = 0;
#if defined(APS_DRAM)
    port0_addr      = 0;
    port1_addr      = 0x15A0000000;
    ctl_base        = 0x9EE03000;
    phy_base        = 0x9F000000;
#else
    port0_addr      = 0;
    port1_addr      = 0x11A0000000;
    ctl_base        = 0x11D003000;
    phy_base        = 0x11D500000;
#endif
    
    bus_width       = 0; //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus
#if defined(SG2667)
    SG              = 2667;
#elif defined(SG2133)
    SG              = 2133;
#else
    SG              = 3200;
#endif
    DBI_EN          = 0;
    DM_EN           = 0;
    lpddr4_training();
#ifdef DEBUG_DISPLAY
printf("Training Done\n");
#endif

	printf("DRAM to NORFLASH\n");
	dram_to_norflash();
	finish(fail);
}

void erase_flash(unsigned long int addr)
{
	printf("addr = %x\n",addr);
    if(addr < FLASH_MEM_BASE)
    {
        printf("ERROR: erase address is less than the range of FLASH Memory\n");
        err0++;
        return;
    }
    else if(addr < 0x112000000){
        cs = 0;
	printf("cs = 0\n");
    }
    else if(addr < 0x114000000){
            cs = 1;
	    printf("cs = 1\n");
    }
    else if(addr < 0x116000000) {
        cs = 2;
	printf("cs = 2\n");
    }
    else if(addr < 0x118000000){
        cs = 3;
	printf("cs = 3\n");
    }
    else
    {
        printf("ERROR: erase address is greater than the range of FLASH Memory\n");
        err0++;
        return;
    }

    write_reg(FLASH_MEM_BASE + (cs<<25) + (0x00555<<2),0x00AA00AA);
    write_reg(FLASH_MEM_BASE + (cs<<25) + (0x002AA<<2),0x00550055);
    write_reg(FLASH_MEM_BASE + (cs<<25) + (0x00555<<2),0x00800080);
    write_reg(FLASH_MEM_BASE + (cs<<25) + (0x00555<<2),0x00AA00AA);
    write_reg(FLASH_MEM_BASE + (cs<<25) + (0x002AA<<2),0x00550055);
    write_reg(FLASH_MEM_BASE + (cs<<25) + (0x00555<<2),0x00100010);  //Erasing chip

 
    wait_on(1000);
    write_polling(FLASH_MEM_BASE + (cs<<25));
    wait_on(100);

}
void dram_to_norflash()
{
	int i,j;
	unsigned long int src_addr[8] = {SRC_BASE_ADDR_CH0,SRC_BASE_ADDR_CH1,SRC_BASE_ADDR_CH2,SRC_BASE_ADDR_CH3,SRC_BASE_ADDR_CH4,SRC_BASE_ADDR_CH5,SRC_BASE_ADDR_CH6,SRC_BASE_ADDR_CH7};
	unsigned long int dst_addr[8] = {DEST_BASE_ADDR_CH0,DEST_BASE_ADDR_CH1,DEST_BASE_ADDR_CH2,DEST_BASE_ADDR_CH3,DEST_BASE_ADDR_CH4,DEST_BASE_ADDR_CH5,DEST_BASE_ADDR_CH6,DEST_BASE_ADDR_CH7};
	unsigned long int desc_addr[8] = {DESC_ADDR_CH0,DESC_ADDR_CH1,DESC_ADDR_CH2,DESC_ADDR_CH3,DESC_ADDR_CH4,DESC_ADDR_CH5,DESC_ADDR_CH6,DESC_ADDR_CH7};
	unsigned long int desc_addr_actual[8] = {DESC_ADDR_CH0,DESC_ADDR_CH1,DESC_ADDR_CH2,DESC_ADDR_CH3,DESC_ADDR_CH4,DESC_ADDR_CH5,DESC_ADDR_CH6,DESC_ADDR_CH7};
	unsigned long int saddr,daddr;
	unsigned long int *loc;
	long int dma_go_code,inst0,inst1;
	int sdata;
	
	for(j = 0; j < 8; j++)
	{
		initialize_memory(desc_addr[i]);
	}
	
	for(i = 0;i < 8; i++)
	{
		saddr = src_addr[i];
		data_preloading(saddr,DATA_LEN_IN_BYTES);
	}
	printf("Prloading Data in DRAM Done\n");
	for(i = 0;i < 8; i++)
	{
		loc = & desc_addr[i];
		DMA_MOV(SAR,src_addr[i],loc);
		DMA_MOV(DAR,dst_addr[i],loc);
		DMA_MOV_CCR(SRC_BURST,DST_BURST,SRC_SIZE_IN_BITS,DST_SIZE_IN_BITS,0,loc);
		DMA_LP(ITER,0,loc);
		DMA_LD(loc);
		DMA_ST(loc);
		DMA_LP_END(0,loc);
		DMA_SEV(loc,i);			
		DMA_END(loc);
		write_reg(MIZAR_DMA_INTEN,(read_reg(MIZAR_DMA_INTEN) | (0x00000001 << i)));		
		
	
	wait_on(10);
	#ifdef MPS_DMA
	if(i == 0)
	{
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA0,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA0) | ((dst_addr[i] >> 32) << 8)));
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA0,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA0) | ((src_addr[i] >> 32))));
	}
	if(i == 1)
	{
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA0,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA0) | ((dst_addr[i] >> 32) << 24)));
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA0,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA0) | ((src_addr[i] >> 32) << 16)));
	}
	if(i == 2)
	{
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA1,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA1) | ((dst_addr[i] >> 32) << 8)));
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA1,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA1) | ((src_addr[i] >> 32))));
	}
	if(i == 3)
	{
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA1,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA1) | ((dst_addr[i] >> 32) << 24)));
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA1,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA1) | ((src_addr[i] >> 32) << 16)));
	}
	if(i == 4)
	{
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA2,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA2) | ((dst_addr[i] >> 32) << 8)));
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA2,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA2) | ((src_addr[i] >> 32))));
	}
	if(i == 5)
	{
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA2,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA2) | ((dst_addr[i] >> 32) << 24)));
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA2,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA2) | ((src_addr[i] >> 32) << 16)));
	}
	if(i == 6)
	{
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA3,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA3) | ((dst_addr[i] >> 32) << 8)));
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA3,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA3) | ((src_addr[i] >> 32))));
	}
	if(i == 7)
	{
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA3,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA3) | ((dst_addr[i] >> 32) << 24)));
		write_reg(MIZAR_MPS_SYSREG_NIC_GDMA3,(read_reg(MIZAR_MPS_SYSREG_NIC_GDMA3) | ((src_addr[i] >> 32) << 16)));
	}
	#endif
	#ifdef APS_DMA
	if(i == 0)
	{
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA0,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA0) | ((dst_addr[i] >> 32) << 8)));
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA0,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA0) | ((src_addr[i] >> 32))));
	}
	if(i == 1)
	{
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA0,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA0) | ((dst_addr[i] >> 32) << 24)));
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA0,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA0) | ((src_addr[i] >> 32) << 16)));
	}
	if(i == 2)
	{
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA1,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA1) | ((dst_addr[i] >> 32) << 8)));
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA1,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA1) | ((src_addr[i] >> 32))));
	}
	if(i == 3)
	{
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA1,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA1) | ((dst_addr[i] >> 32) << 24)));
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA1,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA1) | ((src_addr[i] >> 32) << 16)));
	}
	if(i == 4)
	{
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA2,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA2) | ((dst_addr[i] >> 32) << 8)));
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA2,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA2) | ((src_addr[i] >> 32))));
	}
	if(i == 5)
	{
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA2,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA2) | ((dst_addr[i] >> 32) << 24)));
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA2,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA2) | ((src_addr[i] >> 32) << 16)));
	}
	if(i == 6)
	{
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA3,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA3) | ((dst_addr[i] >> 32) << 8)));
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA3,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA3) | ((src_addr[i] >> 32))));
	}
	if(i == 7)
	{
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA3,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA3) | ((dst_addr[i] >> 32) << 24)));
		write_reg(MIZAR_APS_SYSREG_NIC_GDMA3,(read_reg(MIZAR_APS_SYSREG_NIC_GDMA3) | ((src_addr[i] >> 32) << 16)));
	}
	#endif	
	}
	erase_flash(dst_addr[0]);
	for(i = 0;i < 8;i++)
	{

		parallel_flash_incr_write(dst_addr[i]);		
		dma_go_code = DMA_GO(i,0,desc_addr_actual[i]);
		inst0 = dbg_inst0(0,0,dma_go_code);
		printf("inst0 = 0x%x\n",inst0);
		inst1 = dbg_inst1(dma_go_code);
		printf("inst1 = 0x%x\n",inst1);
	int_pend = 1;	
		write_reg(MIZAR_DMA_DBGINST0,inst0); //dbginst0
		write_reg(MIZAR_DMA_DBGINST1,inst1); //dbginst1
		write_reg(MIZAR_DMA_DBGCMD,0x0) ;//dbgcmd
		wait_for_int();
		parallel_flash_incr_write_finish(dst_addr[i]);
	}
	printf("Got the interrupt\n");

	for(i = 0; i < 8; i++)
	{
		fail = fail + data_integrity(src_addr[i],dst_addr[i],DATA_LEN_IN_BYTES);	
	}
	write_reg(MIZAR_DMA_INTEN,0x0);
}


void parallel_flash_incr_write_finish(unsigned long int addr)
{
	write_reg(addr,0x00290029);//end of incr
	wait_on(50);

//	wait_on(200);
	write_polling(FLASH_MEM_BASE + (cs<<25));
	wait_on(200);	
}

void parallel_flash_incr_write(unsigned long int addr)
{
	#ifdef MPS_DMA
    	write_reg(MPS_SYSREG_BASE + 0x0,0x0ff007f4);
    	write_reg(MPS_SYSREG_BASE + 0x4,0xfe04fe02);
    	write_reg(MPS_SYSREG_BASE + 0x8,0x00000206);
	#endif

	#ifdef APS_DMA
   	write_reg(APS_SYSREG_BASE + 0x0,0x0ff007f4);
   	write_reg(APS_SYSREG_BASE + 0x4,0xfe04fe02);
    	write_reg(APS_SYSREG_BASE + 0x8,0x00000206);
	#endif

   	write_reg(FLASH_BASE + 0x14,0x000273cf);
   	write_reg(FLASH_BASE + 0x18,0x04ff0802);
   	
   	write_reg(FLASH_BASE + 0x10,0x00c00000);   //cs1
   	write_reg(FLASH_BASE + 0x10,0x01400000);   //cs2
   	write_reg(FLASH_BASE + 0x10,0x01c00000);   //cs3

	wait_on(50);
	   
    	read_data = read_reg(FLASH_MEM_BASE);
    	wait_on(50);
    	printf("Backdoor write data 0x0: 0x%x\n", read_data);
    	read_data = read_reg(FLASH_MEM_BASE + 0x4);
    	wait_on(50);
    	printf("Backdoor write data 0x4: 0x%x\n", read_data);


	#ifdef MPS_DMA
	if(addr < 0x110000000)
	{
		printf("ERROR: Write address is less than the range of FLASH Memory\n");
		err0++;
		return;
	}
	else if(addr < 0x112000000)
		cs = 0;
	else if(addr < 0x114000000)
		cs = 1;
	else if(addr < 0x116000000)
		cs = 2;
	else if(addr < 0x118000000)
		cs = 3;
	else
	{
		printf("ERROR: Write address is greater than the range of FLASH Memory\n");
		err0++;
		return;
	}
	#endif

	#ifdef APS_DMA
	if(addr < 0x90000000)
	{
		printf("ERROR: Write address is less than the range of FLASH Memory\n");
		err0++;
		return;
	}
	else if(addr < 0x92000000)
		cs = 0;
	else if(addr < 0x94000000)
		cs = 1;
	else if(addr < 0x96000000)
		cs = 2;
	else if(addr < 0x98000000)
		cs = 3;
	else
	{
		printf("ERROR: Write address is greater than the range of FLASH Memory\n");
		err0++;
		return;
	}
	#endif

	//addr = addr - FLASH_MEM_BASE;
	write_reg(FLASH_MEM_BASE + (cs<<25) + 0x0001554,0x00AA00AA);
	wait_on(50);

	write_reg(FLASH_MEM_BASE + (cs<<25) + 0x0000AA8,0x00550055);
	wait_on(50);

	write_reg(addr ,0x00250025); //incr
	wait_on(50);

	write_reg(addr,0x00030003);//len-1
	wait_on(50);
}

void write_polling(unsigned long int addr)
{
	r1 = read_reg(addr);
	while(1)
	{

		wait_on(22);

		r2 = read_reg(addr);

		if ((r1 & BIT_DQ6) != (r2 & BIT_DQ6))   /*is DQ6 toggling?*/
		{
			if ( !(r2 & BIT_DQ5) )
				if ( !(r2 & BIT_DQ1) )
				{
					r1 = r2;

					continue;
				}

			r1 = read_reg(addr);
			r2 = read_reg(addr);

			if ((r1 & BIT_DQ6) != (r2 & BIT_DQ6))   /* is DQ6 toggling? */
			{
#ifdef DEBUG_DISPLAY
				printf("ERROR while writing data\n");
#endif
				err0++;
			}
		}
		else
			/*DQ6 is not toggling, operation is complete.*/
			break;
	}
}

void preloading_flash_mem(unsigned long int addr, int data)
{
	#ifdef MPS_DMA
	write_reg(MPS_SYSREG_BASE + 0x0,0x0ff007f4);
	write_reg(MPS_SYSREG_BASE + 0x4,0xfe04fe02);
        write_reg(MPS_SYSREG_BASE + 0x8,0x00000206);
	#endif

	#ifdef APS_DMA
	write_reg(APS_SYSREG_BASE + 0x0,0x0ff007f4);
	write_reg(APS_SYSREG_BASE + 0x4,0xfe04fe02);
	write_reg(APS_SYSREG_BASE + 0x8,0x00000206);
	#endif	
	
	write_reg(FLASH_BASE + 0x14,0x000273cf);
	write_reg(FLASH_BASE + 0x18,0x04ff0802);

	write_reg(FLASH_BASE + 0x10,0x00c00000);   //cs1
	write_reg(FLASH_BASE + 0x10,0x01400000);   //cs2
	write_reg(FLASH_BASE + 0x10,0x01c00000);   //cs3

	wait_on(50);
	printf("addr = 0x%lx\n",addr);
	write_flash_mem(addr,data);
}

void write_flash_mem (unsigned long int addr,int data)

{
	#ifdef MPS_DMA
	if(addr < 0x110000000)
	{
		printf("addr = 0x%lx\n",addr);		
		printf("ERROR: Write address is less than the range of FLASH Memory\n");
		err0++;
		return;
	}
	else if(addr < 0x112000000)
		cs = 0;
	else if(addr < 0x114000000)
		cs = 1;
	else if(addr < 0x116000000)
		cs = 2;
	else if(addr < 0x118000000)
		cs = 3;
	else
	{
		printf("ERROR: Write address is greater than the range of FLASH Memory\n");
		err0++;
		return;
	}
	#endif

	#ifdef APS_DMA
	if(addr < 0x90000000)
	{
		printf("ERROR: Write address is less than the range of FLASH Memory\n");
		err0++;
		return;
	}
	else if(addr < 0x92000000)
		cs = 0;
	else if(addr < 0x94000000)
		cs = 1;
	else if(addr < 0x96000000)
		cs = 2;
	else if(addr < 0x98000000)
		cs = 3;
	else
	{
		printf("ERROR: Write address is greater than the range of FLASH Memory\n");
		err0++;
		return;
	}
	#endif

	write_reg(FLASH_MEM_BASE + (cs<<25) + 0x0001554,0x00AA00AA);
	wait_on(50);

	write_reg(FLASH_MEM_BASE + (cs<<25) + 0x0000AA8,0x00550055);
	wait_on(50);

	write_reg(FLASH_MEM_BASE + (cs<<25) + 0x0001554,0x00A000A0);
	wait_on(50);

	write_reg(addr,data);
	wait_on(200);
	write_polling(FLASH_MEM_BASE + (cs<<25));
	wait_on(200);
}
