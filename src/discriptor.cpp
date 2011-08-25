#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../include/libdetector.h"
using namespace Detector;

CDescriptorValue::CDescriptorValue()
{
    m_iRefrenceCount = 0;
    Refrence();
}

CDescriptorValue::~CDescriptorValue()
{
    delete [] g_Values;
}

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
    if(inc < 1) inc = 1;
    total           = 360 / inc; // Find out how many

    int rads;
    for(int i = 0; i < 360; i+=inc)
    {
        rads = M_PI / 180.f * (float)i;
        x = (cos(rads) * flScaleX) + center_x;
        y = (sin(rads) * flScaleY) + center_y;

        if(PMOTION_XY(Motion, x, y) == PIXEL_SCANNEDMOTION)
            motioncount++;
    }
    return (float)motioncount / (float)total;
}

CBaseDescriptor::CBaseDescriptor()
{
    m_iRefrenceCount = 0;
    Refrence();
}

CBaseDescriptor::~CBaseDescriptor()
{
}

CDescriptorValue* CBaseDescriptor::GetDescriptor(motion_t* Motion)
{
    CDescriptorValue* ret = new CDescriptorValue;
    ret->g_Count = 3;
    ret->g_Values = new float[3];

    ret->g_Values[0] = GetRingDensity(Motion, 0.25f);
    ret->g_Values[1] = GetRingDensity(Motion, 0.5f);
    ret->g_Values[2] = GetRingDensity(Motion, 0.75f);

    PRINT(ret->g_Values[0] << " :: " << ret->g_Values[1] << " :: " << ret->g_Values[2] << "\n");

    return ret;
}

char* CBaseDescriptor::GetName(CDescriptorValue* Descriptor)
{
    return (char*)"Not Implemented!";
}

bool CBaseDescriptor::LoadDescriptor(char* File)
{
    return false;
}

