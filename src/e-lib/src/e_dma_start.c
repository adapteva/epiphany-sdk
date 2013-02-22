/*
  File: e_dma_start.c

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

#include "e_ic.h"
#include "e_regs.h"
#include "e_dma.h"

int e_dma_start(e_dma_id_t chan, e_tcb_t *tcb)
{
	unsigned      start;
	e_irq_state_t irq_state;
	int           ret_val = -1;

	if ((chan | 1) != 1)
	{
		return -1;
	}

	irq_state = e_gid();

	/* wait for the DMA engine to be idle */
	while (e_dma_busy(chan));

	start = ((volatile int)(tcb) << 16) | E_DMA_STARTUP;
	switch (chan)
	{
	case 0:
		e_sysreg_write(E_DMA0CONFIG, start);
		ret_val = 0;
		break;
	case 1:
		e_sysreg_write(E_DMA1CONFIG, start);
		ret_val = 0;
		break;
	}

	e_gie_restore(irq_state);
	return ret_val;
}

