/*
  matmul_dma.c

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


// Local implementation of DMA API's

#include "matmul.h"
#include "matmul_edefs.h"


extern volatile e_tcb_t _tcb; // TCB structure for DMA


int __attribute__ ((noinline)) dma0start(e_tcb_t *tcb)
{
	int           status;
	unsigned      start;

	/* wait for the DMA engine to be idle */
	status = ~ 0x0;
	while (status)
	{
		__asm__ __volatile__ ("MOVFS %0, DMA0STATUS"  : "=r" (status) : );
		status = status & 0xf;
	}

#ifdef __E_DMA__ENABLE_BLOCKING_DMA_PATCH__
	int      global_to_internal;
	unsigned stride_i;
	unsigned stride_o;
	unsigned count_i;
	unsigned count_o;
	unsigned size;
	unsigned char volatile *last_addr_g;
	unsigned char volatile *last_addr_i;
	unsigned char           sentinel;
	unsigned int  volatile *wlast_addr;
	unsigned int  register  wsentinel;
	unsigned int            coffset, boffset;

	global_to_internal = 0;
	if (((unsigned) tcb->src_addr & 0xfff00000) && !((unsigned) tcb->dst_addr & 0xfff00000))
		global_to_internal = ~ global_to_internal;

	if (global_to_internal)
	{
		count_i      = (tcb->count >>  0) & 0xffff;
		count_o      = (tcb->count >> 16) & 0xffff;
		stride_i     = (tcb->inner_stride >>  0) & 0xffff;
		stride_o     = (tcb->outer_stride >>  0) & 0xffff;
		size         = (((count_i - 1) * stride_i) + stride_o) * count_o - stride_o;
		last_addr_g  = (unsigned char *) tcb->src_addr + size;
		do {
//			sentinel   = *last_addr;
			wlast_addr = ((unsigned int) last_addr_g) & 0xfffffffc; // align address to word boundary
			coffset    = ((unsigned int) last_addr_g) & 0x00000003; // offset of loaded byte
			boffset    = coffset * 8;
			__asm__ __volatile__ ("nop");
			wsentinel   = *wlast_addr;
#			include "delay_nops.c"
			sentinel = (wsentinel >> boffset) & 0xff;
		} while (0);

		stride_i     = (tcb->inner_stride >> 16) & 0xffff;
		stride_o     = (tcb->outer_stride >> 16) & 0xffff;
		size         = (((count_i - 1) * stride_i) + stride_o) * count_o - stride_o;
		last_addr_i  = (unsigned char *) tcb->dst_addr + size;
//		*last_addr_i = ~sentinel;
		*last_addr_i = 0xff;
	}
#endif // __E_DMA__ENABLE_BLOCKING_DMA_PATCH__


	start = ((volatile int) (tcb) << 16) | E_DMA_STARTUP;
	__asm__ __volatile__ ("MOVTS DMA0CONFIG, %0"  : : "r" (start) );


#ifdef __E_DMA__ENABLE_BLOCKING_DMA_PATCH__
	/* wait for the DMA engine to finish */
	if (global_to_internal)
	{
		while (*last_addr_i == 0xff) { __asm__ __volatile__ ("nop"); };
	} else {
		/* wait for the DMA engine to be idle */
		status = ~ 0x0;
		while (status)
		{
			__asm__ __volatile__ ("MOVFS %0, DMA0STATUS"  : "=r" (status) : );
			status = status & 0xf;
		}
	}
#else
	/* wait for the DMA engine to be idle */
	while (e_dma_busy(E_DMA_0)) {};
#endif

	return 0;
}


int dma0cpyi(void *src, void *dst)
{
	unsigned stride_i_src;
	unsigned stride_i_dst;
	unsigned stride_o_src;
	unsigned stride_o_dst;
	unsigned count_i;
	unsigned count_o;
	unsigned config;

	config       = E_DMA_DOUBLE | E_DMA_MASTER | E_DMA_ENABLE;
	stride_i_src = (1 << 3);
	stride_i_dst = (1 << 3);
	stride_o_src = ((_Smtx - _Score) << 2) + (1 << 3);
	stride_o_dst = (1 << 3);
	count_i      = _Score >> 1;
	count_o      = _Score;

	/* wait for the DMA engine to be idle */
	while (e_dma_busy(E_DMA_0)) {};

	_tcb.config       = config;
	_tcb.inner_stride = (stride_i_dst << 16) | (stride_i_src);
	_tcb.count        = (count_o << 16)      | (count_i);
	_tcb.outer_stride = (stride_o_dst << 16) | (stride_o_src);
	_tcb.src_addr     = src;
	_tcb.dst_addr     = dst;

	dma0start((e_tcb_t *) (&_tcb));

	return 0;
}


int dma0cpyo(void *src, void *dst)
{
	unsigned stride_i_src;
	unsigned stride_i_dst;
	unsigned stride_o_src;
	unsigned stride_o_dst;
	unsigned count_i;
	unsigned count_o;
	unsigned config;

	config       = E_DMA_DOUBLE | E_DMA_MASTER | E_DMA_ENABLE;
	stride_i_src = (1 << 3);
	stride_i_dst = (1 << 3);
	stride_o_src = (1 << 3);
	stride_o_dst = ((_Smtx - _Score) << 2) + (1 << 3);
	count_i      = _Score >> 1;
	count_o      = _Score;

	/* wait for the DMA engine to be idle */
	while (e_dma_busy(E_DMA_0)) {};

	_tcb.config       = config;
	_tcb.inner_stride = (stride_i_dst << 16) | (stride_i_src);
	_tcb.count        = (count_o << 16)      | (count_i);
	_tcb.outer_stride = (stride_o_dst << 16) | (stride_o_src);
	_tcb.src_addr     = src;
	_tcb.dst_addr     = dst;

	dma0start((e_tcb_t *) (&_tcb));

	return 0;
}


int dma0cpyc(void *src, void *dst)
{
	unsigned stride_i_src;
	unsigned stride_i_dst;
	unsigned stride_o_src;
	unsigned stride_o_dst;
	unsigned count_i;
	unsigned count_o;
	unsigned config;

	config       = E_DMA_DOUBLE | E_DMA_MASTER | E_DMA_ENABLE;
	stride_i_src = (1 << 3);
	stride_i_dst = (1 << 3);
	stride_o_src = (1 << 3);
	stride_o_dst = (1 << 3);
	count_i      = _Score >> 1;
	count_o      = _Score;

	/* wait for the DMA engine to be idle */
	while (e_dma_busy(E_DMA_0)) {};

	_tcb.config       = config;
	_tcb.inner_stride = (stride_i_dst << 16) | (stride_i_src);
	_tcb.count        = (count_o << 16)      | (count_i);
	_tcb.outer_stride = (stride_o_dst << 16) | (stride_o_src);
	_tcb.src_addr     = src;
	_tcb.dst_addr     = dst;

	dma0start((e_tcb_t *) (&_tcb));

	return 0;
}
