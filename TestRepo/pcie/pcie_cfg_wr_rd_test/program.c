#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>


#include "pcie.h"
unsigned int data_rd,data_wr,rd_wr_data1;
int err2 = 0;
int err1 = 0;

int test_case()
{
	int i;
	write_reg(0xE6004100,0x0);
	
//link_training(2);   
#ifdef DM0_RC
link_training_dm0_x4(4);  
#endif
#ifdef DM1_RC
link_training_dm1_x4(4);  
#endif
#ifdef DM0_EP
link_training_dm0_x4(4);  
#endif
#ifdef DM1_EP
link_training_dm1_x4(4);  
#endif
//CACHE PROGRAMMING
rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),11,14,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,3,6,0xf); 
write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1);

rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),27,30,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,19,22,0xf); 
write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1); 

rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),11,14,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,3,6,0xf); 
write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1);

rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),27,30,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,19,22,0xf); 
write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1); 

wait_on(20);
rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),11,14,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,3,6,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,27,30,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,19,22,0xf); 
write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1); 

rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),11,14,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,3,6,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,27,30,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,19,22,0xf); 
write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1);
data_rd = read_sii0_reg(0xC0);
#ifdef DEBUG_DISPLAY
printf("TEST SII0 READ DATA data = %x\n",data_rd);
#endif

non_secure_prot_nic();

while(((data_rd)&(0xD1))!=0xD1)
{
data_rd = read_sii0_reg(0xC0);
#ifdef DEBUG_DISPLAY
printf("TEST SII0 READ DATA data = %x\n",data_rd);
#endif
}
#ifdef DM1_RC
data_rd = read_sii1_reg(0xC0);
while(((data_rd)&(0xD1))!=0xD1)
{
#ifdef DEBUG_DISPLAY
printf("TEST SII1 READ DATA data = %x\n",data_rd);
#endif
data_rd = read_sii1_reg(0xC0);
}

#endif

write_reg(0xE6004100,0x11111111);


wait_on(15000);

#ifdef DM0_RC
mem_base_program_dm0_x4();
wait_on(10);
#ifdef DEBUG_DISPLAY
printf("Reading Vendor ID,Device ID and Bars from PCIe Instance0\n");
#endif
for(i =0;i<10;i = i+1)
{
rd_wr_data1 = read_pcie_slv0_reg(i*0x4);
}



printf("Entered 1st set of writes \n");
write_pcie_slv0_reg(0x10,0xFFFFFFFF);
write_pcie_slv0_reg(0x14,0xFFFFFFFF);
write_pcie_slv0_reg(0x18,0xFFFFFFFF);
write_pcie_slv0_reg(0x1c,0xFFFFFFFF);
write_pcie_slv0_reg(0x20,0xFFFFFFFF);
write_pcie_slv0_reg(0x24,0xFFFFFFFF);
printf("1st set of writes completed\n");

printf("Entered 1st set of reads \n");
rd_wr_data = read_pcie_slv0_reg(0x10);
printf("Read data for address 0x10 is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x14);
printf("Read data for address 0x14 is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x18);
printf("Read data for address 0x18 is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x1c);
printf("Read data for address 0x1c is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x20);
printf("Read data for address 0x20 is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x24);
printf("Read data for address 0x24 is %x\n",rd_wr_data );
printf("1st set of reads completed\n");

printf("Entered 2nd set of writes \n");
write_pcie_slv0_reg(0x10,0x0);
write_pcie_slv0_reg(0x14,0x4);
write_pcie_slv0_reg(0x18,0x20000000);
write_pcie_slv0_reg(0x1c,0x40000000);
write_pcie_slv0_reg(0x20,0x60000000);
write_pcie_slv0_reg(0x24,0x80000000);
printf("2nd set of writes completed\n");

printf("Entered 2nd set of reads \n");
rd_wr_data = read_pcie_slv0_reg(0x10);
printf("Read data for address 0x10 is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x14);
printf("Read data for address 0x14 is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x18);
printf("Read data for address 0x18 is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x1c);
printf("Read data for address 0x1c is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x20);
printf("Read data for address 0x20 is %x\n",rd_wr_data );
rd_wr_data = read_pcie_slv0_reg(0x24);
printf("Read data for address 0x24 is %x\n",rd_wr_data );
printf("2nd set of reads completed\n");

#ifdef DEBUG_DISPLAY
printf("Enabling Memory,IO,Bus master enable\n");
#endif
write_pcie_slv0_reg(0x4,0x7);
#endif

#ifdef DM1_RC
mem_base_program_dm1_x4();
#ifdef DEBUG_DISPLAY
printf("Reading Vendor ID,Device ID and Bars from PCIe Instance1\n");
#endif
for(i =0;i<10;i = i+1)
{
rd_wr_data1 = read_pcie_slv1_reg(i*0x4);
}
#ifdef DEBUG_DISPLAY
printf("Enabling Memory,IO,Bus master enable");
#endif
write_pcie_slv1_reg(0x4,0x7);
write_pcie_slv1_reg(0x10,0xFFFFFFFF);
write_pcie_slv1_reg(0x14,0xFFFFFFFF);
write_pcie_slv1_reg(0x18,0xFFFFFFFF);
write_pcie_slv1_reg(0x1c,0xFFFFFFFF);
write_pcie_slv1_reg(0x20,0xFFFFFFFF);
write_pcie_slv1_reg(0x24,0xFFFFFFFF);

rd_wr_data = read_pcie_slv1_reg(0x10);
rd_wr_data = read_pcie_slv1_reg(0x14);
rd_wr_data = read_pcie_slv1_reg(0x18);
rd_wr_data = read_pcie_slv1_reg(0x1c);
rd_wr_data = read_pcie_slv1_reg(0x20);
rd_wr_data = read_pcie_slv1_reg(0x24);

write_pcie_slv1_reg(0x10,0x0);
write_pcie_slv1_reg(0x14,0x4);
write_pcie_slv1_reg(0x18,0x20000000);
write_pcie_slv1_reg(0x1c,0x40000000);
write_pcie_slv1_reg(0x20,0x60000000);
write_pcie_slv1_reg(0x24,0x80000000);

rd_wr_data = read_pcie_slv1_reg(0x10);
rd_wr_data = read_pcie_slv1_reg(0x14);
rd_wr_data = read_pcie_slv1_reg(0x18);
rd_wr_data = read_pcie_slv1_reg(0x1c);
rd_wr_data = read_pcie_slv1_reg(0x20);
rd_wr_data = read_pcie_slv1_reg(0x24);

#endif


wait_on(10);
data_rd = read_reg(0xE6004100); 
while(data_rd != 0x12345678)
{
    wait_on(5);
    #ifdef DEBUG_DISPLAY
    printf("In While loop ,data_rd = %x",data_rd);
    #endif
    data_rd = read_reg(0xE6004100); 
   
}
 #ifdef DEBUG_DISPLAY
    printf("Out of While loop\n ");
 #endif
    finish(0);
}





