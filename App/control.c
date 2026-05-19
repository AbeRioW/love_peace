#include "control.h"
#include "main.h"
#include "oled.h"
#include "flash_storage.h"
#include <string.h>

static SystemMode_t system_mode = MODE_AUTO;
static Page_t current_page = PAGE_MAIN;
static Device_t selected_device = DEV_BEEP;
static Threshold_t selected_threshold = THRESHOLD_TEMP;
static Region_t current_region = REGION_1;
static uint8_t device_state[DEV_MAX] = {0};
static int16_t thresholds[THRESHOLD_MAX] = {30, 2000, 2000, 80, 2000};
static volatile uint8_t key1_pressed = 0;
static volatile uint8_t key2_pressed = 0;
static volatile uint8_t key3_pressed = 0;
static volatile uint8_t key4_pressed = 0;

static const char *device_names[DEV_MAX] = {
    "BEEP ",
    "LED  ",
    "WATER",
    "FAN  "
};

static const char *threshold_names[THRESHOLD_MAX] = {
    "Temp",
    "Light",
    "Soil",
    "Hum  ",
    "CO2  "
};

static void SaveThresholdsToFlash(void)
{
    FlashStorageData_t data;
    data.magic = FLASH_STORAGE_MAGIC;
    for (int i = 0; i < THRESHOLD_MAX; i++) {
        data.thresholds[i] = thresholds[i];
    }
    data.crc = 0;
    data.crc = FlashStorage_CalculateCRC(&data);
    FlashStorage_SaveData(&data);
}

void Control_Init(void)
{
    system_mode = MODE_AUTO;
    current_page = PAGE_MAIN;
    selected_device = DEV_BEEP;
    selected_threshold = THRESHOLD_TEMP;
    current_region = REGION_1;
    memset(device_state, 0, sizeof(device_state));
    
    FlashStorage_Init();
    
    FlashStorageData_t data;
    if (FlashStorage_ReadData(&data)) {
        // 从flash读取成功，使用保存的阈值
        for (int i = 0; i < THRESHOLD_MAX; i++) {
            thresholds[i] = data.thresholds[i];
        }
    } else {
        // 读取失败，使用默认阈值并保存
        thresholds[THRESHOLD_TEMP] = 30;
        thresholds[THRESHOLD_LIGHT] = 50;
        thresholds[THRESHOLD_SOIL] = 40;
        thresholds[THRESHOLD_HUMIDITY] = 80;
        thresholds[THRESHOLD_CO2] = 2000;
        SaveThresholdsToFlash();
    }
}

SystemMode_t Control_GetMode(void)
{
    return system_mode;
}

void Control_SetMode(SystemMode_t mode)
{
    system_mode = mode;
}

Page_t Control_GetPage(void)
{
    return current_page;
}

int16_t Control_GetThreshold(Threshold_t th)
{
    return thresholds[th];
}

void Control_SetThreshold(Threshold_t th, int16_t value)
{
    if(th < THRESHOLD_MAX) {
        thresholds[th] = value;
    }
}

void Device_SetState(Device_t dev, uint8_t state)
{
    device_state[dev] = state;
    switch(dev) {
        case DEV_BEEP:
            //HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
            break;
        case DEV_LED:
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
            break;
        case DEV_WATER:
            HAL_GPIO_WritePin(WATER_GPIO_Port, WATER_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        case DEV_FAN:
            HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

void Device_Toggle(Device_t dev)
{
    Device_SetState(dev, !device_state[dev]);
}

void Control_Process(void)
{
    if(key1_pressed) {
        key1_pressed = 0;
        if(current_page == PAGE_MAIN) {
            system_mode = (system_mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
        } else {
            current_page = PAGE_MAIN;
        }
    }
    
    if(key2_pressed) {
        key2_pressed = 0;
        if(current_page == PAGE_MAIN) {
            if(system_mode == MODE_AUTO) {
                current_page = PAGE_THRESHOLD_SETTING;
                selected_threshold = THRESHOLD_TEMP;
            } else {
                current_page = PAGE_DEVICE_CONTROL;
                selected_device = DEV_BEEP;
            }
        } else if(current_page == PAGE_DEVICE_CONTROL) {
            selected_device = (selected_device + 1) % DEV_MAX;
        } else if(current_page == PAGE_THRESHOLD_SETTING) {
            selected_threshold = (selected_threshold + 1) % THRESHOLD_MAX;
        }
    }
    
    if(key3_pressed) {
        key3_pressed = 0;
        if(current_page == PAGE_MAIN) {
            Control_ToggleRegion();
        } else if(current_page == PAGE_DEVICE_CONTROL) {
            Device_Toggle(selected_device);
        } else if(current_page == PAGE_THRESHOLD_SETTING) {
            uint8_t changed = 0;
            if(selected_threshold == THRESHOLD_TEMP) {
                if(thresholds[selected_threshold] < 100) {
                    thresholds[selected_threshold]++;
                    changed = 1;
                }
            } else if(selected_threshold == THRESHOLD_LIGHT) {
                if(thresholds[selected_threshold] < 100) {
                    thresholds[selected_threshold] += 5;
                    changed = 1;
                }
            } else if(selected_threshold == THRESHOLD_SOIL) {
                if(thresholds[selected_threshold] < 100) {
                    thresholds[selected_threshold] += 5;
                    changed = 1;
                }
            } else if(selected_threshold == THRESHOLD_HUMIDITY) {
                if(thresholds[selected_threshold] < 100) {
                    thresholds[selected_threshold]++;
                    changed = 1;
                }
            } else if(selected_threshold == THRESHOLD_CO2) {
                if(thresholds[selected_threshold] < 5000) {
                    thresholds[selected_threshold] += 100;
                    changed = 1;
                }
            }
            if(changed) {
                SaveThresholdsToFlash();
            }
        }
    }
    
    if(key4_pressed) {
        key4_pressed = 0;
        if(current_page == PAGE_THRESHOLD_SETTING) {
            uint8_t changed = 0;
            if(selected_threshold == THRESHOLD_TEMP) {
                if(thresholds[selected_threshold] > 0) {
                    thresholds[selected_threshold]--;
                    changed = 1;
                }
            } else if(selected_threshold == THRESHOLD_LIGHT) {
                if(thresholds[selected_threshold] > 0) {
                    thresholds[selected_threshold] -= 5;
                    if(thresholds[selected_threshold] < 0) {
                        thresholds[selected_threshold] = 0;
                    }
                    changed = 1;
                }
            } else if(selected_threshold == THRESHOLD_SOIL) {
                if(thresholds[selected_threshold] > 0) {
                    thresholds[selected_threshold] -= 5;
                    if(thresholds[selected_threshold] < 0) {
                        thresholds[selected_threshold] = 0;
                    }
                    changed = 1;
                }
            } else if(selected_threshold == THRESHOLD_HUMIDITY) {
                if(thresholds[selected_threshold] > 0) {
                    thresholds[selected_threshold]--;
                    changed = 1;
                }
            } else if(selected_threshold == THRESHOLD_CO2) {
                if(thresholds[selected_threshold] > 0) {
                    thresholds[selected_threshold] -= 100;
                    if(thresholds[selected_threshold] < 0) {
                        thresholds[selected_threshold] = 0;
                    }
                    changed = 1;
                }
            }
            if(changed) {
                SaveThresholdsToFlash();
            }
        }
    }
}

void Display_MainPage(void)
{
    OLED_ShowString(0, 0, (uint8_t*)"Mode:", 8, 1);
    if(system_mode == MODE_AUTO) {
        OLED_ShowString(50, 0, (uint8_t*)"AUTO", 8, 1);
    } else {
        OLED_ShowString(50, 0, (uint8_t*)"MANUAL", 8, 1);
    }
}

void Display_DeviceControlPage(void)
{
    OLED_ShowString(0, 0, (uint8_t*)"Device Control", 8, 1);
    
    for(int i = 0; i < DEV_MAX; i++) {
        uint8_t y = 16 + i * 12;
        if(i == selected_device) {
            OLED_ShowString(0, y, (uint8_t*)">", 8, 1);
        }
        OLED_ShowString(10, y, (uint8_t*)device_names[i], 8, 1);
        OLED_ShowString(60, y, (uint8_t*)":", 8, 1);
        if(device_state[i]) {
            OLED_ShowString(70, y, (uint8_t*)"ON ", 8, 1);
        } else {
            OLED_ShowString(70, y, (uint8_t*)"OFF", 8, 1);
        }
    }
}

void Display_ThresholdSettingPage(void)
{
    OLED_ShowString(0, 0, (uint8_t*)"Threshold", 8, 1);
    OLED_ShowString(64, 0, (uint8_t*)"Value", 8, 1);
    
    for(int i = 0; i < THRESHOLD_MAX; i++) {
        uint8_t y = 12 + i * 10;
        if(i == selected_threshold) {
            OLED_ShowString(0, y, (uint8_t*)">", 8, 1);
        }
        OLED_ShowString(10, y, (uint8_t*)threshold_names[i], 8, 1);
        OLED_ShowString(44, y, (uint8_t*)":", 8, 1);
        if(i == THRESHOLD_TEMP) {
            OLED_ShowNum(54, y, thresholds[i], 2, 8, 1);
            OLED_ShowString(70, y, (uint8_t*)"C", 8, 1);
        } else if(i == THRESHOLD_HUMIDITY) {
            OLED_ShowNum(54, y, thresholds[i], 2, 8, 1);
            OLED_ShowString(70, y, (uint8_t*)"%", 8, 1);
        } else if(i == THRESHOLD_LIGHT) {
            OLED_ShowNum(54, y, thresholds[i], 2, 8, 1);
            OLED_ShowString(70, y, (uint8_t*)"%", 8, 1);
        } else if(i == THRESHOLD_SOIL) {
            OLED_ShowNum(54, y, thresholds[i], 2, 8, 1);
            OLED_ShowString(70, y, (uint8_t*)"%", 8, 1);
        } else if(i == THRESHOLD_CO2) {
            OLED_ShowNum(54, y, thresholds[i], 4, 8, 1);
            OLED_ShowString(86, y, (uint8_t*)"ppm", 8, 1);
        }
    }
}

void Control_Key1_IRQ(void)
{
    key1_pressed = 1;
}

void Control_Key2_IRQ(void)
{
    key2_pressed = 1;
}

void Control_Key3_IRQ(void)
{
    key3_pressed = 1;
}

void Control_Key4_IRQ(void)
{
    key4_pressed = 1;
}

Region_t Control_GetRegion(void)
{
    return current_region;
}

void Control_ToggleRegion(void)
{
    current_region = (current_region == REGION_1) ? REGION_2 : REGION_1;
}
