#include "pid.h"
#include "sys_conf.h"

PID  pid_inst[MAX_EEVNUM];
pid_change_st pid_change_inst;

extern float integral_coefficient;
uint8_t is_inited_pidCalc = 0x00;
//kc: ��������;
//ti: ����ʱ��;
//td: ΢��ʱ��;
//t0: ����ʱ��;
//pid������ʼ��
void pidInit(PID *pid,uint16_t kc,uint16_t ti,uint16_t td,uint16_t ts)
{
	pid->err = 0;
	pid->lastErr = 0;
	pid->preErr = 0;
	if(ti==0)
	{
		pid->Kp = kc * (1 + (float)td / ts);		
	}
	else
	{
		pid->Kp = kc * (1 + (float)ts/ti + (float)td / ts);		
	}
	pid->Ki = kc * (1 + 2*(float)td/ts) * (-1);
	pid->Kd = kc * (float)td /ts;

//	pid->Kp = (float)kc;
//	pid->Ki = 0;
//	pid->Kd = 0;
}

//PID��������
int16_t pidCalc(PID *pid,int16_t thisErr,uint16_t band)
{
	int16_t increment = 0;
	pid->err = thisErr;
	if(ABS(pid->err) < band)		//�Ƿ���������Χ��
	{
		return 0;
	}
	increment = (pid->Kp * pid->err + pid->Ki * pid->lastErr + pid->Kd * pid->preErr)/10;  //�����ϴκ�ǰ�εĲ�ֵ
	pid->preErr = pid->lastErr;
	pid->lastErr = pid->err;
	return increment;
} 







