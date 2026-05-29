#include <stdio.h>
#include "hal_gpio.h"
#include "gic_funcs.h"
#include <lss_sysreg.h>
#include <gpio_negedge_intr_en.h>

unsigned int gpio_number,test_err,rdata,wr_val,i,addr1;
int int_pend;

int gpio_negedge_intr_en_init(const TestsItem *cfg)
{
    (void)cfg;
    printf("[Test Init] GPIO test: %s\n", cfg->test_name);
    LOGI("[Test Init]   GPIO test: %s\n", cfg->test_name);
    
    return 0;
}

int gpio_negedge_intr_en_run(const TestsItem *cfg, TestOutput *out)
{
    LOGI("[Test Run] GPIO test: %s \n", cfg->test_name);
    test_err = 0;
    #ifdef GPIO0
        GIC_EnableIRQ(87);
    #endif
    
    #ifdef GPIO1
        GIC_EnableIRQ(88);
    #endif
 
    //enabling sysreg interrupt
    #ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_INTR_EN1,LSS_SYSREG_INTR_EN1_GPIO0_INTR);
    #endif

    #ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_INTR_EN1,LSS_SYSREG_INTR_EN1_GPIO1_INTR);
    #endif
    
    write_reg(0xA0243ffc,0xffffffff);
 
    // For enabling input mode and negedge interrupt for GPIOs 8-39
    for(i = 0; i < 32; i++)
    { 
        // Programming GPIO in Input Mode and enabling negedge interrupt(20th bit as '1' & 18th bit as '1')
        addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
     
        write_reg(addr1,0x00140000);

        wait_on(50);

        wr_val = 1<<i;

        //enabling the gpio group interrupt
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,wr_val); //84
     
        //writing into sram location
        wait_on(10);
        write_reg(0xA0243ffc,0xffffffff);

        wait_on(30);
        write_reg(0xA0243ffc,~(wr_val));
    
        int_pend = 1;

        while(int_pend)
        {
            //printf("Waiting for interrupt\n");
            LOGI("Waiting for interrupt %s\n");
            wait_on(10);
        }

    }
  
    return out->status = test_err;
}

void Default_IRQHandler() 
{
    unsigned int rdata_grp,raddr,raddr2;
    //wr_val = 1<<i;
    int_pend = 0;

    write_reg(0xA0243ffc,0xffffffff);

    raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
    rdata = read_reg(raddr);
  
    #ifdef DEBUG_DISPLAY
        //printf("Entered into default IRQ Handler!! with pad value = %d",i);
        LOGI("Entered into default IRQ Handler!! with pad value = %d",i);
    #endif
    //Check for DIN value during negedge for active/enabled Pad  
    if((rdata & 0x1) != 0x0)
    {
        #ifdef DEBUG_DISPLAY
            //printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: DIN value matches with the Pad_value ..read data = %0x\n",i,rdata);
	    LOGI("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: DIN value matches with the Pad_value ..read data = %0x\n",i,rdata);
        #endif
    }
    else 
    {
        //printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: DIN value does not match with the Pad_value read_data = %0x\n",i,rdata);
	LOGI("ERROR: GPIO_NUM = %0d Default_IRQHandler:: DIN value does not match with the Pad_value read_data = %0x\n",i,rdata);
        test_err++; 
    }

    // Check for interrupt raw status bit during negedge for active/enabled Pad
    if((rdata & 0x2) != 0x0)
    {
        #ifdef DEBUG_DISPLAY
            //printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: Raw Interrupt raised at negedge\n",i,rdata);
	    LOGI("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: Raw Interrupt raised at negedge\n",i,rdata);
        #endif
	
	rdata_grp =read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);//88
	
	if((rdata_grp & (wr_val)) != 0)          
        {
            #ifdef DEBUG_DISPLAY
                //printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: group Interrupt raised\n",i,rdata_grp);
	        LOGI("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: group Interrupt raised\n",i,rdata_grp);
            #endif
	}
	else
	{
	    //printf("ERROR: Group Interrupt not occured\n");
	    LOGI("ERROR: Group Interrupt not occured\n");
	    test_err = test_err + 1;
	}
        
        // Clearing the interrupt raw status bit (16th bit of GPIO reg set to '1')
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1,wr_val);

        raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
        write_reg(raddr2,0x00110001);
	rdata = read_reg(raddr2); 

	if(rdata == 0x100001)
	{  
            #ifdef DEBUG_DISPLAY
                //printf("SUCCESS : Interrupt cleared successfully  rdata = %0x\n",rdata);
		LOGI("SUCCESS : Interrupt cleared successfully  rdata = %0x\n",rdata);
            #endif
	}
	else
	{		
            //printf("ERROR : Interrupt clear failed : Interrupt value = %x\n",rdata);
	    LOGI("ERROR : Interrupt clear failed : Interrupt value = %x\n",rdata);
	    test_err = test_err + 1;
	}

        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0x00000000);

	rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);//88
	if(rdata_grp == 0x0)
	{
            #ifdef DEBUG_DISPLAY
	        //printf("SUCCESS : Group Interrupt cleared successfully\n");
	        LOGI("SUCCESS : Group Interrupt cleared successfully\n");
                #endif
	}
	else
	{
            //printf("ERROR : Group Interrupt clear failed: Interrupt value:%x\n",rdata_grp);
	    LOGI("ERROR : Group Interrupt clear failed: Interrupt value:%x\n",rdata_grp);
             test_err = test_err + 1;
	}

        #ifdef GPIO0 
	    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
            GIC_ClearIRQ(87);                     
        #endif

        #ifdef GPIO1          
            write_reg(MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
            GIC_ClearIRQ(88);                 
        #endif 
    }

    else
    {
        //printf("Interrupt Not occured\n");
	LOGI("Interrupt Not occured\n");
	test_err++;
    }

}

int gpio_negedge_intr_en_teardown(const TestsItem *cfg)
{
    (void)cfg;
    //printf("[DONE] GPIO teardown: %s\n", cfg->test_name);
    LOGI("[TEARDOWN] GPIO teardown: %s\n", cfg->test_name);

    return 0;
}
