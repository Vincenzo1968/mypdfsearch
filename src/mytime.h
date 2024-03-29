/*
   Copyright (C) 2019 Vincenzo Lo Cicero

   Author: Vincenzo Lo Cicero.
   e-mail: vincenzo.locicero@libero.it
          
   This file is part of mypdfsearch.

   mypdfsearch is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   mypdfsearch is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mypdfsearch.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef	__MYTIME_H
#define	__MYTIME_H

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */


#if defined(_WIN64) || defined(_WIN32)
   #include <Windows.h> 
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
   /* #define _GNU_SOURCE */
   #include <unistd.h>
   #include <sys/times.h>
   #include <time.h>
#else
   #error "Unable to define getTimes(myTimes *pTimes) for an unknown OS."
#endif

#include <stdio.h>
#include <stdlib.h>

typedef struct _myTimes
{
	double realTime;
	double userTime;
	double systemTime;
} myTimes;


BEGIN_C_DECLS

int getTimes(myTimes *pTimes);

END_C_DECLS

#endif /* __MYTIME_H */
