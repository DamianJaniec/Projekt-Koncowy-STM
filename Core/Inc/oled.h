/*
 * oled.h
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "main.h"

/* Exported functions */
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawLetter(uint8_t page, uint8_t startColumn, const uint8_t letter[]);
void OLED_DrawCol(uint8_t page, uint8_t startColumn, uint8_t letter);
void OLED_temp(void);
void OLED_pressure_Pa(float pressure, int x);
void OLED_dust(void);
static const uint8_t* GetAirQualityEmoji16(float pm25);
void OLED_DrawAirQualityEmoji16(void);

#endif /* INC_OLED_H_ */
