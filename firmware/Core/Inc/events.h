/**
 * @file events.h
 * @date Agosto 2020
 * @brief Arquivo de definições da estrutura do módulo gerenciador de eventos.
 */

#ifndef _EVENTS_MANAGER_H_
#define _EVENTS_MANAGER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "gpio.h"
#include "structs.h"

/**
 *	@brief Módulo gerenciador de eventos.
 *
 *	Handler Type: uint8_t [8 7 6 5 4 3 2 1]
 *
 *	0: nenhum evento
 *	1: seccionamento
 *	2: sobretensão
 *	3: sobrecorrente
 *	4: subtensão
 *	5: superaquecimento
 *	6: fim de vida útil do DPS
 *	7: degradação do painel solar
 */
#define NO_EVENT			0x00
#define EVENT_USER_BREAK	0x01
#define EVENT_OVERVOLTAGE	0x02
#define EVENT_OVERCURRENT	0x04
#define EVENT_UNDERVOLTAGE	0x08
#define EVENT_OVERHEAT		0x10
#define EVENT_DPS_LIFESPAN	0x20

#define OUTPUT_HIGH	0x1
#define OUTPUT_LOW	0x0

debug_mod *events_initialize_debug_mod(char *tag, GPIO_TypeDef *Port, uint16_t Probe_1, uint16_t Probe_2, uint16_t Probe_3);

digital_IOs *events_initialize_digital_ios(char *tag, GPIO_TypeDef *Port, uint16_t Pin, uint8_t invert_output);

void events_blink_debug_module(debug_mod *dbg);

void events_change_state(photovoltaic *ptr, uint8_t event);

bool is_event_active(uint8_t handler, uint8_t event_flag);

void events_handler(photovoltaic *ptr);

void events_output_relays(photovoltaic *ptr, int state);

void events_output_debugger(photovoltaic *ptr, int state);

#endif /*_EVENTS_MANAGER_H_*/
