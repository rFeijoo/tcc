/**
 * @file structs.h
 * @date Agosto 2021
 * @brief Arquivo de definição das estruturas de dados do sistema.
 */

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <stdint.h>
#include <stdbool.h>
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

	digital_IOs *probe_1;
	digital_IOs *probe_2;
	digital_IOs *probe_3;
} debug_mod;

/**
 *	@brief Definição da estrutura de medição para tensão e corrente.
 */
typedef struct {
	ADC_HandleTypeDef *ADC;

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
 *	@brief Definição da estrutura de medição de potência.
 */
typedef struct {
	float frst_level[RMS_THRD_LEVEL_LENGTH];
	float scnd_level[RMS_FRTH_LEVEL_LENGTH];
	float thrd_level[RMS_FFTH_LEVEL_LENGTH];

	uint8_t frst_level_index;
	uint8_t scnd_level_index;
	uint8_t thrd_level_index;
} power_measurement;

/**
 *	@brief Definição da estrutura do sistema fotovoltaico.
 */
typedef struct {
	char *tag;

	bool new_sample;

	rms_measurement *master, *slave;
	power_measurement *power_energy;

	digital_IOs *relay_1, *relay_2, *status;

	debug_mod *dbg_mod;

	float voltage, current, power, energy;
	float temperature;

	uint8_t events_handler;
} photovoltaic;

#endif /*_STRUCTS_H_*/
