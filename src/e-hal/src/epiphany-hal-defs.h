/*
  File: epiphany-hal-defs.h

  This file is part of the Epiphany Software Development Kit.

  Copyright (C) 2013 Adapteva, Inc.
  Contributed by Yaniv Sapir <support@adapteva.com>

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

#ifndef __E_HOST_DEFS_H__
#define __E_HOST_DEFS_H__

#ifndef System_ID
//#	warning Setting default System_ID to 2
#	define System_ID 2
#endif

#ifndef Core_ID
#	define Core_ID 0x808
#endif

#if System_ID == 1 // Zed E16 Ubuntu
#	define EPI_BASE_CORE_ID        Core_ID
#	define DRAM_BASE_ADDRESS    0x1e000000
#	define DRAM_SIZE            0x02000000
#	define EPI_EXT_MEM_BASE     0x8e000000
#	define EPI_EXT_MEM_SIZE     0x02000000
#	define EPI_ROWS                      4
#	define EPI_COLS                      4
#	define ESYS_BASE_REGS       0x808f0000
#elif System_ID == 2 // Zed E64 Ubuntu
#	define EPI_BASE_CORE_ID        Core_ID
#	define DRAM_BASE_ADDRESS    0x1e000000
#	define DRAM_SIZE            0x02000000
#	define EPI_EXT_MEM_BASE     0x8e000000
#	define EPI_EXT_MEM_SIZE     0x02000000
#	define EPI_ROWS                      8
#	define EPI_COLS                      8
#	define ESYS_BASE_REGS       0x808f0000
#	define __E64G4_BURST_PATCH__
#elif System_ID == 3 // Zynq E16 Ubuntu
#	define EPI_BASE_CORE_ID        Core_ID
#	define DRAM_BASE_ADDRESS    0x3e000000
#	define DRAM_SIZE            0x02000000
#	define EPI_EXT_MEM_BASE     0x8e000000
#	define EPI_EXT_MEM_SIZE     0x02000000
#	define EPI_ROWS                      4
#	define EPI_COLS                      4
#	define ESYS_BASE_REGS       0x808f0000
#else
#	error System type not defined
#endif

#endif // __E_HOST_DEFS_H__

