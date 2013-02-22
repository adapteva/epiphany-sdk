/*
  File: epiphany_platform.h

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

/**
 *
 * Desc:  Define Epiphany platform definition structure.
 *
**/

// NO_IOREG can be assigned to ioreg_row and ioreg_col to indicate that
// a <chip> element did not have an <ioreg> child.
#define NO_IOREG (~0u)

// Structure describing each chip in the system
typedef struct
{
	char    *version;          // version of the chip
	unsigned yid;              // chip coordinates (YID pins)
	unsigned xid;              // chip coordinates (XID pins)
	unsigned ioreg_row;        // row within chip where I/O registers are located
	unsigned ioreg_col;        // column within chip where I/O registers are located
	unsigned num_rows;         // number of rows in the chip
	unsigned num_cols;         // number of cols in the chip
	unsigned host_base;        // base address of host (for reset, readback, etc)
	unsigned core_memory_size; // bytes of internal memory in each core
} chip_def_t;

// Structure describing each external memory segment available to the chips
typedef struct
{
	char    *name; // name of the memory segment (can be used in linker script)
	unsigned base; // base address of memory segment
	unsigned size; // number of bytes in the memory segment
} mem_def_t;

// Structure containing the data parsed from the XML file and
// passed to a subordinate function.
typedef struct
{
	unsigned    version;     // version number of this structure
	char       *name;        // name of the platform (e.g. "AAHM")
	char       *lib;         // name of platform library (e.g. "libftdi_target")
	char       *libinitargs; // additional argument string passed to library init fxn
	unsigned    num_chips;   // number of elements in chips[] array
	chip_def_t *chips;       // array of chips[]
	unsigned    num_banks;   // number of elements in ext_mem[] array
	mem_def_t  *ext_mem;     // array of ext_mem[]
} platform_definition_t;

