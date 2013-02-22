/*
  File: e_printf.cpp

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

/* NOT IN USE, created as patch for overcome external memory problem */
#include <string.h>
#include <stdio.h>

void printf_wrapper(char *result_str, const char *fmt, const char *args_buf)
{
	char *p          = (char *) fmt;
	char *b          = (char *) fmt;
	//char *perc;        = (char *) fmt;
	char *p_args_buf = (char *) args_buf;

	unsigned a, a1, a2, a3, a4;

	int found_percent = 0;

	char buf[2048];
	char tmp_str[2048];

	strcpy(result_str, "");
	//sprintf(result_str, "");

	//sprintf(buf, fmt);

	//printf("fmt ----%d ----\n", strlen(fmt));

	//puts(fmt);

	//printf("Parsing\n");

	while (*p)
	{
		if (*p == '%')
		{
			found_percent = 1;
			//perc = p;
		} else if (*p == 's' && (found_percent == 1))
		{
			found_percent = 0;

			strncpy(buf, b, (p-b)+1);
			buf[p-b+1] = '\0';
			b = p + 1;

			//puts(buf);

			//printf("args_buf ----%d ----\n", strlen(p_args_buf));
			//puts(p_args_buf);

			sprintf(tmp_str, buf, p_args_buf);
			sprintf(result_str, "%s%s", result_str, tmp_str);

			p_args_buf = p_args_buf + strlen(p_args_buf) + 1;

		} else if ((*p == 'p' || *p == 'X' || *p == 'u' || *p == 'i' || *p == 'd' || *p == 'x' || *p == 'f') && (found_percent == 1))
		{
			found_percent = 0;

			strncpy(buf, b, (p-b)+1);
			buf[p-b+1] = '\0';
			b = p + 1;

			//print out buf
			//puts(buf);

			a1 = (p_args_buf[0]); a1 &= 0xff;
			a2 = (p_args_buf[1]); a2 &= 0xff;
			a3 = (p_args_buf[2]); a3 &= 0xff;
			a4 = (p_args_buf[3]); a4 &= 0xff;

			//printf("INT <a1> %x <a2> %x <a3> %x <a4> %x\n", a1, a2,a3, a4);
			a = ((a1 << 24) | (a2 << 16) | (a3 << 8) | a4);

			if (*p == 'i')
			{
				//printf("I %i\n", a);
			} else if (*p == 'd')
			{
				//printf("D %d\n", a);
			} else if (*p == 'x')
			{
				//printf("X %x\n", a);
			} else if (*p == 'f')
			{
				//printf("F %f \n", *((float*)&a));
			} else if (*p == 'f') {
				sprintf(tmp_str, buf, *((float*) &a));
			} else {
				sprintf(tmp_str, buf, a);
			}
			sprintf(result_str, "%s%s", result_str, tmp_str);

			p_args_buf = p_args_buf + 4;
		}

		p++;
	}

	//tail
	//puts(b);
	sprintf(result_str, "%s%s", result_str, b);

	//printf("------------- %s ------------- %d ", result_str, strlen(result_str));
}
