/*
 * lsm62s3.c
 *
 *  Created on: Aug 27, 2021
 *      Author: ROG
 */
#include "lsm6ds3.h"

uint8_t lsm6ds3_whoami ()
{
  uint8_t whoami;
  //I2C1_read_reg (0x6A, 0x0F, &whoami, 1);
  HAL_I2C_Mem_Read(&hi2c1, LSM6DS3_I2C_ADDR, WHO_AM_I, 1, &whoami, 1, 100);
  return whoami;
}
