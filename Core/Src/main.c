/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dht11.h"
#include "oled.h"
#include "adc.h"
#include "esp8266.h"
#include "mqtt_publisher.h"
#include "gas_sensor.h"
#include "control.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
DHT11_Data_t dht11_data = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8_t wifi_try = 0, mqtt_try = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
	  HAL_GPIO_WritePin(GPIOA, FAN_Pin|WATER_Pin, GPIO_PIN_RESET);
  OLED_Init();
  OLED_Clear();
  GasSensor_Init();
  Control_Init();
	
		ESP8266_Init();
	  //WIFI连接 
  while (wifi_try < 5 && !ESP8266_ConnectWiFi())
  {
      wifi_try++;
      HAL_Delay(1000);
  }
	
	if(ESP8266_ConnectCloud()==false)
	{
		  while(1);
	}
	HAL_Delay(5000);
	ESP8266_Clear();
	OLED_Clear();
	

	if(!ESP8266_MQTT_Subscribe(MQTT_TOPIC_POST_REPLY,1))
	{
		  while(1);
	}
	

		if(!ESP8266_MQTT_Subscribe(MQTT_TOPIC_SET,0))
	{
		  while(1);
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    char temp_str[32];
    char hum_str[32];
    char light_str[32];
    char soil_str[32];
    static uint32_t last_mqtt_time = 0;

    uint16_t light_value = ADC1_Read_Average(10);
    uint16_t soil_moisture_value = ADC2_Read_Average(10);

    DHT11_READ_DATA(&dht11_data);
    
    /* 格式化数据 */
    snprintf(temp_str, sizeof(temp_str), "%d.%d", dht11_data.temp_int, dht11_data.temp_dec);
    snprintf(hum_str, sizeof(hum_str), "%d.%d", dht11_data.humidity_int, dht11_data.humidity_dec);
    snprintf(light_str, sizeof(light_str), "%d", light_value);
    snprintf(soil_str, sizeof(soil_str), "%d", soil_moisture_value);
    
    /* 定时MQTT发布数据 */
    if(HAL_GetTick() - last_mqtt_time > 5000) {
        if(Control_GetRegion() == REGION_1) {
            MQTT_Publish_Data("temperature", temp_str);
            MQTT_Publish_Data("humidity", hum_str);
            MQTT_Publish_Data("light", light_str);
            MQTT_Publish_Data("soil", soil_str);
        } else {
            MQTT_Publish_Data("temperature1", temp_str);
            MQTT_Publish_Data("humidity1", hum_str);
            MQTT_Publish_Data("light1", light_str);
            MQTT_Publish_Data("soil1", soil_str);
        }
        last_mqtt_time = HAL_GetTick();
    }
    
    /* 自动模式下的设备控制 */
    if(Control_GetMode() == MODE_AUTO) {
        int16_t temp_threshold = Control_GetThreshold(THRESHOLD_TEMP);
        int16_t light_threshold = Control_GetThreshold(THRESHOLD_LIGHT);
        int16_t soil_threshold = Control_GetThreshold(THRESHOLD_SOIL);
        int16_t humidity_threshold = Control_GetThreshold(THRESHOLD_HUMIDITY);
        
        /* 温度控制：高于阈值启动风扇，低于阈值关闭风扇 */
        if(dht11_data.temp_int >= temp_threshold)
        {
            HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, GPIO_PIN_RESET);
        }
        
        /* 光照控制：大于阈值启动LED，小于阈值关闭LED */
        if(light_value >= light_threshold)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        }
        else
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        }
        
        /* 土壤湿度控制：低于阈值启动水泵浇水，高于阈值关闭水泵 */
        if(soil_moisture_value < soil_threshold)
        {
            HAL_GPIO_WritePin(WATER_GPIO_Port, WATER_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(WATER_GPIO_Port, WATER_Pin, GPIO_PIN_RESET);
        }
        
        /* 空气湿度控制：低于阈值启动蜂鸣器提示，高于阈值关闭 */
        if(dht11_data.humidity_int < humidity_threshold)
        {
            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
        }
    }
    
    /* 处理按键 */
    Control_Process();
    
    /* 处理MQTT消息 */
    ESP8266_ProcessMessages();
    
    OLED_Clear();
    if(Control_GetPage() == PAGE_MAIN) {
        /* 显示传感器数据 */
        OLED_ShowString(0, 0, (uint8_t*)"Region:", 8, 1);
        if(Control_GetRegion() == REGION_1) {
            OLED_ShowString(50, 0, (uint8_t*)"REGION1", 8, 1);
        } else {
            OLED_ShowString(50, 0, (uint8_t*)"REGION2", 8, 1);
        }
        
        OLED_ShowString(0, 8, (uint8_t*)"Mode:", 8, 1);
        if(Control_GetMode() == MODE_AUTO) {
            OLED_ShowString(50, 8, (uint8_t*)"AUTO", 8, 1);
        } else {
            OLED_ShowString(50, 8, (uint8_t*)"MANUAL", 8, 1);
        }

        OLED_ShowString(0, 16, (uint8_t*)"Light:", 8, 1); 
         OLED_ShowNum(50, 16, light_value, 4, 8, 1); 
 
         OLED_ShowString(0, 24, (uint8_t*)"Soil:", 8, 1); 
         OLED_ShowNum(50, 24, soil_moisture_value, 4, 8, 1); 
 
         OLED_ShowString(0, 32, (uint8_t*)"Temp:", 8, 1); 
         OLED_ShowNum(50, 32, dht11_data.temp_int, 2, 8, 1); 
         OLED_ShowString(74, 32, (uint8_t*)".", 8, 1); 
         OLED_ShowNum(80, 32, dht11_data.temp_dec, 1, 8, 1); 
         OLED_ShowString(88, 32, (uint8_t*)"C", 8, 1); 
 
         OLED_ShowString(0, 40, (uint8_t*)"Hum:", 8, 1); 
         OLED_ShowNum(50, 40, dht11_data.humidity_int, 2, 8, 1); 
         OLED_ShowString(74, 40, (uint8_t*)".", 8, 1); 
         OLED_ShowNum(80, 40, dht11_data.humidity_dec, 1, 8, 1); 
         OLED_ShowString(88, 40, (uint8_t*)"%", 8, 1); 
 
         OLED_ShowString(0, 48, (uint8_t*)"CO2:", 8, 1); 
         uint16_t co2_value = 0; 
         if (GasSensor_GetCO2(&co2_value)) { 
             OLED_ShowNum(50, 48, co2_value, 4, 8, 1); 
             OLED_ShowString(90, 48, (uint8_t*)"ppm", 8, 1); 
         } else { 
             OLED_ShowString(50, 48, (uint8_t*)"----", 8, 1); 
         }
    } else if(Control_GetPage() == PAGE_DEVICE_CONTROL) {
        /* 显示设备控制页面 */
        Display_DeviceControlPage();
    } else if(Control_GetPage() == PAGE_THRESHOLD_SETTING) {
        /* 显示阈值设置页面 */
        Display_ThresholdSettingPage();
    }

    OLED_Refresh();

    HAL_Delay(100);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
