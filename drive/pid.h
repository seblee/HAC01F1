#ifndef __PID_H
#define	__PID_H

#include "stm32f0xx.h"

#define ABS(x) ((x) > 0 ? (x): (-(x)))
#define DEADZONE	15
typedef struct _PID
{
	float Kp,Ki,Kd;       //pid ���������֡�΢��ϵ��
	int16_t lastErr;        //��һ��ƫ��ֵ
	int16_t preErr;         //���ϴε�ƫ��ֵ
	int16_t err;			//���ε�ƫ��ֵ
}PID;

typedef struct
{
		uint16_t kc;
		uint16_t ti;
		uint16_t td;
		uint16_t ts;
}pid_change_st;

extern pid_change_st pid_change_inst;

extern PID  pid_inst[];
int16_t pidCalc(PID *pid,int16_t thisErr,uint16_t band);
void pidInit(PID *pid,uint16_t kc,uint16_t ti,uint16_t td,uint16_t ts);

#endif /* __PID_H */
