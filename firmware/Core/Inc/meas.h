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
#include "events.h"
#include "lcd_16x2.h"

/**
 *	@brief Definições gerais do sistema.
 */
#define SYSTEM_VCC			3.3
#define ADC_RESOLUTION		4095.0
#define BTN_DEBOUNCE_DELAY 	50

/**
 *	@brief Definição dos ganhos dos circuitos de medição em regime permanente.
 */
#define VOLTAGE_GAIN 151.52
#define CURRENT_GAIN 17.857

/**
 *	@brief Definição dos limiares de tensão e corrente.
 */
#define OVERVOLTAGE_HOLD_LIMIT		475.0
#define OVERVOLTAGE_RELEASE_LIMIT	450.0

#define UNDERVOLTAGE_HOLD_LIMIT		25.00
#define UNDERVOLTAGE_RELEASE_LIMIT	50.00

#define OVERCURRENT_HOLD_LIMIT		19.00
#define OVERCURRENT_RELEASE_LIMIT	18.00

#define OVERHEAT_HOLD_LIMIT			75.00
#define OVERHEAT_RELEASE_LIMIT		50.00

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

photovoltaic *meas_initialize_cell(char *tag, ADC_HandleTypeDef *ADC_master, ADC_HandleTypeDef *ADC_slave, digital_IOs *pos_out, digital_IOs *neg_out, digital_IOs *led_out, debug_mod *debug_mod);

rms_measurement *meas_initialize_rms_objects(char *tag, ADC_HandleTypeDef *ADC);

power_and_energy *meas_initialize_power_and_energy_objects(void);

void meas_decouple_system(photovoltaic *ptr);

void meas_sample_voltage_and_current(photovoltaic *ptr);

void meas_temperature(photovoltaic *ptr);

void meas_objects_handler(photovoltaic *ptr);

void meas_verify_voltage_triggers(photovoltaic *ptr);

void meas_verify_current_triggers(photovoltaic *ptr);

void meas_voltage_aggregation_handler(photovoltaic *ptr);

void meas_current_aggregation_handler(photovoltaic *ptr);

void meas_compute_power_and_energy(photovoltaic *ptr);

int meas_power_energy_interval(photovoltaic *ptr);

float meas_quadratic_average(float *ptr, int length);

#endif /*_MEAS_H_*/
