#include "flash_storage.h"
#include "stm32f1xx_hal.h"

uint32_t FlashStorage_CalculateCRC(FlashStorageData_t *data)
{
    uint32_t crc = 0;
    uint8_t *ptr = (uint8_t *)data;
    uint32_t len = sizeof(FlashStorageData_t) - sizeof(uint32_t); // 不计算CRC本身
    
    for (uint32_t i = 0; i < len; i++) {
        crc = crc ^ ptr[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ 0x04C11DB7;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

uint8_t FlashStorage_ReadData(FlashStorageData_t *data)
{
    FlashStorageData_t *flash_data = (FlashStorageData_t *)FLASH_STORAGE_BASE_ADDR;
    
    // 复制数据
    *data = *flash_data;
    
    // 验证魔法数和CRC
    if (data->magic != FLASH_STORAGE_MAGIC) {
        return 0;
    }
    
    uint32_t calc_crc = FlashStorage_CalculateCRC(data);
    if (data->crc != calc_crc) {
        return 0;
    }
    
    return 1;
}

uint8_t FlashStorage_SaveData(FlashStorageData_t *data)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    uint32_t *data_ptr = (uint32_t *)data;
    uint32_t words = sizeof(FlashStorageData_t) / 4;
    
    // 解锁Flash
    HAL_FLASH_Unlock();
    
    // 擦除页
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = FLASH_STORAGE_BASE_ADDR;
    erase_init.NbPages = 1;
    
    status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return 0;
    }
    
    // 写入数据
    for (uint32_t i = 0; i < words; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
                                    FLASH_STORAGE_BASE_ADDR + i * 4, 
                                    data_ptr[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return 0;
        }
    }
    
    // 锁定Flash
    HAL_FLASH_Lock();
    
    return 1;
}

void FlashStorage_Init(void)
{
    // 初始化函数，目前不需要额外操作
}
