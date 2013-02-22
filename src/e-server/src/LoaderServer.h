/*
  File: LoaderServer.h

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


#ifndef LOADER_SEVER
#define LOADER_SEVER

using namespace std;
#include <iostream>
#include <assert.h>

#include "target_param.h"


#define MAX_BUFFER_TO_SERVER_SIZE MAX_NUM_WRITE_PACKETS*8

#define MAX_BUFFER_FROM_SERVER_SIZE 64

struct TPacket2GdbServer {
	enum ECommand { Kill, Write, Read, Reset, ResumeAndExit};
	ECommand fCommand;
#ifdef _WIN32
	unsigned __int64 fAddr;
#else
	unsigned long fAddr;
#endif
	unsigned char fData[MAX_BUFFER_TO_SERVER_SIZE];//up to MAX_BURST_SIZE byte supported
	unsigned short fDataSizeBytes;
	TPacket2GdbServer() {
		fCommand=Kill;
		fAddr=0;
		fDataSizeBytes=0;
		for (unsigned i=0; i<MAX_BUFFER_TO_SERVER_SIZE; i++) {
			fData[i]=0;
		}
	}
	TPacket2GdbServer(unsigned long _addr, unsigned char *_fData,unsigned short _fDataSize) {
		fCommand=Write;
		fAddr=_addr;
		assert(_fDataSize <= MAX_BUFFER_TO_SERVER_SIZE);
		fDataSizeBytes=_fDataSize;
		memcpy((void *)(fData), (const void *) (_fData), fDataSizeBytes);
	}
};
inline ostream& operator << (ostream& s, TPacket2GdbServer& p) {

	if (p.fCommand == TPacket2GdbServer::Write) {

		s << "Write to addr " << hex << p.fAddr << " size " << p.fDataSizeBytes << endl;
		for (unsigned i=0; i<p.fDataSizeBytes; i++) {
			s << hex << "[" << p.fAddr + i << "]= " << (unsigned short) p.fData[i] << dec << endl;
		}
		s << std::endl;
	}

	if (p.fCommand == TPacket2GdbServer::Read) {
		//s << "Read from " << endl;
		s << hex << "[0x" << p.fAddr << "]/0x" << (unsigned short) p.fDataSizeBytes << dec << endl;
	}
	if (p.fCommand == TPacket2GdbServer::Kill) {
		s << "Kill ........" << endl;
	}
	if (p.fCommand == TPacket2GdbServer::ResumeAndExit) {
		s << "ResumeAndExit ......." << endl;
	}
	if (p.fCommand == TPacket2GdbServer::Reset) {
		s << "Reset " << endl;
	}

	return s;
}


struct TPacketFromGdbServer {
	enum Response { Good, Bad };
	Response resp;
	unsigned char fData[MAX_BUFFER_FROM_SERVER_SIZE];
};
inline ostream& operator << (ostream& s, TPacketFromGdbServer& p) {

	if (p.resp == TPacketFromGdbServer::Good) {
		s << "Data " << endl;
		for (unsigned i=0; i<MAX_BUFFER_FROM_SERVER_SIZE; i++) {
			s << hex << (unsigned short) p.fData[i] << dec << endl;
		}
		s << std::endl;

	} else {
		s << " Get Bad response !!!" << endl;
	}

	return s;
}
#endif
