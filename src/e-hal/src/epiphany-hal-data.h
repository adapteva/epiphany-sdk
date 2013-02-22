/*
  File: epiphany-hal-data.h

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

#ifndef __E_HAL_DATA_H__
#define __E_HAL_DATA_H__


#ifdef __cplusplus
extern "C"
{
#else
typedef enum BOOL {
	false = 0,
	true  = 1,
} bool;
#endif


typedef enum {
	H_D1 = 1,
	H_D2 = 2,
	H_D3 = 3,
	H_D4 = 4,
} e_hal_diag_t;


#define LOC_BASE_MEMS            0x0000
#define MAP_SIZE_MEMS            0x8000
#define MAP_MASK_MEMS (MAP_SIZE_MEMS-1)
#define LOC_BASE_REGS           0xF0000
#define MAP_SIZE_REGS            0x1000
#define MAP_MASK_REGS (MAP_SIZE_REGS-1)
#define EPI_CORES (EPI_ROWS * EPI_COLS)

#ifndef EPI_OK
#	define EPI_OK     0
#	define EPI_ERR    1
#	define EPI_WARN   2
#endif



// Core Registers
#define EPI_ILAT       0x0428
#define EPI_CONFIG     0x0400
#define EPI_STATUS     0x0404
#define EPI_PC         0x0408
#define EPI_COREID     0x0704
#define EPI_CORE_RESET 0x070c

// Epiphany System Registers
#define ESYS_CONFIG    0x0f00
#define ESYS_RESET     0x0f04
#define ESYS_VERSION   0x0f08
#define ESYS_FILTERL   0x0f0c
#define ESYS_FILTERH   0x0f10
#define ESYS_FILTERC   0x0f14
#define ESYS_TIMEOUT   0x0f18


typedef enum {
	E_RESET_CORES = 0,
	E_RESET_CHIP  = 1,
	E_RESET_ESYS  = 3,
} e_resetid_t;


typedef struct {
	off_t           phy_base;    // physical global base address of memory region
	size_t          map_size;    // size of mapped region
	off_t           map_mask;    // for mmap
	void           *mapped_base; // for mmap
	void           *base;        // application space base address of memory region
} Epiphany_mmap_t;
/*
typedef struct {
	off_t           phy_base;    // physical global base address of core's SRAM arrays
	size_t          map_size;    // size of core's SRAM arrays
	off_t           map_mask;    // for mmap
	void           *mapped_base; // for mmap
	void           *base;        // application space base address of core's SRAM arrays
} Epiphany_core_mems_t;

typedef struct {
	off_t           phy_base;    // physical global base address of core's e-regs section
	size_t          map_size;    // size of core's e-regs section
	off_t           map_mask;    // for mmap
	void           *mapped_base; // for mmap
	void           *base;        // application space base address of core's e-regs section
} Epiphany_core_regs_t;

typedef struct {
	off_t           phy_base;    // physical global base address of e-sys regs section
	size_t          map_size;    // size of e-sys regs section
	off_t           map_mask;    // for mmap
	void           *mapped_base; // for mmap
	void           *base;        // application space base address of e-sys regs section
} Epiphany_esys_t;
*/
typedef struct {
	unsigned int    id;     // core ID
	unsigned int    row;    // core absolute row number
	unsigned int    col;    // core absolute col number
	Epiphany_mmap_t mems;   // core's SRAM data structure
	Epiphany_mmap_t regs;   // core's e-regs data structure
} Epiphany_core_t;

typedef struct {
	unsigned int    num_cores;   // number of cores in chip
	int             memfd;       // for mmap
	unsigned int    row;         // chip absolute row number
	unsigned int    col;         // chip absolute col number
	unsigned int    rows;        // number of rows in chip
	unsigned int    cols;        // number of cols in chip
	Epiphany_core_t core[EPI_CORES]; // e-cores data structures array
	Epiphany_mmap_t esys;        // e-system registers data structure
} Epiphany_t;

typedef struct {
	off_t           phy_base;    // physical global base address of eDRAM buffer as seen by host side
	size_t          map_size;    // size of eDRAM allocated buffer for host side
	off_t           ephy_base;   // physical global base address of eDRAM buffer as seen by device side
	size_t          emap_size;   // size of eDRAM allocated buffer for device side
	off_t           map_mask;    // for mmap
	void           *mapped_base; // for mmap
	void           *base;        // application space base address of eDRAM buffer
	int             memfd;       // for mmap
} DRAM_t;


#ifdef __cplusplus
}
#endif

#endif // __E_HAL_DATA_H__
