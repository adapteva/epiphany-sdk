/*
 * targetCntrl.h
 *
 *  Created on: May 5, 2010
 *      Author: oraikhman
 */

#ifndef TARGETCNTRL_V_H_
#define TARGETCNTRL_V_H_

//#include "systemc.h"


//-----------------------------------------------------------------------------
//! Module implementing a verilator target

//! A thread invoked by RSP memory access commands and build the entry transactions list in stimulus
//! The call is blocking. The gdb doesn't gain control until the call is done
//-----------------------------------------------------------------------------
#include "targetCntrl.h"


class TargetControlVerilator : public TargetControl {
public:

public:
	//Constructor
	TargetControlVerilator(unsigned _fCoreID){
		//FIXME : moved to coreID model
		assert(0);
	}
private:
	TargetControlVerilator():TargetControl() {
	}
public:
	// Functions to access memory. All register access on the ATDSP is via memory
	virtual bool readMem32(uint32_t addr,  uint32_t &data);
	virtual bool writeMem32(uint32_t addr, uint32_t  value);

	virtual bool readMem16(uint32_t addr,  uint16_t &data);
	virtual bool writeMem16(uint32_t addr, uint16_t  value);

	virtual bool readMem8(uint32_t addr,   uint8_t &data);
	virtual bool writeMem8(uint32_t addr,  uint8_t  value);

	//burst write
	virtual bool WriteBurst(unsigned long addr, unsigned char *buf, size_t buff_size);
	//busrt read
	virtual bool ReadBurst(unsigned long addr, unsigned char *buf, size_t buff_size) {
		return false; //no supported
	}

	//trace support for vcd Dump
	virtual bool initTrace();
	virtual bool startTrace();
	virtual bool stopTrace();

	//resume model and exit
	virtual void ResumeAndExit();

	virtual std::string GetTargetId() {return "hardware simulation";}

private :


	sc_uint<32> readMem(uint32_t addr, ETRAN_TYPE t_type);
	bool        writeMem(uint32_t addr, ETRAN_TYPE t_type, sc_uint<32> data);


	//use write/read primitive to communicate with target
	bool SendCommandToTarget(REQ_TYPE reqType, uint32_t addr, ETRAN_TYPE t_type, sc_uint<32> &data, bool wait_for_akn);


	bool SendTraceReq(TRACE_REQ_TYPE);

};

#endif /* TARGETCNTRL_H_ */
