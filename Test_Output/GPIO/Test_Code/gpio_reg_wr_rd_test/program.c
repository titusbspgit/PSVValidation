#include "test_define.c"

/* Hidden_Test_Description:
   test_case() calls chk_rst_val() and chk_rd_wr().
   chk_rst_val(): iterates i=0..CNT-1 over addr_array[], skips per skip_rst_array or read_mask==0, then reads via read_reg(addr),
   masks with 0xfffffffe, and compares to default_value_array[i], incrementing def_fail_cnt on mismatch.
   chk_rd_wr(): for each data pattern, writes masked values to each addr if allowed and not skipped, then reads back masked values,
   computing expected value with read/write mask and defaults. finish(1) if any fail counters > 0 else finish(0).
   Notes: VRRW registers are skipped as per skip arrays. DIN may become 1 if not forced; forcing DIN low can impact reads.
*/

static unsigned int def_fail_cnt = 0;
static unsigned int wr_fail_cnt  = 0;

static void chk_rst_val(void)
{
    unsigned int i;
    for(i=0;i<CNT;i++){
        unsigned long addr = addr_array[i];
        if(skip_rst_array[i]==1){
            continue; /* Skip reset-value check for this register */
        }
        if(read_mask_array[i]==0){
            continue; /* Nothing readable to compare */
        }
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEu); /* mask bit0 as per requirement */
        if(data==default_value_array[i]){
            /* pass */
        } else {
            def_fail_cnt++;
            printf("RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\tRead_data : 0x%x\tDATA : 0x%x\n",
                   (unsigned int)addr, default_value_array[i], data, data_rd);
        }
    }
}

static void chk_rd_wr(void)
{
    unsigned int i, j;
    for(j=0;j<6;j++){
        unsigned int data_wr = chk_val[j];
        /* Write phase: write masked data to all writable, non-skipped regs */
        for(i=0;i<CNT;i++){
            unsigned long addr = addr_array[i];
            if(skip_array[i]==1){
                continue;
            }
            if(write_mask_array[i]==0){
                continue;
            }
            write_reg(addr, (data_wr & write_mask_array[i]));
        }
        /* Read/verify phase */
        for(i=0;i<CNT;i++){
            unsigned long addr = addr_array[i];
            if(skip_array[i]==1){
                continue;
            }
            if(write_mask_array[i]==0){
                continue;
            }
            if(read_mask_array[i]==0){
                continue;
            }
            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));
            if(data_rd == exp_val){
                /* pass */
            } else {
                wr_fail_cnt++;
                printf("Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\tRead value=0x%x\n",
                       (unsigned int)addr, exp_val ,data_rd);
            }
        }
    }
}

void test_case(void)
{
    chk_rst_val();
    chk_rd_wr();
    if(def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1); else finish(0);
}
