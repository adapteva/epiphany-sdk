/*
 * e_memprotect.c
 *
 *	$Date: 2011-04-06 09:17:13 -0400 (Wed, 06 Apr 2011) $
 *	$Rev: 483 $
 *	$Author: oraikhman $
 *
 *  $Copyright$
 *  $License$
 */
#include "e_memprotect.h"

void e_memProtectOn(E_MEM_REGION n) {
	volatile register unsigned mem_protect_val = n;

	__asm__ __volatile__ ("movfs %0, memprotect" : "=r" (mem_protect_val) : "r" (mem_protect_val));

	mem_protect_val = mem_protect_val | (1<<n);

	__asm__ __volatile__ ("movts memprotect, %0 " : "=r" (mem_protect_val) : "r" (mem_protect_val));


}
void e_memProtectOff(E_MEM_REGION n) {
	volatile register unsigned mem_protect_val = n;

	__asm__ __volatile__ ("movfs %0, memprotect" : "=r" (mem_protect_val) : "r" (mem_protect_val));

	mem_protect_val = mem_protect_val & (~(1<<n));

	__asm__ __volatile__ ("movts memprotect, %0 " : "=r" (mem_protect_val) : "r" (mem_protect_val));
}
void e_memProtectAll() {

	//volatile register unsigned mem_protect_addr = 0x820f0604;
	volatile register unsigned mem_protect_val = 0xff;

	//__asm__ __volatile__ ("MOV %0,0x0608" : "=r" (mem_protect_addr) : "r" (mem_protect_addr));
	//__asm__ __volatile__ ("MOVT %0,0x820f" : "=r" (mem_protect_addr) : "r" (mem_protect_addr));

	//__asm__ __volatile__ ("movts memprotect, %0 " : "=r" (mem_protect_val) : "r" (mem_protect_val), "r" (mem_protect_val));
	//__asm__ __volatile__ ("str %0,[%1,0]": "=r" (mem_protect_val) : "r" (mem_protect_addr), "r" (mem_protect_val));

	__asm__ __volatile__ ("movts memprotect, %0 " : "=r" (mem_protect_val) : "r" (mem_protect_val));
}
