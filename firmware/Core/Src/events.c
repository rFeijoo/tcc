#include "events.h"

uint8_t events_toggle_bit(uint8_t bit, uint8_t mask)
{
	if ((bit & mask) == 1)
		bit &= ~mask;
	else
		bit |= mask;

	return (bit);
}
