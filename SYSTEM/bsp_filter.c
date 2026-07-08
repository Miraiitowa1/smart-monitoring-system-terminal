#include "bsp_filter.h"

void Filter_Init(Filter *f)
{
	f->alaph = 0.2;
	f->pre_in = 0.0;
	f->pre_out = 0.0;
}

float FilterValue(Filter *f, float in)
{
	//低通 out(t)=a*in(t)+(1-a)*out(t-1) 
	float OutL = f->alaph * in + (1 - f->alaph) * f->pre_out;
	//高通 outH(t)=a*out(t-1)+a*(in(t)-in(t-1))
	float OutH = f->alaph * f->pre_in + f->alaph * (in - f->pre_in);
	//互补 out(t)=a*OutL+(1-a)*OutH
	float Out = f->alaph * OutL + (1 - f->alaph) * OutH;
	
	f->pre_in = in;
	f->pre_out = Out;
	
	return Out;
}