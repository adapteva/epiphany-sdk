/*
  File: e-loader.c

  This file is part of the Epiphany Software Development Kit.

  Copyright (C) 2013 Adapteva, Inc.
  Contributed by Oleg Raikhman, Yaniv Sapir <support@adapteva.com>

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

#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <err.h>

#include "e-hal.h"
#include "e-loader.h"

#define diag(vN)   if (e_load_verbose >= vN)

int e_load_verbose = 0;
FILE *fd;

unsigned int DiscoveredCoreIDs[EPI_CORES];
unsigned int DiscoveredCoreIDidx;

int e_load(char *srecFile, bool reset_target, bool broadcast, bool run_target)
{
	Epiphany_t Epiphany;
	Epiphany_t *pEpiphany;
	int status;
	unsigned coreNum;
	
	DiscoveredCoreIDidx = 0;
	status = EPI_OK;
	
	pEpiphany = &Epiphany;

	if (pEpiphany) {
		if (e_open(pEpiphany))
		{
			fprintf(fd, "\nERROR: Can't establish connection to Epiphany device!\n\n");
			exit(1);
		}

		if (reset_target) {
			diag(L_D1) { fprintf(fd, "Send RESET signal to the Epiphany system..."); diag(L_D2) fprintf(fd, "\n"); else fprintf(fd, " "); }
			e_reset(pEpiphany, E_RESET_ESYS);
			diag(L_D1) { fprintf(fd, "Done.\n"); fflush(stdout); }
		}

		if (srecFile[0] != '\0') {
			diag(L_D1) { fprintf(fd, "Loading SREC file %s ...\n", srecFile); }

			if (parseAndSendSrecFile(srecFile, pEpiphany, broadcast) == EPI_ERR) {
				fprintf(fd, "ERROR: Can't parse SREC file.\n");
				e_close(pEpiphany);
				return EPI_ERR;
			}

			if (run_target) {
				for (coreNum=0; coreNum<DiscoveredCoreIDidx; coreNum++) {
					diag(L_D1) { fprintf(fd, "Send SYNC signal to core ID 0x%03x...", DiscoveredCoreIDs[coreNum]); diag(L_D2) fprintf(fd, "\n"); else fprintf(fd, " "); }
					e_start(pEpiphany, DiscoveredCoreIDs[coreNum]);
					diag(L_D1) { fprintf(fd, "Done.\n"); }
				}
			}

			diag(L_D1) { fprintf(fd, "Done loading.\n"); }
		}

		e_close(pEpiphany);
		diag(L_D1) { fprintf(fd, "Closed connection.\n"); }
	} else {
		fprintf(fd, "ERROR: Can't connect to Epiphany.\n");
		return EPI_ERR;
	}

	diag(L_D1) { fprintf(fd, "Leaving loader.\n"); }

	return status;
}


void e_set_loader_verbosity(e_loader_diag_t verbose)
{
	fd = stderr;
	e_load_verbose = verbose;
	e_set_host_verbosity(verbose);

	return;
}

