#ifndef __ADC_H
#define	__ADC_H

#include "stm32f0xx.h"
#define MAX_ADBUFEVERY		20
enum
{
		ADC_T1=0,
};
#define NUM_2 10       //滤波次数

enum
{
		AI_SENSOR1 = 0,
//		AI_SENSOR2,
//		AI_SENSOR3,
//		AI_SENSOR4,
		AI_NTC1,
		AI_MAX_CNT
};
#define ADC1_PER AI_MAX_CNT       //通道数
extern uint16_t ADC1ConvertedValue[AI_MAX_CNT];
extern void drv_adc_dma_init(void);
extern void ADCValProcess(uint16_t *ptrADCval,uint16_t *ptrADCbuf,uint8_t index);
#endif /* __ADC_H */
