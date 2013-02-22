/*
  File: e_ic.h

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

#ifndef E_IC_H_
#define E_IC_H_
/**
 * @file e_ic.h
 * @brief Interrupt (un)set, (un)mask
 *
 * @section DESCRIPTION
 * The interrupt (un)set, (un)mask routine
 *
 * @section SEE_ALSO SEE ALSO
 * signal and raise from new lib
 *
 */

typedef enum {
	E_SYNC         = 0,
	E_SW_EXCEPTION = 1,
	E_MEM_FAULT    = 2,
	E_TIMER0_INT   = 3,
	E_TIMER1_INT   = 4,
	E_DMA0_INT     = 6,
	E_DMA1_INT     = 7,
	E_SW_INT       = 9
} e_irq_type_t;

typedef enum
{
	E_IRQ_DISABLE  = 0,
	E_IRQ_ENABLE   = 1
} e_irq_state_t;

/** @fn e_irq_state_t e_gid();
    @brief Disable global interrupts
    @return Previous state of the interrupt enable bit
*/
e_irq_state_t e_gid(void);

/** @fn e_irq_state_t e_gie();
    @brief Enable global interrupts
    @return Previous state of the interrupt enable bit
*/
e_irq_state_t e_gie(void);

/** @fn e_irq_state_t e_gie_restore();
    @brief Set or clear the global interrupt enable state
    @param state IRQ_ENABLE or IRQ_DISABLE selects the action.
    @return Previous state of GIE/GID bit.
*/
e_irq_state_t e_gie_restore(e_irq_state_t state);

/** @fn e_irq_state_t e_irq_disable();
    @brief Disable the specified interrupt
    @param irq Interrupt number
    @return Value to restore the interrupt state to its pre-call condition if passed to e_irq_restore()
*/
unsigned e_irq_disable(e_irq_type_t irq);

/** @fn e_irq_state_t e_irq_enable();
    @brief Enable the specified interrupt
    @param irq Interrupt number
    @return Value to restore the interrupt state to its pre-call condition if passed to e_irq_restore()
*/
unsigned e_irq_enable(e_irq_type_t irq);

/** @fn void e_irq_raise();
    @brief Fire interrupt
    @param signal number, see machine/epiphany_config.h for signals definitions
    @return none
*/
int e_irq_raise(e_irq_type_t irq);

/** @fn void e_irq_remote_raise();
    @brief Fire interrupt in remote core
    @param signal number, see machine/epiphany_config.h for signals definitions
    @return none
*/
int e_irq_remote_raise(unsigned coreid, e_irq_type_t irq);

/** @fn e_irq_state_t e_irq_lower();
    @brief Clear a bit in the interrupt latch
    @param irq Interrupt number
*/
void e_irq_lower(e_irq_type_t irq);

/** @fn void e_irq_restore();
    @brief Restore the interrupt state to a previous condition
    @param mask Previous condition.
    @return none
*/
void e_irq_restore(unsigned mask);

#endif /* E_IC_H_ */
