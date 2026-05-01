#include "aht20.h"
#include "i2c.h"

#define AHT20_TIMEOUT_MS     20U

HAL_StatusTypeDef AHT20_Init(void)
{
  uint8_t status = 0;
  uint8_t initCmd[3] = {0xBE, 0x08, 0x00};

  if (HAL_I2C_IsDeviceReady(&hi2c1, AHT20_I2C_ADDR, 2, AHT20_TIMEOUT_MS) != HAL_OK) {
    return HAL_ERROR;
  }

  if (HAL_I2C_Master_Receive(&hi2c1, AHT20_I2C_ADDR, &status, 1, AHT20_TIMEOUT_MS) != HAL_OK) {
    return HAL_ERROR;
  }

  if ((status & 0x08U) == 0U) {
    if (HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_ADDR, initCmd, sizeof(initCmd), AHT20_TIMEOUT_MS) != HAL_OK) {
      return HAL_ERROR;
    }
    HAL_Delay(10);
  }

  return HAL_OK;
}

HAL_StatusTypeDef AHT20_StartMeasurement(void)
{
  uint8_t measureCmd[3] = {0xAC, 0x33, 0x00};

  return HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_ADDR, measureCmd, sizeof(measureCmd), AHT20_TIMEOUT_MS);
}

HAL_StatusTypeDef AHT20_ReadMeasurement(AHT20_Data_t *data)
{
  uint8_t rxData[6];
  uint32_t humidityRaw;
  uint32_t temperatureRaw;

  if (data == NULL) {
    return HAL_ERROR;
  }

  if (HAL_I2C_Master_Receive(&hi2c1, AHT20_I2C_ADDR, rxData, sizeof(rxData), AHT20_TIMEOUT_MS) != HAL_OK) {
    return HAL_ERROR;
  }

  if ((rxData[0] & 0x80U) != 0U) {
    return HAL_BUSY;
  }

  humidityRaw = ((uint32_t)rxData[1] << 12) |
                ((uint32_t)rxData[2] << 4) |
                ((uint32_t)rxData[3] >> 4);

  temperatureRaw = (((uint32_t)rxData[3] & 0x0FU) << 16) |
                   ((uint32_t)rxData[4] << 8) |
                   ((uint32_t)rxData[5]);

  data->humidity_x10 = (uint16_t)((humidityRaw * 1000U) / 1048576U);
  data->temperature_x10 = (int16_t)(((temperatureRaw * 2000U) / 1048576U) - 500);

  return HAL_OK;
}
