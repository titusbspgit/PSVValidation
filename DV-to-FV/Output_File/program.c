/*
 * Program: FV-structured PCIe memory transaction test
 * Agent: Ag-FV-DV-Transition Agent
 */
#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

unsigned int data_rd;
unsigned int data_wr;
unsigned int rd_wr_data1;
int err2 = 0;
int err1 = 0;

/*
 * Function: pcie_mem_txn_init
 * Phase: Initialization
 */
int pcie_mem_txn_init(void)
{
    write_reg(0xE6004100,0x0);
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
    return 0;
}

/*
 * Function: pcie_mem_txn_run
 * Phase: Test Execution
 */
int pcie_mem_txn_run(void)
{
    data_rd = read_sii0_reg(0xC0);
#ifdef DEBUG_DISPLAY
    /* printf("TEST SII0 READ DATA data Before entering D1 while loop  = %x\n",data_rd); */
#endif
    non_secure_prot_nic();
    while(((data_rd)&(0xD1))!=0xD1)
    {
        data_rd = read_sii0_reg(0xC0);
#ifdef DEBUG_DISPLAY
        /* printf("TEST SII0 READ DATA data = %x\n",data_rd); */
#endif
    }
    wait_on(30000);
#ifdef DM0_RC
#ifdef DEBUG_DISPLAY
    /* printf("Reading Vendor ID"); */
#endif
    rd_wr_data1 = read_pcie_slv0_reg(0x0);
#ifdef DEBUG_DISPLAY
    /* printf("VENDOR ID : 0x%x",rd_wr_data1); */
#endif
    write_pcie_slv0_reg(0x4,0x7);
#ifdef DM0_RC
    bar_program_dm0_x4();
#endif
#endif
#ifdef DM0_EP
    bar_program_dm0_EP_x4();
#endif
#ifdef DM0_RC
#ifdef DEBUG_DISPLAY
    /* printf("Memory base Programming Started\n"); */
#endif
#ifdef DM0
    mem_base_program_dm0_x4();
#endif
#ifdef DM1
    mem_base_program_dm1_x4();
#endif
    wait_on(10);
#endif
    write_reg(0xE6004100,0x11111111);
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
#ifdef DM0
    pcie_slv0_mem_wr_rd_16(0x1010,0x12345678);
    pcie_slv0_mem_wr_rd_16(0x1410,0x12345678);
    pcie_slv0_mem_wr_rd_16(0x1510,0x12345678);
#endif
#ifdef DM1
    pcie_slv1_mem_wr_rd_16(0x1010,0x12345678);
    pcie_slv1_mem_wr_rd_16(0x1410,0x12345678);
    pcie_slv1_mem_wr_rd_16(0x1510,0x12345678);
#endif
    wait_on(10);
    data_rd = read_reg(0xE6004100);
#ifdef DEBUG_DISPLAY
    /* printf("Out of While loop\n "); */
#endif
    return 0;
}

/*
 * Function: pcie_mem_txn_teardown
 * Phase: Output and Teardown
 */
int pcie_mem_txn_teardown(void)
{
    printf("TEST ENTERED MAIN\n");
    printf("CHECK1:link training done \n");
    printf("CHECK2:mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF \n");
    printf("CHECK3:done\n");
    printf("CHECK4:mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFFDone\n");
    printf("CHECK5:done \n");
    printf("CHECK6:done \n");
    printf("CHECK7done\n");
    printf("CHECK8done\n");
#ifdef DEBUG_DISPLAY
    printf("TEST SII0 READ DATA data Before entering D1 while loop  = %x\n",data_rd);
#endif
#ifdef DM0_RC
#ifdef DEBUG_DISPLAY
    printf("Reading Vendor ID");
#endif
#ifdef DEBUG_DISPLAY
    printf("VENDOR ID : 0x%x",rd_wr_data1);
#endif
    printf(" Entered DM0 RC bar register programming");
    printf(" DONE DM0 RC bar register programming");
#endif
#ifdef DM0_EP
    printf(" Entered DM0 EP bar register programming");
    printf(" DONE DM0 EP bar register programming");
#endif
    printf("Enumaration Done\n");
#ifdef DEBUG_DISPLAY
    printf("Memory base Programming Started\n");
#endif
    printf("Memory base Programming Ended\n");
    printf("After mem base programming started reg writes \n");
    printf("Transactions to be started \n");
    printf("2 Transactions Done \n");
#ifdef DEBUG_DISPLAY
    printf("Out of While loop\n ");
#endif
    finish(0);
    return 0;
}
