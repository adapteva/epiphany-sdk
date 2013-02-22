/*
  File: Utils.h

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

// GDB Server Utilties: definition

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

//  4 May 09: Jeremy Bennett. Initial version based on the Embecosm GDB server
//                            for Verilator implementation.

// Embecosm Subversion Identification
// ==================================

// $Id: Utils.h 967 2011-12-29 07:07:27Z oraikhman $
//-----------------------------------------------------------------------------

#ifndef UTILS_H
#define UTILS_H

#include <inttypes.h>


//-----------------------------------------------------------------------------
//! A class offering a number of convenience utilities for the GDB Server.

//! All static functions. This class is not intended to be instantiated.
//-----------------------------------------------------------------------------
class Utils
{
public:

	static uint8_t    char2Hex(int c);
	static const char hex2Char(uint8_t d);
	static void       reg2Hex(uint32_t val, char *buf);
	static uint32_t   hex2Reg(char *buf);
	static void       ascii2Hex(char *dest, char *src);
	static void       hex2Ascii(char *dest, char *src);
	static int        rspUnescape(char *buf, int len);
	static uint32_t   htotl(uint32_t hostVal);
	static uint32_t   ttohl(uint32_t targetVal);


private:

	// Private constructor cannot be instantiated
	Utils() {};

}; // class Utils


//-----------------------------------------------------------------------------
//! use the same memory allocation routines as Cgen simulator
//! used for supporting stdio
//-----------------------------------------------------------------------------
void *zalloc(unsigned long size);
void zfree(void *data);

#endif // UTILS_H
