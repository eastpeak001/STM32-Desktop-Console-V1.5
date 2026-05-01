#ifndef __SENSORS_H
#define __SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void Sensors_Init(void);
void Sensors_Task(void);
uint8_t Sensors_GetTempHumi(int16_t *temperature_x10, uint16_t *humidity_x10);
uint16_t Sensors_ReadLightRaw(void);
const char *Sensors_GetLightLevel(uint16_t rawValue);

#ifdef __cplusplus
}
#endif

#endif /* __SENSORS_H */
