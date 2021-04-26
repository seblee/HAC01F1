#include "cmsis_os.h"
#include "stm32f0xx.h"
#include "led.h"
#include "pid.h"
#include "sys_conf.h"
#include "eev_proc.h"
#include "i2c_bsp.h"
#include "dio.h"
#include "Lib.h"
#include "fanctrl.h"
/*********************************************************
  * @name   bkg_proc
    * @brief  system background threads, only toggle led in this program.
    * @calls  led_open()
            led_close()
                        osDelay()
  * @called main()
  * @param  None
  * @retval None
*********************************************************/
// uint8_t rd_data_buf[5] = {0,0,0,0,0};
extern void alarm_acl_exe_process(void);
__IO uint32_t LsiFreq = 40000;

void fill_pid_para(void)  //填充pid 参数(初始化或参数修改)
{
    pid_change_inst.kc = g_sys.config.algorithm.prop_gain;
    pid_change_inst.ti = g_sys.config.algorithm.integ_time;
    pid_change_inst.td = g_sys.config.algorithm.diff_time;
    pid_change_inst.ts = g_sys.config.algorithm.samp_time;
    //		if(g_sys.config.general.Cool_Type==Fixed)
    //		{
    //				g_sys.status.Cal_Time=2;	//2S
    //		}
    //		else
    //		{
    //			g_sys.status.Cal_Time=g_sys.config.algorithm.samp_time;
    //		}
    //    pid_change_inst.ts = g_sys.status.Cal_Time;
    pidInit(&pid_inst[EEV1], g_sys.config.algorithm.prop_gain, g_sys.config.algorithm.integ_time,
            g_sys.config.algorithm.diff_time, pid_change_inst.ts);
    pidInit(&pid_inst[EEV2], g_sys.config.algorithm.prop_gain, g_sys.config.algorithm.integ_time,
            g_sys.config.algorithm.diff_time, pid_change_inst.ts);
}

void change_pid_para(void)
{
    if ((pid_change_inst.kc != g_sys.config.algorithm.prop_gain) ||
        (pid_change_inst.ti != g_sys.config.algorithm.integ_time) ||
        (pid_change_inst.td != g_sys.config.algorithm.diff_time) ||
        (pid_change_inst.ts != g_sys.config.algorithm.samp_time))
    {
        fill_pid_para();
    }
}
/**
 * @brief  WWDG configuration
 * @param  None
 * @retval None
 */
static void iwdg_init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI/32 */
    IWDG_SetPrescaler(IWDG_Prescaler_32);

    /* Set counter reload value to obtain 250ms IWDG TimeOut.
       Counter Reload Value = 250ms/IWDG counter clock period
                            = 250ms / (LSI/32)
                            = 0.25s / (LsiFreq/32)
                            = LsiFreq/(32 * 4)
                            = LsiFreq/128
     */
    IWDG_SetReload(0xc35);  // 2.5s

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}

////运行时间
// static void run_time_process(void)
//{
//    extern sys_reg_st g_sys;
//    extern local_reg_st l_sys;
//    //		time_t now;
//    uint16_t i;
//    static uint16_t u16Sec = 0;

//    //过滤网运行时间累计
//    if ((g_sys.status.dout_bitmap[0] & (0x0001 << DO_FAN_BPOS)) != 0)
//    {
//        g_sys.status.ComSta.u16Runtime[0][DO_FILLTER_DUMMY_BPOS]++;
//        time_calc(&g_sys.status.ComSta.u16Runtime[0][DO_FILLTER_DUMMY_BPOS],
//                  &g_sys.status.ComSta.u16Runtime[1][DO_FILLTER_DUMMY_BPOS]);
//        g_sys.status.ComSta.u16Runtime[0][DO_SOURCE_TANK_BPOS]++;
//        time_calc(&g_sys.status.ComSta.u16Runtime[0][DO_SOURCE_TANK_BPOS],
//                  &g_sys.status.ComSta.u16Runtime[1][DO_SOURCE_TANK_BPOS]);
//    }

//    // get_local_time(&now);
//    // if ((now % FIXED_SAVETIME) == 0) //每15分钟保存一次
//    u16Sec++;
//    // rt_kprintf("adc_value=%d\n", u16Sec);
//    if ((u16Sec % FIXED_SAVETIME) == 0)  //每15分钟保存一次
//    {
//        u16Sec = 0;
//        I2C_EE_BufWrite((uint8_t *)&g_sys.status.ComSta.u16Runtime, STS_REG_EE1_ADDR,
//                        sizeof(g_sys.status.ComSta.u16Runtime));  // when, fan is working update eeprom every minite
//    }
//    return;
//}

void Systime_cal(void)
{
    //通讯模式
    if (g_sys.config.g_u8CfgParameter[CFGCOMMODE])
    {
        if (g_sys.status.Com_error[0] <= COMERR_5S * 10)
        {
            g_sys.status.Com_error[0]++;
        }

        if (g_sys.status.Com_error[1] <= g_sys.config.alarm[ACL_COMMON].alarm_param * 10)
        {
            g_sys.status.Com_error[1]++;
        }
    }
    else
    {
        g_sys.status.Com_error[0] = 0;
        g_sys.status.Com_error[1] = 0;
    }
}

void bkg_proc(void const *argument)
{
    extern sys_reg_st g_sys;
    static uint8_t num[2] = {0};
    led_init();
    alarm_acl_init();
    iwdg_init();  // set iwdg period to 2.5s
    while (1)
    {
        TimeTask();  //时间统计
        alarm_acl_exe_process();
        FanCtrlStatus();  // LED运行状态
        Systime_cal();
        if (++num[0] >= 5)
        {
            num[0] = 0;
            IWDG_ReloadCounter();
            //			led_toggle(LED_RUN);
        }
        if (++num[1] >= 10)
        {
            num[1] = 0;
            DI_update();
            daq_gvar_update();  //与设定值进行比较，得到偏差

            // alarm_acl_exe(0);
        }
        osDelay(BKG_PROC_DLY);
    }
}
