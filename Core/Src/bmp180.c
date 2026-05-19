#include "bmp180.h"
#include <stdio.h>


short AC1, AC2, AC3, B1, B2, MB, MC, MD;
unsigned short AC4, AC5, AC6;


void BMP180_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t calib_data[22];

    if (HAL_I2C_Mem_Read(hi2c, BMP180_ADDR, 0xAA, 1, calib_data, 22, 100) == HAL_OK) {
        AC1 = (calib_data[0] << 8) | calib_data[1];
        AC2 = (calib_data[2] << 8) | calib_data[3];
        AC3 = (calib_data[4] << 8) | calib_data[5];
        AC4 = (calib_data[6] << 8) | calib_data[7];
        AC5 = (calib_data[8] << 8) | calib_data[9];
        AC6 = (calib_data[10] << 8) | calib_data[11];
        B1  = (calib_data[12] << 8) | calib_data[13];
        B2  = (calib_data[14] << 8) | calib_data[15];
        MB  = (calib_data[16] << 8) | calib_data[17];
        MC  = (calib_data[18] << 8) | calib_data[19];
        MD  = (calib_data[20] << 8) | calib_data[21];
        printf("\r\n\033[1;32m[BMP180] Kalibrace z EEPROM uspesne nactena!\033[0m\r\n");
    } else {
        printf("\r\n\033[1;31m[BMP180 CHYBA] Nepodarilo se nacist kalibraci!\033[0m\r\n");
    }
}
/*

float BMP180_Read_Temp(I2C_HandleTypeDef *hi2c) {
    uint8_t i2c_data[2];
    uint16_t raw_temp = 0;
    uint8_t cmd = 0x2E;

   
    if (HAL_I2C_Mem_Write(hi2c, BMP180_ADDR, 0xF4, 1, &cmd, 1, 100) == HAL_OK) {

        HAL_Delay(5);

        
        if (HAL_I2C_Mem_Read(hi2c, BMP180_ADDR, 0xF6, 1, i2c_data, 2, 100) == HAL_OK) {
            raw_temp = (i2c_data[0] << 8) | i2c_data[1];

            
            long X1 = ((raw_temp - AC6) * AC5) >> 15;
            long X2 = (MC << 11) / (X1 + MD);
            long B5 = X1 + X2;
            long real_temp = (B5 + 8) >> 4;

            return real_temp / 10.0f; 
        }
    }
    return -999.0f; 
}
*/


uint8_t BMP180_Read_All(I2C_HandleTypeDef *hi2c, float *temp, float *press) {
    uint8_t i2c_data[3];
    long UT, UP;
    long X1, X2, X3, B3, B5, B6;
    unsigned long B4, B7;
    long p;
    uint8_t oss = 0; 

    // --- A. Čtení surové teploty (UT) ---
    uint8_t cmd = 0x2E;
    if (HAL_I2C_Mem_Write(hi2c, BMP180_ADDR, 0xF4, 1, &cmd, 1, 100) != HAL_OK) return 0;
    HAL_Delay(5);
    if (HAL_I2C_Mem_Read(hi2c, BMP180_ADDR, 0xF6, 1, i2c_data, 2, 100) != HAL_OK) return 0;
    UT = (i2c_data[0] << 8) | i2c_data[1];

    // --- B. Čtení surového tlaku (UP) ---
    cmd = 0x34 + (oss << 6);
    if (HAL_I2C_Mem_Write(hi2c, BMP180_ADDR, 0xF4, 1, &cmd, 1, 100) != HAL_OK) return 0;
    HAL_Delay(5);
    if (HAL_I2C_Mem_Read(hi2c, BMP180_ADDR, 0xF6, 1, i2c_data, 3, 100) != HAL_OK) return 0;
    UP = ((i2c_data[0] << 16) | (i2c_data[1] << 8) | i2c_data[2]) >> (8 - oss);

    // --- C. Výpočet reálné teploty ---
    X1 = ((UT - AC6) * AC5) >> 15;
    X2 = (MC << 11) / (X1 + MD);
    B5 = X1 + X2;
    *temp = ((B5 + 8) >> 4) / 10.0f; 

    // --- D. Výpočet reálného tlaku ---
    B6 = B5 - 4000;
    X1 = (B2 * ((B6 * B6) >> 12)) >> 11;
    X2 = (AC2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = (((((long)AC1) * 4 + X3) << oss) + 2) >> 2;

    X1 = (AC3 * B6) >> 13;
    X2 = (B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = (AC4 * (unsigned long)(X3 + 32768)) >> 15;

    B7 = ((unsigned long)(UP - B3) * (50000 >> oss));
    if (B7 < 0x80000000) {
        p = (B7 << 1) / B4;
    } else {
        p = (B7 / B4) << 1;
    }

    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;
    p = p + ((X1 + X2 + 3791) >> 4);

    *press = p / 100.0f; 

    return 1;
}
