#include "buzzer.h"

#define BUZZER_PORT    GPIOB
#define BUZZER_PIN     GPIO_PIN_15
#define BUZZER_ACTION_BEEP_MS    80U

static uint8_t buzzerActive;
static uint8_t buzzerEnabled;
static uint32_t buzzerStopTick;

void Buzzer_Init(void)
{
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
  buzzerActive = 0;
  buzzerEnabled = 1;
  buzzerStopTick = 0;
}

void Buzzer_Beep(uint32_t durationMs)
{
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
  buzzerActive = 1;
  buzzerStopTick = HAL_GetTick() + durationMs;
}

void Buzzer_SetEnabled(uint8_t enabled)
{
  buzzerEnabled = (enabled != 0U) ? 1U : 0U;
}

uint8_t Buzzer_IsEnabled(void)
{
  return buzzerEnabled;
}

void Buzzer_BeepOnAction(void)
{
  if (buzzerEnabled != 0U) {
    Buzzer_Beep(BUZZER_ACTION_BEEP_MS);
  }
}

void Buzzer_Task(void)
{
  if (buzzerActive != 0U) {
    if ((int32_t)(HAL_GetTick() - buzzerStopTick) >= 0) {
      HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
      buzzerActive = 0;
    }
  }
}
