/*
 * font.h
 *
 *  Created on: Nov 12, 2025
 *      Author: Dejmian
 */

#ifndef INC_FONT_H_
#define INC_FONT_H_

#include "stdint.h"

extern const uint8_t font8x8[][8];

/* Emotikony 16x16 (2x2 znaki) - jakość powietrza */
extern const uint8_t emoji16_excellent[32];   /* :D DOSKONAŁA */
extern const uint8_t emoji16_good[32];        /* :) UMIARKOWANA */
extern const uint8_t emoji16_moderate[32];    /* :| NEUTRALNA */
extern const uint8_t emoji16_poor[32];        /* :( LEKKO SMUTNA */
extern const uint8_t emoji16_bad[32];         /* :C SMUTNA */
extern const uint8_t emoji16_hazardous[32];   /* 😷 Z MASKĄ */

#endif /* INC_FONT_H_ */
