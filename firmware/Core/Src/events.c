#include "events.h"

debug_mod *events_initialize_debug_mod(char *tag, GPIO_TypeDef *Port, uint16_t Probe_1, uint16_t Probe_2, uint16_t Probe_3)
{
	debug_mod *dbg_struct = (debug_mod *)malloc(sizeof(debug_mod));

	if (dbg_struct == NULL)
		return (NULL);

	dbg_struct->tag = tag;

	printf("Initializing %s:\n", dbg_struct->tag);

	printf("\t- ");
	dbg_struct->probe_1 = events_initialize_digital_ios("probe 1", Port, Probe_1, 0);

	printf("\t- ");
	dbg_struct->probe_2 = events_initialize_digital_ios("probe 2", Port, Probe_2, 0);

	printf("\t- ");
	dbg_struct->probe_3 = events_initialize_digital_ios("probe 3", Port, Probe_3, 0);

	events_blink_debug_module(dbg_struct);

	printf("\n");

	return(dbg_struct);
}

digital_IOs *events_initialize_digital_ios(char *tag, GPIO_TypeDef *Port, uint16_t Pin, uint8_t invert_output)
{
	digital_IOs *dig_ios_struct = (digital_IOs *)malloc(sizeof(digital_IOs));

	if (dig_ios_struct == NULL)
		return (NULL);

	dig_ios_struct->tag = tag;

	dig_ios_struct->Port = Port;

	dig_ios_struct->Pin = Pin;

	dig_ios_struct->invert = invert_output;

	printf("Initializing %s\n", dig_ios_struct->tag);

	return (dig_ios_struct);
}

void events_blink_debug_module(debug_mod *dbg)
{
	HAL_GPIO_TogglePin(dbg->probe_1->Port, dbg->probe_1->Pin);
	HAL_GPIO_TogglePin(dbg->probe_2->Port, dbg->probe_2->Pin);
	HAL_GPIO_TogglePin(dbg->probe_3->Port, dbg->probe_3->Pin);

	HAL_Delay(2500);

	HAL_GPIO_TogglePin(dbg->probe_1->Port, dbg->probe_1->Pin);
	HAL_GPIO_TogglePin(dbg->probe_2->Port, dbg->probe_2->Pin);
	HAL_GPIO_TogglePin(dbg->probe_3->Port, dbg->probe_3->Pin);
}

void events_change_state(photovoltaic *ptr, uint8_t event)
{
	if (is_event_active(ptr->events_handler, event))
		ptr->events_handler &= ~event;
	else
		ptr->events_handler |= event;
}

bool is_event_active(uint8_t handler, uint8_t event_flag)
{
	if ((handler & event_flag) == event_flag)
		return (true);

	return (false);
}

void events_handler(photovoltaic *ptr)
{
	// Seccionamento do circuito
	if (is_event_active(ptr->events_handler, EVENT_USER_BREAK)) {
		events_output_relays(ptr, OUTPUT_LOW);
		events_output_debugger(ptr, 0x1);
	}
	// Sobretensão
	else if (is_event_active(ptr->events_handler, EVENT_OVERVOLTAGE)) {
		events_output_relays(ptr, OUTPUT_LOW);
		events_output_debugger(ptr, 0x2);
	}
	// Sobrecorrente
	else if (is_event_active(ptr->events_handler, EVENT_OVERCURRENT)) {
		events_output_relays(ptr, OUTPUT_LOW);
		events_output_debugger(ptr, 0x3);
	}
	// Subtensão
	else if (is_event_active(ptr->events_handler, EVENT_UNDERVOLTAGE)) {
		events_output_relays(ptr, OUTPUT_LOW);
		events_output_debugger(ptr, 0x4);
	}
	// Superaquecimento
	else if (is_event_active(ptr->events_handler, EVENT_OVERHEAT)) {
		events_output_relays(ptr, OUTPUT_HIGH);
		events_output_debugger(ptr, 0x5);
	}
	// Fim de vida util do DPS
	else if (is_event_active(ptr->events_handler, EVENT_DPS_LIFESPAN)) {
		events_output_relays(ptr, OUTPUT_LOW);
		events_output_debugger(ptr, 0x6);
	}
	// Nenhum evento identificado
	else {
		events_output_relays(ptr, OUTPUT_HIGH);
		events_output_debugger(ptr, NO_EVENT);
	}
}

void events_output_relays(photovoltaic *ptr, int state)
{
	// Relé 1
	if (ptr->relay_1->invert)
		HAL_GPIO_WritePin(ptr->relay_1->Port, ptr->relay_1->Pin, ~state);
	else
		HAL_GPIO_WritePin(ptr->relay_1->Port, ptr->relay_1->Pin, state);

	// Relé 2
	if (ptr->relay_2->invert)
		HAL_GPIO_WritePin(ptr->relay_2->Port, ptr->relay_2->Pin, ~state);
	else
		HAL_GPIO_WritePin(ptr->relay_2->Port, ptr->relay_2->Pin, state);

	// Status da saída
	if (ptr->status->invert)
		HAL_GPIO_WritePin(ptr->status->Port, ptr->status->Pin, ~state);
	else
		HAL_GPIO_WritePin(ptr->status->Port, ptr->status->Pin, state);

}

void events_output_debugger(photovoltaic *ptr, int state)
{
	HAL_GPIO_WritePin(ptr->dbg_mod->probe_3->Port, ptr->dbg_mod->probe_3->Pin, 0x1 & state);

	HAL_GPIO_WritePin(ptr->dbg_mod->probe_2->Port, ptr->dbg_mod->probe_2->Pin, 0x2 & state);

	HAL_GPIO_WritePin(ptr->dbg_mod->probe_1->Port, ptr->dbg_mod->probe_1->Pin, 0x4 & state);
}
