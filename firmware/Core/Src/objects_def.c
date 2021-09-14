#include "objects_def.h"

debug_mod	 *dbg;
photovoltaic *cell;

void objects_def_init(void)
{
	dbg  = events_initialize_debug_mod("Debug Module 1", GPIOB, DEBUG_LED1_Pin, DEBUG_LED2_Pin, DEBUG_LED3_Pin);
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
	if (GPIO_Pin == B1_Pin)
		meas_decouple_system(cell);
}

void bin(uint8_t n)
{
    if (n > 1)
        bin(n / 2);

    printf("%d", n % 2);
}

void objects_def_loop(void)
{
	events_handler(cell);
}
