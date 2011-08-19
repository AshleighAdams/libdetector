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

    int size = (center_x + center_y) / 2;

    float flScaleX = (float)center_x * flSizePercent;
    float flScaleY = (float)center_y * flSizePercent;

    // TODO: This inc var needs improving; good enough for now.
    int x,y,inc,total,motioncount;

    motioncount     = 0;
    const int pixel_increase_per_pixelout = 8;
    inc             = 360 / (pixel_increase_per_pixelout * size); // This many iterations (size)
    total           = 360 / inc; // Find out how many

    int rads;
    for(int i = 0; i < 360; i+=inc)
    {
        rads = (M_PI * 2) / i;
        x = (cos(rads) * flScaleX) + center_x;
        y = (sin(rads) * flScaleY) + center_y;

        //PRINT(x << " :: " << y);

        if(PMOTION_XY(Motion, x, y) == PIXEL_SCANNEDMOTION)
            motioncount++;
    }
    PRINT(motioncount << "::" << total);
    return (float)motioncount / (float)total;
}

CDescriptorValue* CBaseDescriptor::GetDescriptor(motion_t* Motion) // Might want to use CDetectorImage but shouldn't need to
{
    CDescriptorValue* ret = new CDescriptorValue;
    ret->g_Count = 3;
    ret->g_Values = new float[3];

    ret->g_Values[0] = GetRingDensity(Motion, 0.25f);
    ret->g_Values[1] = GetRingDensity(Motion, 0.5f);
    ret->g_Values[2] = GetRingDensity(Motion, 0.75f);

    return ret;
}

char* CBaseDescriptor::GetName(CDescriptorValue* Descriptor, int Count)
{
    return (char*)"Not Implented!";
}

