#include "uart.h"
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include <stdarg.h>
#include <stdio.h>


/* Private function prototypes -----------------------------------------------*/

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
/* Private functions ---------------------------------------------------------*/

/*********************************************************
  * @name   uart1_gpio_init
	* @brief  USART1 GPIO initialization. Both uart port pins and direction control pins are configured
	* @calls  device drivers
  * @called uart1_init()
  * @param  None
  * @retval None
*********************************************************/
static void uart1_gpio_init(void)	
{
	GPIO_InitTypeDef  GPIO_InitStructure;        
					
	//USART1 GPIO and port clock enable
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE );
					
	//USART1 GPIO function remapping
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_1);        
	
	//USART1 GPIO configuration
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);     
	
	//RS485 direction control
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA, ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = UART1_DIR_GPIO_PIN;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
    GPIO_ResetBits(UART1_DIR_GPIO, UART1_DIR_GPIO_PIN);
}

/*********************************************************
  * @name   uart1_nvic_init
	* @brief  USART1 related IRQ vector initialization, include DMA and USART IRQs
	* @calls  device drivers
  * @called uart1_init()
  * @param  None
  * @retval None
*********************************************************/
static void uart1_nvic_init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;  
  
  /* Enable the USART1 tx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	
//  USART_ClearFlag(USART1,USART_FLAG_RXNE|USART_FLAG_TC);  	
}


/*********************************************************
  * @name   uart1_init
	* @brief  General USART1 initialization, after which USART1 is ready to use
	* @calls  uart1_gpio_init()
            uart1_nvic_init()
  * @called com_proc()
  * @param  None
  * @retval None
*********************************************************/
void uart1_init(uint32_t baudrate)//串口初始化函数
{  
	USART_InitTypeDef USART_InitStructure; 
	NVIC_SetPriority(USART1_IRQn, 0); /* (4) */
	NVIC_EnableIRQ(USART1_IRQn); /* (5) */	
	uart1_gpio_init();	       
	USART_InitStructure.USART_BaudRate = baudrate;//设置串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//设置数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//设置停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//设置效验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//设置流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//设置工作模式
	USART_Init(USART1, &USART_InitStructure); //配置入结构体
	USART_Cmd(USART1, ENABLE);//使能串口1	
	uart1_nvic_init();	
}	



