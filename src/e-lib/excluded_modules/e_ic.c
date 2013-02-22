/*
 * e_ic.c
 *
 *	$Date: 2011-12-15 00:08:57 -0500 (Thu, 15 Dec 2011) $
 *	$Rev: 912 $
 *	$Author: ysapir $
 *
 *  $Copyright$
 *  $License$
 */

#include <signal.h>
#include <machine/epiphany_config.h>
#include "e_regs.h"
#include "e_coreid.h"
#include "e_ic.h"

e_irq_state_t e_gid(void)
{
	return e_gie_restore(E_IRQ_DISABLE);
}

e_irq_state_t e_gie(void)
{
	return e_gie_restore(E_IRQ_ENABLE);
}

e_irq_state_t e_gie_restore(e_irq_state_t mode)
{
	e_irq_state_t prior;

	/* Read the interrupt enable state before modifying it. */
	prior =  (e_sysreg_read(E_STATUS) & 2) ? E_IRQ_DISABLE : E_IRQ_ENABLE;

	/* Now either enable or disable global interrupts depending on the mode argument. */
	switch (mode)
	{
	case E_IRQ_ENABLE:
		asm("gie");
		break;
	case E_IRQ_DISABLE:
		asm("gid");
		break;
	}

	return prior;
}

unsigned e_irq_disable(e_irq_type_t irq)
{
	unsigned previous;

	previous = e_sysreg_read(E_IMASK);
	e_sysreg_write(E_IMASK, previous | (1<<(irq-SIG_RESET)));
	return previous;
}

unsigned e_irq_enable(e_irq_type_t irq)
{
	unsigned previous;

	previous = e_sysreg_read(E_IMASK);
	e_sysreg_write(E_IMASK, previous & (~(1<<(irq-SIG_RESET))));
	return previous;
}

/* unsigned e_get_coreid(); */

int e_irq_raise(e_irq_type_t irq)
{
	return e_irq_remote_raise(e_get_coreid(), irq);
}

int e_irq_remote_raise(unsigned coreid, e_irq_type_t irq)
{
	int *ilatst = (int *) e_address_from_coreid(coreid, (void *) E_ILATST);
	*ilatst = 1 << (irq - SIG_RESET);
	return 0;
}

void e_irq_lower(e_irq_type_t irq)
{
	e_sysreg_write(E_ILATCL, (unsigned)(1<<(irq-SIG_RESET)));
}

void e_irq_restore(unsigned restore)
{
	e_sysreg_write(E_IMASK, restore);
}

