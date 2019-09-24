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
