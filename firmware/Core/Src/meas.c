#include "meas.h"

photovoltaic *meas_initialize_cell(char *tag, ADC_HandleTypeDef *ADC_master, ADC_HandleTypeDef *ADC_slave, digital_IOs *relay_1, digital_IOs *relay_2, digital_IOs *LED, debug_mod *dbg_mod)
{
	photovoltaic *ph_struct = (photovoltaic *)malloc(sizeof(photovoltaic));

	if (ph_struct == NULL)
		return (NULL);

	ph_struct->tag = tag;

	printf("Initializing %s:\n", ph_struct->tag);

	ph_struct->master = meas_initialize_rms_objects("Voltage", ADC_master);
	ph_struct->slave  = meas_initialize_rms_objects("Current", ADC_slave);

	ph_struct->power_energy = meas_initialize_power_and_energy_objects();

	ph_struct->relay_1 = relay_1;
	ph_struct->relay_2 = relay_2;
	ph_struct->status  = LED;

	ph_struct->dbg_mod = dbg_mod;
	printf("\t- Debugger attached\n\n");

	ph_struct->pe_clk_div_counter = 0;
	ph_struct->events_handler = 0;

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

void meas_sample_voltage_and_current(photovoltaic *ptr)
{
	// Obtém a leitura simultânea dos módulos ADC
	uint32_t raw  = HAL_ADCEx_MultiModeGetValue(ptr->master->ADC);

	// Extrai a tensão a partir da leitura simultanea dos módulos ADC (16 bits LSB)
	ptr->master->sample = (raw & LSB_WORD_BIT_MASK) * ADC_GAIN * VOLTAGE_GAIN_A + VOLTAGE_GAIN_B;

	// Extrai a corrente a partir da leitura simultanea dos módulos ADC (16 bits MSB)
	ptr->slave->sample = (raw >> HALF_WORD_LENGTH) * ADC_GAIN * CURRENT_GAIN_A + CURRENT_GAIN_B;

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
	ptr->master->frst_level[ptr->master->frst_level_index++] = ptr->master->sample;

	if (ptr->master->frst_level_index == RMS_FRST_LEVEL_LENGTH)
	{
		ptr->master->scnd_level_value = meas_quadratic_average(ptr->master->frst_level, RMS_FRST_LEVEL_LENGTH);

		ptr->master->scnd_level[ptr->master->scnd_level_index++] = ptr->master->scnd_level_value;

		if (ptr->master->scnd_level_index == RMS_SCND_LEVEL_LENGTH)
		{
			ptr->master->thrd_level_value = meas_quadratic_average(ptr->master->scnd_level, RMS_SCND_LEVEL_LENGTH);

			meas_verify_voltage_triggers(ptr);

			ptr->master->thrd_level[ptr->master->thrd_level_index++] = ptr->master->thrd_level_value;

			if (ptr->master->thrd_level_index == RMS_THRD_LEVEL_LENGTH)
			{
				ptr->master->frth_level[ptr->master->frth_level_index++] = meas_quadratic_average(ptr->master->thrd_level, RMS_THRD_LEVEL_LENGTH);

				if (ptr->master->frth_level_index == RMS_FRTH_LEVEL_LENGTH)
				{
					ptr->master->ffth_level[ptr->master->ffth_level_index++] = meas_quadratic_average(ptr->master->frth_level, RMS_FRTH_LEVEL_LENGTH);

					if (ptr->master->ffth_level_index == RMS_FFTH_LEVEL_LENGTH)
						ptr->master->ffth_level_index = 0;

					ptr->master->frth_level_index = 0;
				}

				ptr->master->thrd_level_index = 0;
			}

			ptr->master->scnd_level_index = 0;
		}

		ptr->master->frst_level_index = 0;
	}
}

void meas_current_aggregation_handler(photovoltaic *ptr)
{
	ptr->slave->frst_level[ptr->slave->frst_level_index++] = ptr->slave->sample;

	if (ptr->slave->frst_level_index == RMS_FRST_LEVEL_LENGTH)
	{
		ptr->slave->scnd_level_value = meas_quadratic_average(ptr->slave->frst_level, RMS_FRST_LEVEL_LENGTH);

		ptr->slave->scnd_level[ptr->slave->scnd_level_index++] = ptr->slave->scnd_level_value;

		if (ptr->slave->scnd_level_index == RMS_SCND_LEVEL_LENGTH)
		{
			ptr->slave->thrd_level_value = meas_quadratic_average(ptr->slave->scnd_level, RMS_SCND_LEVEL_LENGTH);

			meas_verify_current_triggers(ptr);

			ptr->slave->thrd_level[ptr->slave->thrd_level_index++] = ptr->slave->thrd_level_value;

			if (ptr->slave->thrd_level_index == RMS_THRD_LEVEL_LENGTH)
			{
				ptr->slave->frth_level[ptr->slave->frth_level_index++] = meas_quadratic_average(ptr->slave->thrd_level, RMS_THRD_LEVEL_LENGTH);

				if (ptr->slave->frth_level_index == RMS_FRTH_LEVEL_LENGTH)
				{
					ptr->slave->ffth_level[ptr->slave->ffth_level_index++] = meas_quadratic_average(ptr->slave->frth_level, RMS_FRTH_LEVEL_LENGTH);

					if (ptr->slave->ffth_level_index == RMS_FFTH_LEVEL_LENGTH)
						ptr->slave->ffth_level_index = 0;

					ptr->slave->frth_level_index = 0;
				}

				ptr->slave->thrd_level_index = 0;
			}

			ptr->slave->scnd_level_index = 0;
		}

		ptr->slave->frst_level_index = 0;
	}
}

void meas_verify_voltage_triggers(photovoltaic *ptr)
{
	// Verifica se o evento de sobretensão está iniciado, para resetá-lo
	if ((ptr->events_handler & EVENT_OVERVOLTAGE) == EVENT_OVERVOLTAGE &&
			ptr->master->thrd_level_value <= OVERVOLTAGE_RELEASE_LIMIT)
	{
		ptr->events_handler &= ~EVENT_OVERVOLTAGE;
	}
	// Verifica se o evento de subtensão está iniciado, para resetá-lo
	else if ((ptr->events_handler & EVENT_UNDERVOLTAGE) == EVENT_UNDERVOLTAGE &&
			ptr->master->thrd_level_value >= UNDERVOLTAGE_RELEASE_LIMIT)
	{
		ptr->events_handler &= ~EVENT_UNDERVOLTAGE;
	}
	// Verifica se o evento de sobretensão está resetado, para iniciá-lo
	else if ((ptr->events_handler & EVENT_OVERVOLTAGE) != EVENT_OVERVOLTAGE &&
				ptr->master->thrd_level_value >= OVERVOLTAGE_HOLD_LIMIT)
	{
		ptr->events_handler |= EVENT_OVERVOLTAGE;
	}
	// Verifica se o evento de subtensão está resetado, para iniciá-lo
	else if ((ptr->events_handler & EVENT_UNDERVOLTAGE) != EVENT_UNDERVOLTAGE &&
				ptr->master->thrd_level_value <= UNDERVOLTAGE_HOLD_LIMIT)
	{
		ptr->events_handler |= EVENT_UNDERVOLTAGE;
	}
}

void meas_verify_current_triggers(photovoltaic *ptr)
{
	// Verifica se o evento de sobrecorrente está iniciado, para resetá-lo
	if ((ptr->events_handler & EVENT_OVERCURRENT) == EVENT_OVERCURRENT &&
			ptr->slave->thrd_level_value <= OVERCURRENT_RELEASE_LIMIT)
	{
		ptr->events_handler &= ~EVENT_OVERCURRENT;
	}
	// Verifica se o evento de sobrecorrente está resetado, para iniciá-lo
	else if ((ptr->events_handler & EVENT_OVERCURRENT) != EVENT_OVERCURRENT &&
			ptr->slave->thrd_level_value >= OVERCURRENT_HOLD_LIMIT)
	{
		ptr->events_handler |= EVENT_OVERCURRENT;
	}
}

void meas_verify_temperature_triggers(photovoltaic *ptr)
{
	// Verifica se o evento de superaquecimento está iniciado, para resetá-lo
	if ((ptr->events_handler & EVENT_OVERHEAT) == EVENT_OVERHEAT &&
			ptr->temperature <= OVERHEAT_RELEASE_LIMIT)
	{
		ptr->events_handler &= ~EVENT_OVERHEAT;
	}
	// Verifica se o evento de superaquecimento está resetado, para iniciá-lo
	if ((ptr->events_handler & EVENT_OVERHEAT) != EVENT_OVERHEAT &&
			ptr->temperature >= OVERHEAT_HOLD_LIMIT)
	{
		ptr->events_handler |= EVENT_OVERHEAT;
	}
}

void meas_compute_power_and_energy(photovoltaic *ptr)
{
	// Divisor de clock [3 segundos]
	if (!meas_power_energy_interval(ptr))
		return;

	float voltage = ptr->master->thrd_level[ptr->power_energy->frst_level_index];
	float current = ptr->slave->thrd_level[ptr->power_energy->frst_level_index];

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
	ptr->pe_clk_div_counter++;

	if (ptr->pe_clk_div_counter == POWER_ENERGY_INTERVAL)
	{
		ptr->pe_clk_div_counter = 0;

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

void meas_temperature(photovoltaic *ptr)
{
	// Obtém a leitura do módulo ADC, em bits
	uint32_t raw = HAL_ADC_GetValue(&hadc5);

	// Converte a leitura do módulo ADC em tensão
	float voltage = (float)raw * ADC_GAIN;

	// Converte a tensão em temperatura (ºC)
	ptr->temperature = ((voltage - TEMP_SENSOR_VREF) / TEMP_SENSOR_SLOPE) + TEMP_SENSOR_TREF;

	meas_verify_temperature_triggers(ptr);
}
