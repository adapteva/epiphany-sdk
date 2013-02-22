/*
  File: e_mutex_init.s

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

.file    "e_mutex_init.s"

.section .text;
.type     _e_mutex_init, %function
.global   _e_mutex_init;

.balign 4;
_e_mutex_init:
// ------------------------------------------------------------------------
#define MESH_MASK_COREID  0xfff0 0000

        lsr   r1, r0, 0x14;                  // shift to check the coreid value of the memory address
    //----
        beq   __mutex_init_error;            // if coreid is zero, then this is a local memory address
    //----
        eor   r1, r1, r1;                    // clr the register, to write to the mutex for initialisation
    //----
        str   r1, [r0, 0];                   // store zero to the mutex memory location
    //----
        eor   r0, r0, r0;                    // set return value to zero
    //----
        rts;                                 // return to callee, with zero

.balign 4;
__mutex_init_error:

        mov   r0, 0xffff;                    // set return value to -1
    //----
        movt  r0, 0xffff;
    //----
        rts;                                 // return to callee, with -1

.size	_e_mutex_init, .-_e_mutex_init


/* ------------------------------------------------------------------------
   End of File
   ------------------------------------------------------------------------ */

