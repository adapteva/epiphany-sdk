/*
  File: e_coreid.h

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

#ifndef _E_COREID_H_
#define _E_COREID_H_

/* The following constants should be defined in the LDF file used to build the executable. */
extern const unsigned _NUM_ROWS_IN_CHIP_;
extern const unsigned _NUM_COLS_IN_CHIP_;
extern const unsigned _FIRST_CORE_ROW_;
extern const unsigned _FIRST_CORE_COL_;
#define E_ROWS_IN_CHIP   ((unsigned)(&_NUM_ROWS_IN_CHIP_))
#define E_COLS_IN_CHIP   ((unsigned)(&_NUM_COLS_IN_CHIP_))
#define E_FIRST_CORE_ROW ((unsigned)(&_FIRST_CORE_ROW_))
#define E_FIRST_CORE_COL ((unsigned)(&_FIRST_CORE_COL_))

typedef unsigned int e_coreid_t;

typedef enum
{
/* e_neighbor_id() wrap constants */
	E_CHIP_WRAP = 0,
	E_ROW_WRAP  = 1,
	E_COL_WRAP  = 2,
/* e_neighbor_id() dir constants */
	E_NEXT_CORE = 0,
	E_PREV_CORE = 1
} e_coreid_wrap_t;

e_coreid_t e_get_coreid();

e_coreid_t e_coreid_from_address(const void *ptr);

e_coreid_t e_coreid_from_coords(unsigned row, unsigned col);

void *e_address_from_coreid(e_coreid_t coreid, void *ptr);

void e_coords_from_coreid(e_coreid_t coreid, unsigned *row, unsigned *col);

int e_is_oncore(const void *ptr);

e_coreid_t e_coreid_origin(void);

int e_neighbor_id(e_coreid_t *coreid, e_coreid_wrap_t dir, e_coreid_wrap_t wrap);


#if 0
unsigned e_corenum(e_coreid_t coreid);
int e_is_onchip(const void* ptr);
e_coreid_t e_coreid_se(void);
unsigned e_get_chip_max_row(e_coreid_t coreid);
unsigned e_get_chip_max_col(e_coreid_t coreid);
unsigned e_get_chip_min_row(e_coreid_t coreid);
unsigned e_get_chip_min_col(e_coreid_t coreid);
unsigned e_get_chip_rows(void);
unsigned e_get_chip_cols(void);
#endif /* 0 */

#endif /* _E_COREID_H_ */

