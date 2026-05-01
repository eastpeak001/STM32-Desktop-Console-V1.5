#ifndef __UI_H
#define __UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum {
  UI_MENU_TEMP_HUMI = 0,
  UI_MENU_LIGHT,
  UI_MENU_LED_CONTROL,
  UI_MENU_BUZZER,
  UI_MENU_BLUETOOTH,
  UI_MENU_MOTION,
  UI_MENU_HISTORY,
  UI_MENU_COUNT
} UiMenuItem_t;

void UI_ShowMainMenu(UiMenuItem_t selectedItem);
void UI_ShowPage(UiMenuItem_t page);
void UI_ShowTempHumiPage(uint8_t valid, int16_t temperature_x10, uint16_t humidity_x10);
void UI_ShowLightPage(uint16_t rawValue, const char *level);
void UI_ShowLedControlPage(const char *mode);
void UI_ShowBeepModePage(uint8_t enabled);
void UI_ShowBluetoothPage(void);
void UI_ShowMotionPage(uint8_t valid, int16_t ax, int16_t ay, int16_t az, uint8_t shake);
void UI_ShowHistoryPage(uint8_t flashAvailable,
                        uint16_t count,
                        uint8_t hasLast,
                        int16_t temperature_x10,
                        uint16_t humidity_x10,
                        uint16_t light,
                        uint8_t shake);
const char *UI_GetMenuItemName(UiMenuItem_t item);

#ifdef __cplusplus
}
#endif

#endif /* __UI_H */
