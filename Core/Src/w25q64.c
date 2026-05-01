#include "w25q64.h"
#include "spi.h"

#define W25Q64_CS_PORT             GPIOA
#define W25Q64_CS_PIN              GPIO_PIN_4

#define W25Q64_CMD_WRITE_ENABLE    0x06
#define W25Q64_CMD_READ_STATUS1    0x05
#define W25Q64_CMD_PAGE_PROGRAM    0x02
#define W25Q64_CMD_READ_DATA       0x03
#define W25Q64_CMD_SECTOR_ERASE    0x20
#define W25Q64_CMD_JEDEC_ID        0x9F

#define W25Q64_SPI_TIMEOUT_MS      100U
#define W25Q64_BUSY_TIMEOUT_MS     1000U

static void W25Q64_Select(void)
{
  HAL_GPIO_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, GPIO_PIN_RESET);
}

static void W25Q64_Unselect(void)
{
  HAL_GPIO_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, GPIO_PIN_SET);
}

static void W25Q64_FillAddress(uint8_t *buf, uint32_t addr)
{
  buf[0] = (uint8_t)(addr >> 16);
  buf[1] = (uint8_t)(addr >> 8);
  buf[2] = (uint8_t)addr;
}

HAL_StatusTypeDef W25Q64_Init(void)
{
  uint32_t id;

  W25Q64_Unselect();
  id = W25Q64_ReadID();

  if ((id == 0U) || (id == 0xFFFFFFU)) {
    return HAL_ERROR;
  }

  return HAL_OK;
}

uint32_t W25Q64_ReadID(void)
{
  uint8_t cmd = W25Q64_CMD_JEDEC_ID;
  uint8_t id[3] = {0};

  W25Q64_Select();
  if (HAL_SPI_Transmit(&hspi1, &cmd, 1, W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
    W25Q64_Unselect();
    return 0;
  }
  if (HAL_SPI_Receive(&hspi1, id, sizeof(id), W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
    W25Q64_Unselect();
    return 0;
  }
  W25Q64_Unselect();

  return ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
}

HAL_StatusTypeDef W25Q64_WriteEnable(void)
{
  uint8_t cmd = W25Q64_CMD_WRITE_ENABLE;

  W25Q64_Select();
  if (HAL_SPI_Transmit(&hspi1, &cmd, 1, W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
    W25Q64_Unselect();
    return HAL_ERROR;
  }
  W25Q64_Unselect();

  return HAL_OK;
}

HAL_StatusTypeDef W25Q64_WaitBusy(void)
{
  uint8_t cmd = W25Q64_CMD_READ_STATUS1;
  uint8_t status = 0;
  uint32_t startTick = HAL_GetTick();

  do {
    W25Q64_Select();
    if (HAL_SPI_Transmit(&hspi1, &cmd, 1, W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
      W25Q64_Unselect();
      return HAL_ERROR;
    }
    if (HAL_SPI_Receive(&hspi1, &status, 1, W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
      W25Q64_Unselect();
      return HAL_ERROR;
    }
    W25Q64_Unselect();

    if ((status & 0x01U) == 0U) {
      return HAL_OK;
    }
  } while ((HAL_GetTick() - startTick) < W25Q64_BUSY_TIMEOUT_MS);

  return HAL_TIMEOUT;
}

HAL_StatusTypeDef W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint32_t len)
{
  uint8_t cmd[4];

  if ((buf == NULL) || (len == 0U)) {
    return HAL_ERROR;
  }

  cmd[0] = W25Q64_CMD_READ_DATA;
  W25Q64_FillAddress(&cmd[1], addr);

  W25Q64_Select();
  if (HAL_SPI_Transmit(&hspi1, cmd, sizeof(cmd), W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
    W25Q64_Unselect();
    return HAL_ERROR;
  }
  if (HAL_SPI_Receive(&hspi1, buf, (uint16_t)len, W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
    W25Q64_Unselect();
    return HAL_ERROR;
  }
  W25Q64_Unselect();

  return HAL_OK;
}

HAL_StatusTypeDef W25Q64_PageProgram(uint32_t addr, const uint8_t *buf, uint32_t len)
{
  uint8_t cmd[4];

  if ((buf == NULL) || (len == 0U) || (len > 256U)) {
    return HAL_ERROR;
  }

  if (((addr & 0xFFU) + len) > 256U) {
    return HAL_ERROR;
  }

  if (W25Q64_WriteEnable() != HAL_OK) {
    return HAL_ERROR;
  }

  cmd[0] = W25Q64_CMD_PAGE_PROGRAM;
  W25Q64_FillAddress(&cmd[1], addr);

  W25Q64_Select();
  if (HAL_SPI_Transmit(&hspi1, cmd, sizeof(cmd), W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
    W25Q64_Unselect();
    return HAL_ERROR;
  }
  if (HAL_SPI_Transmit(&hspi1, (uint8_t *)buf, (uint16_t)len, W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
    W25Q64_Unselect();
    return HAL_ERROR;
  }
  W25Q64_Unselect();

  return W25Q64_WaitBusy();
}

HAL_StatusTypeDef W25Q64_SectorErase(uint32_t addr)
{
  uint8_t cmd[4];

  if (W25Q64_WriteEnable() != HAL_OK) {
    return HAL_ERROR;
  }

  cmd[0] = W25Q64_CMD_SECTOR_ERASE;
  W25Q64_FillAddress(&cmd[1], addr);

  W25Q64_Select();
  if (HAL_SPI_Transmit(&hspi1, cmd, sizeof(cmd), W25Q64_SPI_TIMEOUT_MS) != HAL_OK) {
    W25Q64_Unselect();
    return HAL_ERROR;
  }
  W25Q64_Unselect();

  return W25Q64_WaitBusy();
}
