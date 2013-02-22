/*
  File: targetCntrl.h

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

#ifndef TARGETCNTRL_H_
#define TARGETCNTRL_H_

#include <stdint.h>
#include <stdio.h>
typedef uint32_t sc_uint_32;
typedef uint64_t sc_uint_64;

#include <iostream>
using namespace std;

#include <string>
//-----------------------------------------------------------------------------
//! Module implementing a verilator tar

//! A thread inkoked by RSP memory access commans and build the entry transactions list in stimulus
//! The call is blocking. The gdb doesn't gain control untill the call is done
//-----------------------------------------------------------------------------

typedef unsigned int   uint32_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;


class TargetControl{
//private:
//	unsigned long fCoreNumber;

protected:
	unsigned fAttachedCoreId;

public :


	unsigned long GetAttachedCoreId() { return fAttachedCoreId; }
	virtual bool SetAttachedCoreId(unsigned) { return false; }

public:

	enum ETRAN_TYPE     { BYTE_, SHORT_, WORD_ };
	enum REQ_TYPE       { READ_= 0, WRITE_= 1, KILL_ME = 2, TRACE_ME = 3, RESUME_ME = 4 };
	enum TRACE_REQ_TYPE { INIT_TRACE = 0, START_TRACE = 1, STOP_TRACE = 2 };


	// Functions to access memory. All register access on the Epiphany is via memory
	virtual bool readMem32(uint32_t addr, uint32_t &) = 0;
	virtual bool readMem16(uint32_t addr, uint16_t &) = 0;
	virtual bool readMem8 (uint32_t addr, uint8_t  &) = 0;


	virtual bool writeMem32(uint32_t addr, uint32_t value) = 0;
	virtual bool writeMem16(uint32_t addr, uint16_t value) = 0;
	virtual bool writeMem8 (uint32_t addr, uint8_t  value) = 0;

	virtual bool WriteBurst(unsigned long addr, unsigned char *buf, size_t buff_size) = 0;
	virtual bool ReadBurst (unsigned long addr, unsigned char *buf, size_t buff_size) = 0;

	virtual void PlatformReset() {}; // ignore reset

	// trace support for vcd Dump
	virtual bool initTrace()  = 0;
	virtual bool startTrace() = 0;
	virtual bool stopTrace()  = 0;

	// supported only in the Verilator target
	virtual void ResumeAndExit() {
		cerr << "Warning: The Resume and Detach is only supported in the HW simulation target" << endl;
	}


//	void SetCoreNumber(unsigned _cN) { fCoreNumber=_cN;}

	virtual std::string GetTargetId() = 0;
};

#endif /* TARGETCNTRL_H_ */
