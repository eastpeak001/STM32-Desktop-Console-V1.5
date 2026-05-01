#include "bluetooth.h"
#include "app.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

#define BLUETOOTH_TX_TIMEOUT_MS    50U
#define BLUETOOTH_TX_LED_PORT      GPIOB
#define BLUETOOTH_TX_LED_PIN       GPIO_PIN_14
#define BLUETOOTH_TX_LED_MS        80U

static uint8_t rx_byte;
static volatile uint8_t rx_index;
static volatile uint8_t command_ready;
static volatile uint8_t rx_overflow;
static volatile uint8_t command_too_long;
static uint8_t txLedActive;
static uint32_t txLedStopTick;
static char rx_buffer[BLUETOOTH_COMMAND_MAX_LEN];
static char command_buffer[BLUETOOTH_COMMAND_MAX_LEN];

static void Bluetooth_MarkTxActivity(void)
{
  HAL_GPIO_WritePin(BLUETOOTH_TX_LED_PORT, BLUETOOTH_TX_LED_PIN, GPIO_PIN_SET);
  txLedActive = 1;
  txLedStopTick = HAL_GetTick() + BLUETOOTH_TX_LED_MS;
}

static void Bluetooth_TxLedTask(void)
{
  if (txLedActive != 0U) {
    if ((int32_t)(HAL_GetTick() - txLedStopTick) >= 0) {
      HAL_GPIO_WritePin(BLUETOOTH_TX_LED_PORT, BLUETOOTH_TX_LED_PIN, GPIO_PIN_RESET);
      txLedActive = 0;
    }
  }
}

static void Bluetooth_Transmit(const char *str)
{
  if (str == NULL) {
    return;
  }

  Bluetooth_MarkTxActivity();
  HAL_UART_Transmit(&huart1, (uint8_t *)str, (uint16_t)strlen(str), BLUETOOTH_TX_TIMEOUT_MS);
}

static char Bluetooth_ToUpperChar(char ch)
{
  if ((ch >= 'a') && (ch <= 'z')) {
    return (char)(ch - 'a' + 'A');
  }

  return ch;
}

static uint8_t Bluetooth_IsSpace(char ch)
{
  return ((ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n')) ? 1U : 0U;
}

static void Bluetooth_NormalizeCommand(char *command)
{
  uint16_t readIndex = 0;
  uint16_t writeIndex = 0;
  uint16_t length;
  uint8_t lastWasSpace = 0;

  while ((command[readIndex] != '\0') && (Bluetooth_IsSpace(command[readIndex]) != 0U)) {
    readIndex++;
  }

  while (command[readIndex] != '\0') {
    char ch = command[readIndex++];
    if (Bluetooth_IsSpace(ch) != 0U) {
      if (lastWasSpace == 0U) {
        command[writeIndex++] = ' ';
        lastWasSpace = 1U;
      }
    } else {
      command[writeIndex++] = Bluetooth_ToUpperChar(ch);
      lastWasSpace = 0U;
    }
  }
  command[writeIndex] = '\0';

  length = writeIndex;
  while ((length > 0U) && (Bluetooth_IsSpace(command[length - 1U]) != 0U)) {
    command[--length] = '\0';
  }
}

static void Bluetooth_FinishCommandFromIsr(void)
{
  if (rx_overflow != 0U) {
    rx_index = 0;
    rx_overflow = 0;
    return;
  }

  if ((rx_index > 0U) && (command_ready == 0U)) {
    rx_buffer[rx_index] = '\0';
    memcpy(command_buffer, rx_buffer, rx_index + 1U);
    command_ready = 1U;
  }

  rx_index = 0;
}

static uint8_t Bluetooth_GetCommand(char *command, uint16_t size)
{
  if ((command == NULL) || (size == 0U) || (command_ready == 0U)) {
    return 0;
  }

  __disable_irq();
  strncpy(command, command_buffer, size - 1U);
  command[size - 1U] = '\0';
  command_ready = 0;
  __enable_irq();

  return 1;
}

static uint8_t Bluetooth_GetTooLongFlag(void)
{
  uint8_t flag;

  __disable_irq();
  flag = command_too_long;
  command_too_long = 0;
  __enable_irq();

  return flag;
}

static void Bluetooth_SendHelp(void)
{
  Bluetooth_SendString("AVAILABLE COMMANDS:\r\n");
  Bluetooth_SendString("HELP / BANGZHU\r\n");
  Bluetooth_SendString("STATUS / ZHUANGTAI\r\n");
  Bluetooth_SendString("LED ON / KAIDENG\r\n");
  Bluetooth_SendString("LED OFF / GUANDENG\r\n");
  Bluetooth_SendString("LED BLINK / SHANDENG\r\n");
  Bluetooth_SendString("BUZZER / FENGMING\r\n");
  Bluetooth_SendString("BEEP ON / FENGMING ON\r\n");
  Bluetooth_SendString("BEEP OFF / FENGMING OFF\r\n");
  Bluetooth_SendString("BEEP TOGGLE / JINGYIN\r\n");
  Bluetooth_SendString("ICON SMILE / TUBIAOXIAOLIAN\r\n");
  Bluetooth_SendString("ICON TEMP / TUBIAOWENDU\r\n");
  Bluetooth_SendString("ICON LIGHT / TUBIAOGUANGZHAO\r\n");
  Bluetooth_SendString("ICON LED / TUBIAODENG\r\n");
  Bluetooth_SendString("ICON BUZZER / TUBIAOFENGMING\r\n");
  Bluetooth_SendString("ICON BT / TUBIAOLANYA\r\n");
  Bluetooth_SendString("ICON ERROR / TUBIAOCUOWU\r\n");
  Bluetooth_SendString("GET MOTION / ZITAI\r\n");
  Bluetooth_SendString("SAVE NOW / BAOCUN\r\n");
  Bluetooth_SendString("HISTORY STATUS / JILUZHUANGTAI\r\n");
  Bluetooth_SendString("GET HISTORY / DAOCHUJILU\r\n");
  Bluetooth_SendString("CLEAR HISTORY / QINGKONGJILU\r\n");
  Bluetooth_SendString("OK\r\n");
}

static uint8_t Bluetooth_HandleIconCommand(const char *command)
{
  if ((strcmp(command, "ICON SMILE") == 0) ||
      (strcmp(command, "TUBIAO XIAOLIAN") == 0) ||
      (strcmp(command, "TUBIAOXIAOLIAN") == 0)) {
    return App_BluetoothSetIcon("SMILE");
  }

  if ((strcmp(command, "ICON TEMP") == 0) ||
      (strcmp(command, "TUBIAO WENDU") == 0) ||
      (strcmp(command, "TUBIAOWENDU") == 0)) {
    return App_BluetoothSetIcon("TEMP");
  }

  if ((strcmp(command, "ICON LIGHT") == 0) ||
      (strcmp(command, "TUBIAO GUANGZHAO") == 0) ||
      (strcmp(command, "TUBIAOGUANGZHAO") == 0)) {
    return App_BluetoothSetIcon("LIGHT");
  }

  if ((strcmp(command, "ICON LED") == 0) ||
      (strcmp(command, "TUBIAO DENG") == 0) ||
      (strcmp(command, "TUBIAODENG") == 0)) {
    return App_BluetoothSetIcon("LED");
  }

  if ((strcmp(command, "ICON BUZZER") == 0) ||
      (strcmp(command, "TUBIAO FENGMING") == 0) ||
      (strcmp(command, "TUBIAOFENGMING") == 0)) {
    return App_BluetoothSetIcon("BUZZER");
  }

  if ((strcmp(command, "ICON BT") == 0) ||
      (strcmp(command, "TUBIAO LANYA") == 0) ||
      (strcmp(command, "TUBIAOLANYA") == 0)) {
    return App_BluetoothSetIcon("BT");
  }

  if ((strcmp(command, "ICON ERROR") == 0) ||
      (strcmp(command, "TUBIAO CUOWU") == 0) ||
      (strcmp(command, "TUBIAOCUOWU") == 0)) {
    return App_BluetoothSetIcon("ERROR");
  }

  if ((strcmp(command, "ICON MOTION") == 0) ||
      (strcmp(command, "TUBIAO ZITAI") == 0) ||
      (strcmp(command, "TUBIAOZITAI") == 0)) {
    return App_BluetoothSetIcon("MOTION");
  }

  if ((strcmp(command, "ICON HISTORY") == 0) ||
      (strcmp(command, "TUBIAO JILU") == 0) ||
      (strcmp(command, "TUBIAOJILU") == 0)) {
    return App_BluetoothSetIcon("HISTORY");
  }

  return 0;
}

static const char *Bluetooth_GetLedModeFromCommand(const char *command)
{
  if ((strcmp(command, "LED OFF") == 0) || (strcmp(command, "GUANDENG") == 0)) {
    return "OFF";
  }

  if ((strcmp(command, "LED ON") == 0) || (strcmp(command, "KAIDENG") == 0)) {
    return "ON";
  }

  if ((strcmp(command, "LED BLINK") == 0) || (strcmp(command, "SHANDENG") == 0)) {
    return "BLINK";
  }

  return NULL;
}

static const char *Bluetooth_GetBeepModeFromCommand(const char *command)
{
  if ((strcmp(command, "BEEP ON") == 0) || (strcmp(command, "FENGMING ON") == 0)) {
    return "ON";
  }

  if ((strcmp(command, "BEEP OFF") == 0) || (strcmp(command, "FENGMING OFF") == 0)) {
    return "OFF";
  }

  return NULL;
}

static void Bluetooth_SendDone(void)
{
  Bluetooth_SendString("OK\r\n");
}

static void Bluetooth_ExecuteCommand(char *command)
{
  const char *ledMode;
  const char *beepMode;

  Bluetooth_NormalizeCommand(command);

  if ((strcmp(command, "HELP") == 0) || (strcmp(command, "BANGZHU") == 0)) {
    App_BluetoothBeepOnAction();
    Bluetooth_SendHelp();
  } else if ((strcmp(command, "STATUS") == 0) || (strcmp(command, "ZHUANGTAI") == 0)) {
    App_BluetoothSendStatus();
    App_BluetoothBeepOnAction();
    Bluetooth_SendDone();
  } else if ((strcmp(command, "GET MOTION") == 0) || (strcmp(command, "ZITAI") == 0)) {
    App_BluetoothSendMotion();
    App_BluetoothBeepOnAction();
    Bluetooth_SendDone();
  } else if ((strcmp(command, "SAVE NOW") == 0) || (strcmp(command, "BAOCUN") == 0)) {
    if (App_BluetoothSaveHistoryNow() != 0U) {
      App_BluetoothBeepOnAction();
      Bluetooth_SendDone();
    } else {
      Bluetooth_SendString("ERR FLASH\r\n");
    }
  } else if ((strcmp(command, "HISTORY STATUS") == 0) ||
             (strcmp(command, "JILU ZHUANGTAI") == 0) ||
             (strcmp(command, "JILUZHUANGTAI") == 0)) {
    App_BluetoothSendHistoryStatus();
    App_BluetoothBeepOnAction();
    Bluetooth_SendDone();
  } else if ((strcmp(command, "GET HISTORY") == 0) ||
             (strcmp(command, "DAOCHU JILU") == 0) ||
             (strcmp(command, "DAOCHUJILU") == 0)) {
    App_BluetoothSendHistoryRecords();
    App_BluetoothBeepOnAction();
    Bluetooth_SendDone();
  } else if ((strcmp(command, "CLEAR HISTORY") == 0) ||
             (strcmp(command, "QINGKONG JILU") == 0) ||
             (strcmp(command, "QINGKONGJILU") == 0)) {
    if (App_BluetoothClearHistory() != 0U) {
      App_BluetoothBeepOnAction();
      Bluetooth_SendDone();
    } else {
      Bluetooth_SendString("ERR FLASH\r\n");
    }
  } else if ((ledMode = Bluetooth_GetLedModeFromCommand(command)) != NULL) {
    if (App_BluetoothSetLedMode(ledMode) != 0U) {
      Bluetooth_SendDone();
    } else {
      Bluetooth_SendString("ERR UNKNOWN CMD\r\n");
    }
  } else if ((beepMode = Bluetooth_GetBeepModeFromCommand(command)) != NULL) {
    if (App_BluetoothSetBeepMode(beepMode) != 0U) {
      Bluetooth_SendDone();
    } else {
      Bluetooth_SendString("ERR UNKNOWN CMD\r\n");
    }
  } else if ((strcmp(command, "BEEP TOGGLE") == 0) || (strcmp(command, "JINGYIN") == 0)) {
    App_BluetoothToggleBeepMode();
    Bluetooth_SendDone();
  } else if ((strcmp(command, "BUZZER") == 0) || (strcmp(command, "FENGMING") == 0)) {
    App_BluetoothBuzzerBeep();
    Bluetooth_SendDone();
  } else if (Bluetooth_HandleIconCommand(command) != 0U) {
    App_BluetoothBeepOnAction();
    Bluetooth_SendDone();
  } else {
    Bluetooth_SendString("ERR UNKNOWN CMD\r\n");
  }
}

void Bluetooth_Init(void)
{
  rx_index = 0;
  command_ready = 0;
  rx_overflow = 0;
  command_too_long = 0;
  txLedActive = 0;
  txLedStopTick = 0;
  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(command_buffer, 0, sizeof(command_buffer));
  HAL_GPIO_WritePin(BLUETOOTH_TX_LED_PORT, BLUETOOTH_TX_LED_PIN, GPIO_PIN_RESET);
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void Bluetooth_Process(void)
{
  char command[BLUETOOTH_COMMAND_MAX_LEN];

  Bluetooth_TxLedTask();

  if (Bluetooth_GetTooLongFlag() != 0U) {
    Bluetooth_SendString("ERR CMD TOO LONG\r\n");
  }

  while (Bluetooth_GetCommand(command, sizeof(command)) != 0U) {
    Bluetooth_ExecuteCommand(command);
  }
}

void Bluetooth_SendString(const char *str)
{
  Bluetooth_Transmit(str);
}

void Bluetooth_SendStatus(int16_t temperature_x10,
                          uint16_t humidity_x10,
                          uint8_t tempHumiValid,
                          uint16_t lightRaw,
                          const char *ledMode,
                          uint8_t beepEnabled)
{
  char txBuffer[96];
  int length;
  int16_t tempAbs = (temperature_x10 < 0) ? (int16_t)(-temperature_x10) : temperature_x10;
  const char *beepText = (beepEnabled != 0U) ? "ON" : "OFF";

  if (ledMode == NULL) {
    ledMode = "OFF";
  }

  if (tempHumiValid != 0U) {
    length = snprintf(txBuffer, sizeof(txBuffer),
                      "TEMP=%s%d.%d,HUMI=%u.%u,LIGHT=%u,LED=%s,BEEP=%s\r\n",
                      (temperature_x10 < 0) ? "-" : "",
                      tempAbs / 10,
                      tempAbs % 10,
                      humidity_x10 / 10U,
                      humidity_x10 % 10U,
                      lightRaw,
                      ledMode,
                      beepText);
  } else {
    length = snprintf(txBuffer, sizeof(txBuffer),
                      "TEMP=0.0,HUMI=0.0,LIGHT=%u,LED=%s,BEEP=%s\r\n",
                      lightRaw,
                      ledMode,
                      beepText);
  }

  if (length > 0) {
    if ((size_t)length > sizeof(txBuffer)) {
      length = (int)strlen(txBuffer);
    }
    Bluetooth_MarkTxActivity();
    HAL_UART_Transmit(&huart1, (uint8_t *)txBuffer, (uint16_t)length, BLUETOOTH_TX_TIMEOUT_MS);
  }
}

void Bluetooth_SendMotion(int16_t ax, int16_t ay, int16_t az, uint8_t valid, uint8_t shake)
{
  char txBuffer[80];
  int length;

  if (valid == 0U) {
    ax = 0;
    ay = 0;
    az = 0;
    shake = 0;
  }

  length = snprintf(txBuffer, sizeof(txBuffer),
                    "AX=%d,AY=%d,AZ=%d,SHAKE=%u\r\n",
                    ax,
                    ay,
                    az,
                    (shake != 0U) ? 1U : 0U);

  if (length > 0) {
    if ((size_t)length > sizeof(txBuffer)) {
      length = (int)strlen(txBuffer);
    }
    Bluetooth_MarkTxActivity();
    HAL_UART_Transmit(&huart1, (uint8_t *)txBuffer, (uint16_t)length, BLUETOOTH_TX_TIMEOUT_MS);
  }
}

void Bluetooth_SendHistoryStatus(uint16_t recordCount, uint16_t writeIndex)
{
  char txBuffer[64];
  int length = snprintf(txBuffer, sizeof(txBuffer),
                        "RECORDS=%u,WRITE_INDEX=%u\r\n",
                        recordCount,
                        writeIndex);

  if (length > 0) {
    Bluetooth_MarkTxActivity();
    HAL_UART_Transmit(&huart1, (uint8_t *)txBuffer, (uint16_t)length, BLUETOOTH_TX_TIMEOUT_MS);
  }
}

void Bluetooth_SendHistoryRecord(uint16_t number, const HistoryRecord_t *record)
{
  char txBuffer[192];
  int16_t tempAbs;
  int length;

  if (record == NULL) {
    return;
  }

  tempAbs = (record->temperature_x10 < 0) ? (int16_t)(-record->temperature_x10) : record->temperature_x10;
  length = snprintf(txBuffer, sizeof(txBuffer),
                    "REC %u,T=%s%d.%d,H=%u.%u,L=%u,AX=%d,AY=%d,AZ=%d,SHAKE=%u\r\n",
                    number,
                    (record->temperature_x10 < 0) ? "-" : "",
                    tempAbs / 10,
                    tempAbs % 10,
                    record->humidity_x10 / 10U,
                    record->humidity_x10 % 10U,
                    record->light,
                    record->ax,
                    record->ay,
                    record->az,
                    (record->shake != 0U) ? 1U : 0U);

  if (length > 0) {
    if ((size_t)length > sizeof(txBuffer)) {
      length = (int)strlen(txBuffer);
    }
    Bluetooth_MarkTxActivity();
    HAL_UART_Transmit(&huart1, (uint8_t *)txBuffer, (uint16_t)length, BLUETOOTH_TX_TIMEOUT_MS);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    if ((rx_byte == '\r') || (rx_byte == '\n')) {
      Bluetooth_FinishCommandFromIsr();
    } else if (rx_overflow == 0U) {
      if (rx_index < (BLUETOOTH_COMMAND_MAX_LEN - 1U)) {
        rx_buffer[rx_index++] = (char)rx_byte;
      } else {
        rx_index = 0;
        rx_overflow = 1U;
        command_too_long = 1U;
      }
    }

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    rx_index = 0;
    rx_overflow = 0;
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}
