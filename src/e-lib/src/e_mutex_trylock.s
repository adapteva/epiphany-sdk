/*
  File: e_mutex_trylock.s

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

.file    "e_mutex_trylock.s"

.section .text;
.type     _e_mutex_trylock, %function
.global   _e_mutex_trylock;

.balign 4;
_e_mutex_trylock:
// ------------------------------------------------------------------------
// r0 - param1 is the address of the mutex memory location

        movfs r1, coreid;                    // read the coreid register
    //----
        mov   r2, 0xfff;                     // create mask for 12 LSBs for coreid
    //----
        and   r1, r1, r2;                    // mask the coreid, to be written to the mutex
    //----
        eor   r3, r3, r3;                    // clr register. used as an offset for testset instruction
    //----
        testset r1, [r0, r3];                // attempt a mutex lock on the memory address
    //----
        and   r0, r1, r1;                    // move the result into the return register
    //----
        rts;                                 // return to callee, with zero if mutex is gained or non-zero otherwise

.size	_e_mutex_trylock, .-_e_mutex_trylock


/* ------------------------------------------------------------------------
   End of File
   ------------------------------------------------------------------------ */

