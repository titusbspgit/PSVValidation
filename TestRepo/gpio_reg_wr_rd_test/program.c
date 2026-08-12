#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "test_define.c"

unsigned int data_rd,data_wr,data;
unsigned int def_fail_cnt = 0,wr_fail_cnt = 0;

#define SOFT_RST_REG_ADDRESS	0x00000000
#define SOFT_RST_REG_DATA	0x00000000

int test_case()
{

	chk_rst_val();
	#ifdef DEBUG_DISPLAY
		printf("********* Default value check end ************\n");		
	#endif
       // write_reg(0xA0243ffc,0x12345678);
	chk_rd_wr();
	#ifdef DEBUG_DISPLAY
		printf("********* Write & Read from registers end ************\n");		
	#endif

	if(def_fail_cnt > 0 || wr_fail_cnt > 0) {
	        finish(1);
	} else {
	        finish(0);
	}
}

//******************* DEFAULT VALUE_CHECK ****************//

void chk_rst_val()
{
	unsigned int i;
        unsigned long int addr;
	for(i =0;i<CNT;i++)
	{
		addr=addr_array[i];
                if(skip_rst_array[i] == 1)
		{
			#ifdef DEBUG_DISPLAY
			 printf("RST : this Address : 0x%x is skipped because address present in skip_array \n",addr);
			#endif
			continue;
		}

		if(read_mask_array[i] == 0x00000000)
		{
			#ifdef DEBUG_DISPLAY
				printf("RST : This address 0x%x is not readable, hence skipped for reading \n",addr);
			#endif
			continue;
		}

		data_rd = read_reg(addr);
                data = (data_rd & 0xfffffffe);
		if(data == default_value_array[i])
		{
			#ifdef DEBUG_DISPLAY
				printf("RST : PASS Reading Default value from Address :0x%x Expected : 0x%x\tRead_data : 0x%x\n",addr,default_value_array[i],data);		
			#endif
		}
		else
		{
			def_fail_cnt++;
			printf("RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\tRead_data : 0x%x\tDATA : 0x%x\n",addr,default_value_array[i],data,data_rd);
		}
	}
}

//******************* WRITE & READ CHECK ****************//

void chk_rd_wr()
{
	unsigned int i,j,exp_val,wr_n;
        unsigned long int addr;

	unsigned int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000};
	for(j=0;j<6;j++)
	{
		data_wr=chk_val[j];

		// writing into Registers

		for(i =0;i<CNT;i++)
		{
			addr=addr_array[i];	
			if(skip_array[i] == 1)
			{
				#ifdef DEBUG_DISPLAY
					printf("Read_write : Writing into this Address : 0x%x is skipped because address present in skip_array \n",addr);
				#endif
				continue;
			}
			if((write_mask_array[i] == 0x00000000))
			{
				#ifdef DEBUG_DISPLAY
					printf("Read_write : This address 0x%x is not writable, hence skipped for writing \n",addr);
				#endif
				continue;
			}
			else
			{
				write_reg(addr,(data_wr & write_mask_array[i]));
				#ifdef DEBUG_DISPLAY
					printf("Read_write : Writing into register Address : 0x%x\tdata :0x%x\n",addr,data_wr);
				#endif
			}
		}

		//Reading from registers		
	
		for(i =0;i<CNT;i++)
		{
			addr=addr_array[i];
			if(skip_array[i] == 1)
			{
				#ifdef DEBUG_DISPLAY
					printf("Read_write : Reading from this Address : 0x%x is skipped because address present in skip_array \n",addr);
				#endif
				continue;
			}
			if(write_mask_array[i] == 0x00000000)
			{
				#ifdef DEBUG_DISPLAY
					printf("Read_write : This address 0x%x is not Writable , hence skipped for reading \n",addr);
				#endif
				continue;
			}
			if(read_mask_array[i] == 0x00000000)
			{
				#ifdef DEBUG_DISPLAY
					printf("Read_write : This address 0x%x is not Readable , hence skipped for reading \n",addr);
				#endif
				continue;
			}
			else
			{
                                data_rd = (read_reg(addr) & read_mask_array[i]);
				wr_n= (write_mask_array[i] ^ 0xffffffff);
				exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]) );
				if(data_rd == exp_val)
				{
                                          #ifdef DEBUG_DISPLAY
					
						printf("Read_write : PASS : For Address %x, Expected value=0x%x\tRead value=0x%x\n",addr,exp_val ,data_rd);
                                          #endif
				    
				}
				else
				{
                                    
					wr_fail_cnt++;
					printf("Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\tRead value=0x%x\n",addr,exp_val ,data_rd);                        
                                
				}

			}

		}
			
	}

}

void soft_reset_chk() {
	#ifdef 0
	int default_value;
        default_value = read_reg(SOFT_RST_REG_ADDRESS);
	write_reg(SOFT_RST_REG_ADDRESS,SOFT_RST_REG_DATA);
	wait_on(1000);
	write_reg(SOFT_RST_REG_ADDRESS,default_value);
	wait_on(1000);
	#endif
}
