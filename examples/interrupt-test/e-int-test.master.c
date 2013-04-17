/*
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

#include <machine/epiphany_config.h>

#include "e_lib.h"

#define Fhz   (600e6)            // Clock frequency (Hz)
#define Td    (1)                // Delay time (sec)
#define Delay (Td * Fhz)

#define SOFF          0x3        // Signal number offset
#define TIMER0_VECTOR 0x0000000c // address of TIMER0 entry in IVT
#define B_OPCODE      0x000000e8 // OpCode of the B<*> instruction


void timer_isr(int signum);

unsigned volatile M[5] SECTION("shared_dram");


// Define macro for indirect interrupt dispathcer method.
// Undef for the direct and faster, but more explicit, method.
#define INDIRECT

int main(void)
{
	// Save initial state (coreID) to mailbox
	M[0] = 0x808;

#ifdef INDIRECT

	// Register the ISR with the interrupt dispatcher
	signal((E_TIMER0_INT + SOFF), timer_isr);

#else

	unsigned *ivt;

	// The event vector is a relative branch instruction to the IRS.
	// To use the direct ISR dispatch, we need to re-program the
	// IVT entry with the new branch instruction.

	// Set TIMER0 IVT entry address
	ivt  = (unsigned *) TIMER0_VECTOR;
	// Set the relative branch offset.
	*ivt = (((unsigned) (timer_isr - 0x0c)) >> 1) << 8;
	// Add the instruction opcode.
	*ivt = *ivt | B_OPCODE;

#endif

	// Enable the TIMER0 interrupt
	e_irq_enable((E_TIMER0_INT + SOFF));
	e_gie();

	// Start the timer, counting for ~1 sec, and wait for
	// it to finish.
	e_ctimer_set(E_CTIMER_0, E_CTIMER_CLK, Delay);
	while (e_ctimer_get(E_CTIMER_0));
	// At this point, a TIMER0 interrupt should have been fired

	// Save final state to mailbox
	M[4] = 0x44444444;

	return 0;
}


void __attribute__((interrupt)) timer_isr(int signum)
{
	M[2] = 0x22222222;

	e_irq_remote_raise(0x809, (E_SYNC + SOFF));

	M[3] = 0x33333333;

	return;
}
