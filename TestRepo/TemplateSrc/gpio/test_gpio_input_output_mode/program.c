#include<lss_sysreg.h>
#include<stdio.h>
#include<test_define.c>
#include<test_common.h>
int gpio_number,test_err;
int gp0_flag_dout_one = 0;
int gp0_flag_dout_zero = 0;
int gp0_flag_din_one = 0;
int gp0_flag_din_zero = 0;
int i;
void test_case() 
{

int wr_val;
test_err = 0;
wr_val = 1<<i;
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif

#ifdef GPIO1
     GIC_EnableIRQ(88);
#endif

     // For enabling output mode GPIOs 8-39
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,0x00FF00FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,0x00FF00FF);

    
    wait_on(10);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0xFFFFFFFF); 

    for(i = 0;i < 16;i++)
    {
         
            write_reg(0xA0243ffc,1<<i);
            wait_on(10);
            gp0_flag_din_one = 1;
            check_for_pad_value(i);

            wait_on(30);
            write_reg(0xA0243ffc,0x00000000);
            gp0_flag_din_zero = 1;
            check_for_pad_value(i);
     }

     for(i = 16; i < 32;i++)
     {
         
        write_reg(MIZAR_GPIO_GP0_GPIO_8+ (i * 4),0x00200000);
        gp0_flag_dout_one = 1;
        check_for_pad_value(i);

        wait_on(20);
        write_reg(MIZAR_GPIO_GP0_GPIO_8+ (i * 4),0x00000000);
        gp0_flag_dout_zero = 1;
        check_for_pad_value(i);
    }

     finish(test_err);
}

void check_for_pad_value(unsigned int gpio_pad_num) 
{
 int rdata,j;
  #ifdef DEBUG_DISPLAY
    printf("\nEntered into check pad value with pad num = %d\n", gpio_pad_num);
  #endif

  if(((i >= 0) & (i < 8)) != 0)
 {
     rdata = read_reg(MIZAR_GPIO_GPIO_DIN_GROUP1);
 }
 else if(((i >= 8) & (i < 16)) != 0)
 {
     rdata = read_reg(MIZAR_GPIO_GPIO_DIN_GROUP2);
     j = i - 8;
 }
 if(gp0_flag_din_one == 1)
 {
     if(((rdata & (1 << (gpio_pad_num))) != 0) | ((rdata & (1 << j)) != 0))
    {
         #ifdef DEBUG_DISPLAY
          printf("SUCCESS: GPIO_NUM = %0d DIN_ONE:: GPIO Pad value matches with din value %x\n",gpio_pad_num,rdata);
         #endif
          gp0_flag_din_one = 0;
    }

    else 
    {
          printf("ERROR: GPIO_NUM = %0d DIN_ONE:: GPIO Pad value does not match with din value %x\n",gpio_pad_num,rdata);
          gp0_flag_din_one = 0;

          test_err++;
    }
 }
 if(gp0_flag_din_zero == 1)
 {
        // Check Pad value for enabled GPIO
        if(((rdata & (1 << (gpio_pad_num))) == 0) | ((rdata & (1 << j)) == 0))

        {
          #ifdef DEBUG_DISPLAY
            printf("SUCCESS: GPIO_NUM = %0d DIN_ZERO:: GPIO Pad value matches with din value %x\n",gpio_pad_num,rdata);
         #endif
          gp0_flag_din_zero = 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DIN_ZERO:: GPIO Pad value does not match with din value %x\n",gpio_pad_num,rdata);
          gp0_flag_din_zero = 0;

          test_err++;
        }
 }		

  rdata = read_reg(0xA0243ff8);

  if(gp0_flag_dout_one == 1)
  {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << (i - 16))) != 0)
        {
         #ifdef DEBUG_DISPLAY
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value matches with dout value %x\n",gpio_pad_num,rdata);
         #endif
          gp0_flag_dout_one = 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value does not match with dout value %x\n",gpio_pad_num,rdata);
          gp0_flag_dout_one = 0;

          test_err++;
        }
   }

  if(gp0_flag_dout_zero == 1)
 {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << (i - 16))) == 0)
        {
          #ifdef DEBUG_DISPLAY
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value matches with dout value %x\n",gpio_pad_num,rdata);
         #endif
          gp0_flag_dout_zero = 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value does not match with dout value %x\n",gpio_pad_num,rdata);
          gp0_flag_dout_zero = 0;

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

void Default_IRQHandler() {

  printf("ERROR:: Entered Default IRQ_Handler.. :( \n ");
  test_err++;
}



  




