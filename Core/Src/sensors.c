#include "sensors.h"
#include "adc.h"
#include "aht20.h"

#define SENSOR_AHT20_PERIOD_MS    1000U
#define SENSOR_ADC_TIMEOUT_MS     10U

typedef enum {
  AHT20_SENSOR_IDLE = 0,
  AHT20_SENSOR_WAIT_RESULT
} AHT20SensorState_t;

static AHT20SensorState_t aht20State;
static uint32_t aht20LastStartTick;
static AHT20_Data_t aht20Data;
static uint8_t aht20DataValid;
static uint8_t aht20Ready;

void Sensors_Init(void)
{
  aht20State = AHT20_SENSOR_IDLE;
  aht20LastStartTick = HAL_GetTick() - SENSOR_AHT20_PERIOD_MS;
  aht20Data.temperature_x10 = 0;
  aht20Data.humidity_x10 = 0;
  aht20DataValid = 0;
  aht20Ready = (AHT20_Init() == HAL_OK) ? 1U : 0U;
}

void Sensors_Task(void)
{
  uint32_t now = HAL_GetTick();

  if (aht20Ready == 0U) {
    if ((now - aht20LastStartTick) >= SENSOR_AHT20_PERIOD_MS) {
      aht20LastStartTick = now;
      aht20Ready = (AHT20_Init() == HAL_OK) ? 1U : 0U;
    }
    return;
  }

  if (aht20State == AHT20_SENSOR_IDLE) {
    if ((now - aht20LastStartTick) >= SENSOR_AHT20_PERIOD_MS) {
      if (AHT20_StartMeasurement() == HAL_OK) {
        aht20LastStartTick = now;
        aht20State = AHT20_SENSOR_WAIT_RESULT;
      } else {
        aht20DataValid = 0;
        aht20Ready = 0;
        aht20LastStartTick = now;
      }
    }
  } else {
    if ((now - aht20LastStartTick) >= AHT20_MEASURE_MS) {
      if (AHT20_ReadMeasurement(&aht20Data) == HAL_OK) {
        aht20DataValid = 1;
      } else {
        aht20DataValid = 0;
      }
      aht20State = AHT20_SENSOR_IDLE;
    }
  }
}

uint8_t Sensors_GetTempHumi(int16_t *temperature_x10, uint16_t *humidity_x10)
{
  if ((temperature_x10 == NULL) || (humidity_x10 == NULL) || (aht20DataValid == 0U)) {
    return 0;
  }

  *temperature_x10 = aht20Data.temperature_x10;
  *humidity_x10 = aht20Data.humidity_x10;
  return 1;
}

uint16_t Sensors_ReadLightRaw(void)
{
  uint16_t rawValue = 0;

  if (HAL_ADC_Start(&hadc1) != HAL_OK) {
    return 0;
  }

  if (HAL_ADC_PollForConversion(&hadc1, SENSOR_ADC_TIMEOUT_MS) == HAL_OK) {
    rawValue = (uint16_t)HAL_ADC_GetValue(&hadc1);
  }

  HAL_ADC_Stop(&hadc1);
  return rawValue;
}

const char *Sensors_GetLightLevel(uint16_t rawValue)
{
  if (rawValue <= 1200U) {
    return "Bright";
  }

  if (rawValue <= 3000U) {
    return "Normal";
  }

  return "Dark";
}
