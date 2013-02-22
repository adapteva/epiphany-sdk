/**
 * File: e_dma.c
 *
 * Desc:  Software to manage Epiphany dma engines
 *
**/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <machine/epiphany_config.h>
#include "e_ic.h"
#include "e_regs.h"
#include "e_common.h"
#include "e_dma.h"

int e_dma_start(e_dma_id_t chan, e_tcb_t *tcb)
{
	unsigned      start;
	e_irq_state_t irq_state;
	int           ret_val = -1;

	if ((chan | 1) != 1)
	{
		return -1;
	}

	irq_state = e_gid();

	/* wait for the DMA engine to be idle */
	while (e_dma_busy(chan));

	start = ((volatile int)(tcb) << 16) | E_DMA_STARTUP;
	switch (chan)
	{
	case 0:
		e_sysreg_write(E_DMA0CONFIG, start);
		ret_val = 0;
		break;
	case 1:
		e_sysreg_write(E_DMA1CONFIG, start);
		ret_val = 0;
		break;
	}

	e_gie_restore(irq_state);
	return ret_val;
}

int e_dma_busy(e_dma_id_t chan)
{
	switch (chan)
	{
	case 0:
		return e_sysreg_read(E_DMA0STATUS) & 0xf;
	case 1:
		return e_sysreg_read(E_DMA1STATUS) & 0xf;
	default:
		return -1;
	}
}

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

