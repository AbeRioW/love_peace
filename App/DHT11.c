#include "dht11.h"

/**
  * @brief  ��ʪ�ȴ����������źŷ���
  * @param  void
  * @retval None
  */
void DHT11_START(void)
{
    DHT11_GPIO_MODE_SET(0);                         
    
    DHT11_PIN_RESET;                                
    
    HAL_Delay(20);                                  
    
    DHT11_GPIO_MODE_SET(1);                         
}                                                   
 
/**
  * @brief  ��ȡһλ���� 1bit
  * @param  void
  * @retval 0/1
  */
unsigned char DHT11_READ_BIT(void)
{
    uint32_t timeout;

    timeout = 100000;
    while(DHT11_READ_IO == 0)
    {
        if(--timeout == 0) return 0;
    }

    Coarse_delay_us(30);

    if(DHT11_READ_IO)
    {
        while(DHT11_READ_IO);
        return 1;
    }

    return 0;
}
 
/**
  * @brief  ��ȡһ���ֽ����� 1byte / 8bit
  * @param  void
  * @retval temp
  */
unsigned char DHT11_READ_BYTE(void)
{
    uint8_t i,temp = 0;                             
    
    for(i=0; i<8 ;i++)
    {
        temp <<= 1;                                 
        if(DHT11_READ_BIT())                        
        {
            temp |= 1;                              
        }
    }
    return temp;
}
 
/**
  * @brief  ��ȡ��ʪ�ȴ��������� 5byte / 40bit
  * @param  void
  * @retval 0/1
  */
unsigned char DHT11_READ_DATA(DHT11_Data_t *dht_data)
{
    uint8_t i;
    uint8_t data[5] = {0};
    uint32_t timeout;
    
    DHT11_START();
    
    if(DHT11_Check())
    {
        timeout = 100000;
        while(!DHT11_READ_IO)
        {
            if(--timeout == 0) return 1;
        }
        
        timeout = 100000;
        while(DHT11_READ_IO)
        {
            if(--timeout == 0) return 1;
        }
        
        for(i=0; i<5; i++)
        {
            data[i] = DHT11_READ_BYTE();
        }
        
        if(data[0] + data[1] + data[2] + data[3] == data[4])
        {
            dht_data->humidity_int = data[0];
            dht_data->humidity_dec = data[1];
            dht_data->temp_int = data[2];
            dht_data->temp_dec = data[3];
            dht_data->checksum = data[4];
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }
}
 
/**
  * @brief  �����ʪ�ȴ������Ƿ����(���DHT11��Ӧ���ź�)
  * @param  void
  * @retval 0/1
  */
unsigned char DHT11_Check(void)
{

    Coarse_delay_us(40);
    if(DHT11_READ_IO == 0)                          
    {
        return 1;
    }
    else                                            
    {
        return 0;
    }
}
 
/**
  * @brief  ��������ģʽ
  * @param  mode: 0->out, 1->in
  * @retval None
  */
static void DHT11_GPIO_MODE_SET(uint8_t mode)
{
    if(mode)
    {
        /*  ����  */
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = GPIO_PIN_5;                   
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;             
        GPIO_InitStruct.Pull = GPIO_PULLUP;                 
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else 
    {
        /*  ���  */
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.Pin = GPIO_PIN_5;                
        GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;      
        GPIO_InitStructure.Pull = GPIO_PULLUP;              
        GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;    
        HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);
    }
}

void Coarse_delay_us(uint32_t us)
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
    while (delay--)
	{
		;
	}
}



