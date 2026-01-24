/*
 * dht11.h
 *
 *  Created on: Jan 2, 2026
 *      Author: Dejmian
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "main.h"

extern uint8_t DHT11_Data[5];

void delay_us(uint16_t us); //opoznienie w mikrosekundach
int DHT11_Read_Data(void); //zczytaj dane
void DHT11_Pin_Output(void); //zmien pin w tryb output
void DHT11_Pin_Input(void); //zmien pin w tryb input

#endif /* INC_DHT11_H_ */
