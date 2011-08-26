#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../include/libdetector.h"
using namespace Detector;
using namespace std;

CDescriptorValue::CDescriptorValue()
{
    m_iRefrenceCount = 0;
    Refrence();
}

CDescriptorValue::~CDescriptorValue()
{
}


CBaseDescriptor::CBaseDescriptor()
{
    m_iRefrenceCount = 0;
    Refrence();
}

CBaseDescriptor::~CBaseDescriptor()
{
}

int GetDistance(motion_t* Motion, int Angle, int startx, int starty)
{
    int     x0 = a.x * m_sSize.width,
            y0 = a.y * m_sSize.height,
            x1 = b.x * m_sSize.width,
            y1 = b.y * m_sSize.height;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;

    int err = dx - dy;
    int e2;
    while(true)
    {
        SETPIX(x0,y0);
        if(x0 == x1 && y0 == y1) break;
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
}

char* CBaseDescriptor::GetDescriptor(motion_t* Motion)
{
    int LongestPos = 0;
    int LongestDistance = 0;
    unsigned int RealDistances[360-1];

    int distance;
    for(int i = 0; i < 360; i++)
    {
        distance = GetDistance(Motion, i);
    }

    return ret;
}

bool CBaseDescriptor::LoadDescriptor(CDetectorImage* pImage)
{

    return false;
}

