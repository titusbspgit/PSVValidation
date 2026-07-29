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

	int i;
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
data_rd = read_sii1_reg(0xC0);
while(((data_rd)&(0xD1))!=0xD1)
{
#ifdef DEBUG_DISPLAY
printf("TEST SII1 READ DATA data = %x\n",data_rd);
#endif
data_rd = read_sii1_reg(0xC0);
}

//while(data_rd != 0x87654321)
//{
//    wait_on(5);
//    printf("In While loop ,data_rd = %x",data_rd);
//    read_reg(0x401FFF9C,&data_rd); //Index : 'h3fc
//}
//    printf("Out of While loop ,data_rd = %x",data_rd);
#ifdef DM0_RC
#ifdef DEBUG_DISPLAY
printf("Reading Vendor ID");
#endif
rd_wr_data1 = read_pcie_slv0_reg(0x0);
printf("VENDOR ID : 0x%x",rd_wr_data1);
write_pcie_slv0_reg(0x4,0x7);
#ifdef DEBUG_DISPLAY
printf("Memory base Programming Started");
#endif
mem_base_program_dm0_x4();
mem_base_program_dm1_x4();
wait_on(10);
#endif

write_reg(0xE690000C,0x1);
write_reg(0xE6900010,0x1);
write_reg(0xE6900014,0x1);
write_reg(0xE6900018,0x1);
write_reg(0xE6900030,0x1);
write_reg(0xE6900034,0x1);
//write_reg(0xE6004100,0x11111111);
//while(data_rd != 0x12345678)
//{
//    wait_on(5);
//    printf("In While loop ,data_rd = %x",data_rd);
//    read_reg(0x401FFF9C,&data_rd); //Index : 'h3fc
//}
//    printf("Out of While loop ,data_rd = %x",data_rd);

//DISABLE_CACHE PROGRAMMING
rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),11,14,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,3,6,0xf); 
write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1);

rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),27,30,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,19,22,0x0); 
write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1); 

rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),11,14,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,3,6,0xf); 
write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1);

rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),27,30,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,19,22,0x0); 
write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1); 

wait_on(10);
rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF),11,14,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,3,6,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,27,30,0x0); 
rd_wr_data1 = set_data(rd_wr_data1,19,22,0x0); 
write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1); 

rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF),11,14,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,3,6,0xf); 
rd_wr_data1 = set_data(rd_wr_data1,27,30,0x0); 
rd_wr_data1 = set_data(rd_wr_data1,19,22,0x0); 
write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF,rd_wr_data1);

wait_on(30);
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

write_pcie_slv0_reg(0x10,0xFFFFFFFF);
write_pcie_slv0_reg(0x14,0xFFFFFFFF);
write_pcie_slv0_reg(0x18,0xFFFFFFFF);
write_pcie_slv0_reg(0x1c,0xFFFFFFFF);
write_pcie_slv0_reg(0x20,0xFFFFFFFF);
write_pcie_slv0_reg(0x24,0xFFFFFFFF);

rd_wr_data = read_pcie_slv0_reg(0x10);
rd_wr_data = read_pcie_slv0_reg(0x14);
rd_wr_data = read_pcie_slv0_reg(0x18);
rd_wr_data = read_pcie_slv0_reg(0x1c);
rd_wr_data = read_pcie_slv0_reg(0x20);
rd_wr_data = read_pcie_slv0_reg(0x24);

write_pcie_slv0_reg(0x10,0x0);
write_pcie_slv0_reg(0x14,0x4);
write_pcie_slv0_reg(0x18,0x20000000);
write_pcie_slv0_reg(0x1c,0x40000000);
write_pcie_slv0_reg(0x20,0x60000000);
write_pcie_slv0_reg(0x24,0x80000000);

rd_wr_data = read_pcie_slv0_reg(0x10);
rd_wr_data = read_pcie_slv0_reg(0x14);
rd_wr_data = read_pcie_slv0_reg(0x18);
rd_wr_data = read_pcie_slv0_reg(0x1c);
rd_wr_data = read_pcie_slv0_reg(0x20);
rd_wr_data = read_pcie_slv0_reg(0x24);


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





