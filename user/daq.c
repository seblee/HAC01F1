#include "daq.h"
#include "sys_conf.h"
#include "calc.h"

#define NTC_TEMP_SCALE   191
#define NTC_TEMP_OFFSET  39		
#define NTC_TEMP_DT 15
//#define K_FACTOR_HI_PRESS 174
#define K_FACTOR_HI_PRESS 124
#define K_FACTOR_HI_ZERO 62
#define K_FACTOR_LO_PRESS 232


#define ACL_TEM_MAX 1400 //0.1??
#define ACL_TEM_MIN -280

extern  sys_reg_st g_sys;

const uint16_t ntc_lookup_tab[NTC_TEMP_SCALE] = 
{
	193,204,215,227,239,252,265,279,294,309,324,340,357,375,393,411,431,451,471,
	492,514,537,560,584,609,635,661,687,715,743,772,801,831,862,893,925,957,991,
  1024,1058,1093,1128,1164,1200,1236,1273,1310,1348,1386,1424,1463,1501,1540,
  1579,1618,1657,1697,1736,1775,1815,1854,1893,1932,1971,2010,2048,2087,2125,
  2163,2200,2238,2274,2311,2347,2383,2418,2453,2488,2522,2556,2589,2622,2654,
  2685,2717,2747,2777,2807,2836,2865,2893,2921,2948,2974,3000,3026,3051,3075,
  3099,3123,3146,3168,3190,3212,3233,3253,3273,3293,3312,3331,3349,3367,3384,
  3401,3418,3434,3450,3465,3481,3495,3510,3524,3537,3551,3564,3576,3588,3600,
  3612,3624,3635,3646,3656,3667,3677,3686,3696,3705,3714,3723,3732,3740,3748,
  3756,3764,3772,3779,3786,3793,3800,3807,3813,3820,3826,3832,3838,3844,3849,
  3855,3860,3865,3870,3875,3880,3884,3889,3893,3898,3902,3906,3910,3914,3918,
  3922,3925,3929,3932,3936,3939,3942,3945,3949,3952,3954,3957,3960,3963,3966,
  3968,3971,3973
};

enum 
{
	R22,
	R134A,
	R407C,
	R410A,
	MAXREFRIG
};

const int16_t refrigerant[MAXREFRIG][69]={
	/****R22*****/
	{
		-411,-321,-252,-195,-146,-104,-65,-31,1,31,
		59,85,109,132,155,176,196,215,234,252,
		269,286,303,318,334,349,363,377,391,404,
		417,430,443,455,467,479,490,502,513,524,
		534,545,555,565,575,585,595,604,614,623,
		632,641,650,658,667,676,684,692,700,708,
		716,724,732,740,747,755,762,770,777
	},   //--0.0-34.0bar (0.5bar step)
	/****R134A*****/
	{
		-264,-171,-101,-43,7,50,89,125,157,187,
		216,242,267,291,313,335,355,375,394,412,
		430,447,463,479,495,510,524,538,552,566,
		579,592,605,617,629,641,652,664,675,686,
		696,707,717,727,737,747,757,766,776,785,
		794,803,812,820,829,837,846,854,862,871,
		879,888,896,905,914,923,933,943,954
	},
	/****R407C*****/
	{
		-369,-283,-217,-163,-117,-76,-40,-7,24,52,
		78,102,126,148,168,188,207,226,243,260,
		276,292,307,322,336,350,364,377,390,402,
		414,426,438,449,460,471,482,492,502,512,
		522,532,541,551,560,569,578,587,595,604,
		612,620,628,636,644,651,659,667,674,681,
		688,695,702,709,716,723,729,736,742
	},
	/****R410A*****/
	{
		-505,-431,-370,-318,-273,-234,-199,-168,-139,-112,
		-86,-63,-40,-19,1,20,38,56,73,89,
		105,120,134,149,163,176,189,202,214,226,
		238,249,261,272,282,293,303,313,323,333,
		343,352,361,370,379,388,397,405,414,422,
		430,438,446,454,461,469,476,484,491,498,
		505,512,519,526,533,539,546,552,559
	}
};


static int16_t calc_ntc(uint16_t adc_value,int16_t adjust)
{	
		int16_t ntc_temp;
		int16_t index;
		uint16_t offset;
		adc_value= 4096 - adc_value;
		index = bin_search((uint16_t *)ntc_lookup_tab, (NTC_TEMP_SCALE-1), adc_value);  		
		if(index < 0)
		{
				return ABNORMAL_VALUE;				
		}
		else
		{
				offset = (adc_value-ntc_lookup_tab[index-1])*10 / (ntc_lookup_tab[index]-ntc_lookup_tab[index-1]);
				ntc_temp = ( index- NTC_TEMP_OFFSET)*10 + offset + adjust - NTC_TEMP_DT;
				if((ntc_temp > ACL_TEM_MAX)||(ntc_temp < ACL_TEM_MIN))
				{
					return ABNORMAL_VALUE;
				}
				return ntc_temp;
		}		
}

//calculate the hi pressure sensor analog input
//k_factor has 3-valid-digitals integer
static int16_t calc_hi_press_ai(uint16_t adc_value,uint16_t k_factor,int16_t cali)
{
  extern sys_reg_st	g_sys; 
	int32_t ret_val = 0;
	uint16_t u16ADC_OFFSET;
	int16_t i16PressBand=g_sys.config.general.pressMAXBar - (int16_t)g_sys.config.general.pressMINBar;
	int16_t i16VolBand=g_sys.config.general.maxVol-g_sys.config.general.minVol;
	
	g_sys.status.Test_Buff[1]=adc_value;
	u16ADC_OFFSET=g_sys.config.general.minVol/100*4096/4/33-3;
	if(adc_value < u16ADC_OFFSET)//0.5V
	{
		return 0;
	}
	if(adc_value>=(4096-96))//³¬ÏÞ
	{
		return ABNORMAL_VALUE;		
	}
	ret_val = (adc_value-u16ADC_OFFSET) * 33 * i16PressBand/i16VolBand*100;    //µçÑ¹ÅäÖÃÀ©´ó1000±¶
	ret_val >>= 10;			// /1024
	ret_val += ((int16_t)g_sys.config.general.pressMINBar + cali);
	return (int16_t)ret_val;    

//	i32ADC_VO=(int32_t)(adc_value)*999/249;
//	g_sys.status.Test_Buff[2]=i32ADC_VO;
//	ret_val = i32ADC_VO * (g_sys.config.general.pressMAXBar - (int16_t)g_sys.config.general.pressMINBar)/(u16PVmax-u16PVmin);    //¦Ì??1????¨¤?¡ä¨®10¡À?
//	ret_val /= 10;
//	ret_val += ((int16_t)g_sys.config.general.pressMINBar + cali);
//	return (int16_t)ret_val;    
}

unsigned short ADC_Filter2(uint8_t u8ADC_Para)
{
    extern volatile uint16_t ADC1Buff[NUM_2][ADC1_PER];
    extern volatile uint16_t Filter[ADC1_PER][NUM_2];
		extern  uint16_t ADC1ConvertedValue[AI_MAX_CNT];
		unsigned char i=0;
		unsigned char k=0;

		if(u8ADC_Para==ADC_T1)
		{
				for (k = 0; k < ADC1_PER; k++)
				{
					for (i = 0; i < NUM_2; i++)
					{
						Filter[k][i]=ADC1Buff[i][k];	
					}
					ADC1ConvertedValue[k]=MedianFilter((uint16_t *)Filter[k],NUM_2);
				}
//				k=AI_NTC1-8;
//						Filter[k][i]=ADC1ConvertedValue[k][0];	
//				rt_kprintf("Filter[k][0] = %d,Filter[k][1] = %d,Filter[k][2] = %d,Filter[k][3] = %d,Filter[k][4] = %d,ADC1ConvertedValue[k][0] = %d\n", Filter[k][0],Filter[k][1],Filter[k][2],Filter[k][3],Filter[k][4],ADC1ConvertedValue[k][0] );	
//						ADC1ConvertedValue[k][1]=MedianFilter((uint16_t *)Filter[k],NUM_2);
		}
		else
		{
		}
		
    return 0;
}
//ÖÐÖµ·¨ÂË²¨
#define CNT 5
int16_t AVGfilter2(uint8_t i8Type,int16_t i16Value)
{
		UINT8 i;
		static UINT8 i8Num[AI_MAX_CNT]={0};
		static int16_t i16Value_buf[AI_MAX_CNT][CNT];
		int  sum=0;
		
		if(i8Num[i8Type]<CNT)
		{
			i8Num[i8Type]++;
		}
		else
		{
			i8Num[i8Type]=0;		
		}
		i16Value_buf[i8Type][i8Num[i8Type]] = i16Value;		
		bubble_sort(i16Value_buf[i8Type],CNT);
		for(i=1;i<CNT-1;i++)
		{
			sum += i16Value_buf[i8Type][i];
		}		
		sum/=(CNT-2);
		return (int16_t)(sum);		
}


void ai_sts_update(void)
{
		extern volatile uint16_t ADC1Buff[NUM_2][ADC1_PER];
		uint16_t ain_mask_bitmap;
		uint16_t i;
		static 	uint16_t  s_u16AVGain[AI_MAX_CNT];
		static UINT8 s_u8StartPress = 0;
		ain_mask_bitmap = g_sys.config.dev_mask.ain;
	
			ADC_Filter2(ADC_T1);
			g_sys.status.Test_Buff[0]=ADC1ConvertedValue[0];	
//	    for(i = 0;i < AI_MAX_CNT;i++)
//		{
//			ADCValProcess(ADC1ConvertedValue,ADC1Buff,i);
//		}
    	//high_pressure sensor caculation    
		if((ain_mask_bitmap&(0x0001<<AI_SENSOR1)) != 0)
		{
				g_sys.status.ain[AI_SENSOR1] = calc_hi_press_ai(ADC1ConvertedValue[AI_SENSOR1],K_FACTOR_HI_PRESS,g_sys.config.general.ai_cali[AI_SENSOR1]);
		}
		else
		{
			g_sys.status.ain[AI_SENSOR1] = 0;
		}
		for(i = AI_NTC1;i<AI_MAX_CNT;i++)
		{
			g_sys.status.ain[i] = (((ain_mask_bitmap>>i)&0x0001) != 0)?  calc_ntc(ADC1ConvertedValue[i],g_sys.config.general.ntc_cali[i-AI_NTC1]):ABNORMAL_VALUE;
		}
		
//		//ÂË²¨
//		for(i = AI_SENSOR1;i<AI_MAX_CNT;i++)
//		{
//			g_sys.status.ain[i] =AVGfilter2(i,g_sys.status.ain[i]);
//		}		
		//ÂË²¨
		for(i = AI_SENSOR1;i<AI_MAX_CNT;i++)
		{
			s_u16AVGain[i] =AVGfilter2(i,g_sys.status.ain[i]);
		}	
		s_u8StartPress++;
		if(s_u8StartPress>=CNT*2)
		{
			s_u8StartPress=CNT*2;
			for(i = AI_SENSOR1;i<AI_MAX_CNT;i++)
			{
				g_sys.status.ain[i] =s_u16AVGain[i];
			}				
		}
		return;
}

void daq_gvar_update(void)
{
	extern sys_reg_st	g_sys; 
	ai_sts_update();											//update g_ain_inst
//	temp_avg_process();
//	press_avg_process();
//	vapor_temp_avg_process();
}
