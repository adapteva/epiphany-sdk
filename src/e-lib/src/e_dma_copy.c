/*
  File: e_dma_copy.c

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

#include "e_common.h"
#include "e_dma.h"


e_tcb_t _tcb SECTION(".data_bank0");

unsigned dma_configs[8] =
{
	(E_DMA_DOUBLE | E_DMA_MASTER | E_DMA_ENABLE),
	(E_DMA_BYTE   | E_DMA_MASTER | E_DMA_ENABLE),
	(E_DMA_SHORT  | E_DMA_MASTER | E_DMA_ENABLE),
	(E_DMA_BYTE   | E_DMA_MASTER | E_DMA_ENABLE),
	(E_DMA_LONG   | E_DMA_MASTER | E_DMA_ENABLE),
	(E_DMA_BYTE   | E_DMA_MASTER | E_DMA_ENABLE),
	(E_DMA_SHORT  | E_DMA_MASTER | E_DMA_ENABLE),
	(E_DMA_BYTE   | E_DMA_MASTER | E_DMA_ENABLE)
};

int e_dma_copy(e_dma_id_t chan, void *dst, void *src, size_t bytes, e_dma_align_t align)
{
	unsigned index;
	unsigned shift;
	unsigned stride;
	unsigned config;

	switch (align)
	{
	default:
		return -1;
	case E_ALIGN_BYTE:
		config = E_DMA_BYTE   | E_DMA_MASTER | E_DMA_ENABLE;
		break;
	case E_ALIGN_SHORT:
		config = E_DMA_SHORT  | E_DMA_MASTER | E_DMA_ENABLE;
		break;
	case E_ALIGN_LONG:
		config = E_DMA_LONG   | E_DMA_MASTER | E_DMA_ENABLE;
		break;
	case E_ALIGN_DOUBLE:
		config = E_DMA_DOUBLE | E_DMA_MASTER | E_DMA_ENABLE;
		break;
	case E_ALIGN_AUTO:
		index = (((unsigned) dst) | ((unsigned) src) | ((unsigned) bytes)) & 7;
		config = dma_configs[index];
		break;
	}
	shift = config >> 5;
	stride = 0x10001 << shift;

	_tcb.config = config;
	_tcb.inner_stride = stride;
	_tcb.count = 0x10000 | (bytes >> shift);
	_tcb.outer_stride = stride;
	_tcb.src_addr = src;
	_tcb.dst_addr = dst;

	return e_dma_start(chan, &_tcb);
}

