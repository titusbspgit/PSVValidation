#include <stdio.h>
#include <stdlib.h>
#include<test_common.h>
#define LOOP_BACK 1
#define MASTER 1
#include "../common/spi_parameter_def.h"
#include<spi.h>



extern int int_pend;
unsigned int count;
int test_case()
{
	int err1;
	
	err1 = 0;
	//int_pend = 1;
	int j;
	count = 1;

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
	//================= SPI INITIALIZATION ===============================//
	
	spi_cntrl_config();
	
	//Enable Vip
	//spi_vip_handshake();
	
	//int_pend = 1;
	
	for(j=0;j<8;j++) {
		int_pend = 1;
		printf("Debug :: IP SPI :: In Loop : %d\n",j);

		//Wait For Rx/Tx Fifo Interrupt
		while(int_pend) {
			wait_on(5);


		}
		
		count++;
	}
	//Test Status
	
	//if(vip_handshake_status() !=)
	//err = 0;
	//
	
	if(read_reg(MIZAR_SPI_RX_FIFO_LEVEL_REG) != 0){
		err1=1;
	}
	
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

	//Rx Fifo Interrupt	
	if((MaskedInterrupt & 0x2) == 0x2) {
		printf("Debug :: IP SPI :: RX Fifo Interrupt Detected\n");
		//Read Data From Rx Fifo. 
		printf("Debug :: IP SPI :: Verifying Rx Data\n");
		for(i=0;i<SPI_RX_FIFO_THLD;i++) {
			rd_data = read_reg(data_addr);
			printf("Data from  RX DATA reg read %0x\n",rd_data);
		}
		
	} 
	else if((MaskedInterrupt & 0x1) == 0x1) {
		printf("Debug :: IP SPI :: TX Fifo Interrupt Detected\n");
		//Write Transmit Data to Tx Fifo. 
		for(j=0;j<SPI_TX_FIFO_THLD;j++) {
			write_reg(data_addr,count);
			printf("Data write to TX DATA reg %0x\n",j);
		}
		
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
