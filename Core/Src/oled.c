/*
 * oled.c - IMPROVED VERSION with Air Quality
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
static void OLED_DrawHLine(uint8_t page, uint8_t startCol, uint8_t length);
static void OLED_DrawString(uint8_t page, uint8_t col, const char* str);
static int CharToFontIndex(char c);
static const char* GetAirQualityText(float pm25);

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

/* Draw horizontal line */
static void OLED_DrawHLine(uint8_t page, uint8_t startCol, uint8_t length)
{
    uint8_t line = 0xFF;
    uint8_t i;

    OLED_WriteCommand(0xB0 + page);
    OLED_WriteCommand(0x02 + (startCol & 0x0F));
    OLED_WriteCommand(0x10 + (startCol >> 4));

    for (i = 0; i < length; i++)
    {
        OLED_WriteData(&line, 1);
    }
}

/* Helper function to convert char to font index */
static int CharToFontIndex(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 10 + 2*(c - 'A');
    if (c >= 'a' && c <= 'z') return 11 + 2*(c - 'a');
    if (c == ' ') return 62;
    if (c == '.') return 62; /* Use space for dot placeholder */
    if (c == ':') return 62;
    return 62; /* Default to space */
}

/* Get air quality text based on PM2.5 density */
static const char* GetAirQualityText(float pm25)
{
    if (pm25 <= 35.0f)
        return "DOSKONALA         ";          /* Excellent - 9 chars */
    else if (pm25 <= 75.0f)
        return "UMIARKOWANY       ";            /* Average - 7 chars */
    else if (pm25 <= 115.0f)
        return "LEKKIE ZANIECZYSZ ";  /* Light pollution - 17 chars */
    else if (pm25 <= 150.0f)
        return "SREDNIE ZANIECZYSZ";        /* Moderate pollution - 11 chars */
    else if (pm25 <= 250.0f)
        return "CIEZKIE ZANIECZYSZ"; /* Heavy pollution - 18 chars */
    else
        return "POWAZNE ZANIECZYSZ"; /* Serious pollution - 18 chars */
}

/* Draw string of characters */
static void OLED_DrawString(uint8_t page, uint8_t col, const char* str)
{
    uint8_t pos = col;
    while (*str && pos < 120) /* Prevent overflow */
    {
        int idx = CharToFontIndex(*str);
        OLED_DrawLetter(page, pos, font8x8[idx]);
        pos += 8;
        str++;
    }
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

/* IMPROVED: Temperature and Humidity with better formatting */
void OLED_temp(void)
{
    uint8_t tempD, tempL, tempD_dec;
    uint8_t humD, humL;

    /* === TEMPERATURE SECTION (Page 0) === */
    /* Label "T:" */
    OLED_DrawLetter(0, 0, font8x8[48]);   /* T */
    OLED_DrawCol(0, 8, 0b01100000);       /* : */
    OLED_DrawCol(0, 9, 0b01100000);

    /* Temperature value */
    tempD = (uint8_t)(DHT11_Data[2] / 10);
    tempL = (uint8_t)(DHT11_Data[2] - (tempD * 10));

    OLED_DrawLetter(0, 16, font8x8[tempD]);
    OLED_DrawLetter(0, 24, font8x8[tempL]);

    /* Decimal point */
    OLED_DrawCol(0, 32, 0b11000000);
    OLED_DrawCol(0, 33, 0b11000000);

    /* Decimal part */
    tempD_dec = (uint8_t)(DHT11_Data[3] / 10);
    OLED_DrawLetter(0, 34, font8x8[tempD_dec]);

    /* Degree symbol */
    OLED_DrawCol(0, 42, 0b00000110);
    OLED_DrawCol(0, 43, 0b00000110);

    /* "C" */
    OLED_DrawLetter(0, 46, font8x8[14]);  /* C */

    /* === HUMIDITY SECTION (Page 0, right side) === */
    /* Label "H:" */
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

/* IMPROVED: Pressure with labels PM/PL */
void OLED_pressure_Pa(float pressure, int x)
{
    int p = (int)pressure;
    int digits[6];
    int i;
    uint8_t page = (x == 0) ? 2 : 4; /* Page 2 for PM, Page 4 for PL */

    /* Extract digits */
    for (i = 5; i >= 0; i--)
    {
        digits[i] = p % 10;
        p /= 10;
    }

    /* Label */
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

    /* Display pressure value (first 5 digits) */
    for (i = 0; i < 4; i++)
    {
        OLED_DrawLetter(page, (uint8_t)(24 + i * 8), font8x8[digits[i]]);
    }

    /* "Pa" unit */
    OLED_DrawLetter(page, 64, font8x8[40]);   /* P */
    OLED_DrawLetter(page, 72, font8x8[11]);   /* a */
}

/* IMPROVED: Dust sensor with air quality evaluation */
void OLED_dust(void)
{
    int dustInt = (int)dustDensity;
    int d1 = dustInt / 100;
    int d2 = (dustInt / 10) % 10;
    int d3 = dustInt % 10;
    const char* qualityText;


    OLED_DrawAirQualityEmoji();
    /* === PM2.5 VALUE (Page 6) === */
    /* Label "PM:" */
    OLED_DrawLetter(6, 0, font8x8[40]);   /* P */
    OLED_DrawLetter(6, 8, font8x8[34]);   /* M */
    OLED_DrawCol(6, 16, 0b01100000);      /* : */
    OLED_DrawCol(6, 17, 0b01100000);

    /* Value */
    OLED_DrawLetter(6, 24, font8x8[d1]);
    OLED_DrawLetter(6, 32, font8x8[d2]);
    OLED_DrawLetter(6, 40, font8x8[d3]);

    /* Unit "ug/m3" - simplified as "ug" only to avoid artifacts */
    OLED_DrawLetter(6, 56, font8x8[51]);  /* u */
    OLED_DrawLetter(6, 64, font8x8[23]);  /* g */

    /* === AIR QUALITY TEXT (Page 7) === */
    qualityText = GetAirQualityText(dustDensity);
    OLED_DrawString(7, 0, qualityText);
}

static const uint8_t* GetAirQualityEmoji(float pm25)
{
    if (pm25 <= 35.0f)
        return emoji_excellent;     /* :D */
    else if (pm25 <= 75.0f)
        return emoji_good;          /* :) */
    else if (pm25 <= 115.0f)
        return emoji_moderate;      /* :| */
    else if (pm25 <= 150.0f)
        return emoji_poor;          /* :( */
    else if (pm25 <= 250.0f)
        return emoji_bad;           /* :C */
    else
        return emoji_hazardous;     /* maska */
}

void OLED_DrawAirQualityEmoji(void)
{
    const uint8_t* emoji = GetAirQualityEmoji(dustDensity);

    /* Page 0, Column 120 = prawy górny róg (128-8=120) */
    OLED_DrawLetter(0, 120, emoji);
}
