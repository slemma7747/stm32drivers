/**
  ******************************************************************************
  * File Name          : I2C.c
  * Description        : This file provides code for the configuration
  *                      of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10909CEC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void I2C1_write (uint8_t addr, uint8_t* data, uint32_t size)
{
  I2C1->CR1       &=  ~(I2C_CR1_PE);                                                  // Reset PE to reset internal state machines and status bits
  I2C1->CR1       |=  (I2C_CR1_PE);
  I2C1->CR2       =   0;
  // More I2C configuation settings
  I2C1->CR2     |=  (I2C_CR2_AUTOEND);                                                // Autoend mode
  I2C1->CR2     |=  (addr << 1);                                                      // Slave address
  I2C1->CR2     &=  ~(I2C_CR2_RD_WRN);                                                // Write mode
  I2C1->CR2     |=  ((I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (size<<I2C_CR2_NBYTES_Pos));   // Data size
  I2C1->CR2     |=  I2C_CR2_START;                                                    // Set START condition

  while (size--)
  {
   while ( !(I2C1->ISR & (I2C_ISR_TXIS)) )                                            // Wait for Transmission Ready
   {
    if ( (I2C1->ISR & (I2C_ISR_NACKF)) == I2C_ISR_NACKF )
      {
        I2C1->ICR   |=  I2C_ICR_NACKCF | I2C_ICR_STOPCF;
        I2C1->CR2    =  0;
        return;
      }
    }
    I2C1->TXDR  =  *data++;
  }

  while ( !(I2C1->ISR & (I2C_ISR_STOPF)) );                                          // Wait for STOP flag; Set automatically in AUTOEND
  I2C1->ICR   |=  (I2C_ICR_STOPCF);                                                  // Clear STOP flag
  I2C1->CR2    =  0;
}


void I2C1_read (uint8_t addr, uint8_t* data, uint32_t size)
{

  I2C1->CR1       &=  ~(I2C_CR1_PE);                                                  // Reset PE to reset internal state machines and status bits
  I2C1->CR1       |=  (I2C_CR1_PE);
  I2C1->CR2       =  0;
  // More I2C configuation settings
  I2C1->CR2     &=  ~(I2C_CR2_AUTOEND);                                               // Autoend mode
  I2C1->CR2     |=  (addr << 1);                                                      // Slave address
  I2C1->CR2     |=  (I2C_CR2_RD_WRN);                                                 // Read mode
  I2C1->CR2     |=  ((I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (size<<I2C_CR2_NBYTES_Pos));   // Data size
  I2C1->CR2     |=  I2C_CR2_START;                                                    // Set START condition

  while (size>0)
  {
      while ( !(I2C1->ISR & (I2C_ISR_RXNE)) )                                         // Wait for Reception Ready
      {
        if ( (I2C1->ISR & (I2C_ISR_NACKF)) == I2C_ISR_NACKF )
        {
          // TODO : timeout
          I2C1->ICR   |=  (I2C_ICR_NACKCF);
          I2C1->ICR   |=  (I2C_ICR_STOPCF);
          I2C1->CR2    =  0;
          return;
        }
      }
      *data++ = I2C1->RXDR;
      size--;
  }

  I2C1->CR2     |=  I2C_CR2_STOP;
  while ( !(( I2C1->ISR & (I2C_ISR_STOPF) ) == I2C_ISR_STOPF) );                     // Wait for STOP flag; Set automatically in AUTOEND
  I2C1->ICR   |=  (I2C_ICR_STOPCF);                                                  // Clear STOP flag
  I2C1->CR2    =  0;
}

void I2C1_read_reg (uint8_t addr, uint8_t reg_addr, uint8_t* data, uint32_t size)
{
  I2C1->CR1       &=  ~(I2C_CR1_PE);                                                  // Reset PE to reset internal state machines and status bits
  I2C1->CR1       |=  (I2C_CR1_PE);

  I2C1->CR2     &=  ~(I2C_CR2_AUTOEND);                                               // Software end mode
  I2C1->CR2     |=  (addr << 1);                                                      // Slave address
  I2C1->CR2     &=  ~(I2C_CR2_RD_WRN);                                                // Write mode
  I2C1->CR2     |=  ((I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (1<<I2C_CR2_NBYTES_Pos));      // Data size: single byte of slave register address
  I2C1->CR2     |=  I2C_CR2_START;                                                    // Set START condition

  while ( !(I2C1->ISR & (I2C_ISR_TXIS)) )                                             // Wait for Transmission Ready
  {
    if ( (I2C1->ISR & (I2C_ISR_NACKF)) == I2C_ISR_NACKF )                             // Check if Not-acknowledged signal is detected; quit if so
    {
      I2C1->ICR   |=  I2C_ICR_NACKCF | I2C_ICR_STOPCF;
      I2C1->CR2    =  0;                                                              // Clear CR2 before quitting
      return;
    }
  }
  I2C1->TXDR  =  reg_addr;

  I2C1->CR2     |=  (I2C_CR2_RD_WRN);                                                 // Read mode
  I2C1->CR2     |=  ((I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (size<<I2C_CR2_NBYTES_Pos));   // Data size
  I2C1->CR2     |=  I2C_CR2_START;                                                    // Set START condition again for a restart signal

  while (size>0)
   {
       while ( !(I2C1->ISR & (I2C_ISR_RXNE)) )                                         // Wait for Reception Ready
       {
         if ( (I2C1->ISR & (I2C_ISR_NACKF)) == I2C_ISR_NACKF )
         {
           // TODO : timeout
           I2C1->ICR   |=  I2C_ICR_NACKCF | I2C_ICR_STOPCF;
           I2C1->CR2    =  0;
           return;
         }
       }
       *data++ = I2C1->RXDR;
       size--;
   }

   I2C1->CR2     |=  I2C_CR2_STOP;
   while ( !(( I2C1->ISR & (I2C_ISR_STOPF) ) == I2C_ISR_STOPF) );                     // Wait for STOP flag; Set automatically in AUTOEND
   I2C1->ICR   |=  (I2C_ICR_STOPCF);                                                  // Clear STOP flag
   I2C1->CR2    =  0;

}

void I2C1_write_reg (uint8_t addr, uint8_t reg_addr, uint8_t* data, size_t size)
{
  I2C1->CR1       &=  ~(I2C_CR1_PE);                                                  // Reset PE to reset internal state machines and status bits
  I2C1->CR1       |=  (I2C_CR1_PE);

  while (I2C1->ISR & I2C_ISR_BUSY);
  I2C1->CR2     &=  ~(I2C_CR2_AUTOEND);                                               // Software end mode
  I2C1->CR2     |=  (I2C_CR2_RELOAD);
  I2C1->CR2     |=  (addr << 1);                                                      // Slave address
  I2C1->CR2     &=  ~(I2C_CR2_RD_WRN);                                                // Write mode
  I2C1->CR2     |=  ((I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (1<<I2C_CR2_NBYTES_Pos));      // Data size: single byte of slave register address
  I2C1->CR2     |=  I2C_CR2_START;                                                    // Set START condition

  while ( !(I2C1->ISR & (I2C_ISR_TXIS)) )                                             // Wait for Transmission Ready
  {
    if ( (I2C1->ISR & (I2C_ISR_NACKF)) == I2C_ISR_NACKF )                             // Check if Not-acknowledged signal is detected; quit if so
    {
      I2C1->ICR   |=  I2C_ICR_NACKCF | I2C_ICR_STOPCF;
      I2C1->CR2    =  0;                                                              // Clear CR2 before quitting
      return;
    }
  }
  I2C1->TXDR  =  reg_addr;
  while (!(I2C1->ISR & I2C_ISR_TCR));
  I2C1->CR2     &=  ~(I2C_CR2_RELOAD);
  I2C1->CR2     &=  ~(I2C_CR2_RD_WRN);                                                // Write mode
  I2C1->CR2     |=  ((I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (size<<I2C_CR2_NBYTES_Pos));   // Data size
  I2C1->CR2     |=  I2C_CR2_START;                                                    // Set START condition again for a restart signal

  while (size--)
   {
    while ( !(I2C1->ISR & (I2C_ISR_TXIS)) )                                            // Wait for Transmission Ready
    {
     if ( (I2C1->ISR & (I2C_ISR_NACKF)) == I2C_ISR_NACKF )
       {
         I2C1->ICR   |=  I2C_ICR_NACKCF | I2C_ICR_STOPCF;
         I2C1->CR2    =  0;
         return;
       }
     }
     I2C1->TXDR  =  *data++;
   }

   I2C1->CR2     |=  I2C_CR2_STOP;
   while ( !(I2C1->ISR & (I2C_ISR_STOPF)) );                                          // Wait for STOP flag; Set automatically in AUTOEND
   I2C1->ICR   |=  (I2C_ICR_STOPCF);                                                  // Clear STOP flag
   I2C1->CR2    =  0;
}

void I2C1_scan (void)
{
  MY_Printf("[%d] scanning...\n\r", sec);

  I2C1->CR1       |=  (I2C_CR1_PE);

  for (uint8_t addr = 0; addr <= 0x7F; addr++)
  {
    int timeout = 100;
    int scan_time = msec;

    I2C1->CR2    =  0;
    while ( I2C1->ISR & I2C_ISR_BUSY );

    I2C1->CR2     |=  (I2C_CR2_AUTOEND);                                             // Autoend mode
    I2C1->CR2     |=  (addr << 1);                                                   // Slave address
    I2C1->CR2     &=  ~(I2C_CR2_RD_WRN);                                             // Write mode
    I2C1->CR2     |=  ((I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (0<<I2C_CR2_NBYTES_Pos));   // Data size
    I2C1->CR2     |=  (I2C_CR2_START);                                               // Set START condition

    while ( !(I2C1->ISR & (I2C_ISR_STOPF)) )    // Wait for STOP flag; Set automatically in AUTOEND
    {
      if ( msec - scan_time > timeout)
      {
        MY_Printf ("No device found!\n\r");
        MY_Printf ("\n\r");                     // embedding newline/carrier feed in a textline does not display when debugging
        I2C1->CR1 &=  ~(I2C_CR1_PE);
        return;
      }
    }

    I2C1->ICR   |=  (I2C_ICR_STOPCF);            // Clear STOP flag
    if ( I2C1->ISR & (I2C_ISR_NACKF) )
    {
      I2C1->ICR   |= (I2C_ICR_NACKCF);
      if (addr == 0x7F)
        MY_Printf ("\tNo other device found!\n\r");
      continue;
    }

    MY_Printf ("\tDevice found at address:  %#02X \n\r", addr);
    MY_Printf ("\n\r");                     // embedding newline/carrier feed in a textline does not display when debugging

  }
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
