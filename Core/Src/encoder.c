#include "encoder.h"

#define ENCODER_A_PORT        GPIOA
#define ENCODER_A_PIN         GPIO_PIN_1
#define ENCODER_B_PORT        GPIOA
#define ENCODER_B_PIN         GPIO_PIN_2
#define ENCODER_SW_PORT       GPIOA
#define ENCODER_SW_PIN        GPIO_PIN_3

#define ENCODER_DEBOUNCE_MS   30U

static uint8_t encoderLastState;
static int8_t encoderStepCount;
static int8_t encoderPendingRotation;

static GPIO_PinState buttonLastRawState;
static GPIO_PinState buttonStableState;
static uint32_t buttonLastChangeTick;
static uint8_t buttonPressedFlag;

static uint8_t Encoder_ReadAB(void)
{
  uint8_t a = (HAL_GPIO_ReadPin(ENCODER_A_PORT, ENCODER_A_PIN) == GPIO_PIN_SET) ? 1U : 0U;
  uint8_t b = (HAL_GPIO_ReadPin(ENCODER_B_PORT, ENCODER_B_PIN) == GPIO_PIN_SET) ? 1U : 0U;

  return (uint8_t)((a << 1U) | b);
}

void Encoder_Init(void)
{
  encoderLastState = Encoder_ReadAB();
  encoderStepCount = 0;
  encoderPendingRotation = 0;

  buttonLastRawState = HAL_GPIO_ReadPin(ENCODER_SW_PORT, ENCODER_SW_PIN);
  buttonStableState = buttonLastRawState;
  buttonLastChangeTick = HAL_GetTick();
  buttonPressedFlag = 0;
}

void Encoder_Scan(void)
{
  static const int8_t transitionTable[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
  };

  uint8_t currentState = Encoder_ReadAB();
  uint8_t transition = (uint8_t)((encoderLastState << 2U) | currentState);
  int8_t delta = transitionTable[transition & 0x0FU];

  if (delta != 0) {
    encoderStepCount += delta;

    if (encoderStepCount >= 4) {
      encoderPendingRotation++;
      encoderStepCount = 0;
    } else if (encoderStepCount <= -4) {
      encoderPendingRotation--;
      encoderStepCount = 0;
    }
  }

  encoderLastState = currentState;

  GPIO_PinState rawState = HAL_GPIO_ReadPin(ENCODER_SW_PORT, ENCODER_SW_PIN);
  uint32_t now = HAL_GetTick();

  if (rawState != buttonLastRawState) {
    buttonLastRawState = rawState;
    buttonLastChangeTick = now;
  }

  if ((now - buttonLastChangeTick) >= ENCODER_DEBOUNCE_MS) {
    if (buttonStableState != rawState) {
      buttonStableState = rawState;

      if (buttonStableState == GPIO_PIN_RESET) {
        buttonPressedFlag = 1;
      }
    }
  }
}

EncoderDirection_t Encoder_GetRotation(void)
{
  if (encoderPendingRotation > 0) {
    encoderPendingRotation--;
    return ENCODER_CW;
  }

  if (encoderPendingRotation < 0) {
    encoderPendingRotation++;
    return ENCODER_CCW;
  }

  return ENCODER_NONE;
}

uint8_t Encoder_GetButtonPressed(void)
{
  if (buttonPressedFlag != 0U) {
    buttonPressedFlag = 0U;
    return 1U;
  }

  return 0U;
}
