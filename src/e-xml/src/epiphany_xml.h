/*
  File: epiphany_xml.h

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
#include "epiphany_platform.h"

#ifdef _MSC_VER
#define EPIPHANY_XML_API __declspec(dllexport)
#else
#define EPIPHANY_XML_API
#endif


class EPIPHANY_XML_API EpiphanyXML
{
public:
	EpiphanyXML(char* filename);
	~EpiphanyXML();

public:
	int                    Parse(void);
	platform_definition_t* GetPlatform(void);
	void                   PrintPlatform(void);
	unsigned               Version();

private:
	int  ExtractAttr(XMLElement* element, char** value, char* attr);
	int  ExtractAttr(XMLElement* element, unsigned* value, char* attr);
	int  ExtractCoords(XMLElement* element, unsigned* yid, unsigned* xid);
	int  ExtractIOReg(XMLElement* element, unsigned* col, unsigned* row);
	void ExtractChips(XMLElement* child);
	int  ExtractChip(XMLElement* parent, chip_def_t* chip);
	void ExtractExternalMemory(XMLElement* child);
	int  ExtractBank(XMLElement* element, mem_def_t* bank);

	XML*                   m_xml;
	platform_definition_t* m_platform;
};

