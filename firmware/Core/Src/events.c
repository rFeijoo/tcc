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

void events_handler(photovoltaic *ptr)
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

	if (ptr->debugger != NULL)
		events_output_debugger(ptr, output);
}

void events_output_debugger(photovoltaic *ptr, uint8_t value)
{
	// LSB
	HAL_GPIO_WritePin(ptr->debugger->probe_1->Port, ptr->debugger->probe_1->Pin, 0x1 & value);

	HAL_GPIO_WritePin(ptr->debugger->probe_2->Port, ptr->debugger->probe_2->Pin, 0x2 & value);

	// MSB
	HAL_GPIO_WritePin(ptr->debugger->probe_3->Port, ptr->debugger->probe_3->Pin, 0x4 & value);
}
