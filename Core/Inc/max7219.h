#ifndef __MAX7219_H
#define __MAX7219_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum {
  ICON_SMILE = 0,
  ICON_TEMP,
  ICON_LIGHT,
  ICON_LED,
  ICON_BUZZER,
  ICON_BLUETOOTH,
  ICON_MOTION,
  ICON_HISTORY,
  ICON_ERROR
} Max7219Icon_t;

void MAX7219_Init(void);
void MAX7219_Clear(void);
void MAX7219_SetBrightness(uint8_t level);
void MAX7219_DisplayIcon(Max7219Icon_t icon);

#ifdef __cplusplus
}
#endif

#endif /* __MAX7219_H */
