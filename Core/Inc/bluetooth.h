#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "history.h"

#define BLUETOOTH_COMMAND_MAX_LEN     64U

void Bluetooth_Init(void);
void Bluetooth_Process(void);
void Bluetooth_SendString(const char *str);
void Bluetooth_SendStatus(int16_t temperature_x10,
                          uint16_t humidity_x10,
                          uint8_t tempHumiValid,
                          uint16_t lightRaw,
                          const char *ledMode,
                          uint8_t beepEnabled);
void Bluetooth_SendMotion(int16_t ax, int16_t ay, int16_t az, uint8_t valid, uint8_t shake);
void Bluetooth_SendHistoryStatus(uint16_t recordCount, uint16_t writeIndex);
void Bluetooth_SendHistoryRecord(uint16_t number, const HistoryRecord_t *record);

#ifdef __cplusplus
}
#endif

#endif /* __BLUETOOTH_H */
