/*
  File: targetCntrlHardware.h

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

#ifndef TARGETCNTRLHARDWARE_H_
#define TARGETCNTRLHARDWARE_H_

//-----------------------------------------------------------------------------
//! Module implementing a hardware target
//!
//! A thread invoked by RSP memory access commands.
//! The call is blocking. The gdb doesn't gain control until the call is done
//-----------------------------------------------------------------------------


#include "targetCntrl.h"


class TargetControlHardware : public TargetControl {
public:

public:
	// Constructor
	TargetControlHardware(unsigned indexInMemMap);
private:
	TargetControlHardware():TargetControl() {
	}
public:
	// check if specified coreis is supported by HW system
	virtual bool SetAttachedCoreId(unsigned);

	// Functions to access memory. All register access on the ATDSP is via memory
	virtual bool readMem32(uint32_t addr, uint32_t &);
	virtual bool readMem16(uint32_t addr, uint16_t &);
	virtual bool readMem8 (uint32_t addr, uint8_t  &);

	virtual bool writeMem32(uint32_t addr, uint32_t value);
	virtual bool writeMem16(uint32_t addr, uint16_t value);
	virtual bool writeMem8 (uint32_t addr, uint8_t  value);

	//burst write and read
	virtual bool WriteBurst(unsigned long addr, unsigned char *buf, size_t buff_size);
	virtual bool ReadBurst (unsigned long addr, unsigned char *buf, size_t buff_size);

	//send system specific reset for all platfom/chip
	virtual void PlatformReset();

	//no support for the trace
	virtual bool initTrace()  { return true; }
	virtual bool startTrace() { return true; }
	virtual bool stopTrace()  { return true; }

	virtual std::string GetTargetId();


private :
	/* convert the local address to full address */
	unsigned long ConvertAddress(unsigned long);

	/* read and write from target */
	bool readMem (uint32_t addr, sc_uint_32 &data, unsigned burst_size);
	bool writeMem(uint32_t addr, sc_uint_32  data, unsigned burst_size);

};

#endif /* TARGETCNTRLHARDWARE_H_ */
