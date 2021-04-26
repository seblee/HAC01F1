#ifndef __LED_H
#define	__LED_H

#include "stm32f0xx.h"
#define LED1_PIN      GPIO_Pin_10
#define LED1_PORT     GPIOB	
#define LED2_PIN      GPIO_Pin_12
#define LED2_PORT     GPIOA
#define LED3_PIN      GPIO_Pin_1
#define LED3_PORT     GPIOB	

enum
{
		LED_RUN = 0,
		LED_CTRL,
		LED_ALARM,
};
void led_init(void);
void led_open(uint8_t pin_id);
void led_close(uint8_t pin_id);
void led_toggle(uint8_t pin_id);
#endif /* __LED_H */
