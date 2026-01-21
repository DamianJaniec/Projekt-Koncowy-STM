/*
 * dht11.h
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "main.h"

/* Exported variables */
extern uint8_t DHT11_Data[5];

/* Exported functions */
void delay_us(uint16_t us);
int DHT11_Read_Data(void);

#endif /* INC_DHT11_H_ */
