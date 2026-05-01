#include "history.h"
#include "w25q64.h"
#include <string.h>

#define HISTORY_BASE_ADDR          0x000000UL
#define HISTORY_RECORD_BASE_ADDR   0x000100UL
#define HISTORY_SECTOR_ADDR        0x000000UL
#define HISTORY_MAGIC              0x48495354UL
#define HISTORY_RECORD_SIZE        32U

typedef struct {
  uint32_t magic;
  uint16_t recordCount;
  uint16_t writeIndex;
  uint32_t nextRecordIndex;
  uint32_t reserved;
} HistoryHeader_t;

static uint8_t flashAvailable;
static HistoryHeader_t historyHeader;
static HistoryRecord_t records[HISTORY_MAX_RECORDS];

static void History_ResetRam(void)
{
  memset(records, 0, sizeof(records));
  historyHeader.magic = HISTORY_MAGIC;
  historyHeader.recordCount = 0;
  historyHeader.writeIndex = 0;
  historyHeader.nextRecordIndex = 1;
  historyHeader.reserved = 0xFFFFFFFFU;
}

static uint32_t History_RecordAddress(uint16_t slot)
{
  return HISTORY_RECORD_BASE_ADDR + ((uint32_t)slot * HISTORY_RECORD_SIZE);
}

static uint8_t History_IsHeaderValid(const HistoryHeader_t *header)
{
  if (header->magic != HISTORY_MAGIC) {
    return 0;
  }

  if ((header->recordCount > HISTORY_MAX_RECORDS) || (header->writeIndex >= HISTORY_MAX_RECORDS)) {
    return 0;
  }

  if (header->nextRecordIndex == 0U) {
    return 0;
  }

  return 1;
}

static uint8_t History_FlushToFlash(void)
{
  uint8_t headerPage[256];

  if (flashAvailable == 0U) {
    return 0;
  }

  memset(headerPage, 0xFF, sizeof(headerPage));
  memcpy(headerPage, &historyHeader, sizeof(historyHeader));

  if (W25Q64_SectorErase(HISTORY_SECTOR_ADDR) != HAL_OK) {
    flashAvailable = 0;
    return 0;
  }

  if (W25Q64_PageProgram(HISTORY_BASE_ADDR, headerPage, sizeof(headerPage)) != HAL_OK) {
    flashAvailable = 0;
    return 0;
  }

  for (uint16_t i = 0; i < HISTORY_MAX_RECORDS; i++) {
    if (W25Q64_PageProgram(History_RecordAddress(i), (uint8_t *)&records[i], sizeof(HistoryRecord_t)) != HAL_OK) {
      flashAvailable = 0;
      return 0;
    }
  }

  return 1;
}

void History_Init(void)
{
  HistoryHeader_t storedHeader;

  flashAvailable = (W25Q64_Init() == HAL_OK) ? 1U : 0U;
  History_ResetRam();

  if (flashAvailable == 0U) {
    return;
  }

  if (W25Q64_ReadData(HISTORY_BASE_ADDR, (uint8_t *)&storedHeader, sizeof(storedHeader)) != HAL_OK) {
    flashAvailable = 0;
    return;
  }

  if (History_IsHeaderValid(&storedHeader) == 0U) {
    (void)History_FlushToFlash();
    return;
  }

  historyHeader = storedHeader;

  for (uint16_t i = 0; i < HISTORY_MAX_RECORDS; i++) {
    if (W25Q64_ReadData(History_RecordAddress(i), (uint8_t *)&records[i], sizeof(HistoryRecord_t)) != HAL_OK) {
      flashAvailable = 0;
      return;
    }
  }
}

uint8_t History_IsAvailable(void)
{
  return flashAvailable;
}

uint8_t History_SaveRecord(const HistoryRecord_t *record)
{
  HistoryRecord_t savedRecord;

  if ((flashAvailable == 0U) || (record == NULL)) {
    return 0;
  }

  savedRecord = *record;
  savedRecord.index = historyHeader.nextRecordIndex++;
  records[historyHeader.writeIndex] = savedRecord;

  historyHeader.writeIndex = (uint16_t)((historyHeader.writeIndex + 1U) % HISTORY_MAX_RECORDS);
  if (historyHeader.recordCount < HISTORY_MAX_RECORDS) {
    historyHeader.recordCount++;
  }

  return History_FlushToFlash();
}

uint8_t History_Clear(void)
{
  if (flashAvailable == 0U) {
    return 0;
  }

  History_ResetRam();
  return History_FlushToFlash();
}

void History_GetStatus(HistoryStatus_t *status)
{
  if (status == NULL) {
    return;
  }

  status->flashAvailable = flashAvailable;
  status->recordCount = historyHeader.recordCount;
  status->writeIndex = historyHeader.writeIndex;
}

uint8_t History_GetRecord(uint16_t chronologicalIndex, HistoryRecord_t *record)
{
  uint16_t slot;

  if ((record == NULL) || (chronologicalIndex >= historyHeader.recordCount)) {
    return 0;
  }

  if (historyHeader.recordCount < HISTORY_MAX_RECORDS) {
    slot = chronologicalIndex;
  } else {
    slot = (uint16_t)((historyHeader.writeIndex + chronologicalIndex) % HISTORY_MAX_RECORDS);
  }

  *record = records[slot];
  return 1;
}

uint8_t History_GetLastRecord(HistoryRecord_t *record)
{
  uint16_t slot;

  if ((record == NULL) || (historyHeader.recordCount == 0U)) {
    return 0;
  }

  slot = (historyHeader.writeIndex == 0U) ? (HISTORY_MAX_RECORDS - 1U) : (historyHeader.writeIndex - 1U);
  *record = records[slot];
  return 1;
}
