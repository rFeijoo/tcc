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

/**
 * @brief Comprimento de vetores para armazenamento de dados, considerando os diferentes níveis de
 * agregação de dados em regime permanente.
*/
#define RMS_SAMPLES_PER_CYCLE	16
#define RMS_FRST_LEVEL_CYCLES	12
#define RMS_FRST_LEVEL_LENGTH	(RMS_SAMPLES_PER_CYCLE * RMS_FRST_LEVEL_CYCLES)
#define RMS_SCND_LEVEL_LENGTH	15
#define RMS_THRD_LEVEL_LENGTH	200
#define RMS_FRTH_LEVEL_LENGTH	6
#define RMS_FFTH_LEVEL_LENGTH	24
#define POWER_ENERGY_INTERVAL	(RMS_FRST_LEVEL_LENGTH * RMS_SCND_LEVEL_LENGTH)

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

/**
 *	@brief Definição da estrutura de medição para tensão e corrente.
 */
typedef struct {
	ADC_HandleTypeDef *ADC;

	float sample;

	float frst_level[RMS_FRST_LEVEL_LENGTH];
	float scnd_level[RMS_SCND_LEVEL_LENGTH];
	float thrd_level[RMS_THRD_LEVEL_LENGTH];
	float frth_level[RMS_FRTH_LEVEL_LENGTH];
	float ffth_level[RMS_FFTH_LEVEL_LENGTH];

	uint8_t frst_level_index;
	uint8_t scnd_level_index;
	uint8_t thrd_level_index;
	uint8_t frth_level_index;
	uint8_t ffth_level_index;

	uint8_t high_trigger, low_trigger;
} rms_measurement;

/**
 *	@brief Definição da estrutura de medição de potência e energia.
 */
typedef struct {
	float energy;

	float frst_level[RMS_THRD_LEVEL_LENGTH];
	float scnd_level[RMS_FRTH_LEVEL_LENGTH];
	float thrd_level[RMS_FFTH_LEVEL_LENGTH];

	uint8_t frst_level_index;
	uint8_t scnd_level_index;
	uint8_t thrd_level_index;
} power_and_energy;

/**
 *	@brief Definição da estrutura do sistema fotovoltaico.
 */
typedef struct {
	char *tag;

	rms_measurement *voltage;
	rms_measurement *current;

	power_and_energy *power_energy;

	uint16_t pe_interval_cnt;

	uint8_t events_handler;
} photovoltaic;

photovoltaic *meas_initialize_objects(char *tag, ADC_HandleTypeDef *ADC_master, ADC_HandleTypeDef *ADC_slave);

rms_measurement *meas_initialize_rms_objects(char *tag, ADC_HandleTypeDef *ADC, uint16_t high_trigger, uint16_t low_trigger);

power_and_energy *meas_initialize_power_and_energy_objects(void);

void meas_sample(photovoltaic *ptr);

void meas_objects_handler(photovoltaic *ptr);

void meas_verify_voltage_triggers(photovoltaic *ptr);

void meas_verify_current_triggers(photovoltaic *ptr);

void meas_aggregation_handler(rms_measurement *ptr, float value);

void meas_compute_power_and_energy(photovoltaic *ptr);

int meas_power_energy_interval(photovoltaic *ptr);

float meas_quadratic_average(float *ptr, int length);

#endif /*_MEAS_H_*/
