/*
 * bmp280.h
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include "main.h"

#define BMP280_ADDR (0x76 << 1)

void BMP_Init(void); //inicjalizacja czujnika
void BMP_ReadCalibration(void); //zczytaj zmienne kalibrayjne
void BMP_StartMeasurement(void); //wykonaj pomiar
float BMP_ReadTemperature(void); //zczytaj temperature
float BMP_ReadPressure(void);//zczytaj cisnienie
float pressure_to_sealevel(float pressure_hPa, float altitude_m,float temperature_C); //przelicz cisnienie lokalne na ten poziom morza
uint8_t BMP_Read8(uint8_t reg); //f. zczytaj dane
void BMP_Write8(uint8_t reg, uint8_t value); //f. zapisz dane

#endif /* INC_BMP280_H_ */
