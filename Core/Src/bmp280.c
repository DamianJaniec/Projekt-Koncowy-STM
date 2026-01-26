/*
 * bmp280.c
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */
#include "bmp280.h"
#include <math.h>


extern I2C_HandleTypeDef hi2c1;


uint16_t dig_T1;
int16_t dig_T2, dig_T3;
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
int32_t t_fine;


uint8_t BMP_Read8(uint8_t reg)
{
    uint8_t value = 0;
    HAL_I2C_Mem_Read(&hi2c1, BMP280_ADDR, reg, 1, &value, 1, HAL_MAX_DELAY);
    return value;
}

void BMP_Write8(uint8_t reg, uint8_t value)
{
    HAL_I2C_Mem_Write(&hi2c1, BMP280_ADDR, reg, 1, &value, 1, HAL_MAX_DELAY);
}

void BMP_ReadCalibration(void)
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
    BMP_Write8(0xF4, 0b01010011);
    /* Oversampling x16 pressure, x2 temperature, SLEEP MODE */
    /* Mode bits [1:0] = 00 (sleep) - czujnik nie mierzy automatycznie */
    HAL_Delay(50);
}
uint8_t BMP_IsMeasuring(void)
{
    return (BMP_Read8(0xF3) & 0x08);
}
void BMP_StartMeasurement(void)
{

    BMP_Write8(0xF4, 0b01010001);  /* x16 pressure, x2 temp, FORCED */
    /* Poczekaj aż zakończy pomiar (~25ms dla x16 oversampling) */
    while (BMP_IsMeasuring());   // czekaj aż skończy
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

float pressure_to_sealevel(float pressure_hPa, float altitude_m, float temperature_C)
{
    const float LAPSE_RATE = 0.0065f;      /* K/m lapse rate */
    const float KELVIN_OFFSET = 273.15f;   /* Celsiusz na Kelvin */
    const float EXPONENT = -5.257f;        /* eksponenta g*M/(R*L) */

    /* obliczanie temp*/
    float temp_kelvin = temperature_C + KELVIN_OFFSET;
    float denominator = temp_kelvin + LAPSE_RATE * altitude_m;
    float numerator = LAPSE_RATE * altitude_m;

    /* P_SL = P × (1 - (0.0065 × h) / (T + 273.15 + 0.0065 × h))^(-5.257) */
    float base = 1.0f - (numerator / denominator);
    float result = pressure_hPa * powf(base, EXPONENT);

    return result;
}

