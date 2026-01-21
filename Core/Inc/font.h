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

extern const uint8_t emoji_excellent[8];    /* DOSKONAŁA - wielki uśmiech :D */
extern const uint8_t emoji_good[8];         /* UMIARKOWANA - uśmiech :) */
extern const uint8_t emoji_moderate[8];     /* LEKKIE ZANIECZYSZ - neutralna :| */
extern const uint8_t emoji_poor[8];         /* ŚREDNIE - lekko smutna :( */
extern const uint8_t emoji_bad[8];          /* CIĘŻKIE - smutna :C */
extern const uint8_t emoji_hazardous[8];    /* POWAŻNE - z maską */

#endif /* INC_FONT_H_ */
