#ifndef __MPU6050_H
#define __MPU6050_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define MPU6050_I2C_ADDR          (0x68 << 1)
#define MPU6050_SHAKE_THRESHOLD   6000

typedef struct {
  int16_t ax;
  int16_t ay;
  int16_t az;
} MPU6050_AccelData_t;

HAL_StatusTypeDef MPU6050_Init(void);
HAL_StatusTypeDef MPU6050_ReadAccel(MPU6050_AccelData_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __MPU6050_H */
