/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "tim.h"

char   USART_Cmd[CLI_MAX_STR_LEN+1];       // CLI command string pre-processing; +1 for NULL terminator
uint8_t   USART_New_Cmd = FALSE;              // CLI new command flag
uint32_t USART_Cmd_Count = 0;                             // Counter for CLI command string length
/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
/**
  * @brief  Store integer into an ASCII string.
  * @param  value is the integer to be converted.
  * @param  sp points to the string.
  * @param  radix specifies number format.
            This parameter can be: 10 - decimal; 16 - hexadecimal; 8 - octal.
  * @retval The length of the string (max 16) is returned
  */
uint8_t MY_ItoA(int value, char *sp, int radix)
{
  unsigned long len;
  char          tmp[16], *tp = tmp;
  int           i, v, sign   = radix == 10 && value < 0;

  v = sign ? -value : value;

  while (v || tp == tmp)
  {
    i = v % radix;
    v /= radix;
    *tp++ = i < 10 ? (char) (i + '0') : (char) (i + 'A' - 10);
  }

  len = tp - tmp;

  if (sign)
  {
    *sp++ = '-';
    len++;
  }

  while (tp > tmp)
      *sp++ = *--tp;

  *sp++ = '\0';

  return len;
}

/**
  * @brief  Store numerical ASCII sting into an integer.
  * @param  str points to the string.
  * @retval The integer is returned
  */
int MY_AtoI(uint8_t* str)
{
  int res = 0;

  for (int i = 0; str[i] != '\0'; ++i)
      res = res * 10 + str[i] - '0';

  return res;
}

/**
  * @brief  Compare two strings.
  * @param  p1 first string to be compared.
  * @param  p2 second string to be compared.
  * @retval Status of the comparison is returned.
            Returns value < 0: p1 is less than p2;
            Returns value > 0: p2 is less than p2;
            Returns value = 0: p1 is equal to p2.
  */
int MY_StrCmp (const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;

  do
  {
    c1 = (unsigned char) *s1++;
    c2 = (unsigned char) *s2++;
    if (c1 == '\0')
      return c1 - c2;
  } while (c1 == c2);

  return c1 - c2;
}

/**
  * @brief  Send a character over the UART interface in blocking mode.
  * @param  uartHandle UART handle structure.
  * @param  char_data Character to be sent.
  * @retval None
  */
void MY_UART_PutChar (UART_HandleTypeDef* uartHandle, char char_data)
{
  // TODO timeout
  uartHandle->Instance->DR = char_data;
  while ( !(uartHandle->Instance->SR & USART_SR_TXE) );
}

/**
  * @brief  Send a string over the UART interface character by character in blocking mode.
  * @param  uartHandle UART handle structure.
  * @param  str_data String to be sent.
  * @retval None
  */
void MY_UART_PutStr(UART_HandleTypeDef* uartHandle, char * str_data)
{
    while(*str_data != 0)
    {
        MY_UART_PutChar(uartHandle, *str_data);
        str_data++;
    }
}

/**
  * @brief  Printf equivalent to send string over UART interface
  * @param  format string to send.
  * @param  ... argument list equivalent to printf
  * @retval None
  */
void MY_Printf (const char* format, ...)
{
  va_list args;
  static char str_buf[CLI_MAX_STR_LEN];

  va_start (args, format);
  (void) vsnprintf ((char *) str_buf, CLI_MAX_STR_LEN, format, args);

  MY_UART_PutStr(&huart2, str_buf);
  va_end (args);
}

/**
  * @brief  Send an integer over the UART interface in blocking mode.
  * @param  uartHandle UART handle structure.
  * @param  val integer to be sent.
  * @param  radix specifies number format.
            DEC - decimal;
            HEX - hexadecimal;
            OCT - octal;
  * @retval None
  */
void MY_UART_PutInt (UART_HandleTypeDef* uartHandle, int val, int radix)
{
  char temp[16];
  MY_ItoA (val, temp, radix);
  MY_UART_PutStr (uartHandle, temp);
}

/**
  * @brief  Processes the CLI string in usart_cmd
  * @param  uartHandle UART handle structure.
  * @param  val integer to be sent.
  * @retval None
  */
void MY_UART_ProcessCmd (void)
{
  // TODO contingency; TODO what does this mean
  // TODO "help"
  // TODO fix case sensitivity
  uint8_t tmp[CLI_MAX_STR_LEN - 1];
  if ( MY_StrCmp((const char *)USART_Cmd, (const char *)"test") == 0 )
    MY_Printf("\n\rCLI test pass!\n\r");
  else if ( USART_Cmd[0] == '>')
  {
    for (int i = 1; i < CLI_MAX_STR_LEN; i++)
      tmp[i - 1] = USART_Cmd[i];

    htim2.Instance->CCR1 = PERIOD_VALUE * MY_AtoI(tmp) / 100;
    MY_Printf("Duty Cycle set to: %d%%\n\r", MY_AtoI(tmp));
  }
  else if ( MY_StrCmp ((const char *)USART_Cmd, (const char *)"RESET") == 0 )
      NVIC_SystemReset();
//  else if ( MY_StrCmp ((const char *)USART_Cmd, (const char *)"scan") == 0 )
//      I2C1_scan();
//  else if ( MY_StrCmp ((const char *)USART_Cmd, (const char *)"dump") == 0 )
//        dump();
  else
  {
    MY_Printf (USART_Cmd);
    MY_Printf(" :INVALID COMMAND!\n\r");
  }
  USART_New_Cmd = FALSE;
}

/**
  * @brief  UART interrupt handler
  * @param  none
  * @retval None
  */
void MY_UART_Irq (void)
{
// TODO: if possible, try to echo outside of irq
  if ((huart2.Instance->SR & USART_SR_RXNE) == USART_SR_RXNE)
  {
    switch (huart2.Instance->DR)
    {
      case '\r': // add LF to CR
      {
        USART_Cmd[USART_Cmd_Count] = '\0';
        USART_New_Cmd = TRUE;
        USART_Cmd_Count = 0;
        MY_UART_PutStr(&huart2, (char *)"\n\r");
        break;
      }
      case '\b': // DEL
      {
        if(USART_Cmd_Count > 0)
        {
          USART_Cmd_Count--;
          USART_Cmd[USART_Cmd_Count] = '\0';
          MY_UART_PutChar(&huart2, (char)0x7F);
        }
        break;
      }
      default:
      {
        if(USART_Cmd_Count < CLI_MAX_STR_LEN)
        {
          USART_Cmd[USART_Cmd_Count] = huart2.Instance->DR;
          MY_UART_PutChar(&huart2, USART_Cmd[USART_Cmd_Count]);
          USART_Cmd_Count++;
        }
        break;
      }
    }
  }
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
