/**
 * @file events.h
 * @date Agosto 2020
 * @brief Arquivo de definições da estrutura do módulo gerenciador de eventos.
 */


#ifndef _EVENTS_MANAGER_H_
#define _EVENTS_MANAGER_H_

#include <stdint.h>
#include <stdio.h>
#include "gpio.h"
#include "structs.h"

/**
 *	@brief Módulo gerenciador de eventos.
 *
 *	Handler Type: uint8_t [8 7 6 5 4 3 2 1]
 *
 *	1: seccionamento
 *	2: sobretensão
 *	3: sobrecorrente
 *	4: subtensão
 *	5: superaquecimento
 *	6: estado de degradação do painel solar
 *	7: fim de vida útil do DPS
 *	8: reserva
 */

debug_mod *events_initialize_debug_mod(char *tag, GPIO_TypeDef *Port, uint16_t LED1, uint16_t LED2, uint16_t LED3);

void events_handler(photovoltaic *ptr);

void events_set_debugger(photovoltaic *ptr);

void events_output_debugger(photovoltaic *ptr, uint8_t value);

#endif /*_EVENTS_MANAGER_H_*/
