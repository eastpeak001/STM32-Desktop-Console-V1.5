#include "ui.h"
#include "oled.h"
#include <stdio.h>

static const char *menuItems[UI_MENU_COUNT] = {
  "Temp&Humi",
  "Light",
  "LED Control",
  "Beep Mode",
  "Bluetooth",
  "Motion",
  "History"
};

const char *UI_GetMenuItemName(UiMenuItem_t item)
{
  if (item >= UI_MENU_COUNT) {
    return "";
  }

  return menuItems[item];
}

void UI_ShowMainMenu(UiMenuItem_t selectedItem)
{
  char line[18];
  uint8_t firstItem = 0;
  const uint8_t visibleItems = 5;

  if (selectedItem >= visibleItems) {
    firstItem = (uint8_t)(selectedItem - visibleItems + 1U);
  }

  OLED_Clear();
  OLED_ShowString(0, 0, "STM32 Console");

  for (uint8_t row = 0; row < visibleItems; row++) {
    uint8_t i = (uint8_t)(firstItem + row);

    if (i >= UI_MENU_COUNT) {
      break;
    }

    line[0] = (i == (uint8_t)selectedItem) ? '>' : ' ';
    line[1] = ' ';

    uint8_t j = 0;
    const char *name = menuItems[i];
    while ((name[j] != '\0') && (j < sizeof(line) - 3U)) {
      line[j + 2U] = name[j];
      j++;
    }
    line[j + 2U] = '\0';

    OLED_ShowString(0, (uint8_t)(16U + row * 8U), line);
  }

  OLED_UpdateScreen();
}

void UI_ShowPage(UiMenuItem_t page)
{
  OLED_Clear();
  OLED_ShowString(0, 0, UI_GetMenuItemName(page));
  OLED_ShowString(0, 24, "Page Name");
  OLED_ShowString(0, 48, "Press SW Back");
  OLED_UpdateScreen();
}

void UI_ShowTempHumiPage(uint8_t valid, int16_t temperature_x10, uint16_t humidity_x10)
{
  char line[22];
  int16_t tempAbs;

  OLED_Clear();
  OLED_ShowString(0, 0, "Temp&Humi");

  if (valid == 0U) {
    OLED_ShowString(0, 24, "AHT20 ERROR");
  } else {
    tempAbs = (temperature_x10 < 0) ? (int16_t)(-temperature_x10) : temperature_x10;

    snprintf(line, sizeof(line), "Temp: %s%d.%d C",
             (temperature_x10 < 0) ? "-" : "",
             tempAbs / 10,
             tempAbs % 10);
    OLED_ShowString(0, 16, line);

    snprintf(line, sizeof(line), "Humi: %u.%u %%", humidity_x10 / 10U, humidity_x10 % 10U);
    OLED_ShowString(0, 32, line);
  }

  OLED_ShowString(0, 56, "Press SW Back");
  OLED_UpdateScreen();
}

void UI_ShowLightPage(uint16_t rawValue, const char *level)
{
  char line[22];

  OLED_Clear();
  OLED_ShowString(0, 0, "Light");

  snprintf(line, sizeof(line), "ADC: %u", rawValue);
  OLED_ShowString(0, 16, line);

  snprintf(line, sizeof(line), "Level: %s", level);
  OLED_ShowString(0, 32, line);

  OLED_ShowString(0, 56, "Press SW Back");
  OLED_UpdateScreen();
}

void UI_ShowLedControlPage(const char *mode)
{
  char line[22];

  if (mode == NULL) {
    mode = "OFF";
  }

  OLED_Clear();
  OLED_ShowString(0, 0, "LED Control");

  snprintf(line, sizeof(line), "Mode: %s", mode);
  OLED_ShowString(0, 24, line);

  OLED_ShowString(0, 40, "Rotate Change");
  OLED_ShowString(0, 56, "Press SW Back");
  OLED_UpdateScreen();
}

void UI_ShowBeepModePage(uint8_t enabled)
{
  OLED_Clear();
  OLED_ShowString(0, 0, "Beep Mode");
  OLED_ShowString(0, 24, (enabled != 0U) ? "BEEP: ON" : "BEEP: OFF");
  OLED_ShowString(0, 40, "Rotate Change");
  OLED_ShowString(0, 56, "Press SW Back");
  OLED_UpdateScreen();
}

void UI_ShowBluetoothPage(void)
{
  OLED_Clear();
  OLED_ShowString(0, 0, "Bluetooth");
  OLED_ShowString(0, 24, "Sending...");
  OLED_ShowString(0, 56, "Press SW Back");
  OLED_UpdateScreen();
}

void UI_ShowMotionPage(uint8_t valid, int16_t ax, int16_t ay, int16_t az, uint8_t shake)
{
  char line[22];

  OLED_Clear();
  OLED_ShowString(0, 0, "Motion");

  if (valid == 0U) {
    OLED_ShowString(0, 24, "MPU6050 ERROR");
  } else {
    snprintf(line, sizeof(line), "AX:%6d", ax);
    OLED_ShowString(0, 12, line);

    snprintf(line, sizeof(line), "AY:%6d", ay);
    OLED_ShowString(0, 24, line);

    snprintf(line, sizeof(line), "AZ:%6d", az);
    OLED_ShowString(0, 36, line);

    snprintf(line, sizeof(line), "Shake:%s", (shake != 0U) ? "YES" : "NO");
    OLED_ShowString(0, 48, line);
  }

  OLED_UpdateScreen();
}

void UI_ShowHistoryPage(uint8_t flashAvailable,
                        uint16_t count,
                        uint8_t hasLast,
                        int16_t temperature_x10,
                        uint16_t humidity_x10,
                        uint16_t light,
                        uint8_t shake)
{
  char line[22];
  int16_t tempAbs;

  OLED_Clear();
  OLED_ShowString(0, 0, "History");

  if (flashAvailable == 0U) {
    OLED_ShowString(0, 24, "FLASH ERROR");
    OLED_UpdateScreen();
    return;
  }

  snprintf(line, sizeof(line), "Count:%u", count);
  OLED_ShowString(0, 12, line);
  OLED_ShowString(0, 24, "Last:");

  if (hasLast == 0U) {
    OLED_ShowString(0, 40, "No Records");
  } else {
    tempAbs = (temperature_x10 < 0) ? (int16_t)(-temperature_x10) : temperature_x10;
    snprintf(line, sizeof(line), "T:%s%d.%d H:%u.%u",
             (temperature_x10 < 0) ? "-" : "",
             tempAbs / 10,
             tempAbs % 10,
             humidity_x10 / 10U,
             humidity_x10 % 10U);
    OLED_ShowString(0, 36, line);

    snprintf(line, sizeof(line), "L:%u S:%u", light, (shake != 0U) ? 1U : 0U);
    OLED_ShowString(0, 48, line);
  }

  OLED_UpdateScreen();
}
