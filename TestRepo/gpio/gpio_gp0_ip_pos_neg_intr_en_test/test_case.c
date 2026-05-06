#include<stdio.h>
#include<testdef.h>

int gpio_number,flag;
int gp0_num1_posedge_en = 0;
int gp0_num1_negedge_en = 0;
int gp0_num2_posedge_en = 0;
int gp0_num2_negedge_en = 0;
extern int int_pend;

void test_case() {

int i;
  
  flag = 0;
 
  pinmux_for_gpio_func();

  // For enabling input mode, posedge and negedge interrupt for GPIOs 0-7  
  for(i = 0; i < 4; i++) {
    
    printf("Enabling GPIO_3 register fields..\n");
    
    // Programming GPIO in Input Mode; Enabling posedge and negedge interrupt (20th bit as '1', 17th bit as '1' & 18th bit as '1')
    sec_wr(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00160000);

    // Clearing the raw status interrupt bit (Set 16th bit to '1') 
    sec_wr(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00170000);
  }

  // For enabling input mode, posedge and negedge interrupt for GPIOs 8-39  
  for(i = 0; i < 26; i++) {
    
    printf("Enabling GPIO_0 register fields..\n");
    
    // Programming GPIO in Input Mode; Enabling posedge and negedge interrupt (20th bit as '1', 17th bit as '1' & 18th bit as '1')
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00160000);

    // Clearing the raw status interrupt bit (Set 16th bit to '1') 
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00170000);
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

  // Initializing the scratch pad register value to '0'
  sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0);
  sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x0);

  // Writing scratch pad register for GPIOs 0-7
  for(i = 0; i < 4; i++) {
   
    printf("Toggling the GPIO pins by writing into scratch register_1..\n");
   
    gpio_number = i;
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num1_posedge_en = 1;    
 #ifdef MIZAR_40_PIN_PKG

    if(i == 2){

printf("ignored this pad = %0d \n",i);

         continue;
} 

#endif
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,1 << i);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num1_negedge_en = 1;

    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num1_posedge_en = 1;    
    
    // Toggling the GPIO pins by writing into scratch register
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,1 << i);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num1_negedge_en = 1;

    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num1_posedge_en = 1;    
    
    // Toggling the GPIO pins by writing into scratch register
    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,1 << i);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num1_negedge_en = 1;

    sec_wr(I2BW_ADC0_CH6_BUF0_START_ADDR,0x0);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
  }
   
  // Writing scratch pad register for GPIOs 8-39
  for(i = 6; i < 32; i++) {
   
    printf("Toggling the GPIO pins by writing into scratch register_2..\n");
   
    gpio_number = i;
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num2_posedge_en = 1;    
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
 
 
    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,1 << (i - 6));
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num2_negedge_en = 1;
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

    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x0);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num2_posedge_en = 1;    
    
    // Toggling the GPIO pins by writing into scratch register
    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,1 << (i - 6));
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num2_negedge_en = 1;

    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x0);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num2_posedge_en = 1;    
    
    // Toggling the GPIO pins by writing into scratch register
    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,1 << (i - 6));
    wait_on(20);
    while(int_pend) {
       wait_on(10);
    }
    
    // Enable CPU interrupt
    int_pend = 1;
    enable_cpu_intr(i);
    gp0_num2_negedge_en = 1;

    sec_wr(I2BW_ADC0_CH6_BUF1_START_ADDR,0x0);
    wait_on(20);
    while(int_pend) {
       wait_on(10);
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

// Check for DIN, Interrupt raw status and CPU interrupt status for disabled Pads
void chk_din_intr_for_disabled_pads(unsigned int dis_gpio_num) {

int rdata;
 
  if(dis_gpio_num >= 0 && dis_gpio_num < 6)
  { 
    // Reading the DIN and Interrupt raw status values from GPIO Register
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (dis_gpio_num * 4));
    
    // Check for DIN value
    if((rdata & 0x1) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value matches with the Pad_value\n",dis_gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value does not match with the Pad_value\n",dis_gpio_num);
      flag++; 
    }
    
    // Check the interrupt raw status when Pad_value changes from '1' to '0'
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt not raised when Pad_value is 0/1\n",dis_gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt raised on negedge/posedge\n",dis_gpio_num);
      flag++; 
    }
    
    //Check for CPU raw interrupt status
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << dis_gpio_num)) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt not raised\n",dis_gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt raised\n",dis_gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << dis_gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",dis_gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",dis_gpio_num);
        flag++;
      }
    #else
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << dis_gpio_num)) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",dis_gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",dis_gpio_num);
        flag++;
      }
    #endif
  }
  else 
  { 
    // Reading the DIN and Interrupt raw status values from GPIO Register
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((dis_gpio_num - 6) * 4));
    
    // Check for DIN value
    if((rdata & 0x1) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value matches with the Pad_value\n",dis_gpio_num);
    }
    else 
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: DIN value does not match with the Pad_value\n",dis_gpio_num);
      flag++; 
    }
    
    // Check the interrupt raw status when Pad_value changes from '1' to '0'
    if((rdata & 0x2) == 0)
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt not raised when Pad_value is 0/1\n",dis_gpio_num);
    }
    else  
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Raw Interrupt raised on negedge/posedge\n",dis_gpio_num);
      flag++; 
    }
    
    //Check for CPU raw interrupt status
    rdata = rd(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
    
    if((rdata & (1 << (dis_gpio_num - 6))) == 0)      
    {
      printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt not raised\n",dis_gpio_num);
    }
    else
    {
      printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: CPU Raw Interrupt raised\n",dis_gpio_num);
      flag++;
    }

    // Check for CPU interrupt during negedge for inactive Pads 
    #ifdef PACMAN_PROC 
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR2_INTR_STS1);
      
      if((rdata & (1 << (dis_gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",dis_gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",dis_gpio_num);
        flag++;
      }
    #else
      rdata = rd(MIZAR_GP0_GPIO_GP0_INTR1_INTR_STS1);
     
      if((rdata & (1 << (dis_gpio_num - 6))) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt not raised at processor for disabled pads\n",dis_gpio_num);
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHK_DIN_INTR_FOR_DISABLED_PADS:: Interrupt raised at processor\n",dis_gpio_num);
        flag++;
      }
    #endif
  }
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
      if(gp0_num1_posedge_en == 1) 
      { 
        // Check for DIN value during posedge for active/enabled Pad  
        if((rdata & 0x1) != 0)
        {
          printf("SUCCESS: POSEDGE_EN:: Default_IRQHandler:: DIN value matches with the Pad_value\n");
          gp0_num1_posedge_en = 0;
        }
        else 
        {
          printf("ERROR: POSEDGE_EN:: Default_IRQHandler:: DIN value does not match with the Pad_value\n");
          flag++; 
        }
      }
      
      if(gp0_num1_negedge_en == 1) 
      {
        // Check for DIN value during negedge for active/enabled Pad  
        if((rdata & 0x1) == 0)
        {
          printf("SUCCESS: NEGEDGE_EN:: Default_IRQHandler:: DIN value matches with the Pad_value\n");
          gp0_num1_negedge_en  = 0;
        }
        else 
        {
          printf("ERROR: NEGEDGE_EN:: Default_IRQHandler:: DIN value does not match with the Pad_value\n");
          flag++; 
        }
      }
      
      // Check for interrupt raw status bit for active/enabled Pad
      if((rdata & 0x2) != 0)
      {
        printf("SUCCESS: Default_IRQHandler:: Raw Interrupt raised at posedge/negedge\n");
      }
      else 
      {
        printf("ERROR: Default_IRQHandler:: Raw Interrupt not raised\n");
        flag++;
      }
      
      //Check for CPU raw interrupt status
      rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1);
      
      if((rdata & (1 << j)) != 0)      
      {
        printf("SUCCESS: Default_IRQHandler:: CPU Raw Interrupt raised\n");
      }
      else 
      {
        printf("ERROR: Default_IRQHandler:: CPU Raw Interrupt not raised\n");
        flag++;
      }

      // Check for CPU interrupt for active/enabled Pad 
      #ifdef PACMAN_PROC 
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR2_INTR_STS1);
        
        if((rdata & (1 << j)) != 0)
        {
          printf("SUCCESS: Default_IRQHandler:: Interrupt raised at processor\n");
        }
        else 
        {
          printf("ERROR: Default_IRQHandler:: Interrupt not raised at processor\n");
          flag++;
        }
      #else
        rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_INTR1_INTR_STS1);
        
        if((rdata & (1 << j)) != 0)
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
          chk_din_intr_for_disabled_pads(j);
        }
      }
      else {
        if(j % 2 == 0)
        {
          chk_din_intr_for_disabled_pads(j);
        }
      }
    }
  
    // Clearing the interrupt raw status bit (16th bit of GPIO reg set to '1') 
    sec_wr(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4),0x00170000);

    // Clearing the CPU raw interrupt status bit 
    sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_INTR_RAW_STCLR1,0x0);
  }

  for(j = 6;j < 32;j++) 
  { 
    // Reading the DIN and interrupt raw status values from GPIO Register
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((j - 6) * 4));
    
    if(gpio_number == j)  
    {  
      if(gp0_num2_posedge_en == 1)
      {
        // Check for DIN value during posedge for active/enabled Pad  
        if((rdata & 0x1) != 0)
        {
          printf("SUCCESS: POSEDGE_EN:: Default_IRQHandler:: DIN value matches with the Pad_value\n");
          gp0_num2_posedge_en = 0;
        }
        else 
        {
          printf("ERROR: POSEDGE_EN:: Default_IRQHandler:: DIN value does not match with the Pad_value\n");
          flag++; 
        }
      }
      
      if(gp0_num2_negedge_en == 1) 
      {
        // Check for DIN value during negedge for active/enabled Pad  
        if((rdata & 0x1) == 0)
        {
          printf("SUCCESS: NEGEDGE_EN:: Default_IRQHandler:: DIN value matches with the Pad_value\n");
          gp0_num2_negedge_en = 0;
        }
        else 
        {
          printf("ERROR: NEGEDGE_EN:: Default_IRQHandler:: DIN value does not match with the Pad_value\n");
          flag++; 
        }
      }
      
      // Check for interrupt raw status bit during posedge for active/enabled Pad
      if((rdata & 0x2) != 0)
      {
        printf("SUCCESS: Default_IRQHandler:: Raw Interrupt raised at posedge/negedge\n");
      }
      else 
      {
        printf("ERROR: Default_IRQHandler:: Raw Interrupt not raised\n");
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
          chk_din_intr_for_disabled_pads(j);
        }
      }
      else {
        if(j % 2 == 0)
        {
          chk_din_intr_for_disabled_pads(j);
        }
      }
    }
  
    // Clearing the interrupt raw status bit (16th bit of GPIO reg set to '1') 
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((j - 6) * 4),0x00170000);
  
    // Clearing the CPU raw interrupt status bit 
    wr(MIZAR_GP0_GPIO_GPIO_INTR_RAW_STCLR1,0x0);
  }

  // Clearing the Processor interrupt bit 
  disable_cpu_intr();
  
  return;
} 

