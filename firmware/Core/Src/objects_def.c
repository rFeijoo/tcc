#include "objects_def.h"

photovoltaic *cell;
digital_IOs  *relay_pos, *relay_neg, *led_output;
debug_mod	 *dbg;

void objects_def_init(void)
{
	dbg  = events_initialize_debug_mod("Debug Mod 1", GPIOB, CELL_1_DBG1_Pin, CELL_1_DBG2_Pin, CELL_1_DBG3_Pin);

	relay_pos  = events_initialize_digital_ios("Output Relay (+)", GPIOA, CELL_1_OUT_POS_Pin, 0);
	relay_neg  = events_initialize_digital_ios("Output Relay (-)", GPIOC, CELL_1_OUT_NEG_Pin, 0);
	led_output = events_initialize_digital_ios("Output LED", GPIOA, CELL_1_OUT_LED_Pin, 0);

	cell = meas_initialize_objects("Photovoltaic Cell 1", &hadc1, &hadc2, relay_pos, relay_neg, led_output, dbg);
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
