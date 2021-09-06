#include "objects_def.h"

photovoltaic *cell;

void objects_def_init(void)
{
	cell = meas_initialize_objects("Photovoltaic Cell", &hadc1, &hadc2);
}

void objects_def_adc_conversion_complete(ADC_HandleTypeDef *hadc, float v, float i)
{
	if (hadc->Instance == ADC1)
	{
		meas_objects_handler(cell, v, i);
	}
}
