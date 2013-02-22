/*
  File: parseSrecFile.c

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "e-hal.h"
#include "e-loader.h"

#define diag(vN)   if (e_load_verbose >= vN)

#define MAX_NUM_WRITE_PACKETS 256
#define MAX_NUM_READ_PACKETS   64
#define MAX_BUFFER_TO_SERVER_SIZE (MAX_NUM_WRITE_PACKETS * 8)

#define BUFSIZE 1000  // should be sufficient for SREC record

extern unsigned int DiscoveredCoreIDs[EPI_CORES];
extern unsigned int DiscoveredCoreIDidx;

unsigned long addressGlobalSpaceOffset = 0;

extern int e_load_verbose;
extern FILE *fd;


int parseAndSendSrecFile(char *srecFile, Epiphany_t *pEpiphany, bool broadcast) {

	typedef enum {S0, S3, S7} SrecSel;
	FILE *srefStream;
	int corenum, numcores, i;
	int status = EPI_OK;
	char buf[BUFSIZE];

//	diag(L_D1) { fprintf(fd, "Parser verbosity level = %d, EPI_CORES = %d\n", e_load_verbose, EPI_CORES); }

	if (broadcast) {
		numcores = pEpiphany->num_cores;
		diag(L_D1) { fprintf(fd, "Loading chip in broadcast mode.\n"); }
	} else {
		numcores = 1;
		diag(L_D1) { fprintf(fd, "Loading chip in non-broadcast mode; numcores = %d.\n", numcores); }
	}

	for (corenum=0; corenum<numcores; corenum++) {

		srefStream = fopen(srecFile, "r");
		if (srefStream == NULL) {
			fprintf(fd, "Error: Can't open SREC file: %s\n", srecFile);
			return EPI_ERR;
		} else
			; // fprintf(fd, "Opening SREC file: %s\n", srecFile);

		unsigned lineN = 0;

		while(!feof(srefStream) && !ferror(srefStream))
		{
			fgets(buf, BUFSIZE, srefStream);

			if (!feof(srefStream) && !ferror(srefStream)) {
				diag(L_D3) { fprintf(fd, "\n%s", buf); }

				// RECORD TYPE
				SrecSel sSel;
				unsigned addrSize;

				if (buf[0] != 'S') {
					fprintf(fd, "Error: Invalid record format in SREC file line ");
					fprintf(fd, "%d: \"%s\"\n", lineN, buf);
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
					fprintf(fd, "Error: Invalid record types (valid types are S0, S3 and S7) in SREC file line ");
					fprintf(fd, "%d: \"%s\"\n", lineN, buf);
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
					byteCountHex = byteCountHex - (addrSize/2) /* addr */ - 1 /* checksum */;
					diag(L_D3) { fprintf(fd, "Record length = %d\n", byteCountHex); }
				} else {
					fprintf(fd, "Error: Wrong record format in SREC file line ");
					fprintf(fd, "%d: \"%s\"\n", lineN, buf);
					continue;
				}


				// ADDRESS
				unsigned long addrtHex;

				char addrBuf[9];
				strncpy(addrBuf, buf+4, addrSize);
				addrBuf[addrSize] = '\0';
				addrtHex = strtol(addrBuf, NULL, 16);
				if (broadcast) {
					addrtHex = (addrtHex & 0x000FFFFF) | (e_get_id_from_num(corenum) << 20);
				} else {
					if (addrtHex < (1<<20)) { // make internal to global conversion
						addrtHex += addressGlobalSpaceOffset;
					}
				}
				diag(L_D3) { fprintf(fd, "Address: 0x%08x\n", (unsigned int) addrtHex); }


				// DATA
				if (sSel == S0) {
					// Start of Core section
					unsigned long CoreID;

					char dataBuf[5];
					strncpy(dataBuf, buf+4, 4);
					dataBuf[4] = '\0';

					diag(L_D3) { fprintf(fd, " %x\n", (unsigned int) dataBuf); }
//					diag(L_D3) { fprintf(fd, " %02x %c\n", (unsigned int) dataBuf[0], dataBuf[0]); }
//					diag(L_D3) { fprintf(fd, " %02x %c\n", (unsigned int) dataBuf[1], dataBuf[1]); }
//					diag(L_D3) { fprintf(fd, " %02x %c\n", (unsigned int) dataBuf[2], dataBuf[2]); }
//					diag(L_D3) { fprintf(fd, " %02x %c\n", (unsigned int) dataBuf[3], dataBuf[3]); }
//					diag(L_D3) { fprintf(fd, " %02x %c\n", (unsigned int) dataBuf[4], dataBuf[4]); }

					if (broadcast) {
						CoreID = e_get_id_from_num(corenum);
					} else {
						CoreID = strtol(dataBuf, NULL, 16);
					}
					diag(L_D2) { fprintf(fd, "Found coreID 0x%03x in the S0 record.\n", (unsigned int) CoreID); }

					addressGlobalSpaceOffset = CoreID << 20; // 12 bits for core ID, 20 bits for internal space

					DiscoveredCoreIDs[DiscoveredCoreIDidx] = CoreID;
					DiscoveredCoreIDidx++;
				}


				if (sSel == S3) {
					// Core data record

					unsigned int corenum;
					unsigned char Data2Send[MAX_BUFFER_TO_SERVER_SIZE];
					assert(byteCountHex <= MAX_BUFFER_TO_SERVER_SIZE);
					diag(L_D3) { fprintf(fd, "Copying the data (%d bytes)", byteCountHex); }

					// convert text to bytes
					char *pbuf = buf + 4 + addrSize;
					for (i=0; i<byteCountHex; i++) {
						char dataBuf[3];
						dataBuf[0] = *(pbuf++);
						dataBuf[1] = *(pbuf++);
						dataBuf[2] = '\0';

						unsigned long dataHex = strtol(dataBuf, NULL, 16);
						Data2Send[i] = dataHex;

						diag(L_D4) {
							fprintf(fd, " %s", dataBuf); fflush(stdout);
						}
					}
					corenum = e_get_num_from_id(addrtHex >> 20);
					diag(L_D3) { fprintf(fd, " to core no. %d   dev=%p\n", corenum, pEpiphany); }
					e_write_buf(pEpiphany, corenum, addrtHex, Data2Send, byteCountHex);
					diag(L_D3) { fprintf(fd, "\n"); }
				}


				if (sSel == S7) {
					// End of Core section

					char startAddrofProg[9];
					strncpy(startAddrofProg, buf+4, addrSize);
					startAddrofProg[addrSize] = '\0';
					unsigned long startOfProrgram = strtol(startAddrofProg, NULL, 16);
					if (startOfProrgram != 0) {
						fprintf(fd, "Warning (Non zero _start): The start of program is detected in the address ");
						fprintf(fd, "%x\n", (unsigned int) startOfProrgram);
						status = EPI_WARN;
					}
				}
				lineN++;
			}
		}

		fclose(srefStream);
	}

	return status;
}

