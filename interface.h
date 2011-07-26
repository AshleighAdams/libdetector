#ifndef _DET_INTERFACE_LOAD
#error Do not include __FILE__ directly!
#endif
#undef _DET_INTERFACE_LOAD

#ifndef LIB_DET_INTERFACE_H
#define LIB_DET_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>

#define MAX_TARGETS 25 // 25 Seems enough, want more? recompile

namespace Detector
{

    struct imagesize_t
    {
        int width;
        int height;
    };

    bool imagesize_tEqual( imagesize_t a, imagesize_t b );

    // Provides a byte for each pixel
    struct motion_t
    {
        imagesize_t size;
        unsigned char* motion;
    };

    struct motionhelper_t
    {
        imagesize_t size;
        int MinX;
        int MinY;
        int MaxX;
        int MaxY;
        unsigned char* motion;
    };

    struct pixel_t
    {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };

    class CDetectorBaseClass
    {
    public:
        virtual ~CDetectorBaseClass() {}; // GARH, this is so out real destructor is called!
        void Refrence( void )
        {
            m_iRefrenceCount++;
        };
        void UnRefrence( void )
        {
            m_iRefrenceCount--;
            if( m_iRefrenceCount == 0 )
                delete this;
        };
    protected:
        int m_iRefrenceCount;
    };


    class CDetectorImage : public CDetectorBaseClass // Immutable
    {
    public:
        CDetectorImage( int Width, int Height )
        {
            this->Refrence();
            m_sSize.width = Width;
            m_sSize.height = Height;
            m_psPixels = new pixel_t[Width + Width * Height];
        };
        CDetectorImage( imagesize_t size )
        {
            this->Refrence();
            m_sSize.width = size.width;
            m_sSize.height = size.height;
            m_psPixels = new pixel_t[size.width + size.width * size.height];
        };
        ~CDetectorImage()
        {
            delete[] m_psPixels;
        };
        CDetectorImage* Exclusive()
        {
            if( m_iRefrenceCount == 1 )
                return this;
            imagesize_t selfsize = this->GetSize();
            int size_total = ( selfsize.width + selfsize.width * selfsize.height ) * 3; // 3 bytes per pixel;
            CDetectorImage* ptr = new CDetectorImage( selfsize );
            memcpy( ptr->m_psPixels, this->m_psPixels, size_total );
            this->UnRefrence();
            return ptr;
        };
        pixel_t* Pixel( int x, int y )
        {
            return &m_psPixels[x + y * m_sSize.width];
        };
        imagesize_t GetSize()
        {
            return m_sSize;
        };
    private:
        imagesize_t m_sSize;
        pixel_t* m_psPixels;
    };

    struct target_t
    {
        float x;
        float y;
        float width;
        float height;
    };

    class IDetector : public CDetectorBaseClass
    {
    public:
        // Push the next image here, calculates the new target
        virtual bool PushImage( CDetectorImage* Image ) = 0;
        // Returns the number of targets, outputs to argument
        virtual int GetTargets( target_t* Targets[MAX_TARGETS] ) = 0;
    };

    struct velocity_t
    {
        float x;
        float y;
    };

    struct position_t
    {
        float x;
        float y;
    };

    double GetCurrentTime();

    class IDetectorObjectTracker;

    typedef unsigned int targetid;
    class CTrackedObject : public CDetectorBaseClass
    {
        friend class IDetectorObjectTracker;
    public:
        CTrackedObject(imagesize_t ImgSize, targetid ID );
        ~CTrackedObject();
        targetid    ID();
        position_t  Position();
        velocity_t  Velocity();
        double      LastSeen();
    private:
        void        UpdatePosition(position_t Pos);
        imagesize_t m_ImageSize;
        targetid    m_tiID;
        velocity_t  m_sVelocity;
        position_t  m_sPosition;
        double      m_dblLastSeen;
    };

    typedef std::vector<CTrackedObject*> TrackedObjects;

    class IDetectorObjectTracker : public CDetectorBaseClass
    {
    public:
        virtual void PushTargets( target_t* Targets[MAX_TARGETS], int Count ) = 0;
        virtual TrackedObjects* GetTrackedObjects() = 0;
    };

}
#endif // LIB_DET_INTERFACE_H
