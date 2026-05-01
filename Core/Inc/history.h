#ifndef __HISTORY_H
#define __HISTORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define HISTORY_MAX_RECORDS       100U

typedef struct {
  uint32_t index;
  int16_t temperature_x10;
  uint16_t humidity_x10;
  uint16_t light;
  int16_t ax;
  int16_t ay;
  int16_t az;
  uint8_t shake;
  uint8_t reserved[15];
} HistoryRecord_t;

typedef struct {
  uint8_t flashAvailable;
  uint16_t recordCount;
  uint16_t writeIndex;
} HistoryStatus_t;

void History_Init(void);
uint8_t History_IsAvailable(void);
uint8_t History_SaveRecord(const HistoryRecord_t *record);
uint8_t History_Clear(void);
void History_GetStatus(HistoryStatus_t *status);
uint8_t History_GetRecord(uint16_t chronologicalIndex, HistoryRecord_t *record);
uint8_t History_GetLastRecord(HistoryRecord_t *record);

#ifdef __cplusplus
}
#endif

#endif /* __HISTORY_H */
