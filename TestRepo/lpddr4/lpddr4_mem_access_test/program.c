#include <stdio.h>
#include <stdlib.h>

#include "test_common.h"
#include "lpddr4.h"

unsigned long int port0_addr;
unsigned long int port1_addr;
unsigned int err0;



int test_case()
{
    unsigned long int index;
    unsigned long int offset;
    unsigned long int exp_data_array[50];
    unsigned long int read_data;

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

#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))
    check_mem_access(port0_addr);
#endif


    check_mem_access(port1_addr + 0x100000);

    
    finish(err0);
}




void check_mem_access(unsigned long int addr)
{
    unsigned int i;
    char *ptr8,data8;
    unsigned short int *ptr16,data16;
    unsigned int *ptr32,data32;
    unsigned long int *ptr64,data64,data; 
    data = rand();
    data = data | ((unsigned long)rand()<<32);

    ptr8  = &data;
    ptr16 = &data;
    ptr32 = &data;
    ptr64 = &data;

    printf("addr: 0x%lx, data: 0x%08x\n",addr,data);
    write_reg64(addr,data);

    for(i = 0; i < 8; i++)
    {
        data8 = read_reg8(addr + i);
        if(data8 != *(ptr8+i))
        {
            printf("ERROR: 8bit access Addr: 0x%lx expected data: 0x%x, Actual data: 0x%x\n",addr+i,data8,*(ptr8+i));
            err0++;
        }
    }

    for(i = 0; i < 4; i++)
    {
        data16 = read_reg16(addr + (i*2));
        if(data16 != *(ptr16+i))
        {
            printf("ERROR: 16bit access Addr: 0x%lx expected data: 0x%x, Actual data: 0x%x\n",addr+(i*2),data16,*(ptr16+i));
            err0++;
        }
    }

    for(i = 0; i < 2; i++)
    {
        data32 = read_reg(addr + (i*4));
        if(data32 != *(ptr32+i))
        {
            printf("ERROR: 32bit access Addr: 0x%lx expected data: 0x%x, Actual data: 0x%x\n",addr+(i*4),data32,*(ptr32+i));
            err0++;
        }
    }

    read_reg64(addr,&data64);
    if(data64 != *ptr64)
    {
        printf("ERROR: 64bit access Addr: 0x%lx expected data: 0x%lx, Actual data: 0x%lx\n",addr,data64,*ptr64);
        err0++;
    }


    addr = addr + 0x1000;
    data = rand();
    data = data | ((unsigned long)rand()<<32);

    printf("addr: 0x%lx, data: 0x%016lx\n",addr,data);
    write_reg8 (addr + 0,*(ptr8  + 0));
    write_reg8 (addr + 1,*(ptr8  + 1));
    write_reg16(addr + 2,*(ptr16 + 1));
    write_reg(addr + 4,*(ptr32 + 1));
    read_reg64(addr, &data64);
    if(data64 != data)
    {
        printf("ERROR: write access, Addr: 0x%lx, expected data: 0x%lx, Actual data: 0x%lx\n",addr,data,data64);
    }
}

