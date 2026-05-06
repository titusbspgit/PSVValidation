#include<lss_sysreg.h>
#include<stdio.h>
#include<test_define.c>
#include<test_common.h>


unsigned int gpio_number,test_err,i;
extern int int_pend;

void test_case() 
{
unsigned int rdata,wr_val;
test_err = 0;

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
    #ifdef GPIO0
        GIC_ClearIRQ(87);
    #endif

    #ifdef GPIO1
        GIC_ClearIRQ(88);
    #endif	
}





    










