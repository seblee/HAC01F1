//电子膨胀阀底层驱动
#ifndef __EEV_H
#define	__EEV_H

#include "stm32f0xx.h"
#include "sys_conf.h"
#define MAX_EXCHOLD_CNT		3u

#define EEV1_MASKBIT		0xf69f
#define EEV2_MASKBIT		0xff87

#define RCC_EEV1        	RCC_AHBPeriph_GPIOA
#define GPIO_PORT_EEV1  	GPIOA
#define GPIO_PIN_EEV1_A		GPIO_Pin_11
#define GPIO_PIN_EEV1_B		GPIO_Pin_6
#define GPIO_PIN_EEV1_C		GPIO_Pin_5
#define GPIO_PIN_EEV1_D		GPIO_Pin_8

#define RCC_EEV2        	RCC_AHBPeriph_GPIOB
#define GPIO_PORT_EEV2  	GPIOB
#define GPIO_PIN_EEV2_A		GPIO_Pin_6
#define GPIO_PIN_EEV2_B		GPIO_Pin_5
#define GPIO_PIN_EEV2_C		GPIO_Pin_4
#define GPIO_PIN_EEV2_D		GPIO_Pin_3

#define EEV1_TIMER			TIM14
#define EEV1_IRQHandler		TIM14_IRQHandler
#define EEV2_TIMER			TIM15
#define EEV2_IRQHandler		TIM15_IRQHandler

enum
{
	DIRNULL = 0,    	//用于停止
	DIROPEN,         	//开阀
	DIRCLOSE			//关阀
};

enum
{
	STOP = 0,
	RUN
};

enum
{
	stepA  = 0,						//四相八拍
	stepAB,
	stepB ,
	stepBC,
	stepC ,
	stepCD,
	stepD ,
	stepDA,
	stepMAX
};

#define STEP_OFFSET		2
//enum
//{
//	stepA  = 0,						//四相八拍
////	stepAB,
//	stepB ,
////	stepBC,
//	stepC ,
////	stepCD,
//	stepD ,
////	stepDA,
//	stepMAX
//};
#pragma pack(1)
struct eevHWcfg							//eev底层配置
{
	uint32_t eevRcc;					
	GPIO_TypeDef * eevPort;
	uint16_t eevApin;
	uint16_t eevBpin;
	uint16_t eevCpin;
	uint16_t eevDpin;
	uint16_t eev_maskbit;			
	uint16_t eev_table[stepMAX];
};

struct eevMovSts						//运动相关的参数
{
	uint8_t last_dir;					//上一次运动方向
	uint8_t now_dir;					//这次
	uint8_t last_status;				//上次运行状态(停止、运行、保持励磁)
	uint8_t now_status;					//本次
	uint8_t beatIndex;					//处于哪个拍(四相八拍)
	uint16_t curPluse;					//当前的脉冲数
	uint16_t movPluse;					//需要移动的脉冲
	uint16_t finalPluse;                //最终的脉冲数，以便计算阀开度
	uint8_t exc_holdCnt;
};

typedef struct eev_pack
{	
	struct eevHWcfg eevHardWareCfg;
	struct eevMovSts eevMoveInfo;
}EEV_Pack;    
#pragma pack()

extern EEV_Pack eevPack[MAX_EEVNUM];
extern void eev_init(void);
extern void eev1_timerInit(uint8_t ppsVal);
extern void eev2_timerInit(uint8_t ppsVal);
extern uint8_t setEevMovInfo(uint16_t dir,uint16_t steps,uint16_t status,uint8_t index);

extern uint16_t getBeatIndex(uint8_t index);
extern uint16_t getFinalPluse(uint8_t index);
extern uint16_t getOpenValveDegree(uint8_t index);
extern uint16_t getPluseNumofOpenValve(uint16_t valve);
extern uint8_t GetEevMovInfo(uint8_t index);
#endif /* __EEV_H */




