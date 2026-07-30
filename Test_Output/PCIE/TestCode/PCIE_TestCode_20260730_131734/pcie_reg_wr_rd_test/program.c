// Author - AI Force 1.3.2. Date 30-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* =============================================================================
 * Function Declarations
 * ============================================================================= */
static void chk_rst_val(void);
static void chk_rd_wr(void);

/* =============================================================================
 * Global/Test-scoped variables (deterministic, no optimization)
 * ============================================================================= */
static volatile unsigned int data_rd = 0;       /* read buffer */
static volatile int err1 = 0;                   /* error counter 1 */
static volatile int err2 = 0;                   /* error counter 2 */
static int i = 0;                               /* loop index */
static int j = 0;                               /* loop index */

/* =============================================================================
 * Function: chk_rst_val
 * Brief   : Implements the reset-value checks exactly as per Meta Steps
 * ============================================================================= */
static void chk_rst_val(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rst_val()\n");
#endif
    for(i=0;i<5;i++){
        data_rd=read_reg(rc0_ctl_addr[i]);
        if(data_rd!=ctl_default[i]){
            err1++;
            printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n",ctl_default[i],data_rd);
        }
    }
    for(i=0;i<5;i++){
        data_rd=read_reg(rc1_ctl_addr[i]);
        if(data_rd!=ctl_default[i]){
            err2++;
            printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n",ctl_default[i],data_rd);
        }
    }
    for(i=0;i<3;i++){
        data_rd=read_reg(sii0_addr[i]);
        if(data_rd!=sii_default[i]){
            err2++;
            printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n",sii_default[i],data_rd);
        }
    }
    for(i=0;i<3;i++){
        data_rd=read_reg(sii1_addr[i]);
        if(data_rd!=sii_default[i]){
            err2++;
            printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n",sii_default[i],data_rd);
        }
    }
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL,0x01203000);
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL,0x01203000);
    for(i=0;i<3;i++){
        data_rd=read_reg(phy0_addr[i]);
        data_rd=(phy0_addr[i]%4)?(data_rd>>16):(data_rd&0x0000FFFF);
        if(data_rd!=phy0_default[i]){
            err2++;
            printf("Reset value mismatch => Reg address : 0x%x, Default value : 0x%x, Read data : 0x%x : FAILED\n",phy0_addr[i],phy0_default[i],data_rd);
        }
    }
    for(i=0;i<3;i++){
        data_rd=read_reg(phy1_addr[i]);
        data_rd=(phy1_addr[i]%4)?(data_rd>>16):(data_rd&0x0000FFFF);
        if(data_rd!=phy1_default[i]){
            err2++;
            printf("Reset value mismatch => Reg address : 0x%x, Default value : 0x%x, Read data : 0x%x : FAILED\n",phy1_addr[i],phy1_default[i],data_rd);
        }
    }
}

/* =============================================================================
 * Function: chk_rd_wr
 * Brief   : Implements the write/readback tests exactly as per Meta Steps
 * ============================================================================= */
static void chk_rd_wr(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rd_wr()\n");
#endif
    int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};
    int chk_val_phy[3]={0x7baf,0x1,0x003b};
    for(j=0;j<3;j++){
        for(i=0;i<5;i++){
            write_reg(rc0_ctl_addr[i],chk_val[j]);
        }
        for(i=0;i<5;i++){
            write_reg(rc1_ctl_addr[i],chk_val[j]);
        }
        for(i=0;i<3;i++){
            write_reg(sii0_addr[i],(chk_val[j] & sii0_write_mask[i]));
        }
        for(i=0;i<3;i++){
            write_reg(sii1_addr[i],(chk_val[j] & sii1_write_mask[i]));
        }
        write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL,0x01203000);
        write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL,0x01203000);
        for(i=0;i<3;i++){
            write_reg(phy0_addr[i], chk_val_phy[j] & phy0_write_mask[i]);
        }
        for(i=0;i<3;i++){
            write_reg(phy1_addr[i], chk_val_phy[j] & phy1_write_mask[i]);
        }
        for(i=0;i<5;i++){
            data_rd=read_reg(rc0_ctl_addr[i]);
            if(data_rd!=chk_val[j]){
                err1++;
                printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n",chk_val[j],data_rd);
            }
        }
        for(i=0;i<5;i++){
            data_rd=read_reg(rc1_ctl_addr[i]);
            if(data_rd!=chk_val[j]){
                err1++;
                printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n",chk_val[j],data_rd);
            }
        }
        for(i=0;i<3;i++){
            data_rd=read_reg(sii0_addr[i]);
            if(data_rd!=(chk_val[j] & sii0_write_mask[i])){
                err1++;
                printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n",chk_val[j],data_rd);
            }
        }
        for(i=0;i<3;i++){
            data_rd=read_reg(sii1_addr[i]);
            if(data_rd!=(chk_val[j] & sii1_write_mask[i])){
                err1++;
                printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n",chk_val[j],data_rd);
            }
        }
        for(i=0;i<3;i++){
            data_rd=read_reg(phy0_addr[i]);
            data_rd=(phy0_addr[i]%4)?(data_rd>>16):(data_rd&0x0000FFFF);
            if((data_rd & phy0_write_mask[i])!=(chk_val_phy[j] & 0x00001FFF)){
                err1++;
                printf("Data mismatch => Reg address : 0x%x, Write data : 0x%x, Read data : 0x%x : FAILED\n",phy0_addr[i],chk_val_phy[j],data_rd);
            }
        }
        for(i=0;i<3;i++){
            data_rd=read_reg(phy1_addr[i]);
            data_rd=(phy1_addr[i]%4)?(data_rd>>16):(data_rd&0x0000FFFF);
            if((data_rd & phy1_write_mask[i])!=(chk_val_phy[j] & 0x00001FFF)){
                err1++;
                printf("Data mismatch => Reg address : 0x%x, Write data : 0x%x, Read data : 0x%x : FAILED\n",phy1_addr[i],chk_val[j],data_rd);
            }
        }
    }
}

/* =============================================================================
 * Function: test_case (Entry Point)
 * Brief   : Orchestrates the test as per Meta Steps and terminates via finish()
 * ============================================================================= */
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter test_case()\n");
#endif
    printf("Entered test case\n");
    chk_rst_val();
    printf("READ_WRITE test_case called\n");
    chk_rd_wr();
    finish(err2 || err1);
    return 0; /* Not reached if finish() terminates */
}
