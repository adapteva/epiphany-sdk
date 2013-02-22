
// Copyright (C) 2010,2011 Adapteva Inc.

// This file is part of the Adapteva RSP server.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <stdlib.h>     /* for exit() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <string.h>     /* for memset() */

#include <cassert>


#include "LoaderServer.h"

void DieWithError(char const *errorMessage)
{
	std::cerr << "Check the port is not allocated for other server or process" << std::endl;

	perror(errorMessage);

	exit(1);
}


#include "targetCntrlHardware.h"


#define MAXPENDING 5    /* Maximum outstanding connection requests */


enum ETargetResponse {
	WRITE_ERROR,
	READ_ERROR,
	ACCESS_OK
};

class TLoaderServer {
	unsigned currentCore;//used to map internal addresses to external
public:
	TLoaderServer() {currentCore=0;}

private :
	//! Responsible for the memory operation commands in target
	TargetControl *fTargetControl;

public :
	//------------------------------------------------------------------------------
	//! Target control module -- currently only systemC hardware model supported
	void BindTarget(TargetControl *TargetControl) { fTargetControl=TargetControl;}



private:

	TPacketFromGdbServer ParseIncomingPacket(TPacket2GdbServer *packet) {

		TPacketFromGdbServer reponsePacket;
		reponsePacket.resp = TPacketFromGdbServer::Good;

		//TODO
		///ETargetResponse retStatus=ACCESS_OK;
		//cerr  << "Parsing the packet :\n" << *packet;


		if (packet->fCommand == TPacket2GdbServer::Write) {

			fTargetControl->WriteBurst(packet->fAddr,packet->fData, packet->fDataSizeBytes);

			//cerr << "[" << fTargetControl->GetCoreID() << "]" << " " << *packet;

			//TODO check write
		}
		if (packet->fCommand == TPacket2GdbServer::Read) {


			fTargetControl->ReadBurst(packet->fAddr,reponsePacket.fData, packet->fDataSizeBytes);

			//cerr << "----" << reponsePacket.fData[4] << endl;

			//TODO check read
		}
		if (packet->fCommand == TPacket2GdbServer::Kill) {
			cerr << "Disconnecting from loader ... " << endl;

		}
		if (packet->fCommand == TPacket2GdbServer::ResumeAndExit) {
			fTargetControl->ResumeAndExit();
		}
		if (packet->fCommand == TPacket2GdbServer::Reset) {
			fTargetControl->PlatformReset();
		}

		return reponsePacket;
	}


	bool HandleTCPClient(int clntSocket) {



		TPacket2GdbServer packetFromClient;
		int recvMsgSize;   // Size of received message

		while (true) {
			/* Receive message from client */
			if ((recvMsgSize = recv(clntSocket, &packetFromClient, sizeof(packetFromClient), MSG_WAITALL)) < 0) {
				perror("--- recv() failed");
				break;
			}


			//fprintf(stderr, "recvMsgSize %d\n", recvMsgSize);
			TPacketFromGdbServer responseToClient = TPacketFromGdbServer(ParseIncomingPacket(&packetFromClient));



			/* Send received string and receive again until end of transmission */
			if (recvMsgSize == sizeof(packetFromClient)) {
				/* Echo message back to client */
				if (send(clntSocket, &responseToClient, sizeof(TPacketFromGdbServer), 0) != sizeof(TPacketFromGdbServer)) {
					perror("--- send() failed");
					break;
				}

				if (packetFromClient.fCommand == TPacket2GdbServer::Kill) {

					break;

				}
				//fprintf(stderr, "packetToSend\n");
			}
			else if (recvMsgSize == 0) {/* zero indicates end of transmission */
				fprintf(stderr, "end of transmission\n");
				break;

			} else {
				cerr << "Internal ERROR ...  assuming the message is too short to be split it " << endl;
				assert(0);
			}


		}

	}
public:
	int CreateConnection(unsigned short echoServPort) {
		int servSock;                    /* Socket descriptor for server */
		int clntSock;                    /* Socket descriptor for client */
		struct sockaddr_in echoServAddr; /* Local address */
		struct sockaddr_in echoClntAddr; /* Client address */

		unsigned int clntLen;            /* Length of client address data structure */


		/* Create socket for incoming connections */
		if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
			DieWithError("socket() failed");

		/* Construct local address structure */
		memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
		echoServAddr.sin_family = AF_INET;                /* Internet address family */
		echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
		echoServAddr.sin_port = htons(echoServPort);      /* Local port */

		/* Bind to the local address */
		if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
			DieWithError("bind() failed");

		/* Mark the socket so it will listen for incoming connections */
		if (listen(servSock, MAXPENDING) < 0)
			DieWithError("listen() failed");



		fprintf(stderr, "Listening for RSP on port %d\n", echoServPort);

		for (;;) /* Run until disconnected and detach is ON */
		{
			/* Set the size of the in-out parameter */
			clntLen = sizeof(echoClntAddr);

			/* Wait for a client to connect */
			if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
				DieWithError("accept() failed");

			/* clntSock is connected to a client! */

			printf("Handling loader from  %s\n", inet_ntoa(echoClntAddr.sin_addr));

			//cerr << " Core ID " << this->fTargetControl->GetCoreID() << endl;
			HandleTCPClient(clntSock);

			printf("Load Done %s\n", inet_ntoa(echoClntAddr.sin_addr));

			close(clntSock);    /* Close client socket */
		}

	}

};



void* CreateLoaderServer(void *ptr) {
	unsigned *port = (unsigned int *) ptr;
	assert(*port);


	TargetControl *tCntrl;
	tCntrl = new TargetControlHardware(0);

	unsigned short echoServPort= *port;     /* Server port */
	TLoaderServer loaderserver = TLoaderServer();
	loaderserver.BindTarget(tCntrl);

	loaderserver.CreateConnection(echoServPort);

}
