#include "objects_def.h"

debug_mod	 *dbg;
digital_IOs  *relay_pos, *relay_neg, *builtin_led;
photovoltaic *cell;

void objects_def_init(void)
{
	relay_pos = events_initialize_digital_ios("Relay (+)", GPIOA, CELL_1_OUT_POS_Pin, 0);
	relay_neg = events_initialize_digital_ios("Relay (-)", GPIOC, CELL_1_OUT_NEG_Pin, 0);

	builtin_led = events_initialize_digital_ios("Built-In LED", GPIOA, CELL_1_BUILTIN_LED_Pin, 0);

	dbg  = events_initialize_debug_mod("Debug Module", GPIOB, CELL_1_DBG1_Pin, CELL_1_DBG2_Pin, CELL_1_DBG3_Pin);

	cell = meas_initialize_cell("Photovoltaic Cell 1", &hadc1, &hadc2, relay_pos, relay_neg, builtin_led, dbg);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
//	if (hadc->Instance == ADC1)
//		meas_sample_voltage_and_current(cell);
//	else if (hadc->Instance == ADC5)
//		meas_temperature(cell);
}

void objects_def_exti_gpio(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == CELL_1_BTN_Pin) {
		events_change_state(cell, EVENT_USER_BREAK);

		printf("user btn\n");
	}
	else if (GPIO_Pin == CELL_1_DPS_Pin) {
		events_change_state(cell, EVENT_DPS_LIFESPAN);

		printf("DPS lifespan\n");
	}
}

void objects_def_loop(void)
{
	events_handler(cell);
}
