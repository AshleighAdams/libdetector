#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../header/libdetector.h"
using namespace Detector;


// Gets the density of a ring
float GetRingDensity(motion_t* Motion, float flSizePercent)
{
    int center_x, center_y;
    center_x = Motion->size.width * 0.5f;
    center_y = Motion->size.height * 0.5f;

    float flScaleX = (float)center_x * flSizePercent;
    float flScaleY = (float)center_y * flSizePercent;

    // TODO: This inc var needs improving; good enough for now.
    int x,y,inc,total,motioncount;

    motioncount     = 0;
    inc             = (1.f - flSizePercent) / (M_PI * 2.f) + 1;
    total           = (M_PI * 2.f) / inc; // Total number of itterations

    for(float i = 0.f; i < M_PI * 2.f; i += inc)
    {
        x = (cos(i) * flScaleX) + center_x;
        y = (sin(i) * flScaleY) + center_y;

        //PRINT(x << " :: " << y);

        if(PMOTION_XY(Motion, x, y) == PIXEL_SCANNEDMOTION)
            motioncount++;
    }
    PRINT(motioncount);
    return (float)motioncount / (float)total;
}

CDiscriptorValue* CBaseDiscriptor::GetDiscriptor(motion_t* Motion) // Might want to use CDetectorImage but shouldn't need to
{
    CDiscriptorValue* ret = new CDiscriptorValue;
    ret->g_Count = 3;
    ret->g_Values = new float[3];

    ret->g_Values[0] = GetRingDensity(Motion, 0.25f);
    ret->g_Values[1] = GetRingDensity(Motion, 0.5f);
    ret->g_Values[2] = GetRingDensity(Motion, 0.75f);

    return ret;
}

char* CBaseDiscriptor::GetName(CDiscriptorValue* Discriptor, int Count)
{
    return (char*)"Not Implented!";
}

