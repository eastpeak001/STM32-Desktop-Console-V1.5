#ifndef __AHT20_H
#define __AHT20_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define AHT20_I2C_ADDR       (0x38 << 1)
#define AHT20_MEASURE_MS     80U

typedef struct {
  int16_t temperature_x10;
  uint16_t humidity_x10;
} AHT20_Data_t;

HAL_StatusTypeDef AHT20_Init(void);
HAL_StatusTypeDef AHT20_StartMeasurement(void);
HAL_StatusTypeDef AHT20_ReadMeasurement(AHT20_Data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __AHT20_H */
