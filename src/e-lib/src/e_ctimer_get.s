/*
  File: ctimer_get.s

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

.file    "e_ctimer_get.s";


// ------------------------------------------------------------------------
.section .text;
.type    _e_ctimer_get, %function;
.global  _e_ctimer_get;

.balign 4;
_e_ctimer_get:

        and   r0, r0, r0;                    // set the status to check which timer register
    //----
        bne   _ctimer1_get;                  // jump to code for timer1


.balign 4;
_ctimer0_get:

        movfs r0, ctimer0;                   // read the ctimer value
    //----
        rts;                                 // return with the current value of the ctimer


.balign 4;
_ctimer1_get:

        movfs r0, ctimer1;                   // read the ctimer value
    //----
        rts;                                 // return with the current value of the ctimer

.size    _e_ctimer_get, .-_e_ctimer_get;


/* ------------------------------------------------------------------------
   End of File
   ------------------------------------------------------------------------ */

