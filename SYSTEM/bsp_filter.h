#ifndef __BSP_FILTER_H
#define __BSP_FILTER_H

#include "stm32f10x.h"
#include <stdio.h>

typedef struct{
	float alaph;
	float pre_out;
	float pre_in;
}Filter;

void Filter_Init(Filter *f);
float FilterValue(Filter *f, float in);

#endif











































