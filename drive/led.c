
#include "led.h"
/*********************************************************
  * @name   led_init
	* @brief  led gpio and port bank clock initilization
	* @calls  gpio dirvers
  * @called bkg_proc()
  * @param  None
  * @retval None
*********************************************************/
void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = LED1_PIN ;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed =GPIO_Speed_Level_3;
	GPIO_Init(LED1_PORT, &GPIO_InitStruct);
	GPIO_SetBits(LED1_PORT, LED1_PIN);
}

/*********************************************************
  * @name   led_open
	* @brief  led highlight
	* @calls  GPIO_ResetBits()
  * @called None
  * @param  None
  * @retval None
*********************************************************/
void led_open(uint8_t pin_id)
{
	if(pin_id == LED_RUN)
	{
			GPIO_ResetBits(LED1_PORT, LED1_PIN );
	}
	else
	if(pin_id == LED_CTRL)
	{
			GPIO_ResetBits(LED2_PORT, LED2_PIN );
	}
	else
	if(pin_id == LED_ALARM)
	{
			GPIO_ResetBits(LED3_PORT, LED3_PIN );
	}	
	return;
}
/*********************************************************
  * @name   led_close
	* @brief  led shut
	* @calls  GPIO_SetBits()
  * @called None
  * @param  None
  * @retval None
*********************************************************/
void led_close(uint8_t pin_id)
{
	if(pin_id == LED_RUN)
	{
			GPIO_SetBits(LED1_PORT, LED1_PIN );
	}
	else
	if(pin_id == LED_CTRL)
	{
			GPIO_SetBits(LED2_PORT, LED2_PIN );
	}
	else
	if(pin_id == LED_ALARM)
	{
			GPIO_SetBits(LED3_PORT, LED3_PIN );
	}	
	return;
}

/*********************************************************
  * @name   led_toggle
	* @brief  led toggles each time called
	* @calls  GPIO_WriteBit()
  * @called None
  * @param  None
  * @retval None
*********************************************************/
void led_toggle(uint8_t pin_id)
{	
	if(pin_id == LED_RUN)
	{
		GPIO_WriteBit(LED1_PORT, LED1_PIN, 
				(BitAction)((1-GPIO_ReadOutputDataBit(LED1_PORT, LED1_PIN))));
	}
	else
	if(pin_id == LED_CTRL)
	{
		GPIO_WriteBit(LED2_PORT, LED2_PIN, 
				(BitAction)((1-GPIO_ReadOutputDataBit(LED2_PORT, LED2_PIN))));
	}
	else
	if(pin_id == LED_ALARM)
	{
		GPIO_WriteBit(LED3_PORT, LED3_PIN, 
				(BitAction)((1-GPIO_ReadOutputDataBit(LED3_PORT, LED3_PIN))));
	}	
	return;
}
