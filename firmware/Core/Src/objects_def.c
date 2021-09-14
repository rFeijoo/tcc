#include "objects_def.h"

photovoltaic *cell;

void objects_def_init(void)
{
	cell = meas_initialize_objects("Photovoltaic Cell 1", &hadc1, &hadc2);
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
		cell->events_handler = events_toggle_bit(cell->events_handler, 0x01);
}

void bin(uint8_t n)
{
    if (n > 1)
        bin(n / 2);

    printf("%d", n % 2);
}

void objects_def_loop(void)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0x01 & cell->events_handler);

	printf("Events Handler: ");
	bin(cell->events_handler);
	printf("\n");

	HAL_Delay(1000);
}
