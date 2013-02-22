/*
  File: e_mutex.h

  This file is part of the Epiphany Software Development Kit.

  Copyright (C) 2013 Adapteva, Inc.
  Contributed by Oleg Raikhman, Jim Thomas, Yaniv Sapir <support@adapteva.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License (LGPL)
  as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  and the GNU Lesser General Public License along with this program,
  see the files COPYING and COPYING.LESSER.  If not, see
  <http://www.gnu.org/licenses/>.
*/

#ifndef MUTEX_H_
#define MUTEX_H_


#include "e_types.h"


/**
 * @file e_lock.h
 * @brief The manage multi-core lock
 *
 * @section DESCRIPTION
 * The e_mLock functions manage multi-core atomic lock/unlock operations. The lock resource in remote/local core can accessed by coordinates and lock number.\n
 * The \a C_MAX_SUPPORTED_LOCKS defines the number supported locks for each core in the system. The lock call is blocking.
 *
 * @section EXAMPLE
 *	e_mutex_t mymutex;
 *
 *	if (e_mutex_init(&mymutex, NULL))
 *	{
 *		return -1;
 *	}
 *
 *	if (e_mutex_lock(&mymutex))
 *	{
 *		return -1;
 *	}
 *	// operation requiring mutual exclusion goes here
 *	if (e_mutex_unlock(&mymutex))
 *	{
 *		return -1;
 *	}
 *
 *	// Try to get a mutex repeatedly until we've tried too many times
 *	while (e_e_mutex_trylock(&mymutex) && --timeout>0)
 *	{
 *		sleep(1);
 *	}
 *
 *	if (e_mutex_unlock(&mymutex))
 *	{
 *		return -1;
 *	}
 *	if (e_mutex_destory(&mymutex))
 *	{
 *		return -1;
 *	}
 */

typedef struct
{
	unsigned int m;
} e_mutex_t;

typedef struct
{
	unsigned int a;
} e_mutexattr_t;

//-- for user to initialize a mutex structure
#define MUTEX_NULL     {0}
#define MUTEXATTR_NULL {0}
#define MUTEXATTR_DEFAULT MUTEXATTR_NULL


/** @fn int e_mutex_lock(e_mutex_t *mutex);
    @brief  Lock a mutex (keep trying until mutex acquired)
    @param  mutex Global pointer to a mutex of type e_mutex_t.
    @return The function returns zero when successful.
*/

int e_mutex_lock(e_mutex_t *mutex);

/** @fn int e_mutex_unlock(e_mutex_t *mutex);
    @brief  Unlock a mutex
    @param  mutex Global pointer to a mutex of type e_mutex_t.
    @return The function returns zero when successful.
*/
int e_mutex_unlock(e_mutex_t *mutex);


/** @fn int e_e_mutex_trylock(e_mutex_t *mutex);
    @brief  Attempt to lock a mutex (return after one failed attempt)
    @param  mutex Global pointer to a mutex of type e_mutex_t.
    @return The function returns zero when successful, EBUSY if the mutex was locked.
*/
int e_mutex_trylock(e_mutex_t *mutex);

/** @fn int mutex_init(e_mutex_t *mutex, e_mutexattr_t *attr);
    @brief  Initialize a mutex structure
    @param  mutex Global pointer to a mutex of type e_mutex_t.
    @param  attr Pointer to mutex attributes - NULL for default.
    @return The function returns zero when successful.
*/
int e_mutex_init(e_mutex_t *mutex, e_mutexattr_t *attr);

/** @fn int int mutex_destroy(e_mutex_t *mutex);
    @brief  Destroy (de-initialize) a mutex structure
    @param  mutex Global pointer to a mutex of type e_mutex_t.
    @return The function returns zero when successful.
*/
int e_mutex_destroy(e_mutex_t *mutex);

#endif /* MUTEX_H_ */
