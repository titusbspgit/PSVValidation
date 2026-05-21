#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

unsigned int data_rd,data_wr,data1_rd;
unsigned int err2 = 0;
unsigned int err1 = 0;
unsigned int rc0_ctl_addr[5] = {mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,mizar_PCIE0_DBI_DSP_UTILITY_OFF};
unsigned int rc1_ctl_addr[5] = {mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG, mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,mizar_PCIE1_DBI_DSP_UTILITY_OFF};
unsigned int ctl_default[5] = {0x0, 0x0, 0x0, 0x0, 0x0};
unsigned int sii0_addr[3] = {mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2, mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,mizar_PCIE0_SII_PHY_CONTROL_23};
unsigned int sii1_addr[3] = {mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2, mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,mizar_PCIE1_SII_PHY_CONTROL_23};
unsigned int sii_default[3] = {0x0, 0x0,0x0};

unsigned int sii0_write_mask[3] = {0xFFFFFFFF,0xFFFFFFFF,0xF000F};
unsigned int sii1_write_mask[3] = {0xFFFFFFFF,0xFFFFFFFF,0xF000F};

unsigned int phy0_addr[3] = {0xE68860B8,0xE68862B8,0xE68864B8};
unsigned int phy1_addr[3] ={0xE68A60B8,0xE68A62B8,0xE68A64B8};
//unsigned int phy_default[4] = {PCIE0_PHY_SUP_DIG_IDCODE_LO_DEFAULT_VAL,PCIE0_PHY_RAWLANE0_DIG_AON_FAST_FLAGS_DEFAULT_VAL,PCIE0_PHY_RAWLANE3_DIG_AON_FAST_FLAGS_DEFAULT_VAL,PCIE0_PHY_RAWLANEX_DIG_RX_CTL_RX_FSM_CTL_DEFAULT_VAL};
unsigned int phy0_default[3] = {0x0,0x0,0x0};
unsigned int phy1_default[3] = {0x0,0x0,0x0};

unsigned int phy0_write_mask[3] = {0x1FFF,0x1FFF,0x1FFF};

unsigned int phy1_write_mask[3] = {0x1FFF,0x1FFF,0x1FFF};


int test_case()
{
	int i;
        
	printf("Entered test case\n");
	chk_rst_val();
	 printf("READ_WRITE test_case called\n");
	chk_rd_wr();

        finish(err2 || err1);
}




//*******************RESET_CHECK****************
void chk_rst_val()
{
	int i,addr;
	for(i = 0 ; i < 5; i++)
        {
            data_rd = read_reg(rc0_ctl_addr[i]);
            if(data_rd != ctl_default[i])
                {
			err1++;
			 printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n",ctl_default[i],data_rd);
		}        
		} 
       for(i = 0 ; i < 5; i++)
        {
            data_rd = read_reg(rc1_ctl_addr[i]);
            if(data_rd != ctl_default[i])
               {
			err2++;
			 printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n",ctl_default[i],data_rd);
		}
        } 
        
        for(i = 0 ; i < 3; i++)
        {
            data_rd = read_reg(sii0_addr[i]);
            if(data_rd != sii_default[i])
                {
			err2++;
			 printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n",sii_default[i],data_rd);
		}
        }
        for(i = 0 ; i < 3; i++)
        {
            data_rd = read_reg(sii1_addr[i]);
            if(data_rd != sii_default[i])
                {
			err2++;
			 printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n",sii_default[i],data_rd);
		}
        }
        
         write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL,0x01203000);
         write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL,0x01203000);

        for(i = 0 ; i < 3; i++)
        {
            data_rd = read_reg((phy0_addr[i]));
            if(phy0_addr[i]%4)
                data_rd = data_rd >> 16;
            else
                data_rd = data_rd & 0x0000FFFF;
            if(data_rd != phy0_default[i])
                {
			err2++;
			 printf("Reset value mismatch => Reg address : 0x%x, Default value : 0x%x, Read data : 0x%x : FAILED\n",phy0_addr[i],phy0_default[i],data_rd);
		}
        }

        
        for(i = 0 ; i < 3; i++)
        {
            data_rd = read_reg((phy1_addr[i]));
            if(phy1_addr[i]%4)
                data_rd = data_rd >> 16;
            else
                data_rd = data_rd & 0x0000FFFF;
            if(data_rd != phy1_default[i])
                {
			err2++;
			 printf("Reset value mismatch => Reg address : 0x%x, Default value : 0x%x, Read data : 0x%x : FAILED\n",phy1_addr[i],phy1_default[i],data_rd);
		}
        }

}




//*******************READ_WRITE_CHECK****************
void chk_rd_wr()
{
	int i,addr,j,exp_val,wr_n;
	int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};
	int chk_val_phy[3]={0x7baf,0x1,0x003b};
	for(j=0;j<3;j++)
	{
	    for(i = 0; i < 5; i++)
            {
                write_reg(rc0_ctl_addr[i],chk_val[j]);
            } 
            for(i = 0; i < 5; i++)
            {
                write_reg(rc1_ctl_addr[i],chk_val[j]);
            }
            
	    for(i = 0; i < 3; i++)
            {
                write_reg(sii0_addr[i],(chk_val[j] & sii0_write_mask[i]));
            }
	   
            for(i = 0; i < 3; i++)
            {
                write_reg(sii1_addr[i],(chk_val[j] & sii1_write_mask[i]));
            }
            
             write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL,0x01203000);
             write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL,0x01203000);

            for(i = 0; i < 3; i++)
            {
                write_reg(phy0_addr[i],chk_val_phy[j] & phy0_write_mask[i]);
            }
            for(i = 0; i < 3; i++)
            {
                write_reg(phy1_addr[i],chk_val_phy[j] & phy1_write_mask[i]);
            }

            for(i = 0; i < 5; i++)
            {
                data_rd = read_reg(rc0_ctl_addr[i]);
                if(data_rd != chk_val[j]) {
                    err1++;
		     printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n",chk_val[j],data_rd);
		}
            }
            for(i = 0; i < 5; i++)
            {
                data_rd = read_reg(rc1_ctl_addr[i]);
		if(data_rd != chk_val[j])
		{
			err1++;
			 printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n",chk_val[j],data_rd);
		}
	    }
            	    
	 for(i = 0 ; i < 3; i++)
        {
            data_rd = read_reg(sii0_addr[i]);
            if(data_rd != (chk_val[j] & sii0_write_mask[i]))
                {
			err1++;
			 printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n",chk_val[j],data_rd);
		}        
          }
        for(i = 0 ; i < 3; i++)
        {
            data_rd = read_reg(sii1_addr[i]);
            if(data_rd != (chk_val[j] & sii1_write_mask[i]))
               {
			err1++;
			 printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n",chk_val[j],data_rd);
		}        
        }
        for(i = 0; i < 3; i++)
        {
            data_rd = read_reg(phy0_addr[i]);
            if(phy0_addr[i]%4)
                data_rd = data_rd >> 16;
            else
                data_rd = data_rd & 0x0000FFFF;
            if((data_rd & phy0_write_mask[i]) != (chk_val_phy[j] & 0x00001FFF))
               {
			err1++;
			 printf("Data mismatch => Reg address : 0x%x, Write data : 0x%x, Read data : 0x%x : FAILED\n",phy0_addr[i],chk_val_phy[j],data_rd);
		}        }
        
        for(i = 0; i < 3; i++)
        {
            data_rd = read_reg(phy1_addr[i]);
            if(phy1_addr[i]%4)
                data_rd = data_rd >> 16;
            else
                data_rd = data_rd & 0x0000FFFF;
            if((data_rd & phy1_write_mask[i]) != (chk_val_phy[j] & 0x00001FFF))
               {
			err1++;
			 printf("Data mismatch => Reg address : 0x%x, Write data : 0x%x, Read data : 0x%x : FAILED\n",phy1_addr[i],chk_val[j],data_rd);
		}
        }

        }


}
