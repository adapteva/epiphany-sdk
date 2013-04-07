#include <stdio.h>
#include <stddef.h>

#include "matmul.h"

// Calculate the offsets of the core structure members.
// This program should be compiled using e-gcc and run using e-run.

/*
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

	void  *bank[_Nbanks][2];   // Ping Pong Bank local space pointer
	void  *tgt_bk[_Nbanks][2]; // Target Bank for matrix rotate in global space

	int    pingpong; // Ping-Pong bank select indicator

	int    count;
} core_t;
*/

void print_offsets()
{
	printf("Size of core struct: %lu bytes\n", sizeof(core_t));
	printf("\n");
	printf("offset_of(coreID)       = 0x%03x\n", (unsigned) offsetof(core_t, coreID));
	printf("offset_of(corenum)      = 0x%03x\n", (unsigned) offsetof(core_t, corenum));
	printf("offset_of(row)          = 0x%03x\n", (unsigned) offsetof(core_t, row));
	printf("offset_of(col)          = 0x%03x\n", (unsigned) offsetof(core_t, col));
	printf("offset_of(coreIDh)      = 0x%03x\n", (unsigned) offsetof(core_t, coreIDh));
	printf("offset_of(coreIDv)      = 0x%03x\n", (unsigned) offsetof(core_t, coreIDv));
	printf("offset_of(coreIDn)      = 0x%03x\n", (unsigned) offsetof(core_t, coreIDn));
	printf("\n");
	printf("offset_of(go_sync)      = 0x%03x\n", (unsigned) offsetof(core_t, go_sync));
	printf("offset_of(synch)        = 0x%03x\n", (unsigned) offsetof(core_t, synch));
	printf("offset_of(syncv)        = 0x%03x\n", (unsigned) offsetof(core_t, syncv));
	printf("offset_of(dma_sync)     = 0x%03x\n", (unsigned) offsetof(core_t, dma_sync));
	printf("\n");
	printf("offset_of(tgt_go_sync)  = 0x%03x\n", (unsigned) offsetof(core_t, tgt_go_sync));
	printf("offset_of(tgt_synch)    = 0x%03x\n", (unsigned) offsetof(core_t, tgt_synch));
	printf("offset_of(tgt_syncv)    = 0x%03x\n", (unsigned) offsetof(core_t, tgt_syncv));
	printf("offset_of(tgt_dma_sync) = 0x%03x\n", (unsigned) offsetof(core_t, tgt_dma_sync));
	printf("\n");
	printf("offset_of(bank)         = 0x%03x\n", (unsigned) offsetof(core_t, bank));
	printf("offset_of(tgt_bank)     = 0x%03x\n", (unsigned) offsetof(core_t, tgt_bk));
	printf("\n");
	printf("offset_of(pingpong)     = 0x%03x\n", (unsigned) offsetof(core_t, pingpong));
	printf("offset_of(count)        = 0x%03x\n", (unsigned) offsetof(core_t, count));

	return;
}


int main()
{
	print_offsets();

	return 0;
}
