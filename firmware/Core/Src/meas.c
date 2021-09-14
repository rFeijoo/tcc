#include "meas.h"

photovoltaic *meas_initialize_objects(char *tag, ADC_HandleTypeDef *ADC_master, ADC_HandleTypeDef *ADC_slave, debug_mod	*debug_mod)
{
	photovoltaic *ph_struct = (photovoltaic *)malloc(sizeof(photovoltaic));

	if (ph_struct == NULL)
		return (NULL);

	ph_struct->tag = tag;

	ph_struct->pe_interval_cnt = 0;

	ph_struct->events_handler = 0;

	printf("Initializing %s:\n", ph_struct->tag);

	ph_struct->voltage = meas_initialize_rms_objects("Voltage", ADC_master, VOLTAGE_UPPER_LIMIT, VOLTAGE_LOWER_LIMIT);
	ph_struct->current = meas_initialize_rms_objects("Current", ADC_slave,  CURRENT_UPPER_LIMIT, CURRENT_LOWER_LIMIT);

	ph_struct->power_energy = meas_initialize_power_and_energy_objects();

	if (debug_mod == NULL)
		ph_struct->debugger = NULL;
	else
		ph_struct->debugger = debug_mod;

	return(ph_struct);
}

rms_measurement *meas_initialize_rms_objects(char *tag, ADC_HandleTypeDef *ADC, uint16_t high_trigger, uint16_t low_trigger)
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

	rms_struct->high_trigger = high_trigger;
	rms_struct->low_trigger  = low_trigger;

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

void meas_decouple_system(photovoltaic *ptr)
{
	if ((ptr->events_handler & 0x01) == 1)
		ptr->events_handler &= ~0x01;
	else
		ptr->events_handler |= 0x01;
}

void meas_temperature(photovoltaic *ptr)
{
	// Obtém a leitura do módulo ADC, em bits
	uint32_t raw = HAL_ADC_GetValue(&hadc5);

	// Converte a leitura do módulo ADC em tensão
	float voltage = (float)raw * (SYSTEM_VCC / ADC_RESOLUTION);

	// Converte a tensão em temperatura (ºC)
	ptr->temperature = ((voltage - TEMP_SENSOR_VREF) / TEMP_SENSOR_SLOPE) + TEMP_SENSOR_TREF;

	// Verifica se há superaquecimento
	if (ptr->temperature >= TEMPERATURE_LIMIT)
		ptr->events_handler |= 0x10;
	else
		ptr->events_handler &= ~0x10;
}

void meas_sample(photovoltaic *ptr)
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
	// Verifica eventos na medição de tensão (sobretensão, subtensão)
	meas_verify_voltage_triggers(ptr);

	// Gerencia o protocolo de agregação em multi-camadas para medição de tensão
	meas_aggregation_handler(ptr->voltage, ptr->voltage->sample);

	// Verifica eventos na medição de corrente (sobrecorrente)
	meas_verify_current_triggers(ptr);

	// Gerencia o protocolo de agregação em multi-camadas para medição de corrente
	meas_aggregation_handler(ptr->current, ptr->current->sample);

	// Gerencia o protocolo de medição de potência e energia produzidas
	meas_compute_power_and_energy(ptr);
}

void meas_verify_voltage_triggers(photovoltaic *ptr)
{
	// Reseta o(s) bit(s) referente(s) a medição de tensão
	ptr->events_handler &= 0xF5;

	// Identifica eventos de sobretensão
	if (ptr->voltage->sample >= VOLTAGE_UPPER_LIMIT)
	{
		ptr->events_handler |= 0x02;
	}
	// Identifica eventos de subtensão
	else if (ptr->voltage->sample <= VOLTAGE_LOWER_LIMIT)
	{
		ptr->events_handler |= 0x08;
	}
}

void meas_verify_current_triggers(photovoltaic *ptr)
{
	// Reseta o(s) bit(s) referente(s) a medição de corrente
	ptr->events_handler &= 0xFB;

	// Identifica eventos de sobrecorrente
	if (ptr->current->sample >= CURRENT_UPPER_LIMIT)
	{
		ptr->events_handler |= 0x04;
	}
}

void meas_aggregation_handler(rms_measurement *ptr, float value)
{
	ptr->frst_level[ptr->frst_level_index++] = value;

	if (ptr->frst_level_index == RMS_FRST_LEVEL_LENGTH)
	{
		ptr->scnd_level[ptr->scnd_level_index++] = meas_quadratic_average(ptr->frst_level, RMS_FRST_LEVEL_LENGTH);

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
