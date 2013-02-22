/*
  File: e_mutex_unlock.s

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

.file    "e_mutex_unlock.s"

.section .text;
.type     _e_mutex_unlock, %function
.global   _e_mutex_unlock;

.balign 4;
_e_mutex_unlock:
// ------------------------------------------------------------------------
// r0 - param1 is the address of the mutex memory location

        eor   r1, r1, r1;                    // set register to zero; to be written to mutex
    //----
        str   r1, [r0, 0x0];                 // clear mutex lock
    //----
        eor   r0, r0, r0;                    // set the return value to zero
    //----
        rts;                                 // return to callee

.size	_e_mutex_unlock, .-_e_mutex_unlock


/* ------------------------------------------------------------------------
   End of File
   ------------------------------------------------------------------------ */

