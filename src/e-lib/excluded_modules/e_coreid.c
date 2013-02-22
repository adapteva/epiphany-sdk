/*
 * e_coreid.c
 *
 */
#include <signal.h>
#include <machine/epiphany_config.h>
#include "e_coreid.h"

#define INTERNAL_CORE_MASK (((E_ROWS_IN_CHIP - 1) << 6) | (E_COLS_IN_CHIP - 1))
#define CHIP_MIN_COL(coreid) (coreid & (~INTERNAL_CORE_MASK & 0x3f))
#define CHIP_MAX_COL(coreid) ((coreid | INTERNAL_CORE_MASK) & 0x3f)


e_coreid_t e_get_coreid()
{
	register unsigned coreid_in_reg asm("r0");
	__asm__ __volatile__ ("MOVFS %0, coreid" : : "r" (coreid_in_reg));

	return (coreid_in_reg);
}

e_coreid_t e_coreid_from_address(const void *ptr)
{
	e_coreid_t coreid;

	coreid = ((e_coreid_t) ptr >> 20);

	return (coreid) ? coreid : e_get_coreid();
}

e_coreid_t e_coreid_from_coords(unsigned row, unsigned col)
{
	return ((row & 0x3f) << 6) | (col & 0x3f);
}

void *e_address_from_coreid(e_coreid_t coreid, void *ptr)
{
	unsigned uptr;

	if (!e_is_oncore(ptr))
	{
		return ptr;
	}
	/* get the 20 ls bits of the pointer. */
	uptr = (unsigned) ptr & 0x000fffff;

	return (void *)((coreid << 20) | uptr);
}

void e_coords_from_coreid(e_coreid_t coreid, unsigned *row, unsigned *col)
{
	*row = (coreid >> 6) & 0x3f;
	*col = coreid & 0x3f;

	return;
}

/* Is address on this core? */
int e_is_oncore(const void *ptr)
{
	return (e_coreid_from_address(ptr) == e_get_coreid()) ? 1 : 0;
}

e_coreid_t e_coreid_origin(void)
{
	return e_get_coreid() & ~INTERNAL_CORE_MASK;
}

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

#if 0

int e_is_oncore(const void *ptr);
unsigned e_get_chip_min_col(e_coreid_t coreid);
unsigned e_get_chip_max_col(e_coreid_t coreid);

/* Returns the minimum global column number in the chip. */
unsigned e_get_chip_min_col(e_coreid_t coreid)
{
	return (coreid & (~INTERNAL_CORE_MASK & 0x3f));
}

/* Returns the maximum global column number in the chip. */
unsigned e_get_chip_max_col(e_coreid_t coreid)
{
	return (coreid | INTERNAL_CORE_MASK) & 0x3f;
}

unsigned e_corenum(e_coreid_t coreid)
{
	unsigned row, col;

	row = (coreid >> 6) & (E_ROWS_IN_CHIP - 1);
	col =  coreid       & (E_COLS_IN_CHIP - 1);
	return row * E_COLS_IN_CHIP + col;
}

int e_is_onchip(const void *ptr)
{
	return ((e_coreid_from_address(ptr) ^ e_get_coreid()) & ~INTERNAL_CORE_MASK) ? 0:1;
}

e_coreid_t e_coreid_nw(void)
{
	return e_get_coreid() & ~INTERNAL_CORE_MASK;
}

e_coreid_t e_coreid_se(void)
{
	return e_get_coreid() | INTERNAL_CORE_MASK;
}

/* Returns the maximum global row number in the chip. */
unsigned e_get_chip_max_row(e_coreid_t coreid)
{
	return (coreid | INTERNAL_CORE_MASK) >> 6;
}

/* Returns the minimum global row number in the chip.*/
unsigned e_get_chip_min_row(e_coreid_t coreid)
{
	return (coreid & ~INTERNAL_CORE_MASK) >> 6;
}

unsigned e_get_chip_rows(void)
{
	return E_ROWS_IN_CHIP;
}

unsigned e_get_chip_cols(void)
{
	return E_COLS_IN_CHIP;
}

/*
 Returns the maximum global row number in the system (which can consist of
 multiple chips).
*/
unsigned e_get_system_max_row(void)
{
#warning e_get_system_max_row() is not implemented
	return 0;
}

/*
 Returns the maximum global column number in the system (which can consist
 of multiple chips).
*/
unsigned e_get_system_max_col(void)
{
#warning e_get_system_max_col() is not implemented
	return 0;
}

/*
 Returns the minimum global row number in the system (which can consist of
 multiple chips).
*/
unsigned e_get_system_min_row(void)
{
#warning e_get_system_min_row() is not implemented
	return 0;
}

/*
 Returns the minimum global column number in the system (which can consist
 of multiple chips).
*/
unsigned e_get_system_min_col(void)
{
#warning e_get_system_min_col() is not implemented
	return 0;
}

unsigned e_get_system_rows(void)
{
#warning e_get_system_rows() not implemented
	return 0;
}

unsigned e_get_system_cols(void)
{
#warning e_get_system_cols() not implemented
	return 0;
}

#endif /* 0 */

