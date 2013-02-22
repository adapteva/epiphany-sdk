/**
 * File: e_sync.c
 *
 * Desc:  Software to synchronize multiple cores on a chip.
 *
**/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <machine/epiphany_config.h>
#include "e_ic.h"

volatile static int wand_asserted = 0;
void e_assert_sync(int sig_num)
{
	wand_asserted = 1;
}

int e_sync_cores(void)
{
	// save the current gie state and disable interrupts.
	e_irq_state_t irq_state = e_gid();

	wand_asserted = 0;
	asm("wand");
	do
	{
		asm("gie");
		asm("idle");
		asm("gid");
	} while (!wand_asserted);
	wand_asserted = 0;
	e_gie_restore(irq_state);
	return 0;
}

