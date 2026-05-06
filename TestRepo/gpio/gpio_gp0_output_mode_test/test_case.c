#include<stdio.h>
#include<testdef.h>

int gpio_number,flag;
int gp0_flag_dout_one = 0;
int gp0_flag_dout_zero = 0;

void test_case() {

int i,j,k;
  
  flag = 0;
 
  pinmux_for_gpio_func();
// Programming the GPIO Pads 0-7 in Output mode
  for(j = 0;j < 6;j++) {

    printf("Enabling GPIO_%d register fields..\n",j);
  //sensing gpio pads 
    sec_wr(0x400A4000 + (j * 4),0x0);
  }

  // Programming the GPIO Pads 8-39 in Output mode
  for(j = 6;j < 32;j++) {

    printf("Enabling GPIO_%d register fields..\n",j);
   
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8+ ((j-6) * 4),0x0);
  }

  // Writing DOUT field for GPIOs 0-7 
  for(i = 0;i < 4;i++) {

    gpio_number = i;
 #ifdef MIZAR_40_PIN_PKG

    if(i == 2){

printf("ignored this pad = %0d \n",i);

         continue;
} 

#endif 

    printf("GPIO_OUTPUT_MODE:: Toggling the GPIO pins by writing into DOUT field of GPIO register_%d..\n",i);
   
    for(j = 0;j < 5;j++) 
   {
      // Write GPIO register dout bit as '1' 
   //printf("GPIO_OUTPUT_MODE:: Toggling the GPIO pins by writing into DOUT field of GPIO register_3.. to 1\n");
      sec_rmw(0x400A4000+ (i * 4),0x00200000,0x00200000);
      gp0_flag_dout_one = 1;
      for(j = 0; j < 32; j++) {
        check_for_pad_value(j);
      }
          
      // Write GPIO register dout bit as '0' 
      sec_rmw(0x400A4000+ (i * 4),0x00200000,0x00000000);
      gp0_flag_dout_zero = 1;
      for(j = 0; j < 32; j++) {
        check_for_pad_value(j);
      }
    }
  }



  // Writing DOUT field for GPIOs 0-39 
  for(i = 6;i < 32;i++) {

    gpio_number = i;
 #ifdef MIZAR_40_PIN_PKG

if(i>=16 && i <=23){

printf("ignored this pad::GPIO_NUM  = %0d \n",i );

continue;
}

else if (i>=25 && i<=32){



printf("ignored this pad::GPIO_NUM = %0d \n",i);

continue;


}

#endif

    printf("GPIO_OUTPUT_MODE:: Toggling the GPIO pins by writing into DOUT field of GPIO register_%d..\n",i);
   
    //for(j = 0;j < 5;j++) 
    //{
      // Write GPIO register dout bit as '1' 
      rmw(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((i-6 ) * 4),0x00200000,0x00200000);
      gp0_flag_dout_one = 1;
      for(j = 0; j < 32; j++) {
        check_for_pad_value(j);
      }
          
      // Write GPIO register dout bit as '0' 
      rmw(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((i-6 ) * 4),0x00200000,0x00000000);
      gp0_flag_dout_zero = 1;
      for(j = 0; j < 32; j++) {
        check_for_pad_value(j);
      }
    //}
  }  
 
  finish(flag);
}

// Read the scratch register and check the corresponding Pad value for GPIOs 0-39  
void check_for_pad_value(unsigned int gpio_pad_num) {

          printf("Entered into check pad value \n");
int rdata;
    
   
  if(gpio_pad_num >= 0 && gpio_pad_num < 4)
  { 
    // Read the scratch register 1 for corresponding GPIO pad value 
    rdata = sec_rd(I2BW_ADC0_CH6_BUF0_START_ADDR);
    
    if(gpio_number == gpio_pad_num)
    {
      if(gp0_flag_dout_one == 1)
      {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << gpio_pad_num)) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value matches with dout value\n",gpio_pad_num);
          gp0_flag_dout_one = 0;
        }
        else {
          printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value does not match with dout value\n",gpio_pad_num);
          flag++;
        }
      }
      if(gp0_flag_dout_zero == 1)
      {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << gpio_pad_num)) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value matches with dout value\n",gpio_pad_num);
          gp0_flag_dout_zero = 0;
        }
        else {
          printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value does not match with dout value\n",gpio_pad_num);
          flag++;
        }
      }
    }
    else {
      // Check Pad value for disabled GPIOs
      if((rdata & (1 << gpio_pad_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d GPIO Pad value matches with dout value\n",gpio_pad_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d GPIO Pad value does not match with dout value\n",gpio_pad_num);
        flag++;
      }
    } 
  }


  if(gpio_pad_num >= 6 && gpio_pad_num < 32)
  { 
    // Read the scratch register 2 for corresponding GPIO pad value 
    rdata = sec_rd(I2BW_ADC0_CH6_BUF1_START_ADDR);
    //rdata = sec_rd(I2BW_ADC0_CH6_BUF0_START_ADDR);
    //rdata = rd(0x40036004);
    
    if(gpio_number == gpio_pad_num)
    {
      if(gp0_flag_dout_one == 1)
      {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << (gpio_pad_num -6 ))) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value matches with dout value\n",gpio_pad_num);
          gp0_flag_dout_one = 0;
        }
        else {
          printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value does not match with dout value\n",gpio_pad_num);
          flag++;
        }
      }
      if(gp0_flag_dout_zero == 1)
      {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << (gpio_pad_num -6 ))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value matches with dout value\n",gpio_pad_num);
          gp0_flag_dout_zero = 0;
        }
        else {
          printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value does not match with dout value\n",gpio_pad_num);
          flag++;
        }
      }
    }
    else {
      // Check Pad value for disabled GPIOs
      if((rdata & (1 << (gpio_pad_num -6 ))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d GPIO Pad value matches with dout value\n",gpio_pad_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d GPIO Pad value does not match with dout value\n",gpio_pad_num);
        flag++;
      }
    } 
  }
}


void Default_IRQHandler() {

  printf("ERROR:: Entered Default IRQ_Handler.. :( \n ");
  flag++;
}



