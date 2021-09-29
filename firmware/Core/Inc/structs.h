/**
 * @file structs.h
 * @date Agosto 2021
 * @brief Arquivo de definição das estruturas de dados do sistema.
 */

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <stdint.h>
#include "adc.h"
#include "main.h"

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

/**
 *	@brief Definição da estrutura de GPIOs.
 */
typedef struct {
	char *tag;

	GPIO_TypeDef *Port;
	uint16_t Pin;

	uint8_t invert;
} digital_IOs;

/**
 *	@brief Definição da estrutura do módulo DEBUG.
 */
typedef struct {
	char *tag;

	digital_IOs *probe_1, *probe_2, *probe_3;
} debug_mod;

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

	float temperature;

	power_and_energy *power_energy;

	digital_IOs *pos_out, *neg_out, *led_out;

	debug_mod *debugger;

	uint16_t pe_interval_cnt;
	uint8_t events_handler;
} photovoltaic;

#endif /*_STRUCTS_H_*/
