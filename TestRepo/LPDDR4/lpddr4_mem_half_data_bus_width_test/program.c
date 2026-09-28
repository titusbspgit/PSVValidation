#include <stdio.h>
#include <stdlib.h>

#include "test_common.h"
#include "lpddr4.h"

unsigned long int port0_addr;
unsigned long int port1_addr;



int test_case()
{
    unsigned long int index;
    unsigned long int offset;
    unsigned long int read_data0;
    unsigned long int read_data1;
    unsigned long int read_data2;
    unsigned long int read_data3;
    unsigned int err0;

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

    bus_width       = 1; //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus
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
    training_done();

    wait_on(1000);

#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))
        read_reg64(port0_addr + 0x100,&read_data0);
        read_reg64(port0_addr + 0x108,&read_data1);

        if((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222))
        {
            printf("ERROR: port0 read_data0 = %lx, read_data1 = %lx\n",read_data0,read_data1);
            err0++;
        }
#endif

        read_reg64(port1_addr + 0x100,&read_data2);
        read_reg64(port1_addr + 0x108,&read_data3);

        if((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222))
        {
            printf("ERROR: port1 read_data2 = %lx, read_data3 = %lx\n",read_data2,read_data3);
            err0++;
        }   

    finish(err0);
}



