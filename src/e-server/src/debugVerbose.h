/*
  File: debugVerbose.h

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

#ifndef DEBUGVERBOSE_H_
#define DEBUGVERBOSE_H_

enum DebugVerbose {
	D_STOP_RESUME_INFO = 0,
	D_TRAP_AND_RSP_CON = 1,
	D_STOP_RESUME_DETAIL = 2,
	D_TARGET_WR = 10,
	D_CTLR_C_WAIT = 15,
	D_TRAN_DETAIL = 20,
};

extern void   StartOfBaudMeasurement(struct timeval& start);
extern double EndOfBaudMeasurement(struct timeval& start);
#endif /* DEBUGVERBOSE_H_ */
