#include<stdio.h>
#include<testdef.h>

int gpio_number,flag;
int flag_dout_one = 0;
int flag_dout_zero = 0;
int flag_din_one = 0;
int flag_din_zero = 0;
int flag_gpio_din_one = 0;
int flag_gpio_din_zero = 0;

void test_case() {

int i,j,k;
  
  flag = 0;
 
  pinmux_for_gpio_func();

  // Programming the GPIO Pads 0-7 in Output mode
  for(j = 0;j < 4;j++) {

    printf("Enabling GPIO_SSS_%d register fields..\n",j);
   
    sec_wr(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (j * 4),0x0);
  }

  // Programming the GPIO Pads 8-39 in Output mode
  for(j = 0;j < 26;j++) {

    printf("Enabling GPIO_0_%d register fields..\n",j);
   
    wr(MIZAR_GP0_GPIO_GP0_GPIO_8 + (j * 4),0x0);
  }
 
  // Enabling all 8 GPIOs per virtual register ([7:0] and [23:16] set to 'FF') for GPIO Pads 0-7
  sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_IO_CTRL_GROUP1,0x00FF00FF);

  ///// Enabling all 8 GPIOs per virtual register ([7:0] and [23:16] set to 'FF') for GPIO Pads 8-39
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP1,0x00FF00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP2,0x00FF00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP3,0x00FF00FF);
  wr(MIZAR_GP0_GPIO_GPIO_IO_CTRL_GROUP4,0x00FF00FF);

 
  // Writing into DOUT field for GPIOs 0-7 
  for(i = 0;i < 4;i++) {

    printf("GPIO_OUTPUT_MODE:: Toggling the GPIO pins by writing into DOUT field of GPIO_SSS register_3_%d..\n",i);

    gpio_number = i;
#ifdef MIZAR_40_PIN_PKG

    if(i == 2){

printf("ignored this pad = %0d \n",i);

         continue;
} 

#endif 
    // Write DOUT for Group1 GPIOs (0:7)  
    for(j = 0;j < 5;j++)
    { 
      // Write GPIO Group1 register dout bit as '1' 
      sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_DOUT_GROUP1, 1 << i);
      flag_dout_one = 1;
      flag_din_one = 1;
      for(k = 0; k < 32; k++) {
        check_for_pad_value(k);
        check_for_din_value(k);
      }
           
      // Write GPIO Group1 register dout bit as '0' 
      sec_wr(MIZAR_SSS_GP0_GPIO_GPIO_DOUT_GROUP1,0x0);
      flag_dout_zero = 1;
      flag_din_zero = 1;
      for(k = 0; k < 32; k++) {
        check_for_pad_value(k);
        check_for_din_value(k);
      }
    }

    // Write DOUT bit of GPIO register for GPIO Pads 0-7 
    for(j = 0;j < 5;j++) 
    {
      // Write GPIO register dout bit as '1' 
      sec_rmw(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00200000,0x00200000);
      flag_dout_one = 1;
      flag_gpio_din_one = 1;
      for(k = 0; k < 32; k++) {
        check_for_pad_value(k);
        chk_gpio_din_bit(k);
      }    
       
      // Write GPIO register dout bit as '0' 
      sec_rmw(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (i * 4),0x00200000,0x00000000);
      flag_dout_zero = 1;
      flag_gpio_din_zero = 1;
      for(k = 0; k < 32; k++) {
        check_for_pad_value(k);
        chk_gpio_din_bit(k);
      }
    }
  }

  // Writing into DOUT field for GPIOs 8-39 
  for(i = 6;i < 32;i++) {
 
    printf("GPIO_OUTPUT_MODE:: Toggling the GPIO pins by writing into DOUT field of GPIO_0 register_0_%d..\n",i);

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

    // Write DOUT for Group1 GPIOs (8:15)  
    if(i >= 6 && i < 14)
    {
      for(j = 0;j < 5;j++)
      { 
        // Write GPIO Group1 register dout bit as '1' 
        wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP1, 1 << (i - 6));
        flag_dout_one = 1;
        flag_din_one = 1;
        for(k = 0; k < 32; k++) {
          check_for_pad_value(k);
          check_for_din_value(k);
        }
               
        // Write GPIO Group1 register dout bit as '0' 
        wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP1,0x0);
        flag_dout_zero = 1;
        flag_din_zero = 1;
        for(k = 0; k < 32; k++) {
          check_for_pad_value(k);
          check_for_din_value(k);
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
        flag_dout_one = 1;
        flag_din_one = 1;
        for(k = 0; k < 32; k++) {
          check_for_pad_value(k);
          check_for_din_value(k);
        }
               
        // Write GPIO Group2 register dout bit as '0' 
        wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP2,0x0);
        flag_dout_zero = 1;
        flag_din_zero = 1;
        for(k = 0; k < 32; k++) {
          check_for_pad_value(k);
          check_for_din_value(k);
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
        flag_dout_one = 1;
        flag_din_one = 1;
        for(k = 0; k < 32; k++) {
          check_for_pad_value(k);
          check_for_din_value(k);
        }
       
        // Write GPIO Group3 register dout bit as '0' 
        wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP3,0x0);
        flag_dout_zero = 1;
        flag_din_zero = 1;
        for(k = 0; k < 32; k++) {
          check_for_pad_value(k);
          check_for_din_value(k);
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
        flag_dout_one = 1;
        flag_din_one = 1;
        for(k = 0; k < 32; k++) {
          check_for_pad_value(k);
          check_for_din_value(k);
        }
               
        // Write GPIO Group4 register dout bit as '0' 
        wr(MIZAR_GP0_GPIO_GPIO_DOUT_GROUP4,0x0);
        flag_dout_zero = 1;
        flag_din_zero = 1;
        for(k = 0; k < 32; k++) {
          check_for_pad_value(k);
          check_for_din_value(k);
        }
      }
    }
    
    // Write DOUT bit of GPIO register for GPIO Pads 8-39 
    for(j = 0;j < 5;j++) 
    {
      // Write GPIO register dout bit as '1' 
      rmw(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((i - 6) * 4),0x00200000,0x00200000);
      flag_dout_one = 1;
      flag_gpio_din_one = 1;
      for(k = 0; k < 32; k++) {
        check_for_pad_value(k);
        chk_gpio_din_bit(k);
      }
          
      // Write GPIO register dout bit as '0' 
      rmw(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((i - 6) * 4),0x00200000,0x00000000);
      flag_dout_zero = 1;
      flag_gpio_din_zero = 1;
      for(k = 0; k < 32; k++) {
        check_for_pad_value(k);
        chk_gpio_din_bit(k);
      }
    }
  }  

  finish(flag);
}

// Read the scratch register and check for corresponding GPIO Pad value 
void check_for_pad_value(unsigned int gpio_pad_num) {

int rdata;
 
  // Check the scratch pad value for GPIOs 0-7
  if(gpio_pad_num >= 0 && gpio_pad_num < 4)
  { 
    // Read the scratch register for corresponding GPIO pad value 
    rdata = sec_rd(I2BW_ADC0_CH6_BUF0_START_ADDR);
    
    if(gpio_number == gpio_pad_num)
    {
      if(flag_dout_one == 1)
      {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << gpio_pad_num)) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value matches with dout value\n",gpio_pad_num);
          flag_dout_one = 0;
        }
        else {
          printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value does not match with dout value\n",gpio_pad_num);
          flag++;
        }
      }
      if(flag_dout_zero == 1)
      {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << gpio_pad_num)) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value matches with dout value\n",gpio_pad_num);
          flag_dout_zero = 0;
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

  // Check the scratch pad value for GPIOs 8-39
  if(gpio_pad_num >= 6 && gpio_pad_num < 32)
  { 
    // Read the scratch register for corresponding GPIO pad value 
    rdata = sec_rd(I2BW_ADC0_CH6_BUF1_START_ADDR);
    
    if(gpio_number == gpio_pad_num)
    {
      if(flag_dout_one == 1)
      {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << (gpio_pad_num - 6))) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value matches with dout value\n",gpio_pad_num);
          flag_dout_one = 0;
        }
        else {
          printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: GPIO Pad value does not match with dout value\n",gpio_pad_num);
          flag++;
        }
      }
      if(flag_dout_zero == 1)
      {
        // Check Pad value for enabled GPIO
        if((rdata & (1 << (gpio_pad_num - 6))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value matches with dout value\n",gpio_pad_num);
          flag_dout_zero = 0;
        }
        else {
          printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: GPIO Pad value does not match with dout value\n",gpio_pad_num);
          flag++;
        }
      }
    }
    else {
      // Check Pad value for disabled GPIOs
      if((rdata & (1 << (gpio_pad_num - 6))) == 0)
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

// Check for DIN from individual GPIO Register 
void chk_gpio_din_bit(unsigned int gpio_num) {

int rdata;

  // Reading DIN value from GPIO Register for GPIOs 0-7
  if(gpio_num >= 0 && gpio_num < 4)
  {
    rdata = sec_rd(MIZAR_SSS_GP0_GPIO_GP0_GPIO_8 + (gpio_num * 4));
 
    if(gpio_number == gpio_num) 
    {  
      if(flag_gpio_din_one == 1) 
      { 
        // Check DIN for current GPIO Pad when DOUT bit is set to '1'
        if((rdata & 0x1) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHECK_FOR_DIN_VALUE:: DIN value matches with the Pad_value\n",gpio_num);
          flag_gpio_din_one = 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHECK_FOR_DIN_VALUE:: DIN value does not match with the Pad_value\n",gpio_num);
          flag++; 
        }
      }
      
      if(flag_gpio_din_zero == 1) 
      { 
        // Check DIN for current GPIO Pad when DOUT bit is set to '0'
        if((rdata & 0x1) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHECK_FOR_DIN_VALUE:: DIN value matches with the Pad_value\n",gpio_num);
          flag_gpio_din_zero= 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHECK_FOR_DIN_VALUE:: DIN value does not match with the Pad_value\n",gpio_num);
          flag++; 
        }
      }
    }
    else {
      // Check DIN for Pads except the current enabled Pad 
      if((rdata & 0x1) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_VALUE:: DIN value matches with the Pad_value\n",gpio_num);
        flag_gpio_din_zero= 0;
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_VALUE:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
    }
  }

  // Reading DIN value from GPIO Register for GPIOs 8-39
  if(gpio_num >= 6 && gpio_num < 32)
  {
    rdata = rd(MIZAR_GP0_GPIO_GP0_GPIO_8 + ((gpio_num - 6) * 4));
 
    if(gpio_number == gpio_num) 
    {  
      if(flag_gpio_din_one == 1) 
      { 
        // Check DIN for current GPIO Pad when DOUT bit is set to '1'
        if((rdata & 0x1) != 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHECK_FOR_DIN_VALUE:: DIN value matches with the Pad_value\n",gpio_num);
          flag_gpio_din_one = 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHECK_FOR_DIN_VALUE:: DIN value does not match with the Pad_value\n",gpio_num);
          flag++; 
        }
      }
      
      if(flag_gpio_din_zero == 1) 
      { 
        // Check DIN for current GPIO Pad when DOUT bit is set to '0'
        if((rdata & 0x1) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHECK_FOR_DIN_VALUE:: DIN value matches with the Pad_value\n",gpio_num);
          flag_gpio_din_zero= 0;
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHECK_FOR_DIN_VALUE:: DIN value does not match with the Pad_value\n",gpio_num);
          flag++; 
        }
      }
    }
    else {
      // Check DIN for Pads except the current enabled Pad 
      if((rdata & 0x1) == 0)
      {
        printf("SUCCESS: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_VALUE:: DIN value matches with the Pad_value\n",gpio_num);
        flag_gpio_din_zero= 0;
      }
      else 
      {
        printf("ERROR: GPIO_NUM = %0d INSIDE_CHECK_FOR_DIN_VALUE:: DIN value does not match with the Pad_value\n",gpio_num);
        flag++; 
      }
    }  
  }    
}     
       
// Read the DIN value from Group registers for all the GPIO pads 
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
          printf("ERROR: GPIO_NUM = %0d DOUT7_ONE:: INSIDE_CHK_FOR_DIN_GROUP1_GP3:: DIN value does not match with the Pad_value\n",gpio_pad_num);
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
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP1_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_one = 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP1_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
        
        if(flag_din_zero == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 6))) == 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP1_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_zero= 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP1_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
      }
      else {
        // Check DIN for remaining pads except the current Pad 
        if((rdata & (1 << (gpio_pad_num - 6))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP1_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP1_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
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
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP2_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_one = 0;
          }
          else 
          {
            printf("ERROR:GPIO_NUM = %0d  DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP2_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
        
        if(flag_din_zero == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 14))) == 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP2_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_zero= 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP2_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
      }
      else {
        // Check DIN for remaining pads except the current Pad 
        if((rdata & (1 << (gpio_pad_num - 14))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP2_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP2_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
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
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP3_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_one = 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP3_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
        
        if(flag_din_zero == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 22))) == 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP3_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_zero= 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP3_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
      }
      else {
        // Check DIN for remaining pads except the current Pad 
        if((rdata & (1 << (gpio_pad_num - 22))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP3_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP3_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
          flag++; 
        }
      }
      break;
  
      // Check for DIN value for Group4 GPIOs (32:39)
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
      
      // Check DIN for current active GPIO Pad 
      if(gpio_number == gpio_pad_num) 
      {  
        if(flag_din_one == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 30))) != 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP4_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_one = 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ONE:: INSIDE_CHK_FOR_DIN_GROUP4_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
        
        if(flag_din_zero == 1)
        {
          if((rdata & (1 << (gpio_pad_num - 30))) == 0)
          {
            printf("SUCCESS: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP4_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
            flag_din_zero= 0;
          }
          else 
          {
            printf("ERROR: GPIO_NUM = %0d DOUT_ZERO:: INSIDE_CHK_FOR_DIN_GROUP4_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
            flag++; 
          }
        }
      }
      else {
        // Check DIN for remaining pads except the current Pad 
        if((rdata & (1 << (gpio_pad_num - 30))) == 0)
        {
          printf("SUCCESS: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP4_GP1:: DIN value matches with the Pad_value\n",gpio_pad_num);
        }
        else 
        {
          printf("ERROR: GPIO_NUM = %0d DISABLED_INSIDE_CHK_FOR_DIN_GROUP4_GP1:: DIN value does not match with the Pad_value\n",gpio_pad_num);
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


void Default_IRQHandler() {

  printf("ERROR:: Entered Default IRQ_Handler.. :( \n ");
  flag++;
}



