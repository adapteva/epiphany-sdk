
// Copyright (C) 2010, 2011 Adapteva Inc.

// This file is part of the Adapteva RSP server.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
#include <iostream>
#include <fstream>
#include <cassert>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <map>
#include <vector>

#include "e-host.h"
#include "e-loader.h"

#define diag(vN)   if (sverbose >= vN)

using namespace std;

#define MAX_NUM_WRITE_PACKETS 256
#define MAX_NUM_READ_PACKETS   64
#define MAX_BUFFER_TO_SERVER_SIZE (MAX_NUM_WRITE_PACKETS * 8)

unsigned long addressGlobalSpaceOffset = 0;

static int sverbose;

int parseAndSendSrecFile(char *srecFile, Epiphany_t *pEpiphany, bool broadcast, int verbose) {

	enum SrecSel {S0, S3, S7};
	ifstream srefStream;
	int corenum, numcores;
	int status = EPI_OK;

	sverbose = verbose;

	if (broadcast) {
		numcores = EPI_CORES;
		diag(L_D1) { cout << "Loading chip in broadcast mode." << endl; }
	} else
		numcores = 1;

	for (corenum=0; corenum<numcores; corenum++) {
		srefStream.open(srecFile, ifstream::in);
		if (!srefStream.good()) {
			cerr << "Error: Can't open SREC file: " << srecFile << endl;
			return EPI_ERR;
		}

		unsigned lineN = 0;
		char* buf = new char[1000]; // should be sufficient for SREC record

		while(srefStream.good())
		{
			srefStream.getline(buf, sizeof(char[1000]));

			if (srefStream.good()) {
				diag(L_D3) { cout << buf << endl; }

				// RECORD TYPE
				SrecSel sSel;
				unsigned addrSize;

				if (buf[0] != 'S') {
					cerr << "Error: Invalid record format in SREC file line: ";
				        cerr << lineN << ":\"" << buf << "\"" << endl;
					return EPI_ERR;
				}

				if (buf[1] == '0') {
					sSel = S0;
					addrSize = 4;
				} else if (buf[1] == '3') {
					sSel = S3;
					addrSize = 8;
				} else if (buf[1] == '7') {
					sSel = S7;
					addrSize = 8;
				} else {
					cerr << "Error: Invalid record types (valid types are S0, S3 and S7) in SREC file line: ";
				        cerr << lineN << ":\"" << buf << "\"" << endl;
					continue;
				}


				// BYTES COUNT
				char byteCount[5];
				byteCount[0] = '0';
				byteCount[1] = 'x';
				byteCount[2] = buf[2];
				byteCount[3] = buf[3];
				byteCount[4] = '\0';
				unsigned byteCountHex = strtol(byteCount, NULL, 16);
				if (byteCountHex > 0) {
					byteCountHex = byteCountHex - 4 /* addr */ - 1 /* checksum */;
					diag(L_D3) { cout << "Record length = " << dec << byteCountHex << endl; }
				} else {
					cerr << "Error: Wrong record format in SREC file line :";
					cerr << lineN << ":\"" << buf << "\"" << endl;
					continue;
				}


				// ADDRESS
				unsigned long addrtHex;

				char addrBuf[9];
				strncpy(addrBuf, buf+4, addrSize);
				addrBuf[addrSize] = '\0';
				addrtHex = strtol(addrBuf, NULL, 16);
				if (broadcast) {
					addrtHex = (addrtHex & 0x000FFFFF) | (get_id_from_num(corenum) << 20);
				} else {
					if (addrtHex < (1<<20)) { // make internal to global conversion
						addrtHex += addressGlobalSpaceOffset;
					}
				}
				diag(L_D3) { cout << "Address: " << hex << addrtHex << endl; }


				// DATA
				if (sSel == S0) {
					// Start of Core section
					unsigned long CoreID;
					extern vector<unsigned> fDiscoveredCoreIds;

					char dataBuf[5];
					strncpy(dataBuf, buf+4, 4);
					dataBuf[4] = '\0';

					diag(L_D3) { cout << hex << " " << dataBuf << endl; }

					if (broadcast) {
						CoreID = get_id_from_num(corenum);
					} else {
						CoreID = strtol(dataBuf, NULL, 16);
					}
					diag(L_D2) { cout << "Found coreID 0x" << hex << CoreID << dec << " in the S0 record." << endl; }

					addressGlobalSpaceOffset = CoreID << 20; // 12 bits for core ID, 20 bits for internal space

					fDiscoveredCoreIds.push_back(CoreID);
				}


				if (sSel == S3) {
					// Core data record

					unsigned int corenum;
					unsigned char Data2Send[MAX_BUFFER_TO_SERVER_SIZE];
					assert(byteCountHex <= MAX_BUFFER_TO_SERVER_SIZE);

					// convert text to bytes
					char *pbuf = buf + 4 + addrSize;
					for (unsigned i=0; i<byteCountHex; i++) {
						char dataBuf[3];
						dataBuf[0] = *(pbuf++);
						dataBuf[1] = *(pbuf++);
						dataBuf[2] = '\0';

						unsigned long dataHex = strtol(dataBuf, NULL, 16);
						Data2Send[i] = dataHex;

						diag(L_D4) {
							cout << hex << " " << dataBuf << " ";
							cout.width(8);
							cout.fill('0');
							cout << hex << addrtHex+i << " " << dataBuf[0] << dataBuf[1] << dec;
							cout << " +++++ " <<  dataHex << dec << endl;
						}
					}
					corenum = get_num_from_id(addrtHex >> 20);
					e_write_buf(pEpiphany, corenum, addrtHex, Data2Send, byteCountHex);
					diag(L_D3) { cout << endl; }
				}


				if (sSel == S7) {
					// End of Core section

					char startAddrofProg[9];
					strncpy (startAddrofProg, buf+4, addrSize);
					startAddrofProg[addrSize] = '\0';
					unsigned long startOfProrgram = strtol(startAddrofProg, NULL, 16);
					if (startOfProrgram != 0) {
						cerr << "Warning (Non zero _start): The start of program is detected in the address ";
						cerr << hex << startOfProrgram << endl;
						status = EPI_WARN;
					}
				}
				lineN++;
			}
		}
		delete buf;
		srefStream.close();
	}

	return status;
}

