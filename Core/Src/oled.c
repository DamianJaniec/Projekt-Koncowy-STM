/*
 * oled.c - IMPROVED VERSION with Air Quality
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */
#include "oled.h"
#include "font.h"
#include "dht11.h"

extern SPI_HandleTypeDef hspi1;
extern float dustDensity;

void OLED_CS(uint8_t state)
{
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void OLED_DC(uint8_t state)
{
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void OLED_RESET(void)
{
    HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}
void OLED_WriteCommand(uint8_t cmd)
{
    OLED_DC(0);
    OLED_CS(0);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    OLED_CS(1);
}
void OLED_WriteData(uint8_t *data, uint16_t size)
{
    OLED_DC(1);
    OLED_CS(0);
    HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY);
    OLED_CS(1);
}
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
    for (i = 0; i < 8; i++)
    {
        temp[i] = letter[i];
    }

    OLED_WriteCommand(0xB0 + page);
    OLED_WriteCommand(0x02 + (startColumn & 0x0F));
    OLED_WriteCommand(0x10 + (startColumn >> 4));
    OLED_WriteData(temp, 8);
}
int CharToFontIndex(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 10 + 2*(c - 'A');
    if (c >= 'a' && c <= 'z') return 11 + 2*(c - 'a');
    if (c == ' ') return 62;
    if (c == '.') return 62;
    if (c == ':') return 62;
    return 62;
}
void OLED_DrawString(uint8_t page, uint8_t col, const char* str)
{
    uint8_t pos = col;
    while (*str && pos < 120)
    {
        int idx = CharToFontIndex(*str);
        OLED_DrawLetter(page, pos, font8x8[idx]);
        pos += 8;
        str++;
    }
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
    uint8_t tempD, tempL, tempD_dec;
    uint8_t humD, humL;

    /*"T:" */
    OLED_DrawLetter(0, 0, font8x8[48]);   /* T */
    OLED_DrawCol(0, 8, 0b01100000);       /* : */
    OLED_DrawCol(0, 9, 0b01100000);

    /* temp */
    tempD = (uint8_t)(DHT11_Data[2] / 10);
    tempL = (uint8_t)(DHT11_Data[2] - (tempD * 10));

    OLED_DrawLetter(0, 16, font8x8[tempD]);
    OLED_DrawLetter(0, 24, font8x8[tempL]);

    /* kropka*/
    OLED_DrawCol(0, 32, 0b11000000);
    OLED_DrawCol(0, 33, 0b11000000);

    tempD_dec = (uint8_t)(DHT11_Data[3] / 10);
    OLED_DrawLetter(0, 34, font8x8[tempD_dec]);

    /* symbol stopnia */
    OLED_DrawCol(0, 42, 0b00000110);
    OLED_DrawCol(0, 43, 0b00000110);

    /* "C" */
    OLED_DrawLetter(0, 46, font8x8[14]);  /* C */
    /* "H:" */
    OLED_DrawLetter(0, 64, font8x8[24]);  /* H */
    OLED_DrawCol(0, 72, 0b01100000);      /* : */
    OLED_DrawCol(0, 73, 0b01100000);

    /* Humidity value */
    humD = (uint8_t)(DHT11_Data[0] / 10);
    humL = (uint8_t)(DHT11_Data[0] - humD * 10);

    OLED_DrawLetter(0, 80, font8x8[humD]);
    OLED_DrawLetter(0, 88, font8x8[humL]);

    /* "%" */
    OLED_DrawCol(0, 96, 0b01001001);
    OLED_DrawCol(0, 97, 0b00100100);
    OLED_DrawCol(0, 98, 0b00010010);
    OLED_DrawCol(0, 99, 0b01001001);
}
void OLED_pressure_Pa(float pressure, int x)
{
    int p = (int)pressure;
    int digits[6];
    int i;
    uint8_t page = (x == 0) ? 2 : 4; /* strona 2  PM, strona 4 PL */

    for (i = 5; i >= 0; i--)
    {
        digits[i] = p % 10;
        p /= 10;
    }
    if (x == 0)
    {
        /* "PM:" for Poziom Morza (Sea Level) */
        OLED_DrawLetter(page, 0, font8x8[40]);  /* P */
        OLED_DrawLetter(page, 8, font8x8[34]);  /* M */
        OLED_DrawCol(page, 16, 0b01100000);     /* : */
        OLED_DrawCol(page, 17, 0b01100000);
    }
    else
    {
        /* "PL:" for Poziom Lokalny (Local) */
        OLED_DrawLetter(page, 0, font8x8[40]);  /* P */
        OLED_DrawLetter(page, 8, font8x8[32]);  /* L */
        OLED_DrawCol(page, 16, 0b01100000);     /* : */
        OLED_DrawCol(page, 17, 0b01100000);
    }

    for (i = 0; i < 4; i++)
    {
        OLED_DrawLetter(page, (uint8_t)(24 + i * 8), font8x8[digits[i]]);
    }

    OLED_DrawLetter(page, 64, font8x8[40]);   /* P */
    OLED_DrawLetter(page, 72, font8x8[11]);   /* a */
}
void OLED_dust(void)
{
    int dustInt = (int)dustDensity;
    int d1 = dustInt / 100;
    int d2 = (dustInt / 10) % 10;
    int d3 = dustInt % 10;
    const char* qualityText;


    OLED_DrawAirQualityEmoji16();

    /* etykieta "PM:" */
    OLED_DrawLetter(6, 0, font8x8[40]);   /* P */
    OLED_DrawLetter(6, 8, font8x8[34]);   /* M */
    OLED_DrawCol(6, 16, 0b01100000);      /* : */
    OLED_DrawCol(6, 17, 0b01100000);

    /* wartosc*/
    OLED_DrawLetter(6, 24, font8x8[d1]);
    OLED_DrawLetter(6, 32, font8x8[d2]);
    OLED_DrawLetter(6, 40, font8x8[d3]);

    /* jednostka "ug/m3" - uproszczone "ug"*/
    OLED_DrawLetter(6, 56, font8x8[51]);  /* u */
    OLED_DrawLetter(6, 64, font8x8[23]);  /* g */

    qualityText = GetAirQualityText(dustDensity);
    OLED_DrawString(7, 0, qualityText);
}

const uint8_t* GetAirQualityEmoji16(float pm25)
{
    if (pm25 <= 35.0f)
        return emoji16_excellent;   /* :D */
    else if (pm25 <= 75.0f)
        return emoji16_good;        /* :) */
    else if (pm25 <= 115.0f)
        return emoji16_moderate;    /* :| */
    else if (pm25 <= 150.0f)
        return emoji16_poor;        /* :( */
    else if (pm25 <= 250.0f)
        return emoji16_bad;         /* :C */
    else
        return emoji16_hazardous;   /* maska */
}

void OLED_DrawAirQualityEmoji16(void)
{
    const uint8_t* emoji = GetAirQualityEmoji16(dustDensity);
    uint8_t topPart[16];
    uint8_t bottomPart[16];
    int i;
    /* Kopiuj dane do buforów */
    for (i = 0; i < 16; i++)
    {
        topPart[i] = emoji[i];
        bottomPart[i] = emoji[16 + i];
    }
    /* Górna połowa - Page 0, kolumny 112-127 */
    OLED_WriteCommand(0xB0 + 0);           /* Page 0 */
    OLED_WriteCommand(0x02 + (112 & 0x0F)); /* Column 112 low nibble */
    OLED_WriteCommand(0x10 + (112 >> 4));   /* Column 112 high nibble */
    OLED_WriteData(topPart, 16);

    /* Dolna połowa - Page 1, kolumny 112-127 */
    OLED_WriteCommand(0xB0 + 1);           /* Page 1 */
    OLED_WriteCommand(0x02 + (112 & 0x0F));
    OLED_WriteCommand(0x10 + (112 >> 4));
    OLED_WriteData(bottomPart, 16);
}
const char* GetAirQualityText(float pm25)
{
    if (pm25 <= 35.0f)
        return "DOSKONALA       "; /* Excellent */
    else if (pm25 <= 75.0f)
        return "DOBRA           "; /* Average */
    else if (pm25 <= 115.0f)
        return "UMIARKOWANA     "; /* Light pollution */
    else if (pm25 <= 150.0f)
        return "ZANIECZYSZCZONA "; /* Moderate pollution */
    else if (pm25 <= 250.0f)
        return "BARDZO BRUDNA   "; /* Heavy pollution */
    else
        return "NIEBEZPIECZNA   "; /* Serious pollution */
}

