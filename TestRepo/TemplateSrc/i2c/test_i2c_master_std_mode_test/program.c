#include<stdio.h>
#include<test_define.c>
#include<lss_sysreg/lss_sysreg_offset.h>
#include<lss_sysreg/lss_sysreg_def.h>
#include<test_common.h>

int i,tx_data[10],rx_data[10];
extern int int_pend;
int test_err=0,int_status,int_status_lss;

void test_case()
{
    int_pend = 1;
    
#ifdef I2C0
    GIC_EnableIRQ(80);
#endif

#ifdef I2C1
    GIC_EnableIRQ(81);
#endif

#ifdef I2C0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0,LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT);
#endif

#ifdef I2C1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0,LSS_SYSREG_INTR_EN0_I2C1_INTERRUPT);
#endif

    printf("******** i2c standard mode test *********\n----> Programming the I2C registers\n");

    //To Transmit Data
    write_reg(MIZAR_I2C_DEV_CTRL,0x382);
    write_reg(MIZAR_I2C_TGT_SLV_ADDR,0x6E);
    write_reg(MIZAR_I2C_I2C_BYTE_CNT,0xA);
    write_reg(MIZAR_I2C_FF,0x3);
    write_reg(MIZAR_I2C_INTR_CLR,0xFFFFFFFF);
    write_reg(MIZAR_I2C_SF_LCNT,0x22C);
    write_reg(MIZAR_I2C_SF_HCNT,0x1C8);
    write_reg(MIZAR_I2C_MASK_INTR,0XFFFFFFEF);
    write_reg(MIZAR_I2C_TX_FIFO_THLD,0xA);
    printf("----> I2C Configuration done\n");
    for(i = 0;i < 10;i++)
    {
        printf("----> Writing to transmit FIFO\n");
        write_reg(MIZAR_I2C_SMB_HST_BLOCK_DATA,i);
        tx_data[i] = i;
    }
    printf("----> Writing to transmit FIFO Done\n");
    printf("----> Initiating Transactions\n");


    write_reg(MIZAR_I2C_TSFR_CTRL,0x2); 
    wait_on(100000);
    while(int_pend)
    {
        printf("----> Waiting for transfer complete interrupt\n");
        wait_on(10);
    } 
    printf("----> Transfer Done\n");

    wait_on(1000);
    int_pend = 1;
#ifdef I2C0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0,LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT);
#endif

#ifdef I2C1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN0,LSS_SYSREG_INTR_EN0_I2C1_INTERRUPT);
#endif
    
    //To recieve Data
    write_reg(MIZAR_I2C_FF,0x3);
    write_reg(MIZAR_I2C_INTR_CLR,0x1FF);
    write_reg(MIZAR_I2C_TGT_SLV_ADDR,0x6F);
    write_reg(MIZAR_I2C_MASK_INTR,0xFFFFFFEF);
    write_reg(MIZAR_I2C_I2C_BYTE_CNT,0xA);
    write_reg(MIZAR_I2C_RX_FIFO_THLD,0xA);
    for(i = 0;i < 10; i++)
    {
        write_reg(MIZAR_I2C_SMB_HST_BLOCK_DATA,0);
    }
    printf("----> Started Recieving Data\n");
    write_reg(MIZAR_I2C_TSFR_CTRL,0x2);
  
    wait_on(1000);
    while(int_pend)
    {
        printf("----> Waiting for transmit complete Interrupt\n");
        wait_on(10);
    }    
    
  
    for(i=0;i<10;i++)
    {
        printf("----> Reading the recieved data : byte no-%d\n",i);
        rx_data[i] = read_reg(MIZAR_I2C_SMB_HST_BLOCK_DATA);
    }
    printf("----> Data Integrity check\n");
    for(i=0;i<10;i++)
    {
        if(tx_data[i] == rx_data[i])
        {
            printf("Byte %d: Data Integrity Success tx_data = %xrx_data = %x\n",i,tx_data[i],rx_data[i]);
        }
        else
        {
            printf("I2C_MASTER_STD_MODE_TEST: ***** Error ***** \n Byte %d: Data Integrity Failure tx_data = %xrx_data = %x\n",i,tx_data[i],rx_data[i]);
            test_err = test_err + 1;
        }
    }
    printf("----> Data Integrity check done\n");
    wait_on(100);
    finish(test_err);
} 

void Default_IRQHandler()
{
    int_pend = 0;
    printf("----> Entered default IRQ handler\n");
    int_status = read_reg(MIZAR_I2C_INTR_STS);
    if(int_status == 0x0010)
    {
        printf("----> Transfer Complete interrupt occured\n---->Clearing the Interrupt\n");
        write_reg(MIZAR_I2C_INTR_CLR,0x00000010);
    #ifdef I2C0
        GIC_ClearIRQ(80);
    #endif
    
    #ifdef I2C1
        GIC_ClearIRQ(81);
    #endif

    #ifdef I2C0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,LSS_SYSREG_RAW_STCR0_I2C0_INTERRUPT);
    #endif

    #ifdef I2C1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR0,LSS_SYSREG_RAW_STCR0_I2C1_INTERRUPT);
    #endif

        wait_on(100);
        int_status = read_reg(MIZAR_I2C_INTR_STS);
    #ifdef I2C0
        int_status_lss = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0) & LSS_SYSREG_INTR_EN0_I2C0_INTERRUPT;
    #endif

    #ifdef I2C1
        int_status_lss = read_reg(MIZAR_LSS_SYSREG_RAW_STCR0) & LSS_SYSREG_INTR_EN0_I2C1_INTERRUPT;
    #endif

        if(int_status == 0x00 && int_status_lss == 0x00)
        {
            printf("Interrupt cleared successfully\n");
        }
        else
        {
            printf("I2C_MASTER_STD_MODE_TEST: ***** Error ***** Interrupt clear failed i2c_intr = %x sys_intr = %x \n",int_status,int_status_lss);
            test_err = test_err + 1;
        }
    }
    else
    {
        test_err = test_err + 1;
        printf("I2C_MASTER_STD_MODE_TEST: ***** Error ***** Transfer Complete interrupt not occured\n");
    }
    
}
