/*
  File: e-loader.h

  This file is part of the Epiphany Software Development Kit.

  Copyright (C) 2013 Adapteva, Inc.
  Contributed by Oleg Raikhman, Yaniv Sapir <support@adapteva.com>

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

#ifndef __E_LOADER_H__
#define __E_LOADER_H__

#include "e-hal.h"

typedef enum {
	L_D1 = 1,
	L_D2 = 2,
	L_D3 = 3,
	L_D4 = 40,
} e_loader_diag_t;

int e_load(char *srecFile, bool reset_target, bool broadcast, bool run_target);
int parseAndSendSrecFile(char *srecFile, Epiphany_t *p, bool broadcast);

void e_set_loader_verbosity(e_loader_diag_t verbose);

#endif // __E_LOADER_H__
