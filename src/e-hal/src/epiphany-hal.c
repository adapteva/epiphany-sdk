/*
  File: epiphany-hal.c

  This file is part of the Epiphany Software Development Kit.

  Copyright (C) 2013 Adapteva, Inc.
  Contributed by Yaniv Sapir <support@adapteva.com>

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

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "epiphany-hal.h"

bool e_is_on_chip(unsigned coreid);

#define diag(vN)   if (e_host_verbose >= vN)

static int e_host_verbose = 0;
static FILE *fd;


/////////////////////////////////
// Device communication functions
//
// Epiphany access
int e_open(Epiphany_t *dev)
{
	uid_t UID;
	int i;

	UID = getuid();
	if (UID != 0)
	{
		warnx("e_open(): Program must be invoked with superuser privilege (sudo).");
		return EPI_ERR;
	}

	// Set device geometry
	e_get_coords_from_id(EPI_BASE_CORE_ID, &(dev->row), &(dev->col));
	dev->rows      = EPI_ROWS;
	dev->cols      = EPI_COLS;
	dev->num_cores = (dev->rows * dev->cols);

	// Open memory device
	dev->memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (dev->memfd == 0)
	{
		warnx("e_open(): /dev/mem file open failure.");
		return EPI_ERR;
	}

	// Map individual cores to virtual memory space
	for (i=0; i<dev->num_cores; i++)
	{
		diag(H_D2) { fprintf(fd, "e_open(): openning core #%d\n", i); }

		e_get_coords_from_id(EPI_BASE_CORE_ID, &(dev->row), &(dev->col));
		dev->core[i].id = e_get_id_from_num(i);
		e_get_coords_from_id(dev->core[i].id, &(dev->core[i].row), &(dev->core[i].col));

		// SRAM array
		dev->core[i].mems.phy_base = (dev->core[i].id << 20 | LOC_BASE_MEMS);
		dev->core[i].mems.map_size = MAP_SIZE_MEMS;
		dev->core[i].mems.map_mask = MAP_MASK_MEMS;

		dev->core[i].mems.mapped_base = mmap(0, dev->core[i].mems.map_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		                                  dev->memfd, dev->core[i].mems.phy_base & ~dev->core[i].mems.map_mask);
		dev->core[i].mems.base = dev->core[i].mems.mapped_base + (dev->core[i].mems.phy_base & dev->core[i].mems.map_mask);


		// e-core regs
		dev->core[i].regs.phy_base = (dev->core[i].id << 20 | LOC_BASE_REGS);
		dev->core[i].regs.map_size = MAP_SIZE_REGS;
		dev->core[i].regs.map_mask = MAP_MASK_REGS;

		dev->core[i].regs.mapped_base = mmap(0, dev->core[i].regs.map_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		                                  dev->memfd, dev->core[i].regs.phy_base & ~dev->core[i].regs.map_mask);
		dev->core[i].regs.base = dev->core[i].regs.mapped_base + (dev->core[i].regs.phy_base & dev->core[i].regs.map_mask);

		if ((dev->core[i].mems.mapped_base == MAP_FAILED))
		{
			warnx("e_open(): ECORE[%d] MEM mmap failure.", i);
			return EPI_ERR;
		}

		if ((dev->core[i].regs.mapped_base == MAP_FAILED))
		{
			warnx("e_open(): ECORE[%d] REGS mmap failure.", i);
			return EPI_ERR;
		}
	}

	// e-sys regs
	dev->esys.phy_base = ESYS_BASE_REGS;
	dev->esys.map_size = 0x1000;
	dev->esys.map_mask = (dev->esys.map_size - 1);

	dev->esys.mapped_base = mmap(0, dev->esys.map_size, PROT_READ|PROT_WRITE, MAP_SHARED,
	                          dev->memfd, dev->esys.phy_base & ~dev->esys.map_mask);
	dev->esys.base = dev->esys.mapped_base + (dev->esys.phy_base & dev->esys.map_mask);

	if ((dev->esys.mapped_base == MAP_FAILED))
	{
		warnx("e_open(): ESYS mmap failure.");
		return EPI_ERR;
	}

	return EPI_OK;
}


int e_close(Epiphany_t *dev)
{
	int i;

	for (i=0; i<dev->num_cores; i++)
	{
		munmap(dev->core[i].mems.mapped_base, dev->core[i].mems.map_size);
		munmap(dev->core[i].regs.mapped_base, dev->core[i].regs.map_size);
	}

	munmap(dev->esys.mapped_base, dev->esys.map_size);

	close(dev->memfd);

	return EPI_OK;
}


ssize_t e_read(Epiphany_t *dev, unsigned corenum, const off_t from_addr, void *buf, size_t count)
{
	ssize_t rcount;

	if (from_addr < dev->core[corenum].mems.map_size)
		rcount = e_read_buf(dev, corenum, from_addr, buf, count);
	else {
		*((unsigned *) (buf)) = e_read_reg(dev, corenum, from_addr);
		rcount = 4;
	}

	return rcount;
}


ssize_t e_write(Epiphany_t *dev, unsigned corenum, off_t to_addr, const void *buf, size_t count)
{
	ssize_t wcount;
	unsigned reg;

	if (to_addr < dev->core[corenum].mems.map_size)
		wcount = e_write_buf(dev, corenum, to_addr, buf, count);
	else {
		reg = *((unsigned *) (buf));
		e_write_reg(dev, corenum, to_addr, reg);
		wcount = 4;
	}

	return wcount;
}


int e_read_word(Epiphany_t *dev, unsigned corenum, const off_t from_addr)
{
	volatile int *pfrom;
	int           data;

	pfrom = (int *) (dev->core[corenum].mems.base + (from_addr & dev->core[corenum].mems.map_mask));
	data  = *pfrom;

	return data;
}


ssize_t e_write_word(Epiphany_t *dev, unsigned corenum, off_t to_addr, int data)
{
	int *pto;

	pto = (int *) (dev->core[corenum].mems.base + (to_addr & dev->core[corenum].mems.map_mask));
	*pto = data;

	return sizeof(int);
}


ssize_t e_read_buf(Epiphany_t *dev, unsigned corenum, const off_t from_addr, void *buf, size_t count)
{
	const void *pfrom;

	pfrom = (dev->core[corenum].mems.base + (from_addr & dev->core[corenum].mems.map_mask));
#ifdef __E64G4_BURST_PATCH__
	int i;

	//	if ((corenum >= 0) && (corenum <= 63))
	if ((corenum >= 8) && (corenum <= 23))
		for (i=0; i<count; i+=sizeof(char))
			*(((char *) buf) + i) = *(((char *) pfrom) + i);
	else
		memcpy(buf, pfrom, count);
#else // __E64G4_BURST_PATCH__
	memcpy(buf, pfrom, count);
#endif // __E64G4_BURST_PATCH__

	return count;
}


ssize_t e_write_buf(Epiphany_t *dev, unsigned corenum, off_t to_addr, const void *buf, size_t count)
{
	void *pto;

	pto = (dev->core[corenum].mems.base + (to_addr & dev->core[corenum].mems.map_mask));
	memcpy(pto, buf, count);

	return count;
}


int e_read_reg(Epiphany_t *dev, unsigned corenum, const off_t from_addr)
{
	volatile int *pfrom;
	int           data;

	pfrom = (int *) (dev->core[corenum].regs.base + (from_addr & dev->core[corenum].regs.map_mask));
	data  = *pfrom;

	return data;
}


ssize_t e_write_reg(Epiphany_t *dev, unsigned corenum, off_t to_addr, int data)
{
	int *pto;

	pto = (int *) (dev->core[corenum].regs.base + (to_addr & dev->core[corenum].regs.map_mask));
	*pto = data;

	return sizeof(int);
}


int e_read_esys(Epiphany_t *dev, const off_t from_addr)
{
	volatile int *pfrom;
	int           data;

	pfrom = (int *) (dev->esys.base + (from_addr & dev->esys.map_mask));
	data  = *pfrom;

	return data;
}


ssize_t e_write_esys(Epiphany_t *dev, off_t to_addr, int data)
{
	int *pto;

	pto = (int *) (dev->esys.base + (to_addr & dev->esys.map_mask));
	*pto = data;

	return sizeof(int);
}





// eDRAM access
int e_alloc(DRAM_t *dram, off_t mbase, size_t msize)
{
	uid_t UID;

	UID = getuid();
	if (UID != 0)
	{
		warnx("e_alloc(): Program must be invoked with superuser privilege (sudo).");
		return EPI_ERR;
	}

	dram->memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (dram->memfd == 0)
	{
		warnx("e_alloc(): /dev/mem file open failure.");
		return EPI_ERR;
	}

	dram->map_size = msize;
	dram->map_mask = msize - 1;

	dram->phy_base = (DRAM_BASE_ADDRESS + mbase);
	dram->mapped_base = mmap(0, dram->map_size, PROT_READ|PROT_WRITE, MAP_SHARED,
	                                  dram->memfd, dram->phy_base & ~dram->map_mask);
	dram->base = dram->mapped_base + (dram->phy_base & dram->map_mask);

	dram->ephy_base = (EPI_EXT_MEM_BASE + mbase);
	dram->emap_size = msize;

	if (dram->mapped_base == MAP_FAILED)
	{
		warnx("e_alloc(): mmap failure.");
		return EPI_ERR;
	}


	return EPI_OK;
}


int e_free(DRAM_t *dram)
{
	munmap(dram->mapped_base, dram->map_size);
	close(dram->memfd);

	return 0;
}


ssize_t e_mread(DRAM_t *dram, const off_t from_addr, void *buf, size_t count)
{
	ssize_t rcount;

	rcount = e_mread_buf(dram, from_addr, buf, count);

	return rcount;
}


ssize_t e_mwrite(DRAM_t *dram, off_t to_addr, const void *buf, size_t count)
{
	ssize_t wcount;

	wcount = e_mwrite_buf(dram, to_addr, buf, count);

	return wcount;
}


int e_mread_word(DRAM_t *dram, const off_t from_addr)
{
	volatile int *pfrom;
	int           data;

	pfrom = (int *) (dram->base + (from_addr & dram->map_mask));
	data  = *pfrom;

	return data;
}


ssize_t e_mwrite_word(DRAM_t *dram, off_t to_addr, int data)
{
	int *pto;

	pto = (int *) (dram->base + (to_addr & dram->map_mask));
	*pto = data;

	return sizeof(int);
}


ssize_t e_mread_buf(DRAM_t *dram, const off_t from_addr, void *buf, size_t count)
{
	const void *pfrom;

	pfrom = (dram->base + (from_addr & dram->map_mask));
	memcpy(buf, pfrom, count);

	return count;
}


ssize_t e_mwrite_buf(DRAM_t *dram, off_t to_addr, const void *buf, size_t count)
{
	void *pto;

	pto = (dram->base + (to_addr & dram->map_mask));
	memcpy(pto, buf, count);

	return count;
}





/////////////////////////
// Core control functions
int e_reset_core(Epiphany_t *pEpiphany, unsigned corenum)
{
	int RESET0 = 0x0;
	int RESET1 = 0x1;

	diag(H_D1) { fprintf(fd, "   Resetting core %d (0x%03x)...", corenum, pEpiphany->core[corenum].id); }
	e_write_reg(pEpiphany, corenum, EPI_CORE_RESET, RESET1);
	e_write_reg(pEpiphany, corenum, EPI_CORE_RESET, RESET0);
	diag(H_D1) { fprintf(fd, " Done.\n"); }

	return EPI_OK;
}


int e_reset_esys(Epiphany_t *pEpiphany)
{
#if 0
	Epiphany_t Epi;
	Epiphany_t *dev;

	int data;
	int *pto;

	uid_t UID;

	dev = &Epi;

	UID = getuid();
	if (UID != 0)
	{
		warnx("e_reset_esys(): Program must be invoked with superuser privilege (sudo).");
		return EPI_ERR;
	}

	dev->memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (dev->memfd == 0)
	{
		warnx("ESYS: /dev/mem file open failure.");
		return EPI_ERR;
	}

	// e-sys regs
	dev->esys.phy_base = ESYS_BASE_REGS;
	dev->esys.map_size = 0x1000; //MAP_SIZE_REGS;
	dev->esys.map_mask = (dev->esys.map_size - 1);

	dev->esys.mapped_base = mmap(0, dev->esys.map_size, PROT_READ|PROT_WRITE, MAP_SHARED,
	                          dev->memfd, dev->esys.phy_base & ~dev->esys.map_mask);
	dev->esys.base = dev->esys.mapped_base + (dev->esys.phy_base & dev->esys.map_mask);

	if ((dev->esys.mapped_base == MAP_FAILED))
	{
		warnx("ESYS: mmap failure.");
		return EPI_ERR;
	}

	data = 0;
	pto = (int *) (dev->esys.base + ESYS_RESET);
	*pto = data;

	munmap(dev->esys.mapped_base, dev->esys.map_size);

	close(dev->memfd);
#elif 1
	diag(H_D1) { fprintf(fd, "   Resetting ESYS..."); fflush(stdout); }
	e_write_esys(pEpiphany, ESYS_RESET, 0);
	sleep(1);
	diag(H_D1) { fprintf(fd, " Done.\n"); }
#else
	int corenum;

	for (corenum=0; corenum<pEpiphany->num_cores; corenum++) {
		e_reset_core(pEpiphany, corenum);
	}
#endif

	return EPI_OK;
}


int e_reset(Epiphany_t *pEpiphany, e_resetid_t resetid)
{
	int corenum;

	if (resetid == E_RESET_CORES) {
		diag(H_D1) { fprintf(fd, "   Resetting all cores..."); fflush(stdout); }
		for (corenum=0; corenum<pEpiphany->num_cores; corenum++) {
			e_reset_core(pEpiphany, corenum);
		}
	} else if (resetid == E_RESET_CHIP) {
		diag(H_D1) { fprintf(fd, "   Resetting chip..."); fflush(stdout); }
		errx(3, "\nEXITTING\n");
	} else if (resetid == E_RESET_ESYS) {
		diag(H_D1) { fprintf(fd, "   Resetting full ESYS..."); fflush(stdout); }
		e_reset_esys(pEpiphany);
	} else {
		diag(H_D1) { fprintf(fd, "   Invalid RESET ID!\n"); fflush(stdout); }
		return EPI_ERR;
	}
	diag(H_D1) { fprintf(fd, " Done.\n"); fflush(stdout); }

	return EPI_OK;
}


int e_start(Epiphany_t *pEpiphany, unsigned coreid)
{
	int corenum;
	int SYNC = 0x1;
	int *pILAT;

	corenum = e_get_num_from_id(coreid);
	pILAT = (int *) ((char *) pEpiphany->core[corenum].regs.base + EPI_ILAT);
	diag(H_D1) { fprintf(fd, "   SYNC (0x%x) to core %d...", (unsigned) pILAT, corenum); fflush(stdout); }
	*pILAT = (*pILAT) | SYNC;
	diag(H_D1) { fprintf(fd, " Done.\n"); }

	return EPI_OK;
}





////////////////////
// Utility functions
unsigned e_get_num_from_coords(unsigned row, unsigned col)
{
	unsigned corenum;

	corenum = (col & (EPI_COLS-1)) + ((row & (EPI_ROWS-1)) * EPI_COLS);

	return corenum;
}


unsigned e_get_num_from_id(unsigned coreid)
{
	unsigned corenum;

	corenum = (coreid & (EPI_COLS-1)) + (((coreid >> 6) & (EPI_ROWS-1)) * EPI_COLS);

	return corenum;
}


unsigned e_get_id_from_coords(unsigned row, unsigned col)
{
	unsigned coreid;

	coreid = (EPI_BASE_CORE_ID + (col & (EPI_COLS-1))) + ((row & (EPI_ROWS-1)) << 6);

	return coreid;
}


unsigned e_get_id_from_num(unsigned corenum)
{
	int coreid;

	coreid = EPI_BASE_CORE_ID + (corenum & (EPI_COLS-1)) + (((corenum / EPI_COLS) & (EPI_ROWS-1)) << 6);

	return coreid;
}


void e_get_coords_from_id(unsigned coreid, unsigned *row, unsigned *col)
{
	// TODO these are the absolute coords. Do we need relative ones?
	*row = (coreid >> 6) & 0x3f;
	*col = (coreid >> 0) & 0x3f;

	return;
}


void e_get_coords_from_num(unsigned corenum, unsigned *row, unsigned *col)
{
	// TODO this gives the *relative* coords in a *16-core* chip!
	*row = (corenum / EPI_COLS) & (EPI_ROWS-1);
	*col = (corenum >> 0)       & (EPI_COLS-1);

	return;
}


bool e_is_on_chip(unsigned coreid)
{
	unsigned erow, ecol;
	unsigned row, col;

	e_get_coords_from_id(EPI_BASE_CORE_ID, &erow, &ecol);
	e_get_coords_from_id(coreid, &row, &col);

	if ((row >= erow) && (row < (erow + EPI_ROWS)) && (col >= ecol) && (col < (ecol + EPI_COLS)))
		return true;
	else
		return false;
}


void e_set_host_verbosity(e_hal_diag_t verbose)
{
	fd = stderr;
	e_host_verbose = verbose;

	return;
}




////////////////////////////////////
// ftdi_target wrapper functionality
#include <e-xml/src/epiphany_platform.h>

Epiphany_t Epiphany, *pEpiphany;
DRAM_t     ERAM,     *pERAM;

platform_definition_t* platform;


// Global memory access
ssize_t e_read_abs(unsigned address, void* buf, size_t burst_size)
{
	ssize_t  rcount;
	unsigned isglobal, isexternal, isonchip, isregs, ismems;
	unsigned corenum, coreid;
	unsigned row, col, i;

	diag(H_D1) { fprintf(fd, "e_read_abs(): address = 0x%08x\n", address); }
	isglobal = (address & 0xfff00000) != 0;
	if (isglobal)
	{
		if (((address >= pERAM->phy_base)  && (address < (pERAM->phy_base + pERAM->map_size))) ||
		    ((address >= pERAM->ephy_base) && (address < (pERAM->ephy_base + pERAM->emap_size))))
		{
			isexternal = true;
		} else {
			isexternal = false;
			coreid     = address >> 20;
			isonchip   = e_is_on_chip(coreid);
			if (isonchip)
			{
				e_get_coords_from_id(coreid, &row, &col);
				corenum = e_get_num_from_coords(row, col);
				ismems  = (address <  pEpiphany->core[corenum].mems.phy_base + pEpiphany->core[corenum].mems.map_size);
				isregs  = (address >= pEpiphany->core[corenum].regs.phy_base);
			}
		}
	}

	if (isglobal)
	{
		if (isexternal)
		{
			rcount = e_mread_buf(pERAM, address, buf, burst_size);
			diag(H_D1) { fprintf(fd, "e_read_abs(): isexternal -> rcount = %d\n", (int) rcount); }
		} else if (isonchip)
		{
			if (ismems) {
				rcount = e_read_buf(pEpiphany, corenum, address, buf, burst_size);
				diag(H_D1) { fprintf(fd, "e_read_abs(): isonchip/ismems -> rcount = %d\n", (int) rcount); }
			} else if (isregs) {
				for (rcount=0, i=0; i<burst_size; i+=sizeof(unsigned)) {
					*((unsigned *) (buf+i)) = e_read_reg(pEpiphany, corenum, (address+i));
					rcount += sizeof(unsigned);
				}
				diag(H_D1) { fprintf(fd, "e_read_abs(): isonchip/isregs -> rcount = %d\n", (int) rcount); }
			} else {
				rcount = 0;
				diag(H_D1) { fprintf(fd, "e_read_abs(): is a reserved on-chip address -> rcount = %d\n", (int) rcount); }
			}
		} else {
			rcount = 0;
			diag(H_D1) { fprintf(fd, "e_read_abs(): is not a legal address -> rcount = %d\n", (int) rcount); }
		}
	} else {
		rcount = 0;
		diag(H_D1) { fprintf(fd, "e_read_abs(): is not a global address -> rcount = %d\n", (int) rcount); }
	}

	return rcount;
}


ssize_t e_write_abs(unsigned address, void *buf, size_t burst_size)
{
	ssize_t  rcount;
	unsigned isglobal, isexternal, isonchip, isregs, ismems;
	unsigned corenum, coreid;
	unsigned row, col, i;

	diag(H_D1) { fprintf(fd, "e_write_abs(): address = 0x%08x\n", address); }
	isglobal = (address & 0xfff00000) != 0;
	if (isglobal)
	{
		if (((address >= pERAM->phy_base)  && (address < (pERAM->phy_base + pERAM->map_size))) ||
		    ((address >= pERAM->ephy_base) && (address < (pERAM->ephy_base + pERAM->emap_size))))
		{
			isexternal = true;
		} else {
			isexternal = false;
			coreid     = address >> 20;
			isonchip   = e_is_on_chip(coreid);
			if (isonchip)
			{
				e_get_coords_from_id(coreid, &row, &col);
				corenum = e_get_num_from_coords(row, col);
				ismems  = (address <  pEpiphany->core[corenum].mems.phy_base + pEpiphany->core[corenum].mems.map_size);
				isregs  = (address >= pEpiphany->core[corenum].regs.phy_base);
			}
		}
	}

	if (isglobal)
	{
		if (isexternal)
		{
			rcount = e_mwrite_buf(pERAM, address, buf, burst_size);
			diag(H_D1) { fprintf(fd, "e_write_abs(): isexternal -> rcount = %d\n", (int) rcount); }
		} else if (isonchip)
		{
			if (ismems) {
				rcount = e_write_buf(pEpiphany, corenum, address, buf, burst_size);
				diag(H_D1) { fprintf(fd, "e_write_abs(): isonchip/ismems -> rcount = %d\n", (int) rcount); }
			} else if (isregs) {
				for (rcount=0, i=0; i<burst_size; i+=sizeof(unsigned)) {
					rcount += e_write_reg(pEpiphany, corenum, address, *((unsigned *)(buf+i)));
				}
				diag(H_D1) { fprintf(fd, "e_write_abs(): isonchip/isregs -> rcount = %d\n", (int) rcount); }
			} else {
				rcount = 0;
				diag(H_D1) { fprintf(fd, "e_write_abs(): is a reserved on-chip address -> rcount = %d\n", (int) rcount); }
			}
		} else {
			rcount = 0;
			diag(H_D1) { fprintf(fd, "e_write_abs(): is not a legal address -> rcount = %d\n", (int) rcount); }
		}
	} else {
		rcount = 0;
		diag(H_D1) { fprintf(fd, "e_write_abs(): is not a global address -> rcount = %d\n", (int) rcount); }
	}

	return rcount;
}



int init_platform(platform_definition_t* platform_arg, unsigned verbose_mode)
{
	int res;

	pEpiphany = &Epiphany;
	pERAM     = &ERAM;
	platform  = platform_arg;

	e_set_host_verbosity(verbose_mode);
	res = e_alloc(pERAM, 0, DRAM_SIZE);
	res = e_open(pEpiphany);

	return res;
}



int close_platform()
{
	int res;

	res = e_close(pEpiphany);
	res = e_free(pERAM);

	return res;
}



int write_to(unsigned address, void *buf, size_t burst_size)
{
	// The readMem() function which calls read_from() driver function always calls with a global address.
	// need to check which region is being called and use the appropriate e-host API.

	ssize_t rcount;

	rcount = e_write_abs(address, buf, burst_size);

	return rcount;
}



int read_from(unsigned address, void* buf, size_t burst_size)
{
	// The readMem() function which calls read_from() driver function always calls with a global address.
	// need to check which region is being called and use the appropriate e-host API.

	ssize_t rcount;

	rcount = e_read_abs(address, buf, burst_size);

	return rcount;
}



int hw_reset()
{
	int sleepTime = 0;

	e_reset(pEpiphany, E_RESET_ESYS);

	sleep(sleepTime);

	return 0;
}


char TargetId[] = "E16G3 based Parallella";
int get_description(char** targetIdp)
{
	*targetIdp = platform->name;

	return 0;
}
