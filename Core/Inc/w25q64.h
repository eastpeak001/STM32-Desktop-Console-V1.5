#ifndef __W25Q64_H
#define __W25Q64_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

HAL_StatusTypeDef W25Q64_Init(void);
uint32_t W25Q64_ReadID(void);
HAL_StatusTypeDef W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint32_t len);
HAL_StatusTypeDef W25Q64_PageProgram(uint32_t addr, const uint8_t *buf, uint32_t len);
HAL_StatusTypeDef W25Q64_SectorErase(uint32_t addr);
HAL_StatusTypeDef W25Q64_WriteEnable(void);
HAL_StatusTypeDef W25Q64_WaitBusy(void);

#ifdef __cplusplus
}
#endif

#endif /* __W25Q64_H */
