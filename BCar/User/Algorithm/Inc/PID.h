#ifndef __PID_H
#define __PID_H

#include "main.h"
#include "math.h"

typedef struct {
	float Target;
	float Actual;
	float Actual1;
	float Out;
	
	float Kp;
	float Ki;
	float Kd;
	
	float Error0;
	float Error1;
	
	float POut;
	float IOut;
	float DOut;
	
	float OutMax;
	float OutMin;

	float OutOffset_Positive;  // 正向输出补偿（输出>0时叠加）
	float OutOffset_Negative;  // 负向输出补偿（输出<0时叠加）
} PID_t;

void PID_Init(PID_t *p);
void PID_Update(PID_t *p);

#endif

