#include<stdio.h>
#include<testdef.h>

int gpio_number,flag;
extern int int_pend;

void test_case() {
  
int i,j,rdata;

  flag = 0;

  pinmux_for_gpio_func();

  // For enabling negedge interrupt for GPIOs 0-7  
  for(i = 0; i < 4; i++) {
    
    printf("Enabling GPIO_3 register fields..\n");
    
    // Enabling negedge interrupt (18th bit as '1')
    sec_rmw(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00040000,0x00040000);

    // Clearing the raw status interrupt bit (Set 16th bit to '1') 
    sec_rmw(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00010000,0x00010000);
  }
  
  // For enabling negedge interrupt for GPIOs 8-39  
  for(i = 0; i < 26; i++) {
    
    printf("Enabling GPIO register fields..\n");
    
    // Enabling negedge interrupt (18th bit as '1')
    rmw(MIZAR_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00040000,0x00040000); 
    
    // Clearing the raw status interrupt bit (Set 16th bit as '1') 
    rmw(MIZAR_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00010000,0x00010000);
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

  // Enabling all 8 GPIOs per virtual register([7:0] and [23:16] set to 'FF') for GPIO Pads 0-7
  sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_IO_CTRL_GROUP1,0x00FF00FF);

  // Enabling all 8 GPIOs per virtual register([7:0] and [23:16] set to 'FF') for GPIO Pads 8-39
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP1,0x00FF00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP2,0x00FF00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP3,0x00FF00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP4,0x00FF00FF);
 
  // Initializing all the GPIO pins with the value as '1' by writing into scratch register for GPIO Pads 0-7
  sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0F);
 
  // Initializing all the GPIO pins with the value as '1' by writing into scratch register for GPIO Pads 8-39 
  sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x03FFFFFF);
  

  // Writing scratch pad register for GPIOs 0-7 
  for(i = 0;i < 4;i++) {
    
    printf("Toggling the GPIO pins by writing into scratch register_1..\n");
   
    gpio_number = i;
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i); 
  #ifdef MIZAR_40_PIN_PKG

    if(i == 2){

printf("ignored this pad = %0d \n",i);

         continue;
} 

#endif  
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0F & ~(1 << i));
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
 
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0F);
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
      
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0F & ~(1 << i));
    wait_on(20);
    
    for(j = 0; j < 4; j++) {
    
      if(gpio_number == j) 
      {
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);

        // Check DIN value for Group1 GPIOs (0:7)  
        if((rdata & (1 << j)) == 0)
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
        if((rdata & 0x1) == 0)
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
          printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: Raw Interrupt raised at negedge\n",j);
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
    
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0F);
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
    
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0F & ~(1 << i));
    wait_on(20);
    while(int_pend) {
      wait_on(10);
    }
    
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0F);
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

 


 // Writing scratch pad register for GPIOs 8-39 
  for(i = 6; i < 32; i++) {
    
    printf("Toggling the GPIO pins by writing into scratch register_2..\n");
   
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

    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i); 
       sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x03FFFFFF & ~(1 << (i - 6)));
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
 
    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x03FFFFFF);
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
     
    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x03FFFFFF & ~(1 << (i - 6)));
    wait_on(20);
    
    for(j = 6; j < 32; j++) {
    
      if(gpio_number == j) 
      {
        switch(j)
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

          // Check DIN value for Group1 GPIOs (8:15)  
          if((rdata & (1 << (j - 6))) == 0)
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
          case 15:
          case 16:
          case 17:
          case 18:
          case 19:
          case 20:
          case 21:
          rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP2);

          // Check DIN value for Group2 GPIOs (16:23)  
          if((rdata & (1 << (j - 14))) == 0)
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
          case 23:
          case 24:
          case 25:
          case 26:
          case 27:
          case 28:
          case 29:
          rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP3);

          // Check DIN value for Group3 GPIOs (24:31)  
          if((rdata & (1 << (j  - 22))) == 0)
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
          case 31:
          //case 32:
          //case 33:
          //case 34:
          //case 35:
          //case 36:
          //case 37:
          //case 38:
          //case 39:
          rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP4);

          // Check DIN value for Group4 GPIOs (32:39)  
          if((rdata & (1 << (j - 30))) == 0)
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
        if((rdata & 0x1) == 0)
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
          printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: Raw Interrupt raised at negedge\n",j);
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
    
    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x03FFFFFF);
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
    
    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x03FFFFFF & ~(1 << (i - 6)));
    wait_on(20);
    while(int_pend) {
      wait_on(10);
    }
    
    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x03FFFFFF);
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
 
  finish(flag);
}

// Processor selection (CA32/PACMAN) and enabling the corresponding interrupt bit for GPIOs 0-39
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

// Clearing the Processor interrupt bit for GPIOs 0-39
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

// Check for DIN, Interrupt raw status,CPU interrupt status adn IO_CTRL when Pad value changes from '0' to '1' (Posedge)
void check_for_din_and_intr(unsigned int gpio_num) {

int rdata;
#ifdef MIZAR_40_PIN_PKG

 // Check DIN,raw interrupt and IO_CTRL fields for GPIOs Pads 0-7 
  if(gpio_num >= 3 && gpio_num < 4)
  { 
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);
   
    if((rdata & (1 << gpio_num)) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP3:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP3:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
  
    // Reading DIN, Interrupt raw status and IO_CTRL values from GPIO Register for disabled Pads
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
   
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
   
    // Check the interrupt raw status for disabled Pads 
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt not raised for disabled Pad\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt raised for disabled Pad\n",gpio_num);
      flag++; 
    }
 
    // Check the IO CTRL field (20th bit) from GPIO Register 
    if((rdata & 0x100000) != 0) 
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO Pad is in Input mode when pad_value is '0'\n",gpio_num);
    }
    else {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO pad is not in Input mode when pad_value is '0'\n",gpio_num);
      flag++;
    }
            
    // Check for CPU raw interrupt status
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << gpio_num)) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt not raised for disabled\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }

else if(gpio_num >= 0 && gpio_num < 2)
{ 
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);
   
    if((rdata & (1 << gpio_num)) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP3:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP3:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
  
    // Reading DIN, Interrupt raw status and IO_CTRL values from GPIO Register for disabled Pads
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
   
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
   
    // Check the interrupt raw status for disabled Pads 
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt not raised for disabled Pad\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt raised for disabled Pad\n",gpio_num);
      flag++; 
    }
 
    // Check the IO CTRL field (20th bit) from GPIO Register 
    if((rdata & 0x100000) != 0) 
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO Pad is in Input mode when pad_value is '0'\n",gpio_num);
    }
    else {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO pad is not in Input mode when pad_value is '0'\n",gpio_num);
      flag++;
    }
            
    // Check for CPU raw interrupt status
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << gpio_num)) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt not raised for disabled\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }

  // Check DIN,raw interrupt and IO_CTRL fields for GPIOs Pads 8-39
  if(gpio_num >= 6 && gpio_num < 16)
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
   
      if((rdata & (1 << (gpio_num - 6))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;
 
      case 14:
      case 15:
      case 16:
            rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP2);
   
      if((rdata & (1 << (gpio_num - 14))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP2_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP2_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;

      case 23:
      case 24:
      //case 25:
           rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP3);
   
      if((rdata & (1 << (gpio_num - 22))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP3_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP3_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;

            
      default:
      printf("INVALID_GPIO_NUMBER");
      break;
    }
}
else if (gpio_num == 24 )
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
   
      if((rdata & (1 << (gpio_num - 6))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;
 
      case 14:
      case 15:
      case 16:
            rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP2);
   
      if((rdata & (1 << (gpio_num - 14))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP2_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP2_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;

      case 23:
      case 24:
      //case 25:
           rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP3);
   
      if((rdata & (1 << (gpio_num - 22))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP3_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP3_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
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
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
 
    // Check the interrupt raw status for disabled Pads 
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt not raised for disabled Pad\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt raised for disabled Pad\n",gpio_num);
      flag++; 
    }
 
    // Check the IO CTRL field (20th bit) from GPIO Register 
    if((rdata & 0x100000) != 0) 
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO Pad is in Input mode when pad_value is '0'\n",gpio_num);
    }
    else {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO pad is not in Input mode when pad_value is '0'\n",gpio_num);
      flag++;
    }
    
    // Check for CPU raw interrupt status
    rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << (gpio_num - 6))) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt not raised for disabled\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }

#else
  // Check DIN,raw interrupt and IO_CTRL fields for GPIOs Pads 0-7 
  if(gpio_num >= 0 && gpio_num < 4)
  { 
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);
   
    if((rdata & (1 << gpio_num)) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP3:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP3:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
  
    // Reading DIN, Interrupt raw status and IO_CTRL values from GPIO Register for disabled Pads
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
   
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
   
    // Check the interrupt raw status for disabled Pads 
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt not raised for disabled Pad\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt raised for disabled Pad\n",gpio_num);
      flag++; 
    }
 
    // Check the IO CTRL field (20th bit) from GPIO Register 
    if((rdata & 0x100000) != 0) 
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO Pad is in Input mode when pad_value is '0'\n",gpio_num);
    }
    else {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO pad is not in Input mode when pad_value is '0'\n",gpio_num);
      flag++;
    }
            
    // Check for CPU raw interrupt status
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << gpio_num)) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt not raised for disabled\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
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
   
      if((rdata & (1 << (gpio_num - 6))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP1_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
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
   
      if((rdata & (1 << (gpio_num - 14))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP2_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP2_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
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
   
      if((rdata & (1 << (gpio_num - 22))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP3_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP3_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
      break;

      case 30:
      case 31:
    //  case 32:
    //  case 33:
    //  case 34:
    //  case 35:
    //  case 36:
    //  case 37:
      rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP4);
   
      if((rdata & (1 << (gpio_num - 30))) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP4_GP0:: DIN value matches with the Pad_value\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS_GROUP4_GP0:: DIN value does not match with the Pad_value\n",gpio_num);
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
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
 
    // Check the interrupt raw status for disabled Pads 
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt not raised for disabled Pad\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Raw Interrupt raised for disabled Pad\n",gpio_num);
      flag++; 
    }
 
    // Check the IO CTRL field (20th bit) from GPIO Register 
    if((rdata & 0x100000) != 0) 
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO Pad is in Input mode when pad_value is '0'\n",gpio_num);
    }
    else {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: GPIO pad is not in Input mode when pad_value is '0'\n",gpio_num);
      flag++;
    }
    
    // Check for CPU raw interrupt status
    rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << (gpio_num - 6))) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt not raised for disabled\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt not raised at processor for disabled pad\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DIS_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }
 
 #endif
}


void Default_IRQHandler() {

int j,rdata;

  int_pend = 0;

  printf("Entered into default IRQ Handler!!\n");

  // Check for DIN, raw interrupt and IO_CTRL fields for GPIO Pads 0-7  
  for(j = 0;j < 4;j++)
  {
    // Check DIN value during negedge for Group1 GPIOs (0:7)  
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_DIN_GROUP1);
    
    if(gpio_number == j)
    {
      if((rdata & (1 << j)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler_GROUP1_GP3:: DIN value matches with the Pad_value\n",j);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler_GROUP1_GP3:: DIN value does not match with the Pad_value\n",j);
        flag++; 
      }
  
      // Reading DIN, Interrupt raw status and IO_CTRL values from GPIO Register for disabled Pads
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4));
   
      // Check for DIN value
      if((rdata & 0x1) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: DIN value matches with the Pad_value\n",j);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: DIN value does not match with the Pad_value\n",j);
        flag++; 
      }
   
      // Check the interrupt raw status for disabled Pads 
      if((rdata & 0x2) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: Raw Interrupt not raised for disabled Pad\n",j);
      }
      else  
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: Raw Interrupt raised for disabled Pad\n",j);
        flag++; 
      }
      
      // Check the IO CTRL field (20th bit) from GPIO Register 
      if((rdata & 0x100000) != 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: GPIO pad is in Input mode for disabled pad\n",j);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: GPIO pad is not in Input mode for disabled\n",j);
        flag++;
      }
      
      // Check for CPU raw interrupt status
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
      
      if((rdata & (1 << j)) != 0)      
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: CPU Raw Interrupt not raised for disabled\n",j);
      }
      else
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: CPU Raw Interrupt raised\n",j);
        flag++;
      }

      // Check for CPU interrupt during negedge for inactive Pads 
      #ifdef PACMAN_PROC 
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
        
        if((rdata & (1 << j)) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: Interrupt not raised at processor for disabled pad\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: Interrupt raised at processor\n",j);
          flag++;
        }
      #else
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
       
        if((rdata & (1 << j)) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: Interrupt not raised at processor for disabled pad\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: Interrupt raised at processor\n",j);
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
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP1);

        // Check DIN value during negedge for Group1 GPIOs (8:15)  
        if((rdata & (1 << (j - 6))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler_GROUP1:: DIN value matches with the Pad_value\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler_GROUP1:: DIN value does not match with the Pad_value\n",j);
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

        // Check DIN value during negedge for Group2 GPIOs (16:23)  
        if((rdata & (1 << (j - 14))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler_GROUP2:: DIN value matches with the Pad_value\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler_GROUP2:: DIN value does not match with the Pad_value\n",j);
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

        // Check DIN value during negedge for Group3 GPIOs (24:31)  
        if((rdata & (1 << (j - 22))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler_GROUP3:: DIN value matches with the Pad_value\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler_GROUP3:: DIN value does not match with the Pad_value\n",j);
          flag++; 
        }
        break;

        case 30:
        case 31:
        //case 32:
        //case 33:
        //case 34:
        //case 35:
        //case 36:
        //case 37:
        //case 38:
        //case 39:
        rdata = rd(MIZAR_GP0_GPIO_GPIO_DIN_GROUP4);

        // Check DIN value during negedge for Group4 GPIOs (32:39)  
        if((rdata & (1 << (j - 30))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler_GROUP4:: DIN value matches with the Pad_value\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler_GROUP4:: DIN value does not match with the Pad_value\n",j);
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
      if((rdata & 0x1) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: DIN value matches with the Pad_value\n",j);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: DIN value does not match with the Pad_value\n",j);
        flag++; 
      }
  
      // Check for interrupt raw status bit during negedge for active/enabled Pad
      if((rdata & 0x2) != 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: Raw Interrupt raised at negedge\n",j);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: Raw Interrupt not raised\n",j);
        flag++;
      }
     
      // Check the IO CTRL field (20th bit) from GPIO Register 
      if((rdata & 0x100000) != 0) 
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: Current GPIO pad is in Input mode\n",j);
      }
      else {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: Current GPIO pad is not in Input mode\n",j);
        flag++;
      }

      //Check for CPU raw interrupt status
      rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
      
      if((rdata & (1 << (j - 6))) != 0)      
      {
        printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: CPU Raw Interrupt raised\n",j);
      }
      else
      {
        printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: CPU Raw Interrupt not raised\n",j);
        flag++;
      }

      // Check for CPU interrupt during negedge for active Pad 
      #ifdef PACMAN_PROC 
        rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
        
        if((rdata & (1 << (j - 6))) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: Interrupt raised at processor\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: Interrupt not raised at processor\n",j);
          flag++;
        }
      #else
        rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
        
        if((rdata & (1 << (j - 6))) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: Interrupt raised at processor\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: Interrupt not raised at processor\n",j);
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


 

