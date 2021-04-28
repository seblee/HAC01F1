#ifdef STM32F030_CHIP
/**
  ******************************************************************************
  * @file    stm32f0xx_adc.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    23-March-2012
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the Analog to Digital Convertor (ADC) peripheral:
  *           + Initialization and Configuration
  *           + Power saving
  *           + Analog Watchdog configuration
  *           + Temperature Sensor, Vrefint (Internal Reference Voltage) and 
  *             Vbat (Voltage battery) management 
  *           + ADC Channels Configuration
  *           + ADC Channels DMA Configuration
  *           + Interrupts and flags management
  *
  *  @verbatim
================================================================================
                      ##### How to use this driver #####
================================================================================
    [..]
    (#) Enable the ADC interface clock using 
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
    (#) ADC pins configuration
       (++) Enable the clock for the ADC GPIOs using the following function:
            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOx, ENABLE);   
       (++) Configure these ADC pins in analog mode using GPIO_Init();  
    (#) Configure the ADC conversion resolution, data alignment, external
        trigger and edge, scan direction and Enable/Disable the continuous mode
        using the ADC_Init() function.
    (#) Activate the ADC peripheral using ADC_Cmd() function.

    *** ADC channels group configuration ***
    ============================================
    [..] 
    (+) To configure the ADC channels features, use ADC_Init() and 
        ADC_ChannelConfig() functions.
    (+) To activate the continuous mode, use the ADC_ContinuousModeCmd()
        function.
    (+) To activate the Discontinuous mode, use the ADC_DiscModeCmd() functions. 
    (+) To activate the overrun mode, use the ADC_OverrunModeCmd() functions.
    (+) To activate the calibration mode, use the ADC_GetCalibrationFactor() functions.
    (+) To read the ADC converted values, use the ADC_GetConversionValue()
        function.

    *** DMA for ADC channels features configuration ***
    =============================================================
    [..] 
    (+) To enable the DMA mode for ADC channels group, use the ADC_DMACmd() function.
    (+) To configure the DMA transfer request, use ADC_DMARequestModeConfig() function.

  *  @endverbatim
  *
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_adc.h"
#include "stm32f0xx_rcc.h"

/** @addtogroup STM32F0xx_StdPeriph_Driver
  * @{
  */

/** @defgroup ADC 
  * @brief ADC driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* ADC CFGR mask */
#define CFGR1_CLEAR_MASK           ((uint32_t)0xFFFFD203)

/* Calibration time out */
#define CALIBRATION_TIMEOUT       ((uint32_t)0x0000F000)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup ADC_Private_Functions
  * @{
  */

/** @defgroup ADC_Group1 Initialization and Configuration functions
 *  @brief   Initialization and Configuration functions 
 *
@verbatim
 ===============================================================================
          ##### Initialization and Configuration functions #####
 ===============================================================================
    [..] This section provides functions allowing to:
        (+) Initialize and configure the ADC Prescaler
        (+) ADC Conversion Resolution (12bit..6bit)
        (+) ADC Continuous Conversion Mode (Continuous or Single conversion)
        (+) External trigger Edge and source 
        (+) Converted data alignment (left or right)
        (+) The direction in which the channels will be scanned in the sequence
        (+) Enable or disable the ADC peripheral
   
@endverbatim
  * @{
  */

/**
  * @brief  Deinitializes ADC1 peripheral registers to their default reset values.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval None
  */
void ADC_DeInit(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));

  if(ADCx == ADC1)
  {
    /* Enable ADC1 reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* Release ADC1 from reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
  }
}

/**
  * @brief  Initializes the ADCx peripheral according to the specified parameters
  *         in the ADC_InitStruct.
  * @note   This function is used to configure the global features of the ADC ( 
  *         Resolution, Data Alignment, continuous mode activation, External 
  *         trigger source and edge, Sequence Scan Direction).   
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_InitStruct: pointer to an ADC_InitTypeDef structure that contains 
  *         the configuration information for the specified ADC peripheral.
  * @retval None
  */
void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef* ADC_InitStruct)
{
  uint32_t tmpreg = 0;

  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_RESOLUTION(ADC_InitStruct->ADC_Resolution));
  assert_param(IS_FUNCTIONAL_STATE(ADC_InitStruct->ADC_ContinuousConvMode));
  assert_param(IS_ADC_EXT_TRIG_EDGE(ADC_InitStruct->ADC_ExternalTrigConvEdge));
  assert_param(IS_ADC_EXTERNAL_TRIG_CONV(ADC_InitStruct->ADC_ExternalTrigConv));
  assert_param(IS_ADC_DATA_ALIGN(ADC_InitStruct->ADC_DataAlign));
  assert_param(IS_ADC_SCAN_DIRECTION(ADC_InitStruct->ADC_ScanDirection)); 

  /* Get the ADCx CFGR value */
  tmpreg = ADCx->CFGR1;

  /* Clear SCANDIR, RES[1:0], ALIGN, EXTSEL[2:0], EXTEN[1:0] and CONT bits */
  tmpreg &= CFGR1_CLEAR_MASK;

  /*---------------------------- ADCx CFGR Configuration ---------------------*/

  /* Set RES[1:0] bits according to ADC_Resolution value */
  /* Set CONT bit according to ADC_ContinuousConvMode value */
  /* Set EXTEN[1:0] bits according to ADC_ExternalTrigConvEdge value */
  /* Set EXTSEL[2:0] bits according to ADC_ExternalTrigConv value */
  /* Set ALIGN bit according to ADC_DataAlign value */
  /* Set SCANDIR bit according to ADC_ScanDirection value */
 
  tmpreg  |= (uint32_t)(ADC_InitStruct->ADC_Resolution | ((uint32_t)(ADC_InitStruct->ADC_ContinuousConvMode) << 13) |
             ADC_InitStruct->ADC_ExternalTrigConvEdge | ADC_InitStruct->ADC_ExternalTrigConv |
             ADC_InitStruct->ADC_DataAlign | ADC_InitStruct->ADC_ScanDirection);

  /* Write to ADCx CFGR */
  ADCx->CFGR1 = tmpreg;
}

/**
  * @brief  Fills each ADC_InitStruct member with its default value.
  * @note   This function is used to initialize the global features of the ADC ( 
  *         Resolution, Data Alignment, continuous mode activation, External 
  *         trigger source and edge, Sequence Scan Direction).
  * @param  ADC_InitStruct: pointer to an ADC_InitTypeDef structure which will 
  *         be initialized.
  * @retval None
  */
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct)
{
  /* Reset ADC init structure parameters values */
  /* Initialize the ADC_Resolution member */
  ADC_InitStruct->ADC_Resolution = ADC_Resolution_12b;

   /* Initialize the ADC_ContinuousConvMode member */
  ADC_InitStruct->ADC_ContinuousConvMode = DISABLE;

  /* Initialize the ADC_ExternalTrigConvEdge member */
  ADC_InitStruct->ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

  /* Initialize the ADC_ExternalTrigConv member */
  ADC_InitStruct->ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO;

  /* Initialize the ADC_DataAlign member */
  ADC_InitStruct->ADC_DataAlign = ADC_DataAlign_Right;

  /* Initialize the ADC_ScanDirection member */
  ADC_InitStruct->ADC_ScanDirection = ADC_ScanDirection_Upward;
}

/**
  * @brief  Enables or disables the specified ADC peripheral.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  NewState: new state of the ADCx peripheral. 
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Set the ADEN bit to Enable the ADC peripheral */
    ADCx->CR |= (uint32_t)ADC_CR_ADEN;
  }
  else
  {
    /* Set the ADDIS to Disable the ADC peripheral */
    ADCx->CR |= (uint32_t)ADC_CR_ADDIS;
  }
}
/**
  * @brief  Configure the ADC to either be clocked by the asynchronous clock(which is
  *         independent, the dedicated 14MHz clock) or the synchronous clock derived from
  *         the APB clock of the ADC bus interface divided by 2 or 4
  * @note   This function can be called only when ADC is disabled.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  ADC_ClockMode: This parameter can be :
  *            @arg ADC_ClockMode_AsynClk: ADC clocked by the dedicated 14MHz clock
  *            @arg ADC_ClockMode_SynClkDiv2: ADC clocked by PCLK/2
  *            @arg ADC_ClockMode_SynClkDiv4: ADC clocked by PCLK/4  
  * @retval None
  */
void ADC_ClockModeConfig(ADC_TypeDef* ADCx, uint32_t ADC_ClockMode)
{
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CLOCKMODE(ADC_ClockMode));
    ADCx->CFGR2 = (uint32_t)ADC_ClockMode;
}
/**
  * @brief  Enables or disables the jitter when the ADC is clocked by PCLK div2
  *         or div4
  * @note   This function is obsolete and maintained for legacy purpose only. ADC_ClockModeConfig()
  *         function should be used instead.  
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  ADC_JitterOff: This parameter can be :
  *            @arg ADC_JitterOff_PCLKDiv2: Remove jitter when ADC is clocked by PLCK divided by 2
  *            @arg ADC_JitterOff_PCLKDiv4: Remove jitter when ADC is clocked by PLCK divided by 4
  * @param  NewState: new state of the ADCx jitter. 
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_JitterCmd(ADC_TypeDef* ADCx, uint32_t ADC_JitterOff, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_JITTEROFF(ADC_JitterOff));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Disable Jitter */
    ADCx->CFGR2 |= (uint32_t)ADC_JitterOff;
  }
  else
  {
    /* Enable Jitter */
    ADCx->CFGR2 &= (uint32_t)(~ADC_JitterOff);
  }
}

/**
  * @}
  */

/** @defgroup ADC_Group2 Power saving functions
 *  @brief   Power saving functions 
 *
@verbatim
 ===============================================================================
          ##### Power saving functions #####
 ===============================================================================
    [..] This section provides functions allowing to reduce power consumption.
    [..] The two function must be combined to get the maximal benefits:
         When the ADC frequency is higher than the CPU one, it is recommended to 
         (#) Enable the Auto Delayed Conversion mode : 
             ==> using ADC_WaitModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
         (#) Enable the power off in Delay phases :
             ==> using ADC_AutoPowerOffCmd(ADC_TypeDef* ADCx, FunctionalState NewState);

@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the ADC Power Off.
  * @note   ADC power-on and power-off can be managed by hardware to cut the 
  *         consumption when the ADC is not converting. 
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @note   The ADC can be powered down: 
  *         - During the Auto delay phase 
  *           => The ADC is powered on again at the end of the delay (until the 
  *              previous data is read from the ADC data register). 
  *         - During the ADC is waiting for a trigger event 
  *           => The ADC is powered up at the next trigger event (when the 
  *              conversion is started).
  * @param  NewState: new state of the ADCx power Off. 
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_AutoPowerOffCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the ADC Automatic Power-Off */
    ADCx->CFGR1 |= ADC_CFGR1_AUTOFF;
  }
  else
  {
    /* Disable the ADC Automatic Power-Off */
    ADCx->CFGR1 &= (uint32_t)~ADC_CFGR1_AUTOFF;
  }
}

/**
  * @brief  Enables or disables the Wait conversion mode.
  * @note   When the CPU clock is not fast enough to manage the data rate, a 
  *         Hardware delay can be introduced between ADC conversions to reduce 
  *         this data rate. 
  * @note   The Hardware delay is inserted after :
  *         - after each conversions and until the previous data is read from the 
  *           ADC data register
  * @note   This is a way to automatically adapt the speed of the ADC to the speed 
  *         of the system which will read the data.
  * @note   Any hardware triggers wich occur while a conversion is on going or 
  *         while the automatic Delay is applied are ignored 
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  NewState: new state of the ADCx Auto-Delay.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_WaitModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the ADC Automatic Delayed conversion */
    ADCx->CFGR1 |= ADC_CFGR1_AUTDLY ;
  }
  else
  {
    /* Disable the ADC Automatic Delayed conversion */
    ADCx->CFGR1 &= (uint32_t)~ADC_CFGR1_AUTDLY ;
  }
}

/**
  * @}
  */

/** @defgroup ADC_Group3 Analog Watchdog configuration functions
 *  @brief   Analog Watchdog configuration functions 
 *
@verbatim
 ===============================================================================
                   ##### Analog Watchdog configuration functions #####
 ===============================================================================  
    [..] This section provides functions allowing to configure the Analog Watchdog
         (AWD) feature in the ADC.
    [..] A typical configuration Analog Watchdog is done following these steps :
         (#) the ADC guarded channel(s) is (are) selected using the 
             ADC_AnalogWatchdogSingleChannelConfig() function.
         (#) The Analog watchdog lower and higher threshold are configured using the  
             ADC_AnalogWatchdogThresholdsConfig() function.
         (#) The Analog watchdog is enabled and configured to enable the check, on one
             or more channels, using the  ADC_AnalogWatchdogCmd() function.
         (#) Enable the analog watchdog on the selected channel using
             ADC_AnalogWatchdogSingleChannelCmd() function

@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the analog watchdog 
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  NewState: new state of the ADCx Analog Watchdog.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_AnalogWatchdogCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the ADC Analog Watchdog */
    ADCx->CFGR1 |= ADC_CFGR1_AWDEN;
  }
  else
  {
    /* Disable the ADC Analog Watchdog */
    ADCx->CFGR1 &= (uint32_t)~ADC_CFGR1_AWDEN;
  }
}

/**
  * @brief  Configures the high and low thresholds of the analog watchdog. 
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  HighThreshold: the ADC analog watchdog High threshold value.
  *         This parameter must be a 12bit value.
  * @param  LowThreshold: the ADC analog watchdog Low threshold value.
  *         This parameter must be a 12bit value.
  * @retval None
  */
void ADC_AnalogWatchdogThresholdsConfig(ADC_TypeDef* ADCx, uint16_t HighThreshold,
                                        uint16_t LowThreshold)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_THRESHOLD(HighThreshold));
  assert_param(IS_ADC_THRESHOLD(LowThreshold));

  /* Set the ADCx high and low threshold */
  ADCx->TR = LowThreshold | ((uint32_t)HighThreshold << 16);

}

/**
  * @brief  Configures the analog watchdog guarded single channel
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  ADC_AnalogWatchdog_Channel: the ADC channel to configure for the analog watchdog.
  *   This parameter can be one of the following values:
  *     @arg ADC_AnalogWatchdog_Channel_0: ADC Channel0 selected
  *     @arg ADC_AnalogWatchdog_Channel_1: ADC Channel1 selected
  *     @arg ADC_AnalogWatchdog_Channel_2: ADC Channel2 selected
  *     @arg ADC_AnalogWatchdog_Channel_3: ADC Channel3 selected
  *     @arg ADC_AnalogWatchdog_Channel_4: ADC Channel4 selected
  *     @arg ADC_AnalogWatchdog_Channel_5: ADC Channel5 selected
  *     @arg ADC_AnalogWatchdog_Channel_6: ADC Channel6 selected
  *     @arg ADC_AnalogWatchdog_Channel_7: ADC Channel7 selected
  *     @arg ADC_AnalogWatchdog_Channel_8: ADC Channel8 selected
  *     @arg ADC_AnalogWatchdog_Channel_9: ADC Channel9 selected
  *     @arg ADC_AnalogWatchdog_Channel_10: ADC Channel10 selected
  *     @arg ADC_AnalogWatchdog_Channel_11: ADC Channel11 selected
  *     @arg ADC_AnalogWatchdog_Channel_12: ADC Channel12 selected
  *     @arg ADC_AnalogWatchdog_Channel_13: ADC Channel13 selected
  *     @arg ADC_AnalogWatchdog_Channel_14: ADC Channel14 selected
  *     @arg ADC_AnalogWatchdog_Channel_15: ADC Channel15 selected
  *     @arg ADC_AnalogWatchdog_Channel_16: ADC Channel16 selected
  *     @arg ADC_AnalogWatchdog_Channel_17: ADC Channel17 selected
  *     @arg ADC_AnalogWatchdog_Channel_18: ADC Channel18 selected
  * @note   The channel selected on the AWDCH must be also set into the CHSELR 
  *         register 
  * @retval None
  */
void ADC_AnalogWatchdogSingleChannelConfig(ADC_TypeDef* ADCx, uint32_t ADC_AnalogWatchdog_Channel)
{
  uint32_t tmpreg = 0;

  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_ANALOG_WATCHDOG_CHANNEL(ADC_AnalogWatchdog_Channel));

  /* Get the old register value */
  tmpreg = ADCx->CFGR1;

  /* Clear the Analog watchdog channel select bits */
  tmpreg &= ~ADC_CFGR1_AWDCH;

  /* Set the Analog watchdog channel */
  tmpreg |= ADC_AnalogWatchdog_Channel;

  /* Store the new register value */
  ADCx->CFGR1 = tmpreg;
}

/**
  * @brief  Enables or disables the ADC Analog Watchdog Single Channel.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  NewState: new state of the ADCx ADC Analog Watchdog Single Channel.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_AnalogWatchdogSingleChannelCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the ADC Analog Watchdog Single Channel */
    ADCx->CFGR1 |= ADC_CFGR1_AWDSGL;
  }
  else
  {
    /* Disable the ADC Analog Watchdog Single Channel */
    ADCx->CFGR1 &= (uint32_t)~ADC_CFGR1_AWDSGL;
  }
}

/**
  * @}
  */

/** @defgroup ADC_Group4 Temperature Sensor, Vrefint  and Vbat management functions
 *  @brief   Temperature Sensor, Vrefint  and Vbat management functions
 *
@verbatim
 ===============================================================================
 ##### Temperature Sensor, Vrefint  and Vbat management function #####
 ===============================================================================
    [..] This section provides a function allowing to enable/disable the internal 
         connections between the ADC and the Temperature Sensor, the Vrefint and
         Vbat source.
     
    [..] A typical configuration to get the Temperature sensor, Vrefint and Vbat channels 
         voltages is done following these steps :
         (#) Enable the internal connection of Temperature sensor, Vrefint or Vbat sources 
             with the ADC channels using ADC_TempSensorCmd(), ADC_VrefintCmd() or ADC_VbatCmd()
             functions. 
         (#) select the ADC_Channel_16(Temperature sensor), ADC_Channel_17(Vrefint)
             or ADC_Channel_18(Voltage battery) using ADC_ChannelConfig() function 
         (#) Get the voltage values, using ADC_GetConversionValue() function

@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the temperature sensor channel.
  * @param  NewState: new state of the temperature sensor input channel.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_TempSensorCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the temperature sensor channel*/
    ADC->CCR |= (uint32_t)ADC_CCR_TSEN;
  }
  else
  {
    /* Disable the temperature sensor channel*/
    ADC->CCR &= (uint32_t)(~ADC_CCR_TSEN);
  }
}

/**
  * @brief  Enables or disables the Vrefint channel.
  * @param  NewState: new state of the Vref input channel.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_VrefintCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the Vrefint channel*/
    ADC->CCR |= (uint32_t)ADC_CCR_VREFEN;
  }
  else
  {
    /* Disable the Vrefint channel*/
    ADC->CCR &= (uint32_t)(~ADC_CCR_VREFEN);
  }
}

/**
  * @brief  Enables or disables the Vbat channel.
  * @param  NewState: new state of the Vbat input channel.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_VbatCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the Vbat channel*/
    ADC->CCR |= (uint32_t)ADC_CCR_VBATEN;
  }
  else
  {
    /* Disable the Vbat channel*/
    ADC->CCR &= (uint32_t)(~ADC_CCR_VBATEN);
  }
}

/**
  * @}
  */

/** @defgroup ADC_Group5 Channels Configuration functions
 *  @brief    Channels Configuration functions 
 *
@verbatim
 ===============================================================================
            ##### Channels Configuration functions #####
 ===============================================================================
    [..] This section provides functions allowing to manage the ADC channels,
         it is composed of 3 sub sections :
         (#) Configuration and management functions for ADC channels: This subsection 
             provides functions allowing to configure the ADC channels :    
             (++) Select the ADC channels
             (++) Activate ADC Calibration
             (++) Activate the Overrun Mode.
             (++) Activate the Discontinuous Mode 
             (++) Activate the Continuous Mode.
             (++) Configure the sampling time for each channel
             (++) Select the conversion Trigger and Edge for ADC channels
             (++) Select the scan direction.
             -@@- Please Note that the following features for ADC channels are configurated
                  using the ADC_Init() function : 
                  (+@@) Activate the Continuous Mode (can be also activated by ADC_OverrunModeCmd().
                  (+@@) Select the conversion Trigger and Edge for ADC channels
                  (+@@) Select the scan direction.
         (#) Control the ADC peripheral : This subsection permits to command the ADC:
             (++) Stop or discard an on-going conversion (ADSTP command)
             (++) Start the ADC conversion .
         (#) Get the conversion data: This subsection provides an important function in 
             the ADC peripheral since it returns the converted data of the current 
             ADC channel. When the Conversion value is read, the EOC Flag is 
             automatically cleared.

@endverbatim
  * @{
  */

/**
  * @brief  Configures for the selected ADC and its sampling time.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure. 
  *   This parameter can be any combination of the following values:
  *     @arg ADC_Channel_0: ADC Channel0 selected
  *     @arg ADC_Channel_1: ADC Channel1 selected
  *     @arg ADC_Channel_2: ADC Channel2 selected
  *     @arg ADC_Channel_3: ADC Channel3 selected
  *     @arg ADC_Channel_4: ADC Channel4 selected
  *     @arg ADC_Channel_5: ADC Channel5 selected
  *     @arg ADC_Channel_6: ADC Channel6 selected
  *     @arg ADC_Channel_7: ADC Channel7 selected
  *     @arg ADC_Channel_8: ADC Channel8 selected
  *     @arg ADC_Channel_9: ADC Channel9 selected
  *     @arg ADC_Channel_10: ADC Channel10 selected
  *     @arg ADC_Channel_11: ADC Channel11 selected
  *     @arg ADC_Channel_12: ADC Channel12 selected
  *     @arg ADC_Channel_13: ADC Channel13 selected
  *     @arg ADC_Channel_14: ADC Channel14 selected
  *     @arg ADC_Channel_15: ADC Channel15 selected
  *     @arg ADC_Channel_16: ADC Channel16 selected
  *     @arg ADC_Channel_17: ADC Channel17 selected
  *     @arg ADC_Channel_18: ADC Channel18 selected    
  * @param  ADC_SampleTime: The sample time value to be set for the selected 
  *         channel. 
  *   This parameter can be one of the following values:
  *     @arg ADC_SampleTime_1_5Cycles: Sample time equal to 1.5 cycles  
  *     @arg ADC_SampleTime_7_5Cycles: Sample time equal to 7.5 cycles
  *     @arg ADC_SampleTime_13_5Cycles: Sample time equal to 13.5 cycles
  *     @arg ADC_SampleTime_28_5Cycles: Sample time equal to 28.5 cycles
  *     @arg ADC_SampleTime_41_5Cycles: Sample time equal to 41.5 cycles
  *     @arg ADC_SampleTime_55_5Cycles: Sample time equal to 55.5 cycles
  *     @arg ADC_SampleTime_71_5Cycles: Sample time equal to 71.5 cycles
  *     @arg ADC_SampleTime_239_5Cycles: Sample time equal to 239.5 cycles
  * @retval None
  */
void ADC_ChannelConfig(ADC_TypeDef* ADCx, uint32_t ADC_Channel, uint32_t ADC_SampleTime)
{
  uint32_t tmpreg = 0;

  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CHANNEL(ADC_Channel));
  assert_param(IS_ADC_SAMPLE_TIME(ADC_SampleTime));

  /* Configure the ADC Channel */
  ADCx->CHSELR |= (uint32_t)ADC_Channel;

  /* Clear the Sampling time Selection bits */
  tmpreg &= ~ADC_SMPR1_SMPR;

  /* Set the ADC Sampling Time register */
  tmpreg |= (uint32_t)ADC_SampleTime;

  /* Configure the ADC Sample time register */
  ADCx->SMPR = tmpreg ;
}

/**
  * @brief  Enable the Continuous mode for the selected ADCx channels.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  NewState: new state of the Continuous mode.
  *         This parameter can be: ENABLE or DISABLE.
  * @note   It is not possible to have both discontinuous mode and continuous mode
  *         enabled. In this case (If DISCEN and CONT are Set), the ADC behaves 
  *         as if continuous mode was disabled
  * @retval None
  */
void ADC_ContinuousModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
  {
    /* Enable the Continuous mode*/
    ADCx->CFGR1 |= (uint32_t)ADC_CFGR1_CONT;
  }
  else
  {
    /* Disable the Continuous mode */
    ADCx->CFGR1 &= (uint32_t)(~ADC_CFGR1_CONT);
  }
}

/**
  * @brief  Enable the discontinuous mode for the selected ADC channels.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  NewState: new state of the discontinuous mode.
  *         This parameter can be: ENABLE or DISABLE.
  * @note   It is not possible to have both discontinuous mode and continuous mode
  *         enabled. In this case (If DISCEN and CONT are Set), the ADC behaves 
  *         as if continuous mode was disabled
  * @retval None
  */
void ADC_DiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
  {
    /* Enable the Discontinuous mode */
    ADCx->CFGR1 |= (uint32_t)ADC_CFGR1_DISCEN;
  }
  else
  {
    /* Disable the Discontinuous mode */
    ADCx->CFGR1 &= (uint32_t)(~ADC_CFGR1_DISCEN);
  }
}

/**
  * @brief  Enable the Overrun mode for the selected ADC channels.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  NewState: new state of the Overrun mode.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_OverrunModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
  {
    /* Enable the Overrun mode */
    ADCx->CFGR1 |= (uint32_t)ADC_CFGR1_OVRMOD;
  }
  else
  {
    /* Disable the Overrun mode */
    ADCx->CFGR1 &= (uint32_t)(~ADC_CFGR1_OVRMOD);
  }
}

/**
  * @brief  Active the Calibration operation for the selected ADC.
  * @note   The Calibration can be initiated only when ADC is still in the 
  *         reset configuration (ADEN must be equal to 0).
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @retval ADC Calibration factor 
  */
uint32_t ADC_GetCalibrationFactor(ADC_TypeDef* ADCx)
{
  uint32_t tmpreg = 0, calibrationcounter = 0, calibrationstatus = 0;

  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  
  /* Set the ADC calibartion */
  ADCx->CR |= (uint32_t)ADC_CR_ADCAL;
  
  /* Wait until no ADC calibration is completed */
  do
  {
    calibrationstatus = ADCx->CR & ADC_CR_ADCAL;
    calibrationcounter++;  
  } while((calibrationcounter != CALIBRATION_TIMEOUT) && (calibrationstatus != 0x00));
    
  if((uint32_t)(ADCx->CR & ADC_CR_ADCAL) == RESET)
  {
    /*Get the calibration factor from the ADC data register */
    tmpreg = ADCx->DR;
  }
  else
  {
    /* Error factor */
    tmpreg = 0x00000000;
  }
  return tmpreg;
}

/**
  * @brief  Stop the on going conversions for the selected ADC.
  * @note   When ADSTP is set, any on going conversion is aborted, and the ADC 
  *         data register is not updated with current conversion. 
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @retval None
  */
void ADC_StopOfConversion(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  
  ADCx->CR |= (uint32_t)ADC_CR_ADSTP;
}

/**
  * @brief  Start Conversion for the selected ADC channels.
  * @note   In continuous mode, ADSTART is not cleared by hardware with the 
  *         assertion of EOSEQ because the sequence is automatic relaunched
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @retval None
  */
void ADC_StartOfConversion(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  
  ADCx->CR |= (uint32_t)ADC_CR_ADSTART;
}

/**
  * @brief  Returns the last ADCx conversion result data for ADC channel.  
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @retval The Data conversion value.
  */
uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));

  /* Return the selected ADC conversion value */
  return (uint16_t) ADCx->DR;
}

/**
  * @}
  */

/** @defgroup ADC_Group6 DMA Configuration functions
 *  @brief   Regular Channels DMA Configuration functions 
 *
@verbatim
 ===============================================================================
          ##### DMA Configuration functions #####
 ===============================================================================
    [..] This section provides functions allowing to configure the DMA for ADC hannels.
         Since converted channel values are stored into a unique data register, 
         it is useful to use DMA for conversion of more than one channel. This 
         avoids the loss of the data already stored in the ADC Data register. 
         When the DMA mode is enabled (using the ADC_DMACmd() function), after each
         conversion of a channel, a DMA request is generated.
  
    [..] Depending on the "DMA disable selection" configuration (using the 
         ADC_DMARequestModeConfig() function), at the end of the last DMA 
         transfer, two possibilities are allowed:
         (+) No new DMA request is issued to the DMA controller (One Shot Mode) 
         (+) Requests can continue to be generated (Circular Mode).

@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the specified ADC DMA request.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  NewState: new state of the selected ADC DMA transfer.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_DMACmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the selected ADC DMA request */
    ADCx->CFGR1 |= (uint32_t)ADC_CFGR1_DMAEN;
  }
  else
  {
    /* Disable the selected ADC DMA request */
    ADCx->CFGR1 &= (uint32_t)(~ADC_CFGR1_DMAEN);
  }
}

/**
  * @brief  Enables or disables the ADC DMA request after last transfer (Single-ADC mode)
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  ADC_DMARequestMode: the ADC channel to configure. 
  *   This parameter can be one of the following values:
  *     @arg ADC_DMAMode_OneShot  : DMA One Shot Mode 
  *     @arg ADC_DMAMode_Circular : DMA Circular Mode  
  *  @retval None
  */
void ADC_DMARequestModeConfig(ADC_TypeDef* ADCx, uint32_t ADC_DMARequestMode)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));

  ADCx->CFGR1 &= (uint32_t)~ADC_CFGR1_DMACFG;
  ADCx->CFGR1 |= (uint32_t)ADC_DMARequestMode;
}

/**
  * @}
  */

/** @defgroup ADC_Group7 Interrupts and flags management functions
 *  @brief   Interrupts and flags management functions.
 *
@verbatim   
 ===============================================================================
            ##### Interrupts and flags management functions #####
 ===============================================================================
    [..] This section provides functions allowing to configure the ADC Interrupts 
         and get the status and clear flags and Interrupts pending bits.
  
    [..] The ADC provide 6 Interrupts sources and 11 Flags which can be divided into 
         3 groups:

  *** Flags for ADC status ***
  ======================================================
    [..]
        (+)Flags :
           (##) ADC_FLAG_ADRDY : This flag is set after the ADC has been enabled (bit ADEN=1)
               and when the ADC reaches a state where it is ready to accept conversion requests
           (##) ADC_FLAG_ADEN : This flag is set by software to enable the ADC.
                The ADC will be effectively ready to operate once the ADRDY flag has been set.
           (##) ADC_FLAG_ADDIS : This flag is cleared once the ADC is effectively
                disabled.
           (##) ADC_FLAG_ADSTART : This flag is cleared after the execution of
                ADC_StopOfConversion() function, at the same time as the ADSTP bit is
                cleared by hardware
           (##) ADC_FLAG_ADSTP : This flag is cleared by hardware when the conversion
                is effectively discarded and the ADC is ready to accept a new start conversion
           (##) ADC_FLAG_ADCAL : This flag is set once the calibration is complete.

        (+)Interrupts 
           (##) ADC_IT_ADRDY : specifies the interrupt source for ADC ready event.

  *** Flags and Interrupts for ADC channel conversion ***
  =====================================================
    [..]
        (+)Flags :
           (##) ADC_FLAG_EOC : This flag is set by hardware at the end of each conversion
                of a channel when a new data result is available in the data register
           (##) ADC_FLAG_EOSEQ : This bit is set by hardware at the end of the conversion
                of a sequence of channels selected by ADC_ChannelConfig() function.
           (##) ADC_FLAG_EOSMP : This bit is set by hardware at the end of the sampling phase.
           (##) ADC_FLAG_OVR : This flag is set by hardware when an overrun occurs,
                meaning that a new conversion has complete while the EOC flag was already set.

        (+)Interrupts :
           (##) ADC_IT_EOC : specifies the interrupt source for end of conversion event.
           (##) ADC_IT_EOSEQ : specifies the interrupt source for end of sequence event.
           (##) ADC_IT_EOSMP : specifies the interrupt source for end of sampling event.
           (##) ADC_IT_OVR : specifies the interrupt source for Overrun detection 
                event.

  *** Flags and Interrupts for the Analog Watchdog ***
  ================================================
    [..]
        (+)Flags :
           (##) ADC_FLAG_AWD: This flag is set by hardware when the converted
                voltage crosses the values programmed thrsholds

        (+)Interrupts :
           (##) ADC_IT_AWD : specifies the interrupt source for Analog watchdog 
                event.
  
    [..] The user should identify which mode will be used in his application to 
         manage the ADC controller events: Polling mode or Interrupt mode.
  
    [..] In the Polling Mode it is advised to use the following functions:
         (+) ADC_GetFlagStatus() : to check if flags events occur.
         (+) ADC_ClearFlag()     : to clear the flags events.
  
    [..] In the Interrupt Mode it is advised to use the following functions:
         (+) ADC_ITConfig()       : to enable or disable the interrupt source.
         (+) ADC_GetITStatus()    : to check if Interrupt occurs.
         (+) ADC_ClearITPendingBit() : to clear the Interrupt pending Bit 
             (corresponding Flag).

@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the specified ADC interrupts.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt sources to be enabled or disabled.
  *   This parameter can be one of the following values:
  *     @arg ADC_IT_ADRDY: ADC ready interrupt 
  *     @arg ADC_IT_EOSMP: End of sampling interrupt
  *     @arg ADC_IT_EOC: End of conversion interrupt 
  *     @arg ADC_IT_EOSEQ: End of sequence of conversion interrupt
  *     @arg ADC_IT_OVR: overrun interrupt
  *     @arg ADC_IT_AWD: Analog watchdog interrupt
  * @param  NewState: new state of the specified ADC interrupts.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_ITConfig(ADC_TypeDef* ADCx, uint32_t ADC_IT, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  assert_param(IS_ADC_CONFIG_IT(ADC_IT)); 

  if (NewState != DISABLE)
  {
    /* Enable the selected ADC interrupts */
    ADCx->IER |= ADC_IT;
  }
  else
  {
    /* Disable the selected ADC interrupts */
    ADCx->IER &= (~(uint32_t)ADC_IT);
  }
}

/**
  * @brief  Checks whether the specified ADC flag is set or not.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  ADC_FLAG: specifies the flag to check. 
  *   This parameter can be one of the following values:
  *     @arg ADC_FLAG_AWD: Analog watchdog flag
  *     @arg ADC_FLAG_OVR: Overrun flag 
  *     @arg ADC_FLAG_EOSEQ: End of Sequence flag
  *     @arg ADC_FLAG_EOC: End of conversion flag
  *     @arg ADC_FLAG_EOSMP: End of sampling flag
  *     @arg ADC_FLAG_ADRDY: ADC Ready flag
  *     @arg ADC_FLAG_ADEN: ADC enable flag 
  *     @arg ADC_FLAG_ADDIS: ADC disable flag 
  *     @arg ADC_FLAG_ADSTART: ADC start flag 
  *     @arg ADC_FLAG_ADSTP: ADC stop flag
  *     @arg ADC_FLAG_ADCAL: ADC Calibration flag
  * @retval The new state of ADC_FLAG (SET or RESET).
  */
FlagStatus ADC_GetFlagStatus(ADC_TypeDef* ADCx, uint32_t ADC_FLAG)
{
  FlagStatus bitstatus = RESET;
  uint32_t tmpreg = 0;

  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_GET_FLAG(ADC_FLAG));

  if((uint32_t)(ADC_FLAG & 0x01000000))
  {
    tmpreg = ADCx->CR & 0xFEFFFFFF;
  }
  else
  {
    tmpreg = ADCx->ISR;
  }
  
  /* Check the status of the specified ADC flag */
  if ((tmpreg & ADC_FLAG) != (uint32_t)RESET)
  {
    /* ADC_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* ADC_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the ADC_FLAG status */
  return  bitstatus;
}

/**
  * @brief  Clears the ADCx's pending flags.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  ADC_FLAG: specifies the flag to clear. 
  *   This parameter can be any combination of the following values:
  *     @arg ADC_FLAG_AWD: Analog watchdog flag
  *     @arg ADC_FLAG_EOC: End of conversion flag
  *     @arg ADC_FLAG_ADRDY: ADC Ready flag
  *     @arg ADC_FLAG_EOSMP: End of sampling flag
  *     @arg ADC_FLAG_EOSEQ: End of Sequence flag
  *     @arg ADC_FLAG_OVR: Overrun flag 
  * @retval None
  */
void ADC_ClearFlag(ADC_TypeDef* ADCx, uint32_t ADC_FLAG)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CLEAR_FLAG(ADC_FLAG));

  /* Clear the selected ADC flags */
  ADCx->ISR = (uint32_t)ADC_FLAG;
}

/**
  * @brief  Checks whether the specified ADC interrupt has occurred or not.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral
  * @param  ADC_IT: specifies the ADC interrupt source to check.
  *   This parameter can be one of the following values:
  *     @arg ADC_IT_ADRDY: ADC ready interrupt 
  *     @arg ADC_IT_EOSMP: End of sampling interrupt
  *     @arg ADC_IT_EOC: End of conversion interrupt 
  *     @arg ADC_IT_EOSEQ: End of sequence of conversion interrupt
  *     @arg ADC_IT_OVR: overrun interrupt
  *     @arg ADC_IT_AWD: Analog watchdog interrupt
  * @retval The new state of ADC_IT (SET or RESET).
  */
ITStatus ADC_GetITStatus(ADC_TypeDef* ADCx, uint32_t ADC_IT)
{
  ITStatus bitstatus = RESET;
  uint32_t enablestatus = 0;

  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_GET_IT(ADC_IT));

  /* Get the ADC_IT enable bit status */
  enablestatus = (uint32_t)(ADCx->IER & ADC_IT); 

  /* Check the status of the specified ADC interrupt */
  if (((uint32_t)(ADCx->ISR & ADC_IT) != (uint32_t)RESET) && (enablestatus != (uint32_t)RESET))
  {
    /* ADC_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* ADC_IT is reset */
    bitstatus = RESET;
  }
  /* Return the ADC_IT status */
  return  bitstatus;
}

/**
  * @brief  Clears the ADCx's interrupt pending bits.
  * @param  ADCx: where x can be 1 to select the ADC1 peripheral.
  * @param  ADC_IT: specifies the ADC interrupt pending bit to clear.
  *   This parameter can be one of the following values:
  *     @arg ADC_IT_ADRDY: ADC ready interrupt
  *     @arg ADC_IT_EOSMP: End of sampling interrupt
  *     @arg ADC_IT_EOC: End of conversion interrupt
  *     @arg ADC_IT_EOSEQ: End of sequence of conversion interrupt
  *     @arg ADC_IT_OVR: overrun interrupt
  *     @arg ADC_IT_AWD: Analog watchdog interrupt
  * @retval None
  */
void ADC_ClearITPendingBit(ADC_TypeDef* ADCx, uint32_t ADC_IT)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CLEAR_IT(ADC_IT));

  /* Clear the selected ADC interrupt pending bits */
  ADCx->ISR = (uint32_t)ADC_IT; 
}

/**
  * @}
  */

/**
  * @}
  */ 

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
  * @file    stm32f10x_adc.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    11-March-2011
  * @brief   This file provides all the ADC firmware functions.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_adc.h"
#include "stm32f0xx_rcc.h"

/** @addtogroup STM32F10x_StdPeriph_Driver
  * @{
  */

/** @defgroup ADC 
  * @brief ADC driver modules
  * @{
  */

/** @defgroup ADC_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @defgroup ADC_Private_Defines
  * @{
  */

/* ADC DISCNUM mask */
#define CR1_DISCNUM_Reset           ((uint32_t)0xFFFF1FFF)

/* ADC DISCEN mask */
#define CR1_DISCEN_Set              ((uint32_t)0x00000800)
#define CR1_DISCEN_Reset            ((uint32_t)0xFFFFF7FF)

/* ADC JAUTO mask */
#define CR1_JAUTO_Set               ((uint32_t)0x00000400)
#define CR1_JAUTO_Reset             ((uint32_t)0xFFFFFBFF)

/* ADC JDISCEN mask */
#define CR1_JDISCEN_Set             ((uint32_t)0x00001000)
#define CR1_JDISCEN_Reset           ((uint32_t)0xFFFFEFFF)

/* ADC AWDCH mask */
#define CR1_AWDCH_Reset             ((uint32_t)0xFFFFFFE0)

/* ADC Analog watchdog enable mode mask */
#define CR1_AWDMode_Reset           ((uint32_t)0xFF3FFDFF)

/* CR1 register Mask */
#define CR1_CLEAR_Mask              ((uint32_t)0xFFF0FEFF)

/* ADC ADON mask */
#define CR2_ADON_Set                ((uint32_t)0x00000001)
#define CR2_ADON_Reset              ((uint32_t)0xFFFFFFFE)

/* ADC DMA mask */
#define CR2_DMA_Set                 ((uint32_t)0x00000100)
#define CR2_DMA_Reset               ((uint32_t)0xFFFFFEFF)

/* ADC RSTCAL mask */
#define CR2_RSTCAL_Set              ((uint32_t)0x00000008)

/* ADC CAL mask */
#define CR2_CAL_Set                 ((uint32_t)0x00000004)

/* ADC SWSTART mask */
#define CR2_SWSTART_Set             ((uint32_t)0x00400000)

/* ADC EXTTRIG mask */
#define CR2_EXTTRIG_Set             ((uint32_t)0x00100000)
#define CR2_EXTTRIG_Reset           ((uint32_t)0xFFEFFFFF)

/* ADC Software start mask */
#define CR2_EXTTRIG_SWSTART_Set     ((uint32_t)0x00500000)
#define CR2_EXTTRIG_SWSTART_Reset   ((uint32_t)0xFFAFFFFF)

/* ADC JEXTSEL mask */
#define CR2_JEXTSEL_Reset           ((uint32_t)0xFFFF8FFF)

/* ADC JEXTTRIG mask */
#define CR2_JEXTTRIG_Set            ((uint32_t)0x00008000)
#define CR2_JEXTTRIG_Reset          ((uint32_t)0xFFFF7FFF)

/* ADC JSWSTART mask */
#define CR2_JSWSTART_Set            ((uint32_t)0x00200000)

/* ADC injected software start mask */
#define CR2_JEXTTRIG_JSWSTART_Set   ((uint32_t)0x00208000)
#define CR2_JEXTTRIG_JSWSTART_Reset ((uint32_t)0xFFDF7FFF)

/* ADC TSPD mask */
#define CR2_TSVREFE_Set             ((uint32_t)0x00800000)
#define CR2_TSVREFE_Reset           ((uint32_t)0xFF7FFFFF)

/* CR2 register Mask */
#define CR2_CLEAR_Mask              ((uint32_t)0xFFF1F7FD)

/* ADC SQx mask */
#define SQR3_SQ_Set                 ((uint32_t)0x0000001F)
#define SQR2_SQ_Set                 ((uint32_t)0x0000001F)
#define SQR1_SQ_Set                 ((uint32_t)0x0000001F)

/* SQR1 register Mask */
#define SQR1_CLEAR_Mask             ((uint32_t)0xFF0FFFFF)

/* ADC JSQx mask */
#define JSQR_JSQ_Set                ((uint32_t)0x0000001F)

/* ADC JL mask */
#define JSQR_JL_Set                 ((uint32_t)0x00300000)
#define JSQR_JL_Reset               ((uint32_t)0xFFCFFFFF)

/* ADC SMPx mask */
#define SMPR1_SMP_Set               ((uint32_t)0x00000007)
#define SMPR2_SMP_Set               ((uint32_t)0x00000007)

/* ADC JDRx registers offset */
#define JDR_Offset                  ((uint8_t)0x28)

/* ADC1 DR register base address */
#define DR_ADDRESS                  ((uint32_t)0x4001244C)

/**
  * @}
  */

/** @defgroup ADC_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup ADC_Private_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup ADC_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */

/** @defgroup ADC_Private_Functions
  * @{
  */

/**
  * @brief  Deinitializes the ADCx peripheral registers to their default reset values.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval None
  */
void ADC_DeInit(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  
//  if (ADCx == ADC1)
//  {
    /* Enable ADC1 reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
    /* Release ADC1 from reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
//  }
//  else if (ADCx == ADC2)
//  {
//    /* Enable ADC2 reset state */
//    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC2, ENABLE);
//    /* Release ADC2 from reset state */
//    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC2, DISABLE);
//  }
//  else
//  {
//    if (ADCx == ADC3)
//    {
//      /* Enable ADC3 reset state */
//      RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, ENABLE);
//      /* Release ADC3 from reset state */
//      RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, DISABLE);
//    }
//  }
}

/**
  * @brief  Initializes the ADCx peripheral according to the specified parameters
  *         in the ADC_InitStruct.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_InitStruct: pointer to an ADC_InitTypeDef structure that contains
  *         the configuration information for the specified ADC peripheral.
  * @retval None
  */
void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef* ADC_InitStruct)
{
  uint32_t tmpreg1 = 0;
  uint8_t tmpreg2 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_MODE(ADC_InitStruct->ADC_Mode));
  assert_param(IS_FUNCTIONAL_STATE(ADC_InitStruct->ADC_ScanConvMode));
  assert_param(IS_FUNCTIONAL_STATE(ADC_InitStruct->ADC_ContinuousConvMode));
  assert_param(IS_ADC_EXT_TRIG(ADC_InitStruct->ADC_ExternalTrigConv));   
  assert_param(IS_ADC_DATA_ALIGN(ADC_InitStruct->ADC_DataAlign)); 
  assert_param(IS_ADC_REGULAR_LENGTH(ADC_InitStruct->ADC_NbrOfChannel));

  /*---------------------------- ADCx CR1 Configuration -----------------*/
  /* Get the ADCx CR1 value */
  tmpreg1 = ADCx->CR1;
  /* Clear DUALMOD and SCAN bits */
  tmpreg1 &= CR1_CLEAR_Mask;
  /* Configure ADCx: Dual mode and scan conversion mode */
  /* Set DUALMOD bits according to ADC_Mode value */
  /* Set SCAN bit according to ADC_ScanConvMode value */
  tmpreg1 |= (uint32_t)(ADC_InitStruct->ADC_Mode | ((uint32_t)ADC_InitStruct->ADC_ScanConvMode << 8));
  /* Write to ADCx CR1 */
  ADCx->CR1 = tmpreg1;

  /*---------------------------- ADCx CR2 Configuration -----------------*/
  /* Get the ADCx CR2 value */
  tmpreg1 = ADCx->CR2;
  /* Clear CONT, ALIGN and EXTSEL bits */
  tmpreg1 &= CR2_CLEAR_Mask;
  /* Configure ADCx: external trigger event and continuous conversion mode */
  /* Set ALIGN bit according to ADC_DataAlign value */
  /* Set EXTSEL bits according to ADC_ExternalTrigConv value */
  /* Set CONT bit according to ADC_ContinuousConvMode value */
  tmpreg1 |= (uint32_t)(ADC_InitStruct->ADC_DataAlign | ADC_InitStruct->ADC_ExternalTrigConv |
            ((uint32_t)ADC_InitStruct->ADC_ContinuousConvMode << 1));
  /* Write to ADCx CR2 */
  ADCx->CR2 = tmpreg1;

  /*---------------------------- ADCx SQR1 Configuration -----------------*/
  /* Get the ADCx SQR1 value */
  tmpreg1 = ADCx->SQR1;
  /* Clear L bits */
  tmpreg1 &= SQR1_CLEAR_Mask;
  /* Configure ADCx: regular channel sequence length */
  /* Set L bits according to ADC_NbrOfChannel value */
  tmpreg2 |= (uint8_t) (ADC_InitStruct->ADC_NbrOfChannel - (uint8_t)1);
  tmpreg1 |= (uint32_t)tmpreg2 << 20;
  /* Write to ADCx SQR1 */
  ADCx->SQR1 = tmpreg1;
}

/**
  * @brief  Fills each ADC_InitStruct member with its default value.
  * @param  ADC_InitStruct : pointer to an ADC_InitTypeDef structure which will be initialized.
  * @retval None
  */
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct)
{
  /* Reset ADC init structure parameters values */
  /* Initialize the ADC_Mode member */
  ADC_InitStruct->ADC_Mode = ADC_Mode_Independent;
  /* initialize the ADC_ScanConvMode member */
  ADC_InitStruct->ADC_ScanConvMode = DISABLE;
  /* Initialize the ADC_ContinuousConvMode member */
  ADC_InitStruct->ADC_ContinuousConvMode = DISABLE;
  /* Initialize the ADC_ExternalTrigConv member */
  ADC_InitStruct->ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  /* Initialize the ADC_DataAlign member */
  ADC_InitStruct->ADC_DataAlign = ADC_DataAlign_Right;
  /* Initialize the ADC_NbrOfChannel member */
  ADC_InitStruct->ADC_NbrOfChannel = 1;
}

/**
  * @brief  Enables or disables the specified ADC peripheral.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the ADCx peripheral.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Set the ADON bit to wake up the ADC from power down mode */
    ADCx->CR2 |= CR2_ADON_Set;
  }
  else
  {
    /* Disable the selected ADC peripheral */
    ADCx->CR2 &= CR2_ADON_Reset;
  }
}

/**
  * @brief  Enables or disables the specified ADC DMA request.
  * @param  ADCx: where x can be 1 or 3 to select the ADC peripheral.
  *   Note: ADC2 hasn't a DMA capability.
  * @param  NewState: new state of the selected ADC DMA transfer.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_DMACmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_DMA_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC DMA request */
    ADCx->CR2 |= CR2_DMA_Set;
  }
  else
  {
    /* Disable the selected ADC DMA request */
    ADCx->CR2 &= CR2_DMA_Reset;
  }
}

/**
  * @brief  Enables or disables the specified ADC interrupts.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt sources to be enabled or disabled. 
  *   This parameter can be any combination of the following values:
  *     @arg ADC_IT_EOC: End of conversion interrupt mask
  *     @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *     @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  * @param  NewState: new state of the specified ADC interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_ITConfig(ADC_TypeDef* ADCx, uint16_t ADC_IT, FunctionalState NewState)
{
  uint8_t itmask = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  assert_param(IS_ADC_IT(ADC_IT));
  /* Get the ADC IT index */
  itmask = (uint8_t)ADC_IT;
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC interrupts */
    ADCx->CR1 |= itmask;
  }
  else
  {
    /* Disable the selected ADC interrupts */
    ADCx->CR1 &= (~(uint32_t)itmask);
  }
}

/**
  * @brief  Resets the selected ADC calibration registers.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval None
  */
void ADC_ResetCalibration(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Resets the selected ADC calibration registers */  
  ADCx->CR2 |= CR2_RSTCAL_Set;
}

/**
  * @brief  Gets the selected ADC reset calibration registers status.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval The new state of ADC reset calibration registers (SET or RESET).
  */
FlagStatus ADC_GetResetCalibrationStatus(ADC_TypeDef* ADCx)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Check the status of RSTCAL bit */
  if ((ADCx->CR2 & CR2_RSTCAL_Set) != (uint32_t)RESET)
  {
    /* RSTCAL bit is set */
    bitstatus = SET;
  }
  else
  {
    /* RSTCAL bit is reset */
    bitstatus = RESET;
  }
  /* Return the RSTCAL bit status */
  return  bitstatus;
}

/**
  * @brief  Starts the selected ADC calibration process.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval None
  */
void ADC_StartCalibration(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Enable the selected ADC calibration process */  
  ADCx->CR2 |= CR2_CAL_Set;
}

/**
  * @brief  Gets the selected ADC calibration status.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval The new state of ADC calibration (SET or RESET).
  */
FlagStatus ADC_GetCalibrationStatus(ADC_TypeDef* ADCx)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Check the status of CAL bit */
  if ((ADCx->CR2 & CR2_CAL_Set) != (uint32_t)RESET)
  {
    /* CAL bit is set: calibration on going */
    bitstatus = SET;
  }
  else
  {
    /* CAL bit is reset: end of calibration */
    bitstatus = RESET;
  }
  /* Return the CAL bit status */
  return  bitstatus;
}

/**
  * @brief  Enables or disables the selected ADC software start conversion .
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC software start conversion.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_SoftwareStartConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC conversion on external event and start the selected
       ADC conversion */
    ADCx->CR2 |= CR2_EXTTRIG_SWSTART_Set;
  }
  else
  {
    /* Disable the selected ADC conversion on external event and stop the selected
       ADC conversion */
    ADCx->CR2 &= CR2_EXTTRIG_SWSTART_Reset;
  }
}

/**
  * @brief  Gets the selected ADC Software start conversion Status.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval The new state of ADC software start conversion (SET or RESET).
  */
FlagStatus ADC_GetSoftwareStartConvStatus(ADC_TypeDef* ADCx)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Check the status of SWSTART bit */
  if ((ADCx->CR2 & CR2_SWSTART_Set) != (uint32_t)RESET)
  {
    /* SWSTART bit is set */
    bitstatus = SET;
  }
  else
  {
    /* SWSTART bit is reset */
    bitstatus = RESET;
  }
  /* Return the SWSTART bit status */
  return  bitstatus;
}

/**
  * @brief  Configures the discontinuous mode for the selected ADC regular
  *         group channel.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  Number: specifies the discontinuous mode regular channel
  *         count value. This number must be between 1 and 8.
  * @retval None
  */
void ADC_DiscModeChannelCountConfig(ADC_TypeDef* ADCx, uint8_t Number)
{
  uint32_t tmpreg1 = 0;
  uint32_t tmpreg2 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_REGULAR_DISC_NUMBER(Number));
  /* Get the old register value */
  tmpreg1 = ADCx->CR1;
  /* Clear the old discontinuous mode channel count */
  tmpreg1 &= CR1_DISCNUM_Reset;
  /* Set the discontinuous mode channel count */
  tmpreg2 = Number - 1;
  tmpreg1 |= tmpreg2 << 13;
  /* Store the new register value */
  ADCx->CR1 = tmpreg1;
}

/**
  * @brief  Enables or disables the discontinuous mode on regular group
  *         channel for the specified ADC
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC discontinuous mode
  *         on regular group channel.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_DiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC regular discontinuous mode */
    ADCx->CR1 |= CR1_DISCEN_Set;
  }
  else
  {
    /* Disable the selected ADC regular discontinuous mode */
    ADCx->CR1 &= CR1_DISCEN_Reset;
  }
}

/**
  * @brief  Configures for the selected ADC regular channel its corresponding
  *         rank in the sequencer and its sample time.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure. 
  *   This parameter can be one of the following values:
  *     @arg ADC_Channel_0: ADC Channel0 selected
  *     @arg ADC_Channel_1: ADC Channel1 selected
  *     @arg ADC_Channel_2: ADC Channel2 selected
  *     @arg ADC_Channel_3: ADC Channel3 selected
  *     @arg ADC_Channel_4: ADC Channel4 selected
  *     @arg ADC_Channel_5: ADC Channel5 selected
  *     @arg ADC_Channel_6: ADC Channel6 selected
  *     @arg ADC_Channel_7: ADC Channel7 selected
  *     @arg ADC_Channel_8: ADC Channel8 selected
  *     @arg ADC_Channel_9: ADC Channel9 selected
  *     @arg ADC_Channel_10: ADC Channel10 selected
  *     @arg ADC_Channel_11: ADC Channel11 selected
  *     @arg ADC_Channel_12: ADC Channel12 selected
  *     @arg ADC_Channel_13: ADC Channel13 selected
  *     @arg ADC_Channel_14: ADC Channel14 selected
  *     @arg ADC_Channel_15: ADC Channel15 selected
  *     @arg ADC_Channel_16: ADC Channel16 selected
  *     @arg ADC_Channel_17: ADC Channel17 selected
  * @param  Rank: The rank in the regular group sequencer. This parameter must be between 1 to 16.
  * @param  ADC_SampleTime: The sample time value to be set for the selected channel. 
  *   This parameter can be one of the following values:
  *     @arg ADC_SampleTime_1Cycles5: Sample time equal to 1.5 cycles
  *     @arg ADC_SampleTime_7Cycles5: Sample time equal to 7.5 cycles
  *     @arg ADC_SampleTime_13Cycles5: Sample time equal to 13.5 cycles
  *     @arg ADC_SampleTime_28Cycles5: Sample time equal to 28.5 cycles	
  *     @arg ADC_SampleTime_41Cycles5: Sample time equal to 41.5 cycles	
  *     @arg ADC_SampleTime_55Cycles5: Sample time equal to 55.5 cycles	
  *     @arg ADC_SampleTime_71Cycles5: Sample time equal to 71.5 cycles	
  *     @arg ADC_SampleTime_239Cycles5: Sample time equal to 239.5 cycles	
  * @retval None
  */
void ADC_RegularChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime)
{
  uint32_t tmpreg1 = 0, tmpreg2 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CHANNEL(ADC_Channel));
  assert_param(IS_ADC_REGULAR_RANK(Rank));
  assert_param(IS_ADC_SAMPLE_TIME(ADC_SampleTime));
  /* if ADC_Channel_10 ... ADC_Channel_17 is selected */
  if (ADC_Channel > ADC_Channel_9)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR1;
    /* Calculate the mask to clear */
    tmpreg2 = SMPR1_SMP_Set << (3 * (ADC_Channel - 10));
    /* Clear the old channel sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * (ADC_Channel - 10));
    /* Set the new channel sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR1 = tmpreg1;
  }
  else /* ADC_Channel include in ADC_Channel_[0..9] */
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR2;
    /* Calculate the mask to clear */
    tmpreg2 = SMPR2_SMP_Set << (3 * ADC_Channel);
    /* Clear the old channel sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * ADC_Channel);
    /* Set the new channel sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR2 = tmpreg1;
  }
  /* For Rank 1 to 6 */
  if (Rank < 7)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR3;
    /* Calculate the mask to clear */
    tmpreg2 = SQR3_SQ_Set << (5 * (Rank - 1));
    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 1));
    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SQR3 = tmpreg1;
  }
  /* For Rank 7 to 12 */
  else if (Rank < 13)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR2;
    /* Calculate the mask to clear */
    tmpreg2 = SQR2_SQ_Set << (5 * (Rank - 7));
    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 7));
    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SQR2 = tmpreg1;
  }
  /* For Rank 13 to 16 */
  else
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR1;
    /* Calculate the mask to clear */
    tmpreg2 = SQR1_SQ_Set << (5 * (Rank - 13));
    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 13));
    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SQR1 = tmpreg1;
  }
}

/**
  * @brief  Enables or disables the ADCx conversion through external trigger.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC external trigger start of conversion.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_ExternalTrigConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC conversion on external event */
    ADCx->CR2 |= CR2_EXTTRIG_Set;
  }
  else
  {
    /* Disable the selected ADC conversion on external event */
    ADCx->CR2 &= CR2_EXTTRIG_Reset;
  }
}

/**
  * @brief  Returns the last ADCx conversion result data for regular channel.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval The Data conversion value.
  */
uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Return the selected ADC conversion value */
  return (uint16_t) ADCx->DR;
}

/**
  * @brief  Returns the last ADC1 and ADC2 conversion result data in dual mode.
  * @retval The Data conversion value.
  */
uint32_t ADC_GetDualModeConversionValue(void)
{
  /* Return the dual mode conversion value */
  return (*(__IO uint32_t *) DR_ADDRESS);
}

/**
  * @brief  Enables or disables the selected ADC automatic injected group
  *         conversion after regular one.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC auto injected conversion
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_AutoInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC automatic injected group conversion */
    ADCx->CR1 |= CR1_JAUTO_Set;
  }
  else
  {
    /* Disable the selected ADC automatic injected group conversion */
    ADCx->CR1 &= CR1_JAUTO_Reset;
  }
}

/**
  * @brief  Enables or disables the discontinuous mode for injected group
  *         channel for the specified ADC
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC discontinuous mode
  *         on injected group channel.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_InjectedDiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC injected discontinuous mode */
    ADCx->CR1 |= CR1_JDISCEN_Set;
  }
  else
  {
    /* Disable the selected ADC injected discontinuous mode */
    ADCx->CR1 &= CR1_JDISCEN_Reset;
  }
}

/**
  * @brief  Configures the ADCx external trigger for injected channels conversion.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_ExternalTrigInjecConv: specifies the ADC trigger to start injected conversion. 
  *   This parameter can be one of the following values:
  *     @arg ADC_ExternalTrigInjecConv_T1_TRGO: Timer1 TRGO event selected (for ADC1, ADC2 and ADC3)
  *     @arg ADC_ExternalTrigInjecConv_T1_CC4: Timer1 capture compare4 selected (for ADC1, ADC2 and ADC3)
  *     @arg ADC_ExternalTrigInjecConv_T2_TRGO: Timer2 TRGO event selected (for ADC1 and ADC2)
  *     @arg ADC_ExternalTrigInjecConv_T2_CC1: Timer2 capture compare1 selected (for ADC1 and ADC2)
  *     @arg ADC_ExternalTrigInjecConv_T3_CC4: Timer3 capture compare4 selected (for ADC1 and ADC2)
  *     @arg ADC_ExternalTrigInjecConv_T4_TRGO: Timer4 TRGO event selected (for ADC1 and ADC2)
  *     @arg ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4: External interrupt line 15 or Timer8
  *                                                       capture compare4 event selected (for ADC1 and ADC2)                       
  *     @arg ADC_ExternalTrigInjecConv_T4_CC3: Timer4 capture compare3 selected (for ADC3 only)
  *     @arg ADC_ExternalTrigInjecConv_T8_CC2: Timer8 capture compare2 selected (for ADC3 only)                         
  *     @arg ADC_ExternalTrigInjecConv_T8_CC4: Timer8 capture compare4 selected (for ADC3 only)
  *     @arg ADC_ExternalTrigInjecConv_T5_TRGO: Timer5 TRGO event selected (for ADC3 only)                         
  *     @arg ADC_ExternalTrigInjecConv_T5_CC4: Timer5 capture compare4 selected (for ADC3 only)                        
  *     @arg ADC_ExternalTrigInjecConv_None: Injected conversion started by software and not
  *                                          by external trigger (for ADC1, ADC2 and ADC3)
  * @retval None
  */
void ADC_ExternalTrigInjectedConvConfig(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConv)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_EXT_INJEC_TRIG(ADC_ExternalTrigInjecConv));
  /* Get the old register value */
  tmpreg = ADCx->CR2;
  /* Clear the old external event selection for injected group */
  tmpreg &= CR2_JEXTSEL_Reset;
  /* Set the external event selection for injected group */
  tmpreg |= ADC_ExternalTrigInjecConv;
  /* Store the new register value */
  ADCx->CR2 = tmpreg;
}

/**
  * @brief  Enables or disables the ADCx injected channels conversion through
  *         external trigger
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC external trigger start of
  *         injected conversion.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_ExternalTrigInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC external event selection for injected group */
    ADCx->CR2 |= CR2_JEXTTRIG_Set;
  }
  else
  {
    /* Disable the selected ADC external event selection for injected group */
    ADCx->CR2 &= CR2_JEXTTRIG_Reset;
  }
}

/**
  * @brief  Enables or disables the selected ADC start of the injected 
  *         channels conversion.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC software start injected conversion.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_SoftwareStartInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC conversion for injected group on external event and start the selected
       ADC injected conversion */
    ADCx->CR2 |= CR2_JEXTTRIG_JSWSTART_Set;
  }
  else
  {
    /* Disable the selected ADC conversion on external event for injected group and stop the selected
       ADC injected conversion */
    ADCx->CR2 &= CR2_JEXTTRIG_JSWSTART_Reset;
  }
}

/**
  * @brief  Gets the selected ADC Software start injected conversion Status.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @retval The new state of ADC software start injected conversion (SET or RESET).
  */
FlagStatus ADC_GetSoftwareStartInjectedConvCmdStatus(ADC_TypeDef* ADCx)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Check the status of JSWSTART bit */
  if ((ADCx->CR2 & CR2_JSWSTART_Set) != (uint32_t)RESET)
  {
    /* JSWSTART bit is set */
    bitstatus = SET;
  }
  else
  {
    /* JSWSTART bit is reset */
    bitstatus = RESET;
  }
  /* Return the JSWSTART bit status */
  return  bitstatus;
}

/**
  * @brief  Configures for the selected ADC injected channel its corresponding
  *         rank in the sequencer and its sample time.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure. 
  *   This parameter can be one of the following values:
  *     @arg ADC_Channel_0: ADC Channel0 selected
  *     @arg ADC_Channel_1: ADC Channel1 selected
  *     @arg ADC_Channel_2: ADC Channel2 selected
  *     @arg ADC_Channel_3: ADC Channel3 selected
  *     @arg ADC_Channel_4: ADC Channel4 selected
  *     @arg ADC_Channel_5: ADC Channel5 selected
  *     @arg ADC_Channel_6: ADC Channel6 selected
  *     @arg ADC_Channel_7: ADC Channel7 selected
  *     @arg ADC_Channel_8: ADC Channel8 selected
  *     @arg ADC_Channel_9: ADC Channel9 selected
  *     @arg ADC_Channel_10: ADC Channel10 selected
  *     @arg ADC_Channel_11: ADC Channel11 selected
  *     @arg ADC_Channel_12: ADC Channel12 selected
  *     @arg ADC_Channel_13: ADC Channel13 selected
  *     @arg ADC_Channel_14: ADC Channel14 selected
  *     @arg ADC_Channel_15: ADC Channel15 selected
  *     @arg ADC_Channel_16: ADC Channel16 selected
  *     @arg ADC_Channel_17: ADC Channel17 selected
  * @param  Rank: The rank in the injected group sequencer. This parameter must be between 1 and 4.
  * @param  ADC_SampleTime: The sample time value to be set for the selected channel. 
  *   This parameter can be one of the following values:
  *     @arg ADC_SampleTime_1Cycles5: Sample time equal to 1.5 cycles
  *     @arg ADC_SampleTime_7Cycles5: Sample time equal to 7.5 cycles
  *     @arg ADC_SampleTime_13Cycles5: Sample time equal to 13.5 cycles
  *     @arg ADC_SampleTime_28Cycles5: Sample time equal to 28.5 cycles	
  *     @arg ADC_SampleTime_41Cycles5: Sample time equal to 41.5 cycles	
  *     @arg ADC_SampleTime_55Cycles5: Sample time equal to 55.5 cycles	
  *     @arg ADC_SampleTime_71Cycles5: Sample time equal to 71.5 cycles	
  *     @arg ADC_SampleTime_239Cycles5: Sample time equal to 239.5 cycles	
  * @retval None
  */
void ADC_InjectedChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime)
{
  uint32_t tmpreg1 = 0, tmpreg2 = 0, tmpreg3 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CHANNEL(ADC_Channel));
  assert_param(IS_ADC_INJECTED_RANK(Rank));
  assert_param(IS_ADC_SAMPLE_TIME(ADC_SampleTime));
  /* if ADC_Channel_10 ... ADC_Channel_17 is selected */
  if (ADC_Channel > ADC_Channel_9)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR1;
    /* Calculate the mask to clear */
    tmpreg2 = SMPR1_SMP_Set << (3*(ADC_Channel - 10));
    /* Clear the old channel sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3*(ADC_Channel - 10));
    /* Set the new channel sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR1 = tmpreg1;
  }
  else /* ADC_Channel include in ADC_Channel_[0..9] */
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR2;
    /* Calculate the mask to clear */
    tmpreg2 = SMPR2_SMP_Set << (3 * ADC_Channel);
    /* Clear the old channel sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * ADC_Channel);
    /* Set the new channel sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR2 = tmpreg1;
  }
  /* Rank configuration */
  /* Get the old register value */
  tmpreg1 = ADCx->JSQR;
  /* Get JL value: Number = JL+1 */
  tmpreg3 =  (tmpreg1 & JSQR_JL_Set)>> 20;
  /* Calculate the mask to clear: ((Rank-1)+(4-JL-1)) */
  tmpreg2 = JSQR_JSQ_Set << (5 * (uint8_t)((Rank + 3) - (tmpreg3 + 1)));
  /* Clear the old JSQx bits for the selected rank */
  tmpreg1 &= ~tmpreg2;
  /* Calculate the mask to set: ((Rank-1)+(4-JL-1)) */
  tmpreg2 = (uint32_t)ADC_Channel << (5 * (uint8_t)((Rank + 3) - (tmpreg3 + 1)));
  /* Set the JSQx bits for the selected rank */
  tmpreg1 |= tmpreg2;
  /* Store the new register value */
  ADCx->JSQR = tmpreg1;
}

/**
  * @brief  Configures the sequencer length for injected channels
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  Length: The sequencer length. 
  *   This parameter must be a number between 1 to 4.
  * @retval None
  */
void ADC_InjectedSequencerLengthConfig(ADC_TypeDef* ADCx, uint8_t Length)
{
  uint32_t tmpreg1 = 0;
  uint32_t tmpreg2 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_INJECTED_LENGTH(Length));
  
  /* Get the old register value */
  tmpreg1 = ADCx->JSQR;
  /* Clear the old injected sequnence lenght JL bits */
  tmpreg1 &= JSQR_JL_Reset;
  /* Set the injected sequnence lenght JL bits */
  tmpreg2 = Length - 1; 
  tmpreg1 |= tmpreg2 << 20;
  /* Store the new register value */
  ADCx->JSQR = tmpreg1;
}

/**
  * @brief  Set the injected channels conversion value offset
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_InjectedChannel: the ADC injected channel to set its offset. 
  *   This parameter can be one of the following values:
  *     @arg ADC_InjectedChannel_1: Injected Channel1 selected
  *     @arg ADC_InjectedChannel_2: Injected Channel2 selected
  *     @arg ADC_InjectedChannel_3: Injected Channel3 selected
  *     @arg ADC_InjectedChannel_4: Injected Channel4 selected
  * @param  Offset: the offset value for the selected ADC injected channel
  *   This parameter must be a 12bit value.
  * @retval None
  */
void ADC_SetInjectedOffset(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel, uint16_t Offset)
{
  __IO uint32_t tmp = 0;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_INJECTED_CHANNEL(ADC_InjectedChannel));
  assert_param(IS_ADC_OFFSET(Offset));  
  
  tmp = (uint32_t)ADCx;
  tmp += ADC_InjectedChannel;
  
  /* Set the selected injected channel data offset */
  *(__IO uint32_t *) tmp = (uint32_t)Offset;
}

/**
  * @brief  Returns the ADC injected channel conversion result
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_InjectedChannel: the converted ADC injected channel.
  *   This parameter can be one of the following values:
  *     @arg ADC_InjectedChannel_1: Injected Channel1 selected
  *     @arg ADC_InjectedChannel_2: Injected Channel2 selected
  *     @arg ADC_InjectedChannel_3: Injected Channel3 selected
  *     @arg ADC_InjectedChannel_4: Injected Channel4 selected
  * @retval The Data conversion value.
  */
uint16_t ADC_GetInjectedConversionValue(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel)
{
  __IO uint32_t tmp = 0;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_INJECTED_CHANNEL(ADC_InjectedChannel));

  tmp = (uint32_t)ADCx;
  tmp += ADC_InjectedChannel + JDR_Offset;
  
  /* Returns the selected injected channel conversion data value */
  return (uint16_t) (*(__IO uint32_t*)  tmp);   
}

/**
  * @brief  Enables or disables the analog watchdog on single/all regular
  *         or injected channels
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_AnalogWatchdog: the ADC analog watchdog configuration.
  *   This parameter can be one of the following values:
  *     @arg ADC_AnalogWatchdog_SingleRegEnable: Analog watchdog on a single regular channel
  *     @arg ADC_AnalogWatchdog_SingleInjecEnable: Analog watchdog on a single injected channel
  *     @arg ADC_AnalogWatchdog_SingleRegOrInjecEnable: Analog watchdog on a single regular or injected channel
  *     @arg ADC_AnalogWatchdog_AllRegEnable: Analog watchdog on  all regular channel
  *     @arg ADC_AnalogWatchdog_AllInjecEnable: Analog watchdog on  all injected channel
  *     @arg ADC_AnalogWatchdog_AllRegAllInjecEnable: Analog watchdog on all regular and injected channels
  *     @arg ADC_AnalogWatchdog_None: No channel guarded by the analog watchdog
  * @retval None	  
  */
void ADC_AnalogWatchdogCmd(ADC_TypeDef* ADCx, uint32_t ADC_AnalogWatchdog)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_ANALOG_WATCHDOG(ADC_AnalogWatchdog));
  /* Get the old register value */
  tmpreg = ADCx->CR1;
  /* Clear AWDEN, AWDENJ and AWDSGL bits */
  tmpreg &= CR1_AWDMode_Reset;
  /* Set the analog watchdog enable mode */
  tmpreg |= ADC_AnalogWatchdog;
  /* Store the new register value */
  ADCx->CR1 = tmpreg;
}

/**
  * @brief  Configures the high and low thresholds of the analog watchdog.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  HighThreshold: the ADC analog watchdog High threshold value.
  *   This parameter must be a 12bit value.
  * @param  LowThreshold: the ADC analog watchdog Low threshold value.
  *   This parameter must be a 12bit value.
  * @retval None
  */
void ADC_AnalogWatchdogThresholdsConfig(ADC_TypeDef* ADCx, uint16_t HighThreshold,
                                        uint16_t LowThreshold)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_THRESHOLD(HighThreshold));
  assert_param(IS_ADC_THRESHOLD(LowThreshold));
  /* Set the ADCx high threshold */
  ADCx->HTR = HighThreshold;
  /* Set the ADCx low threshold */
  ADCx->LTR = LowThreshold;
}

/**
  * @brief  Configures the analog watchdog guarded single channel
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure for the analog watchdog. 
  *   This parameter can be one of the following values:
  *     @arg ADC_Channel_0: ADC Channel0 selected
  *     @arg ADC_Channel_1: ADC Channel1 selected
  *     @arg ADC_Channel_2: ADC Channel2 selected
  *     @arg ADC_Channel_3: ADC Channel3 selected
  *     @arg ADC_Channel_4: ADC Channel4 selected
  *     @arg ADC_Channel_5: ADC Channel5 selected
  *     @arg ADC_Channel_6: ADC Channel6 selected
  *     @arg ADC_Channel_7: ADC Channel7 selected
  *     @arg ADC_Channel_8: ADC Channel8 selected
  *     @arg ADC_Channel_9: ADC Channel9 selected
  *     @arg ADC_Channel_10: ADC Channel10 selected
  *     @arg ADC_Channel_11: ADC Channel11 selected
  *     @arg ADC_Channel_12: ADC Channel12 selected
  *     @arg ADC_Channel_13: ADC Channel13 selected
  *     @arg ADC_Channel_14: ADC Channel14 selected
  *     @arg ADC_Channel_15: ADC Channel15 selected
  *     @arg ADC_Channel_16: ADC Channel16 selected
  *     @arg ADC_Channel_17: ADC Channel17 selected
  * @retval None
  */
void ADC_AnalogWatchdogSingleChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CHANNEL(ADC_Channel));
  /* Get the old register value */
  tmpreg = ADCx->CR1;
  /* Clear the Analog watchdog channel select bits */
  tmpreg &= CR1_AWDCH_Reset;
  /* Set the Analog watchdog channel */
  tmpreg |= ADC_Channel;
  /* Store the new register value */
  ADCx->CR1 = tmpreg;
}

/**
  * @brief  Enables or disables the temperature sensor and Vrefint channel.
  * @param  NewState: new state of the temperature sensor.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_TempSensorVrefintCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the temperature sensor and Vrefint channel*/
    ADC1->CR2 |= CR2_TSVREFE_Set;
  }
  else
  {
    /* Disable the temperature sensor and Vrefint channel*/
    ADC1->CR2 &= CR2_TSVREFE_Reset;
  }
}

/**
  * @brief  Checks whether the specified ADC flag is set or not.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_FLAG: specifies the flag to check. 
  *   This parameter can be one of the following values:
  *     @arg ADC_FLAG_AWD: Analog watchdog flag
  *     @arg ADC_FLAG_EOC: End of conversion flag
  *     @arg ADC_FLAG_JEOC: End of injected group conversion flag
  *     @arg ADC_FLAG_JSTRT: Start of injected group conversion flag
  *     @arg ADC_FLAG_STRT: Start of regular group conversion flag
  * @retval The new state of ADC_FLAG (SET or RESET).
  */
FlagStatus ADC_GetFlagStatus(ADC_TypeDef* ADCx, uint8_t ADC_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_GET_FLAG(ADC_FLAG));
  /* Check the status of the specified ADC flag */
  if ((ADCx->SR & ADC_FLAG) != (uint8_t)RESET)
  {
    /* ADC_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* ADC_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the ADC_FLAG status */
  return  bitstatus;
}

/**
  * @brief  Clears the ADCx's pending flags.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_FLAG: specifies the flag to clear. 
  *   This parameter can be any combination of the following values:
  *     @arg ADC_FLAG_AWD: Analog watchdog flag
  *     @arg ADC_FLAG_EOC: End of conversion flag
  *     @arg ADC_FLAG_JEOC: End of injected group conversion flag
  *     @arg ADC_FLAG_JSTRT: Start of injected group conversion flag
  *     @arg ADC_FLAG_STRT: Start of regular group conversion flag
  * @retval None
  */
void ADC_ClearFlag(ADC_TypeDef* ADCx, uint8_t ADC_FLAG)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CLEAR_FLAG(ADC_FLAG));
  /* Clear the selected ADC flags */
  ADCx->SR = ~(uint32_t)ADC_FLAG;
}

/**
  * @brief  Checks whether the specified ADC interrupt has occurred or not.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt source to check. 
  *   This parameter can be one of the following values:
  *     @arg ADC_IT_EOC: End of conversion interrupt mask
  *     @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *     @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  * @retval The new state of ADC_IT (SET or RESET).
  */
ITStatus ADC_GetITStatus(ADC_TypeDef* ADCx, uint16_t ADC_IT)
{
  ITStatus bitstatus = RESET;
  uint32_t itmask = 0, enablestatus = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_GET_IT(ADC_IT));
  /* Get the ADC IT index */
  itmask = ADC_IT >> 8;
  /* Get the ADC_IT enable bit status */
  enablestatus = (ADCx->CR1 & (uint8_t)ADC_IT) ;
  /* Check the status of the specified ADC interrupt */
  if (((ADCx->SR & itmask) != (uint32_t)RESET) && enablestatus)
  {
    /* ADC_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* ADC_IT is reset */
    bitstatus = RESET;
  }
  /* Return the ADC_IT status */
  return  bitstatus;
}

/**
  * @brief  Clears the ADCx's interrupt pending bits.
  * @param  ADCx: where x can be 1, 2 or 3 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg ADC_IT_EOC: End of conversion interrupt mask
  *     @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *     @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  * @retval None
  */
void ADC_ClearITPendingBit(ADC_TypeDef* ADCx, uint16_t ADC_IT)
{
  uint8_t itmask = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_IT(ADC_IT));
  /* Get the ADC IT index */
  itmask = (uint8_t)(ADC_IT >> 8);
  /* Clear the selected ADC interrupt pending bits */
  ADCx->SR = ~(uint32_t)itmask;
}

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
