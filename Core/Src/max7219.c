#include "max7219.h"

#define MAX7219_DIN_PORT             GPIOB
#define MAX7219_DIN_PIN              GPIO_PIN_8
#define MAX7219_CS_PORT              GPIOB
#define MAX7219_CS_PIN               GPIO_PIN_9
#define MAX7219_CLK_PORT             GPIOB
#define MAX7219_CLK_PIN              GPIO_PIN_10

#define MAX7219_REG_NOOP             0x00
#define MAX7219_REG_DIGIT0           0x01
#define MAX7219_REG_DECODE_MODE      0x09
#define MAX7219_REG_INTENSITY        0x0A
#define MAX7219_REG_SCAN_LIMIT       0x0B
#define MAX7219_REG_SHUTDOWN         0x0C
#define MAX7219_REG_DISPLAY_TEST     0x0F

#define MAX7219_DEFAULT_BRIGHTNESS   0x07

static const uint8_t iconTable[][8] = {
  {0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C}, /* ICON_SMILE */
  {0x18, 0x24, 0x24, 0x24, 0x24, 0x42, 0x42, 0x3C}, /* ICON_TEMP */
  {0x18, 0x5A, 0x3C, 0x7E, 0x3C, 0x5A, 0x18, 0x00}, /* ICON_LIGHT */
  {0x18, 0x18, 0x7E, 0x7E, 0x3C, 0x3C, 0x18, 0x18}, /* ICON_LED */
  {0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0x18, 0x24, 0x42}, /* ICON_BUZZER */
  {0x18, 0x14, 0x12, 0x7F, 0x12, 0x14, 0x18, 0x00}, /* ICON_BLUETOOTH */
  {0x08, 0x1C, 0x2A, 0x49, 0x08, 0x2A, 0x1C, 0x08}, /* ICON_MOTION */
  {0x7E, 0x42, 0x5A, 0x42, 0x5A, 0x42, 0x7E, 0x00}, /* ICON_HISTORY */
  {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}  /* ICON_ERROR */
};

static void MAX7219_WriteByte(uint8_t data)
{
  for (uint8_t i = 0; i < 8U; i++) {
    HAL_GPIO_WritePin(MAX7219_CLK_PORT, MAX7219_CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MAX7219_DIN_PORT, MAX7219_DIN_PIN,
                      ((data & 0x80U) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MAX7219_CLK_PORT, MAX7219_CLK_PIN, GPIO_PIN_SET);
    data <<= 1U;
  }
}

static void MAX7219_WriteRegister(uint8_t address, uint8_t data)
{
  HAL_GPIO_WritePin(MAX7219_CS_PORT, MAX7219_CS_PIN, GPIO_PIN_RESET);
  MAX7219_WriteByte(address);
  MAX7219_WriteByte(data);
  HAL_GPIO_WritePin(MAX7219_CS_PORT, MAX7219_CS_PIN, GPIO_PIN_SET);
}

void MAX7219_Init(void)
{
  HAL_GPIO_WritePin(MAX7219_CS_PORT, MAX7219_CS_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(MAX7219_CLK_PORT, MAX7219_CLK_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MAX7219_DIN_PORT, MAX7219_DIN_PIN, GPIO_PIN_RESET);

  MAX7219_WriteRegister(MAX7219_REG_DISPLAY_TEST, 0x00);
  MAX7219_WriteRegister(MAX7219_REG_DECODE_MODE, 0x00);
  MAX7219_WriteRegister(MAX7219_REG_SCAN_LIMIT, 0x07);
  MAX7219_SetBrightness(MAX7219_DEFAULT_BRIGHTNESS);
  MAX7219_WriteRegister(MAX7219_REG_SHUTDOWN, 0x01);
  MAX7219_Clear();
}

void MAX7219_Clear(void)
{
  for (uint8_t row = 0; row < 8U; row++) {
    MAX7219_WriteRegister((uint8_t)(MAX7219_REG_DIGIT0 + row), 0x00);
  }
}

void MAX7219_SetBrightness(uint8_t level)
{
  if (level > 0x0FU) {
    level = 0x0F;
  }

  MAX7219_WriteRegister(MAX7219_REG_INTENSITY, level);
}

void MAX7219_DisplayIcon(Max7219Icon_t icon)
{
  if (icon > ICON_ERROR) {
    icon = ICON_ERROR;
  }

  for (uint8_t row = 0; row < 8U; row++) {
    MAX7219_WriteRegister((uint8_t)(MAX7219_REG_DIGIT0 + row), iconTable[icon][row]);
  }
}
