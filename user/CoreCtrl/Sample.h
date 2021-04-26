#ifndef _SAMPLE_H
#define _SAMPLE_H


#include "sys_def.h"
//************************Public global variable definition************************
    	//Analog array index definition
		typedef enum _ANALOGID_
		{
    		ANALOGID_P1,
    		ANALOGID_P2,
    		ANALOGID_NTC1,
    		ANALOGID_NTC2,

 			ANALOGMAXNUM
		} ANALOGID;

	// Analog value array
	extern INT16 g_i16Analog[ANALOGMAXNUM];
	//压力传感器超范围标志
	extern UCHAR g_u8PSensorOutRangeFlag[ANALOGMAXNUM];







//************************Interface function declaration******************************
    //Analog sample and deal with
    void AnalogDeal(void);
	//Initiate ADC
	void InitAnalog(void);

#endif


