/*
 * dust_sensor.c
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#include "dust_sensor.h"
#include "dht11.h"  /* potrzebny nam delay_us */


extern ADC_HandleTypeDef hadc1;

int Filter(int m)
{
    static int flag_first = 0;
    static int _buff[10];
    static int sum = 0;
    const int _buff_max = 10;
    int i;

    if (flag_first == 0)
    {
        flag_first = 1;
        sum = 0;
        for (i = 0; i < _buff_max; i++)
        {
            _buff[i] = m;
            sum += _buff[i];
        }
        return m;
    }
    else
    {
        sum -= _buff[0];
        for (i = 0; i < (_buff_max - 1); i++)
        {
            _buff[i] = _buff[i + 1];
        }
        _buff[9] = m;
        sum += _buff[9];

        i = sum / 10;
        return i;
    }
}

float ReadDustSensor(void)
{
    uint32_t adcValue;
    float voltage;
    HAL_GPIO_WritePin(GP_DIODE_GPIO_Port, GP_DIODE_Pin, GPIO_PIN_SET);
    delay_us(280);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);


    adcValue = HAL_ADC_GetValue(&hadc1);
    adcValue = (uint32_t)Filter((int)adcValue);


    HAL_GPIO_WritePin(GP_DIODE_GPIO_Port, GP_DIODE_Pin, GPIO_PIN_RESET);
    HAL_ADC_Stop(&hadc1);


    voltage = ((float)adcValue * 3300.0f) / 4095.0f;
    return voltage;
}

float VoltageToDustDensity(float voltage)
{
    float density = 0;
    voltage *= 11;

    if (voltage >= 400.0f)
        density = (voltage - 400.0f) * 0.2f;
    else
        density = 0;

    return density;
}
