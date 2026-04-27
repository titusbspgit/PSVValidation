#include <stdio.h>
#include <stdlib.h>
#include<test_common.h>
#include "../common/spi_parameter_def.h"
#undef SPI_INTR_MASK
#define SPI_INTR_MASK 0x0
#undef SPI_TX_FIFO_THLD
#define SPI_TX_FIFO_THLD  0x1
#include<spi.h>
//#include<spi_funcs.c>
//


extern int int_pend;
int count,err1;
int test_case()
{
	
	unsigned int ch_num;
	unsigned int src_addr;
	unsigned int dst_addr;
	unsigned int src_xcnt;
	unsigned int tx_rx;
	unsigned int tc_intr_en;
	unsigned int spi_mst,src_req;

	int i,rd_data,j;
	count=1;
	err1 = 0;

	#ifdef SPI0
		write_reg(0xA000001C, 0x01000000); //Sysreg spi0 intr enable
		GIC_EnableIRQ(76);
	#endif
	#ifdef SPI1
		write_reg(0xA000001C, 0x02000000); //Sysreg spi1 intr enable
		GIC_EnableIRQ(77);
	#endif
	#ifdef SPI2
		write_reg(0xA000001C, 0x04000000); //Sysreg spi2 intr enable
		GIC_EnableIRQ(78);
	#endif
	#ifdef SPI3
		write_reg(0xA000001C, 0x08000000); //Sysreg spi3 intr enable
		GIC_EnableIRQ(79);
	#endif
	
	write_reg(0xA1700008, 0x1);
	write_reg(0xA170000C, 0x1);
	write_reg(0xA1700014, 0x1);
	write_reg(0xA1700018, 0x1);
	write_reg(0xA170001C, 0x1);
	write_reg(0xA1700020, 0x1);
	write_reg(0xA1700024, 0x1);
	write_reg(0xA1700028, 0x1);
	write_reg(0xA170002C, 0x1);
	write_reg(0xA1700030, 0x1);
	write_reg(0xA1700034, 0x1);
	write_reg(0xA1700038, 0x1);
	write_reg(0xA170003C, 0x1);
	write_reg(0xA1700044, 0x1);
	write_reg(0xA1700048, 0x1);
	write_reg(0xA1700050, 0x1);
	write_reg(0xA1700054, 0x1);

	ch_num = 0;
	src_addr = 0xA0243E6C;
	src_xcnt = 8;
	tx_rx = 0;
	tc_intr_en = 0;
	spi_mst = 0;
	src_req = SPI_TX_SRC_REQ;

	//=============== Loding data into memory
	
	for(i=0;i<src_xcnt;i++) {	
		write_reg((src_addr + (i * 4)),(0xaaaaaaa1+i));
                //src_addr = src_addr + 4;
		//i++;
	}

	spi_cntrl_config();
	//Enable Vip
	spi_vip_handshake();
	
	dst_addr = MIZAR_SPI_DATA_REG;
	dma_config(ch_num, src_addr, dst_addr, src_xcnt, tx_rx,	tc_intr_en,src_req, spi_mst);
	
	dma_disable();
	//
	//Wait For Rx/Tx Fifo Interrupt
		write_reg(MIZAR_SPI_IMSC,0x1);
		int_pend = 1;
		while(int_pend) {
			wait_on(5); 
			
		}


	err1 = spi_vip_scbd_status();
	//Finish Test
	finish(err1);

}


void Default_IRQHandler()
{
       	
	unsigned int MaskedInterrupt,i,j,rd_data,data_addr,mis_addr;		
	int_pend = 0;
	data_addr = MIZAR_SPI_DATA_REG;
	mis_addr = MIZAR_SPI_MIS;
	//Reading Masked Interrupt Status Register	
	MaskedInterrupt = read_reg(mis_addr);

 if((MaskedInterrupt & 0x1) == 0x1) {
		printf("Debug :: IP SPI :: TX Interrupt Detected\n");
		//Write Transmit Data to Tx Fifo. 
		//for(j=0;j<SPI_TX_FIFO_THLD;j++) {
		//	write_reg(data_addr,count);
		//	printf("Data write to TX DATA reg %0x\n",count);
		//}
		write_reg(MIZAR_SPI_IMSC,0x0);
		

	}
	#ifdef SPI0
	write_reg(0xA0000018, 0x01000000); //Sysreg spi0 intr enable
	GIC_ClearIRQ(76);
	#endif
	#ifdef SPI1
	write_reg(0xA0000018, 0x02000000); //Sysreg spi1 intr enable
	GIC_ClearIRQ(77);
	#endif
	#ifdef SPI2
	write_reg(0xA0000018, 0x04000000); //Sysreg spi2 intr enable
	GIC_ClearIRQ(78);
	#endif
	#ifdef SPI3
	write_reg(0xA0000018, 0x08000000); //Sysreg spi3 intr enable
	GIC_ClearIRQ(79);
	#endif
		
}

