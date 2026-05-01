#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void Buzzer_Init(void);
void Buzzer_Beep(uint32_t durationMs);
void Buzzer_SetEnabled(uint8_t enabled);
uint8_t Buzzer_IsEnabled(void);
void Buzzer_BeepOnAction(void);
void Buzzer_Task(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H */
