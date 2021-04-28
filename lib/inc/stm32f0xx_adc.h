#ifdef STM32F030_CHIP
/**
  ******************************************************************************
  * @file    stm32f0xx_adc.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    23-March-2012
  * @brief   This file contains all the functions prototypes for the ADC firmware 
  *          library
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F0XX_ADC_H
#define __STM32F0XX_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/** @addtogroup STM32F0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup ADC
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  ADC Init structure definition
  */
  
typedef struct
{
  uint32_t ADC_Resolution;                  /*!< Selects the resolution of the conversion.
                                                 This parameter can be a value of @ref ADC_Resolution */

  FunctionalState ADC_ContinuousConvMode;   /*!< Specifies whether the conversion is performed in
                                                 Continuous or Single mode.
                                                 This parameter can be set to ENABLE or DISABLE. */

  uint32_t ADC_ExternalTrigConvEdge;        /*!< Selects the external trigger Edge and enables the
                                                 trigger of a regular group. This parameter can be a value
                                                 of @ref ADC_external_trigger_edge_conversion */

  uint32_t ADC_ExternalTrigConv;            /*!< Defines the external trigger used to start the analog
                                                 to digital conversion of regular channels. This parameter
                                                 can be a value of @ref ADC_external_trigger_sources_for_channels_conversion */

  uint32_t ADC_DataAlign;                   /*!< Specifies whether the ADC data alignment is left or right.
                                                 This parameter can be a value of @ref ADC_data_align */

  uint32_t  ADC_ScanDirection;              /*!< Specifies in which direction the channels will be scanned
                                                 in the sequence. 
                                                 This parameter can be a value of @ref ADC_Scan_Direction */
}ADC_InitTypeDef;


/* Exported constants --------------------------------------------------------*/

/** @defgroup ADC_Exported_Constants
  * @{
  */ 
#define IS_ADC_ALL_PERIPH(PERIPH)                  ((PERIPH) == ADC1)

/** @defgroup ADC_JitterOff
  * @{
  */ 
#define ADC_JitterOff_PCLKDiv2                    ADC_CFGR2_JITOFFDIV2
#define ADC_JitterOff_PCLKDiv4                    ADC_CFGR2_JITOFFDIV4

#define IS_ADC_JITTEROFF(JITTEROFF) (((JITTEROFF) & 0x3FFFFFFF) == (uint32_t)RESET)

/**
  * @}
  */ 
#define ADC_ClockMode_AsynClk                  ((uint32_t)0x00000000)   /*!< ADC Asynchronous clock mode */
#define ADC_ClockMode_SynClkDiv2               ADC_CFGR2_CKMODE_0   /*!<  Synchronous clock mode divided by 2 */
#define ADC_ClockMode_SynClkDiv4               ADC_CFGR2_CKMODE_1   /*!<  Synchronous clock mode divided by 4 */
#define IS_ADC_CLOCKMODE(CLOCK) (((CLOCK) == ADC_ClockMode_AsynClk) ||\
				                        ((CLOCK) == ADC_ClockMode_SynClkDiv2) ||\
				                        ((CLOCK) == ADC_ClockMode_SynClkDiv4))

/** @defgroup ADC_Resolution
  * @{
  */ 
#define ADC_Resolution_12b                         ((uint32_t)0x00000000)
#define ADC_Resolution_10b                         ADC_CFGR1_RES_0
#define ADC_Resolution_8b                          ADC_CFGR1_RES_1
#define ADC_Resolution_6b                          ADC_CFGR1_RES

#define IS_ADC_RESOLUTION(RESOLUTION) (((RESOLUTION) == ADC_Resolution_12b) || \
                                       ((RESOLUTION) == ADC_Resolution_10b) || \
                                       ((RESOLUTION) == ADC_Resolution_8b) || \
                                       ((RESOLUTION) == ADC_Resolution_6b))

/**
  * @}
  */ 

/** @defgroup ADC_external_trigger_edge_conversion 
  * @{
  */ 
#define ADC_ExternalTrigConvEdge_None              ((uint32_t)0x00000000)
#define ADC_ExternalTrigConvEdge_Rising            ADC_CFGR1_EXTEN_0
#define ADC_ExternalTrigConvEdge_Falling           ADC_CFGR1_EXTEN_1
#define ADC_ExternalTrigConvEdge_RisingFalling     ADC_CFGR1_EXTEN

#define IS_ADC_EXT_TRIG_EDGE(EDGE) (((EDGE) == ADC_ExternalTrigConvEdge_None) || \
                                    ((EDGE) == ADC_ExternalTrigConvEdge_Rising) || \
                                    ((EDGE) == ADC_ExternalTrigConvEdge_Falling) || \
                                    ((EDGE) == ADC_ExternalTrigConvEdge_RisingFalling))
/**
  * @}
  */ 

/** @defgroup ADC_external_trigger_sources_for_channels_conversion
  * @{
  */ 

/* TIM1 */
#define ADC_ExternalTrigConv_T1_TRGO               ((uint32_t)0x00000000)
#define ADC_ExternalTrigConv_T1_CC4                ADC_CFGR1_EXTSEL_0

/* TIM2 */
#define ADC_ExternalTrigConv_T2_TRGO               ADC_CFGR1_EXTSEL_1

/* TIM3 */
#define ADC_ExternalTrigConv_T3_TRGO               ((uint32_t)(ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_EXTSEL_1))

/* TIM15 */
#define ADC_ExternalTrigConv_T15_TRGO              ADC_CFGR1_EXTSEL_2

#define IS_ADC_EXTERNAL_TRIG_CONV(CONV) (((CONV) == ADC_ExternalTrigConv_T1_TRGO) || \
                                         ((CONV) == ADC_ExternalTrigConv_T1_CC4)   || \
                                         ((CONV) == ADC_ExternalTrigConv_T2_TRGO)  || \
                                         ((CONV) == ADC_ExternalTrigConv_T3_TRGO)  || \
                                         ((CONV) == ADC_ExternalTrigConv_T15_TRGO)) 
/**
  * @}
  */ 

/** @defgroup ADC_data_align 
  * @{
  */ 
  
#define ADC_DataAlign_Right                        ((uint32_t)0x00000000)
#define ADC_DataAlign_Left                         ADC_CFGR1_ALIGN

#define IS_ADC_DATA_ALIGN(ALIGN) (((ALIGN) == ADC_DataAlign_Right) || \
                                  ((ALIGN) == ADC_DataAlign_Left))
/**
  * @}
  */

/** @defgroup ADC_Scan_Direction 
  * @{
  */ 
  
#define ADC_ScanDirection_Upward                   ((uint32_t)0x00000000)
#define ADC_ScanDirection_Backward                 ADC_CFGR1_SCANDIR

#define IS_ADC_SCAN_DIRECTION(DIRECTION) (((DIRECTION) == ADC_ScanDirection_Upward) || \
                                          ((DIRECTION) == ADC_ScanDirection_Backward))
/**
  * @}
  */ 

/** @defgroup ADC_Scan_Direction 
  * @{
  */ 
  
#define ADC_DMAMode_OneShot                        ((uint32_t)0x00000000)
#define ADC_DMAMode_Circular                       ADC_CFGR1_DMACFG

#define IS_ADC_DMA_MODE(MODE) (((MODE) == ADC_DMAMode_OneShot) || \
                               ((MODE) == ADC_DMAMode_Circular))
/**
  * @}
  */ 
    
/** @defgroup ADC_analog_watchdog_selection 
  * @{
  */ 
  
#define ADC_AnalogWatchdog_Channel_0                 ((uint32_t)0x00000000)
#define ADC_AnalogWatchdog_Channel_1                 ((uint32_t)0x04000000)
#define ADC_AnalogWatchdog_Channel_2                 ((uint32_t)0x08000000)
#define ADC_AnalogWatchdog_Channel_3                 ((uint32_t)0x0C000000)
#define ADC_AnalogWatchdog_Channel_4                 ((uint32_t)0x10000000)
#define ADC_AnalogWatchdog_Channel_5                 ((uint32_t)0x14000000)
#define ADC_AnalogWatchdog_Channel_6                 ((uint32_t)0x18000000)
#define ADC_AnalogWatchdog_Channel_7                 ((uint32_t)0x1C000000)
#define ADC_AnalogWatchdog_Channel_8                 ((uint32_t)0x20000000)
#define ADC_AnalogWatchdog_Channel_9                 ((uint32_t)0x24000000)
#define ADC_AnalogWatchdog_Channel_10                ((uint32_t)0x28000000)
#define ADC_AnalogWatchdog_Channel_11                ((uint32_t)0x2C000000)
#define ADC_AnalogWatchdog_Channel_12                ((uint32_t)0x30000000)
#define ADC_AnalogWatchdog_Channel_13                ((uint32_t)0x34000000)
#define ADC_AnalogWatchdog_Channel_14                ((uint32_t)0x38000000)
#define ADC_AnalogWatchdog_Channel_15                ((uint32_t)0x3C000000)
#define ADC_AnalogWatchdog_Channel_16                ((uint32_t)0x40000000)
#define ADC_AnalogWatchdog_Channel_17                ((uint32_t)0x44000000)
#define ADC_AnalogWatchdog_Channel_18                ((uint32_t)0x48000000)


#define IS_ADC_ANALOG_WATCHDOG_CHANNEL(CHANNEL) (((CHANNEL) == ADC_AnalogWatchdog_Channel_0)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_1)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_2)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_3)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_4)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_5)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_6)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_7)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_8)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_9)  || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_10) || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_11) || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_12) || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_13) || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_14) || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_15) || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_16) || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_17) || \
                                                 ((CHANNEL) == ADC_AnalogWatchdog_Channel_18))
/**
  * @}
  */ 
  
/** @defgroup ADC_sampling_times 
  * @{
  */ 

#define ADC_SampleTime_1_5Cycles                     ((uint32_t)0x00000000)
#define ADC_SampleTime_7_5Cycles                     ((uint32_t)0x00000001)
#define ADC_SampleTime_13_5Cycles                    ((uint32_t)0x00000002)
#define ADC_SampleTime_28_5Cycles                    ((uint32_t)0x00000003)
#define ADC_SampleTime_41_5Cycles                    ((uint32_t)0x00000004)
#define ADC_SampleTime_55_5Cycles                    ((uint32_t)0x00000005)
#define ADC_SampleTime_71_5Cycles                    ((uint32_t)0x00000006)
#define ADC_SampleTime_239_5Cycles                   ((uint32_t)0x00000007)

#define IS_ADC_SAMPLE_TIME(TIME) (((TIME) == ADC_SampleTime_1_5Cycles)   || \
                                  ((TIME) == ADC_SampleTime_7_5Cycles)   || \
                                  ((TIME) == ADC_SampleTime_13_5Cycles)  || \
                                  ((TIME) == ADC_SampleTime_28_5Cycles)  || \
                                  ((TIME) == ADC_SampleTime_41_5Cycles)  || \
                                  ((TIME) == ADC_SampleTime_55_5Cycles)  || \
                                  ((TIME) == ADC_SampleTime_71_5Cycles)  || \
                                  ((TIME) == ADC_SampleTime_239_5Cycles))
/**
  * @}
  */ 

/** @defgroup ADC_thresholds 
  * @{
  */ 
  
#define IS_ADC_THRESHOLD(THRESHOLD) ((THRESHOLD) <= 0xFFF)

/**
  * @}
  */ 

/** @defgroup ADC_channels 
  * @{
  */ 
  
#define ADC_Channel_0                              ADC_CHSELR_CHSEL0
#define ADC_Channel_1                              ADC_CHSELR_CHSEL1
#define ADC_Channel_2                              ADC_CHSELR_CHSEL2
#define ADC_Channel_3                              ADC_CHSELR_CHSEL3
#define ADC_Channel_4                              ADC_CHSELR_CHSEL4
#define ADC_Channel_5                              ADC_CHSELR_CHSEL5
#define ADC_Channel_6                              ADC_CHSELR_CHSEL6
#define ADC_Channel_7                              ADC_CHSELR_CHSEL7
#define ADC_Channel_8                              ADC_CHSELR_CHSEL8
#define ADC_Channel_9                              ADC_CHSELR_CHSEL9
#define ADC_Channel_10                             ADC_CHSELR_CHSEL10
#define ADC_Channel_11                             ADC_CHSELR_CHSEL11
#define ADC_Channel_12                             ADC_CHSELR_CHSEL12
#define ADC_Channel_13                             ADC_CHSELR_CHSEL13
#define ADC_Channel_14                             ADC_CHSELR_CHSEL14
#define ADC_Channel_15                             ADC_CHSELR_CHSEL15
#define ADC_Channel_16                             ADC_CHSELR_CHSEL16
#define ADC_Channel_17                             ADC_CHSELR_CHSEL17
#define ADC_Channel_18                             ADC_CHSELR_CHSEL18

#define ADC_Channel_TempSensor                     ((uint32_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                        ((uint32_t)ADC_Channel_17)
#define ADC_Channel_Vbat                           ((uint32_t)ADC_Channel_18)

#define IS_ADC_CHANNEL(CHANNEL) (((CHANNEL) != (uint32_t)RESET) && (((CHANNEL) & 0xFFF80000) == (uint32_t)RESET))

/**
  * @}
  */ 
  
/** @defgroup ADC_interrupts_definition 
  * @{
  */ 
  
#define ADC_IT_ADRDY                               ADC_IER_ADRDYIE
#define ADC_IT_EOSMP                               ADC_IER_EOSMPIE
#define ADC_IT_EOC                                 ADC_IER_EOCIE
#define ADC_IT_EOSEQ                               ADC_IER_EOSEQIE
#define ADC_IT_OVR                                 ADC_IER_OVRIE
#define ADC_IT_AWD                                 ADC_IER_AWDIE
 
#define IS_ADC_CONFIG_IT(IT) (((IT) != (uint32_t)RESET) && (((IT) & 0xFFFFFF60) == (uint32_t)RESET))

#define IS_ADC_GET_IT(IT) (((IT) == ADC_IT_ADRDY) || ((IT) == ADC_IT_EOSMP) || \
                           ((IT) == ADC_IT_EOC)   || ((IT) == ADC_IT_EOSEQ) || \
                           ((IT) == ADC_IT_OVR)   || ((IT) == ADC_IT_AWD))

#define IS_ADC_CLEAR_IT(IT) (((IT) != (uint32_t)RESET) && (((IT) & 0xFFFFFF60) == (uint32_t)RESET))

/**
  * @}
  */ 

/** @defgroup ADC_flags_definition 
  * @{
  */ 
  
#define ADC_FLAG_ADRDY                             ADC_ISR_ADRDY
#define ADC_FLAG_EOSMP                             ADC_ISR_EOSMP
#define ADC_FLAG_EOC                               ADC_ISR_EOC
#define ADC_FLAG_EOSEQ                             ADC_ISR_EOSEQ
#define ADC_FLAG_OVR                               ADC_ISR_OVR
#define ADC_FLAG_AWD                               ADC_ISR_AWD

#define ADC_FLAG_ADEN                              ((uint32_t)0x01000001)
#define ADC_FLAG_ADDIS                             ((uint32_t)0x01000002)
#define ADC_FLAG_ADSTART                           ((uint32_t)0x01000004)
#define ADC_FLAG_ADSTP                             ((uint32_t)0x01000010)
#define ADC_FLAG_ADCAL                             ((uint32_t)0x81000000) 

#define IS_ADC_CLEAR_FLAG(FLAG) (((FLAG) != (uint32_t)RESET) && (((FLAG) & 0xFFFFFF60) == (uint32_t)RESET))

#define IS_ADC_GET_FLAG(FLAG) (((FLAG) == ADC_FLAG_ADRDY)   || ((FLAG) == ADC_FLAG_EOSMP) || \
                               ((FLAG) == ADC_FLAG_EOC)     || ((FLAG)== ADC_FLAG_EOSEQ) || \
                               ((FLAG) == ADC_FLAG_AWD)     || ((FLAG)== ADC_FLAG_OVR) || \
                               ((FLAG) == ADC_FLAG_ADEN)    || ((FLAG)== ADC_FLAG_ADDIS) || \
                               ((FLAG) == ADC_FLAG_ADSTART) || ((FLAG)== ADC_FLAG_ADSTP) || \
                               ((FLAG) == ADC_FLAG_ADCAL))
/**
  * @}
  */
  
/**
  * @}
  */ 

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/*  Function used to set the ADC configuration to the default reset state *****/
void ADC_DeInit(ADC_TypeDef* ADCx);

/* Initialization and Configuration functions *********************************/ 
void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef* ADC_InitStruct);
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct);
void ADC_ClockModeConfig(ADC_TypeDef* ADCx, uint32_t ADC_ClockMode);
void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_JitterCmd(ADC_TypeDef* ADCx, uint32_t ADC_JitterOff, FunctionalState NewState);

/* Power saving functions *****************************************************/
void ADC_AutoPowerOffCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_WaitModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);

/* Analog Watchdog configuration functions ************************************/
void ADC_AnalogWatchdogCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_AnalogWatchdogThresholdsConfig(ADC_TypeDef* ADCx, uint16_t HighThreshold,uint16_t LowThreshold);
void ADC_AnalogWatchdogSingleChannelConfig(ADC_TypeDef* ADCx, uint32_t ADC_AnalogWatchdog_Channel);
void ADC_AnalogWatchdogSingleChannelCmd(ADC_TypeDef* ADCx, FunctionalState NewState);

/* Temperature Sensor , Vrefint and Vbat management function ******************/
void ADC_TempSensorCmd(FunctionalState NewState);
void ADC_VrefintCmd(FunctionalState NewState);
void ADC_VbatCmd(FunctionalState NewState);

/* Channels Configuration functions *******************************************/
void ADC_ChannelConfig(ADC_TypeDef* ADCx, uint32_t ADC_Channel, uint32_t ADC_SampleTime);
void ADC_ContinuousModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_DiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_OverrunModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
uint32_t ADC_GetCalibrationFactor(ADC_TypeDef* ADCx);
void ADC_StopOfConversion(ADC_TypeDef* ADCx);
void ADC_StartOfConversion(ADC_TypeDef* ADCx);
uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx);

/* Regular Channels DMA Configuration functions *******************************/
void ADC_DMACmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_DMARequestModeConfig(ADC_TypeDef* ADCx, uint32_t ADC_DMARequestMode);

/* Interrupts and flags management functions **********************************/
void ADC_ITConfig(ADC_TypeDef* ADCx, uint32_t ADC_IT, FunctionalState NewState);
FlagStatus ADC_GetFlagStatus(ADC_TypeDef* ADCx, uint32_t ADC_FLAG);
void ADC_ClearFlag(ADC_TypeDef* ADCx, uint32_t ADC_FLAG);
ITStatus ADC_GetITStatus(ADC_TypeDef* ADCx, uint32_t ADC_IT);
void ADC_ClearITPendingBit(ADC_TypeDef* ADCx, uint32_t ADC_IT);

#ifdef __cplusplus
}
#endif

#endif /*__STM32F0XX_ADC_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

#endif

#ifdef GD32E230_CHIP
/**
  ******************************************************************************
  * @file    stm32f10x_adc.h
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    11-March-2011
  * @brief   This file contains all the functions prototypes for the ADC firmware 
  *          library.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_ADC_H
#define __STM32F10x_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/** @addtogroup STM32F10x_StdPeriph_Driver
  * @{
  */

/** @addtogroup ADC
  * @{
  */

/** @defgroup ADC_Exported_Types
  * @{
  */

/** 
  * @brief  ADC Init structure definition  
  */

typedef struct
{
  uint32_t ADC_Mode;                      /*!< Configures the ADC to operate in independent or
                                               dual mode. 
                                               This parameter can be a value of @ref ADC_mode */

  FunctionalState ADC_ScanConvMode;       /*!< Specifies whether the conversion is performed in
                                               Scan (multichannels) or Single (one channel) mode.
                                               This parameter can be set to ENABLE or DISABLE */

  FunctionalState ADC_ContinuousConvMode; /*!< Specifies whether the conversion is performed in
                                               Continuous or Single mode.
                                               This parameter can be set to ENABLE or DISABLE. */

  uint32_t ADC_ExternalTrigConv;          /*!< Defines the external trigger used to start the analog
                                               to digital conversion of regular channels. This parameter
                                               can be a value of @ref ADC_external_trigger_sources_for_regular_channels_conversion */

  uint32_t ADC_DataAlign;                 /*!< Specifies whether the ADC data alignment is left or right.
                                               This parameter can be a value of @ref ADC_data_align */

  uint8_t ADC_NbrOfChannel;               /*!< Specifies the number of ADC channels that will be converted
                                               using the sequencer for regular channel group.
                                               This parameter must range from 1 to 16. */
}ADC_InitTypeDef;
/**
  * @}
  */

/** @defgroup ADC_Exported_Constants
  * @{
  */

#define IS_ADC_ALL_PERIPH(PERIPH) (((PERIPH) == ADC1) || \
                                   ((PERIPH) == ADC2) || \
                                   ((PERIPH) == ADC3))

#define IS_ADC_DMA_PERIPH(PERIPH) (((PERIPH) == ADC1) || \
                                   ((PERIPH) == ADC3))

/** @defgroup ADC_mode 
  * @{
  */

#define ADC_Mode_Independent                       ((uint32_t)0x00000000)
#define ADC_Mode_RegInjecSimult                    ((uint32_t)0x00010000)
#define ADC_Mode_RegSimult_AlterTrig               ((uint32_t)0x00020000)
#define ADC_Mode_InjecSimult_FastInterl            ((uint32_t)0x00030000)
#define ADC_Mode_InjecSimult_SlowInterl            ((uint32_t)0x00040000)
#define ADC_Mode_InjecSimult                       ((uint32_t)0x00050000)
#define ADC_Mode_RegSimult                         ((uint32_t)0x00060000)
#define ADC_Mode_FastInterl                        ((uint32_t)0x00070000)
#define ADC_Mode_SlowInterl                        ((uint32_t)0x00080000)
#define ADC_Mode_AlterTrig                         ((uint32_t)0x00090000)

#define IS_ADC_MODE(MODE) (((MODE) == ADC_Mode_Independent) || \
                           ((MODE) == ADC_Mode_RegInjecSimult) || \
                           ((MODE) == ADC_Mode_RegSimult_AlterTrig) || \
                           ((MODE) == ADC_Mode_InjecSimult_FastInterl) || \
                           ((MODE) == ADC_Mode_InjecSimult_SlowInterl) || \
                           ((MODE) == ADC_Mode_InjecSimult) || \
                           ((MODE) == ADC_Mode_RegSimult) || \
                           ((MODE) == ADC_Mode_FastInterl) || \
                           ((MODE) == ADC_Mode_SlowInterl) || \
                           ((MODE) == ADC_Mode_AlterTrig))
/**
  * @}
  */

/** @defgroup ADC_external_trigger_sources_for_regular_channels_conversion 
  * @{
  */

#define ADC_ExternalTrigConv_T1_CC1                ((uint32_t)0x00000000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_T1_CC2                ((uint32_t)0x00020000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_T2_CC2                ((uint32_t)0x00060000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_T3_TRGO               ((uint32_t)0x00080000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_T4_CC4                ((uint32_t)0x000A0000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO    ((uint32_t)0x000C0000) /*!< For ADC1 and ADC2 */

#define ADC_ExternalTrigConv_T1_CC3                ((uint32_t)0x00040000) /*!< For ADC1, ADC2 and ADC3 */
#define ADC_ExternalTrigConv_None                  ((uint32_t)0x000E0000) /*!< For ADC1, ADC2 and ADC3 */

#define ADC_ExternalTrigConv_T3_CC1                ((uint32_t)0x00000000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T2_CC3                ((uint32_t)0x00020000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T8_CC1                ((uint32_t)0x00060000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T8_TRGO               ((uint32_t)0x00080000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T5_CC1                ((uint32_t)0x000A0000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T5_CC3                ((uint32_t)0x000C0000) /*!< For ADC3 only */

#define IS_ADC_EXT_TRIG(REGTRIG) (((REGTRIG) == ADC_ExternalTrigConv_T1_CC1) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T1_CC2) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T1_CC3) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T2_CC2) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T3_TRGO) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T4_CC4) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_None) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T3_CC1) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T2_CC3) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T8_CC1) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T8_TRGO) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T5_CC1) || \
                                  ((REGTRIG) == ADC_ExternalTrigConv_T5_CC3))
/**
  * @}
  */

/** @defgroup ADC_data_align 
  * @{
  */

#define ADC_DataAlign_Right                        ((uint32_t)0x00000000)
#define ADC_DataAlign_Left                         ((uint32_t)0x00000800)
#define IS_ADC_DATA_ALIGN(ALIGN) (((ALIGN) == ADC_DataAlign_Right) || \
                                  ((ALIGN) == ADC_DataAlign_Left))
/**
  * @}
  */

/** @defgroup ADC_channels 
  * @{
  */

#define ADC_Channel_0                               ((uint8_t)0x00)
#define ADC_Channel_1                               ((uint8_t)0x01)
#define ADC_Channel_2                               ((uint8_t)0x02)
#define ADC_Channel_3                               ((uint8_t)0x03)
#define ADC_Channel_4                               ((uint8_t)0x04)
#define ADC_Channel_5                               ((uint8_t)0x05)
#define ADC_Channel_6                               ((uint8_t)0x06)
#define ADC_Channel_7                               ((uint8_t)0x07)
#define ADC_Channel_8                               ((uint8_t)0x08)
#define ADC_Channel_9                               ((uint8_t)0x09)
#define ADC_Channel_10                              ((uint8_t)0x0A)
#define ADC_Channel_11                              ((uint8_t)0x0B)
#define ADC_Channel_12                              ((uint8_t)0x0C)
#define ADC_Channel_13                              ((uint8_t)0x0D)
#define ADC_Channel_14                              ((uint8_t)0x0E)
#define ADC_Channel_15                              ((uint8_t)0x0F)
#define ADC_Channel_16                              ((uint8_t)0x10)
#define ADC_Channel_17                              ((uint8_t)0x11)

#define ADC_Channel_TempSensor                      ((uint8_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                         ((uint8_t)ADC_Channel_17)

#define IS_ADC_CHANNEL(CHANNEL) (((CHANNEL) == ADC_Channel_0) || ((CHANNEL) == ADC_Channel_1) || \
                                 ((CHANNEL) == ADC_Channel_2) || ((CHANNEL) == ADC_Channel_3) || \
                                 ((CHANNEL) == ADC_Channel_4) || ((CHANNEL) == ADC_Channel_5) || \
                                 ((CHANNEL) == ADC_Channel_6) || ((CHANNEL) == ADC_Channel_7) || \
                                 ((CHANNEL) == ADC_Channel_8) || ((CHANNEL) == ADC_Channel_9) || \
                                 ((CHANNEL) == ADC_Channel_10) || ((CHANNEL) == ADC_Channel_11) || \
                                 ((CHANNEL) == ADC_Channel_12) || ((CHANNEL) == ADC_Channel_13) || \
                                 ((CHANNEL) == ADC_Channel_14) || ((CHANNEL) == ADC_Channel_15) || \
                                 ((CHANNEL) == ADC_Channel_16) || ((CHANNEL) == ADC_Channel_17))
/**
  * @}
  */

/** @defgroup ADC_sampling_time 
  * @{
  */

#define ADC_SampleTime_1Cycles5                    ((uint8_t)0x00)
#define ADC_SampleTime_7Cycles5                    ((uint8_t)0x01)
#define ADC_SampleTime_13Cycles5                   ((uint8_t)0x02)
#define ADC_SampleTime_28Cycles5                   ((uint8_t)0x03)
#define ADC_SampleTime_41Cycles5                   ((uint8_t)0x04)
#define ADC_SampleTime_55Cycles5                   ((uint8_t)0x05)
#define ADC_SampleTime_71Cycles5                   ((uint8_t)0x06)
#define ADC_SampleTime_239Cycles5                  ((uint8_t)0x07)
#define IS_ADC_SAMPLE_TIME(TIME) (((TIME) == ADC_SampleTime_1Cycles5) || \
                                  ((TIME) == ADC_SampleTime_7Cycles5) || \
                                  ((TIME) == ADC_SampleTime_13Cycles5) || \
                                  ((TIME) == ADC_SampleTime_28Cycles5) || \
                                  ((TIME) == ADC_SampleTime_41Cycles5) || \
                                  ((TIME) == ADC_SampleTime_55Cycles5) || \
                                  ((TIME) == ADC_SampleTime_71Cycles5) || \
                                  ((TIME) == ADC_SampleTime_239Cycles5))
/**
  * @}
  */

/** @defgroup ADC_external_trigger_sources_for_injected_channels_conversion 
  * @{
  */

#define ADC_ExternalTrigInjecConv_T2_TRGO           ((uint32_t)0x00002000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigInjecConv_T2_CC1            ((uint32_t)0x00003000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigInjecConv_T3_CC4            ((uint32_t)0x00004000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigInjecConv_T4_TRGO           ((uint32_t)0x00005000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4 ((uint32_t)0x00006000) /*!< For ADC1 and ADC2 */

#define ADC_ExternalTrigInjecConv_T1_TRGO           ((uint32_t)0x00000000) /*!< For ADC1, ADC2 and ADC3 */
#define ADC_ExternalTrigInjecConv_T1_CC4            ((uint32_t)0x00001000) /*!< For ADC1, ADC2 and ADC3 */
#define ADC_ExternalTrigInjecConv_None              ((uint32_t)0x00007000) /*!< For ADC1, ADC2 and ADC3 */

#define ADC_ExternalTrigInjecConv_T4_CC3            ((uint32_t)0x00002000) /*!< For ADC3 only */
#define ADC_ExternalTrigInjecConv_T8_CC2            ((uint32_t)0x00003000) /*!< For ADC3 only */
#define ADC_ExternalTrigInjecConv_T8_CC4            ((uint32_t)0x00004000) /*!< For ADC3 only */
#define ADC_ExternalTrigInjecConv_T5_TRGO           ((uint32_t)0x00005000) /*!< For ADC3 only */
#define ADC_ExternalTrigInjecConv_T5_CC4            ((uint32_t)0x00006000) /*!< For ADC3 only */

#define IS_ADC_EXT_INJEC_TRIG(INJTRIG) (((INJTRIG) == ADC_ExternalTrigInjecConv_T1_TRGO) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T1_CC4) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T2_TRGO) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T2_CC1) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T3_CC4) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T4_TRGO) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_None) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T4_CC3) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T8_CC2) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T8_CC4) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T5_TRGO) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T5_CC4))
/**
  * @}
  */

/** @defgroup ADC_injected_channel_selection 
  * @{
  */

#define ADC_InjectedChannel_1                       ((uint8_t)0x14)
#define ADC_InjectedChannel_2                       ((uint8_t)0x18)
#define ADC_InjectedChannel_3                       ((uint8_t)0x1C)
#define ADC_InjectedChannel_4                       ((uint8_t)0x20)
#define IS_ADC_INJECTED_CHANNEL(CHANNEL) (((CHANNEL) == ADC_InjectedChannel_1) || \
                                          ((CHANNEL) == ADC_InjectedChannel_2) || \
                                          ((CHANNEL) == ADC_InjectedChannel_3) || \
                                          ((CHANNEL) == ADC_InjectedChannel_4))
/**
  * @}
  */

/** @defgroup ADC_analog_watchdog_selection 
  * @{
  */

#define ADC_AnalogWatchdog_SingleRegEnable         ((uint32_t)0x00800200)
#define ADC_AnalogWatchdog_SingleInjecEnable       ((uint32_t)0x00400200)
#define ADC_AnalogWatchdog_SingleRegOrInjecEnable  ((uint32_t)0x00C00200)
#define ADC_AnalogWatchdog_AllRegEnable            ((uint32_t)0x00800000)
#define ADC_AnalogWatchdog_AllInjecEnable          ((uint32_t)0x00400000)
#define ADC_AnalogWatchdog_AllRegAllInjecEnable    ((uint32_t)0x00C00000)
#define ADC_AnalogWatchdog_None                    ((uint32_t)0x00000000)

#define IS_ADC_ANALOG_WATCHDOG(WATCHDOG) (((WATCHDOG) == ADC_AnalogWatchdog_SingleRegEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_SingleInjecEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_SingleRegOrInjecEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_AllRegEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_AllInjecEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_AllRegAllInjecEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_None))
/**
  * @}
  */

/** @defgroup ADC_interrupts_definition 
  * @{
  */

#define ADC_IT_EOC                                 ((uint16_t)0x0220)
#define ADC_IT_AWD                                 ((uint16_t)0x0140)
#define ADC_IT_JEOC                                ((uint16_t)0x0480)

#define IS_ADC_IT(IT) ((((IT) & (uint16_t)0xF81F) == 0x00) && ((IT) != 0x00))

#define IS_ADC_GET_IT(IT) (((IT) == ADC_IT_EOC) || ((IT) == ADC_IT_AWD) || \
                           ((IT) == ADC_IT_JEOC))
/**
  * @}
  */

/** @defgroup ADC_flags_definition 
  * @{
  */

#define ADC_FLAG_AWD                               ((uint8_t)0x01)
#define ADC_FLAG_EOC                               ((uint8_t)0x02)
#define ADC_FLAG_JEOC                              ((uint8_t)0x04)
#define ADC_FLAG_JSTRT                             ((uint8_t)0x08)
#define ADC_FLAG_STRT                              ((uint8_t)0x10)
#define IS_ADC_CLEAR_FLAG(FLAG) ((((FLAG) & (uint8_t)0xE0) == 0x00) && ((FLAG) != 0x00))
#define IS_ADC_GET_FLAG(FLAG) (((FLAG) == ADC_FLAG_AWD) || ((FLAG) == ADC_FLAG_EOC) || \
                               ((FLAG) == ADC_FLAG_JEOC) || ((FLAG)== ADC_FLAG_JSTRT) || \
                               ((FLAG) == ADC_FLAG_STRT))
/**
  * @}
  */

/** @defgroup ADC_thresholds 
  * @{
  */

#define IS_ADC_THRESHOLD(THRESHOLD) ((THRESHOLD) <= 0xFFF)

/**
  * @}
  */

/** @defgroup ADC_injected_offset 
  * @{
  */

#define IS_ADC_OFFSET(OFFSET) ((OFFSET) <= 0xFFF)

/**
  * @}
  */

/** @defgroup ADC_injected_length 
  * @{
  */

#define IS_ADC_INJECTED_LENGTH(LENGTH) (((LENGTH) >= 0x1) && ((LENGTH) <= 0x4))

/**
  * @}
  */

/** @defgroup ADC_injected_rank 
  * @{
  */

#define IS_ADC_INJECTED_RANK(RANK) (((RANK) >= 0x1) && ((RANK) <= 0x4))

/**
  * @}
  */ 


/** @defgroup ADC_regular_length 
  * @{
  */

#define IS_ADC_REGULAR_LENGTH(LENGTH) (((LENGTH) >= 0x1) && ((LENGTH) <= 0x10))
/**
  * @}
  */

/** @defgroup ADC_regular_rank 
  * @{
  */

#define IS_ADC_REGULAR_RANK(RANK) (((RANK) >= 0x1) && ((RANK) <= 0x10))

/**
  * @}
  */

/** @defgroup ADC_regular_discontinuous_mode_number 
  * @{
  */

#define IS_ADC_REGULAR_DISC_NUMBER(NUMBER) (((NUMBER) >= 0x1) && ((NUMBER) <= 0x8))

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup ADC_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup ADC_Exported_Functions
  * @{
  */

void ADC_DeInit(ADC_TypeDef* ADCx);
void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef* ADC_InitStruct);
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct);
void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_DMACmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_ITConfig(ADC_TypeDef* ADCx, uint16_t ADC_IT, FunctionalState NewState);
void ADC_ResetCalibration(ADC_TypeDef* ADCx);
FlagStatus ADC_GetResetCalibrationStatus(ADC_TypeDef* ADCx);
void ADC_StartCalibration(ADC_TypeDef* ADCx);
FlagStatus ADC_GetCalibrationStatus(ADC_TypeDef* ADCx);
void ADC_SoftwareStartConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
FlagStatus ADC_GetSoftwareStartConvStatus(ADC_TypeDef* ADCx);
void ADC_DiscModeChannelCountConfig(ADC_TypeDef* ADCx, uint8_t Number);
void ADC_DiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_RegularChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);
void ADC_ExternalTrigConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx);
uint32_t ADC_GetDualModeConversionValue(void);
void ADC_AutoInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_InjectedDiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_ExternalTrigInjectedConvConfig(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConv);
void ADC_ExternalTrigInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_SoftwareStartInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
FlagStatus ADC_GetSoftwareStartInjectedConvCmdStatus(ADC_TypeDef* ADCx);
void ADC_InjectedChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);
void ADC_InjectedSequencerLengthConfig(ADC_TypeDef* ADCx, uint8_t Length);
void ADC_SetInjectedOffset(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel, uint16_t Offset);
uint16_t ADC_GetInjectedConversionValue(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel);
void ADC_AnalogWatchdogCmd(ADC_TypeDef* ADCx, uint32_t ADC_AnalogWatchdog);
void ADC_AnalogWatchdogThresholdsConfig(ADC_TypeDef* ADCx, uint16_t HighThreshold, uint16_t LowThreshold);
void ADC_AnalogWatchdogSingleChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel);
void ADC_TempSensorVrefintCmd(FunctionalState NewState);
FlagStatus ADC_GetFlagStatus(ADC_TypeDef* ADCx, uint8_t ADC_FLAG);
void ADC_ClearFlag(ADC_TypeDef* ADCx, uint8_t ADC_FLAG);
ITStatus ADC_GetITStatus(ADC_TypeDef* ADCx, uint16_t ADC_IT);
void ADC_ClearITPendingBit(ADC_TypeDef* ADCx, uint16_t ADC_IT);

#ifdef __cplusplus
}
#endif

#endif /*__STM32F10x_ADC_H */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

#endif
