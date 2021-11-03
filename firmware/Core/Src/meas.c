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

	ph_struct->new_sample = false;
	ph_struct->events_handler = 0;

	ph_struct->voltage = 0.0;
	ph_struct->current = 0.0;
	ph_struct->power   = 0.0;
	ph_struct->energy  = 0.0;

	ph_struct->temperature = TEMP_SENSOR_TREF;

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

power_measurement *meas_initialize_power_and_energy_objects(void)
{
	power_measurement *pe_struct = (power_measurement *)malloc(sizeof(power_measurement));

	if (pe_struct == NULL)
		return (NULL);

	pe_struct->frst_level_index = 0;
	pe_struct->scnd_level_index = 0;
	pe_struct->thrd_level_index = 0;

	printf("\t- Power & Energy measurement initialized\n");

	return (pe_struct);
}

void meas_sample_voltage_and_current(photovoltaic *ptr)
{
	// Obtém a leitura simultânea dos módulos ADC master e slave
	uint32_t raw  = HAL_ADCEx_MultiModeGetValue(ptr->master->ADC);

	// Extrai os dados do módulo ADC master (16 bits LSB), em Volts
	ptr->master->frst_level[ptr->master->frst_level_index++] = (float)(raw & LSB_WORD_BIT_MASK) * ADC_GAIN;

	// Extrai os dados do módulo ADC slave  (16 bits MSB), em Volts
	ptr->slave->frst_level[ptr->slave->frst_level_index++]   = (float)(raw >> HALF_WORD_LENGTH) * ADC_GAIN;

	ptr->new_sample = true;
}

void meas_objects_handler(photovoltaic *ptr, float temperature)
{
	if (ptr->new_sample)
	{
		int thrd_index = ptr->master->thrd_level_index;

		meas_aggregation_handler(ptr->master, VOLTAGE_GAIN_A, VOLTAGE_GAIN_B);
		meas_aggregation_handler(ptr->slave,  CURRENT_GAIN_A, CURRENT_GAIN_B);

		// Verifica se uma nova posição do 3º vetor foi registrada para cálculo da potência
		if (thrd_index != ptr->master->thrd_level_index)
		{
			meas_compute_power_and_energy(ptr, thrd_index);

			meas_verify_voltage_events(&ptr->events_handler, ptr->master->thrd_level[thrd_index]);
			meas_verify_current_events(&ptr->events_handler, ptr->slave->thrd_level[thrd_index]);

			meas_verify_temperature_events(&ptr->events_handler, temperature);

			meas_update_cell_values(ptr, thrd_index, temperature);
		}

		events_handler(ptr);

		ptr->new_sample = false;
	}
}

void meas_aggregation_handler(rms_measurement *ptr, float gain_a, float gain_b)
{
	if (ptr->frst_level_index == RMS_FRST_LEVEL_LENGTH)
	{
		ptr->scnd_level[ptr->scnd_level_index++] = meas_quadratic_average(ptr->frst_level, RMS_FRST_LEVEL_LENGTH) * gain_a + gain_b;

		if (ptr->scnd_level_index == RMS_SCND_LEVEL_LENGTH)
		{
			ptr->thrd_level[ptr->thrd_level_index++] = meas_quadratic_average(ptr->scnd_level, RMS_SCND_LEVEL_LENGTH);

			if (ptr->thrd_level_index == RMS_THRD_LEVEL_LENGTH)
			{
				ptr->frth_level[ptr->frth_level_index++] = meas_quadratic_average(ptr->thrd_level, RMS_THRD_LEVEL_LENGTH);

				if (ptr->frth_level_index == RMS_FRTH_LEVEL_LENGTH)
				{
					ptr->ffth_level[ptr->ffth_level_index++] = meas_quadratic_average(ptr->frth_level, RMS_FRTH_LEVEL_LENGTH);

					if (ptr->ffth_level_index == RMS_FFTH_LEVEL_LENGTH)
						ptr->ffth_level_index = 0;

					ptr->frth_level_index = 0;
				}

				ptr->thrd_level_index = 0;
			}

			ptr->scnd_level_index = 0;
		}
		ptr->frst_level_index = 0;
	}
}

float meas_quadratic_average(float *ptr, int length)
{
    float rms = 0.00;

    for (int i=0; i<length; i++)
    	rms += ptr[i] * ptr[i];

    return (sqrt(rms / length));
}

void meas_verify_voltage_events(uint8_t *handler, float voltage)
{
	// Verifica se o evento de sobretensão está iniciado, para resetá-lo
	if (is_event_active(*handler, EVENT_OVERVOLTAGE) && voltage <= OVERVOLTAGE_RELEASE_LIMIT)
		*handler &= ~EVENT_OVERVOLTAGE;
	// Verifica se o evento de subtensão está iniciado, para resetá-lo
	else if (is_event_active(*handler, EVENT_UNDERVOLTAGE) && voltage >= UNDERVOLTAGE_RELEASE_LIMIT)
		*handler &= ~EVENT_UNDERVOLTAGE;
	// Verifica se o evento de sobretenão está resetado, para iniciá-lo
	else if (!is_event_active(*handler, EVENT_OVERVOLTAGE) && voltage >= OVERVOLTAGE_HOLD_LIMIT)
		*handler |= EVENT_OVERVOLTAGE;
	// Verifica se o evento de subtensão está resetado, para iniciá-lo
	else if (!is_event_active(*handler, EVENT_UNDERVOLTAGE) && voltage <= UNDERVOLTAGE_HOLD_LIMIT)
		*handler |= EVENT_UNDERVOLTAGE;
}

void meas_verify_current_events(uint8_t *handler, float current)
{
	// Verifica se o evento de sobrecorrente está iniciado, para resetá-lo
	if (is_event_active(*handler, EVENT_OVERCURRENT) && current <= OVERCURRENT_RELEASE_LIMIT)
		*handler &= ~EVENT_OVERCURRENT;
	// Verifica se o evento de sobrecorrente está resetado, para iniciá-lo
	else if (!is_event_active(*handler, EVENT_OVERCURRENT) && current >= OVERCURRENT_HOLD_LIMIT)
		*handler |= EVENT_OVERCURRENT;
}

void meas_compute_power_and_energy(photovoltaic *ptr, int index)
{
	float voltage = ptr->master->thrd_level[index];
	float current = ptr->slave->thrd_level[index];

	// Protocolo de agregação em multi-camadas da potência produzida
	ptr->power_energy->frst_level[ptr->power_energy->frst_level_index++] = (voltage * current) / 1000.0;

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

void meas_update_cell_values(photovoltaic *ptr, int index, float temperature)
{
	ptr->voltage = ptr->master->thrd_level[index];
	ptr->current = ptr->slave->thrd_level[index];

	ptr->power   = ptr->power_energy->frst_level[index];
	ptr->energy += ptr->power * POWER_ENERGY_DT;

	ptr->temperature = temperature;
}

float meas_get_temperature(void)
{
	// Obtém a leitura do módulo ADC
	uint32_t raw = HAL_ADC_GetValue(&hadc5);

	// Converte a leitura do módulo ADC em tensão
	float voltage = (float)raw * ADC_GAIN;

	// Converte a tensão lida em temperatura (ºC)
	return (((voltage - TEMP_SENSOR_VREF) / TEMP_SENSOR_SLOPE) + TEMP_SENSOR_TREF);
}

void meas_verify_temperature_events(uint8_t *handler, float temperature)
{
	// Verifica se o evento de superaquecimento está iniciado, para resetá-lo
	if (is_event_active(*handler, EVENT_OVERHEAT) && temperature <= OVERHEAT_RELEASE_LIMIT)
		*handler &= ~EVENT_OVERHEAT;
	else if (!is_event_active(*handler, EVENT_OVERHEAT) && temperature >= OVERHEAT_HOLD_LIMIT)
		*handler |= EVENT_OVERHEAT;
}
