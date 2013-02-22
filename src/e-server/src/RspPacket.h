/*
  File: RspPacket.h

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

// RSP packet: definition

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

// $Id: RspPacket.h 967 2011-12-29 07:07:27Z oraikhman $
//-----------------------------------------------------------------------------

#ifndef RSP_PACKET__H
#define RSP_PACKET__H

#include <iostream>


//-----------------------------------------------------------------------------
//! Class for RSP packets

//! Can't be null terminated, since it may include zero bytes
//-----------------------------------------------------------------------------
class RspPacket
{
public:

	//! The data buffer. Allow direct access to avoid unnecessary copying.
	char *data;

	// Constructor and destructor
	RspPacket(int _bufSize);
	~RspPacket();

	// Pack a constant string into a packet
	void packStr(const char *str); // For fixed packets

	// Accessors
	int  getBufSize();
	int  getLen();
	void setLen(int _len);

private:

	//! The data buffer size
	int bufSize;

	//! Number of chars in the data buffer (<= bufSize)
	int len;
};


//! Stream output
std::ostream &operator << (std::ostream &s, RspPacket &p);


#endif // RSP_PACKET_SC__H
