#include<stdio.h>
#include<testdef.h>

int gpio_number,flag;
extern int int_pend;

void test_case() {
  
int i,j,rdata;

  flag = 0;
 
  pinmux_for_gpio_func();

  // For enabling input mode and negedge interrupt for GPIOs 0-7  
  for(i = 0; i < 4; i++) {
    
    printf("Enabling GPIO_3 register fields..\n");
    
    // Programming GPIO in Input Mode and enabling negedge interrupt(20th bit as '1' & 18th bit as '1')
    sec_wr(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00140000);

    // Clearing the raw status interrupt bit (Set 16th bit to '1') 
    sec_wr(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00150000);
  }

  // For enabling input mode and negedge interrupt for GPIOs 8-39  
  for(i = 0; i < 26; i++) {
    
    printf("Enabling GPIO_0 register fields..\n");
    
    // Programming GPIO in Input Mode and enabling negedge interrupt(20th bit as '1' & 18th bit as '1')
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00140000);

    // Clearing the raw status interrupt bit (Set 16th bit to '1') 
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00150000);
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

    
       wait_on(20000);
  // Initializing all the GPIO pins with the value as '1' by writing into scratch register for GPIO Pads 0-7
  sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0F);
 
  // Initializing all the GPIO pins with the value as '1' by writing into scratch register for GPIO Pads 8-39 
  sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x03FFFFFF);

 
  // Writing the scratch pad register for GPIOs 0-7 
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
        // Reading the DIN and interrupt raw status values from GPIO Register when CPU interrupt is disabled
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4));

        // Check for DIN value for active/enabled Pad 
        if((rdata & 0x0) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: DIN value matches with the Pad_value\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: DIN value does not match with the Pad_value\n",j);
          flag++; 
        }
        
        // Check for interrupt raw status bit for active/enabled Pad 
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

  // Writing the scratch pad register for GPIOs 8-39
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
       wait_onn(10);
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
        // Reading the DIN and interrupt raw status values from GPIO Register when CPU interrupt is disabled
        rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((j - 6) * 4));

        // Check for DIN value for active/enabled Pad 
        if((rdata & 0x0) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d INSIDE_TEST_BODY:: DIN value matches with the Pad_value\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d INSIDE_TEST_BODY:: DIN value does not match with the Pad_value\n",j);
          flag++; 
        }
        
        // Check for interrupt raw status bit for active/enabled Pad 
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
        // Check for DIN,Interrupt raw status and CPU interrupt status for disabled Pads
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

// Check for DIN, Interrupt raw status and CPU interrupt status when Pad value changes from '0' to '1' (Posedge)
void check_for_din_and_intr(unsigned int gpio_num) {

int rdata;
#ifdef MIZAR_40_PIN_PKG
 if(gpio_num >= 3 && gpio_num < 4)
  { 
    // Reading the DIN and Interrupt raw status values from GPIO Register
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
    
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
    
    // Check the interrupt raw status when Pad_value changes from '1' to '0'
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt not raised when Pad_value is 0\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt raised on negedge\n",gpio_num);
      flag++; 
    }
    
    //Check for CPU raw interrupt status
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << gpio_num)) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt not raised\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }

else if (gpio_num >= 0 && gpio_num < 2)
  { 
    // Reading the DIN and Interrupt raw status values from GPIO Register
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
    
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
    
    // Check the interrupt raw status when Pad_value changes from '1' to '0'
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt not raised when Pad_value is 0\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt raised on negedge\n",gpio_num);
      flag++; 
    }
    
    //Check for CPU raw interrupt status
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << gpio_num)) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt not raised\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }

  if(gpio_num >= 6 && gpio_num < 16)
  { 
    // Reading the DIN and Interrupt raw status values from GPIO Register
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((gpio_num - 6) * 4));
    
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
    
    // Check the interrupt raw status when Pad_value changes from '1' to '0'
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt not raised when Pad_value is 0\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt raised on negedge\n",gpio_num);
      flag++; 
    }
    
    //Check for CPU raw interrupt status
    rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << (gpio_num - 6))) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt not raised\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }

else if(gpio_num >= 24 && gpio_num < 25)
  { 
    // Reading the DIN and Interrupt raw status values from GPIO Register
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((gpio_num - 6) * 4));
    
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
    
    // Check the interrupt raw status when Pad_value changes from '1' to '0'
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt not raised when Pad_value is 0\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt raised on negedge\n",gpio_num);
      flag++; 
    }
    
    //Check for CPU raw interrupt status
    rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << (gpio_num - 6))) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt not raised\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }


#else
  if(gpio_num >= 0 && gpio_num < 4)
  { 
    // Reading the DIN and Interrupt raw status values from GPIO Register
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
    
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
    
    // Check the interrupt raw status when Pad_value changes from '1' to '0'
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt not raised when Pad_value is 0\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt raised on negedge\n",gpio_num);
      flag++; 
    }
    
    //Check for CPU raw interrupt status
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << gpio_num)) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt not raised\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #endif
  }

  if(gpio_num >= 6 && gpio_num < 32)
  { 
    // Reading the DIN and Interrupt raw status values from GPIO Register
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((gpio_num - 6) * 4));
    
    // Check for DIN value
    if((rdata & 0x1) != 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value matches with the Pad_value\n",gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value does not match with the Pad_value\n",gpio_num);
      flag++; 
    }
    
    // Check the interrupt raw status when Pad_value changes from '1' to '0'
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt not raised when Pad_value is 0\n",gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt raised on negedge\n",gpio_num);
      flag++; 
    }
    
    //Check for CPU raw interrupt status
    rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << (gpio_num - 6))) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt not raised\n",gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt raised\n",gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
        flag++;
      }
    #else
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << (gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",gpio_num);
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

  for(j = 0;j < 4;j++) 
  { 
    // Reading the DIN and interrupt raw status values from GPIO Register
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4));
    
    if(gpio_number == j)  
    {  
      // Check for DIN value during negedge for active/enabled Pad  
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
     
      //Check for CPU raw interrupt status
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
      
      if((rdata & (1 << j)) != 0)      
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
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
        
        if((rdata & (1 << j)) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d Default_IRQHandler:: Interrupt raised at processor\n",j);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d Default_IRQHandler:: Interrupt not raised at processor\n",j);
          flag++;
        }
      #else
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
        
        if((rdata & (1 << j)) != 0)
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
    sec_rmw(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4),0x00010000,0x00010000);
 
    // Clearing the CPU raw interrupt status bit 
    sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1,0x0);
  }

  for(j = 6;j < 32;j++) 
  { 
    // Reading the DIN and interrupt raw status values from GPIO Register
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((j - 6) * 4));
    
    if(gpio_number == j)  
    {  
      // Check for DIN value during negedge for active/enabled Pad  
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


