#ifndef BMP180_H_
#define BMP180_H_

#include "stm32f4xx_hal.h"


#define BMP180_ADDR 0xEE


void BMP180_Init(I2C_HandleTypeDef *hi2c);
float BMP180_Read_Temp(I2C_HandleTypeDef *hi2c);
uint8_t BMP180_Read_All(I2C_HandleTypeDef *hi2c, float *temp, float *press);

#endif /* BMP180_H_ */
