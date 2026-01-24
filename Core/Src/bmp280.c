/*
 * bmp280.c
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */
#include "bmp280.h"
#include <math.h>

/* External handles */
extern I2C_HandleTypeDef hi2c1;

/* Private variables - calibration data */
static uint16_t dig_T1;
static int16_t dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static int32_t t_fine;

/* Private function prototypes */
static uint8_t BMP_Read8(uint8_t reg);
static void BMP_Write8(uint8_t reg, uint8_t value);
static void BMP_ReadCalibration(void);

/* Private functions */

static uint8_t BMP_Read8(uint8_t reg)
{
    uint8_t value = 0;
    HAL_I2C_Mem_Read(&hi2c1, BMP280_ADDR, reg, 1, &value, 1, HAL_MAX_DELAY);
    return value;
}

static void BMP_Write8(uint8_t reg, uint8_t value)
{
    HAL_I2C_Mem_Write(&hi2c1, BMP280_ADDR, reg, 1, &value, 1, HAL_MAX_DELAY);
}

static void BMP_ReadCalibration(void)
{
    dig_T1 = (uint16_t)(BMP_Read8(0x88) | (BMP_Read8(0x89) << 8));
    dig_T2 = (int16_t)(BMP_Read8(0x8A) | (BMP_Read8(0x8B) << 8));
    dig_T3 = (int16_t)(BMP_Read8(0x8C) | (BMP_Read8(0x8D) << 8));

    dig_P1 = (uint16_t)(BMP_Read8(0x8E) | (BMP_Read8(0x8F) << 8));
    dig_P2 = (int16_t)(BMP_Read8(0x90) | (BMP_Read8(0x91) << 8));
    dig_P3 = (int16_t)(BMP_Read8(0x92) | (BMP_Read8(0x93) << 8));
    dig_P4 = (int16_t)(BMP_Read8(0x94) | (BMP_Read8(0x95) << 8));
    dig_P5 = (int16_t)(BMP_Read8(0x96) | (BMP_Read8(0x97) << 8));
    dig_P6 = (int16_t)(BMP_Read8(0x98) | (BMP_Read8(0x99) << 8));
    dig_P7 = (int16_t)(BMP_Read8(0x9A) | (BMP_Read8(0x9B) << 8));
    dig_P8 = (int16_t)(BMP_Read8(0x9C) | (BMP_Read8(0x9D) << 8));
    dig_P9 = (int16_t)(BMP_Read8(0x9E) | (BMP_Read8(0x9F) << 8));
}

/* Public functions */

void BMP_Init(void)
{
    uint8_t id = BMP_Read8(0xD0);

    if (id != 0x58)
    {
        /* Error - sensor not connected */
        return;
    }

    BMP_ReadCalibration();

    BMP_Write8(0xF5, 0b00001000);  /* Filter x4, standby 0.5ms */
    BMP_Write8(0xF4, 0b01010111);  /* Oversampling x1, normal mode */
    /* Wait for first measurement to complete with x16 oversampling (~25ms) */
    HAL_Delay(50);
}

float BMP_ReadTemperature(void)
{
    int32_t adc_T;
    int32_t var1, var2;
    float T;

    adc_T = (BMP_Read8(0xFA) << 12) | (BMP_Read8(0xFB) << 4) | (BMP_Read8(0xFC) >> 4);

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * (int32_t)dig_T2) >> 11;
    var2 = (((((adc_T >> 4) - (int32_t)dig_T1) * ((adc_T >> 4) - (int32_t)dig_T1)) >> 12) * (int32_t)dig_T3) >> 14;

    t_fine = var1 + var2;
    T = (float)((t_fine * 5 + 128) >> 8);

    return T / 100.0f;
}

float BMP_ReadPressure(void)
{
    int32_t adc_P;
    int64_t var1, var2, p;

    adc_P = (BMP_Read8(0xF7) << 12) | (BMP_Read8(0xF8) << 4) | (BMP_Read8(0xF9) >> 4);

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = (((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12));
    var1 = ((((int64_t)1 << 47) + var1) * (int64_t)dig_P1) >> 33;

    if (var1 == 0)
    {
        return 0;
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (float)p / 256.0f;
}

float pressure_to_sealevel_temp(float pressure_hPa, float altitude_m, float temperature_C)
{
    return pressure_hPa * powf(1.0f -
    		(altitude_m / (44330.0f + 0.0065f * temperature_C)), -5.255f);
}

