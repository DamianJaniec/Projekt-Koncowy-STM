/*
 * bmp280.h
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include "main.h"

/* Defines */
#define BMP280_ADDR (0x76 << 1)

/* Exported functions */
void BMP_Init(void);
float BMP_ReadTemperature(void);
float BMP_ReadPressure(void);
float pressure_to_sealevel_temp(float pressure_hPa, float altitude_m, float temperature_C);

#endif /* INC_BMP280_H_ */
