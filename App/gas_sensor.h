#ifndef __GAS_SENSOR_H
#define __GAS_SENSOR_H

#include "stdint.h"

// 从USART2接收一个字节数据
void GasSensor_UpdateFromUART(uint8_t data);

// 初始化气体传感器
void GasSensor_Init(void);

// 获取CO2浓度值
uint16_t GasSensor_GetCO2(void);

// 获取TVOC浓度值
uint16_t GasSensor_GetTVOC(void);

// 获取甲醛浓度值
uint16_t GasSensor_GetFormaldehyde(void);

// 获取接收字节计数
uint32_t GasSensor_GetRxCount(void);

// 获取有效帧计数
uint32_t GasSensor_GetValidCount(void);

#endif
