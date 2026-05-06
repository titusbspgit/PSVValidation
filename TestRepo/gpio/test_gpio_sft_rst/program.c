#include<lss_sysreg.h>
#include<stdio.h>
#include<test_define.c>
#include<test_common.h>
#define CNT 49

unsigned int gpio_number,test_err,i,k;
extern int int_pend;
unsigned int data_rd,data_wr,data,rst_val;
unsigned int def_fail_cnt = 0,wr_fail_cnt = 0;

void test_case() 
{
unsigned int rdata,wr_val;
test_err = 0;
k=0;
i = 0;
//enabling GIC 
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif

#ifdef GPIO1
     GIC_EnableIRQ(88);
#endif
 
 
 // enabling sysreg interrupt
  #ifdef GPIO0

      write_reg(MIZAR_LSS_SYSREG_INTR_EN1,LSS_SYSREG_INTR_EN1_GPIO0_INTR);

  #endif

  #ifdef GPIO1

       write_reg(MIZAR_LSS_SYSREG_INTR_EN1,LSS_SYSREG_INTR_EN1_GPIO1_INTR);

  #endif

  write_reg(0xA0243ffc,0xffffffff);
  // For enabling input mode and posedge interrupt for GPIOs 8-39
  for(i = 0; i < 32; i++)
  { 
      k = k+1;
     #ifdef STRAP_2_3
      if(i == 24)
      {
         break;
       }
     #endif
     // Programming GPIO in Input Mode and enabling posedge interrupt(20th bit as '1' & 17th bit as '1')
     wr_val = 1<<i;
     write_reg(MIZAR_GPIO_GP0_GPIO_8+ (i * 4),0x00120000);

     wait_on(50);

     //enabling the gpio group interrupt
     write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,wr_val); //84
     
     //writing into sram location
     wait_on(10);
     write_reg(0xA0243ffc,~(wr_val));

     wait_on(30);
     write_reg(0xA0243ffc,0xffffffff);
     int_pend = 0x1;
     
       while(int_pend == 0x1)
     {
          printf("Waiting for interrupt\n");
          wait_on(10);
     }
  }


  for(i = 0; i < 32; i++)
  { 
     #ifdef STRAP_2_3
      if(i == 24)
      {
         break;
       }
     #endif
     // Programming GPIO in Input Mode and enabling posedge interrupt(20th bit as '1' & 17th bit as '1')
     wr_val = 1<<i;
     write_reg(MIZAR_GPIO_GP0_GPIO_8+ (i * 4),0x00120000);

     wait_on(50);

     //enabling the gpio group interrupt
     write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,wr_val); //84
     
     //writing into sram location
     wait_on(10);
     write_reg(0xA0243ffc,~(wr_val));

     wait_on(30);
     write_reg(0xA0243ffc,0xffffffff);
     int_pend = 0x1;
     
       while(int_pend == 0x1)
     {
          printf("Waiting for interrupt\n");
          wait_on(10);
     }
  }

 finish(test_err);
}

void Default_IRQHandler() 
{

  unsigned int j,rdata,rdata_grp,wr_val;
  wr_val = 1<<i;
  int_pend = 0x0;

#ifdef DEBUG_DISPLAY
  printf("Entered into default IRQ Handler!! with pad value = %d\n",i);
#endif
     if(k == 10)
    {
        #ifdef GPIO0
              write_reg(MIZAR_LSS_SYSREG_SFT_RST,0x3ffffbff);
              write_reg(MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        #endif

        #ifdef GPIO1
              write_reg(MIZAR_LSS_SYSREG_SFT_RST,0x3ffff7ff);
              write_reg(MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        #endif
        wait_on(5000);
        write_reg(MIZAR_LSS_SYSREG_SFT_RST,0x3fffffff);
        wait_on(5000);
        chk_rst_val();

        rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)); 
	  if(rdata == 0x100001)
	  {
             #ifdef DEBUG_DISPLAY
		printf("SUCCESS : Interrupt cleared successfully  rdata = %0x\n",rdata);
            #endif
	  }
 	  else
	  {		
		printf("ERROR : Interrupt clear failed : Interrupt value = %x\n",rdata);
		test_err = test_err + 1;
	  }
          i=32;
          k=0;
          int_pend = 0;

     }
  
     
 else
 {
    rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8+(i * 4));
      
      // Check for DIN value during posedge for active/enabled Pad  
      if((rdata & 0x1) != 0)
      { 
        #ifdef DEBUG_DISPLAY
         printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: DIN value matches with the Pad_value ..read data = %0x\n",i,rdata);
        #endif
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: DIN value does not match with the Pad_value read_data = %0x\n",i,rdata);
        test_err++; 
      }

      // Check for interrupt raw status bit during posedge for active/enabled Pad
      if((rdata & 0x2) != 0x0)
      {
          #ifdef DEBUG_DISPLAY
           printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: Raw Interrupt raised at posedge\n",i,rdata);
          #endif
	
	  rdata_grp =read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);//88
	
	  if((rdata_grp & (1<<i)) != 0)      
      	  {
             #ifdef DEBUG_DISPLAY
                printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: group Interrupt raised\n",i,rdata_grp);
             #endif
	  }
	  else
	  {
	     printf("ERROR: Group Interrupt not occured\n");
	     test_err = test_err + 1;
	  }
        
          // Clearing the interrupt raw status bit (16th bit of GPIO reg set to '1')

	  write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4),0x00110001);
	  wait_on(20);
	  rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)); 
	  if(rdata == 0x100001)
	  {
             #ifdef DEBUG_DISPLAY
		printf("SUCCESS : Interrupt cleared successfully  rdata = %0x\n",rdata);
            #endif
	  }
 	  else
	  {		
		printf("ERROR : Interrupt clear failed : Interrupt value = %x\n",rdata);
		test_err = test_err + 1;
	  }

          write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0x00000000);

	  rdata_grp =read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);//88
	  if(rdata_grp == 0x0)
	  {
             #ifdef DEBUG_DISPLAY
	       printf("SUCCESS : Group Interrupt cleared successfully\n");
             #endif
	  }
	  else
	  {
	       printf("ERROR : Group Interrupt clear failed: Interrupt value:%x\n",rdata_grp);
	       test_err = test_err + 1;
	  }
          #ifdef GPIO0
 
	        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
                rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
                if((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) == 0)
                {
                    #ifdef DEBUG_DISPLAY
                       printf("sysreg status cleared : %0x\n",MIZAR_LSS_SYSREG_RAW_STCR1);
                    #endif
                }
                else
                {
                      printf("sysreg status not cleared : %0x\n",MIZAR_LSS_SYSREG_RAW_STCR1);
                      test_err++;
                }

         #endif

         #ifdef GPIO1
                  
                 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
      
                 rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
                 if((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) == 0)
                 {
                     #ifdef DEBUG_DISPLAY
                       printf("sysreg status cleared : %0x\n",MIZAR_LSS_SYSREG_RAW_STCR1);
                     #endif
                 }
                 else
                {
                       printf("sysreg status not cleared : %0x\n",MIZAR_LSS_SYSREG_RAW_STCR1);
                       test_err++;
                }

        #endif 

    }

    else
    {
	printf("Interrupt Not occured\n");
	test_err++;
    }
}
    #ifdef GPIO0
        GIC_ClearIRQ(87);
    #endif

    #ifdef GPIO1
        GIC_ClearIRQ(88);
    #endif	
}

void chk_rst_val()
{
	unsigned int n;
        unsigned long int addr;
	for(n =0;n<CNT;n++)
	{
		addr=addr_array[n];
                if(skip_rst_array[n] == 1)
		{
			#ifdef DEBUG_DISPLAY
			 printf("RST : this Address : 0x%x is skipped because address present in skip_array \n",addr);
			#endif
			continue;
		}

		if(read_mask_array[n] == 0x00000000)
		{
			#ifdef DEBUG_DISPLAY
				printf("RST : This address 0x%x is not readable, hence skipped for reading \n",addr);
			#endif
			continue;
		}

		data_rd = read_reg(addr);
                data = (data_rd & 0xfffffffe);
		if(data == default_value_array[n])
		{
			#ifdef DEBUG_DISPLAY
				printf("RST : PASS Reading Default value from Address :0x%x Expected : 0x%x\tRead_data : 0x%x\n",addr,default_value_array[n],data);		
			#endif
		}
		else
		{
			def_fail_cnt++;
			printf("RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\tRead_data : 0x%x\tDATA : 0x%x\n",addr,default_value_array[n],data,data_rd);
		}
	}
}

