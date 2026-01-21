/*
 * dust_sensor.c
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#include "dust_sensor.h"
#include "dht11.h"  /* for delay_us */

/* External handles */
extern ADC_HandleTypeDef hadc1;

/* Private functions */

static int Filter(int m)
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

/* Public functions */

float ReadDustSensor(void)
{
    uint32_t adcValue;
    float voltage;

    /* Turn on sensor LED */
    HAL_GPIO_WritePin(GP_DIODE_GPIO_Port, GP_DIODE_Pin, GPIO_PIN_SET);

    delay_us(280);

    /* Start conversion */
    HAL_ADC_Start(&hadc1);

    /* Wait for completion */
    HAL_ADC_PollForConversion(&hadc1, 10);

    /* Read result */
    adcValue = HAL_ADC_GetValue(&hadc1);
    adcValue = (uint32_t)Filter((int)adcValue);

    /* Turn off LED */
    HAL_GPIO_WritePin(GP_DIODE_GPIO_Port, GP_DIODE_Pin, GPIO_PIN_RESET);

    /* Stop ADC */
    HAL_ADC_Stop(&hadc1);

    /* Convert to voltage */
    voltage = ((float)adcValue * 3300.0f) / 4095.0f;

    return voltage;
}

float VoltageToDustDensity(float voltage)
{
    float density = 0;

    voltage *= 11;

    if (voltage >= 400.0f)
    {
        density = (voltage - 400.0f) * 0.2f;
    }
    else
    {
        density = 0;
    }

    return density;
}

float ReadDustSensor_Avg(int samples)
{
    float sum = 0;
    int i;

    for (i = 0; i < samples; i++)
    {
        sum += ReadDustSensor();
    }

    return sum / (float)samples;
}
