/*
 ============================================================================
 Name        : e_gpio.h
 Author      : $Author: jimthomas $
 $Date: 2011-09-08 21:42:28 +0300 (Thu, 08 Sep 2011) $
 $Rev: 745 $
*/
/*
  Copyright (c) 2010,2011 All Right Reserved, Adapteva, Inc.

  This source is subject to the Adapteva Software License.
  You should have received a copy of the full license file
  named ("LICENSE") along with this program.  If not, contact
  Adapteva immediately at support@adapteva.com
*/



#ifndef _E_GPIO_H_
#define _E_GPIO_H_

typedef enum
{
	DIR_NORTH = 0,
	DIR_EAST  = 1,
	DIR_SOUTH = 2,
	DIR_WEST  = 3
} LinkDir;

typedef enum
{
	LINK_NORM = 0,
	LINK_LOOP = 1
} LinkMode;

int  e_setLinkMode(LinkDir dir, LinkMode mode);
void e_FlagEnable(void);
void e_FlagWrite(int val);

#endif // _E_GPIO_H_
