#include "mpu6050.h"
#include "i2c.h"

#define MPU6050_TIMEOUT_MS       20U
#define MPU6050_REG_PWR_MGMT_1   0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_ACCEL_CONFIG 0x1C
#define MPU6050_REG_GYRO_CONFIG  0x1B

static HAL_StatusTypeDef MPU6050_WriteReg(uint8_t reg, uint8_t value)
{
  uint8_t txData[2] = {reg, value};

  return HAL_I2C_Master_Transmit(&hi2c1, MPU6050_I2C_ADDR, txData, sizeof(txData), MPU6050_TIMEOUT_MS);
}

HAL_StatusTypeDef MPU6050_Init(void)
{
  if (HAL_I2C_IsDeviceReady(&hi2c1, MPU6050_I2C_ADDR, 2, MPU6050_TIMEOUT_MS) != HAL_OK) {
    return HAL_ERROR;
  }

  if (MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x00) != HAL_OK) {
    return HAL_ERROR;
  }

  if (MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, 0x00) != HAL_OK) {
    return HAL_ERROR;
  }

  if (MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, 0x00) != HAL_OK) {
    return HAL_ERROR;
  }

  return HAL_OK;
}

HAL_StatusTypeDef MPU6050_ReadAccel(MPU6050_AccelData_t *data)
{
  uint8_t reg = MPU6050_REG_ACCEL_XOUT_H;
  uint8_t rxData[6];

  if (data == NULL) {
    return HAL_ERROR;
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, MPU6050_I2C_ADDR, &reg, 1, MPU6050_TIMEOUT_MS) != HAL_OK) {
    return HAL_ERROR;
  }

  if (HAL_I2C_Master_Receive(&hi2c1, MPU6050_I2C_ADDR, rxData, sizeof(rxData), MPU6050_TIMEOUT_MS) != HAL_OK) {
    return HAL_ERROR;
  }

  data->ax = (int16_t)((uint16_t)rxData[0] << 8 | rxData[1]);
  data->ay = (int16_t)((uint16_t)rxData[2] << 8 | rxData[3]);
  data->az = (int16_t)((uint16_t)rxData[4] << 8 | rxData[5]);

  return HAL_OK;
}
