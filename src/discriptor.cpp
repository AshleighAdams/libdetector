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

float GetDistance(motion_t* Motion, int Angle, int startx, int starty)
{
	float rads = ((float)Angle / 180.f) * M_PI;
	float xdir = cos(rads);
	float ydir = sin(rads);



	int	 x0 = startx,
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

		if(PMOTION_XY(Motion, x0, y0) != PIXEL_MOTION) break;

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

	return Q_sqrt(xx * xx + yy * yy);
}

char* CBaseDescriptor::GetDescriptor(motion_t* Motion)
{
	int LongestPos = 0;
	float flLongestDistance = 0;
	float RealDistances[360-1];

	int sx = Motion->size.width / 2, sy = Motion->size.width / 2;

	float distance;
	for(int i = 0; i < 360; i++)
	{
		distance = GetDistance(Motion, i, sx, sy);
		if(distance == 0) distance = 1;
		RealDistances[i] = distance;
		if(distance > flLongestDistance)
		{
			flLongestDistance = distance;
			LongestPos = i;
		}
	}

	for(int i = 0; i < 360; i++)
		m_Histogram.values[i] = RealDistances[i] / flLongestDistance;

	m_Histogram.highestvalue = LongestPos;

	return "";
}

bool CBaseDescriptor::LoadDescriptor(CDetectorImage* pImage)
{

	return false;
}

