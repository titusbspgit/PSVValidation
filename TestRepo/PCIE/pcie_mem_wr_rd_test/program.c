#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>


#include "pcie.h"
unsigned int data_rd,data_wr,rd_wr_data1;
int err2 = 0;
int err1 = 0;

int test_case()
{
	write_reg(0xE6004100,0x0);

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
printf("TEST SII0 READ DATA data before entering DM0_EP  = %x\n",data_rd);
#endif
//FOR DM0_RC
#ifdef DM0
while(((data_rd)&(0xD1))!=0xD1)
{
   data_rd = read_sii0_reg(0xC0);
  // #ifdef DEBUG_DISPLAY
   printf("TEST SII0 READ DATA data = %x\n",data_rd);
  // #endif
}
#endif

//For DM1_RC
#ifdef DM1
data_rd = read_sii1_reg(0xC0);
while(((data_rd)&(0xD1))!=0xD1)
{
#ifdef DEBUG_DISPLAY
printf("TEST SII1 READ DATA data = %x\n",data_rd);
#endif
data_rd = read_sii1_reg(0xC0);
}
#endif


#ifdef DM0_EP
wait_on(30000);
#endif
//while(data_rd != 0x87654321)
//{
//    wait_on(5);
//    printf("In While loop ,data_rd = %x",data_rd);
//    read_reg(0x401FFF9C,&data_rd); //Index : 'h3fc
//}
//    printf("Out of While loop ,data_rd = %x",data_rd);
//

#ifdef DM0_RC 
#ifdef DEBUG_DISPLAY
printf("Reading Vendor ID DM0\n");
#endif
rd_wr_data1 = read_pcie_slv0_reg(0x0);
printf("VENDOR ID : 0x%x\n",rd_wr_data1);
write_pcie_slv0_reg(0x4,0x7);
//rd_wr_data1 = read_pcie_slv0_reg(0x4);
//printf("COMMAND REG : 0x%x\n",rd_wr_data1);
bar_program_dm0_x4();
#ifdef DEBUG_DISPLAY
printf("\nMemory base Programming Started\n");
#endif
wait_on(10);
mem_base_program_dm0_x4();
#endif

#ifdef DM1_RC 
#ifdef DEBUG_DISPLAY
printf("Reading Vendor ID DM1\n");
#endif
rd_wr_data1 = read_pcie_slv1_reg(0x0);
printf("VENDOR ID : 0x%x\n",rd_wr_data1);
write_pcie_slv1_reg(0x4,0x7);
//rd_wr_data1 = read_pcie_slv0_reg(0x4);
//printf("COMMAND REG : 0x%x\n",rd_wr_data1);
bar_program_dm1_x4();
#ifdef DEBUG_DISPLAY
printf("\nMemory base Programming Started\n");
#endif
wait_on(10);
mem_base_program_dm1_x4();
#endif
 
#ifdef DM0_EP
   printf(" Entered DM0 EP bar register programming\n");
   bar_program_dm0_EP_x4();
   printf(" DONE DM0 EP bar register programming\n");
   wait_on(10);
   printf("\nMemory base Programming Started\n");
   mem_base_program_dm0_x4();
   printf("\nMemory base Programming Ended\n");
#endif

#ifdef DM1_EP
   printf(" Entered DM1 EP bar register programming");
   bar_program_dm1_EP_x4();
   printf(" DONE DM1 EP bar register programming");
   wait_on(10);
   printf("\nMemory base Programming Started\n");
   mem_base_program_dm1_x4();
   printf("\nMemory base Programming Ended\n");

#endif
    non_secure_prot_nic();
    write_reg(0xE6004100,0x11111111);
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
#ifdef DM0_RC
pcie_slv0_mem_wr_rd(0x01040000,0xa5a5a5a5);
pcie_slv0_mem_wr_rd(0x01000020,0xa6a6a6a6);
pcie_slv0_mem_wr_rd(0x01004000,0xa7a7a7a7);
#endif

#ifdef DM1_RC
pcie_slv1_mem_wr_rd(0x01040000,0xb5b5b5b5);
pcie_slv1_mem_wr_rd(0x01000020,0xb5b5b6b6);
pcie_slv1_mem_wr_rd(0x01004000,0xb7b7b5b5);
#endif

#ifdef DM0_EP
  pcie_slv0_mem_wr_rd(0x10100,0x5a5a5a5a); // For Bar1
  pcie_slv0_mem_wr_rd(0x20100,0x5a5a5a5a); // For Bar1
  pcie_slv0_mem_wr_rd(0x1B100,0x5a5a5a5a); // For Bar1
  pcie_slv0_mem_wr_rd(0x2B100,0x5a5a5a5a); // For Bar1
  pcie_slv0_mem_wr_rd(0x30100,0x5a5a5a5a); // For Bar1
#endif


#ifdef DM1_EP
  pcie_slv1_mem_wr_rd(0x10100,0x5a5a5a5a); // For Bar1
  pcie_slv1_mem_wr_rd(0x20100,0x5a5a5a5a); // For Bar1
  pcie_slv1_mem_wr_rd(0x1B100,0x5a5a5a5a); // For Bar1
  pcie_slv1_mem_wr_rd(0x2B100,0x5a5a5a5a); // For Bar1
  pcie_slv1_mem_wr_rd(0x30100,0x5a5a5a5a); // For Bar1
#endif

 wait_on(10);
 data_rd = read_reg(0xE6004100); 
 while(data_rd != 0x12345678)
 {
     wait_on(5);
     #ifdef DEBUG_DISPLAY
     printf("In While loop ,data_rd = %x\n",data_rd);
     #endif
     data_rd = read_reg(0xE6004100); 
    
 }
 #ifdef DEBUG_DISPLAY 
     printf("Out of While loop\n");
 #endif    
    finish(0);

}
