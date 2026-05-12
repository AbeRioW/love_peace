#ifndef __GAS_SENSOR_H
#define __GAS_SENSOR_H

#include "stdint.h"

#define GAS_DATA_LEN 9

typedef struct {
    uint16_t tvoc;
    uint16_t ch2o;
    uint16_t co2;
    uint8_t valid;
} GasSensor_Data_t;

void GasSensor_Init(void);
void GasSensor_ProcessData(void);
uint8_t GasSensor_GetCO2(uint16_t *co2);
void GasSensor_UpdateFromUART(uint8_t byte);

#endif
