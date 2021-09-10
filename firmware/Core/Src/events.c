#include "events.h"

uint8_t events_toggle_bit(uint8_t bit, uint8_t mask)
{
	if ((bit & mask) == 1)
		bit &= 0xFE;
	else
		bit |= 0x01;

	return (bit);
}
