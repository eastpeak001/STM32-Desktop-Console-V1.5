#include "app.h"
#include "bluetooth.h"
#include "buzzer.h"
#include "encoder.h"
#include "history.h"
#include "max7219.h"
#include "mpu6050.h"
#include "oled.h"
#include "sensors.h"
#include "ui.h"
#include <string.h>

#define APP_UI_REFRESH_INTERVAL_MS    80U
#define APP_TEMP_HUMI_REFRESH_MS      1000U
#define APP_LIGHT_REFRESH_MS          300U
#define APP_MOTION_REFRESH_MS         300U
#define APP_LED_BLINK_INTERVAL_MS     500U
#define APP_BUZZER_BEEP_MS            100U
#define APP_BLUETOOTH_SEND_MS         2000U
#define APP_HISTORY_SAVE_MS           10000U
#define APP_HISTORY_PAGE_REFRESH_MS   1000U

#define APP_LED_PORT                  GPIOB
#define APP_LED1_PIN                  GPIO_PIN_12
#define APP_LED2_PIN                  GPIO_PIN_13
#define APP_LED_ALL_PINS              (APP_LED1_PIN | APP_LED2_PIN)

typedef enum {
  APP_SCREEN_MENU = 0,
  APP_SCREEN_PAGE
} AppScreen_t;

typedef enum {
  APP_LED_MODE_OFF = 0,
  APP_LED_MODE_ON,
  APP_LED_MODE_BLINK,
  APP_LED_MODE_COUNT
} AppLedMode_t;

static AppScreen_t currentScreen;
static UiMenuItem_t selectedMenuItem;
static AppLedMode_t ledMode;
static uint8_t ledBlinkState;
static uint8_t uiDirty;
static uint32_t lastUiRefreshTick;
static uint32_t lastPageRefreshTick;
static uint32_t lastLedBlinkTick;
static uint32_t lastBluetoothSendTick;
static uint32_t lastHistorySaveTick;
static Max7219Icon_t currentMatrixIcon;
static uint8_t matrixIconValid;
static MPU6050_AccelData_t motionData;
static MPU6050_AccelData_t lastMotionData;
static uint8_t motionDataValid;
static uint8_t motionReady;
static uint8_t motionShake;
static uint8_t motionHasPrevious;

static int32_t App_Abs32(int32_t value)
{
  return (value < 0) ? -value : value;
}

static const char *App_GetLedModeName(void)
{
  static const char *ledModeNames[APP_LED_MODE_COUNT] = {
    "OFF",
    "ON",
    "BLINK"
  };

  return ledModeNames[ledMode];
}

static void App_SetLeds(uint8_t on)
{
  HAL_GPIO_WritePin(APP_LED_PORT, APP_LED_ALL_PINS, (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void App_ApplyLedMode(uint32_t now)
{
  if (ledMode == APP_LED_MODE_OFF) {
    ledBlinkState = 0;
    App_SetLeds(0);
    return;
  }

  if (ledMode == APP_LED_MODE_ON) {
    ledBlinkState = 1;
    App_SetLeds(1);
    return;
  }

  if ((now - lastLedBlinkTick) >= APP_LED_BLINK_INTERVAL_MS) {
    lastLedBlinkTick = now;
    ledBlinkState = (ledBlinkState == 0U) ? 1U : 0U;
    App_SetLeds(ledBlinkState);
  }
}

static void App_SelectNextMenuItem(void)
{
  selectedMenuItem = (UiMenuItem_t)((selectedMenuItem + 1U) % UI_MENU_COUNT);
  uiDirty = 1;
}

static void App_SelectPreviousMenuItem(void)
{
  if (selectedMenuItem == 0U) {
    selectedMenuItem = (UiMenuItem_t)(UI_MENU_COUNT - 1U);
  } else {
    selectedMenuItem = (UiMenuItem_t)(selectedMenuItem - 1U);
  }

  uiDirty = 1;
}

static void App_SelectPreviousLedMode(void)
{
  if (ledMode == 0U) {
    ledMode = (AppLedMode_t)(APP_LED_MODE_COUNT - 1U);
  } else {
    ledMode = (AppLedMode_t)(ledMode - 1U);
  }
}

static void App_SetLedMode(AppLedMode_t mode, uint32_t now)
{
  ledMode = mode;
  ledBlinkState = 0;
  lastLedBlinkTick = now;
  App_ApplyLedMode(now);

  if ((currentScreen == APP_SCREEN_PAGE) && (selectedMenuItem == UI_MENU_LED_CONTROL)) {
    uiDirty = 1;
  }
}

static void App_SendBluetoothStatus(void)
{
  int16_t temperature_x10 = 0;
  uint16_t humidity_x10 = 0;
  uint8_t valid = Sensors_GetTempHumi(&temperature_x10, &humidity_x10);
  uint16_t lightRaw = Sensors_ReadLightRaw();

  Bluetooth_SendStatus(temperature_x10,
                       humidity_x10,
                       valid,
                       lightRaw,
                       App_GetLedModeName(),
                       Buzzer_IsEnabled());
}

static void App_UpdateMotionData(void)
{
  MPU6050_AccelData_t newData;

  if (motionReady == 0U) {
    motionReady = (MPU6050_Init() == HAL_OK) ? 1U : 0U;
    if (motionReady == 0U) {
      motionDataValid = 0;
      motionShake = 0;
      return;
    }
  }

  if (MPU6050_ReadAccel(&newData) != HAL_OK) {
    motionDataValid = 0;
    motionReady = 0;
    motionShake = 0;
    return;
  }

  if (motionHasPrevious != 0U) {
    int32_t diff = App_Abs32((int32_t)newData.ax - lastMotionData.ax) +
                   App_Abs32((int32_t)newData.ay - lastMotionData.ay) +
                   App_Abs32((int32_t)newData.az - lastMotionData.az);
    motionShake = (diff > MPU6050_SHAKE_THRESHOLD) ? 1U : 0U;
  } else {
    motionShake = 0;
    motionHasPrevious = 1;
  }

  motionData = newData;
  lastMotionData = newData;
  motionDataValid = 1;
}

static void App_FillHistoryRecord(HistoryRecord_t *record)
{
  int16_t temperature_x10 = 0;
  uint16_t humidity_x10 = 0;
  uint8_t tempValid = Sensors_GetTempHumi(&temperature_x10, &humidity_x10);

  if (record == NULL) {
    return;
  }

  App_UpdateMotionData();

  record->index = 0;
  record->temperature_x10 = (tempValid != 0U) ? temperature_x10 : 0;
  record->humidity_x10 = (tempValid != 0U) ? humidity_x10 : 0;
  record->light = Sensors_ReadLightRaw();
  record->ax = motionDataValid ? motionData.ax : 0;
  record->ay = motionDataValid ? motionData.ay : 0;
  record->az = motionDataValid ? motionData.az : 0;
  record->shake = motionShake;
  memset(record->reserved, 0xFF, sizeof(record->reserved));
}

static uint8_t App_SaveHistoryRecord(void)
{
  HistoryRecord_t record;

  App_FillHistoryRecord(&record);
  return History_SaveRecord(&record);
}

static void App_HandleHistoryAutoSave(uint32_t now)
{
  if ((now - lastHistorySaveTick) >= APP_HISTORY_SAVE_MS) {
    lastHistorySaveTick = now;
    (void)App_SaveHistoryRecord();
  }
}

static void App_SetMatrixIcon(Max7219Icon_t icon)
{
  if ((matrixIconValid == 0U) || (currentMatrixIcon != icon)) {
    MAX7219_DisplayIcon(icon);
    currentMatrixIcon = icon;
    matrixIconValid = 1;
  }
}

static Max7219Icon_t App_GetPageIcon(uint8_t tempHumiValid)
{
  if (selectedMenuItem == UI_MENU_TEMP_HUMI) {
    return (tempHumiValid != 0U) ? ICON_TEMP : ICON_ERROR;
  }

  if (selectedMenuItem == UI_MENU_LIGHT) {
    return ICON_LIGHT;
  }

  if (selectedMenuItem == UI_MENU_LED_CONTROL) {
    return ICON_LED;
  }

  if (selectedMenuItem == UI_MENU_BUZZER) {
    return ICON_BUZZER;
  }

  if (selectedMenuItem == UI_MENU_BLUETOOTH) {
    return ICON_BLUETOOTH;
  }

  if (selectedMenuItem == UI_MENU_MOTION) {
    return ICON_MOTION;
  }

  if (selectedMenuItem == UI_MENU_HISTORY) {
    return ICON_HISTORY;
  }

  return ICON_SMILE;
}

static void App_ShowCurrentPage(void)
{
  if (selectedMenuItem == UI_MENU_TEMP_HUMI) {
    int16_t temperature_x10 = 0;
    uint16_t humidity_x10 = 0;
    uint8_t valid = Sensors_GetTempHumi(&temperature_x10, &humidity_x10);

    UI_ShowTempHumiPage(valid, temperature_x10, humidity_x10);
    App_SetMatrixIcon(App_GetPageIcon(valid));
  } else if (selectedMenuItem == UI_MENU_LIGHT) {
    uint16_t rawValue = Sensors_ReadLightRaw();

    UI_ShowLightPage(rawValue, Sensors_GetLightLevel(rawValue));
    App_SetMatrixIcon(App_GetPageIcon(1));
  } else if (selectedMenuItem == UI_MENU_LED_CONTROL) {
    UI_ShowLedControlPage(App_GetLedModeName());
    App_SetMatrixIcon(App_GetPageIcon(1));
  } else if (selectedMenuItem == UI_MENU_BUZZER) {
    UI_ShowBeepModePage(Buzzer_IsEnabled());
    App_SetMatrixIcon(App_GetPageIcon(1));
  } else if (selectedMenuItem == UI_MENU_BLUETOOTH) {
    UI_ShowBluetoothPage();
    App_SetMatrixIcon(App_GetPageIcon(1));
  } else if (selectedMenuItem == UI_MENU_MOTION) {
    App_UpdateMotionData();
    UI_ShowMotionPage(motionDataValid, motionData.ax, motionData.ay, motionData.az, motionShake);
    App_SetMatrixIcon(App_GetPageIcon(1));
  } else if (selectedMenuItem == UI_MENU_HISTORY) {
    HistoryStatus_t status;
    HistoryRecord_t lastRecord;
    uint8_t hasLast;

    History_GetStatus(&status);
    hasLast = History_GetLastRecord(&lastRecord);
    UI_ShowHistoryPage(status.flashAvailable,
                       status.recordCount,
                       hasLast,
                       lastRecord.temperature_x10,
                       lastRecord.humidity_x10,
                       lastRecord.light,
                       lastRecord.shake);
    App_SetMatrixIcon(App_GetPageIcon(1));
  } else {
    UI_ShowPage(selectedMenuItem);
    App_SetMatrixIcon(App_GetPageIcon(1));
  }
}

static void App_EnterSelectedPage(uint32_t now)
{
  currentScreen = APP_SCREEN_PAGE;
  uiDirty = 1;
  lastPageRefreshTick = now;
  Buzzer_BeepOnAction();

  if (selectedMenuItem == UI_MENU_BLUETOOTH) {
    lastBluetoothSendTick = now - APP_BLUETOOTH_SEND_MS;
  } else if (selectedMenuItem == UI_MENU_LED_CONTROL) {
    lastLedBlinkTick = now;
  }
}

static void App_ReturnToMenu(void)
{
  Buzzer_BeepOnAction();
  currentScreen = APP_SCREEN_MENU;
  uiDirty = 1;
}

static void App_HandleMenuInput(EncoderDirection_t direction, uint8_t buttonPressed, uint32_t now)
{
  if (direction == ENCODER_CW) {
    App_SelectNextMenuItem();
  } else if (direction == ENCODER_CCW) {
    App_SelectPreviousMenuItem();
  }

  if (buttonPressed != 0U) {
    App_EnterSelectedPage(now);
  }
}

static void App_HandleLedControlInput(EncoderDirection_t direction, uint32_t now)
{
  if (direction == ENCODER_NONE) {
    return;
  }

  if (direction == ENCODER_CW) {
    App_SetLedMode((AppLedMode_t)((ledMode + 1U) % APP_LED_MODE_COUNT), now);
  } else {
    App_SelectPreviousLedMode();
    App_SetLedMode(ledMode, now);
  }
  Buzzer_BeepOnAction();
  uiDirty = 1;
}

static void App_ToggleBeepMode(void)
{
  Buzzer_SetEnabled((Buzzer_IsEnabled() == 0U) ? 1U : 0U);
  Buzzer_BeepOnAction();
  uiDirty = 1;
}

static void App_RequestTimedPageRefresh(uint32_t now)
{
  if (selectedMenuItem == UI_MENU_TEMP_HUMI) {
    if ((now - lastPageRefreshTick) >= APP_TEMP_HUMI_REFRESH_MS) {
      uiDirty = 1;
      lastPageRefreshTick = now;
    }
  } else if (selectedMenuItem == UI_MENU_LIGHT) {
    if ((now - lastPageRefreshTick) >= APP_LIGHT_REFRESH_MS) {
      uiDirty = 1;
      lastPageRefreshTick = now;
    }
  } else if (selectedMenuItem == UI_MENU_BLUETOOTH) {
    if ((now - lastBluetoothSendTick) >= APP_BLUETOOTH_SEND_MS) {
      App_SendBluetoothStatus();
      lastBluetoothSendTick = now;
    }
  } else if (selectedMenuItem == UI_MENU_MOTION) {
    if ((now - lastPageRefreshTick) >= APP_MOTION_REFRESH_MS) {
      uiDirty = 1;
      lastPageRefreshTick = now;
    }
  } else if (selectedMenuItem == UI_MENU_HISTORY) {
    if ((now - lastPageRefreshTick) >= APP_HISTORY_PAGE_REFRESH_MS) {
      uiDirty = 1;
      lastPageRefreshTick = now;
    }
  }
}

static void App_HandlePageInput(EncoderDirection_t direction, uint8_t buttonPressed, uint32_t now)
{
  if (selectedMenuItem == UI_MENU_LED_CONTROL) {
    App_HandleLedControlInput(direction, now);
  } else if ((selectedMenuItem == UI_MENU_BUZZER) && (direction != ENCODER_NONE)) {
    App_ToggleBeepMode();
  }

  if (buttonPressed != 0U) {
    App_ReturnToMenu();
  } else {
    App_RequestTimedPageRefresh(now);
  }
}

static void App_UpdateScreenIfNeeded(uint32_t now)
{
  if ((uiDirty == 0U) || ((now - lastUiRefreshTick) < APP_UI_REFRESH_INTERVAL_MS)) {
    return;
  }

  if (currentScreen == APP_SCREEN_MENU) {
    UI_ShowMainMenu(selectedMenuItem);
    App_SetMatrixIcon(ICON_SMILE);
  } else {
    App_ShowCurrentPage();
  }

  uiDirty = 0;
  lastUiRefreshTick = now;
}

void App_Init(void)
{
  OLED_Init();
  MAX7219_Init();
  motionReady = (MPU6050_Init() == HAL_OK) ? 1U : 0U;
  Encoder_Init();
  Buzzer_Init();
  Bluetooth_Init();
  Sensors_Init();
  History_Init();

  currentScreen = APP_SCREEN_MENU;
  selectedMenuItem = UI_MENU_TEMP_HUMI;
  ledMode = APP_LED_MODE_OFF;
  ledBlinkState = 0;
  uiDirty = 1;
  lastUiRefreshTick = 0;
  lastPageRefreshTick = 0;
  lastLedBlinkTick = 0;
  lastBluetoothSendTick = 0;
  lastHistorySaveTick = HAL_GetTick();
  currentMatrixIcon = ICON_SMILE;
  matrixIconValid = 0;
  motionData.ax = 0;
  motionData.ay = 0;
  motionData.az = 0;
  lastMotionData = motionData;
  motionDataValid = 0;
  motionShake = 0;
  motionHasPrevious = 0;
  App_SetLeds(0);
  App_SetMatrixIcon(ICON_SMILE);
}

void App_BluetoothSendStatus(void)
{
  App_SendBluetoothStatus();
}

void App_BluetoothSendMotion(void)
{
  App_UpdateMotionData();
  Bluetooth_SendMotion(motionData.ax, motionData.ay, motionData.az, motionDataValid, motionShake);
}

uint8_t App_BluetoothSaveHistoryNow(void)
{
  lastHistorySaveTick = HAL_GetTick();
  return App_SaveHistoryRecord();
}

void App_BluetoothSendHistoryStatus(void)
{
  HistoryStatus_t status;

  History_GetStatus(&status);
  Bluetooth_SendHistoryStatus(status.recordCount, status.writeIndex);
}

void App_BluetoothSendHistoryRecords(void)
{
  HistoryStatus_t status;
  HistoryRecord_t record;

  History_GetStatus(&status);
  for (uint16_t i = 0; i < status.recordCount; i++) {
    if (History_GetRecord(i, &record) != 0U) {
      Bluetooth_SendHistoryRecord((uint16_t)(i + 1U), &record);
    }
  }
}

uint8_t App_BluetoothClearHistory(void)
{
  lastHistorySaveTick = HAL_GetTick();
  return History_Clear();
}

uint8_t App_BluetoothSetLedMode(const char *mode)
{
  uint32_t now = HAL_GetTick();

  if (mode == NULL) {
    return 0;
  }

  if (strcmp(mode, "OFF") == 0) {
    App_SetLedMode(APP_LED_MODE_OFF, now);
  } else if (strcmp(mode, "ON") == 0) {
    App_SetLedMode(APP_LED_MODE_ON, now);
  } else if (strcmp(mode, "BLINK") == 0) {
    App_SetLedMode(APP_LED_MODE_BLINK, now);
  } else {
    return 0;
  }

  Buzzer_BeepOnAction();
  return 1;
}

void App_BluetoothBuzzerBeep(void)
{
  Buzzer_Beep(APP_BUZZER_BEEP_MS);
}

void App_BluetoothBeepOnAction(void)
{
  Buzzer_BeepOnAction();
}

uint8_t App_BluetoothSetBeepMode(const char *mode)
{
  if (mode == NULL) {
    return 0;
  }

  if (strcmp(mode, "ON") == 0) {
    Buzzer_SetEnabled(1);
    Buzzer_BeepOnAction();
  } else if (strcmp(mode, "OFF") == 0) {
    Buzzer_SetEnabled(0);
  } else {
    return 0;
  }

  if ((currentScreen == APP_SCREEN_PAGE) && (selectedMenuItem == UI_MENU_BUZZER)) {
    uiDirty = 1;
  }

  return 1;
}

uint8_t App_BluetoothToggleBeepMode(void)
{
  App_ToggleBeepMode();
  return 1;
}

uint8_t App_BluetoothSetIcon(const char *icon)
{
  if (icon == NULL) {
    return 0;
  }

  if (strcmp(icon, "SMILE") == 0) {
    App_SetMatrixIcon(ICON_SMILE);
  } else if (strcmp(icon, "TEMP") == 0) {
    App_SetMatrixIcon(ICON_TEMP);
  } else if (strcmp(icon, "LIGHT") == 0) {
    App_SetMatrixIcon(ICON_LIGHT);
  } else if (strcmp(icon, "LED") == 0) {
    App_SetMatrixIcon(ICON_LED);
  } else if (strcmp(icon, "BUZZER") == 0) {
    App_SetMatrixIcon(ICON_BUZZER);
  } else if (strcmp(icon, "BT") == 0) {
    App_SetMatrixIcon(ICON_BLUETOOTH);
  } else if (strcmp(icon, "MOTION") == 0) {
    App_SetMatrixIcon(ICON_MOTION);
  } else if (strcmp(icon, "HISTORY") == 0) {
    App_SetMatrixIcon(ICON_HISTORY);
  } else if (strcmp(icon, "ERROR") == 0) {
    App_SetMatrixIcon(ICON_ERROR);
  } else {
    return 0;
  }

  return 1;
}

void App_Loop(void)
{
  uint32_t now = HAL_GetTick();

  Sensors_Task();
  Buzzer_Task();
  Bluetooth_Process();
  App_HandleHistoryAutoSave(now);
  App_ApplyLedMode(now);
  Encoder_Scan();

  EncoderDirection_t direction = Encoder_GetRotation();
  uint8_t buttonPressed = Encoder_GetButtonPressed();

  if (currentScreen == APP_SCREEN_MENU) {
    App_HandleMenuInput(direction, buttonPressed, now);
  } else {
    App_HandlePageInput(direction, buttonPressed, now);
  }

  App_UpdateScreenIfNeeded(now);
}
