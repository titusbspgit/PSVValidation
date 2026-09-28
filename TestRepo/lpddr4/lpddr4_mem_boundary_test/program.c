#include <stdio.h>
#include <stdlib.h>

#include "test_common.h"
#include "lpddr4.h"

unsigned long int port0_addr;
unsigned long int port1_addr;



int test_case()
{
    unsigned long int index1;
    unsigned long int offset;
    unsigned long int exp_data_array[150];
    unsigned long int read_data;
    unsigned int err0;
    unsigned long val_2g;
    unsigned long val_16g;

    val_2g  = 0x0000000080000000;
    val_16g = 0x0000000400000000;

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

#ifdef PORT0_OFF
    offset = PORT0_OFF;
#else
    offset = 0x1000000;
#endif
    printf("port0 offset = 0x%x\n",offset);
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))
    for(index1 = 0; ((unsigned long)(index1*offset)) < val_2g; index1++)
    {
        exp_data_array[index1] = rand();
        exp_data_array[index1] = exp_data_array[index1]|((unsigned long)rand()<<32);
        write_reg64(port0_addr + ((unsigned long)(index1*offset)),exp_data_array[index1]);
        printf("port0_0 index1: %d write addr:0x%016lx\n",index1,port0_addr + ((unsigned long)(index1*offset)));
    }

    for(index1 = 0; ((unsigned long)(index1*offset)) < val_2g; index1++)
    {
        read_reg64(port0_addr + ((unsigned long)(index1*offset)),&read_data);
        printf("port0_0 index1: %d read addr:0x%016lx\n",index1,port0_addr + ((unsigned long)(index1*offset)));

        if(read_data != exp_data_array[index1])
        {
            printf("ERROR_0: port0 index1 = %d, exp_data = %lx, actual_data = %lx\n",index1,exp_data_array[index1],read_data);
            err0++;
        }
    }

    for(index1 = 0; ((unsigned long)(index1*offset)) < val_2g; index1++)
    {
        read_reg64(port1_addr + ((unsigned long)(index1*offset)),&read_data);
        printf("port1_0 index1: %d read addr:0x%016lx\n",index1,port1_addr + ((unsigned long)(index1*offset)));
        if(read_data != exp_data_array[index1])
        {
            printf("ERROR_0: port1 index1 = %d, exp_data = %lx, actual_data = %lx\n",index1,exp_data_array[index1],read_data);
            err0++;
        }
    }
#endif

#ifdef PORT1_OFF
    offset = PORT1_OFF;
#else
    offset = 0x08000000;
#endif
    printf("port1 offset = 0x%x\n",offset);

    for(index1 = 0; ((unsigned long)(index1*offset)) < val_16g; index1++)
    {
        exp_data_array[index1] = rand();
        exp_data_array[index1] = exp_data_array[index1]|((unsigned long)rand()<<32);
        write_reg64(port1_addr + ((unsigned long)(index1*offset)),exp_data_array[index1]);
        printf("port1_1 index1: %d write addr:0x%016lx\n",index1,port1_addr + ((unsigned long)(index1*offset)));
    }

    for(index1 = 0; ((unsigned long)(index1*offset)) < val_16g; index1++)
    {
        read_reg64(port1_addr + ((unsigned long)(index1*offset)),&read_data);
        printf("port1_1 index1: %d read addr:0x%016lx\n",index1,port1_addr + ((unsigned long)(index1*offset)));
        if(read_data != exp_data_array[index1])
        {
            printf("ERROR_1: port1 index1 = %d, exp_data = %lx, actual_data = %lx\n",index1,exp_data_array[index1],read_data);
            err0++;
        }
    }

    
    finish(err0);
}



