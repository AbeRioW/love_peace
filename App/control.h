#ifndef __CONTROL_H
#define __CONTROL_H

#include "stdint.h"

typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL
} SystemMode_t;

typedef enum {
    PAGE_MAIN = 0,
    PAGE_DEVICE_CONTROL,
    PAGE_THRESHOLD_SETTING
} Page_t;

typedef enum {
    DEV_BEEP = 0,
    DEV_LED,
    DEV_WATER,
    DEV_FAN,
    DEV_MAX
} Device_t;

typedef enum {
    THRESHOLD_TEMP = 0,
    THRESHOLD_LIGHT,
    THRESHOLD_SOIL,
    THRESHOLD_HUMIDITY,
    THRESHOLD_CO2,
    THRESHOLD_MAX
} Threshold_t;

typedef enum {
    REGION_1 = 0,
    REGION_2
} Region_t;

void Control_Init(void);
void Control_Process(void);
void Control_Key1_IRQ(void);
void Control_Key2_IRQ(void);
void Control_Key3_IRQ(void);
void Control_Key4_IRQ(void);
SystemMode_t Control_GetMode(void);
void Control_SetMode(SystemMode_t mode);
Page_t Control_GetPage(void);
void Display_MainPage(void);
void Display_DeviceControlPage(void);
void Display_ThresholdSettingPage(void);
int16_t Control_GetThreshold(Threshold_t th);
void Control_SetThreshold(Threshold_t th, int16_t value);
Region_t Control_GetRegion(void);
void Control_ToggleRegion(void);

#endif
