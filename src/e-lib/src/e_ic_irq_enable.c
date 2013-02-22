/*
  File: e_ic_irq_enable.c

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

#include <machine/epiphany_config.h>
#include "e_regs.h"
#include "e_ic.h"

unsigned e_irq_enable(e_irq_type_t irq)
{
	unsigned previous;

	previous = e_sysreg_read(E_IMASK);
	e_sysreg_write(E_IMASK, previous & (~(1<<(irq-SIG_RESET))));
	return previous;
}
