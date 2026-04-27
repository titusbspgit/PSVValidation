#include<stdio.h>
#include<lss_sysreg/lss_sysreg_offset.h>
#include<lss_sysreg/lss_sysreg_def.h>
#include<test_common.h>
#include<dma/dma_def.h>
#include<dma/dma_offset.h>
#include<test_define.c>

#define SRAM_ADDR_1 0xA0243F00
#define SRAM_ADDR_2 0xA0243E00

int i,tx_data[5],rx_data[5];
extern int int_pend;
int test_err=0,int_status,int_status_lss;

void test_case()
{
    unsigned int addr,src_addr,dest_addr;
    int data_sent,data_rcvd,data_rd;
    int_pend = 1;
 
    	/* Making LSS NIC non secure Slave IF*/
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

    GIC_EnableIRQ(80);
    GIC_EnableIRQ(81);

    write_reg(MIZAR_LSS_SYSREG_INTR_EN0,LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT | LSS_SYSREG_INTR_EN0_I2C1_INTERRUPT);
    //write_reg(MIZAR_LSS_SYSREG_INTR_EN0,LSS_SYSREG_INTR_EN0_I2C1_INTERRUPT);
    printf("******** i2c0 master i2c1 slave dma standard mode test *********\n");
        //I2C Configuration
    write_reg(MIZAR_I2C0_FF,0x3);
    write_reg(MIZAR_I2C1_FF,0x3);

    write_reg(MIZAR_I2C0_INTR_CLR,0xFFFFFFFF);
    write_reg(MIZAR_I2C1_INTR_CLR,0xFFFFFFFF);

    write_reg(MIZAR_I2C0_DEV_CTRL,0x38A);
    write_reg(MIZAR_I2C1_DEV_CTRL,0x38A);
    
    write_reg(MIZAR_I2C1_SLV_ADDR,0x37);
    write_reg(MIZAR_I2C0_TGT_SLV_ADDR,0x6E);

    write_reg(MIZAR_I2C0_I2C_BYTE_CNT,0x5);
    write_reg(MIZAR_I2C1_I2C_BYTE_CNT,0x5);
    
    write_reg(MIZAR_I2C0_SF_LCNT,0x2C);
    write_reg(MIZAR_I2C0_SF_HCNT,0x18);

    write_reg(MIZAR_I2C0_I2C_HS_LCNT,0xC);
    write_reg(MIZAR_I2C0_I2C_HS_HCNT,0x8);

    write_reg(MIZAR_I2C0_I2C_MSTR_CODE,0x2);  //0x7     
    write_reg(MIZAR_I2C0_MASK_INTR,0XFFFFFFEF);

    write_reg(MIZAR_I2C0_TX_FIFO_THLD,0x5);
    write_reg(MIZAR_I2C1_TX_FIFO_THLD,0x5);

    write_reg(MIZAR_I2C0_RX_FIFO_THLD,0x5);
    write_reg(MIZAR_I2C1_RX_FIFO_THLD,0x5);

    write_reg(MIZAR_I2C0_DMA_CTRL,0x2);
    
    src_addr = SRAM_ADDR_1;
    dest_addr = MIZAR_I2C0_SMB_HST_BLOCK_DATA;

    //Data preloading
    for(i = 0,addr = src_addr;i < 5; i++,addr = addr+4)
    {
        tx_data[i] = i * 5;
        write_reg(addr,tx_data[i]);
    }

   //DMA Configuration
    write_reg(MIZAR_DMA_CH0_CTRL,0x8028028);
    write_reg(MIZAR_DMA_CH0_SRC_ADDR,src_addr);
    write_reg(MIZAR_DMA_CH0_DEST_ADDR,dest_addr);
    write_reg(MIZAR_DMA_CH0_SRC_XCNT,0x5);
    write_reg(MIZAR_DMA_CH0_SRC_XMDFY,0x4);
    write_reg(MIZAR_DMA_CH0_DEST_XMDFY,0x0);
    write_reg(MIZAR_DMA_CH0_SRC_REQ,0x5);
    write_reg(MIZAR_DMA_DMA_CH_EN,0x1);


    //Waiting for dma transfers to be completed
    dma_disable();

    //Starting i2c0 to i2c1 transfer
    write_reg(MIZAR_I2C0_TSFR_CTRL,0x2);
    
    data_rd = read_reg(MIZAR_I2C0_I2C_CURRENT_BYTECNT);
    while(data_rd != 0)
    {
        data_rd = read_reg(MIZAR_I2C0_I2C_CURRENT_BYTECNT);
        printf("Data bytes left in I2C0 TX FIFO : %x\n",data_rd);
        wait_on(100);
    }

    while(int_pend)
    {
        printf("Waiting for transfer complete interrupt\n ");
        wait_on(10);
    }

    //Initiating DMA transfers
    write_reg(MIZAR_I2C1_DMA_CTRL,0x1);    
    dest_addr = SRAM_ADDR_2;
    src_addr = MIZAR_I2C1_SMB_HST_BLOCK_DATA;

    write_reg(MIZAR_DMA_CH1_CTRL,0x8024028);
    write_reg(MIZAR_DMA_CH1_SRC_ADDR,src_addr);
    write_reg(MIZAR_DMA_CH1_DEST_ADDR,dest_addr);
    write_reg(MIZAR_DMA_CH1_SRC_XCNT,0x5);
    write_reg(MIZAR_DMA_CH1_SRC_XMDFY,0x0);
    write_reg(MIZAR_DMA_CH1_DEST_XMDFY,0x4);
    write_reg(MIZAR_DMA_CH1_SRC_REQ,0x6);
    write_reg(MIZAR_DMA_DMA_CH_EN,0x2);
    
    dma_disable();

    for(i=0;i<5;i++)
    {
        data_sent = read_reg((SRAM_ADDR_1+(i*4)));
        data_rcvd = read_reg((SRAM_ADDR_2+(i*4)));
        if(data_sent == data_rcvd)
        {
            printf("Data Integrity Success : Data sent = %x   Data rcvd = %x\n",data_sent,data_rcvd);
        }
        else
        {
            printf("Data Integrity Failure : Data sent = %x   Data rcvd = %x\n",data_sent,data_rcvd);
            test_err = test_err + 1;
        }
    }
     wait_on(100);
    finish(test_err);
}

void Default_IRQHandler()
{
    int_pend = 0;
    printf("----> Entered default IRQ handler\n");
    int_status = read_reg(MIZAR_I2C0_INTR_STS);
    if(int_status == 0x0010)
    {
        printf("----> Transfer Complete interrupt occured\n---->Clearing the Interrupt\n");
        write_reg(MIZAR_I2C0_INTR_CLR,0x00000010);
        GIC_ClearIRQ(80);
        GIC_ClearIRQ(81);

        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,LSS_SYSREG_RAW_STCR0_I2C0_INTERRUPT);
        //write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,LSS_SYSREG_RAW_STCR0_I2C1_INTERRUPT);

        wait_on(100);
        int_status = read_reg(MIZAR_I2C0_INTR_STS);
        int_status_lss = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0) & LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT;
 //       int_status_lss = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0) & LSS_SYSREG_INTR_EN0_I2C1_INTERRUPT;

        if(int_status == 0x00 && int_status_lss == 0x00)
        {
            printf("Interrupt cleared successfully\n");
        }
        else
        {
            printf("I2C0_MST_I2C1_SLV_DMA_STD_MODE_TEST: ***** Error ***** Interrupt clear failed i2c_intr = %x sys_intr = %x \n",int_status,int_status_lss);
            test_err = test_err + 1;
        }
    }
    else
    {
        test_err = test_err + 1;
        printf("I2C0_MST_I2C1_SLV_DMA_STD_MODE_TEST: ***** Error ***** Transfer Complete interrupt not occured\n");
    }
    
}
