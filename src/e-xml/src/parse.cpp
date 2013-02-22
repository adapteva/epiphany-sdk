/*
  File: parse.cpp

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
 * Desc:  Program to test the epiphany XML parsing library.
 *
**/

#include <stdio.h>
#include "epiphany_xml.h"

int main(int argc, char* argv[])
{
	int ret_val = 0;
	platform_definition_t* platform;

	if (argc != 2)
	{
		printf("Usage: %s <filenem>\n", argv[0]);
		return -1;
	}

	EpiphanyXML* xml = new EpiphanyXML(argv[1]);
	if (xml->Parse())
	{
		printf("Parse() could not parse the XML\n");
		ret_val = -1;
	}

	else if ((platform = xml->GetPlatform()) == 0)
	{
		printf("GetPlatform() failed\n");
		ret_val = -1;
	}

	else
	{
		xml->PrintPlatform();
	}
	delete xml;

	return ret_val;
}
