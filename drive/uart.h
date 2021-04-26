#ifndef __UART_H
#define	__UART_H

#include "stm32f0xx.h"
#include <stdio.h>

/* USART1 */
#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_GPIO			GPIOA
#define UART1_DIR_GPIO	GPIOA
#define UART1_DIR_GPIO_PIN	GPIO_Pin_11

#define TE485 GPIO_SetBits(UART1_DIR_GPIO, UART1_DIR_GPIO_PIN);
#define RE485 GPIO_ResetBits(UART1_DIR_GPIO, UART1_DIR_GPIO_PIN);

void uart1_init(uint32_t baudrate);
int fputc(int ch, FILE *f);
void UART_send_byte(uint8_t byte);
void UART_Send(uint8_t *Buffer, uint32_t Length);
uint8_t UART_Recive(void);

#endif /* __UART_H */
