// This file contains implentations for stuff that is not directly related to the tracker, but is used


#include "string.h"
#include <iostream>
#include <exception>

#define _USE_MATH_DEFINES
#include <cmath>

#define DETECTOR_INTERNAL
#include "../header/libdetector.h"
using namespace Detector;



#ifdef WINDOWS // We need a standard for this...
/*
    Thanks to Carl Staelin for this snippet
    http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
*/
//#define CLOCKS_PER_SEC 0
LARGE_INTEGER getFILETIMEoffset()
{
	SYSTEMTIME s;
	FILETIME f;
	LARGE_INTEGER t;

	s.wYear = 1970;
	s.wMonth = 1;
	s.wDay = 1;
	s.wHour = 0;
	s.wMinute = 0;
	s.wSecond = 0;
	s.wMilliseconds = 0;
	SystemTimeToFileTime( &s, &f );
	t.QuadPart = f.dwHighDateTime;
	t.QuadPart <<= 32;
	t.QuadPart |= f.dwLowDateTime;
	return t;
}

int ::clock_gettime( int X, struct timeval* tv )
{
	LARGE_INTEGER           t;
	FILETIME                f;
	double                  microseconds;
	static LARGE_INTEGER    offset;
	static double           frequencyToMicroseconds;
	static int              initialized = 0;
	static BOOL             usePerformanceCounter = 0;

	if ( !initialized )
	{
		LARGE_INTEGER performanceFrequency;
		initialized = 1;
		usePerformanceCounter = QueryPerformanceFrequency( &performanceFrequency );
		if ( usePerformanceCounter )
		{
			QueryPerformanceCounter( &offset );
			frequencyToMicroseconds = ( double )performanceFrequency.QuadPart / 1000000.;
		}
		else
		{
			offset = getFILETIMEoffset();
			frequencyToMicroseconds = 10.;
		}
	}
	if ( usePerformanceCounter )
        QueryPerformanceCounter( &t );
	else
	{
		GetSystemTimeAsFileTime( &f );
		t.QuadPart = f.dwHighDateTime;
		t.QuadPart <<= 32;
		t.QuadPart |= f.dwLowDateTime;
	}

	t.QuadPart -= offset.QuadPart;
	microseconds = ( double )t.QuadPart / frequencyToMicroseconds;
	t.QuadPart = microseconds;
	tv->tv_sec = t.QuadPart / 1000000;
	tv->tv_usec = t.QuadPart % 1000000;
	return 0;
}
#endif // WINDOWS

double Detector::GetCurrentTime()
{
	struct timespec now;
	clock_gettime( CLOCK_MONOTONIC, &now );

    // return ( double )( now.tv_nsec / CLOCKS_PER_SEC ) / 1000.0 + ( double )now.tv_sec; // Why was I deviding by 0?
	return ( double )( now.tv_nsec ) / 1000.0 + ( double )now.tv_sec;
}

bool Detector::imagesize_tEqual( imagesize_t a, imagesize_t b )
{
	return a.width == b.width && a.height == b.height;
}

float GetRingDensity(motion_t* Motion, float SizePercent)
{
    int center_x, center_y;
    center_x = Motion->size.width * 0.5f;
    center_y = Motion->size.height * 0.5f;

    int motioncount = 0;
    int total;

    // TODO: This inc var needs improving; good enough for now.
    int x,y;
    const float inc = (1.0f - SizePercent) / (M_PI * 2);
    total = (M_PI * 2.0f) / inc; // Total number of itterations

    for(float i = 0.0f; i < M_PI * 2.0f; i += inc)
    {
        x = ((1.0f + cos(i)) * 0.5f) * ((float)center_x * SizePercent);
        y = ((1.0f + sin(i)) * 0.5f) * ((float)center_y * SizePercent);
        if(PMOTION_XY(Motion, x, y) == PIXEL_MOTION)
            motioncount++;
    }
    return motioncount / total;
}

float Detector::GetDiscriptor(target_t* Targ, motion_t* Motion) // Might want to use CDetectorImage but shouldn't need to
{




    return 0.0f;
}






