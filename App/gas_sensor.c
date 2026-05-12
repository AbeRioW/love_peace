#include "gas_sensor.h"
#include "usart.h"
#include "string.h"
#include "stdint.h"

// 气体传感器数据缓冲区
#define GAS_DATA_LEN 9
uint8_t rx_buffer[GAS_DATA_LEN] = {0};
uint8_t rx_index = 0;
uint16_t co2_value = 0xFFFF;
uint16_t tvoc_value = 0xFFFF;
uint16_t formaldehyde_value = 0xFFFF;
uint32_t rx_count = 0;
uint32_t valid_count = 0;

// 从USART2接收一个字节数据
void GasSensor_UpdateFromUART(uint8_t data)
{
    rx_count++;
    rx_buffer[rx_index] = data;
    rx_index++;

    // 检查是否接收到完整的9字节帧
    if (rx_index >= GAS_DATA_LEN)
    {
        // 检查帧头：0x2C, 0xE4
        if (rx_buffer[0] == 0x2C && rx_buffer[1] == 0xE4)
        {
            // 计算校验和
            uint8_t checksum = 0;
            for (int i = 0; i < 8; i++)
            {
                checksum += rx_buffer[i];
            }

            // 验证校验和
            if (checksum == rx_buffer[8])
            {
                valid_count++;
                
                // 解析TVOC（字节2-3）
                tvoc_value = (rx_buffer[2] << 8) | rx_buffer[3];
                
                // 解析甲醛（字节4-5）
                formaldehyde_value = (rx_buffer[4] << 8) | rx_buffer[5];
                
                // 解析CO2（字节6-7）
                co2_value = (rx_buffer[6] << 8) | rx_buffer[7];
            }
        }

        // 重置索引
        rx_index = 0;
        memset(rx_buffer, 0, GAS_DATA_LEN);
    }
}

// 初始化气体传感器
void GasSensor_Init(void)
{
    memset(rx_buffer, 0, GAS_DATA_LEN);
    rx_index = 0;
    co2_value = 0xFFFF;
    tvoc_value = 0xFFFF;
    formaldehyde_value = 0xFFFF;
    rx_count = 0;
    valid_count = 0;
}

// 获取CO2浓度值
uint16_t GasSensor_GetCO2(void)
{
    return co2_value;
}

// 获取TVOC浓度值
uint16_t GasSensor_GetTVOC(void)
{
    return tvoc_value;
}

// 获取甲醛浓度值
uint16_t GasSensor_GetFormaldehyde(void)
{
    return formaldehyde_value;
}

// 获取接收字节计数
uint32_t GasSensor_GetRxCount(void)
{
    return rx_count;
}

// 获取有效帧计数
uint32_t GasSensor_GetValidCount(void)
{
    return valid_count;
}
