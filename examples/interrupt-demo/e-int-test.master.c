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

#include "e_lib.h"

#define Fhz   (600e6)            // Clock frequency (Hz)
#define Td    (1)                // Delay time (sec)
#define Delay (Td * Fhz)

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
	e_irq_attach(E_TIMER0_INT, timer_isr);

#else

	unsigned *ivt;

	// The event vector is a relative branch instruction to the IRS.
	// To use the direct ISR dispatch, we need to re-program the
	// IVT entry with the new branch instruction.

	// Set TIMER0 IVT entry address
	ivt  = (unsigned *) TIMER0_VECTOR;
	// Set the relative branch offset.
	*ivt = (((unsigned) timer_isr - (unsigned) ivt) >> 1) << 8;
	// Add the instruction opcode.
	*ivt = *ivt | B_OPCODE;

#endif

	// Enable the TIMER0 interrupt
	e_ctimer_start(E_CTIMER_0, E_CTIMER_OFF);
	e_irq_mask(E_TIMER0_INT, E_FALSE);
	e_irq_global_mask(E_FALSE);

	// Start the timer, counting for ~1 sec, and wait for
	// it to finish.
	e_ctimer_set(E_CTIMER_0, Delay);
	e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
	while (e_ctimer_get(E_CTIMER_0));
	// At this point, a TIMER0 interrupt event should have been fired

	// Save final progress state to mailbox
	M[4] = 0x44444444;

	return 0;
}


void __attribute__((interrupt)) timer_isr(int signum)
{
	// This ISR is called when the respective event occured (IOW, when
	// the event it was attached to by e_irq_attach() is raised). Upon
	// call, it sends the SYNC signal to the neighor core, causing it
	// to start runnign its program.

	// Save progress state to mailbox
	M[2] = 0x22222222;

	// Raise a SYNC event on neighbor core, to make it start running
	e_irq_set(0, 1, E_SYNC);

	// Save progress state to mailbox
	M[3] = 0x33333333;

	return;
}

