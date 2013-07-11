/*
  Copyright (C) 2013 Adapteva, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <e-hal.h>

void print_mbox(e_epiphany_t *dev, e_mem_t *emem, char *msg);

#define _BufOffset (0x01000000)

unsigned M[5];
unsigned IVT[10];

int main(int argc, char *argv[])
{
	unsigned row, col, coreid;
	unsigned di, ci, go, go_all;
	e_platform_t platform;
	e_epiphany_t dev;
	e_mem_t      emem;

	// Initialize progress state in mailbox
	M[0]  = 0xdeadbeef;
	M[1]  = M[2] = M[3] = M[4] = M[0];

	// initialize system, read platform params from
	// default HDF. Then, reset the platform.
	e_init(NULL);
	e_reset_system();

	// Open the first and second cores for master and slave programs, resp.
	e_open(&dev, 0, 0, 1, 2);
	
	// Allocate the ext. mem. mailbox
	e_alloc(&emem, _BufOffset, sizeof(M));

	// Load programs on cores.
	e_load("e-int-test.master.srec", &dev, 0, 0, E_FALSE);
	e_load("e-int-test.slave.srec",  &dev, 0, 1, E_FALSE);

	// clear mailbox.
	e_write(&emem, 0, 0, (off_t) (0x0000), (void *) &(M[0]), sizeof(M));

	// Print mbox status.
	print_mbox(&dev, &emem, "1. Clearing mbox:");
	
	// start the master program
	e_start(&dev, 0, 0);
	
	usleep(500e3);
	print_mbox(&dev, &emem, "2. Master started:");
	
	usleep(1e6);
	print_mbox(&dev, &emem, "3. Slave started:");

	// At this point, the  mailbox should contain all of the progress
	// indicators, and look like the following:
	//
	// 0x808       0x809       0x22222222  0x33333333  0x44444444
	//
	// If there is a "0xdeadbeef" state in one of the slots, it means
	// that something went wrong.

	// Finalize
	e_close(&dev);
	e_free(&emem);
	e_finalize();

	return 0;
}


void print_mbox(e_epiphany_t *dev, e_mem_t *emem, char *msg)
{
	e_read(emem, 0, 0, (off_t) (0x0000), (void *) &(M[0]), sizeof(M));

	printf("%-20s 0x%-8x  0x%-8x  0x%-8x  0x%-8x  0x%-8x\n", msg, M[0], M[1], M[2], M[3], M[4]);

	return;
}

