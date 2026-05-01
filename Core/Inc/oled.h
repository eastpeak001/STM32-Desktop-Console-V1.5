#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define OLED_WIDTH        128
#define OLED_HEIGHT       64
#define OLED_I2C_ADDR     (0x3C << 1)

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void OLED_UpdateScreen(void);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
