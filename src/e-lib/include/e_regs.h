/*
  File: e_regs.h

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

#ifndef E_REGS_H_
#define E_REGS_H_

/**
 * @file e_regs.h
 * @brief Special register access
 *
 * @section DESCRIPTION
 * The special register access routine
 *
 */

/** @enum e_reg_id_t
 * The special registers access address offset
 */
typedef enum
{
	/* eCore Registers */
	E_CONFIG = 0xf0400,
	E_STATUS = 0xf0404,
	E_PC     = 0xf0408,
	E_IRET   = 0xf0420,
	E_IMASK  = 0xf0424,
	E_ILAT   = 0xf0428,
	E_ILATST = 0xf042C,
	E_ILATCL = 0xf0430,
	E_IPEND  = 0xf0434,

	/* DMA registers */
	E_DMA0CONFIG   = 0xf0500,
	E_DMA0STRIDE   = 0xf0504,
	E_DMA0COUNT    = 0xf0508,
	E_DMA0SRCADDR  = 0xf050C,
	E_DMA0DSTADDR  = 0xf0510,
	E_DMA0AUTODMA0 = 0xf0514,
	E_DMA0AUTODMA1 = 0xf0518,
	E_DMA0STATUS   = 0xf051C,
	E_DMA1CONFIG   = 0xf0520,
	E_DMA1STRIDE   = 0xf0524,
	E_DMA1COUNT    = 0xf0528,
	E_DMA1SRCADDR  = 0xf052C,
	E_DMA1DSTADDR  = 0xf0530,
	E_DMA1AUTODMA0 = 0xf0534,
	E_DMA1AUTODMA1 = 0xf0538,
	E_DMA1STATUS   = 0xf053C,

	/* Event Timer Registers */
	E_CTIMER0 = 0xf0438,
	E_CTIMER1 = 0xf043C,

	/* Processor Control Registers */
	E_MEMPROTECT = 0xf0608,
	E_COREID     = 0xf0704,
} e_reg_id_t;

/** @fn e_reg_id_t e_sysreg_read();
    @brief  Read a system register
    @param reg_id special register address offset ID
    @return register value
*/
unsigned e_sysreg_read(e_reg_id_t reg_id);

/** @fn e_reg_id_t e_sysreg_write();
    @brief  Write a value to a system register
    @param reg_id special register address offset ID
    @param val  register value
    @return void
*/
void e_sysreg_write(e_reg_id_t reg_id, unsigned val);

#endif /* E_REGS_H_ */
