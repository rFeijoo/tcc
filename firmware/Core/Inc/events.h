/**
 * @file events.h
 * @date Agosto 2020
 * @brief Arquivo de definições da estrutura do módulo gerenciador de eventos.
 */


#ifndef _EVENTS_MANAGER_H_
#define _EVENTS_MANAGER_H_

#include <stdint.h>
#include <stdio.h>
#include "meas.h"

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

uint8_t events_toggle_bit(uint8_t bit, uint8_t mask);

#endif /*_EVENTS_MANAGER_H_*/
