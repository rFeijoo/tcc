#include "meas.h"

photovoltaic *meas_initialize_objects(char *tag, ADC_HandleTypeDef *ADC_master, ADC_HandleTypeDef *ADC_slave)
{
	photovoltaic *ph_struct = (photovoltaic *)malloc(sizeof(photovoltaic));

	if (ph_struct == NULL)
		return (NULL);

	ph_struct->tag = tag;

	ph_struct->pe_interval_cnt = 0;

	printf("Initializing %s:\n", ph_struct->tag);

	ph_struct->voltage = meas_initialize_rms_objects("Voltage", ADC_master);
	ph_struct->current = meas_initialize_rms_objects("Voltage", ADC_slave);

	ph_struct->power_energy = meas_initialize_power_and_energy_objects();

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

void meas_objects_handler(photovoltaic *ptr, float voltage, float current)
{
	meas_aggregation_handler(ptr->voltage, voltage);
	meas_aggregation_handler(ptr->current, current);

	meas_compute_power_and_energy(ptr);
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
	if (!meas_power_energy_interval(ptr))
		return;

	float voltage = ptr->voltage->thrd_level[ptr->power_energy->frst_level_index];
	float current = ptr->current->thrd_level[ptr->power_energy->frst_level_index];

	float power_3s = (voltage * current) / 1000.0;

	ptr->power_energy->energy += (power_3s / 3600.0);

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
