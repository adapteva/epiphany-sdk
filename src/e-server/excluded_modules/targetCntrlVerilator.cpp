/*
 * targetCntrl.cpp
 *
 *  Created on: May 5, 2010
 *      Author: oraikhman
 */

#include "targetCntrlVerilator.h"

#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <named_fifo.h>

#include <pthread.h>
pthread_mutex_t targetControlAccess_m = PTHREAD_MUTEX_INITIALIZER;
int             pipe_id_0, pipe_id_1;




//Establish communication between gdbserver and target system
bool /*TargetControlVerilator::*/ InitNamedPipe() {

	signal(SIGPIPE, SIG_IGN);

	pipe_id_0 = open(FIFO_0, O_RDWR);

	if (pipe_id_0<0)
	{
		if (errno == ENXIO) {
			fprintf(stderr, "The target system has not opened the pipe yet\n");
		}
		else {
			perror(FIFO_0);
		}

		fprintf(stderr, "Please run the hardware model driver first in the same folder as the gdbserver\n");
		exit(1);
	}

	pipe_id_1=open(FIFO_1, O_RDWR);//O_RONLY

	if (pipe_id_1<0) {
		if (errno == ENXIO) {
			fprintf(stderr,"The target system has not opened the pipe yet\n");
		}
		else {
			perror(FIFO_1);
		}
		fprintf(stderr, "Please run the hardware model driver first in the same folder as the gdbserver\n");
		exit(1);
	}
}



/*
 *  Please see for protocol detail the namedPipeServer.cpp
 */

bool TargetControlVerilator::SendCommandToTarget(REQ_TYPE regType, uint32_t addr, ETRAN_TYPE t_type, sc_uint<32>& data, bool wait_for_akn) {

	//
	unsigned long buf_p[5];

	buf_p[0]=regType;

	//transaction size
	buf_p[1]=t_type;//b/s/l
	//address (can be internal)
	buf_p[2]=addr;
	//data to write
	buf_p[3]=data;
	//core id to make internal to external conversion if necessary
	buf_p[4]=GetAttachedCoreId();

	ssize_t io_op_res;

	io_op_res=write(pipe_id_0,buf_p, sizeof(buf_p));
	if (io_op_res<0) {
		perror("abnormal error on writing to pipe");
		exit(1);
	}

	if (wait_for_akn) {
		io_op_res=read(pipe_id_1,buf_p, sizeof(buf_p));
		if (io_op_res<0) {
			perror("abnormal error on reading from pipe");
			exit(1);
		}
	} else {
		//result true;
		buf_p[0]=0;
	}

	bool res = (buf_p[0]==0);
	data = buf_p[1];
	return res;

}


sc_uint<32> TargetControlVerilator::readMem (uint32_t addr, ETRAN_TYPE t_type) {

	sc_uint<32> data=0;//TODO check data
	pthread_mutex_lock(&targetControlAccess_m);
	bool res = SendCommandToTarget(READ_, addr, t_type, data, true);
	pthread_mutex_unlock(&targetControlAccess_m);
	return data;

}

bool TargetControlVerilator::writeMem(uint32_t addr, ETRAN_TYPE t_type, sc_uint<32> data) {

	//return true;
	pthread_mutex_lock(&targetControlAccess_m);
	bool res = SendCommandToTarget(WRITE_, addr, t_type, data, true);
	pthread_mutex_unlock(&targetControlAccess_m);
	return res;
}




// Functions to access memory. All register access on the ATDSP is via memory
bool TargetControlVerilator::readMem32(uint32_t addr, uint32_t& data) {
	data = (readMem(addr, WORD_))(31, 0);
	return true;
}

bool TargetControlVerilator::readMem16(uint32_t addr, uint16_t& data) {
	data = (readMem(addr, SHORT_))(15, 0);
	return true;
}

bool TargetControlVerilator::readMem8(uint32_t addr, uint8_t& data) {
	data = (readMem(addr, BYTE_))(7, 0);
	return true;
}




bool TargetControlVerilator::writeMem32(uint32_t addr, uint32_t value) {
	sc_uint<32> v = value;
	return writeMem(addr, WORD_, v);
}


bool TargetControlVerilator::writeMem16(uint32_t addr, uint16_t value){
	sc_uint<32> v=0;
	v(15,0) = value;
	return writeMem(addr, SHORT_, v);
}


bool TargetControlVerilator::writeMem8(uint32_t addr, uint8_t value){
	sc_uint<32> v=0;
	v(7,0) = value;
	return writeMem(addr, BYTE_, v);
}

//burst write
bool TargetControlVerilator::WriteBurst(unsigned long addr, unsigned char *buf, size_t buff_size) {
	bool res = false;
	for (unsigned int i = 0; i < buff_size; i++) {
		res = res && writeMem8(i + addr, buf[i]);
	}
	return res;
}



bool TargetControlVerilator::SendTraceReq(TRACE_REQ_TYPE traceReq) {
	sc_uint<32> data=0;
	pthread_mutex_lock(&targetControlAccess_m);
	bool res = SendCommandToTarget(TRACE_ME, traceReq, BYTE_, data, false /* don't need wait for akn */);
	pthread_mutex_unlock(&targetControlAccess_m);
	return res;
}

//trace support for vcd Dump
bool TargetControlVerilator::initTrace() {
	return SendTraceReq(INIT_TRACE);
}
bool TargetControlVerilator::startTrace() {
	return SendTraceReq(START_TRACE);
}
bool TargetControlVerilator::stopTrace() {
	return SendTraceReq(STOP_TRACE);
}


void TargetControlVerilator::ResumeAndExit() {
	sc_uint<32> data=0;
	pthread_mutex_lock(&targetControlAccess_m);
	bool res = SendCommandToTarget(RESUME_ME, 0, BYTE_, data, false /* don't need wait for akn */);
	pthread_mutex_unlock(&targetControlAccess_m);
}

