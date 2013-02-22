/*
  File: e_coreid_neighbor_id.c

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
#include "e_coreid.h"

#define INTERNAL_CORE_MASK (((E_ROWS_IN_CHIP - 1) << 6) | (E_COLS_IN_CHIP - 1))
#define CHIP_MIN_COL(coreid) (coreid & (~INTERNAL_CORE_MASK & 0x3f))
#define CHIP_MAX_COL(coreid) ((coreid | INTERNAL_CORE_MASK) & 0x3f)

/*
 This function calculates the coreid of the neighboring core, where neighbor
 can be the next core to north, east, south, or west. It will always calculate
 the id of another core on the same chip, wrapping on a row, column, or chip
 boundary as specified by the wrap argument.
 The return value is the difference between the neighboring coreid and the current
 one.
*/
int e_neighbor_id(e_coreid_t *coreid, e_coreid_wrap_t dir, e_coreid_wrap_t wrap)
{
	unsigned   row, col;
	e_coreid_t next;
	int        diff;
	unsigned   chip_mask;

	/* Indexed by [wrap][dir] */
	static const int core_adjust_table[3][2] =
	{
		{ 1,   -1 }, /* CHIP_WRAP */
		{ 1,   -1 }, /* ROW_WRAP  */
		{ 64, -64 }  /* COL_WRAP  */
	};

	chip_mask = *coreid & ~INTERNAL_CORE_MASK;
	next = ((*coreid + core_adjust_table[wrap][dir]) & INTERNAL_CORE_MASK) | chip_mask;
	if (wrap == E_CHIP_WRAP)
	{
		e_coords_from_coreid(next, &row, &col);
		if (dir == E_NEXT_CORE)
		{ /* if we wrapped back to the first column, we need to advance to the next row. */
			/* if (col == e_get_chip_min_col(next)) */
			if (col == CHIP_MIN_COL(next))
			{
				next = ((next + 0x40) & INTERNAL_CORE_MASK) | chip_mask;
			}
		}
		else /* dir == E_PREV_CORE */
		{ /* if we wrapped to the last column, we need to advance to the previous row. */
			/* if (col == e_get_chip_max_col(next)) */
			if (col == CHIP_MAX_COL(next))
			{
				next = ((next - 0x40) & INTERNAL_CORE_MASK) | chip_mask;
			}
		}
	}

	diff = next - *coreid;
	*coreid = next;

	return diff;
}
