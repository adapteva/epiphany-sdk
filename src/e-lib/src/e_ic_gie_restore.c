/*
  File: e_ic_restore.c

  This file is part of the Epiphany Software Development Kit.

  Copyright (C) 2013 Adapteva, Inc.
  Contributed by Oleg Raikhman, Jim Thomas, Yaniv Sapir <support@adapteva.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License (LGPL)
  as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  and the GNU Lesser General Public License along with this program,
  see the files COPYING and COPYING.LESSER.  If not, see
  <http://www.gnu.org/licenses/>.
*/

#include <signal.h>
#include <machine/epiphany_config.h>
#include "e_regs.h"
#include "e_ic.h"

e_irq_state_t e_gie_restore(e_irq_state_t mode)
{
	e_irq_state_t prior;

	/* Read the interrupt enable state before modifying it. */
	prior = (e_sysreg_read(E_STATUS) & 2) ? E_IRQ_DISABLE : E_IRQ_ENABLE;

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
