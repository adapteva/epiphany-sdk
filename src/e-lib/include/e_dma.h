/*
  File: e_dma.h

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

#ifndef _EPIPHANY_DMA_H_
#define _EPIPHANY_DMA_H_

#include <stdlib.h>
#include <errno.h>

/*
  These defs can be or'd together to form a value suitable for
  the dma config reg.
*/
typedef enum
{
	E_DMA_ENABLE  = (1<<0),
	E_DMA_MASTER  = (1<<1),
	E_DMA_CHAIN   = (1<<2),
	E_DMA_STARTUP = (1<<3),
	E_DMA_IRQEN   = (1<<4),
	E_DMA_BYTE    = (0<<5),
	E_DMA_SHORT   = (1<<5),
	E_DMA_LONG    = (2<<5),
	E_DMA_DOUBLE  = (3<<5)
} e_dma_tcb_cfg_t;

typedef enum
{
	E_DMA_0 = 0,
	E_DMA_1 = 1
} e_dma_id_t;

typedef enum
{
	E_ALIGN_BYTE,
	E_ALIGN_SHORT,
	E_ALIGN_LONG,
	E_ALIGN_DOUBLE,
	E_ALIGN_AUTO,
} e_dma_align_t;

typedef struct
{
	unsigned long config;
	unsigned long inner_stride;
	unsigned long count;
	unsigned long outer_stride;
	void         *src_addr;
	void         *dst_addr;
} e_tcb_t;

int e_dma_start(e_dma_id_t chan, e_tcb_t *usr_tcb);
int e_dma_busy(e_dma_id_t chan);
int e_dma_copy(e_dma_id_t chan, void *dst, void *src, size_t bytes, e_dma_align_t align);

#endif /* _EPIPHANY_DMA_H_ */
