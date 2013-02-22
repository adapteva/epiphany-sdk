/*
  File: MpHash.h

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

// Matchpoint hash table: definition

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

//  2 May 09: Jeremy Bennett. Initial version based on the Embecosm reference
//            implementation.

// Embecosm Subversion Identification
// ==================================

// $Id: MpHash.h 967 2011-12-29 07:07:27Z oraikhman $
//-----------------------------------------------------------------------------

#ifndef MP_HASH__H
#define MP_HASH__H

#include <inttypes.h>


//! Default size of the matchpoint hash table. Largest prime < 2^10
#define DEFAULT_MP_HASH_SIZE  1021


//! Enumeration of different types of matchpoint.

//! These have explicit values matching the second digit of 'z' and 'Z'
//! packets.
enum MpType {
	BP_MEMORY   = 0,
	BP_HARDWARE = 1,
	WP_WRITE    = 2,
	WP_READ     = 3,
	WP_ACCESS   = 4
};


class MpHash;


//-----------------------------------------------------------------------------
//! A structure for a matchpoint hash table entry
//-----------------------------------------------------------------------------
struct MpEntry
{
public:

	friend class MpHash; // The only one which can get at next

	MpType   type;       //!< Type of matchpoint
	uint32_t addr;       //!< Address with the matchpoint
	uint16_t instr;      //!< Substituted (16-bit) instruction


private:

	MpEntry *next;       //!< Next in this slot
};


//-----------------------------------------------------------------------------
//! A hash table for matchpoints

//! We do this as our own open hash table. Our keys are a pair of entities
//! (address and type), so STL map is not trivial to use.
//-----------------------------------------------------------------------------
class MpHash
{
public:

	// Constructor and destructor
	MpHash(int _size = DEFAULT_MP_HASH_SIZE);
	~MpHash();

	// Accessor methods
	void     add(MpType type, uint32_t addr, uint16_t instr);
	MpEntry *lookup(MpType type, uint32_t addr);
	bool     remove(MpType type, uint32_t addr, uint16_t *instr = NULL);

private:

	//! The hash table
	MpEntry **hashTab;

	//! Size of the hash table
	int size;
};

#endif // MP_HASH__H
