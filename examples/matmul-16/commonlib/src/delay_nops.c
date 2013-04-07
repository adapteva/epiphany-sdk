/*
  delay_nops.c

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



//__asm__ __volatile__ ("ldrb %[xb], [%[xa]]" : [xb] "=r" (sentinel): [xa] "r" (last_addr)); // sentinel = *last_addr;

// 1-16
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");


// 17-32
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");


// 33-48
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");

__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
__asm__ __volatile__ ("nop");
