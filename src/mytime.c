#include "mytime.h"

int getTimes(myTimes *pTimes)
{
#if defined(_WIN64) || defined(_WIN32)
	/* Windows -------------------------------------------------- */
	FILETIME createTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;	
	LARGE_INTEGER freq, val;
	
	if ( !QueryPerformanceFrequency(&freq) )
		return 0;
		
	if ( !QueryPerformanceCounter(&val) )
		return 0;
	
	pTimes->realTime = (double)val.QuadPart / (double)freq.QuadPart;	
	
	if ( GetProcessTimes( GetCurrentProcess( ),
		&createTime, &exitTime, &kernelTime, &userTime ) != -1 )
	{
		SYSTEMTIME userSystemTime;
		SYSTEMTIME kernelSystemTime;
		
		if ( FileTimeToSystemTime( &userTime, &userSystemTime ) != -1 )
			pTimes->userTime = (double)userSystemTime.wHour * 3600.0 +
				(double)userSystemTime.wMinute * 60.0 +
				(double)userSystemTime.wSecond +
				(double)userSystemTime.wMilliseconds / 1000.0;
		else
			return 0;

		if ( FileTimeToSystemTime( &kernelTime, &kernelSystemTime ) != -1 )
			pTimes->systemTime = (double)kernelSystemTime.wHour * 3600.0 +
				(double)kernelSystemTime.wMinute * 60.0 +
				(double)kernelSystemTime.wSecond +
				(double)kernelSystemTime.wMilliseconds / 1000.0;
		else
			return 0;
			
		return 1;
	}

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
	/* AIX, BSD, Cygwin, HP-UX, Linux, OSX, and Solaris --------- */
	{
		const double ticks = (double)sysconf( _SC_CLK_TCK );
		struct tms tms;
		clock_t mytime = times(&tms);
		if ( mytime != (clock_t)-1 )
		{
			pTimes->realTime = 	(double)mytime / ticks;
			pTimes->userTime = (double)tms.tms_utime / ticks;
			pTimes->systemTime = (double)tms.tms_stime / ticks;
			
			return 1;
		}		
	}

#endif

	return 0;		/* Failed. */
}

/*
gcc -Wall -W -pedantic -O2 -std=c99 mytime.c -o mytime
*/

/*
int main()
{
	int i, k;

	myTimes mt_start;
	myTimes mt_end;
	
	getTimes(&mt_start);

	k = 0;
	for ( i = 0; i < 100000; i++ )
	{
		k += 5;
		printf("k = %d\n", k);
	}

	getTimes(&mt_end);
	
	printf("\nTempo impiegato(in secondi): \n");
    printf("Real Time: %5.5f\nUser Time: %5.5f\nSystem Time: %5.5f\n",
        (mt_end.realTime - mt_start.realTime),
        (mt_end.userTime - mt_start.userTime),
        (mt_end.systemTime - mt_start.systemTime));

	return 0;
}
*/
