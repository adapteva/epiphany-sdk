/*
  File: e_ctimer_start.s

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

.file    "e_ctimer_start.s";

// ------------------------------------------------------------------------
.section .text;
.type    _e_ctimer_start, %function;
.global  _e_ctimer_start;

.balign 4;
_e_ctimer_start:

        and   r0, r0, r0;                    // set the status to check which timer register
    //----
        bne   _ctimer1_start;                // jump to code for timer1


.balign 4;
_ctimer0_start:

        movfs r3, config;                    // get the current config setting
    //----
        mov   r0, %low(0xffffff0f);          // load mask for the config register
    //----
        movt  r0, %high(0xffffff0f);         // load mask for the config register
    //----
        lsl   r1, r1, 0x4;                   // the ctimer0 control bits start at bit4
    //----
        and   r3, r3, r0;                    // apply the mask to clear TIMERMODE bits from previous config setting
    //----
        movts config, r3;                    // turn the ctimer0 off
    //----
        orr   r3, r3, r1;                    // add the desired TIMERMODE to config
    //----
        movts config, r3;                    // start the ctimer counter
    //----
        movfs r0, ctimer0;                   // read the ctimer value
    //----
        rts;                                 // return with the current value of the ctimer


.balign 4;
_ctimer1_start:

        movfs r3, config;                    // get the current config setting
    //----
        mov   r0, %low(0xfffff0ff);          // load mask for the config register
    //----
        movt  r0, %high(0xfffff0ff);         // load mask for the config register
    //----
        lsl   r1, r1, 0x8;                   // the ctimer1 control bits start at bit8
    //----
        and   r3, r3, r0;                    // apply the mask to clear TIMERMODE bits from previous config setting
    //----
        movts config, r3;                    // turn the ctimer1 off
    //----
        orr   r3, r3, r1;                    // add the desired TIMERMODE to config
    //----
        movts config, r3;                    // start the ctimer counter
    //----
        movfs r0, ctimer1;                   // read the ctimer value
    //----
        rts;                                 // return with the current value of the ctimer


.size    _e_ctimer_start, .-_e_ctimer_start;


/* ------------------------------------------------------------------------
   End of File
   ------------------------------------------------------------------------ */

