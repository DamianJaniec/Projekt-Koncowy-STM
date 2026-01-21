/*
 * dust_sensor.h
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#ifndef INC_DUST_SENSOR_H_
#define INC_DUST_SENSOR_H_

#include "main.h"

/* Exported functions */
float ReadDustSensor(void);
float VoltageToDustDensity(float voltage);
float ReadDustSensor_Avg(int samples);

#endif /* INC_DUST_SENSOR_H_ */
