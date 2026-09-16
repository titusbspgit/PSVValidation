#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"

#include "mipi_csi2.h"

#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000

#ifdef DPHY_LANES_TEST
  #ifndef LS_LE_EN
     #define LS_LE_EN			1
  #endif
  #ifndef TOTAL_FRAME
    #define TOTAL_FRAME 		1
  #endif
  #ifndef VC_ID
    #define VC_ID 			3
  #endif
  #ifndef VRES
    #ifdef GDMA0_FULL_MEM
      #define VRES 			1080  
    #else
      #define VRES 			3
    #endif
  #endif
  #ifndef HRES
    #ifdef GDMA0_FULL_MEM
      #define HRES 			1920
    #else
      #define HRES 			64
    #endif
  #endif
  #ifndef DATA_TYPE
    #define DATA_TYPE 		CSI2_RGB888
  #endif 
#endif //DPHY_LANES_TEST



int data_rd,data_wr;
int def_fail_cnt = 0,wr_fail_cnt = 0;
int vcid_csi2_wrap_reg;
int gdma_path;

unsigned int ch0_pc, ch0_preload_loc;
unsigned int ch1_pc, ch1_preload_loc;

unsigned int gdma_reg_base;
unsigned int rd_data;
long int csi_ctrl_data;
int gdma_int_rsts;

//unsigned int csi_ctrl_data;
long int word_count;
int gdma_int_rsts;
int csi_data_size;

int test_case()
{
	long long int rx_desc, tx_desc;
	long long int gdma_tx_trnsfr_size, gdma_trnsfr_size;
	int cntrl_pkt_cnt;
	printf("start line\n");
        csi2_enable_interrupt(); //Enable CSI-2 interrupt
//	wait_on(100);
    #ifdef GDMA3_PATH
       vcid_csi2_wrap_reg =  VC_ID;
       gdma_path = 3;
    #elif GDMA2_PATH
       vcid_csi2_wrap_reg =  (VC_ID << 4);
       gdma_path = 2;
    #elif GDMA1_PATH
       vcid_csi2_wrap_reg = (VC_ID << 8);
       gdma_path = 1;
    #elif GDMA0_PATH //GDMA0_PATH
       vcid_csi2_wrap_reg = (VC_ID << 12);
       printf("VC_ID=%d\n",VC_ID);
       gdma_path = 0;
    #endif   
    gdma_reg_base = 0xE6A00000;
	write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg); //Write csi-2 virtual channel register
	write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1); //Enable control data transfer
    printf("vcid_csi2_wrap_reg=%0x\n",vcid_csi2_wrap_reg);
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg); //Write csi-2 virtual channel register
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1); //Enable control data transfer
    //Enable csi-2 & dma interrupts
//nmitta temp    csi2_subsys_enable_interrupt();
    //Step-2 : D-Phy initialization sequence
    snps_phy_init();
	//Wait for phy to enter stopstate
	 rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
	while(!(rd_data == 0x1000f))
	{
	 rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
	}


	ch0_pc = 0xE6000000;
	ch1_pc = 0xE6000500;


	for(int lane_num=3;lane_num>=0; lane_num--)
	{
	  write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES,lane_num);//Configure number of lanes in csi2_controller
	  cntrl_pkt_cnt = ((VRES*3) + 2);
	  printf("DEBUG: cntrl_pkt_cnt=%d\n",cntrl_pkt_cnt);
	  write_reg(0xa0243ffc, (lane_num+1));//After this write occur, start csi2_sequence
		

	  for(int i=0;i< cntrl_pkt_cnt/*((VRES*3) + 2)*/;i++) //Repeat the loop for each packet
	  {
	   ch0_preload_loc = ch0_pc;
	   ch1_preload_loc = ch1_pc;
           
	   write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET,0x3); //enable dma_irq[1] & dma_irq[0] by writing dma_irq register

	   //Control data transfer programming
	   dma_trnsfr_instn_preload(ch0_preload_loc /*dma_pc*/, gdma_reg_base, 0x8000 /*src_addr*/, 0xE6001000/*dest_addr*/, 8 /*trnsfr_size*/, 0/*irq_num*/); 

	   DMAGO_CSI(gdma_reg_base, ch0_pc/*pc_addr*/, 0x0/*ch_num*/);

	   rd_data = 0;
	   while((rd_data & 0x1) == 0x0)
	   {
	      rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);//Read dma int_st register to check completion of control_packet transfer
	   	printf("polling irq; irq_status_reg rd_data =%0x\n",rd_data);
	   }  
	   write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET,0x1); //clear dma_irq register
	      rd_data = read_reg(gdma_reg_base + 0x28);//Read dma int_st register to check completion of control_packet transfer
	   printf("DEBUG : irq polling completed ch0; rd_data = %0x \n",rd_data); 
	 
	   csi_ctrl_data = read_reg(0xE6001000);
	   printf("DEBUG: csi_ctrl_data=%0x\n",csi_ctrl_data);
	   //csi data transfer programming
	   if((csi_ctrl_data & 0x3f) > 0xf)
	   {
    	     word_count = ((csi_ctrl_data >> 6)&0xffff);
       	     csi_data_size = (word_count%8) ? (word_count/8 + 1 )* 8 : word_count;    
	     dma_trnsfr_instn_preload(ch1_preload_loc /*dma_pc*/, gdma_reg_base, 0x0000 /*src_addr*/, 0xE6002000/*dest_addr*/, csi_data_size /*trnsfr_size*/, 1/*irq_num*/); 

	     DMAGO_CSI(gdma_reg_base, ch1_pc/*pc_addr*/, 0x1/*ch_num*/);

	     rd_data = 0;
	     while((rd_data & 0x2) == 0x0)
	     {
	        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);//Read dma int_st register to check completion of control_packet transfer
	     	printf("polling irq; irq_status_reg rd_data =%0x\n",rd_data);
	     }  
	      rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);//Read dma int_st register to check completion of control_packet transfer
	     write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET,0x2); //clear dma_irq register
     	     printf("DEBUG : irq polling completed ch1; rd_data=%0x\n",rd_data); 
	   }
	  }
	}
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



