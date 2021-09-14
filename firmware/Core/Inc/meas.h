/**
 * @file meas.h
 * @date Agosto 2021
 * @brief Arquivo de definição das estruturas de dados e funções para medição em regime permanente.
 */

#ifndef _MEAS_H_
#define _MEAS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "adc.h"
#include "tim.h"
#include "structs.h"

/**
 *	@brief Definições gerais do sistema.
 */
#define SYSTEM_VCC		3.3
#define ADC_RESOLUTION	4095.0

/**
 *	@brief Definição dos ganhos dos circuitos de medição em regime permanente.
 */
#define VOLTAGE_GAIN 151.52
#define CURRENT_GAIN 17.857

/**
 *	@brief Definição dos limiares de tensão e corrente.
 */
#define VOLTAGE_UPPER_LIMIT 475.0
#define VOLTAGE_LOWER_LIMIT	25.00
#define CURRENT_UPPER_LIMIT	19.00
#define CURRENT_LOWER_LIMIT	0.000
#define TEMPERATURE_LIMIT	70.00

/**
 *	@brief Definição do divisor de clock para computar potência e energia.
 */
#define POWER_ENERGY_INTERVAL	(RMS_FRST_LEVEL_LENGTH * RMS_SCND_LEVEL_LENGTH)

/**
 *	@brief Definições do sensor de temperatura integrado.
 */
#define TEMP_SENSOR_TREF	30.000
#define TEMP_SENSOR_VREF	0.7600
#define TEMP_SENSOR_SLOPE	0.0025

photovoltaic *meas_initialize_objects(char *tag, ADC_HandleTypeDef *ADC_master, ADC_HandleTypeDef *ADC_slave, debug_mod	*debug_mod);

rms_measurement *meas_initialize_rms_objects(char *tag, ADC_HandleTypeDef *ADC, uint16_t high_trigger, uint16_t low_trigger);

power_and_energy *meas_initialize_power_and_energy_objects(void);

void meas_decouple_system(photovoltaic *ptr);

void meas_sample(photovoltaic *ptr);

void meas_temperature(photovoltaic *ptr);

void meas_objects_handler(photovoltaic *ptr);

void meas_verify_voltage_triggers(photovoltaic *ptr);

void meas_verify_current_triggers(photovoltaic *ptr);

void meas_aggregation_handler(rms_measurement *ptr, float value);

void meas_compute_power_and_energy(photovoltaic *ptr);

int meas_power_energy_interval(photovoltaic *ptr);

float meas_quadratic_average(float *ptr, int length);

#endif /*_MEAS_H_*/
