/*
  File: MpHash.cpp

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

// Matchpoint hash table: implementation

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

// $Id: MpHash.cpp 967 2011-12-29 07:07:27Z oraikhman $
//-----------------------------------------------------------------------------

#include <cstdlib>

#include "MpHash.h"


//-----------------------------------------------------------------------------
//! Constructor

//! Allocate the hash table
//! @param[in] size  Number of slots in the  hash table. Defaults to
//!                  DEFAULT_MP_HASH_SIZE.
//-----------------------------------------------------------------------------
MpHash::MpHash(int _size) :
size (_size)
{
	// Allocate and clear the hash table
	hashTab = new MpEntry *[size];

	for (int i=0; i<size; i++)
	{
		hashTab[i] = NULL;
	}
} // MpHash()


//-----------------------------------------------------------------------------
//! Destructor

//! Free the hash table
//-----------------------------------------------------------------------------
MpHash::~MpHash()
{
	delete [] hashTab;
} // ~MpHash()


//-----------------------------------------------------------------------------
//! Add an entry to the hash table

//! Add the entry if it wasn't already there. If it was there do nothing. The
//! match just be on type and addr. The instr need not match, since if this is
//! a duplicate insertion (perhaps due to a lost packet) they will be
//! different.

//! @note This method allocates memory. Care must be taken to delete it when
//! done.

//! @param[in] type   The type of matchpoint
//! @param[in] addr   The address of the matchpoint
//! @para[in]  instr  The instruction to associate with the address
//-----------------------------------------------------------------------------
void
MpHash::add(MpType type, uint32_t addr, uint16_t instr)
{
	int      hv = addr % size;
	MpEntry *curr;

	// See if we already have the entry
	for (curr = hashTab[hv]; NULL != curr; curr = curr->next)
	{
		if ((type == curr->type) && (addr == curr->addr))
		{
			return; // We already have the entry
		}
	}

	// Insert the new entry at the head of the chain
	curr = new MpEntry();

	curr->type  = type;
	curr->addr  = addr;
	curr->instr = instr;
	curr->next  = hashTab[hv];

	hashTab[hv] = curr;
} // add()


//-----------------------------------------------------------------------------
//!Look up an entry in the matchpoint hash table

//! The match must be on type AND addr.

//! @param[in] type   The type of matchpoint
//! @param[in] addr   The address of the matchpoint

//! @return  The entry found, or NULL if the entry was not found
//-----------------------------------------------------------------------------
MpEntry *
MpHash::lookup(MpType type, uint32_t addr)
{
	int hv = addr % size;

	// Search
	for (MpEntry *curr = hashTab[hv]; NULL != curr; curr = curr->next)
	{
		if ((type == curr->type) && (addr == curr->addr))
		{
			return curr; // The entry was found
		}
	}

	return NULL; // The entry was not found
} // lookup()


//-----------------------------------------------------------------------------
//! Delete an entry from the matchpoint hash table

//! If it is there the entry is deleted from the hash table. If it is not
//! there, no action is taken. The match must be on type AND addr. The entry
//! (MpEntry::) is itself deleted

//! The usual fun and games tracking the previous entry, so we can delete
//! things.

//! @param[in]  type   The type of matchpoint
//! @param[in]  addr   The address of the matchpoint
//! @param[out] instr  Location to place the instruction found. If NULL (the
//!                    default) then the instruction is not written back.

//! @return  TRUE if an entry was found and deleted
//-----------------------------------------------------------------------------
bool
MpHash::remove(MpType type, uint32_t addr, uint16_t *instr)
{
	int      hv   = addr % size;
	MpEntry *prev = NULL;
	MpEntry *curr;

	// Search
	for (curr = hashTab[hv]; NULL != curr; curr = curr->next)
	{
		if ((type == curr->type) && (addr == curr->addr))
		{
			// Found - delete. Method depends on whether we are the head of
			// chain.
			if (NULL == prev)
			{
				hashTab[hv] = curr->next;
			}
			else
			{
				prev->next = curr->next;
			}

			if (NULL != instr)
			{
				*instr = curr->instr; // Return the found instruction
			}

			delete curr;
			return true; // Success
		}

		prev = curr;
	}

	return false; // Not found
} // remove()
