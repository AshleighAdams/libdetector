#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../include/libdetector.h"
using namespace Detector;
using namespace std;

CBaseDescriptor::CBaseDescriptor()
{
    m_iRefrenceCount = 0;
    Refrence();
}

CBaseDescriptor::~CBaseDescriptor()
{
}

float Q_sqrt( float number ) // Thanks whoever made this (this implentation is from Quake III Arena)
{
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    return number * y;
}

int GetDistance(motion_t* Motion, int Angle, int startx, int starty)
{
    float rads = (360.f / 180.f) * M_PI;
    float xdir = cos(rads);
    float ydir = sin(rads);



    int     x0 = startx,
            y0 = starty,
            x1 = startx + 100.f * xdir, // TODO: Fix 100.f for guess size
            y1 = starty + 100.f * ydir;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;

    int err = dx - dy;
    int e2;
    int count = 0;
    while(true)
    {
        if(x0 == x1 && y0 == y1) break;
        if(x0 < 0 || x0 > Motion->size.width) break;
        if(y0 < 0 || y0 > Motion->size.height) break;

        count++;

        e2 = 2 * err;
        if(e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if(e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }

    float xx = abs(startx - x0);
    float yy = abs(starty - y0);

    return (int)Q_sqrt(xx * xx + yy * yy);
}

char* CBaseDescriptor::GetDescriptor(motion_t* Motion)
{
    int LongestPos = 0;
    int LongestDistance = 0;
    unsigned int RealDistances[360-1];

    int sx = Motion->size.width / 2, sy = Motion->size.width / 2;

    int distance;
    for(int i = 0; i < 360; i++)
    {
        distance = GetDistance(Motion, i, sx, sy);
    }

    return "";
}

bool CBaseDescriptor::LoadDescriptor(CDetectorImage* pImage)
{

    return false;
}

