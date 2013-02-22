/*
  File: e_regs_sysreg_write.c

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

#include "e_regs.h"
#include "e_coreid.h"

void e_sysreg_write(e_reg_id_t reg_id, unsigned val)
{
	volatile register unsigned reg_val = val;
	unsigned *addr;

	switch (reg_id)
	{
	case E_CONFIG:
		__asm__ __volatile__ ("MOVTS config,%0" : : "r" (reg_val));
		break;
	case E_STATUS:
		__asm__ __volatile__ ("MOVTS status,%0" : : "r" (reg_val));
		break;
	case E_PC:
		break;
	default:
		addr = (unsigned *) e_address_from_coreid(e_get_coreid(), (void *) reg_id);
		*addr = val;
		break;
	}
}

