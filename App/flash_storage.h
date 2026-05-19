#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#include "stdint.h"

#define FLASH_STORAGE_BASE_ADDR  0x0801F800  // 使用最后一页存储数据
#define FLASH_STORAGE_MAGIC      0x5A5AA5A5  // 魔法数，用于验证数据有效性

typedef struct {
    uint32_t magic;              // 魔法数
    int16_t thresholds[5];       // 阈值数据：温度、光照、土壤、湿度、CO2
    uint32_t crc;                // CRC校验
} FlashStorageData_t;

void FlashStorage_Init(void);
uint8_t FlashStorage_SaveData(FlashStorageData_t *data);
uint8_t FlashStorage_ReadData(FlashStorageData_t *data);
uint32_t FlashStorage_CalculateCRC(FlashStorageData_t *data);

#endif /* __FLASH_STORAGE_H */
