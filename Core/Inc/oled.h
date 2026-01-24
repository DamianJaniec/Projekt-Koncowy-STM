/*
 * oled.h
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "main.h"

void OLED_CS(uint8_t state); //Ustawienie pinu CS
void OLED_DC(uint8_t state); //Ustawienie pinu DC
void OLED_RESET(void); //Ustawienie pinu Reset
void OLED_WriteCommand(uint8_t cmd); //Wyslij komende
void OLED_WriteData(uint8_t *data, uint16_t size); //Wyslij Dane
void OLED_DrawLetter(uint8_t page, uint8_t startColumn, const uint8_t letter[]); //narysuj litere
void OLED_DrawCol(uint8_t page, uint8_t startColumn, uint8_t letter); //narysuj kolumne
void OLED_DrawString(uint8_t page, uint8_t col, const char* str); //Narysuj Tekst
int CharToFontIndex(char c); //Przetlumacz ciag znakow na indeksy naszej czcionki

void OLED_Init(void); //inicjalizacja
void OLED_Clear(void); //czyszczenie danych z ramu OLED
void OLED_temp(void); //wyswietl segment z temperatura
void OLED_pressure_Pa(float pressure, int x); //wyswietl segment z cisnieniem
void OLED_dust(void); //wyswietl segment z zanieczyszczeniem
const uint8_t* GetAirQualityEmoji16(float pm25); //wyswietl emotkion
void OLED_DrawAirQualityEmoji16(void); //wyswietl tekst z jakoscia powietrza
const char* GetAirQualityText(float pm25); //zwroc tekst z jakoscia powietrza do wyswietlenia

#endif /* INC_OLED_H_ */
