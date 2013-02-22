/*
 * elib_common.c
 *
 *  Created on: Mar 24, 2011
 *      Author: oraikhman
 */
#include <stdio.h>
#include <signal.h>
#include <machine/epiphany_config.h>

#include "e_types.h"

inline void e_Assert(int expr)
{
#ifndef __linux__
	if (!expr)
	{
		asm("trap 5");//FAIL
	}
#else
#include <assert.h>

	assert(expr);
#endif

}

inline unsigned e_getCoreBaseId()
{
	register unsigned coreid_in_reg asm("r0");
	__asm__ __volatile__ ("MOVFS %0, coreid" : : "r" (coreid_in_reg));

	return (coreid_in_reg);
}

inline unsigned e_getCoreRowNumber()
{
	return (e_getCoreBaseId() >> 6) & 0x3f;
}

inline unsigned e_getCoreColNumber()
{
	return (e_getCoreBaseId()) & 0x3f;
}

inline unsigned* e_getBaseAddrRowCol(unsigned row, unsigned col)
{
	return (void*)(((row << 6) | col) << 20);
}

int is_oncore(const void* ptr)
{
	unsigned id;
	unsigned uptr;

	id = e_getCoreBaseId();
	if ((uptr = ((unsigned)ptr >> 20)) == 0)
	{
		return 1;
	}

	return (uptr == id) ? 1:0;
}

void* e_getPtrRowCol(unsigned row, unsigned col, const void* ptr)
{
	unsigned id;
	unsigned uptr;

	if (!is_oncore(ptr))
	{
		return (void*)ptr;
	}
	// get the 20 ls bits of the pointer.
	uptr = (unsigned)ptr & 0x000fffff;
	id = (row << 6) | (col & 0x3f);
	return (void*)(uptr | (id << 20));
}

void* e_getPtrCore(unsigned core, const void* ptr)
{
	unsigned uptr;

	if (!is_oncore(ptr))
	{
		return (void*)ptr;
	}
	// get the 20 ls bits of the pointer.
	uptr = (unsigned)ptr & 0x000fffff;
	return (void*)(uptr | (core << 20));
}


extern unsigned _MAX_NUM_CORES_IN_ROWS__;
extern unsigned _MAX_NUM_CORES_IN_COLS__;

inline unsigned e_getMaxNumRows()
{
	return (unsigned)(&_MAX_NUM_CORES_IN_ROWS__);
}

inline unsigned e_getMaxNumCols()
{
	return (unsigned)(&_MAX_NUM_CORES_IN_COLS__);
}

inline unsigned e_getUpperLeftCoreId(void)
{
	unsigned id = e_getCoreBaseId();
	unsigned rowmask = (_MAX_NUM_CORES_IN_ROWS__ - 1) << 6;
	unsigned colmask = (_MAX_NUM_CORES_IN_COLS__ - 1);
	return id & ~(rowmask | colmask);
}

inline unsigned e_getLowerRightCoreId(void)
{
	unsigned id = e_getCoreBaseId();
	unsigned rowmask = (_MAX_NUM_CORES_IN_ROWS__ - 1) << 6;
	unsigned colmask = (_MAX_NUM_CORES_IN_COLS__ - 1);
	return id | (rowmask | colmask);
}

