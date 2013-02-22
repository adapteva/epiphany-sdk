/*
  File: GdbServerSC.h

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

// Based on SystemC GDB RSP server definition

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

//  4 May 09: Jeremy Bennett. Initial version based on the Embecosm GDB server
//                            for Verilator implementation.

// Embecosm Subversion Identification
// ==================================

// $Id: GdbServerSC.h 1286 2013-01-02 19:09:49Z ysapir $
//-----------------------------------------------------------------------------

#ifndef GDB_SERVER_SC__H
#define GDB_SERVER_SC__H

#include <inttypes.h>

#include <stdlib.h>
typedef uint8_t  sc_uint_2;
typedef uint8_t  sc_uint_3;
typedef uint8_t  sc_uint_4;
typedef uint8_t  sc_uint_6;
typedef uint8_t  sc_uint_8;
typedef uint16_t sc_uint_16;
typedef uint32_t sc_uint_32;
typedef uint64_t sc_uint_64;
#include <assert.h>
#include <string.h>

#include "RspConnection.h"
#include "MpHash.h"
#include "RspPacket.h"

#include "maddr_defs.h"
#include "targetCntrl.h"
//-----------------------------------------------------------------------------
//! Module implementing a GDB RSP server.

//! A thread listens for RSP requests, which are converted to requests to read
//! and write registers or memory or control the CPU in the debug unit
//-----------------------------------------------------------------------------


class GdbServerSC {

private :
	//! Responsible for the memory operation commands in target
	TargetControl *fTargetControl;

public :

	//! Get CoreID from target
	unsigned GetAttachedTargetCoreId() {
		assert(fTargetControl);
		return fTargetControl->GetAttachedCoreId();
	}

public:
	//-----------------------------------------------------------------------------
	//! Constructor for the GDB RSP server.

	//! We create a SC_THREAD which will act as the listener. Must be a
	//! thread, since we need to wait for the actions to complete.
	//! @param[in] name      Name of this module, passed to the parent constructor.
	//! @param[in] rspPort   The TCP/IP port number to listen on

	int rspPort;

	GdbServerSC(int _rspPort){

		fTargetControl = 0;

		rspPort = _rspPort;

		// Create subsidiary classes
		pkt    = new RspPacket(RSP_PKT_MAX);
		rsp    = new RspConnection(rspPort);
		mpHash = new MpHash();

		fIsTargetRunning=false;

	}

	~GdbServerSC();

private :
	//he dgbserver will send the resume command to the target after gdb client kills the debug session

//	bool fResumeOnDetach;

private:

	//! Definition of GDB target signals.

	//! Data taken from the GDB 6.8 source. Only those we use defined here.
	enum TargetSignal {
		// Used some places (e.g. stop_signal) to record the concept that there is no signal.
		TARGET_SIGNAL_NONE  = 0,
		TARGET_SIGNAL_FIRST = 0,
		TARGET_SIGNAL_HUP   = 1,
		TARGET_SIGNAL_INT   = 2,
		TARGET_SIGNAL_QUIT  = 3,
		TARGET_SIGNAL_ILL   = 4,
		TARGET_SIGNAL_TRAP  = 5,
		TARGET_SIGNAL_ABRT  = 6,
		TARGET_SIGNAL_EMT   = 7,
		TARGET_SIGNAL_FPE   = 8,
		TARGET_SIGNAL_KILL  = 9,
		TARGET_SIGNAL_BUS   = 10,
		TARGET_SIGNAL_SEGV  = 11,
		TARGET_SIGNAL_SYS   = 12,
		TARGET_SIGNAL_PIPE  = 13,
		TARGET_SIGNAL_ALRM  = 14,
		TARGET_SIGNAL_TERM  = 15,
	};


	enum SPECIAL_INSTR_OPCODES {
		IDLE_OPCODE = 0x1b2
	};

	enum EPIPHANY_EX_CASE {
		E_UNALIGMENT_LS = 0x2,
		E_FPU           = 0x3,
		E_UNIMPL        = 0x4
	};


	//! Number of general purpose registers (GPRs).
	static const int ATDSP_NUM_GPRS        = 64;

	//! Number of Special Core Registers (SCRs).
	static const int ATDSP_NUM_SCRS        = 32;

	//! Number of Special Core Registers (SCRs), first group
	static const int ATDSP_NUM_SCRS_0      = 16;

	//! Number of Special Core Registers (SCRs), DMA group
	static const int ATDSP_NUM_SCRS_1      = 16;

	//! Number of raw registers used.
	static const int ATDSP_NUM_REGS        = ATDSP_NUM_GPRS + ATDSP_NUM_SCRS;

	//! Total number of pseudo registers (none in this implementation).
	static const int ATDSP_NUM_PSEUDO_REGS = 0;

	//! Total of registers used.
	static const int ATDSP_TOTAL_NUM_REGS  = ATDSP_NUM_REGS + ATDSP_NUM_PSEUDO_REGS;

	//! Maximum size of RSP packet. Enough for all the registers as hex
	//! characters (8 per reg) + 1 byte end marker.
	static const int RSP_PKT_MAX = ATDSP_TOTAL_NUM_REGS * 8 + 1;

	// Values for the Debug register. The value can be read to determine the
	// state, or written to force the state.
	enum AtdspDebugStates {
		ATDSP_DEBUG_RUN           = 0, //!< Is running/should run
		ATDSP_DEBUG_HALT          = 1, //!< Is halted/should halt
		ATDSP_DEBUG_EMUL_MODE_IN  = 2, //! Enter Emulation mode, can access to all registers
		ATDSP_DEBUG_EMUL_MODE_OUT = 3, //! Leave Emulation mode
	};
	// Need to check the outstanding transaction status, the core is in the debug state if no pending transaction to external memory
	enum AtdspOutStandingTransationPendingState {
		ATDSP_OUT_TRAN_TRUE  = 1,
		ATDSP_OUT_TRAN_FALSE = 0
	};


	//! Number entries in IVT table
	static const uint32_t ATDSP_NUM_ENTRIES_IN_IVT = 8;

	//! Location in core memory where the GPRs are mapped:
	static const uint32_t ATDSP_GPR_MEM_BASE = 0x000ff000;

	//! Location in core memory where the SCRs are mapped:
	static const uint32_t ATDSP_SCR_MEM_BASE = 0x000ff100;

	//! GDB register number for the first GPR. All GPR's form a contiguous
	//! sequence form here.
	static const int ATDSP_R0_REGNUM   =  0;

	//! GDB register number for a single word result.
	static const int ATDSP_RV_REGNUM   = ATDSP_R0_REGNUM;

	//! GDB register number for the stack pointer
	static const int ATDSP_SP_REGNUM   = ATDSP_GPR_SP;

	//! GDB register number for the program counter
	static const int ATDSP_PC_REGNUM   = ATDSP_NUM_GPRS + ATDSP_SCR_PC;

	//! GDB register number for the status register
	static const int ATDSP_SR_REGNUM   = ATDSP_NUM_GPRS + ATDSP_SCR_STATUS;

	//! GDB register number for the frame pointer
	static const int ATDSP_FP_REGNUM   = ATDSP_GPR_FP;

	//! GDB register number for the link register
	static const int ATDSP_LR_REGNUM   = ATDSP_GPR_LR;

	//! GDB register number for the interrupt return register
	static const int ATDSP_IRET_REGNUM = ATDSP_NUM_GPRS + ATDSP_SCR_IRET;

	//! GDB register number for the static base register
	static const int ATDSP_SB_REGNUM   = ATDSP_GPR_SB;

	//! GDB register number for the stack limit register
	static const int ATDSP_SL_REGNUM   = ATDSP_GPR_SL;

	//! GDB register number for the Inter-procedure call register
	static const int ATDSP_IP_REGNUM   = ATDSP_GPR_IP;

	// Bits in the status register
	static const uint32_t ATDSP_SCR_STATUS_STALLED = 0x00000001; //!< Stalled

	// 16-bit instructions for ATDSP
	static const uint16_t ATDSP_NOP_INSTR  = 0x01a2; //!< NOP instruction
//	static const uint16_t ATDSP_IDLE_INSTR = 0x01b2; //!< IDLE instruction
	static const uint16_t ATDSP_BKPT_INSTR = 0x01c2; //!< BKPT instruction
	static const uint16_t ATDSP_RTI_INSTR  = 0x01d2; //!< RTI instruction
	static const uint16_t ATDSP_TRAP_INSTR = 0x03e2; //!< TRAP instruction

	// Instruction lengths
	static const int ATDSP_INST16LEN = 2; //!< Short instruction
	static const int ATDSP_INST32LEN = 4; //!< Long instruction

	//! Size of the breakpoint instruction (in bytes)
	static const int ATDSP_BKPT_INSTLEN = ATDSP_INST16LEN;
	//! Size of the trap instruction (in bytes)
	static const int ATDSP_TRAP_INSTLEN = ATDSP_BKPT_INSTLEN;

	//! Interrupt vector bits
	static const uint32_t ATDSP_EXCEPT_RESET = 0x00000001;
	static const uint32_t ATDSP_EXCEPT_NMI   = 0x00000002;
	static const uint32_t ATDSP_EXCEPT_FPE   = 0x00000004;
	static const uint32_t ATDSP_EXCEPT_IRQH  = 0x00000008;
	static const uint32_t ATDSP_EXCEPT_TIMER = 0x00000010;
	static const uint32_t ATDSP_EXCEPT_DMA   = 0x00000020;
	static const uint32_t ATDSP_EXCEPT_IRQL  = 0x00000040;
	static const uint32_t ATDSP_EXCEPT_SWI   = 0x00000080;

	//! Interrupt vector locations
	static const uint32_t ATDSP_VECTOR_RESET = 0x00000000;
	static const uint32_t ATDSP_VECTOR_NMI   = 0x00000004;
	static const uint32_t ATDSP_VECTOR_FPE   = 0x00000008;
	static const uint32_t ATDSP_VECTOR_IRQH  = 0x0000000c;
	static const uint32_t ATDSP_VECTOR_TIMER = 0x00000010;
	static const uint32_t ATDSP_VECTOR_DMA   = 0x00000014;
	static const uint32_t ATDSP_VECTOR_IRQL  = 0x00000018;
	static const uint32_t ATDSP_VECTOR_SWI   = 0x0000001c;

	enum EDebugState {CORE_RUNNING, CORE_ON_DEBUG};

	//! Thread ID used by ATDSP
	static const int ATDSP_TID = 1;


	//! Our associated RSP interface (which we create)
	RspConnection *rsp;

	//! The packet pointer. There is only ever one packet in use at one time, so
	//! there is no need to repeatedly allocate and delete it.
	RspPacket *pkt;

	//IVT save buffer
	unsigned char fIVTSaveBuff[ATDSP_NUM_ENTRIES_IN_IVT*4];

	//! Hash table for matchpoints
	MpHash *mpHash;

	// Main RSP request handler
	void rspClientRequest();

	// Handle the various RSP requests
	void rspReportException(unsigned stoppedPC, unsigned threadID, unsigned exCause);
	void rspContinue();
	void rspContinue(uint32_t except);
	void rspContinue(uint32_t addr, uint32_t except);
	void rspReadAllRegs();
	void rspWriteAllRegs();
	void rspReadMem();
	void rspWriteMem();
	void rspReadReg();
	void rspWriteReg();
	void rspQuery();
	void rspCommand();
	void rspSet();
	void rspRestart();
	void rspStep();
	void rspStep(uint32_t except);
	void rspStep(uint32_t addr, uint32_t except);
	void targetResume();
	void rspVpkt();
	void rspWriteMemBin();
	void rspRemoveMatchpoint();
	void rspInsertMatchpoint();

	void rspQThreadExtraInfo();
	void rspThreadSubOperation();
	void rspFileIOreply();
	void rspSuspend();

	void rspAttach();
	void rspDetach();

	// Convenience functions to control and report on the CPU
	void targetSWReset();
	void targetHWReset();

	// Convenience wrappers for getting particular registers, which are really
	// memory mapped and special care of conversion between internal <-> external addresses

	//Read the of mesh core status register
	uint32_t readCoreStatus();
	// Read the value of mesh coreID register
	uint32_t readCoreId();

	uint32_t readPc();
	void     writePc(uint32_t addr);

	uint32_t readLr();
	void     writeLr(uint32_t addr);

	uint32_t readSp();
	void     writeSp(uint32_t addr);

	uint32_t readFp();
	void     writeFp(uint32_t addr);


	uint32_t readGpr(unsigned int gprNum);
	void     writeGpr(unsigned int gprNum, uint32_t value);

	uint32_t readScrGrp0(unsigned int scrNum);
	void     writeScrGrp0(unsigned int scrNum, uint32_t value);

	uint32_t readScrDMA(unsigned int scrNum);
	void     writeScrDMA(unsigned int scrNum, uint32_t value);


	void putBreakPointInstruction(unsigned long);
	bool isHitInBreakPointInstruction(unsigned long);
	bool isTargetInDebugState();
	bool isTargetInIldeState();
	bool isTargetExceptionState(unsigned &);
	bool targetHalt();

	void saveIVT();
	void restoreIVT();

private:
	//! thread control
	void NanoSleepThread(unsigned long timeout);

public:


	// main loop for core
	void rspServer(TargetControl *TargetControl);


private :
	void redirectSdioOnTrap(sc_uint_6 trapNumber);

private :
	//used in cont command to support CTRL-C from gdb client
	bool fIsTargetRunning;


public:

	//FIXME
	//release the coreID, the target core can be selected by other gdb client
	void ReleaseGdbCmdSelectedCoreId() {};

}; // GdbServerSC()

extern int debug_level;

// YS - provide the SystemC equivalent to the bit range selection operator.
inline sc_uint_8  getfield(sc_uint_8   x, int _lt, int _rt);
inline sc_uint_16 getfield(sc_uint_16  x, int _lt, int _rt);
inline sc_uint_32 getfield(sc_uint_32  x, int _lt, int _rt);
inline sc_uint_64 getfield(sc_uint_64  x, int _lt, int _rt);
inline void       setfield(sc_uint_32 &x, int _lt, int _rt, sc_uint_32 val);

#endif // GDB_SERVER_SC__H
