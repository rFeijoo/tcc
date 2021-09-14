#include "objects_def.h"

debug_mod	 *dbg;
photovoltaic *cell;

void objects_def_init(void)
{
	dbg  = events_initialize_debug_mod("Debug Mod 1", GPIOA, CELL_1_DBG1_Pin, CELL_1_DBG2_Pin, CELL_1_DBG3_Pin);

	cell = meas_initialize_objects("Photovoltaic Cell 1", &hadc1, &hadc2, dbg);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
		meas_sample(cell);
	else if (hadc->Instance == ADC5)
		meas_temperature(cell);
}

void objects_def_exti_gpio(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == CELL_1_BTN_Pin)
		meas_decouple_system(cell);
}

void objects_def_loop(void)
{
	events_handler(cell);
}
