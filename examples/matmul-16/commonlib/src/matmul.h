/*
  matmul.h

  Copyright (C) 2012 Adapteva, Inc.
  Contributed by Yaniv Sapir <yaniv@adapteva.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program, see the file COPYING.  If not, see
  <http://www.gnu.org/licenses/>.
*/


#ifndef __MATMUL_H__
#define __MATMUL_H__

#include <stdint.h>

#define _Nchips 4                  // # of chips in operand matrix side
#define _Nside  4                  // # of cores in chip side
#define _Ncores (_Nside * _Nside)  // Num of cores = 16
#define _Score  32                 // side size of per-core sub-submatrix (max 32)
#define _Schip  (_Score * _Nside)  // side size of per-chip submatrix
#define _Smtx   (_Schip * _Nchips) // side size of operand matrix

#define _Nbanks 4                  // Num of SRAM banks on core

#define _BankP  0
#define _BankA  1
#define _BankB  2
#define _BankC  3
#define _PING   0
#define _PONG   1


typedef struct {
	int    coreID;
	int    corenum;
	int    row;
	int    col;
	int    coreIDh;
	int    coreIDv;
	int    coreIDn;

	int    go_sync;      // The "go" signal from prev core
	int    synch;        // Sync with horizontal peer core
	int    syncv;        // Sync with vertical peer core
	int    dma_sync;     // Sync external DMA to avoid thrashing
	int   *tgt_go_sync;  // ptr to go_sync of next core
	int   *tgt_synch;    // ptr to synch of target neighbor
	int   *tgt_syncv;    // ptr to syncv of target neighbor
	int   *tgt_dma_sync; // ptr to dma_sync of target neighbor

	int    mystate;

	void  *bank[_Nbanks][2];   // Ping Pong Bank local space pointer
	void  *tgt_bk[_Nbanks][2]; // Target Bank for matrix rotate in global space
	
	int    pingpong; // Ping-Pong bank select indicator

	int    count;
} core_t;


typedef struct {
	int      go[_Ncores]; // Call for matmul function
	int      ready;       // Core is ready after reset
	int      clocks;      // Cycle count
} mbox_t;


typedef struct {
	float  A[_Smtx * _Smtx]; // Global A matrix 
	float  B[_Smtx * _Smtx]; // Global B matrix 
	float  C[_Smtx * _Smtx]; // Global C matrix 
	mbox_t core;
} shared_buf_t;


typedef struct {
	void   *pBase;       // ptr to base of shared buffers
	float  *pA;          // ptr to global A matrix
	float  *pB;          // ptr to global B matrix
	float  *pC;          // ptr to global C matrix
	mbox_t *pCore;       // ptr to cores mailbox
} shared_buf_ptr_t;


#endif // __MATMUL_H__
