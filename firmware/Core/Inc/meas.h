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

/**
 *	@brief Definições gerais do sistema.
 */
#define SYSTEM_VCC			3.3
#define ADC_RESOLUTION		4095.0
#define ADC_GAIN			(SYSTEM_VCC / ADC_RESOLUTION)
#define BTN_DEBOUNCE_DELAY 	50

/**
 *	@brief Definição dos ganhos dos circuitos de medição em regime permanente.
 */
#define VOLTAGE_GAIN_A 164.10
#define VOLTAGE_GAIN_B 2.0356
#define CURRENT_GAIN_A 6.9016
#define CURRENT_GAIN_B 0.0174

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
 *	@brief Definição do diferencial de tempo para cálculo da energia.
 */
#define POWER_ENERGY_INTERVAL_S	3.0
#define SECONDS_IN_ONE_HOUR		3600.0
#define POWER_ENERGY_DT			(POWER_ENERGY_INTERVAL_S / SECONDS_IN_ONE_HOUR)

/**
 *	@brief Definições do sensor de temperatura integrado.
 */
#define TEMP_SENSOR_TREF	30.000
#define TEMP_SENSOR_VREF	0.7600
#define TEMP_SENSOR_SLOPE	0.0025

photovoltaic *meas_initialize_cell(char *tag, ADC_HandleTypeDef *ADC_master, ADC_HandleTypeDef *ADC_slave, digital_IOs *relay_1, digital_IOs *relay_2, digital_IOs *LED, debug_mod *dbg_mod);

rms_measurement *meas_initialize_rms_objects(char *tag, ADC_HandleTypeDef *ADC);

power_measurement *meas_initialize_power_and_energy_objects(void);

void meas_sample_voltage_and_current(photovoltaic *ptr);

void meas_objects_handler(photovoltaic *ptr, float temperature);

void meas_aggregation_handler(rms_measurement *ptr, float gain_a, float gain_b);

float meas_quadratic_average(float *ptr, int length);

void meas_verify_voltage_events(uint8_t *handler, float voltage);

void meas_verify_current_events(uint8_t *handler, float current);

void meas_compute_power_and_energy(photovoltaic *ptr, int index);

void meas_update_cell_values(photovoltaic *ptr, int index, float temperature);

float meas_get_temperature(void);

void meas_verify_temperature_events(uint8_t *handler, float temperature);

#endif /*_MEAS_H_*/
