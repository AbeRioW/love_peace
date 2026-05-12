#include "gas_sensor.h"
#include "string.h"

static uint8_t rx_buffer[GAS_DATA_LEN];
static uint8_t rx_index = 0;
static GasSensor_Data_t gas_data = {0};

void GasSensor_Init(void) {
    memset(rx_buffer, 0, GAS_DATA_LEN);
    rx_index = 0;
    gas_data.valid = 0;
}

void GasSensor_UpdateFromUART(uint8_t byte) {
    if (rx_index == 0) {
        if (byte == 0x2C) {
            rx_buffer[rx_index++] = byte;
        }
    } else if (rx_index == 1) {
        if (byte == 0xE4) {
            rx_buffer[rx_index++] = byte;
        } else {
            rx_index = 0;
        }
    } else if (rx_index < GAS_DATA_LEN) {
        rx_buffer[rx_index++] = byte;
        if (rx_index == GAS_DATA_LEN) {
            uint8_t checksum = 0;
            for (int i = 0; i < GAS_DATA_LEN - 1; i++) {
                checksum += rx_buffer[i];
            }
            if (checksum == rx_buffer[GAS_DATA_LEN - 1]) {
                gas_data.tvoc = (rx_buffer[2] << 8) | rx_buffer[3];
                gas_data.ch2o = (rx_buffer[4] << 8) | rx_buffer[5];
                gas_data.co2 = (rx_buffer[6] << 8) | rx_buffer[7];
                gas_data.valid = 1;
            }
            rx_index = 0;
        }
    }
}

uint8_t GasSensor_GetCO2(uint16_t *co2) {
    if (gas_data.valid) {
        *co2 = gas_data.co2;
        return 1;
    }
    return 0;
}
