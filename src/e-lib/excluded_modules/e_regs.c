/*
 * e_regs.c
 *
 *	$Date$
 *	$Rev$
 *	$Author$
 *
 *  $Copyright$
 *  $License$
 */

#include "e_regs.h"
#include "e_coreid.h"

unsigned e_sysreg_read(e_reg_id_t reg_id)
{
	volatile register unsigned reg_val;
	unsigned *addr;

	switch (reg_id)
	{
	case E_CONFIG:
		__asm__ __volatile__ ("MOVFS %0,config" : "=r" (reg_val) : );
		return reg_val;
	case E_STATUS:
		__asm__ __volatile__ ("MOVFS %0,status" : "=r" (reg_val) : );
		return reg_val;
	case E_COREID:
		__asm__ __volatile__ ("MOVFS %0,coreid" : "=r" (reg_val) : );
		return reg_val;
	default:
		addr = (unsigned *) e_address_from_coreid(e_get_coreid(), (void *) reg_id);
		return *addr;
	}
}

void e_sysreg_write(e_reg_id_t reg_id, unsigned val)
{
	volatile register unsigned reg_val = val;
	unsigned *addr;

	switch (reg_id)
	{
	case E_CONFIG:
		__asm__ __volatile__ ("MOVTS config,%0" : : "r" (reg_val));
		break;
	case E_STATUS:
		__asm__ __volatile__ ("MOVTS status,%0" : : "r" (reg_val));
		break;
	case E_PC:
		break;
	default:
		addr = (unsigned *) e_address_from_coreid(e_get_coreid(), (void *) reg_id);
		*addr = val;
		break;
	}
}

