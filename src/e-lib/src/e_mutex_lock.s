/*
  File: e_mutex_lock.s

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

.file    "e_mutex_lock.s"

.section .text;
.type     _e_mutex_lock, %function
.global   _e_mutex_lock;

.balign 4;
_e_mutex_lock:
// ------------------------------------------------------------------------
// r0 - param1 is the address of the mutex memory location

        and   r1, r0, r0;                    // copy mutex address into r1
    //----
        movfs r2, coreid;                    // read the coreid register
    //----
        mov   r3, 0xfff;                     // create mask for 12 LSBs for coreid
    //----
        and   r2, r2, r3;                    // mask the coreid, to be written to the mutex
    //----

//FIXME-- due to silicon issues, there seems to be a problem when reading the mutex memory location.
//we want to check that the mutex memory value is not already equal to this core's coreid value

//        ldr   r3, [r1, 0];                   // read the contents of the mutex location
    //----
//        eor   r3, r3, r2;                    // compare the mutex contents with the coreid
    //----
//        beq   __mutex_lock_error;            // they are the same! we have a problem
    //----
        eor   r3, r3, r3;                    // clr register. used as an offset for testset instruction

.balign 4;
__mutex_lock_test:

        testset r2, [r1, r3];                // attempt to gain mutex lock
    //----
        and  r0, r2, r2;                     // set the status flags
    //----
        bne  __mutex_lock_test;              // if r2 is zero, then we own the mutex
    //----
        rts;                                 // return to callee once mutex is gained, return a zero for success

.balign 4;
__mutex_lock_error:

        mov   r0, 0xffff;                    // set return value to -1
    //----
        movt  r0, 0xffff;
    //----
        rts;                                 // return to callee, with -1


.size	_e_mutex_lock, .-_e_mutex_lock


/* ------------------------------------------------------------------------
   End of File
   ------------------------------------------------------------------------ */

