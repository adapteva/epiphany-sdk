/*
 ============================================================================
 Name        : e_memprotect.h
 Author      : $Author: oraikhman $
 $Date: 2011-09-28 15:30:32 +0300 (Wed, 28 Sep 2011) $
 $Rev: 745 $
*/
/*
  Copyright (c) 2010,2011 All Right Reserved, Adapteva, Inc.

  This source is subject to the Adapteva Software License.
  You should have received a copy of the full license file
  named ("LICENSE") along with this program.  If not, contact
  Adapteva immediately at support@adapteva.com
*/

/**
 * @file e_memprotect.h
 * @brief The memory region protection
 *
 * The memory protection routines allow to prevent the write access to the internal memory banks. Each half bank can be configured independently.
 * The PAGE FAULT exception will be occurred in case of write to the protected region.
 */



#ifndef MEMPROTECT_H_
#define MEMPROTECT_H_


/** @enum MEM_PROTECT_REGIONS
 * The 4KB (half bank) memory regions
 */

typedef enum MEM_PROTECT_REGIONS {
	MEM_0_4K=0,
	MEM_4_8K=1,
	MEM_8_12K=2,
	MEM_12_16K=3,
	MEM_16_20K=4,
	MEM_20_24K=5,
	MEM_24_28K=6,
	MEM_28_32K=7
} E_MEM_REGION;

/** @fn void e_memProtectOn(E_MEM_REGION mem_region);
    @brief Turn on \a mem_region protection.
    @param mem_region The memory region to be set
*/
void e_memProtectOn(E_MEM_REGION mem_region);

/** @fn void e_memProtectOff(E_MEM_REGION mem_region);
    @brief Turn off \a mem_region protection.
    @param mem_region
*/
void e_memProtectOff(E_MEM_REGION mem_region);


/** @fn void e_memProtectAll();
    @brief Protect all internal memory from writing. The read only sections can be placed in the internal banks.
    @param none
 */
void e_memProtectAll();


#endif /* MEMPROTECT_H_ */
