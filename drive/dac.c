#include "dac.h"

static void dac_gpio_config(void);
static void dac_tim_config(void);
static void dac_dma_init(void);

//const uint16_t sinewave[32] = {
//                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
//                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909, 
//                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};	
const uint16_t sinewave[32] = {1638,	1958,	2265,	2548,	2796,	3000,	3152,	3245,	3276,	3245,	3152,	3000,	2796,	2548,	2265,	1958,	1638,	1318,	1011,	728,	479,	276,	124,	31,	0,	31,	124,	276,	479,	728,	1011,	1318,
};
											
											
void dac_init(void)
{
   // dac_gpio_config();
   // dac_tim_config();
	//  dac_dma_init();
}

static void dac_gpio_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* DMA1 clock enable (to be used with DAC) */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* GPIOA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  /* Configure PA.04 (DAC_OUT1) as analog */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void dac_tim_config(void)
{    
	  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    /* Time base configuration */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
    TIM_TimeBaseStructure.TIM_Period = 0xc;
    TIM_TimeBaseStructure.TIM_Prescaler = 0x0;       
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    /* TIM2 TRGO selection */
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
    
    /* TIM2 enable counter */
    TIM_Cmd(TIM2, ENABLE);
}

static void dac_dma_init(void)
{
    DMA_InitTypeDef            DMA_InitStructure;
	  DAC_InitTypeDef            DAC_InitStructure;
	  DAC_DeInit(); 
    
    /* DAC channel1 Configuration */
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    
    /* DMA channel3 Configuration */
    DMA_DeInit(DMA1_Channel3); 
    DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_ADDRESS;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&sinewave;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 32;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    
    /* Enable DMA1 Channel3 */
    DMA_Cmd(DMA1_Channel3, ENABLE);
    
    /* DAC Channel1 Init */
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);
    
    /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
       automatically connected to the DAC converter. */
    DAC_Cmd(DAC_Channel_1, ENABLE);
    
    /* Enable DMA for DAC Channel1 */
    DAC_DMACmd(DAC_Channel_1, ENABLE);
}
