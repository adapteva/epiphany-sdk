/*
  File: epiphany-hal-api.h

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

#ifndef __E_HAL_API_H__
#define __E_HAL_API_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/////////////////////////////////
// Device communication functions
//
// Epiphany access
int     e_open(Epiphany_t *dev);
int     e_close(Epiphany_t *dev);
ssize_t e_read(Epiphany_t *dev, unsigned corenum, const off_t from_addr, void *buf, size_t count);
ssize_t e_write(Epiphany_t *dev, unsigned corenum, off_t to_addr, const void *buf, size_t count);
int     e_read_word(Epiphany_t *dev, unsigned corenum, const off_t from_addr);
ssize_t e_write_word(Epiphany_t *dev, unsigned corenum, off_t to_addr, int data);
ssize_t e_read_buf(Epiphany_t *dev, unsigned corenum, const off_t from_addr, void *buf, size_t count);
ssize_t e_write_buf(Epiphany_t *dev, unsigned corenum, off_t to_addr, const void *buf, size_t count);
int     e_read_reg(Epiphany_t *dev, unsigned corenum, const off_t from_addr);
ssize_t e_write_reg(Epiphany_t *dev, unsigned corenum, off_t to_addr, int data);
int     e_read_esys(Epiphany_t *dev, const off_t from_addr);
ssize_t e_write_esys(Epiphany_t *dev, off_t to_addr, int data);
//
// eDRAM access
int     e_alloc(DRAM_t *dram, off_t mbase, size_t msize);
int     e_free(DRAM_t *dram);
ssize_t e_mread(DRAM_t *dram, const off_t from_addr, void *buf, size_t count);
ssize_t e_mwrite(DRAM_t *dram, off_t to_addr, const void *buf, size_t count);
int     e_mread_word(DRAM_t *dram, const off_t from_addr);
ssize_t e_mwrite_word(DRAM_t *dram, off_t to_addr, int data);
ssize_t e_mread_buf(DRAM_t *dram, const off_t from_addr, void *buf, size_t count);
ssize_t e_mwrite_buf(DRAM_t *dram, off_t to_addr, const void *buf, size_t count);
//
// For legacy code support
ssize_t e_read_abs(unsigned address, void* buf, size_t burst_size);
ssize_t e_write_abs(unsigned address, void *buf, size_t burst_size);

/////////////////////////
// Core control functions
int e_reset_core(Epiphany_t *pEpiphany, unsigned corenum);
int e_reset(Epiphany_t *pEpiphany, e_resetid_t resetid);
int e_start(Epiphany_t *pEpiphany, unsigned coreid);


////////////////////
// Utility functions
unsigned e_get_num_from_coords(unsigned row, unsigned col);
unsigned e_get_num_from_id(unsigned coreid);
unsigned e_get_id_from_coords(unsigned row, unsigned col);
unsigned e_get_id_from_num(unsigned corenum);
void     e_get_coords_from_id(unsigned coreid, unsigned *row, unsigned *col);
void     e_get_coords_from_num(unsigned corenum, unsigned *row, unsigned *col);

void e_set_host_verbosity(e_hal_diag_t verbose);

#ifdef __cplusplus
}
#endif

#endif // __E_HAL_API_H__

