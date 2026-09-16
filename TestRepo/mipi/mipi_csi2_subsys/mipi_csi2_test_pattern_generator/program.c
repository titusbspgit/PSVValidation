#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"

#include "mipi_csi2.h"

#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000


int data_rd,data_wr;
int def_fail_cnt = 0,wr_fail_cnt = 0;
int vcid_csi2_wrap_reg;
int gdma_path;

long int csi_ctrl_data;
int gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;
extern int_pend;
unsigned int tx_trnsfr_size; //temp

void csi2_ctrlr_pg_enable()
{
  write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES,0x10); //PG_VRES
  write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140 /*{16'd7,16'd320}*/); //PG_HRES
  write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401/*{14'h0,2'h0,2'h3,6'h24,7'h0,1'h1}*/); //PG_CONFIG
  							//16'h0,8'hE4,8'h01
  write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1); //PG_ENABLE
}

int test_case()
{
    long long int rx_desc, tx_desc;
    long long int csi2_data_trnsfr_size, single_axi_trnsfr_size;
    int vcid_unselected_path, vcid;
    int num_beats;     
    unsigned long int dma_ch0_pc, dma_ch0_instn_preload_addr;
    //write dma registers with dmago instruction
    unsigned int dmago_instn_0, dmago_instn_1, instn;
    int total_loop_cnt, lc1_iter, lc0_iter;
    int dma_dest_addr_incr_flag;
    int valid_bits_per_pixel;
    unsigned int rd_data;
    int vres, hres;
     
//    GIC_EnableIRQ(CSI2_INTR_NO);
    int_pend = 1;
    printf("start line\n");
    vcid = 3;
    vcid_unselected_path = ((vcid + 1)&0xf);
    
    //Step-1 : Subsystem level registers update
   #ifdef GDMA3_PATH
       vcid_csi2_wrap_reg =  ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+vcid);
       gdma_path = 3;
    #elif GDMA2_PATH
       vcid_csi2_wrap_reg =  ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid << 4)+(vcid_unselected_path));
       gdma_path = 2;
    #elif GDMA1_PATH
       vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid << 8)+(vcid_unselected_path<<4)+(vcid_unselected_path));
       gdma_path = 1;
    #else  //GDMA0_PATH
       vcid_csi2_wrap_reg = ((vcid << 12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path));
       gdma_path = 0;
    #endif   
    gdma_reg_base = 0xE6A00000 + ((gdma_path)*0x1000);
    printf("vcid_csi2_wrap_reg=%0x\n",vcid_csi2_wrap_reg);

    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg); //Write csi-2 virtual channel register
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0); //Disable control data transfer

    //Enable csi-2 & dma interrupts
    csi2_subsys_enable_interrupt();
    //Step-2 : D-Phy initialization sequence
    snps_phy_init();
    //Wait for phy to enter stopstate
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while(!(rd_data == 0x1000f))
    {
     rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }

    hres = 320;
    vres = 16;
    valid_bits_per_pixel = 24;
    //Step-3 : DMA programming sequence 
    csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres)/8 )%8)? (((valid_bits_per_pixel * hres)/8 ) + 8 - (((valid_bits_per_pixel * hres)/8 )%8)) : ((valid_bits_per_pixel * hres)/8))*vres); //number of bytes of transfer
//   num_beats = csi2_data_trnsfr_size / 8;
#ifdef CDEBUG_ON
   printf("DEBUG_1: csi2_data_trnsfr_size=%0d \n",csi2_data_trnsfr_size);
#endif //CDEBUG_ON

	 //Programming higher order arm dma address bits
         write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA,0x100); 
         write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION,0x0); 
         write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA,0x0); 
         write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION,0x0); 

         write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1); //Enable sending fracdiv output to csi2 subsystem

	 dma_ch0_pc = 0xE6000000;
	 dma_ch0_instn_preload_addr = dma_ch0_pc;
    #ifdef FPS60
         dma_dest_addr_incr_flag = 0;
    #else
         dma_dest_addr_incr_flag = 1;
    #endif //FPS60

         dma_trnsfr_instn_preload_incr_addr( dma_ch0_pc, gdma_reg_base, 0x00 /*src_addr*/, 0xE6001000 /*dest_addr*/, csi2_data_trnsfr_size /*trnsfr_size*/, 0/*src_incr_addr_flag*/, dma_dest_addr_incr_flag /*dest_incr_addr_flag*/, 0 /*irq_num*/);


        DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0/*ch_num*/);
         
    

	csi2_ctrlr_pg_enable(); //Enable pattern generator
	wait_on(100);
 	write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0); //PG_ENABLE -- disable pattern_generator
    

        rd_data = read_reg( gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET/*MIZAR_MIPI_CSI2_DMA0_INTMIS*/);


        while((rd_data&0x1)==0)
        {
            rd_data = read_reg( gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET/*MIZAR_MIPI_CSI2_DMA0_INTMIS*/);
    #ifdef CDEBUG_ON
          printf("Polling dma_irq[0]=%0d\n",rd_data);
    #endif //CDEBUG_ON
        }
    
        wait_on(10000);
	finish(0);
}


void csi2_enable_interrupt()
{
    int rd_data;
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);//Read INT_ST_MAIN register to clear interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL,0x0000000f); //Write INT_MSK_PHY_FATAL register to enable phyy_fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL,0x00000003); //Write INT_MSK_PKT_FATAL register to enable pkt_fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY,0x000f000f); //Write INT_MSK_PHY register to enable phy interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE,0x000f000f); //Write INT_MSK_LINE register to enable line interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL,0x0000ffff); //Write INT_MSK_BNDRY_FRAME_FATAL  register to enable boundary frame fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL,0x0000ffff); //Write INT_MSK_SEQ_FRAME_FATAL register to enable INT_MSK_SEQ_FRAME_FATAL interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL,0x0000ffff); //Write INT_MSK_CRC_FRAME_FATAL register to enable INT_MSK_CRC_FRAME_FATAL interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL,0x0000ffff); //Write INT_MSK_PLD_CRC_FATAL register to enable INT_MSK_PLD_CRC_FATAL interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID,0x0000ffff); //Write INT_MSK_DATA_ID register to enable INT_MSK_DATA_ID interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED,0x0000ffff); //Write INT_MSK_ECC_CORRECTED register to enable INT_MSK_ECC_CORRECTED interrupts
//TODO -- Add DATA_ID programming also
}

