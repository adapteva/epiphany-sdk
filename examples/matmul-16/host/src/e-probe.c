#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

typedef unsigned int e_coreid_t;

#include "e-hal.h"
#include "matmul.h"

//#define _HI(x) ((unsigned int) (x >> 32))
//#define _LO(x) ((unsigned int) (x >>  0))

int  main(int argc, char *argv[]);

#define nDREGS 64

#define NUM_ROWS EPI_ROWS
#define NUM_COLS EPI_COLS
#define COLS_IN_GLOBAL_MESH 64
#define baseid   0x808
#define baseaddr 0x7018
#define dregsaddr 0xf0000
#define sregsaddr 0xf0000
//#define sregsaddr 0xf0400

#if 0
typedef uint32_t eptr;

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

	// probe-padding

	void  *bank[_Nbanks][2];   // Ping Pong Bank local space pointer
	void  *tgt_bk[_Nbanks][2]; // Target Bank for matrix rotate in global space

	int    pingpong; // Ping-Pong bank select indicator

	int    count;
} core_l;


typedef struct {
	int      go[_Ncores]; // Call for matmul function
	int      ready;       // Core is ready after reset
	int      clocks;      // Cycle count
} mbox_l;


typedef struct {
	float  A[_Smtx * _Smtx]; // Global A matrix
	float  B[_Smtx * _Smtx]; // Global B matrix
	float  C[_Smtx * _Smtx]; // Global C matrix
	mbox_l core;
} shared_buf_l;
#endif

int coredata[34];
int mailbox[_Ncores+2];
//int mailbox[34];

typedef struct {
	int dim1;
	int dim2;
	int dim3;
	int pad;
	char str[255];
	char fmt[15];
} core_str_t;

typedef core_str_t mbox_str_t;

core_str_t core_str[] = {
	{1, 1, 1, 0, "coreID        ", "0x%03x  "},
	{1, 1, 1, 0, "corenum       ", "%u  "},
	{1, 1, 1, 0, "row           ", "%u  "},
	{1, 1, 1, 0, "col           ", "%u  "},
	{1, 1, 1, 0, "coreIDh       ", "0x%03x  "},
	{1, 1, 1, 0, "coreIDv       ", "0x%03x  "},
	{1, 1, 1, 0, "coreIDn       ", "0x%03x\n"},

	{1, 1, 1, 0, "go_sync       ", "%u  "},
	{1, 1, 1, 0, "synch         ", "%u  "},
	{1, 1, 1, 0, "syncv         ", "%u  "},
	{1, 1, 1, 0, "dma_sync      ", "%u  "},
	{1, 1, 1, 0, "tgt_go_sync   ", "0x%08x  "},
	{1, 1, 1, 0, "tgt_synch     ", "0x%08x  "},
	{1, 1, 1, 0, "tgt_syncv     ", "0x%08x  "},
	{1, 1, 1, 0, "tgt_dma_sync  ", "0x%08x\n"},
//	{1, 1, 1, 1, "tgt_dma_sync  ", "0x%08x\n"},
	{1, 1, 1, 0, "mystate       ", "%u\n"},

//	{0, 0, 0, 0, "", ""},

	{_Nbanks, 2, 1, 0, "bank[Nb][2]   ", "0x%08x  "},
	{_Nbanks, 2, 1, 0, "tgt_bk[Nb][2] ", "0x%08x  "},

	{1, 1, 1, 0, "pingpong      ", "%u  "},
	{1, 1, 1, 0, "count         ", "%u  "},

	{0, 0, 0, 0, "", ""},
};



mbox_str_t mbox_str[] = {
	{_Ncores/_Nside, _Ncores/_Nside, 1, 0, "go[Nc]        ", "%u  "},

	{        1,         1, 1, 0, "ready         ", "%u  "},
	{        1,         1, 1, 0, "clocks        ", "%u  "},

	{0, 0, 0, 0, "", ""},
};



typedef enum {
	eCONFIG       = 0,
	eSTATUS,
	ePC,
	eIRET,
	eIMASK,
	eILAT,
	eILATST,
	eILATCL,
	eIPEND,

	eCTIMER0      = 9,
	eCTIMER1,
	eMEMPROTECT,
	eCOREID,

	eDMA0CONFIG   = 13,
	eDMA0STRIDE,
	eDMA0COUNT,
	eDMA0SRCADDR,
	eDMA0DSTADDR,
	eDMA0AUTO0,
	eDMA0AUTO1,
	eDMA0STATUS,

	eDMA1CONFIG   = 21,
	eDMA1STRIDE,
	eDMA1COUNT,
	eDMA1SRCADDR,
	eDMA1DSTADDR,
	eDMA1AUTO0,
	eDMA1AUTO1,
	eDMA1STATUS,

	eEOF,
} core_regs_e;

typedef struct {
	unsigned int a;
	uint32_t     v;
	char         n[16];
} core_regs_t;

uint32_t    DREGS[nDREGS];
core_regs_t SREGS[eEOF] = {
		{0x0400, 0,   "CONFIG     "}, {0x0404, 0, "STATUS     "}, {0x0408, 0, "PC         "},
		{0x0420, 0, "\nIRET       "}, {0x0424, 0, "IMASK      "},
		{0x0428, 0, "\nILAT       "}, {0x042c, 0, "ILATST     "}, {0x0430, 0, "ILATCL     "}, {0x0434, 0, "IPEND      "},
		{0x0438, 0, "\nCTIMER0    "}, {0x043c, 0, "CTIMER1    "}, {0x0608, 0, "MEMPROTECT "}, {0x0704, 0, "COREID     "},
		{0x0500, 0, "\nDMA0CONFIG "}, {0x0504, 0, "DMA0STRIDE "}, {0x0508, 0, "DMA0COUNT  "}, {0x050c, 0, "DMA0SRCADDR"},
		{0x0510, 0, "\nDMA0DSTADDR"}, {0x0514, 0, "DMA0AUTO0  "}, {0x0518, 0, "DMA0AUTO1  "}, {0x051c, 0, "DMA0STATUS "},
		{0x0520, 0, "\nDMA1CONFIG "}, {0x0524, 0, "DMA1STRIDE "}, {0x0528, 0, "DMA1COUNT  "}, {0x052c, 0, "DMA1SRCADDR"},
		{0x0530, 0, "\nDMA1DSTADDR"}, {0x0534, 0, "DMA1AUTO0  "}, {0x0538, 0, "DMA1AUTO1  "}, {0x053c, 0, "DMA1STATUS "},
};



extern e_platform_t e_platform;

int main(int argc, char *argv[])
{
	e_epiphany_t Epiphany, *pEpiphany;
	e_mem_t      DRAM,     *pDRAM;
	unsigned int msize;

	off_t    mbox_addr;
	size_t   sz;
	FILE    *fo;
	unsigned coreid, corenum, row, col;
	core_str_t *rec;
	int i, j, k, datum, value, coredata_items;
	char fmt[255];


	// Initialize
	// ==========

	pEpiphany = &Epiphany;
	pDRAM     = &DRAM;
	msize     = 0x00400000;
	mbox_addr = offsetof(shared_buf_t, core); // sizeof(float) * 3 * _Smtx * _Smtx;

	fo = stderr;
	if (fo == NULL)
	{
		fprintf(stderr, "CAN'T OPEN OUTPUT FILE!!\n\n");
		exit(1);
	}

	e_set_host_verbosity(H_D0);
	e_init(NULL);

	if (e_open(pEpiphany, 0, 0, e_platform.chip[0].rows, e_platform.chip[0].cols))
	{
		fprintf(fo, "\nERROR: Can't establish connection to Epiphany device!\n\n");
		exit(1);
	}
	if (e_alloc(pDRAM, 0x00000000, msize))
	{
		fprintf(fo, "\nERROR: Can't allocate Epiphany DRAM!\n\n");
		exit(1);
	}

	// Process input arguments
	if (argc == 1) {
		fprintf(fo, "Enter core number (0-%d) to probe.\n\n", (pEpiphany->num_cores - 1));
		exit(1);
	} else {
		corenum = atoi(argv[1]);
	}


	// Read data from device
	// =====================

	e_get_coords_from_num(pEpiphany, corenum, &row, &col);
	e_read(pEpiphany, row, col, sregsaddr + SREGS[eCOREID].a, &coreid, sizeof(coreid)); // 0xfff; //

	fprintf(fo, "Probing Core ID 0x%03x\n", coreid);

//	fprintf(fo, "Reading core 0x%03x GP registers from address %08x...\n", coreid, dregsaddr);
//	for (i=0; i<nDREGS; i++)
//	{
//		fprintf(fo, "Reading register R[%2d]\n", i);
//		e_read(pEpiphany, corenum, dregsaddr + i * sizeof(DREGS[0]), &(DREGS[i]), sizeof(DREGS[i]));
//	}

	fprintf(fo, "Reading core 0x%03x system registers from address %08x...\n", coreid, sregsaddr);
	for (i=0; i<eEOF; i++)
	{
		fprintf(fo, "%s\n", SREGS[i].n);
		e_read(pEpiphany, row, col, sregsaddr + SREGS[i].a, &(SREGS[i].v), sizeof(SREGS[i].v));
	}

	sz = sizeof(coredata);
	coredata_items = 34;
	sz = sizeof(int) * coredata_items; // x86_64 pointers are 8-bytes wide, so put the explicit size here.
	fprintf(fo, "Reading core 0x%03x data structure [%uB] from address %08x...\n", coreid, sz, baseaddr);
	e_read(pEpiphany, row, col, baseaddr, (void *) &coredata, sz);

	sz = sizeof(mailbox);
	fprintf(fo, "Reading Mailbox [%uB] from address %08x...\n", sz, (unsigned int) mbox_addr);
	e_read(pDRAM, 0, 0, mbox_addr, (void *) mailbox, sz);


	// Write output
	// ============

	// Write GP registers
	fprintf(fo, "\n");
	for (i=0; i<(nDREGS/8); i++)
	{
		for (j=0; j<8; j++)
			fprintf(fo, "R[%2d] = 0x%08x  ", (i+j*(nDREGS/8)), DREGS[i+j*(nDREGS/8)]);
		fprintf(fo, "\n");
	}
	fprintf(fo, "\n");

	// Write system registers
	for (i=0; i<eEOF; i++)
		fprintf(fo, "%s = 0x%08x       ", SREGS[i].n, SREGS[i].v);
//	for (i=0; i<(eEOF/4); i++)
//	{
//		for (j=0; j<4; j++)
//			fprintf(fo, "%s = 0x%08x       ", SREGS[i+j*(eEOF/4)].n, SREGS[i+j*(eEOF/4)].v);
//		fprintf(fo, "\n");
//	}
//	fprintf(fo, "%s = 0x%08x\n", SREGS[eCONFIG].n, SREGS[eCONFIG].v);
//	fprintf(fo, "%s = 0x%08x\n", SREGS[eSTATUS].n, SREGS[eSTATUS].v);
//	fprintf(fo, "%s = 0x%08x\n", SREGS[ePC    ].n, SREGS[ePC    ].v);
//	fprintf(fo, "%s = 0x%08x\n", SREGS[eCOREID].n, SREGS[eCOREID].v);
	fprintf(fo, "\n");
	fprintf(fo, "\n");

	// Write core structure data
	rec = &core_str[0];
	datum = 0;
	while (rec->dim1 > 0)
	{
		for (i=0; i<rec->dim1; i++)
		{
			fprintf(fo, "0x%08x  %s = ", (baseaddr + datum * sizeof(int)), rec->str);
			for (j=0; j<rec->dim2; j++)
			{
				for (k=0; k<rec->dim3; k++)
				{
					value = *(((int *)(&coredata)) + datum);
					sprintf(fmt, "%s", rec->fmt);
					fprintf(fo, rec->fmt, value);

					datum++;
				}
			}
			fprintf(fo, "\n");
		}

		datum += rec->pad;
		rec++;
	}
	fprintf(fo, "\n");
	fprintf(fo, "\n");

	// Write Mailbox data
	rec = &mbox_str[0];
	datum = 0;
	while (rec->dim1 > 0)
	{
		for (i=0; i<rec->dim1; i++)
		{
//			fprintf(fo, "i=%03d  ", i);
			fprintf(fo, "0x%08x  %s = ", (unsigned int) (mbox_addr + datum * sizeof(int)), rec->str);
			for (j=0; j<rec->dim2; j++)
			{
//				fprintf(fo, "j=%03d  ", j);
				for (k=0; k<rec->dim3; k++)
				{
//					fprintf(fo, "k=%03d  ", k);
					value = *(((int *)(&mailbox)) + datum);
					sprintf(fmt, "%s", rec->fmt);
					fprintf(fo, rec->fmt, value);

					datum++;
				}
			}
			fprintf(fo, "\n");
		}

		datum += rec->pad;
		rec++;
	}


	fflush(fo);
	fclose(fo);

	// Close connection to e-server
	if (e_close(pEpiphany))
	{
		fprintf(fo, "\nERROR: Can't close connection to E-SERVER!\n\n");
		exit(1);
	}
	if (e_free(pDRAM))
	{
		fprintf(fo, "\nERROR: Can't release Epiphany DRAM!\n\n");
		exit(1);
	}

	e_finalize();

	return 0;
}
