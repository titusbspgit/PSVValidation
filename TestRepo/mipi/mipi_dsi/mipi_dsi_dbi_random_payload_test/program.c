#include <stdio.h>
#include <stdlib.h>

#include "test_common.h"
#include "mipi_dsi.h"
#include <math.h>


//CH0 - data
//CH1 - command


desc_t data_tdbdcb[10];
desc_t data_rebdcb[10];
desc_t cmd_tdbdcb[10];
desc_t cmd_rebdcb[10];



int test_case()
{
    pixel_num_bytes_wr_cmd_size_t pixel_attr;
    unsigned int phy_if_cfg;
    unsigned long int gen_pld_data;
    unsigned int num_bytes;
    unsigned int num_descriptors;
    unsigned int num_of_pixel;
    unsigned int itter;
    unsigned int wr_cmd_arr[27];
    unsigned int WR_CMD_SIZE[27];
    unsigned int i;
    unsigned int total_num_bytes;
    unsigned int num_wr_cmds;
    unsigned int rd_data;
    phy_stop_wait_time = 0x40;
    n_lanes = 3;

    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg,MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,phy_stop_wait_time);
    write_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG,phy_if_cfg); //PHY_IF_CFG      
    write_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3d  );      //PCKHDL_CFG
    write_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107 );      //CLKMGR_CFG

    write_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL,0);         //DBI_enable
    phy_init();

    //DBI_VCID
    dbi_vcid = 0x3;

    load_cmd_or_data_to_sram = 1;       //To load command and data into lss SRAM, for data integrity

    //DBI_CFG
    lut_size_conf = 0x1;
    out_dbi_conf = 0xb;
    in_dbi_conf = 0x0;

    //DBI_PARTITIONING_EN
    partitioning_en = 0x0;

    //DBI_CMDSIZE
   // allowed_cmd_size = 0x7;
    allowed_cmd_size = 0x25;
    wr_cmd_size = 192 + 1;

    num_of_pixel = 40;
    pixel_attr = pixel_to_bytes_wr_cmd_size(num_of_pixel);
    wr_cmd_size = pixel_attr.wr_cmd_size;
    num_bytes = pixel_attr.num_bytes;

    max_rd_pkt_size     = 0x0;
    dcs_lw_tx           = 0x0;
    dcs_sr_0p_tx        = 0x0;
    dcs_sw_1p_tx        = 0x0;
    dcs_sw_0p_tx        = 0x0;
    gen_lw_tx           = 0x0;
    gen_sr_2p_tx        = 0x0;
    gen_sr_1p_tx        = 0x0;
    gen_sr_0p_tx        = 0x0;
    gen_sw_2p_tx        = 0x0;
    gen_sw_1p_tx        = 0x0;
    gen_sw_0p_tx        = 0x0;
    ack_rqst_en         = 0x0;
    tear_fx_en          = 0x1;

    generic_vc_id       = 0x2;


//Added begin
    dpi_clk_time_period     = 16.012400; //older revision svn rtl dbi_clk period = ~16.012400ns
    dpi_clk_freq = 1000000000/dpi_clk_time_period;
    program_dpi_clock(dpi_clk_freq); 
//Added end
    dbi_config();


    num_descriptors = 1;




    write_reg(MIZAR_MIPI_DSI_DMAC_INTEN,0x3);
    //for(i = 0;i < 50; i++)
    for(i = 0;i < 10; i++)
    {
   //     num_of_pixel = (rand() % 8192) ;
        num_of_pixel =  ((rand() % 1024) ) * 8 ;
        pixel_attr = pixel_to_bytes_wr_cmd_size(num_of_pixel);
        wr_cmd_size = pixel_attr.wr_cmd_size;
        num_bytes = pixel_attr.num_bytes;
        printf("i = %d, wr_cmd_size = %d\n",i,wr_cmd_size);
        dbi_config();
        //CH0 - data
        //CH1 - command
        ch0_desc_addr = RAM_BASE + 0x3F000;
        ch1_desc_addr = RAM_BASE + 0x3F800;
        ch0_desc_addr_act = ch0_desc_addr;
        ch1_desc_addr_act = ch1_desc_addr;        
        for(itter = 0; itter < num_descriptors; itter++)
        {

            data_tdbdcb[itter].addr = RAM_BASE + 0x10000 + (itter * 0x1000);
            data_tdbdcb[itter].len = num_bytes;
            data_tdbdcb[itter].eop = (itter == (num_descriptors - 1));

            data_rebdcb[itter].addr = 0x10000000000;
            data_rebdcb[itter].len = num_bytes;

            cmd_tdbdcb[itter].addr = RAM_BASE + 0x0000;
            cmd_tdbdcb[itter].len = 4;
            cmd_tdbdcb[itter].eop = (itter == (num_descriptors - 1));

            cmd_rebdcb[itter].addr = 0x10000008000;
            cmd_rebdcb[itter].len = 4;


            DMAMOV(&ch0_desc_addr,DMA_SAR, data_tdbdcb[itter].addr);
            DMAMOV(&ch0_desc_addr,DMA_DAR, data_rebdcb[itter].addr);
            program_data_num_bytes(&ch0_desc_addr,(ceil((double)data_tdbdcb[itter].len/8))*8);
            DMAWMB(&ch0_desc_addr);
            DMASEV(&ch0_desc_addr,0);
            DMAEND(&ch0_desc_addr);

            DMAMOV(&ch1_desc_addr,DMA_SAR, cmd_tdbdcb[itter].addr);
            DMAMOV(&ch1_desc_addr,DMA_DAR, cmd_rebdcb[itter].addr);
            program_data_num_bytes(&ch1_desc_addr,cmd_tdbdcb[itter].len);
            DMAWMB(&ch1_desc_addr);
            DMASEV(&ch1_desc_addr,1);
            DMAEND(&ch1_desc_addr);



            ////loading random data
            load_rand_data(data_tdbdcb[itter]);

            ////loading cmd in SRAM
            load_wr_command(cmd_tdbdcb[itter].addr,num_bytes,DSI_WRITE_MEMORY_START);

        }
        write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0 , 0x00A00000);
        write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1 , ch0_desc_addr_act);
        write_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD   , 0x0);

        write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0 , 0x01A00000);
        write_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1 , ch1_desc_addr_act);
        write_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD   , 0x0);


        rd_data = read_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        while(rd_data != 0x3)
        {
            rd_data = read_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        }
        write_reg(MIZAR_MIPI_DSI_DMAC_INTCLR,rd_data);


#ifdef PROC_MODE
        wait_on(100000);
#else
        wait_on(100000);
#endif
    }

    finish(0);
}




