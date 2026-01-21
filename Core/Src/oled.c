/*
 * oled.c
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */
#include "oled.h"
#include "font.h"
#include "dht11.h"

/* External handles */
extern SPI_HandleTypeDef hspi1;
extern float dustDensity;

/* Private function prototypes */
static void OLED_CS(uint8_t state);
static void OLED_DC(uint8_t state);
static void OLED_RESET(void);
static void OLED_WriteCommand(uint8_t cmd);
static void OLED_WriteData(uint8_t *data, uint16_t size);

/* Private functions */

static void OLED_CS(uint8_t state)
{
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void OLED_DC(uint8_t state)
{
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void OLED_RESET(void)
{
    HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

static void OLED_WriteCommand(uint8_t cmd)
{
    OLED_DC(0);
    OLED_CS(0);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    OLED_CS(1);
}

static void OLED_WriteData(uint8_t *data, uint16_t size)
{
    OLED_DC(1);
    OLED_CS(0);
    HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY);
    OLED_CS(1);
}

/* Public functions */

void OLED_Init(void)
{
    OLED_RESET();
    HAL_Delay(100);

    OLED_WriteCommand(0xAE); /* Display OFF */
    OLED_WriteCommand(0xA8); /* Multiplex Ratio */
    OLED_WriteCommand(0x3F); /* 64MUX */
    OLED_WriteCommand(0xD3); /* Display Offset */
    OLED_WriteCommand(0x00); /* no offset */
    OLED_WriteCommand(0x40); /* Start Line = 0 */
    OLED_WriteCommand(0x8D); /* Charge Pump */
    OLED_WriteCommand(0x14); /* enable */
    OLED_WriteCommand(0xAD); /* DC-DC Control */
    OLED_WriteCommand(0x8B); /* Enable DC-DC */
    OLED_WriteCommand(0xA1); /* Segment Remap */
    OLED_WriteCommand(0xC8); /* COM Scan Direction */
    OLED_WriteCommand(0xDA); /* COM Pins config */
    OLED_WriteCommand(0x12); /* for 128x64 */
    OLED_WriteCommand(0x81); /* Contrast */
    OLED_WriteCommand(0x7F); /* medium */
    OLED_WriteCommand(0xA4); /* Resume RAM display */
    OLED_WriteCommand(0xA6); /* Normal (not inverted) */
    OLED_WriteCommand(0xAF); /* Display ON */
}

void OLED_Clear(void)
{
    uint8_t zeros[128] = {0};
    const uint8_t offset = 2;
    uint8_t page;

    for (page = 0; page < 8; page++)
    {
        OLED_WriteCommand(0xB0 + page);
        OLED_WriteCommand(0x00 + (offset & 0x0F));
        OLED_WriteCommand(0x10 + (offset >> 4));
        OLED_WriteData(zeros, 128);
    }
}

void OLED_DrawLetter(uint8_t page, uint8_t startColumn, const uint8_t letter[])
{
    uint8_t temp[8];
    int i;

    /* Copy const data to writable buffer */
    for (i = 0; i < 8; i++)
    {
        temp[i] = letter[i];
    }

    OLED_WriteCommand(0xB0 + page);
    OLED_WriteCommand(0x02 + (startColumn & 0x0F));
    OLED_WriteCommand(0x10 + (startColumn >> 4));
    OLED_WriteData(temp, 8);
}

void OLED_DrawCol(uint8_t page, uint8_t startColumn, uint8_t letter)
{
    OLED_WriteCommand(0xB0 + page);
    OLED_WriteCommand(0x02 + (startColumn & 0x0F));
    OLED_WriteCommand(0x10 + (startColumn >> 4));
    OLED_WriteData(&letter, 1);
}

void OLED_temp(void)
{
    uint8_t tempD, tempL, tempD_dec, tempL_dec;
    uint8_t humD, humL, hum_dec;

    OLED_DrawLetter(0, 0, font8x8[48]);

    tempD = (uint8_t)(DHT11_Data[2] / 10);
    tempL = (uint8_t)(DHT11_Data[2] - (tempD * 10));

    OLED_DrawLetter(0, 8, font8x8[tempD]);
    OLED_DrawLetter(0, 16, font8x8[tempL]);

    tempD_dec = (uint8_t)(DHT11_Data[3] / 10);
    tempL_dec = (uint8_t)(DHT11_Data[3] - (tempD_dec * 10));

    OLED_DrawLetter(0, 24, font8x8[tempL_dec]);

    OLED_DrawCol(0, 23, 0b11000000);
    OLED_DrawCol(0, 24, 0b11000000);
    OLED_DrawCol(0, 7, 0b11011000);

    OLED_DrawLetter(0, 40, font8x8[24]);

    humD = (uint8_t)(DHT11_Data[0] / 10);
    humL = (uint8_t)(DHT11_Data[0] - humD * 10);
    hum_dec = DHT11_Data[1];

    (void)hum_dec; /* Suppress unused warning */

    OLED_DrawLetter(0, 48, font8x8[humD]);
    OLED_DrawLetter(0, 56, font8x8[humL]);
    OLED_DrawLetter(0, 64, font8x8[humL]);

    OLED_DrawCol(0, 64, 0b11000000);
    OLED_DrawCol(0, 48, 0b11011000);
}

void OLED_pressure_Pa(float pressure, int x)
{
    int p = (int)pressure;
    int digits[6];
    int i;

    for (i = 5; i >= 0; i--)
    {
        digits[i] = p % 10;
        p /= 10;
    }

    for (i = 0; i < 4; i++)
    {
        OLED_DrawLetter(1, (uint8_t)(i * 8 + x), font8x8[digits[i]]);
    }

    OLED_DrawLetter(1, (uint8_t)(32 + x), font8x8[40]);
    OLED_DrawLetter(1, (uint8_t)(40 + x), font8x8[25]);
}

void OLED_dust(void)
{
    int dustInt = (int)dustDensity;
    int d1 = dustInt / 100;
    int d2 = (dustInt / 10) % 10;
    int d3 = dustInt % 10;

    OLED_DrawLetter(3, 0, font8x8[40]);
    OLED_DrawLetter(3, 8, font8x8[34]);
    OLED_DrawLetter(3, 16, font8x8[d1]);
    OLED_DrawLetter(3, 24, font8x8[d2]);
    OLED_DrawLetter(3, 32, font8x8[d3]);
}

