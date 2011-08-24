#include "string.h"
#include <iostream>
#include <exception>

#define _USE_MATH_DEFINES
#include <cmath>

#define DETECTOR_INTERNAL
#include "../include/libdetector.h"
using namespace Detector;
using namespace std;

CDetectorImage::CDetectorImage( int Width, int Height )
{
    this->Refrence();
    m_sSize.width = Width;
    m_sSize.height = Height;
    m_psPixels = new pixel_t[Width + Width * Height];
}

CDetectorImage::CDetectorImage( imagesize_t size )
{
    this->Refrence();
    m_sSize.width = size.width;
    m_sSize.height = size.height;
    m_psPixels = new pixel_t[size.width + size.width * size.height];
}

CDetectorImage::~CDetectorImage()
{
    delete[] m_psPixels;
}

CDetectorImage* CDetectorImage::Exclusive()
{
    if( m_iRefrenceCount == 1 )
        return this;
    imagesize_t selfsize = this->GetSize();
    int size_total = ( selfsize.width + selfsize.width * selfsize.height ) * 3; // 3 bytes per pixel;
    CDetectorImage* ptr = new CDetectorImage( selfsize );
    memcpy( ptr->m_psPixels, this->m_psPixels, size_total );
    this->UnRefrence();
    return ptr;
}

pixel_t* CDetectorImage::Pixel( int x, int y )
{
    return &m_psPixels[x + y * m_sSize.width];
}

imagesize_t CDetectorImage::GetSize()
{
    return m_sSize;
}
