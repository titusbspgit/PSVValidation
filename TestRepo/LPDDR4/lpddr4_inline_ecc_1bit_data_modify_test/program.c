#include <stdio.h>
#include <stdlib.h>

#include "test_common.h"
#include "mps_sysreg.h"
#include "aps_sysreg.h"
#include "lpddr4.h"
#include "lpddr4_aps_sii.h"
#include "lpddr4_sii.h"


unsigned long int port0_addr;
unsigned long int port1_addr;
unsigned int int_expected;
unsigned int eccstat;
unsigned int count;
unsigned int region;
unsigned int ecc_region_map;
unsigned int sbr_init;
unsigned int err0;

extern int int_pend;
unsigned int int_pend1;
#ifdef APS_DRAM
    
    #define CTL_INT_NO                                   APS_LPDDR4_CTL_INT_NO
    #define PHY_INT_NO                                   APS_LPDDR4_PHY_INT_NO

    #define SYSREG_INTR_EN_ADDR                          MIZAR_APS_SYSREG_INTR_EN
    #define SYSREG_CTRL_INTR_EN_MASK                     APS_SYSREG_INTR_EN_DDR_CTRL_INTR
    #define SYSREG_PHY_INTR_EN_MASK                      APS_SYSREG_INTR_EN_DDRPHY_DWC_DDRPHY_INT_N
    
    #define SYSREG_INTR_MSTS_ADDR                        MIZAR_APS_SYSREG_INTR_MSTS
    #define SYSREG_CTRL_INTR_MSTS_MASK                   APS_SYSREG_INTR_MSTS_DDR_CTRL_INTR
    #define SYSREG_PHY_INTR_MSTS_MASK                    APS_SYSREG_INTR_MSTS_DDRPHY_DWC_DDRPHY_INT_N
    
    #define SYSREG_INTR_RSTCR_ADDR                       MIZAR_APS_SYSREG_INTR_RSTCR
    #define SYSREG_CTRL_INTR_RSTCR_MASK                  APS_SYSREG_INTR_RSTCR_DDR_CTRL_INTR
    #define SYSREG_PHY_INTR_RSTCR_MASK                   APS_SYSREG_INTR_RSTCR_DDRPHY_DWC_DDRPHY_INT_N

    #define SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK     LPDDR4_APS_SII_DDR_CNTRL_INTR_EN_DDR_ECC_CORRECTED_ERR_INTR_FAULT
    #define SII_INTR_EN_SBR_DONE_INTR_MASK               LPDDR4_APS_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR

#else
    
    #define CTL_INT_NO                                   MPS_LPDDR4_CTL_INT_NO
    #define PHY_INT_NO                                   MPS_LPDDR4_PHY_INT_NO

    #define SYSREG_INTR_EN_ADDR                          MIZAR_MPS_SYSREG_INTR_EN
    #define SYSREG_CTRL_INTR_EN_MASK                     MPS_SYSREG_INTR_EN_DDR_CTRL_INTR
    #define SYSREG_PHY_INTR_EN_MASK                      MPS_SYSREG_INTR_EN_DDRPHY_DWC_DDRPHY_INT_N
    
    #define SYSREG_INTR_MSTS_ADDR                        MIZAR_MPS_SYSREG_INTR_MSTS
    #define SYSREG_CTRL_INTR_MSTS_MASK                   MPS_SYSREG_INTR_MSTS_DDR_CTRL_INTR
    #define SYSREG_PHY_INTR_MSTS_MASK                    MPS_SYSREG_INTR_MSTS_DDRPHY_DWC_DDRPHY_INT_N
    
    #define SYSREG_INTR_RSTCR_ADDR                       MIZAR_MPS_SYSREG_INTR_RSTCR
    #define SYSREG_CTRL_INTR_RSTCR_MASK                  MPS_SYSREG_INTR_RSTCR_DDR_CTRL_INTR
    #define SYSREG_PHY_INTR_RSTCR_MASK                   MPS_SYSREG_INTR_RSTCR_DDRPHY_DWC_DDRPHY_INT_N

    #define SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK     LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_CORRECTED_ERR_INTR_FAULT
    #define SII_INTR_EN_SBR_DONE_INTR_MASK               LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR
#endif


int test_case()
{
    unsigned long int offset;
    unsigned long int exp_data_array[50];
    unsigned long int read_data;

    int_pend = 1;
    GIC_EnableIRQ(CTL_INT_NO);	
    write_reg(SYSREG_INTR_EN_ADDR,SYSREG_CTRL_INTR_EN_MASK);
    gpv_programming();
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
    DM_EN           = 1;
    ECC_EN          = 1;
    lpddr4_training();

    write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK); //enabling intr

    ecc_region_map = 0x55;
    write_reg(ctl_base + 0x00000320,0x00000000);    //UMCTL2_REGS.SWCTL
    write_reg(ctl_base + 0x00000070,(ecc_region_map<<8)|0xb4);    //UMCTL2_REGS.ECCCFG0
    write_reg(ctl_base + 0x00000074,0x00000330);    //UMCTL2_REGS.ECCCFG1
    //write_reg(ctl_base + 0x00000078,0x00000000);    //UMCTL2_REGS.ECCSTAT
    write_reg(ctl_base + 0x00000320,0x00000001);    //UMCTL2_REGS.SWCTL

    int_expected = 0;

    offset = 0x80000000;
    for(region = 0; region < 7; region++)
    {
        if((ecc_region_map>>region)&0x1)
        {
            memory_init_scrb(offset*region);
        }
    }
    

    for(region = 0; region < 7; region++)
    {
        exp_data_array[region] = rand();
        exp_data_array[region] = exp_data_array[region]|((unsigned long)rand()<<32);
        write_reg64(port1_addr + (region*offset),exp_data_array[region]);
    }

    wait_on(1000);
    training_done();
    wait_on(1000);


    write_reg(ctl_base + 0x0000007c,0x00000000);    //UMCTL2_REGS.ECCCTL

    read_reg64(port1_addr,&read_data);
    eccstat = read_reg(ctl_base + 0x00000078);    //UMCTL2_REGS.ECCSTAT
    while(!(eccstat&(1<<8)))
    {
        eccstat = read_reg(ctl_base + 0x00000078);    //UMCTL2_REGS.ECCSTAT
    }
    write_reg(ctl_base + 0x0000007c,read_reg(ctl_base + 0x0000007c)|0x00000001);    //UMCTL2_REGS.ECCCTL
    write_reg(ctl_base + 0x00008194,SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK);
    wait_on(1000);
    if(exp_data_array[0] != read_data)
    {
        printf("ERROR region 0 no int: exp_data = %lx, actual_data = %lx\n",exp_data_array[0],read_data);
        err0++;
    }


    write_reg(ctl_base + 0x0000819C,SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK); //enabling intr
    write_reg(ctl_base + 0x0000007c,1<<8);    //UMCTL2_REGS.ECCCTL      Interrupt enable
    for(region = 0; region < 7;region++)
    {
        printf("region = %d\n",region);
        int_expected = (ecc_region_map>>region)&0x1;
        int_pend1 = 1;
        read_reg64(port1_addr + (region*offset),&read_data);
        wait_on(1000);
        if(int_expected)
        {
            if(exp_data_array[region] != read_data)
            {
                printf("ERROR Data Mismatch,region: %d Expected data: 0x%016x Actual Data: 0x%016x\n",region,exp_data_array[region],read_data);
                err0++;
            }
            while(int_pend1)
            {
                wait_on(10);
            }
        }
        else
        {
            if(exp_data_array[region] == read_data)
            {
                printf("ERROR Data matched,region: %d Expected data: 0x%016x Actual Data: 0x%016x\n",region,exp_data_array[region],read_data);
                err0++;
            }
        }

    }

    count = read_reg(ctl_base + 0x00000080);    //UMCTL2_REGS.ECCERRCNT
    if(count != 5)
    {
        printf("ERROR: Expected count = 5, Actual count = %d\n",count);
        err0++;
    }

    finish(err0);
}



void Default_IRQHandler()
{
    unsigned int data;
    unsigned int rd_data;
    unsigned int sbr_data;
    int_pend = 0;
    data = 0;
   
    data = read_reg(SYSREG_INTR_MSTS_ADDR);
    if(data == SYSREG_CTRL_INTR_MSTS_MASK)
    {
        if(sbr_init)
        {
            sbr_data = read_reg(ctl_base + 0x00008194);
            if((sbr_data &SII_INTR_EN_SBR_DONE_INTR_MASK) == SII_INTR_EN_SBR_DONE_INTR_MASK)
            {
                    rd_data = read_reg(ctl_base + 0x00000f28);  //UMCTL2_MP.SBRRANGE1
                    while(rd_data & 0x1)
                    {
                        rd_data = read_reg(ctl_base + 0x00000f28);  //UMCTL2_MP.SBRRANGE1
                    }

                    write_reg(ctl_base + 0x00000f24,set_data(read_reg(ctl_base + 0x00000f24),0,0,0));    //UMCTL2_MP.SBRCTL
                    write_reg(ctl_base + 0x00008194,SII_INTR_EN_SBR_DONE_INTR_MASK);
                    printf("SBR Interrupt cleared\n");
                    int_pend1 = 0;
            }
            else
            {
                err0++;
                printf("ERROR2: Unexpected interrupt\n");
            }  

        }
        else
        {
            if(!int_expected)
            {
                printf("ERROR0: Unexpected interrupt\n");
                err0++;
                int_pend = 1;
                return;
            }
            eccstat = read_reg(ctl_base + 0x00000078);    //UMCTL2_REGS.ECCSTAT
            if((eccstat&0x100) == 0x100)
            {
                write_reg(ctl_base + 0x0000007c,read_reg(ctl_base + 0x0000007c)|0x00000001);    //UMCTL2_REGS.ECCCTL
                write_reg(ctl_base + 0x00008194,SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK);
                printf("Interrupt cleared\n");
                int_pend1 = 0;
            }
            else
            {
                printf("ERROR: Unexpected interrupt\n");
                err0++;
            }
        }
        write_reg(SYSREG_INTR_RSTCR_ADDR,SYSREG_CTRL_INTR_RSTCR_MASK);
        GIC_ClearIRQ(CTL_INT_NO);	
    }
    else
    {
        printf("ERROR1: Unexpected interrupt\n");
        err0++;
    }
    int_pend = 1;
}


void memory_init_scrb(unsigned long int address)
{
    unsigned long int i;
    unsigned int rd_data;

    sbr_init = 1;
    //step 1: ECCCFG1.ecc_region_parity_lock to 1 
    //Programmed before task call

    //step 2: PCTRL_n.port_en
    write_reg(ctl_base + 0x00000490,0x00000000);    //UMCTL2_MP.PCTRL_0
    write_reg(ctl_base + 0x00000540,0x00000000);    //UMCTL2_MP.PCTRL_1

    //step 3: skip 

    //step 4: SBRCTL.scrub_mode to 1
    write_reg(ctl_base + 0x00000f24,set_data(read_reg(ctl_base + 0x00000f24),2,2,1));    //UMCTL2_MP.SBRCTL

    //step 5: SBRCTL.scrub_interval = 0
    write_reg(ctl_base + 0x00000f24,set_data(read_reg(ctl_base + 0x00000f24),8,31,0));    //UMCTL2_MP.SBRCTL

    //step 6: pattern through SBRWDATA0 and SBRWDATA1 registers
    write_reg(ctl_base + 0x00000f2c,0x0);    //UMCTL2_MP.SBRWDATA0
    write_reg(ctl_base + 0x00000f30,0x0);    //UMCTL2_MP.SBRWDATA1


    int_pend1 = 1;
    write_reg(ctl_base + 0x00000f38,(address>>3)&0xFFFFFFFF);    //UMCTL2_MP.SBRSTART0
    write_reg(ctl_base + 0x00000f3c,((address>>3)>>32));    //UMCTL2_MP.SBRSTART1
    write_reg(ctl_base + 0x00000f40,((address+0x1000)>>3)&0xFFFFFFFF);    //UMCTL2_MP.SBRRANGE0
    write_reg(ctl_base + 0x00000f44,(((address+0x1000)>>3)>>32));    //UMCTL2_MP.SBRRANGE1

    //step 7: SBRCTL.scrub_en = 1.
    write_reg(ctl_base + 0x00000f24,set_data(read_reg(ctl_base + 0x00000f24),0,0,1));    //UMCTL2_MP.SBRCTL
    //step 8: Poll SBRSTAT.scrub_done = 1 
    while(int_pend1)
    {
        wait_on(10);
    }

    //step 9: Poll SBRSTAT.scrub_busy = 0  included in IRQ handler

    //rd_data = read_reg(ctl_base + 0x00000f28);  //UMCTL2_MP.SBRRANGE1

    //step 10: Disable SBR by programming SBRCTL.scrub_en = 0.

    //step 11: skip

    //step 12: SBRCTL.scrub_mode = 0 and SBRCTL.scrub_interval to tSCRUBI.
    rd_data = read_reg(ctl_base + 0x00000f24);    //UMCTL2_MP.SBRCTL
    rd_data = set_data(rd_data,2,2,0);
    rd_data = set_data(rd_data,8,31,1);
    write_reg(ctl_base + 0x00000f24,rd_data);    //UMCTL2_MP.SBRCTL

    //step 13: SBRCTL.scrub_en = 1.
    write_reg(ctl_base + 0x00000f24,set_data(read_reg(ctl_base + 0x00000f24),0,0,0));    //UMCTL2_MP.SBRCTL

    //step 14:
    write_reg(ctl_base + 0x00000490,0x00000001);    //UMCTL2_MP.PCTRL_0
    write_reg(ctl_base + 0x00000540,0x00000001);    //UMCTL2_MP.PCTRL_1

    sbr_init = 0;

}
