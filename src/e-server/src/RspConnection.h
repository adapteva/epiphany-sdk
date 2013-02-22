/*
  File: RspConnection.h

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

// Remote Serial Protocol connection: definition

// Copyright (C) 2008, 2009, Embecosm Limited
// Copyright (C) 2009 Adapteva Inc.

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

// This file is part of the Adapteva RSP server.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.  */

//-----------------------------------------------------------------------------
// This RSP server for the Adapteva ATDSP was written by Jeremy Bennett on
// behalf of Adapteva Inc.

// Implementation is based on the Embecosm Application Note 4 "Howto: GDB
// Remote Serial Protocol: Writing a RSP Server"
// (http://www.embecosm.com/download/ean4.html).

// Note that the ATDSP is a little endian architecture.

// Commenting is Doxygen compatible.

// Change Management
// =================

//  2 May 09: Jeremy Bennett. Initial version based on the Embedosm
//                            reference implementation


// Embecosm Subversion Identification
// ==================================

// $Id: RspConnection.h 967 2011-12-29 07:07:27Z oraikhman $
//-----------------------------------------------------------------------------

#ifndef RSP_CONNECTION__H
#define RSP_CONNECTION__H

#include "RspPacket.h"


//! The default service to use if port number = 0 and no service specified
#define DEFAULT_RSP_SERVICE  "atdsp-rsp"


//-----------------------------------------------------------------------------
//! Class implementing the RSP connection listener

//! RSP requests received from TCP/IP are queued on the output FIFO for
//! processing by the GDB server. Packets read from the input FIFO from the
//! GDB server are sent back via TCP/IP.

//! The packets are received serially, ie. a new packet is not sent until the
//! previous ones have been dealt with. Some packets need no reply, so they
//! will be sent one after the other. But for packets that need a reply (which
//! may be one or several packets), new packets are not sent until the reply
//! from the previous one is received.

//! The upshot of this is that we can avoid any risk of deadlock by always
//! giving priority to any outgoing reply packets.

//! Two threads are used, one to listen for TCP/IP connections from the
//! client, the other to look for FIFO packets from the GDB server to write
//! back to the client. Both must be non-blocking in the SystemC sense
//! (i.e. allow other SystemC threads to run).
//-----------------------------------------------------------------------------
class RspConnection
{
public:
	// Constructors and destructor
	RspConnection(int         _portNum);
	RspConnection(const char *_serviceName = DEFAULT_RSP_SERVICE);
	~RspConnection();

	// Public interface: manage client connections
	bool rspConnect();
	void rspClose();
	bool isConnected();

	// Public interface: get packets from the stream and put them out
	bool getPkt(RspPacket *pkt);
	bool putPkt(RspPacket *pkt);

	bool GetBreakCommand();

private:

	// Generic initializer
	void rspInit(int _portNum, const char *_serviceName);

	// Internal routines to handle individual chars
	bool putRspChar(char c);
	int  getRspChar();

	//! The port number to listen on
	int  portNum;

	//! The service name to listen on
	const char *serviceName;

	//! The client file descriptor
	int  clientFd;

}; // RspConnection()

#endif // RSP_CONNECTION__H
