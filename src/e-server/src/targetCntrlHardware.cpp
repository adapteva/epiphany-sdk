/*
  File: targetCntrlHardware.cpp

  This file is part of the Epiphany Software Development Kit.

  Copyright (C) 2013 Adapteva, Inc.
  Contributed by Oleg Raikhman, Yaniv Sapir <support@adapteva.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING).  If not, see
  <http://www.gnu.org/licenses/>.
*/


#include "target_param.h"

#include "targetCntrlHardware.h"
#include "debugVerbose.h"
#include <string>
#include <e-xml/src/epiphany_platform.h>
#include <e-hal/src/epiphany-hal-defs.h>
#include <e-hal/src/epiphany-hal-data.h>

enum
{
	_BYTE_   = 1,
	_SHORT_  = 2,
	_WORD_   = 4,
	_DOUBLE_ = 8,
};

#include <map>
#include <utility>

using namespace std;

Epiphany_t Epiphany, *pEpiphany;
DRAM_t     pDRAM;

int (*init_platform)(platform_definition_t *platform, unsigned verbose);
int (*close_platform)();
int (*write_to)(unsigned address, void *buf, size_t busrt_size);
int (*read_from)(unsigned address, void *data, size_t busrt_size);
int (*hw_reset)();

///int (*init_platform)(platform_definition_t *platform, unsigned verbose);
///int (*close_platform)();
///int (*write_to)(unsigned address, void *buf, size_t busrt_size);
///int (*read_from)(unsigned address, void *data, size_t busrt_size);
int (*get_description)(char **);
int (*get_memory_map)(unsigned long *);
int (*get_number_supported_cores)(unsigned long *);
int (*set_chip_x_y)(unsigned x, unsigned y);
///int (*hw_reset)();
///int (*use_spi)();
///int (*use_link)();

int     (*e_open)(Epiphany_t *dev);
int     (*e_close)(Epiphany_t *dev);
ssize_t (*e_read)(Epiphany_t *dev, int corenum, const off_t from_addr, void *buf, size_t count);
ssize_t (*e_write)(Epiphany_t *dev, int corenum, off_t to_addr, const void *buf, size_t count);
int     (*e_reset)(Epiphany_t *pEpiphany, e_resetid_t resetid);
void    (*e_set_host_verbosity)(int verbose);


#include <string.h>
#include <iostream>
#include <map>
#include <utility>

#include <stdlib.h>
#include <dlfcn.h>

#include <signal.h>
#include <cassert>

using namespace std;

/* common data base for ALL threads */
static void *handle = 0;
map<unsigned, pair<unsigned long, unsigned long> > memory_map;
map<unsigned, pair<unsigned long, unsigned long> > register_map;

extern int debug_level;

#define CORE_SPACE 0x00100000


inline unsigned IS_ADDRESS_INTERNAL(unsigned addr)
{
	return addr < CORE_SPACE;
}


pthread_mutex_t targetControlHWAccess_m = PTHREAD_MUTEX_INITIALIZER;


void TargetControlHardware::PlatformReset()
{
	pthread_mutex_lock(&targetControlHWAccess_m);

	hw_reset(); // ESYS_RESET

	pthread_mutex_unlock(&targetControlHWAccess_m);
}


TargetControlHardware::TargetControlHardware(unsigned indexInMemMap)
{
	// indexInMemMap is essentially the core number or ext_mem segment number
	// FIXME
	assert(indexInMemMap < memory_map.size());

	fAttachedCoreId = (memory_map[indexInMemMap].first) >> 20;
}


bool TargetControlHardware::SetAttachedCoreId(unsigned coreId)
{
	// check if coreId is a valid core for this device by iterating over the memory map. if it is, set the fAttachedCoreId.
	for (map<unsigned, pair<unsigned long, unsigned long> >::iterator ii=memory_map.begin(); ii!=memory_map.end(); ++ii)
	{
		unsigned long startAddr = (*ii).second.first;

		if ((coreId << 20) == startAddr)
		{
			fAttachedCoreId = coreId;
			return true;
		}
	}

	return false;
}


// Convert an internal address to a global one.
unsigned long TargetControlHardware::ConvertAddress(unsigned long address)
{
	assert(handle);

	if (IS_ADDRESS_INTERNAL(address))
	{
		return (fAttachedCoreId << 20) + address;
	}

	for (map<unsigned, pair<unsigned long, unsigned long> >::iterator ii=memory_map.begin(); ii!=memory_map.end(); ++ii)
	{
		unsigned long startAddr = (*ii).second.first;
		unsigned long endAddr   = (*ii).second.second;

		if ((address >= startAddr) && (address <= endAddr))
		{
			// found
			return address;
		}
	}

	for (map<unsigned, pair<unsigned long, unsigned long> >::iterator ii=register_map.begin(); ii!=register_map.end(); ++ii)
	{
		unsigned long startAddr = (*ii).second.first;
		unsigned long endAddr   = (*ii).second.second;

		if ((address >= startAddr) && (address <= endAddr))
		{
			// found
			return address;
		}
	}

	return 0;
}


extern bool dont_check_hw_address;

bool TargetControlHardware::readMem (uint32_t addr, sc_uint_32 &data, unsigned burst_size)
{
	bool retSt = false;

	//struct timeval start_t;
	// StartOfBaudMeasurement(start_t);

	pthread_mutex_lock(&targetControlHWAccess_m);

	uint32_t fullAddr = ConvertAddress(addr);
	// bool iSAligned = (fullAddr == ());
	if (fullAddr || dont_check_hw_address)
	{
		// supported only word size or smaller
		assert(burst_size <= 4);
		char buf[8];
		unsigned int res = (*read_from)(fullAddr, (void *) buf, burst_size);

		if (res != burst_size)
		{
			cerr << "ERROR (" << res << "): mem read failed for addr " << hex << addr << dec << endl;
		} else {
			// pack returned data
			for (unsigned i=0; i<burst_size; i++)
			{
				data = (data & (~(0xff << (i*8)))) | (buf[i] << (i*8));
			}
			if (debug_level > D_TARGET_WR)
			{
				cerr << "TARGET READ (" << burst_size << ") " << hex << fullAddr << " >> " << data << dec << endl;
			}
		}
		retSt = (res == burst_size);
	} else {
		cerr << "WARNING (READ_MEM ignored): The address " << hex << addr << " is not in the valid range for target "
		     << this->GetTargetId() << dec << endl;
	}

	// double mes = EndOfBaudMeasurement(start_t);
	// cerr << "--- READ milliseconds: " << mes << endl;

	pthread_mutex_unlock(&targetControlHWAccess_m);

	return retSt;
}


bool TargetControlHardware::writeMem (uint32_t addr, sc_uint_32 data, unsigned burst_size)
{
	bool retSt = false;
	pthread_mutex_lock(&targetControlHWAccess_m);
	uint32_t fullAddr = ConvertAddress(addr);

	// bool iSAligned = (fullAddr == ());
	if (fullAddr || dont_check_hw_address)
	{
		assert(burst_size <= 4);
		char buf[8];

		for (unsigned i = 0; i < burst_size; i++)
		{
			buf[i] = (data >> (i*8)) & 0xff;
		}

		// struct timeval start_t;
		// StartOfBaudMeasurement(start_t);

		unsigned int res = (*write_to)(fullAddr, (void *) buf, burst_size);

		// double mes = EndOfBaudMeasurement(start_t);
		// cerr << "--- WRITE (writeMem)(" << burst_size << ") milliseconds: " << mes << endl;

		if (debug_level > D_TARGET_WR)
		{
			cerr << "TARGET WRITE (" << burst_size << ") " << hex << fullAddr << " >> " << data << dec << endl;
		}

		if (res != burst_size)
		{
			cerr << "ERROR (" << res << "): mem write failed for addr " << hex << fullAddr << dec << endl;
		}

		retSt = (res == burst_size);
	} else {
		cerr << "WARNING (WRITE_MEM ignored): The address " << hex << addr << " is not in the valid range for target "
		     << this->GetTargetId() << dec << endl;
	}

	pthread_mutex_unlock(&targetControlHWAccess_m);

	return retSt;
}


bool TargetControlHardware::readMem32 (uint32_t addr, uint32_t &data)
{
	bool retSt = false;
	sc_uint_32 data32;

	retSt = readMem(addr, data32, 4);
	data = data32;

	return retSt;
}


bool TargetControlHardware::readMem16 (uint32_t addr, uint16_t &data)
{
	bool retSt = false;
	sc_uint_32 data32;

	retSt = readMem(addr, data32, 2);
	data = data32 & 0x0000ffff;

	return retSt;
}


bool TargetControlHardware::readMem8 (uint32_t addr, uint8_t &data)
{
	bool retSt = false;
	sc_uint_32 data32;

	retSt = readMem(addr, data32, 1);
	data = data32 & 0x000000ff;

	return retSt;
}


bool TargetControlHardware::writeMem32 (uint32_t addr, uint32_t value)
{
	bool retSt = false;
	sc_uint_32 data32 = value;

	retSt = writeMem(addr, data32, 4);

	return retSt;
}


bool TargetControlHardware::writeMem16 (uint32_t addr, uint16_t value)
{
	bool retSt = false;
	sc_uint_32 data32 = 0;

	data32 = value & 0x0000ffff;
	retSt = writeMem(addr, data32, 2);

	return retSt;
}


bool TargetControlHardware::writeMem8 (uint32_t addr, uint8_t value)
{
	bool retSt = false;
	sc_uint_32 data32 = 0;

	data32 = value & 0x000000ff;
	retSt = writeMem(addr, data32, 1);

	return retSt;
}


// see target driver

// burst read
bool TargetControlHardware::ReadBurst(unsigned long addr, unsigned char *buf, size_t buff_size)
{
	bool ret = true;

	uint32_t fullAddr = ConvertAddress(addr);

	// cerr << "READ burst " << hex << fullAddr << " Size " << dec << buff_size << endl;

	if (fullAddr || dont_check_hw_address)
	{
		if ((fullAddr % _WORD_) == 0)
		{
			pthread_mutex_lock(&targetControlHWAccess_m);

			// struct timeval start_time;
			// StartOfBaudMeasurement(start_time);
			for (unsigned k=0; k<buff_size/(MAX_NUM_READ_PACKETS*_WORD_); k++)
			{
				int res = (*read_from)(fullAddr + k * MAX_NUM_READ_PACKETS * _WORD_, (void *) (buf + k * MAX_NUM_READ_PACKETS * _WORD_), (MAX_NUM_READ_PACKETS * _WORD_));

				if (res != (MAX_NUM_READ_PACKETS * _WORD_))
				{
					cerr << "ERROR (" << res << "): memory read failed for full address " << hex << fullAddr << dec << endl;
					ret = false;
				}
			}

			unsigned trailSize = (buff_size % (MAX_NUM_READ_PACKETS * _WORD_));
			if (trailSize != 0)
			{
				unsigned int res = (*read_from) (fullAddr + buff_size - trailSize, (void *) (buf + buff_size - trailSize), trailSize);

				if (res != trailSize)
				{
					cerr << "ERROR (" << res << "): memory read failed for full address " << hex << fullAddr << dec << endl;
					ret = false;
				}
			}

			// cerr << " done nbytesToSend " << buff_size << " msec " << EndOfBaudMeasurement(start_time) << endl;
			pthread_mutex_unlock(&targetControlHWAccess_m);
		} else {
			for (unsigned i = 0; i < buff_size; i++)
			{
				ret = ret && readMem8(fullAddr+i, buf[i]);
			}
		}
	} else {
		cerr << "WARNING (READ_BURST ignored): The address " << hex << addr << " is not in the valid range for target "
		     << this->GetTargetId() << dec << endl;
		ret = false;
	}

	return ret;
}


// burst write
bool TargetControlHardware::WriteBurst(unsigned long addr, unsigned char *buf, size_t buff_size)
{
	bool ret = true;

	if (buff_size == 0) return true;

	pthread_mutex_lock(&targetControlHWAccess_m);

	uint32_t fullAddr = ConvertAddress(addr);

	// cerr << "---Write burst " << hex << fullAddr << " Size " << dec << buff_size << endl;

	assert(buff_size > 0);

	if (fullAddr || dont_check_hw_address)
	{
		if ((buff_size == _WORD_) && ((fullAddr % _WORD_) == 0))
		{
			// register access -- should be word transaction
			// cerr << "---Write WORD " << hex << fullAddr << " Size " << dec << _WORD_ << endl;
			int res = (*write_to)(fullAddr, (void *) (buf), _WORD_);
			if (res != _WORD_)
			{
				cerr << "ERROR (" << res << "): mem write failed for full address " << hex << fullAddr << dec << endl;
				ret = false;
			}
		} else {
			// head up to double boundary
			if ((fullAddr % _DOUBLE_) != 0)
			{
				unsigned int headSize = _DOUBLE_ - (fullAddr % _DOUBLE_);

				if (headSize > buff_size)
				{
					headSize = buff_size;
				}

				// head
				for (unsigned n=0; n<headSize; n++)
				{
					// cerr << "head fullAddr " << hex << fullAddr << " size " << 1 << endl;
					int res = (*write_to)(fullAddr, (void *) (buf), 1);
					if (res != 1)
					{
						cerr << "ERROR (" << res << "): mem write failed for full address " << hex << fullAddr << dec << endl;
						ret = false;
					}
					buf += 1;
					fullAddr += 1;
					buff_size = buff_size - 1;
				}
			}

			assert(buff_size == 0 || (fullAddr % _DOUBLE_) == 0);
			assert((MAX_NUM_WRITE_PACKETS % _DOUBLE_) == 0);
			size_t numMaxBurst = buff_size / (MAX_NUM_WRITE_PACKETS * _DOUBLE_);

			for (unsigned k=0; k<numMaxBurst; k++)
			{
				unsigned cBufSize = (MAX_NUM_WRITE_PACKETS * _DOUBLE_);

				// cerr << "BIG DOUBLE BURST " << k << " fullAddr " << hex << fullAddr << " size " << cBufSize << endl;

				size_t res = (*write_to)(fullAddr, (void *) (buf), cBufSize);
				if (res != cBufSize)
				{
					cerr << "ERROR (" << res << "): mem write failed for full address " << hex << fullAddr << dec << endl;
					ret = false;
				}
				fullAddr += cBufSize;
				buf += cBufSize;
				buff_size = buff_size - cBufSize;
			}

			size_t trailSize = buff_size%_DOUBLE_;
			if (buff_size > trailSize)
			{
				// cerr << "LAST DOUBLE BURST " << " fullAddr " << hex << fullAddr << " size " << buff_size-trailSize << endl;
				size_t res = (*write_to)(fullAddr, (void *) (buf), buff_size-trailSize);
				if (res != buff_size-trailSize)
				{
					cerr << "ERROR (" << res << "): mem write failed for full address " << hex << fullAddr << dec << endl;
					ret = false;
				}
				fullAddr += buff_size - trailSize;
				buf += buff_size - trailSize;
			}

			// trail
			if (trailSize > 0)
			{
				for (unsigned n=0; n<trailSize; n++)
				{
					// cerr << "TRAIL " << " fullAddr " << hex << fullAddr << " size " << 1 << endl;
					int res = (*write_to)(fullAddr, (void *) (buf), 1);
					if (res != 1)
					{
						cerr << "ERROR (" << res << "): mem write failed for full address " << hex << fullAddr << dec << endl;
						ret = false;
					}

					buf += 1;
					fullAddr += 1;
				}
			}
		}
	} else {
		cerr << "WARNING (WRITE_BURST ignored): The address " << hex << addr << " is not in the valid range for target "
		     << this->GetTargetId() << dec << endl;
		ret = false;
	}

	pthread_mutex_unlock(&targetControlHWAccess_m);

	return ret;
}


string TargetControlHardware::GetTargetId()
{
	char *targetId;
	get_description(&targetId);

	return string(targetId);
}


void BreakSignalHandler(int signum);

#include <sys/time.h>
#include <sys/types.h>

int InitHWPlatform(platform_definition_t *platform)
{
	char *error;

	handle = dlopen(platform->lib, RTLD_LAZY);
	if (!handle)
	{
		cerr << "HW target ERORR: Can't open library " << platform->lib << ", error: " << dlerror() << endl;
		exit(EXIT_FAILURE);
	}
	dlerror(); /* Clear any existing error */

#if 1
	*(void **) (&init_platform) = dlsym(handle, "init_platform");
	if ((error = dlerror()) != NULL)
	{
		cerr << "init_platform: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&close_platform) = dlsym(handle, "close_platform");
	if ((error = dlerror()) != NULL)
	{
		cerr << "close_platform: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&write_to) = dlsym(handle, "write_to");
	if ((error = dlerror()) != NULL)
	{
		cerr << "write_to_platform: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&read_from) = dlsym(handle, "read_from");
	if ((error = dlerror()) != NULL)
	{
		cerr << "read_from_platform: " << error << endl;
		exit(EXIT_FAILURE);
	}
#endif

	*(void **) (&e_open) = dlsym(handle, "e_open");
	if ((error = dlerror()) != NULL)
	{
		cerr << "e_open: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&e_close) = dlsym(handle, "e_close");
	if ((error = dlerror()) != NULL)
	{
		cerr << "e_close: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&e_write) = dlsym(handle, "e_write");
	if ((error = dlerror()) != NULL)
	{
		cerr << "e_write_to_platform: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&e_read) = dlsym(handle, "e_read");
	if ((error = dlerror()) != NULL)
	{
		cerr << "e_read_platform: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&get_description) = dlsym(handle, "get_description");
	if ((error = dlerror()) != NULL)
	{
		cerr << "platform_id: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&hw_reset) = dlsym(handle, "hw_reset");
	if ((error = dlerror()) != NULL)
	{
		cerr << "platform_id: " << error << endl;
		exit(EXIT_FAILURE);
	}

	*(void **) (&e_set_host_verbosity) = dlsym(handle, "e_set_host_verbosity");
	if ((error = dlerror()) != NULL)
	{
		cerr << "platform_id: " << error << endl;
		exit(EXIT_FAILURE);
	}


	// add signal handler to close target connection
	if (signal(SIGINT, BreakSignalHandler) < 0)
	{
		perror("signal");
		exit(EXIT_FAILURE);
	}


	// Open target platform
#if 1
	(*e_set_host_verbosity)(debug_level);
	int intRes = (*init_platform)(platform, debug_level);
	if (intRes < 0)
	{
		cerr << "Cannot initialize target device. Bailing out!" << endl;
		exit(EXIT_FAILURE);
	}
#else
	pEpiphany = &Epiphany;
	(*e_set_host_verbosity)(debug_level);
	int intRes = (*e_open)(pEpiphany);
	if (intRes != 0)
	{
		cerr << "Cannot initialize target device. Bailing out!" << endl;
		exit(EXIT_FAILURE);
	}
#endif

	extern bool skip_platform_reset;
	if (!skip_platform_reset) {
		if (hw_reset() != 0)
		{
			cerr << "Cannot reset the hardware. Exiting!" << endl;
			exit(EXIT_FAILURE);
		}
	} else {
		cerr << "Warning: The platform hardware reset has not been sent to target" << endl;
	}
	return 0;
}


void StartOfBaudMeasurement(struct timeval &start)
{
	gettimeofday(&start, NULL);
}


double EndOfBaudMeasurement(struct timeval &start)
{
	struct timeval end;
	gettimeofday(&end, NULL);

	double seconds  = end.tv_sec  - start.tv_sec;
	double useconds = end.tv_usec - start.tv_usec;

	double ret = ((seconds) * 1000 + useconds/1000.0) + 0.5;

	return ret;
}


unsigned InitDefaultMemoryMap(platform_definition_t *platform)
{
	unsigned chip;
	unsigned bank;
	unsigned row;
	unsigned col;
	unsigned entry = 0;
	unsigned base;
	unsigned num_cores;

	// Add core memory to memory_map and core registers to register_map
	for (chip=0; chip<platform->num_chips; chip++)
	{
		for (row=0; row < platform->chips[chip].num_rows; row++)
		{
			for (col=0; col < platform->chips[chip].num_cols; col++)
			{
				base = ((platform->chips[chip].yid + row) << 26) +
				       ((platform->chips[chip].xid + col) << 20);

				memory_map[entry] = pair<unsigned long, unsigned long>
				        (base, base + platform->chips[chip].core_memory_size - 1);
				register_map[entry] = pair<unsigned long, unsigned long>
				        (base + 0xf0000, base + 0xf1000 - 1);
				entry++;
			}
		}
	}
	num_cores = entry;
	// Add external memory banks to memory map
	for (bank=0; bank < platform->num_banks; bank++)
	{
		memory_map[entry++] = pair<unsigned long, unsigned long>
		(platform->ext_mem[bank].base, platform->ext_mem[bank].base + platform->ext_mem[bank].size - 1);
	}

	return num_cores;
}


// close the target by having controlC signal
// TODO have reset from client
void BreakSignalHandler(int signum)
{
	cerr << " Get OS signal .. exiting ..." << endl;
	// give chance to finish usb drive
	// sleep(1);

	// hw_reset();

	// sleep(1);

	if (handle)
	{
		close_platform();
///		e_close(pEpiphany);
		dlclose(handle);
	}

	exit(0);
}
