#include "meas.h"

photovoltaic *meas_initialize_cell(char *tag, ADC_HandleTypeDef *ADC_master, ADC_HandleTypeDef *ADC_slave, digital_IOs *pos_out, digital_IOs *neg_out, digital_IOs *led_out, debug_mod *debug_mod)
{
	photovoltaic *ph_struct = (photovoltaic *)malloc(sizeof(photovoltaic));

	if (ph_struct == NULL)
		return (NULL);

	ph_struct->tag = tag;

	ph_struct->pe_interval_cnt = 0;

	ph_struct->events_handler = 0;

	printf("Initializing %s:\n", ph_struct->tag);

	ph_struct->voltage = meas_initialize_rms_objects("Voltage", ADC_master);
	ph_struct->current = meas_initialize_rms_objects("Current", ADC_slave);

	ph_struct->power_energy = meas_initialize_power_and_energy_objects();

	ph_struct->pos_out = pos_out;
	ph_struct->neg_out = neg_out;
	ph_struct->led_out = led_out;

	if (debug_mod == NULL)
	{
		ph_struct->debugger = NULL;

		printf("\t- Debugger not attached\n\n");
	}
	else
	{
		ph_struct->debugger = debug_mod;

		printf("\t- Debugger attached\n\n");
	}

	return(ph_struct);
}

rms_measurement *meas_initialize_rms_objects(char *tag, ADC_HandleTypeDef *ADC)
{
	rms_measurement *rms_struct = (rms_measurement *)malloc(sizeof(rms_measurement));

	if (rms_struct == NULL)
		return (NULL);

	rms_struct->ADC = ADC;

	rms_struct->frst_level_index = 0;
	rms_struct->scnd_level_index = 0;
	rms_struct->thrd_level_index = 0;
	rms_struct->frth_level_index = 0;
	rms_struct->ffth_level_index = 0;

	HAL_ADCEx_Calibration_Start(rms_struct->ADC, ADC_SINGLE_ENDED);

	printf("\t- %s measurement initialized\n", tag);

	return (rms_struct);
}

power_and_energy *meas_initialize_power_and_energy_objects(void)
{
	power_and_energy *pe_struct = (power_and_energy *)malloc(sizeof(power_and_energy));

	if (pe_struct == NULL)
		return (NULL);

	pe_struct->energy = 0.0;

	pe_struct->frst_level_index = 0;
	pe_struct->scnd_level_index = 0;
	pe_struct->thrd_level_index = 0;

	printf("\t- Power & Energy measurement initialized\n");

	return (pe_struct);
}

void meas_temperature(photovoltaic *ptr)
{
	// Obtém a leitura do módulo ADC, em bits
	uint32_t raw = HAL_ADC_GetValue(&hadc5);

	// Converte a leitura do módulo ADC em tensão
	float voltage = (float)raw * (SYSTEM_VCC / ADC_RESOLUTION);

	// Converte a tensão em temperatura (ºC)
	ptr->temperature = ((voltage - TEMP_SENSOR_VREF) / TEMP_SENSOR_SLOPE) + TEMP_SENSOR_TREF;

	// Verifica se o evento de superaquecimento está iniciado, para resetá-lo
	if ((ptr->events_handler & EVENT_OVERHEAT) == EVENT_OVERHEAT &&
			ptr->temperature <= OVERHEAT_RELEASE_LIMIT)
	{
		ptr->events_handler &= ~EVENT_OVERHEAT;
	}
	// Verifica se o evento de superaquecimento está resetado, para iniciá-lo
	if ((ptr->events_handler & EVENT_OVERHEAT) == 0x00 &&
			ptr->temperature >= OVERHEAT_HOLD_LIMIT)
	{
		ptr->events_handler |= EVENT_OVERHEAT;
	}
}

void meas_sample_voltage_and_current(photovoltaic *ptr)
{
	// Obtém a leitura simultânea dos módulos ADC
	uint32_t raw  = HAL_ADCEx_MultiModeGetValue(ptr->voltage->ADC);

	// Extrai a tensão a partir da leitura simultanea dos módulos ADC (16 bits LSB)
	ptr->voltage->sample = (raw & LSB_WORD_BIT_MASK) * (VCC / ADC_RES_BITS) * VOLTAGE_GAIN;

	// Extrai a corrente a partir da leitura simultanea dos módulos ADC (16 bits MSB)
	ptr->current->sample = (raw >> HALF_WORD_LENGTH) * (VCC / ADC_RES_BITS) * CURRENT_GAIN;

	// Inicia o processamento das medições de tensão e corrente
	meas_objects_handler(ptr);
}

void meas_objects_handler(photovoltaic *ptr)
{
	// Gerencia o protocolo de agregação em multi-camadas para medição de tensão
	meas_voltage_aggregation_handler(ptr);

	// Gerencia o protocolo de agregação em multi-camadas para medição de corrente
	meas_current_aggregation_handler(ptr);

	// Gerencia o protocolo de medição de potência e energia produzidas
	meas_compute_power_and_energy(ptr);
}

void meas_voltage_aggregation_handler(photovoltaic *ptr)
{
	ptr->voltage->frst_level[ptr->voltage->frst_level_index++] = ptr->voltage->sample;

	if (ptr->voltage->frst_level_index == RMS_FRST_LEVEL_LENGTH)
	{
		ptr->voltage->scnd_level_value = meas_quadratic_average(ptr->voltage->frst_level, RMS_FRST_LEVEL_LENGTH);

		ptr->voltage->scnd_level[ptr->voltage->scnd_level_index++] = ptr->voltage->scnd_level_value;

		if (ptr->voltage->scnd_level_index == RMS_SCND_LEVEL_LENGTH)
		{
			ptr->voltage->thrd_level_value = meas_quadratic_average(ptr->voltage->scnd_level, RMS_SCND_LEVEL_LENGTH);

			meas_verify_voltage_triggers(ptr);

			ptr->voltage->thrd_level[ptr->voltage->thrd_level_index++] = ptr->voltage->thrd_level_value;

			if (ptr->voltage->thrd_level_index == RMS_THRD_LEVEL_LENGTH)
			{
				ptr->voltage->frth_level[ptr->voltage->frth_level_index++] = meas_quadratic_average(ptr->voltage->thrd_level, RMS_THRD_LEVEL_LENGTH);

				if (ptr->voltage->frth_level_index == RMS_FRTH_LEVEL_LENGTH)
				{
					ptr->voltage->ffth_level[ptr->voltage->ffth_level_index++] = meas_quadratic_average(ptr->voltage->frth_level, RMS_FRTH_LEVEL_LENGTH);

					if (ptr->voltage->ffth_level_index == RMS_FFTH_LEVEL_LENGTH)
						ptr->voltage->ffth_level_index = 0;

					ptr->voltage->frth_level_index = 0;
				}

				ptr->voltage->thrd_level_index = 0;
			}

			ptr->voltage->scnd_level_index = 0;
		}

		ptr->voltage->frst_level_index = 0;
	}
}

void meas_current_aggregation_handler(photovoltaic *ptr)
{
	ptr->current->frst_level[ptr->current->frst_level_index++] = ptr->current->sample;

	if (ptr->current->frst_level_index == RMS_FRST_LEVEL_LENGTH)
	{
		ptr->current->scnd_level_value = meas_quadratic_average(ptr->current->frst_level, RMS_FRST_LEVEL_LENGTH);

		ptr->current->scnd_level[ptr->current->scnd_level_index++] = ptr->current->scnd_level_value;

		if (ptr->current->scnd_level_index == RMS_SCND_LEVEL_LENGTH)
		{
			ptr->current->thrd_level_value = meas_quadratic_average(ptr->current->scnd_level, RMS_SCND_LEVEL_LENGTH);

			meas_verify_current_triggers(ptr);

			ptr->current->thrd_level[ptr->current->thrd_level_index++] = ptr->current->thrd_level_value;

			if (ptr->current->thrd_level_index == RMS_THRD_LEVEL_LENGTH)
			{
				ptr->current->frth_level[ptr->current->frth_level_index++] = meas_quadratic_average(ptr->current->thrd_level, RMS_THRD_LEVEL_LENGTH);

				if (ptr->current->frth_level_index == RMS_FRTH_LEVEL_LENGTH)
				{
					ptr->current->ffth_level[ptr->current->ffth_level_index++] = meas_quadratic_average(ptr->current->frth_level, RMS_FRTH_LEVEL_LENGTH);

					if (ptr->current->ffth_level_index == RMS_FFTH_LEVEL_LENGTH)
						ptr->current->ffth_level_index = 0;

					ptr->current->frth_level_index = 0;
				}

				ptr->current->thrd_level_index = 0;
			}

			ptr->current->scnd_level_index = 0;
		}

		ptr->current->frst_level_index = 0;
	}
}

void meas_verify_voltage_triggers(photovoltaic *ptr)
{
	// Verifica se o evento de sobretensão está iniciado, para resetá-lo
	if ((ptr->events_handler & EVENT_OVERVOLTAGE) == EVENT_OVERVOLTAGE &&
			ptr->voltage->thrd_level_value <= OVERVOLTAGE_RELEASE_LIMIT)
	{
		ptr->events_handler &= ~EVENT_OVERVOLTAGE;
	}
	// Verifica se o evento de subtensão está iniciado, para resetá-lo
	else if ((ptr->events_handler & EVENT_UNDERVOLTAGE) == EVENT_UNDERVOLTAGE &&
			ptr->voltage->thrd_level_value >= UNDERVOLTAGE_RELEASE_LIMIT)
	{
		ptr->events_handler &= ~EVENT_UNDERVOLTAGE;
	}
	// Verifica se o evento de sobretensão está resetado, para iniciá-lo
	else if ((ptr->events_handler & EVENT_OVERVOLTAGE) == 0x00 &&
				ptr->voltage->thrd_level_value >= OVERVOLTAGE_HOLD_LIMIT)
	{
		ptr->events_handler |= EVENT_OVERVOLTAGE;
	}
	// Verifica se o evento de subtensão está resetado, para iniciá-lo
	else if ((ptr->events_handler & EVENT_UNDERVOLTAGE) == 0x00 &&
				ptr->voltage->thrd_level_value <= UNDERVOLTAGE_HOLD_LIMIT)
	{
		ptr->events_handler |= EVENT_UNDERVOLTAGE;
	}
}

void meas_verify_current_triggers(photovoltaic *ptr)
{
	// Verifica se o evento de sobrecorrente está iniciado, para resetá-lo
	if ((ptr->events_handler & EVENT_OVERCURRENT) == EVENT_OVERCURRENT &&
			ptr->current->thrd_level_value <= OVERCURRENT_RELEASE_LIMIT)
	{
		ptr->events_handler &= 0xFB;
	}
	// Verifica se o evento de sobrecorrente está resetado, para iniciá-lo
	else if ((ptr->events_handler & EVENT_OVERCURRENT) == 0x00 &&
			ptr->current->thrd_level_value >= OVERCURRENT_HOLD_LIMIT)
	{
		ptr->events_handler |= 0x04;
	}
}

void meas_compute_power_and_energy(photovoltaic *ptr)
{
	// Divisor de clock [3 segundos]
	if (!meas_power_energy_interval(ptr))
		return;

	float voltage = ptr->voltage->thrd_level[ptr->power_energy->frst_level_index];
	float current = ptr->current->thrd_level[ptr->power_energy->frst_level_index];

	// Conversão W -> KW
	float power_3s = (voltage * current) / 1000.0;

	// Conversão KW -> (KW / h)
	ptr->power_energy->energy += (power_3s / 3600.0);

	// Protocolo de agregação em multi-camadas da potência produzida
	ptr->power_energy->frst_level[ptr->power_energy->frst_level_index++] = power_3s;

	if (ptr->power_energy->frst_level_index == RMS_THRD_LEVEL_LENGTH)
	{
		ptr->power_energy->scnd_level[ptr->power_energy->scnd_level_index++] = meas_quadratic_average(ptr->power_energy->frst_level, RMS_THRD_LEVEL_LENGTH);

		if (ptr->power_energy->scnd_level_index == RMS_FRTH_LEVEL_LENGTH)
		{
			ptr->power_energy->thrd_level[ptr->power_energy->thrd_level_index++] = meas_quadratic_average(ptr->power_energy->scnd_level, RMS_FRTH_LEVEL_LENGTH);

			if (ptr->power_energy->thrd_level_index == RMS_FFTH_LEVEL_LENGTH)
				ptr->power_energy->thrd_level_index = 0;

			ptr->power_energy->scnd_level_index = 0;
		}

		ptr->power_energy->frst_level_index = 0;
	}
}

int meas_power_energy_interval(photovoltaic *ptr)
{
	ptr->pe_interval_cnt++;

	if (ptr->pe_interval_cnt == POWER_ENERGY_INTERVAL)
	{
		ptr->pe_interval_cnt = 0;

		return (1);
	}

	return (0);
}

float meas_quadratic_average(float *ptr, int length)
{
    float rms = 0.00;

    for (int i=0; i<length; i++)
    	rms += ptr[i] * ptr[i];

    return (sqrt(rms / length));
}
