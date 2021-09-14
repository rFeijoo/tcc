#include "events.h"

debug_mod *events_initialize_debug_mod(char *tag, GPIO_TypeDef *Port, uint16_t LED1, uint16_t LED2, uint16_t LED3)
{
	debug_mod *dbg_struct = (debug_mod *)malloc(sizeof(debug_mod));

	if (dbg_struct == NULL)
		return (NULL);

	dbg_struct->tag = tag;

	dbg_struct->Port = Port;

	dbg_struct->LED1 = LED1;
	dbg_struct->LED2 = LED2;
	dbg_struct->LED3 = LED3;

	printf("Initializing %s:\n", dbg_struct->tag);

	return(dbg_struct);
}

void events_handler(photovoltaic *ptr)
{
	if (ptr->debugger != NULL)
		events_set_debugger(ptr);
}

void events_set_debugger(photovoltaic *ptr)
{
	uint8_t output = 0x00;

	// Seccionamento do circuito
	if ((ptr->events_handler & 0x01) == 0x01)
		output = 0x1;
	// Sobretensão
	else if ((ptr->events_handler & 0x02) == 0x02)
		output = 0x2;
	// Sobrecorrente
	else if ((ptr->events_handler & 0x04) == 0x04)
		output = 0x3;
	// Subtensão
	else if ((ptr->events_handler & 0x08) == 0x08)
		output = 0x4;
	// Superaquecimento
	else if ((ptr->events_handler & 0x10) == 0x10)
		output = 0x5;
	// Estado de degradação do painel solar
	else if ((ptr->events_handler & 0x20) == 0x20)
		output = 0x6;
	// Fim de vida util do DPS
	else if ((ptr->events_handler & 0x40) == 0x40)
		output = 0x7;

	events_output_debugger(ptr, output);

}

void events_output_debugger(photovoltaic *ptr, uint8_t value)
{
	// LSB
	HAL_GPIO_WritePin(ptr->debugger->Port, ptr->debugger->LED1, 0x1 & value);

	HAL_GPIO_WritePin(ptr->debugger->Port, ptr->debugger->LED2, 0x2 & value);

	// MSB
	HAL_GPIO_WritePin(ptr->debugger->Port, ptr->debugger->LED3, 0x4 & value);
}
