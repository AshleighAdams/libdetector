// This file contains implentations for stuff that is not directly related to the tracker, but is used


#include "string.h"
#include <iostream>
#include <exception>

#define _USE_MATH_DEFINES
#include <cmath>

#define DETECTOR_INTERNAL
#include "../include/libdetector.h"
using namespace Detector;
using namespace std;


#ifdef WINDOWS // We need a standard for this...
/*
    Thanks to Carl Staelin for this snippet
    http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
*/
#define CLOCKS_PER_SEC 0
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

double StartTime = -1.0;
double Detector::GetCurrentTime()
{
    if(StartTime < 0.0)
    {
        struct timespec now;
        clock_gettime( CLOCK_MONOTONIC, &now );

        StartTime = ( double )( now.tv_nsec / CLOCKS_PER_SEC ) / 1000.0 + ( double )now.tv_sec;
    }
	struct timespec now;
	clock_gettime( CLOCK_MONOTONIC, &now );

    return (( double )( now.tv_nsec / CLOCKS_PER_SEC ) / 1000.0 + ( double )now.tv_sec) - StartTime; // Why was I deviding by 0?
	//return ( double )( now.tv_nsec ) / 1000.0 + ( double )now.tv_sec;
}

bool Detector::imagesize_tEqual( imagesize_t a, imagesize_t b )
{
	return a.width == b.width && a.height == b.height;
}

void Detector::MotionBlur(CDetectorImage* Refrence, CDetectorImage* New, float flBlurAmmount, float flMaxChange)
{
    if( !imagesize_tEqual(Refrence->GetSize(), New->GetSize()) )
        return;
    imagesize_t size = Refrence->GetSize();
    pixel_t *pixa, *pixb;
    XY_LOOP(size.width, size.height)
    {
        pixa = Refrence->Pixel(x,y);
        pixb = New->Pixel(x,y);

        pixa->r -= max(-flMaxChange, min(flMaxChange, (float)(pixa->r - pixb->r) * flBlurAmmount));
        pixa->g -= max(-flMaxChange, min(flMaxChange, (float)(pixa->g - pixb->g) * flBlurAmmount));
        pixa->b -= max(-flMaxChange, min(flMaxChange, (float)(pixa->b - pixb->b) * flBlurAmmount));
    }
}

// This blur is so the "blobs" join together much better

void Detector::BlurMotion(motion_t* motion)
{
    unsigned char* newmotion = new unsigned char[motion->size.width + motion->size.height * motion->size.width];
    int w = motion->size.width;
    int h = motion->size.height;
    int blursize = 3;

    int blur, start, end;
    XY_LOOP(w,h)
    {
        blur = 0;

        start = max(0, y - blursize);
        end = min(h, y + blursize + 1);
        for(int i = start; i < end; i++)
            blur += PMOTION_XY(motion, x, i);

        start = max(0, x - blursize);
        end = min(w, x + blursize + 1);
        for(int i = start; i < end; i++)
            blur += PMOTION_XY(motion, i, y);

        newmotion[x + y * w] = blur > 0; // Change to >1 to reduce noise
    }
    delete [] motion->motion;
    motion->motion = newmotion;
}

void S_BlurMotion(motion_t* motion)
{
    unsigned char* newmotion = new unsigned char[motion->size.width + motion->size.height * motion->size.width];

    XY_LOOP(motion->size.width, motion->size.height)
    {
        if(x == 0 || x == motion->size.width - 1) continue;
        if(y == 0 || y == motion->size.height - 1) continue;

        short has = 0;
        has += PMOTION_XY(motion, x-1,  y-1);
        has += PMOTION_XY(motion, x,    y-1);
        has += PMOTION_XY(motion, x+1,  y-1);

        has += PMOTION_XY(motion, x-1,  y+1);
        has += PMOTION_XY(motion, x,    y+1);
        has += PMOTION_XY(motion, x+1,  y+1);

        has += PMOTION_XY(motion, x-1, y);
        has += PMOTION_XY(motion, x+1, y);

        has += PMOTION_XY(motion, x, y);


        newmotion[x + y * motion->size.width] = has > 1 ? PIXEL_MOTION : PIXEL_NOMOTION;
    }

    delete [] motion->motion;
    motion->motion = newmotion;
}

pixel_t* _pix;
void inline SetPixelRed(CDetectorImage* Img, int x, int y)
{
    _pix = Img->Pixel(x,y);
    _pix->r = 255;
    _pix->g = 0;
    _pix->b = 0;
}
void inline SetPixelGreen(CDetectorImage* Img, int x, int y)
{
    _pix = Img->Pixel(x,y);
    _pix->r = 0;
    _pix->g = 255;
    _pix->b = 0;
}


