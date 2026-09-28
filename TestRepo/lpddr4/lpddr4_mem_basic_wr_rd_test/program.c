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
    unsigned long int exp_data_array[50];
    unsigned long int read_data;
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

    offset = 0x1000;
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))
    for(index = 0; index < 10; index++)
    {
        exp_data_array[index] = rand();
        exp_data_array[index] = exp_data_array[index]|((unsigned long)rand()<<32);
        write_reg64(port0_addr + ((unsigned long)index*offset),exp_data_array[index]);
    }

    for(index = 0; index < 10; index++)
    {
        read_reg64(port0_addr + ((unsigned long)index*offset),&read_data);

        if(read_data != exp_data_array[index])
        {
            printf("ERROR_0: port0 index = %d, exp_data = %lx, actual_data = %lx\n",index,exp_data_array[index],read_data);
            err0++;
        }
    }

    for(index = 0; index < 10; index++)
    {
        read_reg64(port1_addr + ((unsigned long)index*offset),&read_data);
        if(read_data != exp_data_array[index])
        {
            printf("ERROR_0: port1 index = %d, exp_data = %lx, actual_data = %lx\n",index,exp_data_array[index],read_data);
            err0++;
        }
    }
#endif

    offset = 0x20000000;
    for(index = 0; index < 32; index++)
    {
        exp_data_array[index] = rand();
        exp_data_array[index] = exp_data_array[index]|((unsigned long)rand()<<32);
        write_reg64(port1_addr + ((unsigned long)index*offset),exp_data_array[index]);
    }

    for(index = 0; index < 32; index++)
    {
        read_reg64(port1_addr + ((unsigned long)index*offset),&read_data);
        if(read_data != exp_data_array[index])
        {
            printf("ERROR_1: port1 index = %d, exp_data = %lx, actual_data = %lx\n",index,exp_data_array[index],read_data);
            err0++;
        }
    }

    
    finish(err0);
}



