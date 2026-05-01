#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void App_Init(void);
void App_Loop(void);
void App_BluetoothSendStatus(void);
void App_BluetoothSendMotion(void);
uint8_t App_BluetoothSaveHistoryNow(void);
void App_BluetoothSendHistoryStatus(void);
void App_BluetoothSendHistoryRecords(void);
uint8_t App_BluetoothClearHistory(void);
uint8_t App_BluetoothSetLedMode(const char *mode);
void App_BluetoothBuzzerBeep(void);
void App_BluetoothBeepOnAction(void);
uint8_t App_BluetoothSetBeepMode(const char *mode);
uint8_t App_BluetoothToggleBeepMode(void);
uint8_t App_BluetoothSetIcon(const char *icon);

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */
