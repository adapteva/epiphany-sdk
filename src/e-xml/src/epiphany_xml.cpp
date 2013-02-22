/*
  File: epiphany_xml.cpp

  This file is part of the Epiphany Software Development Kit.

  Copyright (C) 2013 Adapteva, Inc.
  Contributed by Oleg Raikhman, Yaniv Sapir <support@adapteva.com>

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

/**
 *
 * Desc:  Utility to parse an Epiphany XML platform configuration file.
 *
**/

#include "xml.h"
#include "epiphany_xml.h"

// Extract a string value from an attribute
int EpiphanyXML::ExtractAttr(XMLElement* element, char** value, char* attr)
{
	// Given the attribute's name, find it in the element.
	XMLVariable* v = element->FindVariableZ(attr);

	// If the attribute could not be found, set *value to NULL and return an error.
	if (!v)
	{
		*value = NULL;
		return -1;
	}

	// Find the length of the attribute
	int valuelen = v->GetValue(0, 0);

	// Create a new string big enough to hold the attribute's value.
	*value = new char[valuelen+1];

	// Copy the value of the attribute to the provided string pointer.
	v->GetValue(*value);

	return 0;
}

// Extract an unsigned value from an attribute
int EpiphanyXML::ExtractAttr(XMLElement* element, unsigned* uns_p, char* name)
{
	char buf[128];
	XMLVariable* v;

	v = element->FindVariableZ(name);

	// If the variable could not be found, return an error.
	if (!v)
	{
		return -1;
	}

	// Else, convert it to an unsigned.
	v->GetValue(buf);
	*uns_p = strtoul(buf, NULL, 0);
	return 0;
}

// Extract coordinates from a "(y,x)" string
int EpiphanyXML::ExtractCoords(XMLElement* element, unsigned* yid, unsigned* xid)
{
	int valuelen;

	// Given the attribute's name, find it in the element.
	XMLVariable* v = element->FindVariableZ("id");

	if ((!v) || ((valuelen = v->GetValue(0, 0)) == 0))
	{
		return -1;
	}
	else
	{
		int valuelen = v->GetValue(0, 0);
		char buf[valuelen+1];
		v->GetValue(buf);
		return (sscanf(buf, "(%u,%u)", yid, xid) == 2) ? 0: -1;
	}
}

// Extract the column and row attributes from an ioreg element.
int EpiphanyXML::ExtractIOReg(XMLElement* element, unsigned* row, unsigned* col)
{
	// ioregs is a child of chip and should have a col and row attribute
	int nkids;
	int i;
	XMLElement* child;
	char buf[128];

	nkids = element->GetChildrenNum();
	for (i=0; i<nkids; i++)
	{
		if ((child = element->GetChildren()[i]))
		{
			child->GetElementName(buf);
			if ((strcmp(buf, "ioregs") == 0) &&
			    (ExtractAttr(child, row, "row") == 0) &&
			    (ExtractAttr(child, col, "col") == 0))
			{
				// found the ioregs element and liked it.
				return 0;
			}
		}
	}
	*row = NO_IOREG;
	*col = NO_IOREG;
	return -1;
}

// Extract data from a <chip> element.
int EpiphanyXML::ExtractChip(XMLElement* element, chip_def_t* chip)
{
	ExtractAttr(element, &chip->version, "version");
	ExtractIOReg(element, &chip->ioreg_row, &chip->ioreg_col);
	return (ExtractCoords(element, &chip->yid, &chip->xid)      ||
	        ExtractAttr(element, &chip->num_rows, "rows")       ||
	        ExtractAttr(element, &chip->num_cols, "cols")       ||
	        ExtractAttr(element, &chip->host_base, "host_base") ||
	        ExtractAttr(element, &chip->core_memory_size, "core_memory_size")
	       ) ? -1 : 0;
}

// Extract data from a <chips> element.
void EpiphanyXML::ExtractChips(XMLElement* element)
{
	int i;
	int nkids;
	XMLElement* child;
	char buf[128];

	// Figure out how many chips there are.
	if ((nkids = element->GetChildrenNum()))
	{
		// If we've already found a chip tag, we should just add more
		// chips to m_platform->chips. To do this, we allocate a new
		// one, copy the old one into it, free the old one, and replace the old with
		// the new.
		if (m_platform->num_chips)
		{
			chip_def_t* new_platform = new chip_def_t[nkids + m_platform->num_chips];
			memcpy(new_platform, m_platform->chips, sizeof(chip_def_t) * m_platform->num_chips);
			delete[] m_platform->chips;
			m_platform->chips = new_platform;
		}
		else
		{
			// If the XML is well-formed, all of the child elements of <chips>
			// should be <chip>. Allocating enough chip_def_t's for all the children
			// will only waste memory if the XML is not well-formed.
			m_platform->chips = new chip_def_t[nkids];
		}

		// Now populate the array with each valid chip we find
		for (i = 0; i < nkids; i++)
		{
			child = element->GetChildren()[i];
			child->GetElementName(buf);
			if (!strcmp(buf, "chip") && ExtractChip(child, &m_platform->chips[m_platform->num_chips]) == 0)
			{
				m_platform->num_chips += 1;
			}
		}
	}
}

// Extract data from a <bank> element.
int EpiphanyXML::ExtractBank(XMLElement* element, mem_def_t* bank)
{
	return ((ExtractAttr(element, &bank->name, "name")  == 0) &&
	        (ExtractAttr(element, &bank->base, "start") == 0) &&
	        (ExtractAttr(element, &bank->size, "size")  == 0)
	       ) ? 0 : -1;
}

// Extract data from an <external_memory> element.
void EpiphanyXML::ExtractExternalMemory(XMLElement* element)
{
	int i;
	int nkids;
	XMLElement* child;
	char buf[128];

	// Figure out how many chips there are.
	if ((nkids = element->GetChildrenNum()))
	{
		// If we've already found an external memory tag, we should just add more
		// memory banks to m_platform->ext_mem. To do this, we allocate a new
		// one, copy the old one into it, free the old one, and replace the old with
		// the new.
		if (m_platform->num_banks)
		{
			mem_def_t *new_mem = new mem_def_t[nkids + m_platform->num_banks];
			memcpy(new_mem, m_platform->ext_mem, sizeof(mem_def_t) * m_platform->num_banks);
			delete[] m_platform->ext_mem;
			m_platform->ext_mem = new_mem;
		}
		else
		{
			// If the XML is well-formed, all of the child elements of <external_memory>
			// should be <bank>. Allocating enough mem_def_t's for all the children
			// will only waste memory if the XML is not well-formed.
			m_platform->ext_mem = new mem_def_t[nkids];
		}
		// Now populate the array with each bank we find
		for (i = 0; i < nkids; i++)
		{
			child = element->GetChildren()[i];
			child->GetElementName(buf);
			if (!strcmp(buf, "bank"))
			{
				ExtractBank(child, &m_platform->ext_mem[m_platform->num_banks]);
				m_platform->num_banks += 1;
			}
		}
	}
}

unsigned EpiphanyXML::Version()
{
	return m_platform->version;
}

EpiphanyXML::EpiphanyXML(char* filename)
{
	m_xml = new XML(filename);
	m_platform = new platform_definition_t;
	m_platform->num_chips = 0;
	m_platform->num_banks = 0;
	m_platform->lib = 0;
	m_platform->libinitargs = 0;
	m_platform->name = 0;
	m_platform->chips = 0;
	m_platform->ext_mem = 0;
}

EpiphanyXML::~EpiphanyXML()
{
	unsigned i;

	if (m_platform)
	{
		if (m_platform->chips)
		{
			for(i=0; i<m_platform->num_chips; i++)
			{
				delete m_platform->chips[i].version;
			}
			delete [] m_platform->chips;
		}
		if (m_platform->ext_mem)
		{
			for(i=0; i<m_platform->num_banks; i++)
			{
				delete m_platform->ext_mem[i].name;
			}
			delete [] m_platform->ext_mem;
		}
		if (m_platform->name) delete m_platform->name;
		if (m_platform->lib) delete m_platform->lib;
		if (m_platform->libinitargs) delete m_platform->libinitargs;
		delete m_platform;
		delete m_xml;
	}
}

int EpiphanyXML::Parse()
{
	int nkids = 0;
	int i;
	XMLElement* root;
	XMLElement* child;
	//XMLElement* gchild;
	char buf[256];

	root = m_xml->GetRootElement();
	root->GetElementName(buf);
	if (strcmp(buf, "platform") ||
	    ExtractAttr(root, &m_platform->version, "version") ||
	    ExtractAttr(root, &m_platform->name, "name") ||
	    ExtractAttr(root, &m_platform->lib, "lib") ||
	    ExtractAttr(root, &m_platform->libinitargs, "libinitargs"))
	{
		return -1;
	}

	if (m_platform->version != 1)
	{
		return -2;
	}

	// process platform children
	nkids = root->GetChildrenNum();
	for (i=0; i < nkids; i++)
	{
		child = root->GetChildren()[i];
		child->GetElementName(buf);
		if (strcmp(buf, "chips") == 0)
		{
			ExtractChips(child);
		}
		else if (strcmp(buf, "external_memory") == 0)
		{
			ExtractExternalMemory(child);
		}
	}

	return 0;
}

platform_definition_t* EpiphanyXML::GetPlatform()
{
	return m_platform;
}

void EpiphanyXML::PrintPlatform(void)
{
	unsigned i;

	printf("The Epiphany XML Parser uses the XML library developed by Michael Chourdakis\n");
	printf("----------------------------------------------------------------------------\n");
	printf("Struct version: %u\n", m_platform->version);
	printf("name: %s\n", m_platform->name);
	printf("lib: %s\n", m_platform->lib);
	printf("libinitargs: %s\n", m_platform->libinitargs);
	for(i=0; i<m_platform->num_chips; i++)
	{
		printf("chip[%d]\n", i);
		printf("\tversion: %s\n", m_platform->chips[i].version);
		printf("\tyid: %d\n", m_platform->chips[i].yid);
		printf("\txid: %d\n", m_platform->chips[i].xid);
		if ((m_platform->chips[i].ioreg_row == NO_IOREG) ||
		    (m_platform->chips[i].ioreg_col == NO_IOREG))
		{
			printf("\tNo I/O registers\n");
		}
		else
		{
			printf("\tioreg_row: %u\n", m_platform->chips[i].ioreg_row);
			printf("\tioreg_col: %u\n", m_platform->chips[i].ioreg_col);
		}
		printf("\tnum_rows: %d\n", m_platform->chips[i].num_rows);
		printf("\tnum_cols: %d\n", m_platform->chips[i].num_cols);
		printf("\thost_base: 0x%x\n", m_platform->chips[i].host_base);
		printf("\tcore_memory_size: 0x%xu\n", m_platform->chips[i].core_memory_size);
	}
	for(i=0; i<m_platform->num_banks; i++)
	{
		printf("ext_mem[%d]\n", i);
		printf("\tname: %s\n", m_platform->ext_mem[i].name);
		printf("\tbase: 0x%x\n", m_platform->ext_mem[i].base);
		printf("\tsize: 0x%x\n", m_platform->ext_mem[i].size);
	}
}


