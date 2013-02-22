/*
  File: main.cpp

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


const char *copyrigth = "Copyright (C) 2010, 2011, 2012 Adapteva Inc.\n";
static char const * revision= "$Rev: 1362 $";

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <string.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

#include "maddr_defs.h"
#include <e-xml/src/epiphany_xml.h>

unsigned int PORT_BASE_NUM;

char const hdf_env_var_name[] = "EPIPHANY_HW_DEF_FILE";
char TTY_[1024];


// accesses by all threads
int  debug_level = 0;
bool show_memory_map= false;
bool haltOnAttach = true;
FILE *tty_out = 0;
bool with_tty_support = false;
bool dont_check_hw_address = false;

void *CreateGdbServer(void *ptr);

string plafrom_args;
bool skip_platform_reset = false;

void usage()
{
	cerr << "\033[4mUsage:\033[0m" << endl << endl;
	cerr << "e-server -hdf <hdf-file> [-p <base-port-number>] [-show-memory-map]" << endl;
	cerr << "         [-Wpl,<options>] [-Xpl <arg>] [-version]" << endl << endl;
	cerr << "\033[4mProgram options:\033[0m" << endl << endl;
	cerr << "  \033[1m-hdf\033[0m <hdf-file>\n\t\tSpecify a platform definition file. This parameter is mandatory if no" << endl;
	cerr << "                EPIPHANY_HW_DEF_FILE environment variable is set" << endl;
	cerr << "  \033[1m-p\033[0m\n\t\tBase port number. Default is 51000" << endl;
	cerr << "  \033[1m-show-memory-map\033[0m\n\t\tPrint out the supported memory map" << endl;
	cerr << "  \033[1m-Wpl,\033[0m <options>\n\t\tPass comma-separated <options> on to the platform driver" << endl;
	cerr << "  \033[1m-Xpl\033[0m <arg>\n\t\tPass <arg> on to the platform driver" << endl;
	cerr << "  \033[1m-version\033[0m\n\t\tDisplay the version number and copyrights" << endl;
}

void usage_hidden()
{
	cerr << "\nDebug options: [-d <debug-level>] [-tty <terminal>] [-dont-halt-on-attach]\n" << endl;
	cerr << "  -d <debug-level>\n\t\tRun the e-server in debug mode. Default is 0 (silent)" << endl;
	cerr << "  -tty <terminal>\n\t\tRedirect the e_printf to terminal with tty name" << endl;
	cerr << "  -dont-halt-on-attach\n\t\tWhen starting an e-gdb session, the debugger initiates an attachment\n\t\tprocedure when executing the 'target remote:' command. By default,\n\t\tthis procedure includes a reset sequence sent to the attached core.\n\t\tUse this option to disable the intrusive attachment and perform a non\n\t\t-intrusive one that does not change the core's state. This allows to\n\t\tconnect and monitor a core that is currently running a program." << endl;
	cerr << "  -dont-check-hw-address\n\t\tThe e-server filters out transactions if the address is invalid (not\n\t\tincluded in the device supported memeory map). Use this option to\n\t\tdisable this protection" << endl;
	cerr << "\nTarget validation/check additional options:" << endl << endl;
	cerr << "  -skip-platform-reset\n\t\tDon't make the hardware reset during initialization" << endl;
}

void copyright()
{
	std::cerr << "e-server " << "(compiled on " << __DATE__ << ")";
	std::cerr << "<" << string(revision).substr(string(revision).find(" "), string(revision).rfind("$") - string(revision).find(" "))
	          << ">" << std::endl;

	std::cerr << copyrigth << std::endl;
	std::cerr << "The Epiphany XML Parser uses the XML library developed by Michael Chourdakis\n";
}


extern int InitHWPlatform(platform_definition_t *platform);

// char DEFAULT_XML_REL_PATH[] = "/bsps/emek3/emek3.xml";

int main(int argc, char *argv[])
{
	//int iret = 0;
	int mainRetStatus = 0;

	//char *epiphany_home = getenv("EPIPHANY_HOME");
	//string xml_default;
	//if (epiphany_home)
	//{
	//	xml_default = string(epiphany_home) + string(DEFAULT_XML_REL_PATH);
	//}


	char *hdf_file = getenv(hdf_env_var_name); // xml_default.c_str();

	PORT_BASE_NUM = 51000;

	sprintf(TTY_, "tty");

	/////////////////////////////
	// parse command line options
	for (int n = 1; n < argc; n++)
	{
		if (!strcmp(argv[n], "-version"))
		{
			copyright();
			cerr << "\n\033[4mPlease report bugs to:\033[0m \033[1m<support-sdk@adapteva.com>\033[0m" << endl;
			return 1;
		}

		if ((!strcmp(argv[n], "-h")) || (!strcmp(argv[n], "--help")))
		{
			usage();
			return 1;
		}

		if (!strcmp(argv[n], "-help-hidden"))
		{
			usage();
			usage_hidden();
			return 1;
		}

		if (!strcmp(argv[n], "-dont-check-hw-address"))
		{
			dont_check_hw_address = true;
		}

		if (!strcmp(argv[n], "-dont-halt-on-attach"))
		{
			haltOnAttach = false;
		}


		if (!strcmp(argv[n], "-skip-platform-reset"))
		{
			skip_platform_reset = true;

		}

		if (!strcmp(argv[n], "-Xpl")) {
			n += 1;
			if (n< argc)
			{
				plafrom_args += string(argv[n]) + " ";

			} else {
				usage();
				return 3;
			}
			continue;
		}

		if (!strncmp(argv[n], "-Wpl,",strlen("-Wpl,"))) {
			//-Wpl,-abc,-123,-reset_timeout,4

			string str = string(argv[n]);
			char const *delim = ",";
			char *pch = strtok((char*)str.c_str(), delim);
			if (pch) pch = strtok(NULL, delim);
			while (pch != NULL) {
				plafrom_args += " " + string(pch);
				pch = strtok(NULL, delim);
			}

			continue;
		}

		if (!strcmp(argv[n], ""))
		{
			return 1;
		}



		if (!strcmp(argv[n], "-p"))
		{
			n += 1;
			if (n < argc)
			{
				PORT_BASE_NUM = atoi(argv[n]);
				std::cout << "Setting base port number to " << PORT_BASE_NUM << "." << std::endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if (!strcmp(argv[n], "-tty"))
		{
			n += 1;
			if (n< argc)
			{
				with_tty_support = true;
				strncpy(TTY_, (char *) argv[n], sizeof(TTY_));
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if (!strcmp(argv[n], "-d"))
		{
			n += 1;
			if (n< argc)
			{
				debug_level = atoi(argv[n]);
				std::cout << "setting debug level to " << debug_level << std::endl;
			} else {
				usage();
				return 3;
			}
			continue;
		}

		if (!strcmp(argv[n], "-show-memory-map"))
		{
			show_memory_map = true;
		}

		if (!strcmp(argv[n], "-hdf"))
		{
			n += 1;
			if (n >= argc)
			{
				usage();
				return 3;
			}
			else
			{
				if (hdf_file) {
					cerr << "Warning: The HDF environment variable <" << hdf_env_var_name << "> is overwritten by command line option." << endl;
				}
				hdf_file = argv[n];
			}
		}
	}


	////////////////
	// parse the HDF
	if (hdf_file == 0)
	{
		cerr << "Please specify a platform definition file." << endl << endl;
		usage();
		return 3;
	}

	cout << "Using the HDF file: " << hdf_file << endl;
	EpiphanyXML *xml = new EpiphanyXML((char *) hdf_file);
	if (xml->Parse())
	{
		delete xml;
		cerr << "Can't parse Epiphany HDF file: " << hdf_file << endl;
		exit(3);
	}
	platform_definition_t *platform = xml->GetPlatform();
	if (!platform)
	{
		delete xml;
		cerr << "Could not extract platform information from " << hdf_file << endl;
		exit(3);
	}

	// prepare args list to fardware driver library
	plafrom_args += string(" ") + string(platform->libinitargs);
	platform->libinitargs = (char *) plafrom_args.c_str();

	//////////////////////////////////////////////////////
	// populate the chip and ext_mem list of memory ranges
	extern unsigned InitDefaultMemoryMap(platform_definition_t *);
	unsigned ncores = InitDefaultMemoryMap(platform);

	extern map<unsigned, pair<unsigned long, unsigned long> > memory_map;
	extern map<unsigned, pair<unsigned long, unsigned long> > register_map;

	if (show_memory_map)
	{
		xml->PrintPlatform();
		cout << "Supported registers map: " << endl;
		for (map<unsigned, pair<unsigned long, unsigned long> >::iterator ii=register_map.begin(); ii!=register_map.end(); ++ii)
		{
			unsigned long startAddr = (*ii).second.first;
			unsigned long endAddr   = (*ii).second.second;
			cout << " [" << hex << startAddr << "," << endAddr << dec << "]\n";
		}
		unsigned core_num = 0;
		cout << "Supported memory map: " << endl;
		for (map<unsigned, pair<unsigned long, unsigned long> >::iterator ii=memory_map.begin(); ii!=memory_map.end(); ++ii)
		{
			unsigned long startAddr = (*ii).second.first;
			unsigned long endAddr   = (*ii).second.second;

			if (core_num < ncores)
			{
				cout << "";
			} else {
				cout << "External: ";
			}
			cout << " [" << hex << startAddr << "," << endAddr << dec << "]\n";
			core_num++;
		}
	}

	// open terminal
	if (with_tty_support)
	{
		tty_out = fopen(TTY_, "w");
		if (tty_out == NULL)
		{
			cerr << "Can't open tty " << TTY_ << endl;
			exit(2);
		}
	}


	////////////////////////
	// initialize the device
	InitHWPlatform(platform);


	if (true) {
		// Create independent threads each of which will execute function
		// FIXME: switch to dynamic port creation model
		// n threads for gdb, thread for loader, thread for reset client (Used in Eclipse)
		unsigned portsNum[ncores];

		// loader
///		portsNum[ncores] = PORT_BASE_NUM-1;
		// host reset proxy
///		portsNum[ncores+1] = PORT_BASE_NUM-2;

		for (unsigned i=0; i<ncores; i++)
		{
			portsNum[i] = PORT_BASE_NUM + i;
		}

		pthread_t thread[ncores];

		//////////////////////////////////////////////
		// create and execute the thread for the cores
		for (unsigned i=0; i<ncores; i++)
		{
			pthread_create(&(thread[i]), NULL, CreateGdbServer, (void *) (portsNum+i));
			//iret = pthread_create(&(thread[i]), NULL, CreateGdbServer, (void *) (portsNum+i));
		}

		sleep(1);

		/////////////////////////////
		// wait for threads to finish
		/* Wait till threads are complete before main continues. Unless we */
		/* wait we run the risk of executing an exit which will terminate  */
		/* the process and all threads before the threads have completed.  */
		for (unsigned i=0; i<(ncores); i++)
		{
			pthread_join((thread[i]), NULL);
		}

	}

	if (tty_out) {
		fclose(tty_out);
	}

	delete xml;

	return mainRetStatus;
}

