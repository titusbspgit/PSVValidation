#include<testdef.h>

int gpio_number,flag;
int flag_din_one = 0;
int flag_din_zero = 0;
extern int int_pend;

void test_case() {

int i,j,k,rdata;
  
  flag = 0;
  
  pinmux_for_gpio_func();

  // For enabling posedge interrupt for GPIOs 0-7  
  for(i = 0; i < 4; i++) {
    
    printf("Enabling GPIO_3 register fields..\n");
    
    // Enabling posedge interrupt(17th bit as '1')
    sec_wr(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00020000);

    // Clearing the raw status interrupt bit (Set 16th bit to '1') 
    sec_wr(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00030000);
  }
 
  // For enabling posedge interrupt for GPIOs 8-39  
  for(i = 0;i < 26;i++) {
    
    printf("Enabling GPIO_0 register fields..\n");
    
    // Enabling posedge interrupt(17th bit as '1')
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00020000);

    // Clearing the raw status interrupt bit (Set 16th bit to '1') 
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00030000);
  }

  // For clearing the combined interrupt status register for GPIOs 0-7 
  for(i = 0; i < 4; i++) {
    
    printf("Enabling GPIO_3 register fields..\n");
    
    // Clearing the CPU raw interrupt status bit 
    sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1,0x0);
  }
  
  // For clearing the combined interrupt status register for GPIOs 8-39
  for(i = 0; i < 26; i++) {
    
    printf("Enabling GPIO_0 register fields..\n");
    
    // Clearing the CPU raw interrupt status bit 
    wr(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1,0x0);
  }

  // Enabling alternate GPIOs per virtual register([7:0] and [23:16] set to 'FF' and 'AA' respectively) for GPIO Pads 0-7
  sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_IO_CTRL_GROUP1,0x00AA00FF);

  // Enabling alternate GPIOs per virtual register([7:0] and [23:16] set to 'FF' and 'AA' respectively) for GPIO Pads 8-39
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP1,0x00AA00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP2,0x00AA00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP3,0x00AA00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP4,0x00AA00FF);

  // Initializing the scratch pad register value to '0'
  sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0);
  sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x0);

  // Writing the scratch pad register for GPIO pads 0-7 
  for(i = 0;i < 4; i++) {
 
    printf("Toggling the GPIO pins by writing into scratch register_1..\n");

    gpio_number = i;
  #ifdef MIZAR_40_PIN_PKG

    if(i == 2){

printf("ignored this pad = %0d \n",i);

         continue;
} 

#endif
  
    if((i % 2) == 0) 
    {
      // Enable CPU interrupt
      int_pend = 1;
      enable_cpu_intr(i);
 
      sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,1 << i);
      wait_on(20);
      while(int_pend) {
        wait_on(10);
      }
      
      sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0);
      wait_on(500);
      for(j = 0; j < 32; j++) {
        if(gpio_number % 2 == 0)
        {  
          if(j % 2 != 0)
          {
            check_for_din_and_intr(j);
          }
        }
        else {
          if(j % 2 == 0)
          {
            check_for_din_and_intr(j);
          }
        }
      }
 
      sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,1 << i);
      wait_on(20);
    
      for(j = 0; j < 6; j++) {
    
        if(gpio_number == j) 
        {
          rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);

          // Check DIN value for Group1 GPIOs (0:7)  
          if((rdata & (1 << j)) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP1_GP3:: DIN value matches with the Pad_value\n",j);
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP1_GP3:: DIN value does not match with the Pad_value\n",j);
            flag++; 
          }

          // Reading the DIN and interrupt raw status values from GPIO Register
          rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4));

          // Check for DIN value for active/enabled Pad when CPU interrupt is disabled
          if((rdata & 0x1) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: DIN value matches with the Pad_value\n",j);
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: DIN value does not match with the Pad_value\n",j);
            flag++; 
          }
          
          // Check for interrupt raw status bit for active/enabled Pad when CPU interrupt is disabled
          if((rdata & 0x2) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: Raw Interrupt raised at posedge\n",j);
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: Raw Interrupt not raised\n",j);
            flag++;
          }
          
          //Check for CPU raw interrupt status
          rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
          
          if((rdata & (1 << j)) != 0)      
          {
            printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: CPU Raw Interrupt raised\n",j);
          }
          else
          {
            printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: CPU Raw Interrupt not raised\n",j);
            flag++;
          }
          
          // Check for processor interrupt when the CPU interrupt is disabled 
          #ifdef PACMAN_PROC 
            rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
            
            if((rdata & (1 << j)) == 0)
            {
              printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: Interrupt not raised at processor\n",j);
            }
            else 
            {
              printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: Interrupt raised at processor\n",j);
              flag++;
            }
          #else
            rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
            
            if((rdata & (1 << j)) == 0)
            {
              printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: Interrupt not raised at processor\n",j);
            }
            else 
            {
              printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: Interrupt raised at processor\n",j);
              flag++;
            }
          #endif
        }
        else {
          // Check for DIN, Interrupt raw status and CPU interrupt status for disabled Pads
          if(gpio_number % 2 == 0)
          {  
            if(j % 2 != 0)
            {
              check_for_din_and_intr(j);
            }
          }
          else {
            if(j % 2 == 0)
            {
              check_for_din_and_intr(j);
            }
          }
        }
      }     
 
      // Enable CPU interrupt
      int_pend = 1;
      enable_cpu_intr(i);
      while(int_pend) {
        wait_on(10);
      }
      
      sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0);
      wait_on(500);
      for(j = 0; j < 32; j++) {
        if(gpio_number % 2 == 0)
        {  
          if(j % 2 != 0)
          {
            check_for_din_and_intr(j);
          }
        }
        else {
          if(j % 2 == 0)
          {
            check_for_din_and_intr(j);
          }
        }
      }
      
      // Enable CPU interrupt
      int_pend = 1;
      enable_cpu_intr(i);
 
      sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,1 << i);
      wait_on(20);
      while(int_pend) {
        wait_on(10);
      }
      
      sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0);
      wait_on(500);
      for(j = 0; j < 32; j++) {
        if(gpio_number % 2 == 0)
        {  
          if(j % 2 != 0)
          {
            check_for_din_and_intr(j);
          }
        }
        else {
          if(j % 2 == 0)
          {
            check_for_din_and_intr(j);
          }
        }
      }
    }
    else {
      // Write DOUT for Group1 GPIOs (0:7)  
      for(j = 0;j < 5;j++)
      { 
        // Write GPIO Group1 register dout bit as '1' 
        sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_DOUT_GROUP1, 1 << i);
        flag_din_one = 1;
        for(k = 0; k < 32; k++) {
          if(gpio_number % 2 == 0)
          {  
            if(k % 2 != 0)
            {
              check_for_din_value(k);
              check_for_io_ctrl_bit(k);
            }
          }
          else {
            if(k % 2 == 0)
            {
              check_for_din_value(k);
              check_for_io_ctrl_bit(k);
            }
          }
        }
        
        // Write GPIO Group1 register dout bit as '0' 
        sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_DOUT_GROUP1,0x0);
        flag_din_zero = 1;
        for(k = 0; k < 32; k++) {
          if(gpio_number % 2 == 0)
          {  
            if(k % 2 != 0)
            {
              check_for_din_value(k);
              check_for_io_ctrl_bit(k);
            }
          }
          else {
            if(k % 2 == 0)
            {
              check_for_din_value(k);
              check_for_io_ctrl_bit(k);
            }
          }
        }
      }
    }
  }
 
  // Writing the scratch pad register for GPIO pads 8-39 
  for(i = 6;i < 32; i++) { 

    printf("Toggling the GPIO pins by writing into scratch register_2..\n");

    gpio_number = i;
    #ifdef MIZAR_40_PIN_PKG

if(i>=16 && i <=23){

printf("ignored this pad::GPIO_NUM  = %0d \n",i );

continue;
}

else if (i>=26 && i<=32){



printf("ignored this pad::GPIO_NUM = %0d \n",i);

continue;


}

#endif

    if((i % 2) == 0) 
    {
      // Enable CPU interrupt
      int_pend = 1;
      enable_cpu_intr(i);
 
      sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,1 << (i - 6));
      wait_on(20);
      while(int_pend) {
        wait_on(10);
      }
      
      sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x0);
      wait_on(500);
      for(j = 0; j < 32; j++) {
        if(gpio_number % 2 == 0)
        {  
          if(j % 2 != 0)
          {
            check_for_din_and_intr(j);
          }
        }
        else {
          if(j % 2 == 0)
          {
            check_for_din_and_intr(j);
          }
        }
      }

      sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,1 << (i - 6));
      wait_on(20);
    
      for(j = 6; j < 32; j++) {
    
        if(gpio_number == j) 
        {
          switch(j)
          { 
            case 6:
            case 8:
            case 10:
            case 12:
            rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP1);

            // Check DIN value for Group1 GPIOs (8:15)  
            if((rdata & (1 << (j - 6))) != 0)
            {
              printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP1_GP0:: DIN value matches with the Pad_value\n",j);
            }
            else 
            {
              printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP1_GP0:: DIN value does not match with the Pad_value\n",j);
              flag++; 
            }
            break;
            
            case 14:
            case 16:
            case 18:
            case 20:
            rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP2);

            // Check DIN value for Group2 GPIOs (16:23)  
            if((rdata & (1 << (j - 14))) != 0)
            {
              printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP2_GP0:: DIN value matches with the Pad_value\n",j);
            }
            else 
            {
              printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP2_GP0:: DIN value does not match with the Pad_value\n",j);
              flag++; 
            }
            break;
           
            case 22:
            case 24:
            case 26:
            case 28:
            rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP3);

            // Check DIN value for Group3 GPIOs (24:31)  
            if((rdata & (1 << (j  - 22))) != 0)
            {
              printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP3_GP0:: DIN value matches with the Pad_value\n",j);
            }
            else 
            {
              printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP3_GP0:: DIN value does not match with the Pad_value\n",j);
              flag++; 
            }
            break;
            
            case 30:
          //  case 32:
          //  case 34:
          //  case 36:
            rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP4);

            // Check DIN value for Group4 GPIOs (32:39)  
            if((rdata & (1 << (j - 30))) != 0)
            {
              printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP4_GP0:: DIN value matches with the Pad_value\n",j);
            }
            else 
            {
              printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY_GROUP4_GP0:: DIN value does not match with the Pad_value\n",j);
              flag++; 
            }
            break;
            
            default:
            printf("INVALID_GPIO_NUMBER");
            break;
          } 

          // Reading the DIN and interrupt raw status values from GPIO Register
          rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((j - 6) * 4));

          // Check for DIN value for active/enabled Pad when CPU interrupt is disabled
          if((rdata & 0x1) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: DIN value matches with the Pad_value\n",j);
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: DIN value does not match with the Pad_value\n",j);
            flag++; 
          }
          
          // Check for interrupt raw status bit for active/enabled Pad when CPU interrupt is disabled
          if((rdata & 0x2) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: Raw Interrupt raised at posedge\n",j);
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: Raw Interrupt not raised\n",j);
            flag++;
          }
          
          //Check for CPU raw interrupt status
          rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
          
          if((rdata & (1 << (j - 6))) != 0)      
          {
            printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: CPU Raw Interrupt raised\n",j);
          }
          else
          {
            printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: CPU Raw Interrupt not raised\n",j);
            flag++;
          }
          
          // Check for processor interrupt when the CPU interrupt is disabled 
          #ifdef PACMAN_PROC 
            rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
            
            if((rdata & (1 << (j - 6))) == 0)
            {
              printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: Interrupt not raised at processor\n",j);
            }
            else 
            {
              printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: Interrupt raised at processor\n",j);
              flag++;
            }
          #else
            rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
            
            if((rdata & (1 << (j - 6))) == 0)
            {
              printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: Interrupt not raised at processor\n",j);
            }
            else 
            {
              printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: Interrupt raised at processor\n",j);
              flag++;
            }
          #endif
        }
        else {
          // Check for DIN, Interrupt raw status and CPU interrupt status for disabled Pads
          if(gpio_number % 2 == 0)
          {  
            if(j % 2 != 0)
            {
              check_for_din_and_intr(j);
            }
          }
          else {
            if(j % 2 == 0)
            {
              check_for_din_and_intr(j);
            }
          }
        }
      }     
 
      // Enable CPU interrupt
      int_pend = 1;
      enable_cpu_intr(i);
      while(int_pend) {
        wait_on(10);
      }
      
      sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x0);
      wait_on(500);
      for(j = 0; j < 32; j++) {
        if(gpio_number % 2 == 0)
        {  
          if(j % 2 != 0)
          {
            check_for_din_and_intr(j);
          }
        }
        else {
          if(j % 2 == 0)
          {
            check_for_din_and_intr(j);
          }
        }
      }
      
      // Enable CPU interrupt
      int_pend = 1;
      enable_cpu_intr(i);
 
      sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,1 << (i - 6));
      wait_on(20);
      while(int_pend) {
        wait_on(10);
      }
      
      sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x0);
      wait_on(500);
      for(j = 0; j < 32; j++) {
        if(gpio_number % 2 == 0)
        {  
          if(j % 2 != 0)
          {
            check_for_din_and_intr(j);
          }
        }
        else {
          if(j % 2 == 0)
          {
            check_for_din_and_intr(j);
          }
        }
      }
    }
    else {
      // Write DOUT for Group1 GPIOs (8:15)  
      if(i >= 6 && i < 14)
      {
        for(j = 0;j < 5;j++)
        { 
          // Write GPIO Group1 register dout bit as '1' 
          wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP1, 1 << (i - 6));
          flag_din_one = 1;
          for(k = 0; k < 32; k++) {
            if(gpio_number % 2 == 0)
            {  
              if(k % 2 != 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
            else {
              if(k % 2 == 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
          }

          // Write GPIO Group1 register dout bit as '0' 
          wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP1,0x0);
          flag_din_zero = 1;
          for(k = 0; k < 32; k++) {
            if(gpio_number % 2 == 0)
            {  
              if(k % 2 != 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
            else {
              if(k % 2 == 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
          }
        }
      }

      // Write DOUT for Group2 GPIOs (16:23)  
      if(i >= 14 && i < 22) 
      { 
        for(j = 0;j < 5;j++)
        { 
          // Write GPIO Group2 register dout bit as '1' 
          wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP2, 1 << (i - 14));
          flag_din_one = 1;
          for(k = 0; k < 32; k++) {
            if(gpio_number % 2 == 0)
            {  
              if(k % 2 != 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
            else {
              if(k % 2 == 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
          }
          
          // Write GPIO Group2 register dout bit as '0' 
          wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP2,0x0);
          flag_din_zero = 1;
          for(k = 0; k < 32; k++) {
            if(gpio_number % 2 == 0)
            {  
              if(k % 2 != 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
            else {
              if(k % 2 == 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
          }
        }
      }
   
      // Write DOUT for Group3 GPIOs (24:31)  
      if(i >= 22 && i < 30) 
      { 
        for(j = 0;j < 5;j++)
        { 
          // Write GPIO Group3 register dout bit as '1' 
          wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP3, 1 << (i - 22));
          flag_din_one = 1;
          for(k = 0; k < 32; k++) {
            if(gpio_number % 2 == 0)
            {  
              if(k % 2 != 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
            else {
              if(k % 2 == 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
          }

          // Write GPIO Group3 register dout bit as '0' 
          wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP3,0x0);
          flag_din_zero = 1;
          for(k = 0; k < 32; k++) {
            if(gpio_number % 2 == 0)
            {  
              if(k % 2 != 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
            else {
              if(k % 2 == 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
          }
        }
      }
      
      // Write DOUT for Group4 GPIOs (32:39)  
      if(i >= 30 && i < 32)
      { 
        for(j = 0;j < 5;j++)
        { 
          // Write GPIO Group4 register dout bit as '1' 
          wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP4, 1 << (i - 30));
          flag_din_one = 1;
          for(k = 0; k < 32; k++) {
            if(gpio_number % 2 == 0)
            {  
              if(k % 2 != 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
            else {
              if(k % 2 == 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
          }
          
          // Write GPIO Group4 register dout bit as '0' 
          wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP4,0x0);
          flag_din_zero = 1;
          for(k = 0; k < 32; k++) {
            if(gpio_number % 2 == 0)
            {  
              if(k % 2 != 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
            else {
              if(k % 2 == 0)
              {
                check_for_din_value(k);
                check_for_io_ctrl_bit(k);
              }
            }
          }
        }
      }
    }
  }
  
  
  finish(flag); 
}

// Processor selection (CA32/PACMAN) and enabling the corresponding interrupt bit
void enable_cpu_intr(unsigned int gpio_number) {

  if(gpio_number >= 0 && gpio_number < 6) 
  {
    #ifdef PACMAN_PROC 
      sec_wr(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_EN1,1 << gpio_number);
    #else
      sec_wr(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_EN1,1 << gpio_number);
    #endif
  }
  else  
  {
    #ifdef PACMAN_PROC 
      wr(MIZAR_GP0_GPIO_GP0_INTR2_INTR_EN1,1 << (gpio_number - 6));
    #else
      wr(MIZAR_GP0_GPIO_GP0_INTR1_INTR_EN1,1 << (gpio_number - 6));
    #endif
  }
} 

// Clearing the Processor interrupt bit 
void disable_cpu_intr() {
  
  if(gpio_number >= 0 && gpio_number < 6) 
  {
    #ifdef PACMAN_PROC 
      sec_wr(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_EN1,0);
    #else
      sec_wr(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_EN1,0);
    #endif
  }
  else  
  {
    #ifdef PACMAN_PROC 
      wr(MIZAR_GP0_GPIO_GP0_INTR2_INTR_EN1,0);
    #else
      wr(MIZAR_GP0_GPIO_GP0_INTR1_INTR_EN1,0);
    #endif
  }
}

// Read the DIN value from Group registers for Output mode GPIO pads 
void check_for_din_value(unsigned int gpio_pad_num) {

int rdata;

  // Check DIN value for GPIOs Pads 0-7 
  if(gpio_pad_num >= 0 && gpio_pad_num < 4) 
  { 
    // Check for DIN value for Group1 GPIOs (0:7)
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);
    
    // Check DIN for current active GPIO Pad 
    if(gpio_number == gpio_pad_num) 
    {  
      if(flag_din_one == 1)
      {
        if((rdata & (1 << gpio_pad_num)) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP1_GP3:: DIN value matches with the Pad_value\n",gpio_pad_num);
          flag_din_one = 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP1_GP3:: DIN value does not match with the Pad_value\n",gpio_pad_num);
          flag++; 
        }
      }
      
      if(flag_din_zero == 1)
      {
        if((rdata & (1 << gpio_pad_num)) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP1_GP3:: DIN value matches with the Pad_value\n",gpio_pad_num);
          flag_din_zero= 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP1_GP3:: DIN value does not match with the Pad_value\n",gpio_pad_num);
          flag++; 
        }
      }
    }
    else {
      // Check DIN for remaining pads except the current Pad 
      if((rdata & (1 << gpio_pad_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP1_GP3:: DIN value matches with the Pad_value\n",gpio_pad_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP1_GP3:: DIN value does not match with the Pad_value\n",gpio_pad_num);
        flag++; 
      }
    }
  }
 
  // Check DIN value for GPIOs Pads 8-39
  if(gpio_pad_num >= 6 && gpio_pad_num < 32)
  { 
    switch(gpio_pad_num)
    {
      // Check for DIN value for Group1 GPIOs (8:15)
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP1);
      
      // Check DIN for current active GPIO Pad 
      if(gpio_number == gpio_pad_num) 
      {  
        if(flag_din_one == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 6))) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP1_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_one = 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP1_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
        
        if(flag_din_zero == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 6))) == 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP1_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_zero= 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP1_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
      }
      else {
        // Check DIN for remaining pads except the current Pad 
        if((rdata & (1 << (gpio_pad_num - 6))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP1_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP1_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
          flag++; 
        }
      }
      break;
    
      // Check for DIN value for Group2 GPIOs (16:23)
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP2);
      
      // Check DIN for current active GPIO Pad 
      if(gpio_number == gpio_pad_num) 
      {  
        if(flag_din_one == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 14))) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP2_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_one = 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP2_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
        
        if(flag_din_zero == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 14))) == 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP2_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_zero= 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP2_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
      }
      else {
        // Check DIN for remaining pads except the current Pad 
        if((rdata & (1 << (gpio_pad_num - 14))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP2_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP2_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
          flag++; 
        }
      }
      break;
    
      // Check for DIN value for Group3 GPIOs (24:31)
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP3);
     
      // Check DIN for current active GPIO Pad 
      if(gpio_number == gpio_pad_num) 
      {  
        if(flag_din_one == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 22))) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP3_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_one = 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP3_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
        
        if(flag_din_zero == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 22))) == 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP3_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_zero= 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP3_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
      }
      else {
        // Check DIN for remaining pads except the current Pad 
        if((rdata & (1 << (gpio_pad_num - 22))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP3_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP3_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
          flag++; 
        }
      }
      break;
  
      // Check for DIN value for Group4 GPIOs (32:39)
      case 30:
      case 31:
     // case 32:
     // case 33:
     // case 34:
     // case 35:
     // case 36:
     // case 37:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP4);
      
      // Check DIN for current active GPIO Pad 
      if(gpio_number == gpio_pad_num) 
      {  
        if(flag_din_one == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 30))) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP4_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_one = 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP4_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
        
        if(flag_din_zero == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 30))) == 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP4_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_zero= 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP4_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
      }
      else {
        // Check DIN for remaining pads except the current Pad 
        if((rdata & (1 << (gpio_pad_num - 30))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP4_GP0:: DIN value matches with the Pad_value\n",gpio_pad_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP4_GP0:: DIN value does not match with the Pad_value\n",gpio_pad_num);
          flag++; 
        }
      }
      break;

      default: 
      printf("INVALID_GPIO_NUMBER");
      break;
    }
  }
}

// Check for IO CTRL status by reading the GPIO register 20th field for the Pads which have IO_mask_value as '1' 
void check_for_io_ctrl_bit(unsigned int gpio_num) {

int rdata;

  // Reading the IO CTRL field (20th bit) from GPIO Register for GPIO Pads 0-7 
  if(gpio_num >= 0 && gpio_num < 4)
  {
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
    
    if(gpio_num % 2 != 0)
    {
      if((rdata & 0x100000) == 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_IO_CTRL_BIT_GP3:: Disabled pad is in Output mode\n",gpio_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_IO_CTRL_BIT_GP3:: Disabled pad is not in Output mode\n",gpio_num);
        flag++;
      }
    }
    else 
    {
      if((rdata & 0x100000) != 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_IO_CTRL_BIT_GP3:: Enabled pad is in Input mode\n",gpio_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_IO_CTRL_BIT_GP3:: Enabled pad is not in Input mode\n",gpio_num);
        flag++;
      }
    }
  }

  // Reading the IO CTRL field (20th bit) from GPIO Register for GPIO Pads 8-39 
  if(gpio_num >= 6 && gpio_num < 32)
  {
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((gpio_num - 6) * 4));
    
    if(gpio_num % 2 != 0)
    {
      if((rdata & 0x100000) == 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_IO_CTRL_BIT_GP0:: Disabled pad is in Output mode\n",gpio_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_IO_CTRL_BIT_GP0:: Disabled pad is not in Output mode\n",gpio_num);
        flag++;
      }
    }
    else 
    {
      if((rdata & 0x100000) != 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_IO_CTRL_BIT_GP0:: Enabled pad is in Input mode\n",gpio_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_IO_CTRL_BIT_GP0:: Enabled pad is not in Input mode\n",gpio_num);
        flag++;
      }
    }
  }
}

// Check for DIN, Interrupt raw status,CPU interrupt status and IO_CTRL when Pad value changes from '1' to '0' (Negedge)
void check_for_din_and_intr(unsigned int gpio_num) {

int rdata;

  // Check DIN,raw interrupt and IO_CTRL fields for GPIOs Pads 0-7 
  if(gpio_num >= 0 && gpio_num < 4)
  { 
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);
   
    if((rdata & (1 << gpio_num)) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP1_GP3:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP1_GP3:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
  
    // Reading DIN, Interrupt raw status and IO_CTRL values from GPIO Register for disabled Pads
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
   
    // Check for DIN value
    if((rdata & 0x1) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
   
    // Check the interrupt raw status for disabled Pads 
    if(gpio_num % 2 == 0)
    { 
      if((rdata & 0x2) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Raw Interrupt not raised for disabled Pad\n",gpio_num);
      }
      else  
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Raw Interrupt raised for disabled Pad\n",gpio_num);
        flag++; 
      }
    }
 
    // Check the IO CTRL field (20th bit) from GPIO Register 
    if(gpio_num % 2 == 0)
    { 
      // Check the IO CTRL field (20th bit) from GPIO Register for active pads 
      if((rdata & 0x100000) != 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_FOR_DIN_AND_INTR:: GPIO Pad is in Input mode when pad_value is '0'\n",gpio_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_FOR_DIN_AND_INTR:: GPIO pad is not in Input mode when pad_value is '0'\n",gpio_num);
        flag++;
      }
    }
    else {
      // Check the IO CTRL field (20th bit) from GPIO Register for inactive pads 
      if((rdata & 0x100000) == 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_FOR_DIN_AND_INTR:: GPIO disabled Pad is in Output mode\n",gpio_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_FOR_DIN_AND_INTR:: GPIO disabled pad is not in Output mode\n",gpio_num);
        flag++;
      }
    }
        
    // Check for CPU raw interrupt status
    if(gpio_num % 2 == 0)
    { 
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
      
      if((rdata & (1 << gpio_num)) == 0)      
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: CPU Raw Interrupt not raised for disabled\n",gpio_num);
      }
      else
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: CPU Raw Interrupt raised\n",gpio_num);
        flag++;
      }

      // Check for CPU interrupt during negedge for inactive Pads 
      #ifdef PACMAN_PROC 
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
        
        if((rdata & (1 << gpio_num)) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Interrupt not raised at processor for disabled pad\n",gpio_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Interrupt raised at processor\n",gpio_num);
          flag++;
        }
      #else
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
       
        if((rdata & (1 << gpio_num)) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Interrupt not raised at processor for disabled pad\n",gpio_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Interrupt raised at processor\n",gpio_num);
          flag++;
        }
      #endif
    }
  }

  // Check DIN,raw interrupt and IO_CTRL fields for GPIOs Pads 8-39
  if(gpio_num >= 6 && gpio_num < 32)
  { 
    switch(gpio_num)
    {
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP1);
   
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP1:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP1:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;
 
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP2);
   
      if((rdata & (1 << (gpio_num - 14))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP2:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP2:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;

      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP3);
   
      if((rdata & (1 << (gpio_num - 22))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP3:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP3:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;

      case 30:
      case 31:
   //   case 32:
   //   case 33:
   //   case 34:
   //   case 35:
   //   case 36:
   //   case 37:
   //   case 38:
   //   case 39:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP4);
   
      if((rdata & (1 << (gpio_num - 30))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP4:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR_GROUP4:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;
      
      default:
      printf("INVALID_GPIO_NUMBER");
      break;
    }

    // Reading DIN, Interrupt raw status and IO_CTRL values from GPIO Register for disabled Pads
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((gpio_num - 6) * 4));
 
    // Check for DIN value
    if((rdata & 0x1) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
 
    // Check the interrupt raw status for disabled Pads 
    if(gpio_num % 2 == 0)
    { 
      if((rdata & 0x2) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Raw Interrupt not raised for disabled Pad\n",gpio_num);
      }
      else  
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Raw Interrupt raised for disabled Pad\n",gpio_num);
        flag++; 
      }
    }
 
    // Check the IO CTRL field (20th bit) from GPIO Register 
    if(gpio_num % 2 == 0)
    { 
      // Check the IO CTRL field (20th bit) from GPIO Register for active pads 
      if((rdata & 0x100000) != 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_FOR_DIN_AND_INTR:: GPIO Pad is in Input mode when pad_value is '0'\n",gpio_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_FOR_DIN_AND_INTR:: GPIO pad is not in Input mode when pad_value is '0'\n",gpio_num);
        flag++;
      }
    }
    else {
      // Check the IO CTRL field (20th bit) from GPIO Register for inactive pads 
      if((rdata & 0x100000) == 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_FOR_DIN_AND_INTR:: GPIO disabled Pad is in Output mode\n",gpio_num);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_FOR_DIN_AND_INTR:: GPIO disabled pad is not in Output mode\n",gpio_num);
        flag++;
      }
    }

    if(gpio_num % 2 == 0)
    { 
      // Check for CPU raw interrupt status
      rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
      
      if((rdata & (1 << (gpio_num - 6))) == 0)      
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: CPU Raw Interrupt not raised for disabled\n",gpio_num);
      }
      else
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: CPU Raw Interrupt raised\n",gpio_num);
        flag++;
      }

      // Check for CPU interrupt during negedge for inactive Pads 
      #ifdef PACMAN_PROC 
        rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
        
        if((rdata & (1 << (gpio_num - 6))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Interrupt not raised at processor for disabled pad\n",gpio_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Interrupt raised at processor\n",gpio_num);
          flag++;
        }
      #else
        rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
       
        if((rdata & (1 << (gpio_num - 6))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Interrupt not raised at processor for disabled pad\n",gpio_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_AND_INTR:: Interrupt raised at processor\n",gpio_num);
          flag++;
        }
      #endif
    }
  }
}


void Default_IRQHandler() {

int j,rdata;

  int_pend = 0;

  printf("Entered into default IRQ Handler!!\n");

  // Check for DIN, raw interrupt and IO_CTRL fields for GPIO Pads 0-7  
  for(j = 0;j < 4;j++)
  {
    // Check DIN value during posedge for Group1 GPIOs (0:7)  
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);
    
    if(gpio_number == j)
    {
      if((rdata & (1 << j)) != 0)
      {
        printf("SUCCESS: Default_IRQHandler_GROUP1_GP3:: DIN value matches with the Pad_value\n");
      }
      else 
      {
        printf("ERROR: Default_IRQHandler_GROUP1_GP3:: DIN value does not match with the Pad_value\n");
        flag++; 
      }
  
      // Reading DIN, Interrupt raw status and IO_CTRL values from GPIO Register for disabled Pads
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4));
   
      // Check for DIN value
      if((rdata & 0x1) != 0)
      {
        printf("SUCCESS: Default_IRQHandler:: DIN value matches with the Pad_value\n");
      }
      else 
      {
        printf("ERROR: Default_IRQHandler:: DIN value does not match with the Pad_value\n");
        flag++; 
      }
   
      // Check the interrupt raw status for disabled Pads 
      if((rdata & 0x2) != 0)
      {
        printf("SUCCESS: Default_IRQHandler:: Raw Interrupt not raised for disabled Pad\n");
      }
      else  
      {
        printf("ERROR: Default_IRQHandler:: Raw Interrupt raised for disabled Pad\n");
        flag++; 
      }
      
      // Check the IO CTRL field (20th bit) from GPIO Register 
      if((rdata & 0x100000) != 0) 
      {
        printf("SUCCESS: Default_IRQHandler:: GPIO pad is in Input mode for disabled pad\n");
      }
      else {
        printf("ERROR: Default_IRQHandler:: GPIO pad is not in Input mode for disabled\n");
        flag++;
      }
      
      // Check for CPU raw interrupt status
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
      
      if((rdata & (1 << j)) != 0)      
      {
        printf("SUCCESS: Default_IRQHandler:: CPU Raw Interrupt not raised for disabled\n");
      }
      else
      {
        printf("ERROR: Default_IRQHandler:: CPU Raw Interrupt raised\n");
        flag++;
      }

      // Check for CPU interrupt during negedge for inactive Pads 
      #ifdef PACMAN_PROC 
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
        
        if((rdata & (1 << j)) != 0)
        {
          printf("SUCCESS: Default_IRQHandler:: Interrupt not raised at processor for disabled pad\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler:: Interrupt raised at processor\n");
          flag++;
        }
      #else
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
       
        if((rdata & (1 << j)) != 0)
        {
          printf("SUCCESS: Default_IRQHandler:: Interrupt not raised at processor for disabled pad\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler:: Interrupt raised at processor\n");
          flag++;
        }
      #endif
    }
    else {
      // Check for DIN, Interrupt raw status and CPU interrupt status for disabled Pads
      if(gpio_number % 2 == 0)
      {  
        if(j % 2 != 0)
        {
          check_for_din_and_intr(j);
        }
      }
      else {
        if(j % 2 == 0)
        {
          check_for_din_and_intr(j);
        }
      }
    }
  
    // Clearing the interrupt raw status bit (16th bit of GPIO reg set to '1') 
    sec_rmw(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4),0x00010000,0x00010000);
  
    // Clearing the CPU raw interrupt status bit 
    sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1,0x0);
  }

  // Check for DIN, raw interrupt and IO_CTRL fields for GPIO Pads 8-39  
  for(j = 6;j < 32;j++) 
  { 
    if(gpio_number == j)  
    { 
      switch(j)
      { 
        case 6:
        case 8:
        case 10:
        case 12:
        rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP1);

        // Check DIN value during posedge for Group1 GPIOs (8:15)  
        if((rdata & (1 << (j - 6))) != 0)
        {
          printf("SUCCESS: Default_IRQHandler_GROUP1_GP0:: DIN value matches with the Pad_value\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler_GROUP1_GP0:: DIN value does not match with the Pad_value\n");
          flag++; 
        }
        break;

        case 14:
        case 16:
        case 18:
        case 20:
        rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP2);

        // Check DIN value during posedge for Group2 GPIOs (16:23)  
        if((rdata & (1 << (j - 14))) != 0)
        {
          printf("SUCCESS: Default_IRQHandler_GROUP2_GP0:: DIN value matches with the Pad_value\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler_GROUP2_GP0:: DIN value does not match with the Pad_value\n");
          flag++; 
        }
        break;
        
        case 22:
        case 24:
        case 26:
        case 28:
        rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP3);

        // Check DIN value during posedge for Group3 GPIOs (24:31)  
        if((rdata & (1 << (j - 22))) != 0)
        {
          printf("SUCCESS: Default_IRQHandler_GROUP3_GP0:: DIN value matches with the Pad_value\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler_GROUP3_GP0:: DIN value does not match with the Pad_value\n");
          flag++; 
        }
        break;
        
        case 30:
     //   case 32:
     //   case 34:
     //   case 36:
     //   case 38:
        rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP4);

        // Check DIN value during posedge for Group4 GPIOs (32:39)  
        if((rdata & (1 << (j - 30))) != 0)
        {
          printf("SUCCESS: Default_IRQHandler_GROUP4_GP0:: DIN value matches with the Pad_value\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler_GROUP4_GP0:: DIN value does not match with the Pad_value\n");
          flag++; 
        }
        break;

        default:
        printf("INVALID_GPIO_NUMBER");
        break;
      } 
      
      // Reading DIN,interrupt raw status and IO_CTRL values from GPIO Register
      rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((j - 6) * 4));
      
      // Check for DIN value
      if((rdata & 0x1) != 0)
      {
        printf("SUCCESS: Default_IRQHandler:: DIN value matches with the Pad_value\n");
      }
      else 
      {
        printf("ERROR: Default_IRQHandler:: DIN value does not match with the Pad_value\n");
        flag++; 
      }
  
      // Check for interrupt raw status bit during posedge for active/enabled Pad
      if((rdata & 0x2) != 0)
      {
        printf("SUCCESS: Default_IRQHandler:: Raw Interrupt raised at posedge\n");
      }
      else 
      {
        printf("ERROR: Default_IRQHandler:: Raw Interrupt not raised\n");
        flag++;
      }
     
      // Check the IO CTRL field (20th bit) from GPIO Register 
      if((rdata & 0x100000) != 0) 
      {
        printf("SUCCESS: Default_IRQHandler:: Current GPIO pad is in Input mode\n");
      }
      else {
        printf("ERROR: Default_IRQHandler:: Current GPIO pad is not in Input mode\n");
        flag++;
      }

      //Check for CPU raw interrupt status
      rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
      
      if((rdata & (1 << (j - 6))) != 0)      
      {
        printf("SUCCESS: Default_IRQHandler:: CPU Raw Interrupt raised\n");
      }
      else
      {
        printf("ERROR: Default_IRQHandler:: CPU Raw Interrupt not raised\n");
        flag++;
      }

      // Check for CPU interrupt during posedge for active Pad 
      #ifdef PACMAN_PROC 
        rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
        
        if((rdata & (1 << (j - 6))) != 0)
        {
          printf("SUCCESS: Default_IRQHandler:: Interrupt raised at processor\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler:: Interrupt not raised at processor\n");
          flag++;
        }
      #else
        rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
        
        if((rdata & (1 << (j - 6))) != 0)
        {
          printf("SUCCESS: Default_IRQHandler:: Interrupt raised at processor\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler:: Interrupt not raised at processor\n");
          flag++;
        }
      #endif
    }
    else {
      // Check for DIN, Interrupt raw status and CPU interrupt status for disabled Pads
      if(gpio_number % 2 == 0)
      {  
        if(j % 2 != 0)
        {
          check_for_din_and_intr(j);
        }
      }
      else {
        if(j % 2 == 0)
        {
          check_for_din_and_intr(j);
        }
      }
    }
  
    // Clearing the interrupt raw status bit (16th bit of GPIO reg set to '1') 
    rmw(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((j - 6) * 4),0x00010000,0x00010000);
  
    // Clearing the CPU raw interrupt status bit 
    wr(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1,0x0);
  }
  
  // Clearing the Processor interrupt bit 
  disable_cpu_intr();
  
  return;
} 

