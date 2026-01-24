/*
 * dht11.c
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */
#include "dht11.h"

extern TIM_HandleTypeDef htim1;

uint8_t DHT11_Data[5];

void delay_us(uint16_t us)
{
    uint16_t start = __HAL_TIM_GET_COUNTER(&htim1);
    while ((__HAL_TIM_GET_COUNTER(&htim1) - start) < us);
}

void DHT11_Pin_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

void DHT11_Pin_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

int DHT11_Read_Data(void)
{
    uint16_t timeout = 0;
    int i, j;

    /* Start communication */
    DHT11_Pin_Output();
    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
    HAL_Delay(18);
    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
    delay_us(20);
    DHT11_Pin_Input();

    /* Wait for response - LOW (80 us) */
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET)
    {
        timeout++;
        if (timeout > 1000) return 0;
    }

    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_RESET)
    {
        timeout++;
        if (timeout > 1000) return 0;
    }

    /* Read data */
    for (i = 0; i < 5; i++)
    {
        uint8_t wartosc = 0;
        DHT11_Data[i] = 0;

        for (j = 0; j < 8; j++)
        {
            uint8_t bit = 0;
            uint16_t dlugosc = 0;
            uint16_t start;

            while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET);
            while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_RESET);

            start = __HAL_TIM_GET_COUNTER(&htim1);
            while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET);
            dlugosc = __HAL_TIM_GET_COUNTER(&htim1) - start;

            if (dlugosc < 40)
                bit = 0;
            else
                bit = 1;

            if (dlugosc > 99)
                return 0;

            wartosc |= (bit << (7 - j));
        }
        DHT11_Data[i] = wartosc;
    }

    /* Checksum verification */
    if (DHT11_Data[4] == (uint8_t)(DHT11_Data[0] + DHT11_Data[1] + DHT11_Data[2] + DHT11_Data[3]))
        return 1;
    else
        return 0;
}

