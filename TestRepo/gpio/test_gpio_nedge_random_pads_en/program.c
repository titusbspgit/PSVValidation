#include<lss_sysreg.h>
#include<stdlib.h>
#include<stdio.h>
#include<test_define.c>
#include<test_common.h>
#include<time.h>

int pad_num,test_err;
extern int int_pend;
int i,j;

int test_case() 
{
int rdata,wr_val,num;
int arr[32];
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif

#ifdef GPIO1
     GIC_EnableIRQ(88);
#endif

printf("test_case\n");

  
  test_err = 0;
 
 
 // enabling sysreg interrupt
  #ifdef GPIO0

      write_reg(MIZAR_LSS_SYSREG_INTR_EN1,LSS_SYSREG_INTR_EN1_GPIO0_INTR);

  #endif

  #ifdef GPIO1
       write_reg(MIZAR_LSS_SYSREG_INTR_EN1,LSS_SYSREG_INTR_EN1_GPIO1_INTR);

 #endif

 srand(time(NULL));
 write_reg(0xA0243ffc,0xffffffff);

  // For enabling input mode and negedge interrupt for GPIOs 8-39
  for(i = 0; i < 32; i++)
  { 
        pad_num = rand() % 32;
        for(j=0;j <= i-1;j++)
        {
            if(pad_num == arr[j])
            {
                break;
            }
        }
        if(i == j)
        {
            arr[i] = pad_num;
            wr_val = 1<<pad_num;
            write_reg(MIZAR_GPIO_GP0_GPIO_8+ (pad_num * 4),0x00140000);
            wait_on(50);

            //enabling the gpio group interrupt
            write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,1<<pad_num); //84
     
            //writing into sram location
            
            wait_on(10);
            write_reg(0xA0243ffc,~(wr_val));

            wait_on(10);
            write_reg(0xA0243ffc,0xffffffff);
            int_pend = 1;   
            while(int_pend == 0x1)
           {
              printf("Waiting for interrupt\n");
              wait_on(10);
           }

        }
        else
        {
            i=i-1;
        }
 }
 
 finish(test_err);
}

void Default_IRQHandler() 
{

  int j,rdata,rdata_grp,wr_val;
  wr_val = 1<<pad_num;
  int_pend = 0x0;
    #ifdef DEBUG_DISPLAY
      printf("\nEntered into default IRQ Handler!! with pad value = %d\n",pad_num);
    #endif


      rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8+(pad_num * 4));
      
      // Check for DIN value during negedge for active/enabled Pad  
      if((rdata & 0x1) != 0)
      {
        #ifdef DEBUG_DISPLAY
           printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: DIN value matches with the Pad_value ..read data = %0x\n",pad_num,rdata);
        #endif
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: DIN value does not match with the Pad_value read_data = %0x\n",pad_num,rdata);
        test_err++; 
      }

      // Check for interrupt raw status bit during negedge for active/enabled Pad
      if((rdata & 0x2) != 0x0)
      {
          #ifdef DEBUG_DISPLAY
             printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: Raw Interrupt raised at negedge\n",pad_num,rdata);
          #endif
	
	  rdata_grp =read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);//88
	
	if((rdata_grp & (1<<pad_num)) != 0)      
      	{
          #ifdef DEBUG_DISPLAY
            printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: group Interrupt raised\n",pad_num,rdata_grp);
          #endif
	}
	else
	{
	printf("ERROR: Group Interrupt not occured\n");
	test_err = test_err + 1;
	}
        
        // Clearing the interrupt raw status bit (16th bit of GPIO reg set to '1')
       write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0x00000000);

	write_reg(MIZAR_GPIO_GP0_GPIO_8 + (pad_num * 4),0x00110001);
	wait_on(2);
	rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (pad_num * 4)); 
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



