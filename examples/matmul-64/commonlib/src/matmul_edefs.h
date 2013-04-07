/*
  matmul_dma.h

  Copyright (C) 2012 Adapteva, Inc.
  Contributed by Yaniv Sapir <yaniv@adapteva.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program, see the file COPYING.  If not, see
  <http://www.gnu.org/licenses/>.
*/


#ifndef __MATMUL_EDEFS_H__
#define __MATMUL_EDEFS_H__

#include <e_coreid.h>
#include <e_ctimers.h>
#include <e_dma.h>


#define __E_DMA__ENABLE_BLOCKING_DMA_PATCH__


int  dma0cpyi(void *src, void *dst);
int  dma0cpyo(void *src, void *dst);
int  dma0cpyc(void *src, void *dst);

#endif // __MATMUL_EDEFS_H__
