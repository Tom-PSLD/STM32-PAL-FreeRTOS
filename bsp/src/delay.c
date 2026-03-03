#include "delay.h"



void BSP_DELAY_ms(uint32_t delay)
{
	uint32_t	i;
	for(i=0; i<(delay*2500); i++);		// Tuned for ms at 48MHz
}

